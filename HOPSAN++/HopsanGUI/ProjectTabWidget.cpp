/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$


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
#include <QMap>

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
GraphicsView::GraphicsView(ProjectTab *parent)
        : QGraphicsView(parent)
{
    mpParentProjectTab = parent;


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

    mCtrlKeyPressed = false;
    mIsRenamingObject = false;

    mZoomFactor = 1.0;

    mpCopyData = new QString;

    undoStack = new UndoStack(this);
    //undoStack->show();

    MainWindow *pMainWindow = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(this->systemPortAction, SIGNAL(triggered()), SLOT(addSystemPort()));
    connect(pMainWindow->cutAction, SIGNAL(triggered()), this,SLOT(cutSelected()));
    connect(pMainWindow->copyAction, SIGNAL(triggered()), this,SLOT(copySelected()));
    connect(pMainWindow->pasteAction, SIGNAL(triggered()), this,SLOT(paste()));
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


//! Defines the right click menu event
void GraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
    if(!mIsCreatingConnector and !mJustStoppedCreatingConnector)
    {
        if (QGraphicsItem *item = itemAt(event->pos()))
            QGraphicsView::contextMenuEvent(event);
        // Context menu when right-clicking:
//        else
//        {
//            QMenu menu(this);
//            menu.addMenu(menuInsert);
//            menu.exec(event->globalPos());
//        }
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
        mpParentProjectTab->hasChanged();

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
        mpTempGUIObject= new GUISubsystem(appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }
    else if (componentTypeName == "SystemPort")
    {
        mpTempGUIObject = new GUISystemPort(appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }
    else //Assume some standard component type
    {
        mpTempGUIObject = new GUIComponent(appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }

    qDebug() << "The name: " <<  name;
    //qDebug() << "=====================Get initial name: " << mpTempGUIObject->getName() << "requested: " << name;
    if (!name.isEmpty())
    {
        qDebug() << "name not empty, setting to: " << name;
        //Set name, do NOT try to do smart rename. (If component already exist with new component default name that other component would be renamed)
        mpTempGUIObject->setName(name, true);
    }

    mpTempGUIObject->refreshDisplayName();
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


//! @brief A function that ads a system port to the current system
void GraphicsView::addSystemPort()
{
    //qDebug() <<"Adding a system port";
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
    //qDebug() << "deleteGUIObject()";
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
        //qDebug() << "GUI rename: " << oldName << " " << obj_ptr->getName();
    }
    else
    {
        //qDebug() << "Old name: " << oldName << " not found";
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
    qreal wheelDelta;
    if(this->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mInvertWheel)
    {
        wheelDelta = event->delta();
    }
    else
    {
        wheelDelta = -event->delta();
    }

    if (event->modifiers().testFlag(Qt::ControlModifier) or event->modifiers().testFlag(Qt::AltModifier))
    {
        qreal factor = pow(1.41,(-wheelDelta/240.0));
        this->scale(factor,factor);
        mZoomFactor = mZoomFactor * factor;
        emit zoomChange();
    }
    else if(event->modifiers().testFlag(Qt::ShiftModifier))
    {

        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()-wheelDelta);
    }
    else
    {
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value()+wheelDelta);
    }
}


//! Defines what shall happen when various keys or key combinations are pressed.
//! @param event contains information about the key press event.
void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    bool doNotForwardEvent = false;
    bool ctrlPressed = event->modifiers().testFlag(Qt::ControlModifier);
    bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
    bool altPressed = event->modifiers().testFlag(Qt::AltModifier);

    if (event->key() == Qt::Key_Delete and !mIsRenamingObject)
    {
        if(isObjectSelected() or isConnectorSelected())
        {
            undoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressDelete();
    }
    else if (ctrlPressed and event->key() == Qt::Key_R and !mIsRenamingObject)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlR();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        if(this->mIsCreatingConnector)
        {
            delete(mpTempConnector);
            mIsCreatingConnector = false;
        }
    }
    else if(shiftPressed and event->key() == Qt::Key_K and !mIsRenamingObject)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressShiftK();
    }
    else if(shiftPressed and event->key() == Qt::Key_L and !mIsRenamingObject)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressShiftL();
    }
    else if(ctrlPressed and event->key() == Qt::Key_Up)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressCtrlUp();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed and event->key() == Qt::Key_Down)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlDown();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed and event->key() == Qt::Key_Left and !mIsRenamingObject)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
        }
        emit keyPressCtrlLeft();
        doNotForwardEvent = true;
    }
    else if(ctrlPressed and event->key() == Qt::Key_Right and !mIsRenamingObject)
    {
        if(isObjectSelected())
        {
            undoStack->newPost();
            mpParentProjectTab->hasChanged();
        }
        emit keyPressCtrlRight();
        doNotForwardEvent = true;
    }
    else if (ctrlPressed and event->key() == Qt::Key_A and !mIsRenamingObject)
    {
        this->selectAll();
    }
    else if (ctrlPressed)
    {
        if (this->mIsCreatingConnector and !mpTempConnector->isMakingDiagonal())
        {
            mpTempConnector->makeDiagonal(true);
            mpTempConnector->drawConnector();
            this->setBackgroundBrush(mBackgroundColor);
        }
        else
        {
            mCtrlKeyPressed = true;
            this->setDragMode(RubberBandDrag);
        }
    }

    if(!doNotForwardEvent)
    {
        QGraphicsView::keyPressEvent ( event );
    }
}


