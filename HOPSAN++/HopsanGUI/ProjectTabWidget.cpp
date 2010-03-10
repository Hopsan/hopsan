//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Project Tabs
//!
//$Id$


#include "HopsanCore.h"
#include "ProjectTabWidget.h"
#include "GUIComponent.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "LibraryWidget.h"
#include "mainwindow.h"
#include "SimulationSetupWidget.h"
#include "MessageWidget.h"
#include "SimulationThread.h"
#include "InitializationThread.h"
#include <QtGui>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>


//! @class GraphicsView
//! @brief The GraphicsView class is a class which display the content of a scene of components.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(HopsanEssentials *hopsan, ComponentSystem *model, ProjectTab *parent)
        : QGraphicsView(parent)
{
    mpParentProjectTab = parent;

    this->mpHopsan = hopsan;
    this->mpModel = model;

    this->setDragMode(RubberBandDrag);

    this->setInteractive(true);
    this->setEnabled(true);
    this->setAcceptDrops(true);
    this->mIsCreatingConnector = false;
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //this->setTransformationAnchor(QGraphicsView::NoAnchor);

    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent->parent()->parent()->parent())); //Ugly!!!
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));
}


//! Destructor.
GraphicsView::~GraphicsView()
{
    //delete guiComponent; //Skumt att ta bort en guiComponent?
}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}


//! Defines what happens when drop an object in a GraphicsView.
//! @param event contains information of the drop operation.
void GraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        QByteArray *data = new QByteArray;
        *data = event->mimeData()->data("application/x-text");

        QDataStream stream(data,QIODevice::ReadOnly);

        QStringList parameterData;
        stream >> parameterData;

        QString componentTypeName = parameterData.at(0);
        QString iconDir = parameterData.at(1);

        event->accept();

  //      QCursor cursor;
//        QPoint position = this->mapFromScene(cursor.pos());

        QPoint position = event->pos();

        qDebug() << "GraphicsView: " << "x=" << position.x() << "  " << "y=" << position.y();

//        GUIComponent *guiComponent = new GUIComponent(mpHopsan,iconDir,componentTypeName,mapToScene(position).toPoint(),this);
      //  GUIComponent *guiComponent = new GUIComponent(mpHopsan,parameterData,mapToScene(position).toPoint(),this);

        //this->addComponent(parameterData, this->mapToScene(position).toPoint());
        this->addComponent(parameterData.at(0), this->mapToScene(position).toPoint());

        delete data;
    }
}


//void GraphicsView::addComponent(QStringList parameterData, QPoint position)
//{
//    GUIComponent *guiComponent = new GUIComponent(mpHopsan,parameterData,position,this);
//
//    //Core interaction
//    qobject_cast<ProjectTab *>(this->parent())->mpModel->addComponent(guiComponent->mpCoreComponent);
//    guiComponent->refreshName();
//    emit checkMessages();
//    //
//
//    //guiComponent->setPos(this->mapToScene(position));
//    qDebug() << "GraphicsView: " << guiComponent->parent();
//
//    this->scene()->addItem(guiComponent);
//}
GUIConnector *GraphicsView::getTempConnector()
{
    return this->mpTempConnector;
}


//! Adds a new component to the GraphicsView.
//! @param parameterType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
void GraphicsView::addComponent(QString parameterType, QPoint position, QString name)
{
    qDebug() << "Request to add component at (" << position.x() << " " << position.y() << ")";
    MainWindow *pMainWindow = qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent());
    LibraryWidget *pLibrary = pMainWindow->mpLibrary;
    QStringList parameterData = pLibrary->getParameterData(parameterType);
    GUIComponent *pGuiComponent = new GUIComponent(mpHopsan,parameterData,position,this->mpParentProjectTab->mpGraphicsScene);
    if (!name.isEmpty())
    {
        pGuiComponent->setName(name);
    }

    //Core interaction
    this->mpParentProjectTab->mpComponentSystem->addComponent(pGuiComponent->mpCoreComponent);
    //    qobject_cast<ProjectTab *>(this->parent())->mpModel->addComponent(guiComponent->mpCoreComponent);
    pGuiComponent->refreshName();
    emit checkMessages();
    //

    //guiComponent->setPos(this->mapToScene(position));
    qDebug() << "GraphicsView: " << pGuiComponent->parent();

     //mLibraryMapPtrs.insert(libraryName, newLibContent);
    //this->mComponentMap.insert()
    this->mComponentMap.insert(pGuiComponent->getName(), pGuiComponent);
    //APAthis->scene()->addItem(guiComponent);

    qDebug() << "Component created at (" << pGuiComponent->x() << " " << pGuiComponent->y() << ")";
}


