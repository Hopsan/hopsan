/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ModelWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-06-25
//!
//! @brief Contain the class for model widgets
//!
//$Id$

//Qt includes
#include <QGraphicsColorizeEffect>
#include <QSizePolicy>
#include <QHash>
#include <QFileDialog>
#include <QLabel>
#include <QHBoxLayout>


//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "Utilities/GUIUtilities.h"
//#include "HcomWidget.h"
#include "InitializationThread.h"
#include "ModelHandler.h"
//#include "ProgressBarThread.h"
#include "ProjectTabWidget.h"
#include "SimulationThreadHandler.h"
//#include "Utilities/XMLUtilities.h"
#include "version_gui.h"
#include "Widgets/AnimationWidget.h"
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/QuickNavigationWidget.h"
#include "SymHop.h"

//Needed for Modelica experiments, move later if necessary
#include "ModelicaLibrary.h"
#include "LibraryHandler.h"
#include "GUIPort.h"
#include "PlotWidget.h"


//! @class ModelWidget
//! @brief The ModelWidget class is a Widget to contain a simulation model
//!
//! ModelWidget contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ModelWidget::ModelWidget(ModelHandler *pModelHandler, CentralTabWidget *parent)
    : QWidget(parent)
{
    mpMessageHandler = 0;
    mpAnimationWidget = 0;

    mStartTime.setNum(0.0,'g',10);
    mStopTime.setNum(10.0,'g',10);

    mEditingEnabled = true;
    this->setPalette(gpConfig->getPalette());
    this->setMouseTracking(true);

    mpParentModelHandler = pModelHandler;
    mpQuickNavigationWidget = new QuickNavigationWidget(this);

    mpExternalSystemWidget = new QWidget(this);
    QLabel *pExternalSystemLabel = new QLabel("<font color='darkred'>External Subsystem (editing disabled)</font>", mpExternalSystemWidget);
    QFont tempFont = pExternalSystemLabel->font();
    tempFont.setPixelSize(18);
    tempFont.setBold(true);
    pExternalSystemLabel->setFont(tempFont);
    pExternalSystemLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QPushButton *pOpenExternalSystemButton = new QPushButton("Load As Internal System");
    pOpenExternalSystemButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(pOpenExternalSystemButton, SIGNAL(clicked()), this, SLOT(openCurrentContainerInNewTab()));
    QHBoxLayout *pExternalSystemLayout = new QHBoxLayout();
    pExternalSystemLayout->addWidget(pExternalSystemLabel);
    pExternalSystemLayout->addStretch(1);
    pExternalSystemLayout->addWidget(pOpenExternalSystemButton);
    mpExternalSystemWidget->setLayout(pExternalSystemLayout);
    mpExternalSystemWidget->hide();

    mpToplevelSystem = new SystemContainer(this, 0);
    mpSimulationThreadHandler = new SimulationThreadHandler();
    setMessageHandler(gpMessageHandler);

    connect(mpSimulationThreadHandler, SIGNAL(done(bool)), this, SIGNAL(simulationFinished()));
    connect(gpModelHandler->mpSimulationThreadHandler, SIGNAL(done(bool)), this, SIGNAL(simulationFinished()));
    connect(this, SIGNAL(simulationFinished()), this, SLOT(unlockSimulateMutex()));
    connect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()), Qt::UniqueConnection);

    emit checkMessages();

    mIsSaved = true;

    mpGraphicsView  = new GraphicsView(this);
    mpGraphicsView->setScene(mpToplevelSystem->getContainedScenePtr());

//#ifdef XMAS
//    QLabel *pBalls = new QLabel(this);
//    QPixmap imageStars;
//    imageStars.load(QString(GRAPHICSPATH) + "balls.png");
//    pBalls->setPixmap(imageStars);
//    pBalls->setAlignment(Qt::AlignRight | Qt::AlignTop);
//    pBalls->setFixedWidth(200);
//    pBalls->setFixedHeight(217);
//    pBalls->setAttribute(Qt::WA_TransparentForMouseEvents, true);
//    //mpCentralGridLayout->addWidget(pStars,0,0,1,1);
//#endif

    //QVBoxLayout *tabLayout = new QVBoxLayout(this);
    QGridLayout *tabLayout = new QGridLayout(this);
    tabLayout->setSpacing(0);
    tabLayout->addWidget(mpQuickNavigationWidget,0,0,1,2);
    tabLayout->addWidget(mpGraphicsView,1,0,2,2);
//#ifdef XMAS
//    tabLayout->addWidget(pBalls, 1,1);
//#endif
    tabLayout->addWidget(mpExternalSystemWidget,3,0);
    //this->setLayout(tabLayout);

    mpGraphicsView->centerView();

    mLastSimulationTime = 0;
}


ModelWidget::~ModelWidget()
{
    delete mpAnimationWidget;

    //First make sure that we go to the top level system, we dont want to be inside a subsystem while it is beeing deleted
    this->mpQuickNavigationWidget->gotoContainerAndCloseSubcontainers(0);
    //Now delete the root system, first remove in core (will also trigger delete for all sub modelobjects)
    mpToplevelSystem->deleteInHopsanCore();
    mpToplevelSystem->deleteLater();
    mpSimulationThreadHandler->deleteLater();
}