//! Defines what shall happen when a key is released.
//! @param event contains information about the keypress operation.
void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    //qDebug() << "keyReleaseEvent";
        // Releasing ctrl key while creating a connector means return from diagonal mode to orthogonal mode.
    if(event->key() == Qt::Key_Control and mIsCreatingConnector)
    {
        mpTempConnector->makeDiagonal(false);
        mpTempConnector->drawConnector();
        this->setBackgroundBrush(mBackgroundColor);
    }

    if(event->key() == Qt::Key_Control)
    {
        mCtrlKeyPressed = false;
        this->setDragMode(RubberBandDrag);
    }

    QGraphicsView::keyReleaseEvent(event);
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

    //No rubber band during connecting:
    if (this->mIsCreatingConnector)
    {
        this->setDragMode(NoDrag);
    }
    else if(mCtrlKeyPressed)
    {
        this->setDragMode(ScrollHandDrag);
    }
    else
    {
        this->setDragMode(RubberBandDrag);
    }

    if (event->button() == Qt::RightButton and this->mIsCreatingConnector)
    {
        if((mpTempConnector->getNumberOfLines() == 1 and mpTempConnector->isMakingDiagonal()) or (mpTempConnector->getNumberOfLines() == 2 and !mpTempConnector->isMakingDiagonal()))
        {
            mpTempConnector->getStartPort()->isConnected = false;
            mpTempConnector->getStartPort()->show();
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
        //qDebug() << "mIsCreatingConnector = " << mIsCreatingConnector;
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

        GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(pPort->getPortType(), this->mpParentProjectTab->useIsoGraphics);
        mpTempConnector = new GUIConnector(oldPos, pConnApp, this);

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
        GUIPort *pStartPort = mpTempConnector->getStartPort();

        bool success = mpParentProjectTab->mGUIRootSystem.connect(pStartPort->getGUIComponentName(), pStartPort->getName(), pPort->getGUIComponentName(), pPort->getName() );
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

            undoStack->newPost();
            mpParentProjectTab->hasChanged();
            if(!doNotRegisterUndo)
            {
                undoStack->registerAddedConnector(mpTempConnector);
            }
        }
        emit checkMessages();
     }
}


