//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$


#include "HopsanCore.h"
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "LibraryWidget.h"
#include "mainwindow.h"
#include "SimulationSetupWidget.h"
#include "MessageWidget.h"
#include "SimulationThread.h"
#include "InitializationThread.h"
#include <QtGui>
#include <QSizePolicy>
#include "version.h"
#include "GUIUtilities.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>

//! @class GraphicsView
//! @brief The GraphicsView class is a class which display the content of a scene of components.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(HopsanEssentials *pHopsanCore, ComponentSystem *pCoreComponentSystem, ProjectTab *parent)
        : QGraphicsView(parent)
{
    mpParentProjectTab = parent;

    this->mpHopsanCore = pHopsanCore;
    this->mpCoreComponentSystem = pCoreComponentSystem;
    this->setDragMode(RubberBandDrag);
    this->setInteractive(true);
    this->setEnabled(true);
    this->setAcceptDrops(true);
    this->mIsCreatingConnector = false;
    this->mPortsHidden = false;
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setSceneRect(0,0,5000,5000);
    this->centerOn(this->sceneRect().center());
    this->mBackgroundColor = QColor(Qt::white);
    this->setBackgroundBrush(mBackgroundColor);
    this->createActions();
    this->createMenus();

    mPrimaryPenPowerIso = QPen(QColor("black"),1, Qt::SolidLine, Qt::RoundCap);
    mActivePenPowerIso = QPen(QColor("red"), 2, Qt::SolidLine, Qt::RoundCap);
    mHoverPenPowerIso = QPen(QColor("darkRed"),2, Qt::SolidLine, Qt::RoundCap);

    mPrimaryPenSignalIso = QPen(QColor("blue"),1, Qt::DashLine);
    mActivePenSignalIso = QPen(QColor("red"), 2, Qt::DashLine);
    mHoverPenSignalIso = QPen(QColor("darkRed"),2, Qt::DashLine);

    mPrimaryPenPowerUser = QPen(QColor("black"),2, Qt::SolidLine, Qt::RoundCap);
    mActivePenPowerUser = QPen(QColor("red"), 3, Qt::SolidLine, Qt::RoundCap);
    mHoverPenPowerUser = QPen(QColor("darkRed"),3, Qt::SolidLine, Qt::RoundCap);

    mPrimaryPenSignalUser = QPen(QColor("blue"),1, Qt::DashLine);
    mActivePenSignalUser = QPen(QColor("red"), 2, Qt::DashLine);
    mHoverPenSignalUser = QPen(QColor("darkRed"),2, Qt::DashLine);

    undoStack = new UndoStack(this);
    //undoStack->show();

    MainWindow *pMainWindow = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(this->systemPortAction, SIGNAL(triggered()), SLOT(addSystemPort()));
    connect(pMainWindow->cutAction, SIGNAL(triggered()), this,SLOT(cutSelected()));
    connect(pMainWindow->copyAction, SIGNAL(triggered()), this,SLOT(copySelected()));
    connect(pMainWindow->pasteAction, SIGNAL(triggered()), this,SLOT(paste()));
    connect(pMainWindow->hidePortsAction, SIGNAL(triggered(bool)), this,SLOT(hidePorts(bool)));
    connect(pMainWindow->undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(pMainWindow->redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(pMainWindow->mpUndoWidget->undoButton, SIGNAL(pressed()), this, SLOT(undo()));
    connect(pMainWindow->mpUndoWidget->redoButton, SIGNAL(pressed()), this, SLOT(redo()));
    connect(pMainWindow->mpUndoWidget->clearButton, SIGNAL(pressed()), this, SLOT(clearUndo()));
}


//! @todo Finish this!
bool GraphicsView::isObjectSelected()
{
    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        if(it.value()->isSelected())
        {
            return true;
        }
    }
    return false;
}


//! @todo Finish this!
bool GraphicsView::isConnectorSelected()
{
    for(int i = 0; i != mConnectorVector.size(); ++i)
    {
        if (mConnectorVector[i]->isActive())
        {
            return true;
        }
    }
    return false;
}


void GraphicsView::createMenus()
{
    menuInsert = new QMenu(this);
    menuInsert->setObjectName("menuInsert");
    menuInsert->setTitle("Insert");
    menuInsert->addAction(systemPortAction);
}

void GraphicsView::createActions()
{
    systemPortAction = new QAction(this);
    systemPortAction->setText("System Port");
}


void GraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
    if(!mIsCreatingConnector and !mJustStoppedCreatingConnector)
    {
        if (QGraphicsItem *item = itemAt(event->pos()))
            QGraphicsView::contextMenuEvent(event);
        else
        {
            QMenu menu(this);
            menu.addMenu(menuInsert);
            menu.exec(event->globalPos());
        }
        mJustStoppedCreatingConnector = true;
    }
}


////! Destructor.
//GraphicsView::~GraphicsView()
//{
//
//}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    //if (event->mimeData()->hasFormat("application/x-text"))
    if (event->mimeData()->hasText())
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
    //if (event->mimeData()->hasFormat("application/x-text"))
    if (event->mimeData()->hasText())
    {
        undoStack->newPost();

        //qDebug() << "dropEvent: hasText";
        //QByteArray *data = new QByteArray;
        //*data = event->mimeData()->data("application/x-text");

        QString datastr =  event->mimeData()->text();
        QTextStream stream(&datastr, QIODevice::ReadOnly);

        //qDebug() << "drop string: \n" << datastr;

        AppearanceData appearanceData;
        stream >> appearanceData;

        //! @todo Check if appearnaceData OK otherwihse do not add

        //qDebug() << "Drop appearanceData: " <<  appearanceData;

        event->accept();


        QPoint position = event->pos();
        //qDebug() << "GraphicsView: " << "x=" << position.x() << "  " << "y=" << position.y();

        this->addGUIObject(appearanceData.getTypeName(), appearanceData, this->mapToScene(position).toPoint());
    }
}


