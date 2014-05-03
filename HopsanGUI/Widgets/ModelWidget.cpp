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


//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "HcomWidget.h"
#include "InitializationThread.h"
#include "ModelHandler.h"
#include "ProgressBarThread.h"
#include "ProjectTabWidget.h"
#include "SimulationThreadHandler.h"
#include "Utilities/XMLUtilities.h"
#include "version_gui.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/DebuggerWidget.h"
#include "MessageHandler.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/QuickNavigationWidget.h"


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
    if (!isSaved())
    {
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    if(!mSimulateMutex.tryLock()) return false;

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
    if (!isSaved())
    {
        //! @todo this should be a help function, also we may not want to call it every time when we run optimization (not sure if that is done now but probably)
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    if(!mSimulateMutex.tryLock()) return false;

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

void ModelWidget::generateModelicaCode()
{
    QString output = "model "+mpToplevelSystem->getName()+"\n";

    foreach(ModelObject* object, mpToplevelSystem->getModelObjects())
    {
        if(object->getTypeName() == "ModelicaComponent")
        {
            //Add type name for modelica component
        }
        else
        {
            output.append("    TLM_"+object->getTypeName()+" "+object->getName()+"(");

            foreach(const QString &parName, object->getParameterNames())
            {
                output.append(parName+"="+object->getParameterValue(parName)+",");
            }
            output.chop(1);
            output.append(");\n");
        }
    }

    output.append("equation\n");

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
        output.append("    connect("+connector->getStartComponentName()+"."+connector->getStartPortName()+
                      ","+connector->getEndComponentName()+"."+connector->getEndPortName()+");\n");
    }

    output.append("end "+mpToplevelSystem->getName());

    qDebug() << output;




    QMap<QString, QString> modelicaLib;
    modelicaLib.insert("NodeHydraulic", "connector NodeHydraulic \"Hydraulic Connector\"\n"
                                        "Real         p \"Pressure\";\n"
                                        "flow Real    q \"Volume flow\"\n"
                                        "end Pin;");

    modelicaLib.insert("FlowSource", "model FlowSource\n"
                                     "    parameter Real q_ref;\n"
                                     "    NodeHydraulic P1;\n"
                                     "equation\n"
                                     "    P1.q = q_ref;\n"
                                     "end FlowSource;");

    modelicaLib.insert("Tank", "model Tank\n"
                               "    NodeHydraulic P1;\n"
                               "equation\n"
                               "    P1.p = 100000;\n"
                               " end Tank;");

    modelicaLib.insert("LaminarOrifice", "model LaminarOrifice\n"
                                         "    parameter Real Kc;\n"
                                         "    NodeHydraulic P1, P2;\n"
                                         "equation\n"
                                         "    P2.q = Kc*(P1.p-P2.p);\n"
                                         "    0 = P1.q + P2.q;\n"
                                         "end LaminarOrifice;");

    modelicaLib.insert("Volume", "model Volume\n"
                                 "    parameter Real V;\n"
                                 "    parameter Real betae;\n"
                                 "    NodeHydraulic P1, P2;\n"
                                 "    Real P;\n"
                                 "equation\n"
                                 "    P = (P1.p+P2.p)/2;\n"
                                 "    V/betae*der(P) = P1.q+P2.q;\n"
                                 "end Volume;");


    QString modelEquations = "model TestModel\n"
                            "    FlowSource qSource;\n"
                            "    Volume volume;\n"
                            "    LaminarOrifice orifice;\n"
                            "    Tank tank;\n"
                            "equation\n"
                            "    connect(qSource1.P1, volume.P1);\n"
                            //"    connect(volume.P2, orifice.P1);\n"
                            "    connect(orifice.P2, tank.P1);\n"
                            "end TestModel;";



    QList<QList<QPair<QString, QString> > > groups;
    QStringList splitModelEquations = modelEquations.split("\n");
    int i=1;
    while(splitModelEquations[i] != "equation")
    {
        QString equation = splitModelEquations[i].remove(";").trimmed();
        groups.append(QList<QPair<QString, QString> >());
        groups.last().append(QPair<QString, QString>());
        groups.last().last().first = equation.split(" ").first();
        groups.last().last().second = equation.split(" ").last();
        ++i;
    }
    ++i;
    while(!splitModelEquations[i].startsWith("end "))
    {
        QString equation = splitModelEquations[i].trimmed();
        if(equation.startsWith("connect("))
        {
            QString comp1 = equation.section("connect(",1,1).section(".",0,0);
            QString comp2 = equation.section(",",1,1).section(".",0,0).trimmed();
            int id1, id2;
            for(int i=0; i<groups.size(); ++i)
            {
                for(int j=0; j<groups[i].size(); ++j)
                {
                    if(groups[i][j].second == comp1)
                    {
                        id1 = i;
                    }
                    else if(groups[i][j].second == comp2)
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
        ++i;
    }

    int apa=3;
    int ko=apa+2;

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
    if(saveAsFlag == ExistingFile)
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


        modelFilePathToSave = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                     gpConfig->getLoadModelDir(),
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