void ModelWidget::setMessageHandler(GUIMessageHandler *pMessageHandler)
{
    // Disconnect old message handler
    if (mpMessageHandler)
    {
        disconnect(this, SIGNAL(checkMessages()), mpMessageHandler, SLOT(collectHopsanCoreMessages()));
    }

    // Assign new message handler
    mpMessageHandler = pMessageHandler;
    mpSimulationThreadHandler->setMessageHandler(pMessageHandler);
    connect(this, SIGNAL(checkMessages()), mpMessageHandler, SLOT(collectHopsanCoreMessages()), Qt::UniqueConnection);
}

void ModelWidget::setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime)
{
    mStartTime = startTime;
    mStopTime = stopTime;

    // First fix Stop time
    if (mStopTime.toDouble() < mStartTime.toDouble())
    {
        mStopTime = mStartTime;
    }

    //Then fix timestep
    if ( timeStep.toDouble() > (mStopTime.toDouble() - mStartTime.toDouble()) )
    {
        mpToplevelSystem->setTimeStep(mStopTime.toDouble() - mStartTime.toDouble());
    }
    else
    {
        getTopLevelSystemContainer()->setTimeStep(timeStep.toDouble());
    }

    // Notify about any changes made (this will go back up to the main window simtime edit
    emit simulationTimeChanged(getStartTime(), getTimeStep(), getStopTime());

    // Tag model as changed
    this->hasChanged();
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
}

QString ModelWidget::getStartTime()
{
    return mStartTime;
}

QString ModelWidget::getTimeStep()
{
    QString num;
    num.setNum(getTopLevelSystemContainer()->getTimeStep(), 'g', 10);
    return num;
}

QString ModelWidget::getStopTime()
{
    return mStopTime;
}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ModelWidget::hasChanged()
{
    if (mIsSaved)
    {
        QString tabName = gpCentralTabWidget->tabText(gpCentralTabWidget->indexOf(this));

        if(!tabName.endsWith("*"))
        {
            tabName.append("*");
        }
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(this), tabName);

        mIsSaved = false;
    }
}


//! @brief Returns a pointer to the system in the tab
SystemContainer *ModelWidget::getTopLevelSystemContainer()
{
    return mpToplevelSystem;
}

//! @brief Returns a pointer to the currently opened container in this model
ContainerObject *ModelWidget::getViewContainerObject()
{
    return mpGraphicsView->getContainerPtr();
}


//! @brief Returns a pointer to the graphics view displayed in the tab
GraphicsView *ModelWidget::getGraphicsView()
{
    return mpGraphicsView;
}


//! @brief Returns a pointer to the quick navigation widget
QuickNavigationWidget *ModelWidget::getQuickNavigationWidget()
{
    return mpQuickNavigationWidget;
}


//! @brief Sets last simulation time (only use this from project tab widget!)
void ModelWidget::setLastSimulationTime(int time)
{
    mLastSimulationTime = time;
}


//! @brief Returns last simulation time for tab
int ModelWidget::getLastSimulationTime()
{
    return mpSimulationThreadHandler->getLastSimulationTime();
}


bool ModelWidget::isEditingEnabled()
{
    return mEditingEnabled;
}


//! @brief Returns whether or not the current project is saved
bool ModelWidget::isSaved()
{
    return mIsSaved;
}


//! @brief Set function to tell the tab whether or not it is saved
void ModelWidget::setSaved(bool value)
{
    if(value)
    {
        QString tabName = gpCentralTabWidget->tabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()));
        if(tabName.endsWith("*"))
        {
            tabName.chop(1);
        }
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
    }
    mIsSaved = value;
}