GUIConnector *GraphicsView::getTempConnector()
{
    return this->mpTempConnector;
}


//! @brief Temporary addSubSystem functin should be same later on
//! Adds a new component to the GraphicsView.
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
void GraphicsView::addGUIObject(QString componentTypeName, AppearanceData appearanceData, QPoint position, qreal rotation, QString name, bool startSelected, bool doNotRegisterUndo)
{
    if (componentTypeName == "Subsystem")
    {
        GUISubsystem *pSys = new GUISubsystem(mpHopsanCore, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
        this->mpParentProjectTab->mpCoreComponentSystem->addComponent(pSys->getHopsanCoreSystemComponentPtr()); //core interaction

        mpTempGUIObject = pSys;
    }
    else if (componentTypeName == "SystemPort")
    {
        mpTempGUIObject = new GUISystemPort(mpParentProjectTab->mpCoreComponentSystem, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }
    else //Assume some standard component type
    {
        GUIComponent *pComp = new GUIComponent(mpHopsanCore, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
        this->mpParentProjectTab->mpCoreComponentSystem->addComponent(pComp->getHopsanCoreComponentPtr()); //core interaction
        mpTempGUIObject = pComp;
    }

    //qDebug() << "=====================Get initial name: " << mpTempGUIObject->getName() << "requested: " << name;
    if (!name.isEmpty())
    {
        qDebug() << "name not empty, setting to: " << name;
        //Set name, do NOT try to do smart rename. (If component already exist with new component default name that other component would be renamed)
        mpTempGUIObject->setName(name, true);
    }

    mpTempGUIObject->refreshName();
    emit checkMessages();
    //qDebug() << "=====================Get name after add: " << mpTempGUIObject->getName();
    //

    mpTempGUIObject->setIcon(this->mpParentProjectTab->useIsoGraphics);

    while (mpTempGUIObject->rotation() != rotation)
    {
        mpTempGUIObject->rotate();
    }

    //guiComponent->setPos(this->mapToScene(position));
    //qDebug() << "GraphicsView: " << mpTempGUIObject->parent();

    if ( mGUIObjectMap.contains(mpTempGUIObject->getName()) )
    {
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget->printGUIErrorMessage("Trying to add component with name: " + mpTempGUIObject->getName() + " that already exist in GUIObjectMap, (Not adding)");
    }
    else
    {
        mGUIObjectMap.insert(mpTempGUIObject->getName(), mpTempGUIObject);
        //qDebug() << "GUI Object created at (" << mpTempGUIObject->x() << " " << mpTempGUIObject->y() << ")";
    }

        //Deselect all other comonents
    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(false);
    }
    mpTempGUIObject->setSelected(startSelected);
    this->setFocus(Qt::OtherFocusReason);

    if(!doNotRegisterUndo)
    {
        undoStack->registerAddedObject(mpTempGUIObject);
    }
}

////! Adds a new component to the GraphicsView.
////! @param componentType is a string defining the type of component.
////! @param position is the position where the component will be created.
////! @param name will be the name of the component.
//void GraphicsView::addComponent(QString componentType, QPoint position, QString name, bool startSelected)
//{
//    MainWindow *pMainWindow = qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent());
//    LibraryWidget *pLibrary = pMainWindow->mpLibrary;
//    QStringList appearanceData = pLibrary->getAppearanceData(componentType);
//
//    GUIComponent *pGuiComponent = new GUIComponent(mpHopsan, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
//
//    qDebug() << "=====================Get initial name: " << pGuiComponent->getName() << "requested: " << name;
//    if (!name.isEmpty())
//    {
//        qDebug() << "name not empty, setting to: " << name;
//        //Set name, do NOT try to do smart rename. (If component already exist with new component default name that other component would be renamed)
//        pGuiComponent->setName(name, true);
//    }
//
//    pGuiComponent->setSelected(startSelected);
//
//    //Core interaction
//    qDebug() << "=====================Get name before add: " << pGuiComponent->getName();
//    this->mpParentProjectTab->mpComponentSystem->addComponent(pGuiComponent->mpCoreComponent);
//    //    qobject_cast<ProjectTab *>(this->parent())->mpModel->addComponent(guiComponent->mpCoreComponent);
//    pGuiComponent->refreshName();
//    emit checkMessages();
//    qDebug() << "=====================Get name after add: " << pGuiComponent->getName();
//    //
//
//    //guiComponent->setPos(this->mapToScene(position));
//    qDebug() << "GraphicsView: " << pGuiComponent->parent();
//
//    //mLibraryMapPtrs.insert(libraryName, newLibContent);
//    //this->mComponentMap.insert()
//    this->mGUIObjectMap.insert(pGuiComponent->getName(), pGuiComponent);
//    //APAthis->scene()->addItem(guiComponent);
//
//}

void GraphicsView::addSystemPort()
{
    qDebug() <<"Adding a system port";
    QCursor cursor;
    QPointF position = this->mapToScene(this->mapFromGlobal(cursor.pos()));
    this->setBackgroundBrush(mBackgroundColor);
    //QPoint position = QPoint(2300,2400);

    AppearanceData appearanceData;
    QTextStream appstream;

    appstream << "TypeName SystemPort";
    appstream << "ICONPATH ../../HopsanGUI/systemporttmp.svg";
    appstream >> appearanceData;

    addGUIObject(QString("SystemPort"), appearanceData, position.toPoint());
}

//! Delete GUIObject with specified name
//! @param objectName is the name of the componenet to delete
void GraphicsView::deleteGUIObject(QString objectName)
{
    QMap<QString, GUIObject *>::iterator it;
    it = mGUIObjectMap.find(objectName);

    //for(int i = 0; i != mConnectorVector.size(); ++i)
    int i = 0;
    while(i != mConnectorVector.size())
    {
        if((mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == objectName) or
           (mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == objectName))
        {
            this->removeConnector(mConnectorVector[i]);
            i= 0;   //Restart iteration if map has changed
        }
        else
        {
            ++i;
        }
    }
        //Register removal of connector in undo stack (must be done after removal of connectors or the order of the commands in the undo stack will be wrong!)
    this->undoStack->registerDeletedObject(it.value());

    if (it != mGUIObjectMap.end())
    {
        GUIObject* obj_ptr = it.value();
        mGUIObjectMap.erase(it);
        obj_ptr->deleteInHopsanCore();
        scene()->removeItem(obj_ptr);
        delete(obj_ptr);
        emit checkMessages();
    }
    else
    {
        qDebug() << "In delete GUIObject: could not find object with name " << objectName;
    }
    this->setBackgroundBrush(mBackgroundColor);
}

//! This function is used to rename a GUI Component (including key rename in component map)
void GraphicsView::renameGUIObject(QString oldName, QString newName)
{
    //First find record with old name
    QMap<QString, GUIObject *>::iterator it = mGUIObjectMap.find(oldName);
    if (it != mGUIObjectMap.end())
    {
        //Make a backup copy
        GUIObject* obj_ptr = it.value();
        //Erase old record
        mGUIObjectMap.erase(it);
        //Rename (core rename will be handled by core), here we force a core only rename (true) so that we dont get stuck in a loop (as rename might be called again)
        obj_ptr->setName(newName, true);
        //Re insert
        mGUIObjectMap.insert(obj_ptr->getName(), obj_ptr);
        qDebug() << "GUI rename: " << oldName << " " << obj_ptr->getName();
    }
    else
    {
        qDebug() << "Old name: " << oldName << " not found";
    }

    undoStack->registerRenameObject(oldName, newName);

    emit checkMessages();
}


//! Tells whether or not a component with specified name exist in the GraphicsView
bool GraphicsView::haveGUIObject(QString name)
{
    if (mGUIObjectMap.count(name) > 0)
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
    if (event->modifiers() and (Qt::ControlModifier or Qt::Key_Alt))
    {
        qreal factor = pow(1.41,(-event->delta()/240.0));
        this->scale(factor,factor);
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{

    if (event->key() == Qt::Key_Delete)
    {
        if(isObjectSelected() or isConnectorSelected())
        {
            undoStack->newPost();
        }
        emit keyPressDelete();
    }
    else if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_R)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressR();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        if(this->mIsCreatingConnector)
        {
            delete(mpTempConnector);
            mIsCreatingConnector = false;
        }
    }
    else if(event->modifiers() and Qt::ShiftModifier and event->key() == Qt::Key_Left)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressShiftLeft();
    }
    else if(event->modifiers() and Qt::ShiftModifier and event->key() == Qt::Key_Right)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressShiftRight();
    }
    else if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Up)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressUp();
    }
    else if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Down)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressDown();
    }
    else if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Left)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressLeft();
    }
    else if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Right)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressRight();
    }
    else if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_A)
        this->selectAll();
    else if (event->modifiers() and Qt::ControlModifier)
    {
        if (this->mIsCreatingConnector)
        {
            //QCursor cursor;
            mpTempConnector->makeDiagonal(true);
            mpTempConnector->drawConnector();
            this->setBackgroundBrush(mBackgroundColor);
        }
        else
        {
            this->setDragMode(QGraphicsView::ScrollHandDrag);
        }
    }
    else
        QGraphicsView::keyPressEvent ( event );
}