//! Delete componenet with specified name
//! @param componentName is the name of the componenet to delete
void GraphicsView::deleteComponent(QString componentName)
{
    //qDebug() << "In delete component";
    QMap<QString, GUIComponent *>::iterator it;
    it = mComponentMap.find(componentName);
    if (it != mComponentMap.end())
    {
        GUIComponent* c_ptr = it.value();
        mComponentMap.erase(it);
        c_ptr->mpCoreComponent->getSystemParent()->removeSubComponent(c_ptr->mpCoreComponent, true);
        scene()->removeItem(c_ptr);
        delete(c_ptr);
        emit checkMessages();
    }
    else
    {
        qDebug() << "In delete component: could not find component with name " << componentName;
    }
}

//! This function is used to rename a GUI Component (including key rename in component map)
void GraphicsView::renameComponent(QString oldName, QString newName)
{
    //First find record with old name
    QMap<QString, GUIComponent *>::iterator it = mComponentMap.find(oldName);
    if (it != mComponentMap.end())
    {
        //Make a backup copy
        GUIComponent* c_ptr = it.value();
        //Erase old record
        mComponentMap.erase(it);
        //Rename (core rename will be handled by core)
        c_ptr->setName(newName);
        //Re insert
        mComponentMap.insert(c_ptr->getName(), c_ptr);
        qDebug() << "GUI rename: " << oldName << " " << c_ptr->getName();
    }
    else
    {
        qDebug() << "Old name: " << oldName << " not found";
    }
}


//! Tells whether or not a component with specified name exist in the GraphicsView
bool GraphicsView::haveComponent(QString name)
{
    if (mComponentMap.count(name) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//! Defines what happens when scrolling the mouse in a GraphicsView.
//! @param event contains information of the scrolling operation.
void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        qreal factor = pow(1.41,(-event->delta()/240.0));
        this->scale(factor,factor);
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        //this->setDragMode(QGraphicsView::ScrollHandDrag);       //Zoom function
        if (this->mIsCreatingConnector)
        {
            qDebug() << "blabla";
            QCursor cursor;
            mpTempConnector->getThisLine()->setGeometry(GUIConnectorLine::DIAGONAL);
        }
    }

    if (event->key() == Qt::Key_Delete)
    {
        emit keyPressDelete();
    }
    if (event->key() == Qt::Key_R)
    {
        emit keyPressR();
    }

    QGraphicsView::keyPressEvent ( event );
}


void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    this->setDragMode(QGraphicsView::RubberBandDrag);
    if (this->mIsCreatingConnector)
    {
        if (mpTempConnector->getLastLine()->getGeometry()==GUIConnectorLine::HORIZONTAL)
        {
           mpTempConnector->getThisLine()->setGeometry(GUIConnectorLine::VERTICAL);
        }
        else
        {
           mpTempConnector->getThisLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        }
    }

    QGraphicsView::keyReleaseEvent ( event );
}


//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QCursor cursor;
    //qDebug() << "X=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "Y=" << this->mapFromGlobal(cursor.pos()).y();
    this->setBackgroundBrush(Qt::NoBrush);

    if (this->mIsCreatingConnector)
    {
        mpTempConnector->drawLine(mpTempConnector->startPos, this->mapToScene(event->pos()));
    }
}


//! Defines what happens when clicking in a GraphicsView.
//! @param event contains information of the mouse click operation.
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        if (this->mIsCreatingConnector)
        {
            if (mpTempConnector->getNumberOfLines() < 3)
            {
                this->mIsCreatingConnector = false;
            }
            mpTempConnector->removeLine(this->mapToScene(event->pos()));
        }
    }
    else if  ((event->button() == Qt::LeftButton) && (this->mIsCreatingConnector))
    {
        if (mpTempConnector->getThisLine()->getGeometry()==GUIConnectorLine::DIAGONAL)
        {
            mpTempConnector->addLine();
            mpTempConnector->getThisLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        }
        mpTempConnector->addLine();
    }
    emit viewClicked();
    QGraphicsView::mousePressEvent(event);
}