//! Removes the connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param doNotRegisterUndo is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void GraphicsView::removeConnector(GUIConnector* pConnector, bool doNotRegisterUndo)
{
    bool doDelete = false;
    bool startPortHasMoreConnections = false;
    bool endPortWasConnected = false;
    bool endPortHasMoreConnections = false;
    int indexToRemove;
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
             if(pConnector->isConnected())
             {
                 GUIPort *pStartP = pConnector->getStartPort();
                 GUIPort *pEndP = pConnector->getEndPort();
                 mpParentProjectTab->mGUIRootSystem.disconnect(pStartP->getGUIComponentName(), pStartP->getName(), pEndP->getGUIComponentName(), pEndP->getName());
                 emit checkMessages();
                 endPortWasConnected = true;
             }
             doDelete = true;
             indexToRemove = i;
        }
        else if( (pConnector->getStartPort() == mConnectorVector[i]->getStartPort()) or
                 (pConnector->getStartPort() == mConnectorVector[i]->getEndPort()) )
        {
            startPortHasMoreConnections = true;
        }
        else if( (pConnector->getEndPort() == mConnectorVector[i]->getStartPort()) or
                 (pConnector->getEndPort() == mConnectorVector[i]->getEndPort()) )
        {
            endPortHasMoreConnections = true;
        }
        if(mConnectorVector.empty())
            break;
    }

    if(endPortWasConnected and !endPortHasMoreConnections)
    {
        pConnector->getEndPort()->setVisible(!mPortsHidden);
        pConnector->getEndPort()->isConnected = false;
    }

    if(!startPortHasMoreConnections)
    {
        pConnector->getStartPort()->setVisible(!mPortsHidden);
        pConnector->getStartPort()->isConnected = false;
    }
    else if(startPortHasMoreConnections and !endPortWasConnected)
    {
        pConnector->getStartPort()->setVisible(false);
        pConnector->getStartPort()->isConnected = true;
    }

    if(doDelete)
    {
        scene()->removeItem(pConnector);
        delete pConnector;
        mConnectorVector.remove(indexToRemove);
    }
    this->setBackgroundBrush(mBackgroundColor);
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
//! @todo What about paramter values
void GraphicsView::copySelected()
{
    delete(mpCopyData);
    mpCopyData = new QString;

    QTextStream copyStream;
    copyStream.setString(mpCopyData);
    //copyStream.readAll();
    //copyStreamRot.clear();
    //copyStreamPos.clear();

    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        if (it.value()->isSelected())
        {
            it.value()->saveToTextStream(copyStream);
        }
    }

    for(int i = 0; i != mConnectorVector.size(); ++i)
    {
        if(mConnectorVector[i]->getStartPort()->getGuiObject()->isSelected() and mConnectorVector[i]->getEndPort()->getGuiObject()->isSelected() and mConnectorVector[i]->isActive())
        {
            mConnectorVector[i]->saveToTextStream(copyStream);
        }
    }
}