//! @note this is experimental code to replace madness simulation code in the future
bool ModelWidget::simulate_nonblocking()
{
    // Save backup copy (if needed)
    if (!isSaved() && gpConfig->getAutoBackup())
    {
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    if(!mSimulateMutex.tryLock()) return false;

    //If model contains at least one modelica component, the special code for simulating models with Modelica components must be used
    foreach(const ModelObject *comp, mpToplevelSystem->getModelObjects())
    {
        if(comp->getTypeName() == MODELICATYPENAME)
        {
            simulateModelica();
            unlockSimulateMutex();
            return true;        //! @todo Should use return value from simulateModelica() function instead
        }
    }

    qDebug() << "Calling simulate_nonblocking()";
    //QVector<SystemContainer*> vec;
    //vec.push_back(mpSystem);
    //mSimulationHandler.initSimulateFinalize( vec, mStartTime.toDouble(), mStopTime.toDouble(), mpSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->initSimulateFinalize(mpToplevelSystem);

    return true;
    //! @todo fix return code
}

bool ModelWidget::simulate_blocking()
{
    // Save backup copy
    if (!isSaved() && gpConfig->getAutoBackup())
    {
        //! @todo this should be a help function, also we may not want to call it every time when we run optimization (not sure if that is done now but probably)
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    if(!mSimulateMutex.tryLock()) return false;

    //If model contains at least one modelica component, the special code for simulating models with Modelica components must be used
    foreach(const ModelObject *comp, mpToplevelSystem->getModelObjects())
    {
        if(comp->getTypeName() == MODELICATYPENAME)
        {
            simulateModelica();
            return true;        //! @todo Should use return value from simulateModelica() function instead
        }
    }

    mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
    QVector<SystemContainer*> vec;
    vec.push_back(mpToplevelSystem);
    mpSimulationThreadHandler->initSimulateFinalize_blocking(vec);

    return true;
    //! @todo fix return code
}

void ModelWidget::startCoSimulation()
{
    CoreSimulationHandler *pHandler = new CoreSimulationHandler();
    pHandler->runCoSimulation(mpToplevelSystem->getCoreSystemAccessPtr());
    delete(pHandler);

    emit checkMessages();
}


//! Slot that saves current project to old file name if it exists.
//! @see saveModel(int index)
void ModelWidget::save()
{
    saveModel(ExistingFile);
}


//! Slot that saves current project to a new model file.
//! @see saveModel(int index)
void ModelWidget::saveAs()
{
    saveModel(NewFile);
}

void ModelWidget::exportModelParameters()
{
    saveModel(NewFile, ParametersOnly);


//    //saveModel(NEWFILE);

//    QDir fileDialogSaveDir;
//    QString modelFilePath;
//    modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
//                                                 gConfig.getLoadModelDir(),
//                                                 tr("Hopsan Parameter File (*.hmf)"));

//    if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
//    {
//        return;
//    }

//    mpSystem->setModelFile(modelFilePath);
//    QFileInfo fileInfo = QFileInfo(modelFilePath);
//    gConfig.setLoadModelDir(fileInfo.absolutePath());

//    QFile file(mpSystem->getModelFileInfo().filePath());   //Create a QFile object
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        return;
//    }

//    //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
//    mpSystem->setName(mpSystem->getModelFileInfo().baseName());

//    //Update the basepath for relative appearance data info
//    mpSystem->setAppearanceDataBasePath(mpSystem->getModelFileInfo().absolutePath());

//    //Save xml document
//    QDomDocument domDocument;
//    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, mpSystem->getCoreSystemAccessPtr()->getHopsanCoreVersion());

//    // Save the required external lib names
//    QVector<QString> extLibNames;
//    CoreParameterData coreParaAccess;
//    //coreParaAccess.getLoadedLibNames(extLibNames);


////    //! @todo need HMF defines for hardcoded strings
////    QDomElement reqDom = appendDomElement(hmfRoot, "requirements");
////    for (int i=0; i<coreParaAccess.size(); ++i)
////    {
////        appendDomTextNode(reqDom, "Parameters", extLibNames[i]);
////    }

////    QDomElement XMLparameters = appendDomElement(hmfRoot, "parameters");
////    for(int i = 0; i < gpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.size(); ++i)
////    {
////        QDomElement XMLparameter = appendDomElement(XMLparameters, "parameter");
////        appendDomTextNode(XMLparameter, "componentname", gpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mComponentName);
////        appendDomTextNode(XMLparameter, "parametername", gpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mParameterName);
////        appendDomValueNode2(XMLparameter, "minmax", gpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mMin, gpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mMax);
////    }

////    //Save the model component hierarcy
////    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
//    mpSystem->saveToDomElement(hmfRoot);

//    //Save to file
//    const int IndentSize = 4;
//    QFile xmlhmf(mpSystem->getModelFileInfo().filePath());
//    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        gpMessageHandler->addErrorMessage("Could not save to file: " + mpSystem->getModelFileInfo().filePath());
//        return;
//    }
//    QTextStream out(&xmlhmf);
//    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//    domDocument.save(out, IndentSize);

//    //Close the file
//    xmlhmf.close();

//    //Set the tab name to the model name, efectively removing *, also mark the tab as saved
//    QString tabName = mpSystem->getModelFileInfo().baseName();
//    mpParentCentralTabWidget->setTabText(mpParentCentralTabWidget->currentIndex(), tabName);
//    gConfig.addRecentModel(mpSystem->getModelFileInfo().filePath());
//    gpMainWindow->updateRecentList();
//    this->setSaved(true);

//    gpMessageHandler->addInfoMessage("Saved model: " + tabName);

}


void ModelWidget::setExternalSystem(bool value)
{
    setEditingEnabled(!value);
    mpExternalSystemWidget->setVisible(value);
}


void ModelWidget::setEditingEnabled(bool value)
{
    mEditingEnabled = value;

    if(!mEditingEnabled)
    {
        QStringList objects = mpGraphicsView->getContainerPtr()->getModelObjectNames();
        for(int i=0; i<objects.size(); ++i)
        {
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsMovable, false);
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsSelectable, false);

            QGraphicsColorizeEffect *grayEffect = new QGraphicsColorizeEffect();
            grayEffect->setColor(QColor("gray"));
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setGraphicsEffect(grayEffect);

            QList<Connector*> connectors = mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->getConnectorPtrs();
            for(int j=0; j<connectors.size(); ++j)
            {
                QGraphicsColorizeEffect *grayEffect2 = new QGraphicsColorizeEffect();
                grayEffect2->setColor(QColor("gray"));
                connectors.at(j)->setGraphicsEffect(grayEffect2);
            }
        }

        QList<Widget*> widgetList = mpGraphicsView->getContainerPtr()->getWidgets();
        for(int w=0; w<widgetList.size(); ++w)
        {
            QGraphicsColorizeEffect *grayEffect = new QGraphicsColorizeEffect();
            grayEffect->setColor(QColor("gray"));
            widgetList.at(w)->setGraphicsEffect(grayEffect);
        }
    }
    else
    {
        QStringList objects = mpGraphicsView->getContainerPtr()->getModelObjectNames();
        for(int i=0; i<objects.size(); ++i)
        {
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsMovable, true);
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsSelectable, true);

            if(mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->graphicsEffect())
                mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->graphicsEffect()->setEnabled(false);

            QList<Connector*> connectors = mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->getConnectorPtrs();
            for(int j=0; j<connectors.size(); ++j)
            {
                if(connectors.at(j)->graphicsEffect())
                    connectors.at(j)->graphicsEffect()->setEnabled(false);
            }
        }

        QList<Widget*> widgetList = mpGraphicsView->getContainerPtr()->getWidgets();
        for(int w=0; w<widgetList.size(); ++w)
        {
            if(widgetList.at(w)->graphicsEffect())
                widgetList.at(w)->graphicsEffect()->setEnabled(false);
        }
    }
}