//! Returns a pointer to the component with specified name.
GUIComponent *GraphicsView::getComponent(QString name)
{
    //qDebug() << mComponentMap.size();
    return mComponentMap.find(name).value();
    //! @todo Cast exception or something if component is not found
}


//! Begin creation of connector or complete creation of connector depending on the mIsCreatingConnector boolean.
void GraphicsView::addConnector(GUIPort *pPort)
{
    if (!mIsCreatingConnector)
    {
        std::cout << "GraphicsView: " << "Adding connector";
        QPointF oldPos = pPort->mapToScene(pPort->boundingRect().center());
        QPen passivePen = QPen(QColor("black"),2);
        QPen activePen = QPen(QColor("red"), 2*1.6180339887499);
        QPen hoverPen = QPen(QColor("darkRed"),2*1.6180339887499);
        mpTempConnector = new GUIConnector(oldPos.x(), oldPos.y(), oldPos.x(), oldPos.y(), passivePen, activePen, hoverPen, this);
        qDebug() << "DEBUG 0.3";
        this->scene()->addItem(mpTempConnector);
        qDebug() << "DEBUG 0.4";
        this->mIsCreatingConnector = true;
        pPort->getComponent()->addConnector(mpTempConnector);
        mpTempConnector->addLine();
        mpTempConnector->setStartPort(pPort);
        qDebug() << "DEBUG 0.5";
        qDebug() << "DEBUG 0.6";
    }
    else
    {
        //! @todo This will lead to crash if you click to fast to moany times on the same port
        mpTempConnector->removeLine(pPort->mapToScene(pPort->boundingRect().center()));
        //Core interaction
        Port *start_port = mpTempConnector->getStartPort()->mpCorePort;
        Port *end_port = pPort->mpCorePort;
        bool sucess = mpModel->connect(start_port, end_port);
        if (sucess)
        {
            mIsCreatingConnector = false;
            QPointF newPos = pPort->mapToScene(pPort->boundingRect().center());
            mpTempConnector->drawLine(mpTempConnector->startPos, newPos);
            pPort->getComponent()->addConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);

            mpTempConnector->getStartPort()->hide();
            mpTempConnector->getEndPort()->hide();

            std::stringstream tempStream;
            tempStream << mpTempConnector->getStartPort()->getComponent()->getName().toStdString() << " " << mpTempConnector->getStartPort()->getPortNumber() << " " <<
                          mpTempConnector->getEndPort()->getComponent()->getName().toStdString() << " " << mpTempConnector->getEndPort()->getPortNumber();
            this->mConnectionMap.insert(QString(tempStream.str().c_str()), mpTempConnector);

            //qDebug() << mConnectionVector.last();
        }
        emit checkMessages();
        //
    }
}


void GraphicsView::removeConnector(GUIConnector* pConnector)
{
    //! @todo some error handling both ports must exist and be connected to each other
    //Core interaction
    mpModel->disconnect(pConnector->getStartPort()->mpCorePort, pConnector->getEndPort()->mpCorePort);
    emit checkMessages();
    //
    scene()->removeItem(pConnector);
    delete pConnector;
}

ComponentSystem *GraphicsView::getModelPointer()
{
    return this->mpModel;
}


//! @class GraphicsScene
//! @brief The GraphicsScene class is a container for graphicsl components in a simulationmodel.
//!

//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsScene::GraphicsScene(ProjectTab *parent)
        :   QGraphicsScene(parent)
{
    mpParentProjectTab = parent;
    setSceneRect(0.0, 0.0, 800.0, 600.0);
    connect(this, SIGNAL(changed( const QList<QRectF> & )),this->parent(), SLOT(hasChanged()));
}


//! @class ProjectTab
//! @brief The ProjectTab class is a Widget to contain a simulation model
//!
//! ProjectTab contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTab::ProjectTab(ProjectTabWidget *parent)
    : QWidget(parent)
{
    mpParentProjectTabWidget = parent;

    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent->parent()->parent())); //Ugly!!!
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    //Core interaction
    mpComponentSystem = mpParentProjectTabWidget->mpHopsan->CreateComponentSystem();
    mpComponentSystem->setName("APA");
    mpComponentSystem->setDesiredTimestep(.001);
    mpComponentSystem->setTypeCQS("S");
    emit checkMessages();
    //

    double timeStep = mpComponentSystem->getDesiredTimeStep();
    mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->setTimeStepLabel(timeStep);

    mIsSaved = true;
    mModelFileName.clear();

    mpGraphicsScene = new GraphicsScene(this);
    mpGraphicsView  = new GraphicsView(mpParentProjectTabWidget->mpHopsan, mpComponentSystem, this);

    //mpView = view;

    mpGraphicsView->setScene(mpGraphicsScene);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(mpGraphicsView);