//! Creates each item in the copy stack, and places it on its respective position in the position copy stack.
//! @see cutSelected()
//! @see copySelected()
void GraphicsView::paste()
{
    //qDebug() << "mpCopyData = " << *mpCopyData;

    undoStack->newPost();
    mpParentProjectTab->hasChanged();

    QTextStream copyStream;
    copyStream.setString(mpCopyData);

        //Deselect all components
    QMap<QString, GUIObject*>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(false);
    }

        //Deselect all connectors
    for(int i = 0; i != mConnectorVector.size(); ++i)
    {
        mConnectorVector[i]->doSelect(false, -1);
        mConnectorVector[i]->setPassive();
    }

    QMap<QString, QString> renameMap;       //Used to track name changes, so that connectors will know what components are called
    QString inputWord;
    QString componentName;
    QString componentType;
    QString startComponentName, endComponentName;
    QString startPortName, endPortName;
    QString parameterName;
    qreal parameterValue;

    //! @todo Could we not use some common load function for the stuff bellow

    while ( !copyStream.atEnd() )
    {
        //Extract first word on line
        copyStream >> inputWord;

        if(inputWord == "COMPONENT")
        {
            qreal posX, posY, rotation, nameTextPos;
            componentType = readName(copyStream);
            componentName = readName(copyStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
            copyStream >> posX;
            copyStream >> posY;
            copyStream >> rotation;
            copyStream >> nameTextPos;

            AppearanceData appearanceData = *mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(componentType);
            this->addGUIObject(componentType, appearanceData, QPoint(posX-50, posY-50), rotation, componentName, true);
            mpTempGUIObject->setNameTextPos(nameTextPos);
            renameMap.insert(componentName, mpTempGUIObject->getName());
            mpTempGUIObject->setSelected(true);
        }
        else if(inputWord == "CONNECT")
        {
            startComponentName = renameMap.find(readName(copyStream)).value();
            startPortName = readName(copyStream);
            endComponentName = renameMap.find(readName(copyStream)).value();
            endPortName = readName(copyStream);
            GUIPort *startPort = this->getGUIObject(startComponentName)->getPort(startPortName);
            GUIPort *endPort = this->getGUIObject(endComponentName)->getPort(endPortName);

            bool success = mpParentProjectTab->mGUIRootSystem.connect(startComponentName, startPortName, endComponentName, endPortName);
            if (!success)
            {
                qDebug() << "Unsuccessful connection try" << endl;
            }
            else
            {
                QVector<QPointF> tempPointVector;
                qreal tempX, tempY;

                QString restOfLineString = copyStream.readLine();
                QTextStream restOfLineStream(&restOfLineString);
                while( !restOfLineStream.atEnd() )
                {
                    restOfLineStream >> tempX;
                    restOfLineStream >> tempY;
                    tempPointVector.push_back(QPointF(tempX-50, tempY-50));
                }

                //! @todo: Store useIso bool in model file and pick the correct line styles when loading
                GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(startPort->getPortType(), this->mpParentProjectTab->useIsoGraphics);
                GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, pConnApp, this);

                this->scene()->addItem(pTempConnector);

                //Hide connected ports
                startPort->hide();
                endPort->hide();

                connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
                connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

                this->mConnectorVector.append(pTempConnector);
            }
        }

        else if ( inputWord == "PARAMETER" )
        {
            componentName = renameMap.find(readName(copyStream)).value();
            copyStream >> parameterName;
            copyStream >> parameterValue;

            this->mGUIObjectMap.find(componentName).value()->setParameterValue(parameterName, parameterValue);
        }
    }

        //Select all pasted comonents
    QMap<QString, QString>::iterator itn;
    for(itn = renameMap.begin(); itn != renameMap.end(); ++itn)
    {
        this->mGUIObjectMap.find(itn.value()).value()->setSelected(true);
    }

    this->setBackgroundBrush(mBackgroundColor);
}


//! @todo This is not used anywhere and can probably be removed. Why would you want to do it like this?
//void GraphicsView::setScale(const QString &scale)
//{
//    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
//    QMatrix oldMatrix = this->matrix();
//    this->resetMatrix();
//    this->translate(oldMatrix.dx(), oldMatrix.dy());
//    this->scale(newScale, newScale);
//}


//! Resets zoom factor to 100%.
//! @see zoomIn()
//! @see zoomOut()
void GraphicsView::resetZoom()
{
    this->resetMatrix();
    mZoomFactor = 1.0;
    emit zoomChange();
}


//! Increases zoom factor by 15%.
//! @see resetZoom()
//! @see zoomOut()
void GraphicsView::zoomIn()
{
    this->scale(1.15, 1.15);
    mZoomFactor = mZoomFactor * 1.15;
    emit zoomChange();
}