//! @brief Slot that tells the current system to collect plot data from core
void ModelWidget::collectPlotData()
{
    //If we collect plot data, we can plot and calculate losses, so enable these buttons
    //gpMainWindow->mpPlotAction->setEnabled(true);
    //gpMainWindow->mpShowLossesAction->setEnabled(true); //! @todo Can this be done without including main window?
   // gpMainWindow->mpAnimateAction->setEnabled(true);

    // Tell container to do the job
    mpToplevelSystem->collectPlotData();
}


void ModelWidget::openAnimation()
{
    //Generate animation dialog
    if(mpAnimationWidget !=0)
    {
        delete mpAnimationWidget;
        mpAnimationWidget = 0;
    }
    if(!getTopLevelSystemContainer()->getModelObjectNames().isEmpty())   //Animation widget cannot be created with no objects
    {
        mpAnimationWidget = new AnimationWidget(this);
        gpCentralGridLayout->addWidget(mpAnimationWidget, 0, 0, 4, 4);  //! @todo Can this be done without including main window?
        mpAnimationWidget->show();
        gpCentralTabWidget->hide();
    }
}

void ModelWidget::lockTab(bool locked)
{
    setDisabled(locked);
}

void ModelWidget::simulateModelica()
{
    QString modelName = mpToplevelSystem->getName();
    if(modelName.isEmpty())
    {
        modelName = "Unsaved";
    }
    QString modelicaCode = "model "+modelName+"\n";

    foreach(ModelObject* object, mpToplevelSystem->getModelObjects())
    {
        if(object->getTypeName() == MODELICATYPENAME)
        {
            QString model = object->getParameterValue("model");
            modelicaCode.append("    "+model+" "+object->getName()+"(");
            //Add type name for modelica component
        }
        else
        {
            modelicaCode.append("    TLM_"+object->getTypeName()+" "+object->getName()+"(");
        }
        foreach(const QString &parName, object->getParameterNames())
        {
            modelicaCode.append(parName+"="+object->getParameterValue(parName)+",");
        }
        modelicaCode.chop(1);
        modelicaCode.append(");\n");
    }

    modelicaCode.append("equation\n");

    QList<Connector*> connectorPtrs;
    foreach(ModelObject* object, mpToplevelSystem->getModelObjects())
    {
        foreach(Connector* connector, object->getConnectorPtrs())
        {
            if(!connectorPtrs.contains(connector))
            {
                connectorPtrs.append(connector);
            }
        }
    }

    foreach(const Connector* connector, connectorPtrs)
    {
        modelicaCode.append("    connect("+connector->getStartComponentName()+"."+connector->getStartPortName()+
                      ","+connector->getEndComponentName()+"."+connector->getEndPortName()+");\n");
    }

    modelicaCode.append("end "+modelName);

    qDebug() << modelicaCode;






    ModelicaModel mainModel(modelicaCode);

    QList<ModelicaVariable> variables = mainModel.getVariables();

    //Add each component to one group each (except TLM components)
    QList<QList<QPair<QString, QString> > > groups;
    foreach(const ModelicaVariable &variable, variables)
    {
        QString type = variable.getType();
        QString name = variable.getName();
        if(type.startsWith("TLM_"))
        {
            variables.removeAll(variable);
        }
        else
        {
            groups.append(QList<QPair<QString, QString> >());
            groups.last().append(QPair<QString, QString>(name, type));
        }
    }

    //Merge all groups of connected components
    QStringList equations;
    mainModel.getEquations(equations);
    foreach(const QString &equation, equations)
    {
        if(equation.startsWith("connect("))
        {
            QString comp1 = equation.section("connect(",1,1).section(".",0,0);
            QString comp2 = equation.section(",",1,1).section(".",0,0).trimmed();
            bool found1 = false;
            bool isTlm1 = false;
            bool found2 = false;
            bool isTlm2 = false;
            foreach(const ModelicaVariable &variable, variables)
            {
                if(variable.getName() == comp1)
                {
                    found1 = true;
                    isTlm1 = variable.getType().startsWith("TLM_");
                }
                else if(variable.getName() == comp2)
                {
                    found2 = true;
                    isTlm2 = variable.getType().startsWith("TLM_");
                }
            }
            if(!found1 || !found2 || isTlm1 || isTlm2)
            {
                continue;       //Ignore connections with TLM components
            }
            int id1, id2;
            for(int i=0; i<groups.size(); ++i)
            {
                for(int j=0; j<groups[i].size(); ++j)
                {
                    if(groups[i][j].first == comp1)
                    {
                        id1 = i;
                    }
                    else if(groups[i][j].first == comp2)
                    {
                        id2 = i;
                    }
                }
            }

            if(id1 != id2)
            {
                groups[min(id1,id2)].append(groups[max(id1, id2)]);
                groups.removeAt(max(id1,id2));
            }
        }
    }

    gpMessageHandler->addInfoMessage("Found "+QString::number(groups.size())+" independent groups of Modelica components.");

    QList<QStringList> groupNames;
    for(int i=0; i<groups.size(); ++i)
    {
        groupNames.append(QStringList());
        for(int j=0; j<groups.at(i).size(); ++j)
        {
            groupNames[i].append(groups.at(i).at(j).first);
        }
    }

    QMap<QString, QList<QStringList> > modelicaModels;
    QStringList loadedLibs;

    gpMessageHandler->addInfoMessage("Compiling Modelica code...");

    for(int i=0; i<groupNames.size(); ++i)
    {
        qDebug() << "Group " << i << ":";
        QStringList flatInitAlgorithms;
        QStringList flatPreAlgorithms;
        QStringList flatEquations;
        QMap<QString,QString> localVars;
        mainModel.toFlatEquations(flatInitAlgorithms, flatPreAlgorithms, flatEquations, localVars, "", groupNames[i]);

        //Ugly hack, because HopsanGenerator assumes port variables to use point notation (e.g. P1.q) while points are replaced with "__" above
        foreach(const QString &var, localVars.keys())
        {
            if(localVars.find(var).value() == "NodeSignalIn")
            {
                for(int i=0; i<flatInitAlgorithms.size(); ++i)
                {
                    flatInitAlgorithms[i].replace(var+"__y", var+".y");
                }
                for(int i=0; i<flatEquations.size(); ++i)
                {
                    flatEquations[i].replace(var+"__y", var+".y");
                }
            }
            else if(localVars.find(var).value() == "NodeHydraulic")
            {
                for(int i=0; i<flatInitAlgorithms.size(); ++i)
                {
                    flatInitAlgorithms[i].replace(var+"__p", var+".p");
                    flatInitAlgorithms[i].replace(var+"__q", var+".q");
                    flatInitAlgorithms[i].replace(var+"__c", var+".c");
                    flatInitAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatPreAlgorithms.size(); ++i)
                {
                    flatPreAlgorithms[i].replace(var+"__p", var+".p");
                    flatPreAlgorithms[i].replace(var+"__q", var+".q");
                    flatPreAlgorithms[i].replace(var+"__c", var+".c");
                    flatPreAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatEquations.size(); ++i)
                {
                    flatEquations[i].replace(var+"__p", var+".p");
                    flatEquations[i].replace(var+"__q", var+".q");
                    flatEquations[i].replace(var+"__c", var+".c");
                    flatEquations[i].replace(var+"__Zc", var+".Zc");
                }
            }
            else if(localVars.find(var).value() == "NodeMechanic")
            {
                for(int i=0; i<flatInitAlgorithms.size(); ++i)
                {
                    flatInitAlgorithms[i].replace(var+"__f", var+".f");
                    flatInitAlgorithms[i].replace(var+"__x", var+".x");
                    flatInitAlgorithms[i].replace(var+"__v", var+".v");
                    flatInitAlgorithms[i].replace(var+"__c", var+".c");
                    flatInitAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatPreAlgorithms.size(); ++i)
                {
                    flatPreAlgorithms[i].replace(var+"__f", var+".f");
                    flatPreAlgorithms[i].replace(var+"__x", var+".x");
                    flatPreAlgorithms[i].replace(var+"__v", var+".v");
                    flatPreAlgorithms[i].replace(var+"__c", var+".c");
                    flatPreAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatEquations.size(); ++i)
                {
                    flatEquations[i].replace(var+"__f", var+".f");
                    flatEquations[i].replace(var+"__x", var+".x");
                    flatEquations[i].replace(var+"__v", var+".v");
                    flatEquations[i].replace(var+"__c", var+".c");
                    flatEquations[i].replace(var+"__Zc", var+".Zc");
                }
            }
        }

        //Generate file name for temporary modelica component
        QString name = "ModelicaTemp";
        int uid = int(rand() / (double)RAND_MAX * 1000000); //! @todo Verify that component with same uid does not exist!
        name.append(QString::number(uid));

        QString flatModel;
        flatModel.append("model "+name+"\n");
        flatModel.append("    annotation(hopsanCqsType = \"Q\");\n\n");
        foreach(const QString &var, localVars.keys())
        {
            flatModel.append("    "+localVars.find(var).value()+" "+var+";\n");
        }
        flatModel.append("initial algorithm\n");
        foreach(const QString &algorithm, flatInitAlgorithms)
        {
            flatModel.append("    "+algorithm+";\n");
        }
        flatModel.append("algorithm\n");
        foreach(const QString &algorithm, flatPreAlgorithms)
        {
            flatModel.append("    "+algorithm+";\n");
        }
        flatModel.append("equation\n");
        foreach(const QString &equation, flatEquations)
        {
            flatModel.append("    "+equation+";\n");
        }
        flatModel.append("end "+name+";\n");

        //Create folder for temporary modelica component
        QString path = gpDesktopHandler->getDataPath()+"ModelicaTempComponents/"+name;
        QDir().mkpath(path);

        QFile moFile(path+"/"+name+".mo");
        moFile.open(QFile::WriteOnly | QFile::Text);
        moFile.write(flatModel.toUtf8());
        moFile.close();

        CoreGeneratorAccess coreGen;
        coreGen.generateFromModelica(QFileInfo(moFile).absoluteFilePath(), true, 0, true);
        qDebug() << flatModel;

        gpLibraryHandler->loadLibrary(path+"/"+name+"._lib.xml");
        if(!gpLibraryHandler->getLoadedTypeNames().contains(name))
        {
            gpMessageHandler->addErrorMessage("Could not load temporary Modelica library. Aborting.");
        }
        loadedLibs.append(name);

        QList<QStringList> connections;
        foreach(const QString &comp, groupNames[i])
        {
            foreach(const Connector *pConnector, mpToplevelSystem->getModelObject(comp)->getConnectorPtrs())
            {
                QString startComp = pConnector->getStartComponentName();
                QString endComp = pConnector->getEndComponentName();
                QString startPort = pConnector->getStartPortName();
                QString endPort = pConnector->getEndPortName();
                QString startType = mpToplevelSystem->getModelObject(startComp)->getTypeName();
                QString endType = mpToplevelSystem->getModelObject(endComp)->getTypeName();

                if(startType != MODELICATYPENAME)
                {
                    connections.append(QStringList());
                    connections.last().append(startComp);
                    connections.last().append(startPort);
                    connections.last().append(name);
                    connections.last().append(startComp+"__"+startPort);
                }
                else if(endType != MODELICATYPENAME)
                {
                    connections.append(QStringList());
                    connections.last().append(name);
                    connections.last().append(endComp+"__"+endPort);
                    connections.last().append(endComp);
                    connections.last().append(endPort);
                }
            }
        }

        modelicaModels.insert(name, connections);
    }

    gpMessageHandler->addInfoMessage("Creating temporary simulation model...");

    //Create new hidden temporary model
    ModelWidget *pTempModel = mpParentModelHandler->addNewModel("TempModel", true);
    SystemContainer *pTempSys = pTempModel->getTopLevelSystemContainer();

    //Add all non-Modelica components
    for(int i=0; i<mpToplevelSystem->getModelObjectNames().size(); ++i)
    {
        QString name = mpToplevelSystem->getModelObjectNames().at(i);
        QString type = mpToplevelSystem->getModelObjects().at(i)->getTypeName();
        if(type != MODELICATYPENAME)
        {
            ModelObject *pModelObject = pTempSys->addModelObject(type, QPointF(0,0));
            if(!pModelObject)
            {
                gpMessageHandler->addErrorMessage("Failed to add model object of type: "+type+" to temporary model. Aborting.");
                return;
            }
            pTempSys->renameModelObject(pModelObject->getName(), name);

            //Set parameters in added component
            foreach(const QString par, pModelObject->getParameterNames())
            {
                pModelObject->setParameterValue(par, mpToplevelSystem->getModelObject(name)->getParameterValue(par));
            }
        }
    }

    //Add all connections between non-Modelica components
    foreach(const Connector *pConnector, mpToplevelSystem->getSubConnectorPtrs())
    {
        QString startComp = pConnector->getStartComponentName();
        QString endComp = pConnector->getEndComponentName();
        QString startPort = pConnector->getStartPortName();
        QString endPort = pConnector->getEndPortName();

        if(pTempSys->hasModelObject(startComp) && pTempSys->getModelObject(startComp)->getPort(startPort) &&
           pTempSys->hasModelObject(endComp) && pTempSys->getModelObject(endComp)->getPort(endPort))
        {
            pTempSys->createConnector(pTempSys->getModelObject(startComp)->getPort(startPort), pTempSys->getModelObject(endComp)->getPort(endPort));
        }
    }

    //Add and connect the Modelica components
    QMapIterator<QString, QList<QStringList> > it(modelicaModels);
    while(it.hasNext())
    {
        it.next();
        if(!pTempSys->addModelObject(it.key(), QPointF(0,0)))
        {
            gpMessageHandler->addErrorMessage("Failed to add temporary Modelica component. Aborting.");
            return;
        }

        //Make sure all output variable ports are enabled
        //for(int p=0; p<pTempSys->getModelObject(it.key())->getPortListPtrs().size(); ++p)
        QVector<CoreVariameterDescription> variameters;
        pTempSys->getModelObject(it.key())->getVariameterDescriptions(variameters);
        foreach(CoreVariameterDescription variameter, variameters)
        {
            if(variameter.mVariameterType == 1) //1 = output variable
            {
                pTempSys->getModelObject(it.key())->createRefreshExternalPort(variameter.mPortName);
                pTempSys->getModelObject(it.key())->getPort(variameter.mPortName)->setEnable(true);
            }
        }

        foreach(const QStringList &connection, it.value())
        {
            if(connection.size() == 4)      //Should always be 4
            {
                if(!pTempSys->hasModelObject(connection[0]) || !pTempSys->getModelObject(connection[0])->getPort(connection[1]) ||
                   !pTempSys->hasModelObject(connection[2]) || !pTempSys->getModelObject(connection[2])->getPort(connection[3]))
                {
                    gpMessageHandler->addErrorMessage("Failed to create connector, one object or port does not exist. Aborting.");
                    return;
                }
                pTempSys->createConnector(pTempSys->getModelObject(connection[0])->getPort(connection[1]), pTempSys->getModelObject(connection[2])->getPort(connection[3]));
            }
        }


        //Set parameter values for Modelica components
        pTempSys->getModelObject(it.key())->setParameterValue("solverType", "4", true);
        pTempSys->getModelObject(it.key())->setParameterValue("nIter","2", true);
        foreach(const QString parameter, pTempSys->getModelObject(it.key())->getParameterNames())
        {
            if(parameter.contains("__"))
            {
                //! @todo Check that original value exists first!

               QString orgComp = parameter.section("__",0,0);
                QString orgPar = parameter.section("__",1,1);
                if(mpToplevelSystem->hasModelObject(orgComp) &&
                   mpToplevelSystem->getModelObject(orgComp)->hasParameter(orgPar))
                {
                    QString orgValue = mpToplevelSystem->getModelObject(orgComp)->getParameterValue(orgPar);
                    pTempSys->getModelObject(it.key())->setParameterValue(parameter, orgValue);
                }
            }
        }
    }

    pTempModel->setTopLevelSimulationTime(mStartTime, QString::number(this->getTopLevelSystemContainer()->getTimeStep()), mStopTime);
    pTempModel->getTopLevelSystemContainer()->setNumberOfLogSamples(mpToplevelSystem->getNumberOfLogSamples());
    pTempModel->simulate_blocking();

    //Move all data from temporary model to actual model
    mpToplevelSystem->getLogDataHandler()->takeOwnershipOfData(pTempSys->getLogDataHandler());

    //Rename variables so that their component name and port names are converted for actual model
    QMapIterator<QString, QList<QStringList> > it2(modelicaModels);
    QStringList varsToRemove;
    while(it2.hasNext())
    {
        it2.next();
        for(int i=0; i<mpToplevelSystem->getLogDataHandler()->getAllVariablesAtGeneration(-1).size(); ++i)
        {
            HopsanVariable pVar = mpToplevelSystem->getLogDataHandler()->getAllVariablesAtGeneration(-1).at(i);
            if(pVar.mpContainer->getName().startsWith(it2.key()+"#"))
            {
                QString newName = pVar.mpContainer->getName();
                newName.remove(it2.key()+"#");
                newName.remove("#Value");
                if(!newName.startsWith("_"))    //Ignore all internal variables (will be removed)
                {
                    SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription());
                    pVarDesc->mComponentName = newName.section("__",0,0);
                    pVarDesc->mPortName = newName.section("__",1,1);
                    pVarDesc->mDataName = newName.section("__",2,2);
                    if(pVarDesc->mDataName.isEmpty())
                        pVarDesc->mDataName = "Value";
                    SharedVectorVariableT pNewVar = createFreeVariable(pVar.mpVariable->getVariableType(), pVarDesc);
                    pNewVar->assignFrom(pVar.mpVariable);
                    mpToplevelSystem->getLogDataHandler()->insertNewHopsanVariable(pNewVar);
                }
                varsToRemove.append(pVar.mpContainer->getName());
            }
        }
    }

    gpPlotWidget->updateList();

    foreach(const QString &var, varsToRemove)
    {
        mpToplevelSystem->getLogDataHandler()->deleteVariableContainer(var);
    }


    //Cleanup
    pTempModel->close();
    delete(pTempModel);

    mpParentModelHandler->setCurrentModel(this);