//    addStretch(1);

//    setWindowModified(true);

    setLayout(tabLayout);

}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ProjectTab::hasChanged()
{
    if (mIsSaved)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());

        tabName.append("*");
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);

        mIsSaved = false;
    }
}


//! @class ProjectTabWidget
//! @brief The ProjectTabWidget class is a container class for ProjectTab class
//!
//! ProjectTabWidget contains ProjectTabWidget widgets.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTabWidget::ProjectTabWidget(MainWindow *parent)
        :   QTabWidget(parent)
{
    mpParentMainWindow = parent;

    mpHopsan = HopsanEssentials::getInstance();

    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent)); //Ugly!!!
    pMainWindow->mpMessageWidget->setHopsanCorePtr(mpHopsan);
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));


    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

}


//! Access current tabwidget.
//! @return the current tabwidget
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
}


//! Adds an existing ProjectTab object to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab(ProjectTab *projectTab, QString tabName)
{
    projectTab->setParent(this);

    addTab(projectTab, tabName);
    setCurrentWidget(projectTab);
}


//! Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addNewProjectTab(QString tabName)
{
    tabName.append(QString::number(mNumberOfUntitledTabs));

    ProjectTab *newTab = new ProjectTab(this);
    newTab->mIsSaved = false;

    addTab(newTab, tabName.append(QString("*")));
    setCurrentWidget(newTab);

    mNumberOfUntitledTabs += 1;
}


//! Saves current project.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTab()
{
    saveProjectTab(currentIndex(), false);
}

//! Saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTabAs()
{
    saveProjectTab(currentIndex(), true);
}


//! Saves project at index.
//! @param index defines which project to save.
//! @see saveProjectTab()
void ProjectTabWidget::saveProjectTab(int index, bool saveAs)
{
    ProjectTab *currentTab = qobject_cast<ProjectTab *>(widget(index));
    QString tabName = tabText(index);

    if (currentTab->mIsSaved)
    {
        //Nothing to do
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" is already saved")));
    }
    else
    {
        /*Add some "saving code" in the future:
         *
         *
         *
         */
        tabName.chop(1);
        setTabText(index, tabName);
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" saved")));
        std::cout << "ProjectTabWidget: " << qPrintable(QString("Project: ").append(tabName).append(QString(" saved"))) << std::endl;
        currentTab->mIsSaved = true;
    }
    this->saveModel(saveAs);
}


//! Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(qobject_cast<ProjectTab *>(widget(index))->mIsSaved))
    {
        QString modelName;
        modelName = tabText(index);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            std::cout << "ProjectTabWidget: " << "Save and close" << std::endl;
            saveProjectTab(index, false);
            removeTab(index);
            return true;
        case QMessageBox::Discard:
            // Don't Save was clicked
            removeTab(index);
            return true;
        case QMessageBox::Cancel:
            // Cancel was clicked
            std::cout << "ProjectTabWidget: " << "Cancel closing" << std::endl;
            return false;
        default:
            // should never be reached
            return false;
        }
    }
    else
    {
        std::cout << "ProjectTabWidget: " << "Closing project: " << qPrintable(tabText(index)) << std::endl;
        //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
        removeTab(index);
        return true;
    }
}


//! Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
bool ProjectTabWidget::closeAllProjectTabs()
{
    while(count() > 0)
    {
        setCurrentIndex(count()-1);
        if (!closeProjectTab(count()-1))
        {
            return false;
        }
    }
    return true;
}