//! Defines what shall happen when a key is released.
//! @param event contains information about the keypress operation.
void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
        // Releasing ctrl key while creating a connector means return from diagonal mode to orthogonal mode.
    if(event->key() == Qt::Key_Control and mIsCreatingConnector)
    {
        mpTempConnector->makeDiagonal(false);
        mpTempConnector->drawConnector();
        this->setBackgroundBrush(mBackgroundColor);
    }

    this->setDragMode(QGraphicsView::RubberBandDrag);

    QGraphicsView::keyReleaseEvent ( event );
}


//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    this->setBackgroundBrush(mBackgroundColor);     //Refresh the viewport

        //If creating connector, the end port shall be updated to the mouse position.
    if (this->mIsCreatingConnector)
    {
        mpTempConnector->updateEndPoint(this->mapToScene(event->pos()));
        mpTempConnector->drawConnector();
    }
}


//! Defines what happens when clicking in a GraphicsView.
//! @param event contains information of the mouse click operation.
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    //undoStack->newPost();       //! @todo: This will trigger unnecessary clear redo stack
    emit viewClicked();
    mJustStoppedCreatingConnector = false;

    if (event->button() == Qt::RightButton and this->mIsCreatingConnector)
    {
        if((mpTempConnector->getNumberOfLines() == 1 and mpTempConnector->isMakingDiagonal()) or (mpTempConnector->getNumberOfLines() == 2 and !mpTempConnector->isMakingDiagonal()))
        {
            mIsCreatingConnector = false;
            mJustStoppedCreatingConnector = true;
        }
        mpTempConnector->removePoint(true);
        if(mIsCreatingConnector)
        {
            mpTempConnector->updateEndPoint(this->mapToScene(event->pos()));
            mpTempConnector->drawConnector();
            this->setBackgroundBrush(mBackgroundColor);
        }
        qDebug() << "mIsCreatingConnector = " << mIsCreatingConnector;
    }
    else if  ((event->button() == Qt::LeftButton) && (this->mIsCreatingConnector))
    {
        mpTempConnector->addPoint(this->mapToScene(event->pos()));
    }
    QGraphicsView::mousePressEvent(event);
}


//! Returns a pointer to the component with specified name.
GUIObject *GraphicsView::getGUIObject(QString name)
{
    if(!mGUIObjectMap.contains(name))
    {
        qDebug() << "Request for pointer to non-existing component" << endl;
        assert(false);
    }
    return mGUIObjectMap.find(name).value();
}


//! Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param doNotRegisterUndo is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
void GraphicsView::addConnector(GUIPort *pPort, bool doNotRegisterUndo)
{
        //When clicking start port
    if (!mIsCreatingConnector)
    {
        std::cout << "GraphicsView: " << "Adding connector";
        QPointF oldPos = pPort->mapToScene(pPort->boundingRect().center());

        //! @todo We can not determine appearcne of connector based on first port clicked, This will fail when we are connecting system ports and are selecting the system port first
        if(this->mpParentProjectTab->useIsoGraphics)
        {
            if((pPort->mpCorePort->getNodeType() == "NodeHydraulic") | (pPort->mpCorePort->getNodeType() == "NodeMechanic"))
                mpTempConnector = new GUIConnector(oldPos, getPen("Primary", "Power", "Iso"), getPen("Active", "Power", "Iso"), getPen("Hover", "Power", "Iso"), this);
            else if(pPort->mpCorePort->getNodeType() == "NodeSignal")
                mpTempConnector = new GUIConnector(oldPos, getPen("Primary", "Signal", "Iso"), getPen("Active", "Signal", "Iso"), getPen("Hover", "Signal", "Iso"), this);
        }
        else
        {
            if((pPort->mpCorePort->getNodeType() == "NodeHydraulic") | (pPort->mpCorePort->getNodeType() == "NodeMechanic"))
                mpTempConnector = new GUIConnector(oldPos, getPen("Primary", "Power", "User"), getPen("Active", "Power", "User"), getPen("Hover", "Power", "User"), this);
            else if(pPort->mpCorePort->getNodeType() == "NodeSignal")
                mpTempConnector = new GUIConnector(oldPos, getPen("Primary", "Signal", "User"), getPen("Active", "Signal", "User"), getPen("Hover", "Signal", "User"), this);
        }
        this->scene()->addItem(mpTempConnector);
        this->mIsCreatingConnector = true;
        pPort->getGuiObject()->addConnector(mpTempConnector);

        QCursor cursor;

        mpTempConnector->setStartPort(pPort);
        mpTempConnector->addPoint(oldPos);
        mpTempConnector->addPoint(oldPos);
        mpTempConnector->drawConnector();
    }

        //When clicking end port
    else
    {
        //Core interaction
        Port *start_port = mpTempConnector->getStartPort()->mpCorePort;
        Port *end_port = pPort->mpCorePort;
        bool success = mpCoreComponentSystem->connect(start_port, end_port);
        if (success)
        {
            mIsCreatingConnector = false;
            QPointF newPos = pPort->mapToScene(pPort->boundingRect().center());
            mpTempConnector->updateEndPoint(newPos);
            pPort->getGuiObject()->addConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);

            mpTempConnector->getStartPort()->hide();
            mpTempConnector->getEndPort()->hide();

            this->mConnectorVector.append(mpTempConnector);
        }
        emit checkMessages();
        //

        undoStack->newPost();
        if(!doNotRegisterUndo)
        {
            undoStack->registerAddedConnector(mpTempConnector);
        }
    }
}


//! Removes the connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param doNotRegisterUndo is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void GraphicsView::removeConnector(GUIConnector* pConnector, bool doNotRegisterUndo)
{
    bool doDelete = false;
    int i;

    if(!doNotRegisterUndo)
    {
        undoStack->registerDeletedConnector(pConnector);
    }
    for(i = 0; i != mConnectorVector.size(); ++i)
    {
        if(mConnectorVector[i] == pConnector)
        {
             //! @todo some error handling both ports must exist and be connected to each other
             //Core interaction
             if(pConnector->isConnected())
             {
                 mpCoreComponentSystem->disconnect(pConnector->getStartPort()->mpCorePort, pConnector->getEndPort()->mpCorePort);
                 emit checkMessages();
                 pConnector->getEndPort()->show();
                 pConnector->getEndPort()->isConnected = false;
             }
             //
             scene()->removeItem(pConnector);
             pConnector->getStartPort()->show();
             pConnector->getStartPort()->isConnected = false;
             delete pConnector;
             doDelete = true;
             break;
        }
        if(mConnectorVector.empty())
            break;
    }
    if(doDelete)
    {
        mConnectorVector.remove(i);
    }    
}


//! Selects all objects and connectors.
void GraphicsView::selectAll()
{
        //Select all components
    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(true);
    }
        //Deselect all connectors
    QMap<QString, GUIConnector*>::iterator it2;
    for(int i = 0; i != mConnectorVector.size(); ++i)
    {
        mConnectorVector[i]->doSelect(true, -1);
    }
}


ComponentSystem *GraphicsView::getModelPointer()
{
    return this->mpCoreComponentSystem;
}


//! Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void GraphicsView::cutSelected()
{
    this->copySelected();
    emit keyPressDelete();      //Ugly...
    this->setBackgroundBrush(mBackgroundColor);
}


//! Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void GraphicsView::copySelected()
{
    mCopyData.clear();
    //mCopyDataRot.clear();
    //mCopyDataPos.clear();

    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        if (it.value()->isSelected())
        {
            QStringList tempStringList;
            QString stringRotation, stringX, stringY;
            stringRotation.setNum(it.value()->rotation());
            stringX.setNum(it.value()->mapToScene(it.value()->boundingRect().center()).x());
            stringY.setNum(it.value()->mapToScene(it.value()->boundingRect().center()).y());
            tempStringList << "COMPONENT" << it.value()->getTypeName() << it.value()->getName() << stringRotation << stringX << stringY;
            mCopyData.append(tempStringList);
        }
    }

    for(int i = 0; i != mConnectorVector.size(); ++i)
    {
        if(mConnectorVector[i]->getStartPort()->getGuiObject()->isSelected() and mConnectorVector[i]->getEndPort()->getGuiObject()->isSelected() and mConnectorVector[i]->isActive())
        {
            QStringList tempStringList;
            tempStringList << "CONNECT" << mConnectorVector[i]->getStartPort()->getGuiObject()->getName() << mConnectorVector[i]->getStartPort()->getName() <<
                                           mConnectorVector[i]->getEndPort()->getGuiObject()->getName() << mConnectorVector[i]->getEndPort()->getName();

            QString stringX, stringY;
            for(size_t j = 0; j != mConnectorVector[i]->getPointsVector().size(); ++j)
            {
                stringX.setNum(mConnectorVector[i]->getPointsVector()[j].x());
                stringY.setNum(mConnectorVector[i]->getPointsVector()[j].y());
                tempStringList << stringX << stringY;
            }

            mCopyData.append(tempStringList);
        }
    }
}