//! Decreases zoom factor by 13.04% (1 - 1/1.15).
//! @see resetZoom()
//! @see zoomIn()
void GraphicsView::zoomOut()
{
    this->scale(1/1.15, 1/1.15);
    mZoomFactor = mZoomFactor / 1.15;
    emit zoomChange();
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



//! Exports the graphics view to PDF
//! @todo Check if it is possible to export to SVG instead. It appears as it is not possible with the current QT version, but I am not sure.
void GraphicsView::exportPDF()
{
     QString fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", QString(),
        "Adobe PDF Documents (*.pdf)");
    if ( !fileName.isEmpty() )
    {
        QPrinter *printer;
        printer = new QPrinter(QPrinter::HighResolution);
        //printer = new QPrinter();
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(fileName);
        QPainter *painter = new QPainter(printer);
        this->render(painter);
        painter->end();
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
    //connect(this, SIGNAL(changed( const QList<QRectF> & )),this->parent(), SLOT(hasChanged()));
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

    mStartTime = 0;
    mTimeStep = 0.01;
    mStopTime = 5;

    MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    mGUIRootSystem.setDesiredTimeStep(.001);
    mGUIRootSystem.setRootTypeCQS("S");

    emit checkMessages();

    double timeStep = mGUIRootSystem.getDesiredTimeStep();

    mpParentProjectTabWidget->mpParentMainWindow->setTimeStepLabel(timeStep);

    mIsSaved = true;
    mModelFileName.clear();

    mpGraphicsScene = new GraphicsScene(this);

    mpGraphicsView  = new GraphicsView(this);

    mpGraphicsView->setScene(mpGraphicsScene);

    QVBoxLayout *tabLayout = new QVBoxLayout;

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


//! Slot that updates start time value of the current project to the one in the simulation setup widget.
//! @see updateTimeStep()
//! @see updateStopTime()
void ProjectTab::updateStartTime()
{
    mStartTime = mpParentProjectTabWidget->mpParentMainWindow->getStartTimeLabel();
}


//! Slot that updates time step value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateStopTime()
void ProjectTab::updateTimeStep()
{
    mTimeStep = mpParentProjectTabWidget->mpParentMainWindow->getTimeStepLabel();
}


//! Slot that updates stop time value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateTimeStep()
void ProjectTab::updateStopTime()
{
    mStopTime = mpParentProjectTabWidget->mpParentMainWindow->getFinishTimeLabel();
}


//! Returns the start time value of the current project.
//! @see getTimeStep()
//! @see getStopTime()
double ProjectTab::getStartTime()
{
    return mStartTime;
}


//! Returns the time step value of the current project.
//! @see getStartTime()
//! @see getStopTime()
double ProjectTab::getTimeStep()
{
    return mTimeStep;
}


//! Returns the stop time value of the current project.
//! @see getStartTime()
//! @see getTimeStep()
double ProjectTab::getStopTime()
{
    return mStopTime;
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
    //MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent)); //Ugly!!!

    connect(this, SIGNAL(checkMessages()), mpParentMainWindow->mpMessageWidget, SLOT(checkMessages()));

    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

    connect(this,SIGNAL(currentChanged(int)),this, SLOT(updateSimulationSetupWidget()));

    connect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentStartTime()));
    connect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentTimeStep()));
    connect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentStopTime()));
    connect(mpParentMainWindow->hidePortsAction, SIGNAL(triggered(bool)), this,SLOT(hidePortsInCurrentTab(bool)));

    connect(mpParentMainWindow->newAction, SIGNAL(triggered()), this,SLOT(addNewProjectTab()));
    connect(mpParentMainWindow->openAction, SIGNAL(triggered()), this,SLOT(loadModel()));
    connect(mpParentMainWindow->saveAction, SIGNAL(triggered()), this,SLOT(saveProjectTab()));
    connect(mpParentMainWindow->exportPDFAction, SIGNAL(triggered()), this,SLOT(exportCurrentToPDF()));
    connect(mpParentMainWindow->saveAsAction, SIGNAL(triggered()), this,SLOT(saveProjectTabAs()));
    connect(mpParentMainWindow->simulateAction, SIGNAL(triggered()), this,SLOT(simulateCurrent()));
    connect(mpParentMainWindow->resetZoomAction, SIGNAL(triggered()),this,SLOT(resetZoom()));
    connect(mpParentMainWindow->zoomInAction, SIGNAL(triggered()),this,SLOT(zoomIn()));
    connect(mpParentMainWindow->zoomOutAction, SIGNAL(triggered()),this,SLOT(zoomOut()));
    connect(mpParentMainWindow->hideNamesAction,SIGNAL(triggered()),this, SLOT(hideNames()));
    connect(mpParentMainWindow->showNamesAction,SIGNAL(triggered()),this, SLOT(showNames()));
}