//! Simulates the model in current open tab in a separate thread, the GUI runs a progressbar parallel to the simulation.
void ProjectTabWidget::simulateCurrent()
{
    if (!currentWidget())
    {
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("There is no open system to simulate"));
        return;
    }

    ProjectTab *pCurrentTab = getCurrentTab();

    double startTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->getStartTimeLabel();
    double finishTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->getFinishTimeLabel();

    double *pCoreComponentTime = pCurrentTab->mpComponentSystem->getTimePtr();
    QString timeTxt;
    double dt = finishTime - startTime;
    size_t nSteps = dt/pCurrentTab->mpComponentSystem->getDesiredTimeStep();

    QProgressDialog progressBar(tr("Initialize simulation..."), tr("&Abort initialization"), 0, 0, this);
    std::cout << progressBar.geometry().width() << " " << progressBar.geometry().height() << std::endl;
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Simulate!"));

    InitializationThread actualInitialization(pCurrentTab->mpComponentSystem, startTime, finishTime, this);
    size_t i=0;
    actualInitialization.start();
    actualInitialization.setPriority(QThread::TimeCriticalPriority);
    while (actualInitialization.isRunning())
    {
        progressBar.setValue(i++);
        if (progressBar.wasCanceled())
        {
            actualInitialization.terminate(); //! @todo not a good idea to terninate here
            break;
        }
    }
    progressBar.setValue(i);
    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation

    if (!progressBar.wasCanceled())
    {
        SimulationThread actualSimulation(pCurrentTab->mpComponentSystem, startTime, finishTime, this);
        actualSimulation.start();
        actualSimulation.setPriority(QThread::TimeCriticalPriority);
        progressBar.setLabelText(tr("Running simulation..."));
        progressBar.setCancelButtonText(tr("&Abort simulation"));
        progressBar.setMinimum(0);
        progressBar.setMaximum(nSteps);
        while (actualSimulation.isRunning())
        {
            progressBar.setValue((size_t)(*pCoreComponentTime/dt * nSteps));
            if (progressBar.wasCanceled())
            {
                actualSimulation.terminate(); //! @todo not a good idea to terninate here
                break;
            }
//            QWaitCondition w; //Not good, it makes the event loop to sleep... but it works
//            QMutex sleepmutex;
//            sleepmutex.lock();
//            w.wait(&sleepmutex, 500);
//            sleepmutex.unlock();
        }
        progressBar.setValue(nSteps);
        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    }

    mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulated '").append(QString::fromStdString(pCurrentTab->mpComponentSystem->getName())).append(tr("' successfully!"))));
    emit checkMessages();

}

//! Loads a model from a file and opens it in a new project tab.
//! @see saveModel(bool saveAs)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;

    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath(),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if (modelFileName.isEmpty())
        return;

    qDebug() << "Opening model file: " << modelFileName.toStdString().c_str();

    std::ifstream modelFile (modelFileName.toStdString().c_str());

    QFileInfo fileInfo(modelFileName);

    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    pCurrentTab->mModelFileName = modelFileName;

        //Necessary declarations
    string inputLine;
    string inputWord;
    string componentType;
    string componentName;
    string startComponentName, endComponentName;
    int startPortNumber, endPortNumber;
    int length, heigth;
    int posX, posY;
    int nameTextPos;

    while (! modelFile.eof() )
    {
            //Read the line
        getline(modelFile,inputLine);                                   //Read a line
        stringstream inputStream(inputLine);

            //Extract first word unless stream is empty
        if ( inputStream >> inputWord )
        {
            qDebug() << QString(inputWord.c_str());

            //----------- Create New SubSystem -----------//

            if ( inputWord == "COMPONENT" )
            {
                inputStream >> componentType;
                inputStream >> componentName;
                inputStream >> posX;
                inputStream >> posY;
                inputStream >> nameTextPos;
                pCurrentTab->mpGraphicsView->addComponent(QString(componentType.c_str()), QPoint(posX, posY), QString(componentName.c_str()));
                pCurrentTab->mpGraphicsView->getComponent(QString(componentName.c_str()))->setNameTextPos(nameTextPos);
            }
            if ( inputWord == "CONNECT" )
            {
                GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
                inputStream >> startComponentName;
                inputStream >> startPortNumber;
                inputStream >> endComponentName;
                inputStream >> endPortNumber;
                qDebug() << "DEBUG 0";
                GUIPort *startPort = pCurrentView->getComponent(QString(startComponentName.c_str()))->getPort(startPortNumber);
                pCurrentView->addConnector(startPort);
                qDebug() << "DEBUG 1";
                GUIConnector *pTempConnector = pCurrentView->getTempConnector();
                qDebug() << "DEBUG 2";
                pCurrentView->scene()->addItem(pTempConnector);
                while(inputStream >> inputWord)
                {
                    if(inputWord == "VERTICAL")
                    {
                        inputStream >> heigth;
                        qDebug() << "Heigth = " << heigth;
                        //pTempConnector->getThisLine()->setGeometry(GUIConnectorLine::VERTICAL);
                        pTempConnector->addFixedLine(0, heigth, GUIConnectorLine::VERTICAL);
                    }
                    else if (inputWord == "HORIZONTAL")
                    {
                        inputStream >> length;
                       // pTempConnector->getThisLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
                        pTempConnector->addFixedLine(length, 0, GUIConnectorLine::HORIZONTAL);
                    }
                    else if (inputWord == "DIAGONAL")
                    {
                        inputStream >> length;
                        inputStream >> heigth;
                        //pTempConnector->getThisLine()->setGeometry(GUIConnectorLine::DIAGONAL);
                        pTempConnector->addFixedLine(length, heigth, GUIConnectorLine::DIAGONAL);
                    }
                    else
                    {
                    }
                }
                GUIPort *endPort = pCurrentView->getComponent(QString(endComponentName.c_str()))->getPort(endPortNumber);
                QPointF newPos = endPort->mapToScene(endPort->boundingRect().center());
                pTempConnector->drawLine(pTempConnector->startPos, newPos);
                endPort->getComponent()->addConnector(pTempConnector);
                pTempConnector->setEndPort(endPort);
                pTempConnector->getStartPort()->hide();
                pTempConnector->getEndPort()->hide();
                pCurrentView->mIsCreatingConnector = false;

                std::stringstream tempStream;
                tempStream << startPort->getComponent()->getName().toStdString() << " " << startPort->getPortNumber() << " " <<
                              endPort->getComponent()->getName().toStdString() << " " << endPort->getPortNumber();
                pCurrentView->mConnectionMap.insert(QString(tempStream.str().c_str()), pTempConnector);
                bool success = pCurrentView->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort);
            }

        }
    }
    emit checkMessages();
}