//! Creates each item in the copy stack, and places it on its respective position in the position copy stack.
//! @see cutSelected()
//! @see copySelected()
void GraphicsView::paste()
{
    QMap<QString, GUIObject*>::iterator it;

        //Deselect all components
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(false);
    }

        //Deselect all connectors
    for(int i = 0; i != mConnectorVector.size(); ++i)
    {
        mConnectorVector[i]->doSelect(false, -1);
    }

    QMap<QString, QString> renameMap;       //Used to track name changes, so that connectors will know what components are called
    QString tempString;
    QString componentName;
    QString componentType;
    QString startComponentName, endComponentName;
    QString startPortName, endPortName;
    int j = 0;      //Used for calculating which rotation and position to use
    for(int i = 0; i!=mCopyData.size(); ++i)
    {
        if(mCopyData[i][0] == "COMPONENT")
        {
            componentType = mCopyData[i][1];
            componentName = mCopyData[i][2];
            int tempRotation = mCopyData[i][3].toInt();
            qreal tempX = mCopyData[i][4].toDouble();
            qreal tempY = mCopyData[i][5].toDouble();
           
            AppearanceData appearanceData = *mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(componentType);
            this->addGUIObject(componentType, appearanceData, QPoint(tempX-50, tempY-50), tempRotation, componentName, true);
            ++j;
            renameMap.insert(componentName, mpTempGUIObject->getName());
        }
        else if(mCopyData[i][0] == "CONNECT")
        {
            startComponentName = mCopyData[i][1];
            startPortName = mCopyData[i][2];
            endComponentName = mCopyData[i][3];
            endPortName = mCopyData[i][4];

            startComponentName = renameMap.find(startComponentName).value();
            endComponentName = renameMap.find(endComponentName).value();

            GUIPort *startPort = this->getGUIObject(startComponentName)->getPort(startPortName);
            GUIPort *endPort = this->getGUIObject(endComponentName)->getPort(endPortName);

            QVector<QPointF> tempPointVector;
            qreal tempX, tempY;
            for(j = 5; j != mCopyData[i].size(); ++j)
            {
                tempPointVector.push_back(QPointF(mCopyData[i][j].toDouble()-50, mCopyData[i][j+1].toDouble()-50));
                ++j;
            }

            GUIConnector *pTempConnector;

            QString type, style;
            if((startPort->mpCorePort->getNodeType() == "NodeHydraulic") | (startPort->mpCorePort->getNodeType() == "NodeMechanic"))
                type = "Power";
            else if(startPort->mpCorePort->getNodeType() == "NodeSignal")
                type = "Signal";
            if(mpParentProjectTab->useIsoGraphics)
                style = "Iso";
            else
                style = "User";
            pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, this->getPen("Primary", type, style),
                                              this->getPen("Active", type, style), this->getPen("Hover", type, style), this);

            this->scene()->addItem(pTempConnector);
            pTempConnector->selectIfBothComponentsSelected();

                //Hide connected ports
            startPort->hide();
            endPort->hide();

            connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
            connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

            //std::stringstream tempStream2;
            //tempStream2 << startPort->getGuiObject()->getName().toStdString() << " " << startPort->getPortNumber() << " " <<
            //              endPort->getGuiObject()->getName().toStdString() << " " << endPort->getPortNumber();
            mConnectorVector.append(pTempConnector);
            bool success = this->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort);
            if (!success)
            {
                qDebug() << "Unsuccessful connection try" << endl;
                assert(false);
            }
        pTempConnector->setActive();
        }
    }
    this->setBackgroundBrush(mBackgroundColor);
}

void GraphicsView::setScale(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QMatrix oldMatrix = this->matrix();
    this->resetMatrix();
    this->translate(oldMatrix.dx(), oldMatrix.dy());
    this->scale(newScale, newScale);
}


//! Resets zoom factor to 100%.
//! @see zoomIn()
//! @see zoomOut()
void GraphicsView::resetZoom()
{
    this->resetMatrix();
}


//! Increases zoom factor by 15%.
//! @see resetZoom()
//! @see zoomOut()
void GraphicsView::zoomIn()
{
    this->scale(1.15, 1.15);
}


//! Decreases zoom factor by 13.04% (1 - 1/1.15).
//! @see resetZoom()
//! @see zoomIn()
void GraphicsView::zoomOut()
{
    this->scale(1/1.15, 1/1.15);
}


//! Hides all component names.
//! @see showNames()
void GraphicsView::hideNames()
{
    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->hideName();
    }
}


//! Shows all component names.
//! @see hideNames()
void GraphicsView::showNames()
{
    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->showName();
    }
}


//! Slot that sets hide ports flag to true
//! @see unHidePorts()
void GraphicsView::hidePorts(bool doIt)
{
    mPortsHidden = doIt;
}


//! Slot that tells the undoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void GraphicsView::undo()
{
    undoStack->undoOneStep();
}


//! Slot that tells the undoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void GraphicsView::redo()
{
    undoStack->redoOneStep();
}

//! Slot that tells the undoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void GraphicsView::clearUndo()
{
    undoStack->clear();
}