//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.
void ProjectTabWidget::exportCurrentToPDF()
{
    getCurrentTab()->mpGraphicsView->exportPDF();
}


//! Slot that tells the current project tab to hide its ports.
//! @param doIt is true if ports shall be hidden, otherwise false.
void ProjectTabWidget::hidePortsInCurrentTab(bool doIt)
{
    this->getCurrentTab()->mpGraphicsView->hidePorts(doIt);
}


//! Slot that tells current project tab to update its start time value.
//! @see updateCurrentTimeStep()
//! @see updateCurrentStopTime()
void ProjectTabWidget::updateCurrentStartTime()
{
    getCurrentTab()->updateStartTime();
}


//! Slot that tells current project tab to update its time step value.
//! @see updateCurrentStartTime()
//! @see updateCurrentStopTime()
void ProjectTabWidget::updateCurrentTimeStep()
{
    getCurrentTab()->updateTimeStep();
}


//! Slot that tells current project tab to update its stop time value.
//! @see updateCurrentStartTime()
//! @see updateCurrentTimeStep()
void ProjectTabWidget::updateCurrentStopTime()
{
    getCurrentTab()->updateStopTime();
}


//! Slot that updates the values in the simulation setup widget to display new values when current project tab is changed.
void ProjectTabWidget::updateSimulationSetupWidget()
{
    if(this->count() != 0)  //Don't do anything if there are no current tab
    {
        mpParentMainWindow->setStartTimeLabel(getCurrentTab()->getStartTime());
        mpParentMainWindow->setTimeStepLabel(getCurrentTab()->getTimeStep());
        mpParentMainWindow->setFinishTimeLabel(getCurrentTab()->getStopTime());
    }
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
    //newTab->mIsSaved = false;

    newTab->mGUIRootSystem.setRootSystemName(tabName);

    //addTab(newTab, tabName.append(QString("*")));
    addTab(newTab, tabName);
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

    if(!this->getCurrentTab()->mGUIRootSystem.isSimulationOk())
    {
        mpParentMainWindow->mpMessageWidget->printCoreMessages();
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("Simulation failed"));
        return;
    }


    ProjectTab *pCurrentTab = getCurrentTab();

    double startTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->getStartTimeLabel();
    double finishTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->getFinishTimeLabel();
    
    QString timeTxt;
    double dt = finishTime - startTime;
    size_t nSteps = dt/pCurrentTab->mGUIRootSystem.getDesiredTimeStep();

    QProgressDialog progressBar(tr("Initialize simulation..."), tr("&Abort initialization"), 0, 0, this);
    std::cout << progressBar.geometry().width() << " " << progressBar.geometry().height() << std::endl;
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Simulate!"));

    InitializationThread actualInitialization(&(pCurrentTab->mGUIRootSystem), startTime, finishTime, this);

    size_t i=0;
    actualInitialization.start();
//    actualInitialization.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio
    actualInitialization.setPriority(QThread::HighestPriority);
    while (actualInitialization.isRunning())
    {
        progressBar.setValue(i++);
        if (progressBar.wasCanceled())
        {
            pCurrentTab->mGUIRootSystem.stop();
        }
    }
    progressBar.setValue(i);
    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation

    if (!progressBar.wasCanceled())
    {
        SimulationThread actualSimulation(&(pCurrentTab->mGUIRootSystem), startTime, finishTime, this);
        actualSimulation.start();
//        actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio
        actualSimulation.setPriority(QThread::HighestPriority);
        progressBar.setLabelText(tr("Running simulation..."));
        progressBar.setCancelButtonText(tr("&Abort simulation"));
        progressBar.setMinimum(0);
        progressBar.setMaximum(nSteps);
        while (actualSimulation.isRunning())
        {
            progressBar.setValue((size_t)(getCurrentTab()->mGUIRootSystem.getCurrentTime()/dt * nSteps));
            if (progressBar.wasCanceled())
            {
                pCurrentTab->mGUIRootSystem.stop();
            }
        }
        progressBar.setValue((size_t)(getCurrentTab()->mGUIRootSystem.getCurrentTime()/dt * nSteps));
        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    }

    if (progressBar.wasCanceled())
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(pCurrentTab->mGUIRootSystem.getName()).append(tr("' was terminated!"))));
    else
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulated '").append(pCurrentTab->mGUIRootSystem.getName()).append(tr("' successfully!"))));

    emit checkMessages();
}