//! Saves the model in the active project tab to a model file.
//! @param saveAs tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTabWidget::saveModel(bool saveAs)
{
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;

    QString modelFileName;
    if(pCurrentTab->mModelFileName.isEmpty() | saveAs)
    {
        QDir fileDialogSaveDir;
        modelFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath(),
                                                             tr("Hopsan Model Files (*.hmf)"));
        pCurrentTab->mModelFileName = modelFileName;
    }
    else
    {
        modelFileName = pCurrentTab->mModelFileName;
    }
    std::ofstream modelFile (modelFileName.toStdString().c_str());
    QFileInfo fileInfo(modelFileName);

    QMap<QString, GUIComponent *>::iterator it;
    for(it = pCurrentView->mComponentMap.begin(); it!=pCurrentView->mComponentMap.end(); ++it)
    {
        QPointF pos = it.value()->mapToScene(it.value()->boundingRect().center());
        modelFile << "COMPONENT " << it.value()->getTypeName().toStdString() << " " << it.key().toStdString()
                  << " " << pos.x() << " " << pos.y() << " " << it.value()->getNameTextPos() << std::endl;
        //<< " " << it.value()->mapToScene(it.value()->pos()).x() << " " << it.value()->mapToScene(it.value()->pos()).y() << std::endl;
    }

    QMap<QString, GUIConnector *>::iterator it2;
    for(it2 = pCurrentView->mConnectionMap.begin(); it2!=pCurrentView->mConnectionMap.end(); ++it2)
    {
        modelFile << "CONNECT " << it2.key().toStdString();
        for(int i = 0; i!=it2.value()->mLines.size(); ++i)
        {
            int geometry = it2.value()->mLines[i]->getGeometry();
            switch (geometry)
            {
                case 0:
                    modelFile << " VERTICAL " << (it2.value()->mLines[i]->endPos.y()-it2.value()->mLines[i]->startPos.y());
                    break;
                case 1:
                    modelFile << " HORIZONTAL " << (it2.value()->mLines[i]->endPos.x()-it2.value()->mLines[i]->startPos.x());
                    break;
                case 2:
                    modelFile << " DIAGONAL" << (it2.value()->mLines[i]->endPos.x()-it2.value()->mLines[i]->startPos.x()) << (it2.value()->mLines[i]->endPos.y()-it2.value()->mLines[i]->startPos.y());
                    break;
            }
        }
        modelFile << "\n";

    }
}