//! Get function for primary pen style
QPen GraphicsView::getPen(QString situation, QString type, QString style)
{
    if(situation == "Primary")
    {
        if(type == "Power")
        {
            if(style == "Iso")
                return mPrimaryPenPowerIso;
            if(style == "User")
                return mPrimaryPenPowerUser;
        }
        if(type == "Signal")
        {
            if(style == "Iso")
                return mPrimaryPenSignalIso;
            if(style == "User")
                return mPrimaryPenSignalUser;
        }
    }
    else if(situation == "Active")
    {
        if(type == "Power")
        {
            if(style == "Iso")
                return mActivePenPowerIso;
            if(style == "User")
                return mActivePenPowerUser;
        }
        if(type == "Signal")
        {
            if(style == "Iso")
                return mActivePenSignalIso;
            if(style == "User")
                return mActivePenSignalUser;
        }
    }
    else if(situation == "Hover")
    {
        if(type == "Power")
        {
            if(style == "Iso")
                return mHoverPenPowerIso;
            if(style == "User")
                return mHoverPenPowerUser;
        }
        if(type == "Signal")
        {
            if(style == "Iso")
                return mHoverPenSignalIso;
            if(style == "User")
                return mHoverPenSignalUser;
        }
    }
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

    MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;
    //MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent->parent()->parent())); //Ugly!!!
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    //Core interaction
    mpCoreComponentSystem = mpParentProjectTabWidget->mpHopsanCore->CreateComponentSystem();
    mpCoreComponentSystem->setDesiredTimestep(.001);
    mpCoreComponentSystem->setTypeCQS("S");
    emit checkMessages();
    //

    double timeStep = mpCoreComponentSystem->getDesiredTimeStep();
    mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->setTimeStepLabel(timeStep);

    mIsSaved = true;
    mModelFileName.clear();

    mpGraphicsScene = new GraphicsScene(this);
    mpGraphicsView  = new GraphicsView(mpParentProjectTabWidget->mpHopsanCore, mpCoreComponentSystem, this);

    mpGraphicsView->setScene(mpGraphicsScene);

    QVBoxLayout *tabLayout = new QVBoxLayout;

//    //BE donar med detta lite
//    QTextEdit *texttmp = new QTextEdit("dfsdf",this);
//    texttmp->setTextInteractionFlags(Qt::NoTextInteraction);
//    tabLayout->addWidget(texttmp);
//    //

    tabLayout->addWidget(mpGraphicsView);

   //    addStretch(1);

//    setWindowModified(true);

    setLayout(tabLayout);

    this->useIsoGraphics = false;

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
    mpHopsanCore = HopsanEssentials::getInstance();

    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent)); //Ugly!!!
    pMainWindow->mpMessageWidget->setHopsanCorePtr(mpHopsanCore);
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

    connect(pMainWindow->newAction, SIGNAL(triggered()), this,SLOT(addNewProjectTab()));
    connect(pMainWindow->openAction, SIGNAL(triggered()), this,SLOT(loadModel()));
    connect(pMainWindow->saveAction, SIGNAL(triggered()), this,SLOT(saveProjectTab()));
    connect(pMainWindow->saveAsAction, SIGNAL(triggered()), this,SLOT(saveProjectTabAs()));
    connect(pMainWindow->simulateAction, SIGNAL(triggered()), this,SLOT(simulateCurrent()));
    connect(pMainWindow->resetZoomAction, SIGNAL(triggered()),this,SLOT(resetZoom()));
    connect(pMainWindow->zoomInAction, SIGNAL(triggered()),this,SLOT(zoomIn()));
    connect(pMainWindow->zoomOutAction, SIGNAL(triggered()),this,SLOT(zoomOut()));
    connect(pMainWindow->hideNamesAction,SIGNAL(triggered()),this, SLOT(hideNames()));
    connect(pMainWindow->showNamesAction,SIGNAL(triggered()),this, SLOT(showNames()));

}


//! Returns a pointer to the currently active project tab
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

    newTab->mpCoreComponentSystem->setName(tabName.toStdString());

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

    double *pCoreComponentTime = pCurrentTab->mpCoreComponentSystem->getTimePtr();
    QString timeTxt;
    double dt = finishTime - startTime;
    size_t nSteps = dt/pCurrentTab->mpCoreComponentSystem->getDesiredTimeStep();

    QProgressDialog progressBar(tr("Initialize simulation..."), tr("&Abort initialization"), 0, 0, this);
    std::cout << progressBar.geometry().width() << " " << progressBar.geometry().height() << std::endl;
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Simulate!"));

    InitializationThread actualInitialization(pCurrentTab->mpCoreComponentSystem, startTime, finishTime, this);
    size_t i=0;
    actualInitialization.start();
    actualInitialization.setPriority(QThread::TimeCriticalPriority);
    while (actualInitialization.isRunning())
    {
        progressBar.setValue(i++);
        if (progressBar.wasCanceled())
        {
            pCurrentTab->mpCoreComponentSystem->stop();
        }
    }
    progressBar.setValue(i);
    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation

    if (!progressBar.wasCanceled())
    {
        SimulationThread actualSimulation(pCurrentTab->mpCoreComponentSystem, startTime, finishTime, this);
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
                pCurrentTab->mpCoreComponentSystem->stop();
            }
        }
        progressBar.setValue((size_t)(*pCoreComponentTime/dt * nSteps));
        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    }

    if (progressBar.wasCanceled())
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(QString::fromStdString(pCurrentTab->mpCoreComponentSystem->getName())).append(tr("' was terminated!"))));
    else
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulated '").append(QString::fromStdString(pCurrentTab->mpCoreComponentSystem->getName())).append(tr("' successfully!"))));

    emit checkMessages();

}