//! Loads a model from a file and opens it in a new project tab.
//! @see saveModel(bool saveAs)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath() + QString("/../../Models"),
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
    pCurrentTab->mIsSaved = true;

        //Necessary declarations
    QString inputWord, componentType, componentName, startComponentName, endComponentName, parameterName, startPortName, endPortName, tempString;
    //int length, heigth;
    qreal posX, posY;
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

        if ( inputWord == "STARTTIME" )
        {
            double startTime;
            inputStream >> startTime;
            mpParentMainWindow->setStartTimeLabel(startTime);
        }

        if ( inputWord == "TIMESTEP" )
        {
            double timeStep;
            inputStream >> timeStep;
            mpParentMainWindow->setTimeStepLabel(timeStep);
        }

        if ( inputWord == "FINISHTIME" )
        {
            double finishTime;
            inputStream >> finishTime;
            mpParentMainWindow->setFinishTimeLabel(finishTime);
        }

        if ( inputWord == "COMPONENT" )
        {
            componentType = readName(inputStream);
            componentName = readName(inputStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
            inputStream >> posX;
            inputStream >> posY;
            inputStream >> rotation;
            inputStream >> nameTextPos;

            //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
            //qDebug() << "componentType: " << componentType;
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

            //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
            pCurrentTab->mpGraphicsView->mGUIObjectMap.find(componentName).value()->setParameterValue(parameterName, parameterValue);
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

            bool success = pCurrentTab->mGUIRootSystem.connect(startComponentName, startPortName, endComponentName, endPortName);
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
                GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(startPort->getPortType(), pCurrentTab->useIsoGraphics);
                GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, pConnApp, pCurrentView);

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
    getCurrentTab()->mGUIRootSystem.setRootSystemName(fileInfo.fileName());

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
                                                             fileDialogSaveDir.currentPath() + QString("/../../Models"),
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

    modelFile << "STARTTIME " << mpParentMainWindow->getStartTimeLabel() << "\n";
    modelFile << "TIMESTEP " << mpParentMainWindow->getTimeStepLabel() << "\n";
    modelFile << "FINISHTIME " << mpParentMainWindow->getFinishTimeLabel() << "\n";
    modelFile << "--------------------------------------------------------------\n";

    QMap<QString, GUIObject*>::iterator it;
    for(it = pCurrentView->mGUIObjectMap.begin(); it!=pCurrentView->mGUIObjectMap.end(); ++it)
    {
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
    pCurrentTab->mGUIRootSystem.setRootSystemName(fileInfo.fileName());
    this->setTabText(this->currentIndex(), fileInfo.fileName());
}


//! Tells the current tab to change to or from ISO graphics.
//! @param value is true if ISO should be activated and false if it should be deactivated.
//! @todo Break out the guiconnector appearance stuff into a separate general function
void ProjectTabWidget::setIsoGraphics(bool useISO)
{
    this->getCurrentTab()->useIsoGraphics = useISO;

    mpParentMainWindow->mpLibrary->useIsoGraphics(useISO);

    ProjectTab *pCurrentTab = getCurrentTab();
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
    //QMap<QString, GUIConnector *>::iterator it;

    for(int i = 0; i!=pCurrentView->mConnectorVector.size(); ++i)
    {
        pCurrentView->mConnectorVector[i]->setIsoStyle(useISO);
    }

    QMap<QString, GUIObject*>::iterator it2;
    for(it2 = pCurrentView->mGUIObjectMap.begin(); it2!=pCurrentView->mGUIObjectMap.end(); ++it2)
    {
        it2.value()->setIcon(useISO);
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