//    foreach(const QString &lib, loadedLibs)
//    {
//        gpLibraryHandler->unloadLibrary(lib);
//        removeDir(gpDesktopHandler->getDataPath()+"/ModelicaTempComponents/"+lib);
//    }

    //! @todo We should not recompile every time, only if model structure has changed
}


void ModelWidget::closeAnimation()
{
    gpCentralGridLayout->removeWidget(mpAnimationWidget); //! @todo Can this be done without including main window?
    delete mpAnimationWidget;
    mpAnimationWidget = 0;
    gpCentralTabWidget->show();
}


void ModelWidget::unlockSimulateMutex()
{
    mSimulateMutex.unlock();
}


//! @brief Opens current container in new tab
//! Used for opening external subsystems for editing. If current container is not external, it will
//! iterate through parent containers until it finds one that is.
void ModelWidget::openCurrentContainerInNewTab()
{
    ContainerObject *pContainer = mpGraphicsView->getContainerPtr();

    while(true)
    {
        if(pContainer == mpToplevelSystem)
        {
            break;
        }
        else if(!pContainer->isExternal())
        {
            pContainer = pContainer->getParentContainerObject();
        }
        else
        {
            //mpParentModelHandler->loadModel(pContainer->getModelFileInfo().filePath());
            pContainer->setModelFile("");
            setEditingEnabled(true);
            mpExternalSystemWidget->setVisible(false);
            break;
        }
    }


}


