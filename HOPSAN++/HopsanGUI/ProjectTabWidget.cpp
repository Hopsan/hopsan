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

#include <QtGui>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>


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

void GraphicsView::addComponent(QString parameterType, QPoint position, QString name)
{
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
}

void GraphicsView::deleteComponent(QString componentName)
{
    qDebug() << "In delete component";
    QMap<QString, GUIComponent *>::iterator it;
    it = mComponentMap.find(componentName);
    GUIComponent* c_ptr = it.value();
    mComponentMap.erase(it);
    c_ptr->mpCoreComponent->getSystemParent()->removeSubComponent(c_ptr->mpCoreComponent, true);
    scene()->removeItem(c_ptr);
    delete(c_ptr);
    emit checkMessages();
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
            QCursor cursor;
            mpTempConnector->getThisLine()->setGeometry(GUIConnectorLine::DIAGONAL);
        }
    }

    if (event->key() == Qt::Key_Delete)
    {
        emit keyPressDelete();
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
    //std::cout << "X=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "Y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;
    this->setBackgroundBrush(Qt::NoBrush);

    if (this->mIsCreatingConnector)
    {
        mpTempConnector->drawLine(mpTempConnector->startPos, this->mapToScene(event->pos()));
    }
}


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
    //case InsertLine:
//        lineH = new QGraphicsLineItem(QLineF(event->x(), event->y(), event->x(), event->y()));
//        lineV = new QGraphicsLineItem(QLineF(event->x(), event->y(), event->x(), event->y()));
//        lineH->setPen(QPen(myLineColor, 2));
//        lineV->setPen(QPen(myLineColor, 2));
//        this->scene()->addItem(lineH);
//        this->scene()->addItem(lineV);
//        //break;

        QGraphicsView::mousePressEvent(event);
}

GUIComponent *GraphicsView::getComponent(QString name)
{
    qDebug() << mComponentMap.size();
    return mComponentMap.find(name).value();
}

void GraphicsView::addConnector(GUIPort *pPort)
{
    if (!mIsCreatingConnector)
    {
        std::cout << "GraphicsView: " << "Adding connector";
        QPointF oldPos = pPort->mapToScene(pPort->boundingRect().center());
        QPen passivePen = QPen(QColor("black"),2);
        QPen activePen = QPen(QColor("red"), 3);
        QPen hoverPen = QPen(QColor("darkRed"),2);
        mpTempConnector = new GUIConnector(oldPos.x(), oldPos.y(), oldPos.x(), oldPos.y(), passivePen, activePen, hoverPen, this);
        this->scene()->addItem(mpTempConnector);
        this->mIsCreatingConnector = true;
        pPort->getComponent()->addConnector(mpTempConnector);
        mpTempConnector->setStartPort(pPort);
        mpTempConnector->addLine();
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
        }
        emit checkMessages();
        //
    }
}


void GraphicsView::removeConnection(GUIConnector* pConnector)
{
    //! @todo some error handling both ports must exist and be connected to each other
    //Core interaction
    mpModel->disconnect(pConnector->getStartPort()->mpCorePort, pConnector->getEndPort()->mpCorePort);
    emit checkMessages();
    //
}


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

    mIsSaved = true;

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
    saveProjectTab(currentIndex());
}


//! Saves project at index.
//! @param index defines which project to save.
//! @see saveProjectTab()
void ProjectTabWidget::saveProjectTab(int index)
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
            saveProjectTab(index);
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


//! Simulates the model in current open tab.
void ProjectTabWidget::simulateCurrent()
{
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());

    pCurrentTab->mpComponentSystem->initialize(0.0, 5.0); //HARD CODED
    pCurrentTab->mpComponentSystem->simulate(0.0, 5.0); //HARD CODED
    //pCurrentTab->mpModel->getSubComponent("DefaultLaminarOrificeName")->getPort("P1").saveLogData("output.txt");
    emit checkMessages();

}


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

        //Necessary declarations
    string inputLine;
    string inputWord;
    string componentType;
    string componentName;
    string startComponentName, endComponentName;
    int startPortNumber, endPortNumber;
    int length, heigth;
    int posX, posY;

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
                pCurrentTab->mpGraphicsView->addComponent(QString(componentType.c_str()), QPoint(posX, posY), QString(componentName.c_str()));
            }
            if ( inputWord == "CONNECT" )
            {
                GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
                inputStream >> startComponentName;
                inputStream >> startPortNumber;
                inputStream >> endComponentName;
                inputStream >> endPortNumber;
                qDebug() << "DEBUG1";
                pCurrentView->addConnector(pCurrentView->getComponent(QString(startComponentName.c_str()))->getPort(startPortNumber));
                qDebug() << "DEBUG2";
                GUIConnector *pTempConnector = pCurrentView->getTempConnector();
                qDebug() << "DEBUG3";
                pCurrentView->scene()->addItem(pTempConnector);
                qDebug() << "DEBUG4";
                while(inputStream >> inputWord)
                {
                    if(inputWord == "VERTICAL")
                    {
                        inputStream >> heigth;
                        pTempConnector->getThisLine()->setGeometry(GUIConnectorLine::VERTICAL);
                        pTempConnector->addFixedLine(0, heigth, GUIConnectorLine::VERTICAL);
                    }
                    else if (inputWord == "HORIZONTAL")
                    {
                        inputStream >> length;
                        pTempConnector->getThisLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
                        pTempConnector->addFixedLine(length, 0, GUIConnectorLine::HORIZONTAL);
                    }
                    else if (inputWord == "DIAGONAL")
                    {
                        inputStream >> length;
                        inputStream >> heigth;
                        pTempConnector->getThisLine()->setGeometry(GUIConnectorLine::DIAGONAL);
                        pTempConnector->addFixedLine(length, heigth, GUIConnectorLine::DIAGONAL);
                    }
                    else
                    {
                    }
                    GUIPort *endPort = pCurrentView->getComponent(QString(endComponentName.c_str()))->getPort(endPortNumber);
                    QPointF newPos = endPort->mapToScene(endPort->boundingRect().center());
                    pTempConnector->drawLine(pTempConnector->startPos, newPos);
                    endPort->getComponent()->addConnector(pTempConnector);
                    pTempConnector->setEndPort(endPort);
                    pCurrentView->mIsCreatingConnector = false;
                }
            }
        }
    }
}