//! Loads a model from a file and opens it in a new project tab.
//! @see saveModel(bool saveAs)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;

    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath() + QString("/../.."),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if (modelFileName.isEmpty())
        return;

    QFile file(modelFileName);   //Create a QFile object
    QFileInfo fileInfo(file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file or not a text file: " + modelFileName;
        return;
    }
    QTextStream inputStream(&file);  //Create a QTextStream object to stream the content of file


    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    pCurrentTab->mModelFileName = modelFileName;
    pCurrentTab->mpGraphicsView->undoStack->newPost();

        //Necessary declarations
    QString inputWord, componentType, componentName, startComponentName, endComponentName, parameterName, startPortName, endPortName, tempString;
    //int length, heigth;
    int posX, posY;
    int nameTextPos;
    qreal rotation;
    double parameterValue;

    while ( !inputStream.atEnd() )
    {
        //Extract first word on line
        inputStream >> inputWord;

        //----------- Create New SubSystem -----------//

        if ( inputWord == "HOPSANGUIVERSION")
        {
            inputStream >> tempString;
            if(tempString > QString(HOPSANGUIVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(tempString < QString(HOPSANGUIVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else if ( inputWord == "HOPSANGUIMODELFILEVERSION")
        {
            inputStream >> tempString;
            if(tempString > QString(HOPSANGUIMODELFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(tempString < QString(HOPSANGUIMODELFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else if ( inputWord == "HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION")
        {
            inputStream >> tempString;
            qDebug() << inputWord << " " << tempString;
            if(tempString > QString(HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(tempString < QString(HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }


        if ( inputWord == "COMPONENT" )
        {
            inputStream >> componentType;
            componentName = readName(inputStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
            inputStream >> posX;
            inputStream >> posY;
            inputStream >> rotation;
            inputStream >> nameTextPos;

            //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
            AppearanceData appearanceData = *mpParentMainWindow->mpLibrary->getAppearanceData(componentType);
            pCurrentTab->mpGraphicsView->addGUIObject(componentType, appearanceData, QPoint(posX, posY), 0, componentName);
            pCurrentTab->mpGraphicsView->getGUIObject(componentName)->setNameTextPos(nameTextPos);
            while(pCurrentTab->mpGraphicsView->getGUIObject(componentName)->rotation() != rotation)
            {
                pCurrentTab->mpGraphicsView->getGUIObject(componentName)->rotate();
            }

        }


        if ( inputWord == "PARAMETER" )
        {
            componentName = readName(inputStream);
            inputStream >> parameterName;
            inputStream >> parameterValue;

            pCurrentTab->mpGraphicsView->mGUIObjectMap.find(componentName).value()->setParameter(parameterName, parameterValue);
        }


        if ( inputWord == "CONNECT" )
        {

            GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
            startComponentName = readName(inputStream);
            startPortName = readName(inputStream);
            endComponentName = readName(inputStream);
            endPortName = readName(inputStream);
            GUIPort *startPort = pCurrentView->getGUIObject(startComponentName)->getPort(startPortName);
            GUIPort *endPort = pCurrentView->getGUIObject(endComponentName)->getPort(endPortName);

            bool success = pCurrentView->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort); //This is core access
            if (!success)
            {
                qDebug() << "Unsuccessful connection try" << endl;
            }
            else
            {

                QVector<QPointF> tempPointVector;
                qreal tempX, tempY;

                QString restOfLineString = inputStream.readLine();
                QTextStream restOfLineStream(&restOfLineString);
                while( !restOfLineStream.atEnd() )
                {
                    restOfLineStream >> tempX;
                    restOfLineStream >> tempY;
                    tempPointVector.push_back(QPointF(tempX, tempY));
                }

                //! @todo: Store useIso bool in model file and pick the correct line styles when loading
                GUIConnector *pTempConnector;

                //! @todo Avoid core access here, shoudl make this a function that can be called later if we change iso/user graphics mode
                QString type, style;
                if((startPort->mpCorePort->getNodeType() == "NodeHydraulic") || (startPort->mpCorePort->getNodeType() == "NodeMechanic"))
                    type = "Power";
                else if(startPort->mpCorePort->getNodeType() == "NodeSignal")
                    type = "Signal";
                if(pCurrentTab->useIsoGraphics)
                    style = "Iso";
                else
                    style = "User";
                pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, pCurrentView->getPen("Primary", type, style),
                                                  pCurrentView->getPen("Active", type, style), pCurrentView->getPen("Hover", type, style), pCurrentView);

                pCurrentView->scene()->addItem(pTempConnector);

                //Hide connected ports
                startPort->hide();
                endPort->hide();

                connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
                connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

                pCurrentView->mConnectorVector.append(pTempConnector);
            }
        }
    }
    //Deselect all comonents
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
    QMap<QString, GUIObject *>::iterator it;
    for(it = pCurrentView->mGUIObjectMap.begin(); it!=pCurrentView->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(false);
    }

    //Sets the file name as model name
    pCurrentView->getModelPointer()->setName(fileInfo.fileName().toStdString()); //! @todo This is core access maybe should try to access some other way

    pCurrentView->undoStack->clear();

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

    QFile file(modelFileName);   //Create a QFile object
    QFileInfo fileInfo(file);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " + modelFileName;
        return;
    }
    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file

    modelFile << "--------------------------------------------------------------\n";
    modelFile << "-------------------  HOPSAN NG MODEL FILE  -------------------\n";
    modelFile << "--------------------------------------------------------------\n";
    modelFile << "HOPSANGUIVERSION " << HOPSANGUIVERSION << "\n";
    modelFile << "HOPSANGUIMODELFILEVERSION " << HOPSANGUIMODELFILEVERSION << "\n";
    modelFile << "HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION " << HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION << "\n";
    modelFile << "--------------------------------------------------------------\n";

    QMap<QString, GUIObject*>::iterator it;
    for(it = pCurrentView->mGUIObjectMap.begin(); it!=pCurrentView->mGUIObjectMap.end(); ++it)
    {
//        QPointF pos = it.value()->mapToScene(it.value()->boundingRect().center());
//        modelFile << "COMPONENT " << it.value()->getTypeName() << " " << addQuotes(it.value()->getName()) << " "
//                  << pos.x() << " " << pos.y() << " " << it.value()->rotation() << " " << it.value()->getNameTextPos() << "\n";
//
//        //! @todo wrap this in the gui object (don wnat to access core directly here)
//        Component *mpCoreComponent = it.value()->getHopsanCoreComponentPtr();
//        vector<CompParameter> paramVector = mpCoreComponent->getParameterVector();
//        std::vector<CompParameter>::iterator itp;
//        for ( itp=paramVector.begin() ; itp !=paramVector.end(); ++itp )
//        {
//            modelFile << "PARAMETER " << addQuotes(it.key()) << " " << QString::fromStdString(itp->getName()) << " " << itp->getValue() << "\n";
//            //qDebug() << it.key() << " - " << itp->getName().c_str() << " - " << itp->getValue();
//        }
        it.value()->saveToTextStream(modelFile);

    }

    modelFile << "--------------------------------------------------------------\n";

   // QMap<QString, GUIConnector *>::iterator it2;
    for(int i = 0; i != pCurrentView->mConnectorVector.size(); ++i)
    {
//        QString startPortName  = pCurrentView->mConnectorVector[i]->getStartPort()->getName();
//        QString endPortName = pCurrentView->mConnectorVector[i]->getEndPort()->getName();
//        modelFile << "CONNECT " << QString(addQuotes(pCurrentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName()) + " " + addQuotes(startPortName) + " " +
//                                           addQuotes(pCurrentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName()) + " " + addQuotes(endPortName));
//        for(size_t j = 0; j != pCurrentView->mConnectorVector[i]->getPointsVector().size(); ++j)
//        {
//            modelFile << " " << pCurrentView->mConnectorVector[i]->getPointsVector()[j].x() << " " << pCurrentView->mConnectorVector[i]->getPointsVector()[j].y();
//        }
//        modelFile << "\n";
        pCurrentView->mConnectorVector[i]->saveToTextStream(modelFile);
    }
    modelFile << "--------------------------------------------------------------\n";

    //Sets the model name
    pCurrentTab->mpCoreComponentSystem->setName(fileInfo.fileName().toStdString()); //!< @todo BAD should not be core access here, find some other way to encapsule
    this->setTabText(this->currentIndex(), fileInfo.fileName());
}


//! Tells the current tab to change to or from ISO graphics.
//! @param value is true if ISO should be activated and false if it should be deactivated.
void ProjectTabWidget::setIsoGraphics(bool value)
{
    this->getCurrentTab()->useIsoGraphics = value;

    ProjectTab *pCurrentTab = getCurrentTab();
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
    QMap<QString, GUIConnector *>::iterator it;
    for(size_t i = 0; i!=pCurrentView->mConnectorVector.size(); ++i)
    {
        if(value)
        {
            if((pCurrentView->mConnectorVector[i]->getEndPort()->mpCorePort->getNodeType() == "NodeHydraulic") | (pCurrentView->mConnectorVector[i]->getEndPort()->mpCorePort->getNodeType() == "NodeMechanic"))
                pCurrentView->mConnectorVector[i]->setPens(pCurrentView->getPen("Primary", "Power", "Iso"),
                                                           pCurrentView->getPen("Active", "Power", "Iso"),
                                                           pCurrentView->getPen("Hover", "Power", "Iso"));
            else if(pCurrentView->mConnectorVector[i]->getEndPort()->mpCorePort->getNodeType() == "NodeSignal")
                    pCurrentView->mConnectorVector[i]->setPens(pCurrentView->getPen("Primary", "Signal", "Iso"),
                                                               pCurrentView->getPen("Active", "Signal", "Iso"),
                                                               pCurrentView->getPen("Hover", "Signal", "Iso"));
        }
        else
        {
            if((pCurrentView->mConnectorVector[i]->getEndPort()->mpCorePort->getNodeType() == "NodeHydraulic") | (pCurrentView->mConnectorVector[i]->getEndPort()->mpCorePort->getNodeType() == "NodeMechanic"))
                pCurrentView->mConnectorVector[i]->setPens(pCurrentView->getPen("Primary", "Power", "User"),
                                                           pCurrentView->getPen("Active", "Power", "User"),
                                                           pCurrentView->getPen("Hover", "Power", "User"));
            else if(pCurrentView->mConnectorVector[i]->getEndPort()->mpCorePort->getNodeType() == "NodeSignal")
                pCurrentView->mConnectorVector[i]->setPens(pCurrentView->getPen("Primary", "Signal", "User"),
                                                           pCurrentView->getPen("Active", "Signal", "User"),
                                                           pCurrentView->getPen("Hover", "Signal", "User"));
        }
    }

    QMap<QString, GUIObject*>::iterator it2;
    for(it2 = pCurrentView->mGUIObjectMap.begin(); it2!=pCurrentView->mGUIObjectMap.end(); ++it2)
    {
        it2.value()->setIcon(value);
    }
}


//! Tells the current tab to reset zoom to 100%.
//! @see zoomIn()
//! @see zoomOut()
void ProjectTabWidget::resetZoom()
{
    this->getCurrentTab()->mpGraphicsView->resetZoom();
}


//! Tells the current tab to increase its zoom factor.
//! @see resetZoom()
//! @see zoomOut()
void ProjectTabWidget::zoomIn()
{
    this->getCurrentTab()->mpGraphicsView->zoomIn();
}


//! Tells the current tab to decrease its zoom factor.
//! @see resetZoom()
//! @see zoomIn()
void ProjectTabWidget::zoomOut()
{
    this->getCurrentTab()->mpGraphicsView->zoomOut();
}


//! Tells the current tab to hide all component names.
//! @see showNames()
void ProjectTabWidget::hideNames()
{
    this->getCurrentTab()->mpGraphicsView->hideNames();
}


//! Tells the current tab to show all component names.
//! @see hideNames()
void ProjectTabWidget::showNames()
{
    this->getCurrentTab()->mpGraphicsView->showNames();
}