//! Saves the model and the viewport settings in the tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveModel()
//! @see loadModel()
void ModelWidget::saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents)
{
    // Backup old save file before saving (if old file exists)
    if(saveAsFlag == ExistingFile && gpConfig->getAutoBackup())
    {
        QFile backupFile(mpToplevelSystem->getModelFileInfo().filePath());
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        QString backupFilePath = gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_save_backup.hmf";
        if(QFile::exists(backupFilePath))
        {
            QFile::remove(backupFilePath);
        }
        backupFile.copy(backupFilePath);
    }

    //Get file name in case this is a save as operation
    QString modelFilePathToSave = mpToplevelSystem->getModelFileInfo().filePath();
    if((modelFilePathToSave.isEmpty()) || (saveAsFlag == NewFile))
    {
        QString filter;
        if(contents==FullModel)
        {
            filter = tr("Hopsan Model Files (*.hmf)");
        }
        else if(contents==ParametersOnly)
        {
            filter = tr("Hopsan Parameter Files (*.hpf)");
        }

        QString modelPath = getTopLevelSystemContainer()->getModelFileInfo().absolutePath();
        if(modelPath.isEmpty())
        {
            modelPath = gpConfig->getLoadModelDir();
        }

        modelFilePathToSave = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                     modelPath,
                                                     filter);

        if(modelFilePathToSave.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        if(contents==FullModel)
        {
            mpToplevelSystem->setModelFile(modelFilePathToSave);
        }
        QFileInfo fileInfo = QFileInfo(modelFilePathToSave);
        gpConfig->setLoadModelDir(fileInfo.absolutePath());
    }

    bool success = saveTo(modelFilePathToSave, contents);


    if(success)
    {
            //Set the tab name to the model name, efectively removing *, also mark the tab as saved
        //! @todo this should not happen when saving parameters, This needs to be rewritten in a smarter way so that we do not need a special argument and lots of ifs to do special saving of parameters, actually parameters should be saved using the CLI method (and that code should be in a shared utility library)
        QString tabName = mpToplevelSystem->getModelFileInfo().baseName();
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
        if(contents == FullModel)
        {
            gpConfig->addRecentModel(mpToplevelSystem->getModelFileInfo().filePath());
            this->setSaved(true);
        }

        mpMessageHandler->addInfoMessage("Saved model: " + modelFilePathToSave);

        mpToplevelSystem->getCoreSystemAccessPtr()->addSearchPath(mpToplevelSystem->getModelFileInfo().absolutePath());

        emit modelSaved(this);
    }
}


bool ModelWidget::saveTo(QString path, SaveContentsEnumT contents)
{
    QFile file(path);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        mpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
        return false;
    }

    if(contents==FullModel)
    {
            //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
        mpToplevelSystem->setName(mpToplevelSystem->getModelFileInfo().baseName());

            //Update the basepath for relative appearance data info
        mpToplevelSystem->setAppearanceDataBasePath(mpToplevelSystem->getModelFileInfo().absolutePath());
    }

        //Save xml document
    QDomDocument domDocument;
    QDomElement rootElement;
    if(contents==FullModel)
    {
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
    }
    else
    {
        rootElement = domDocument.createElement(HPF_ROOTTAG);
        domDocument.appendChild(rootElement);
    }

    if(contents==FullModel)
    {
            // Save the required external lib names
        QVector<QString> extLibNames;
        CoreLibraryAccess coreLibAccess;
        coreLibAccess.getLoadedLibNames(extLibNames);


        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (int i=0; i<extLibNames.size(); ++i)
        {
            appendDomTextNode(reqDom, "componentlibrary", extLibNames[i]);
        }
    }
        //Save the model component hierarcy
    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
    mpToplevelSystem->saveToDomElement(rootElement, contents);

        //Save to file
    QFile xmlFile(path);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        mpMessageHandler->addErrorMessage("Could not save to file: " + path);
        return false;
    }
    QTextStream out(&xmlFile);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, XMLINDENTATION);

    //Close the file
    xmlFile.close();

    return true;
}
