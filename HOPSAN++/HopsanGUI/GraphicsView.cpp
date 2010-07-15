#include "GraphicsView.h"
#include "GUIUtilities.h"

#include "GUIObject.h"
#include "GUIPort.h"
#include "UndoStack.h"
#include "GUIConnector.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "MessageWidget.h"
#include "LibraryWidget.h"

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

//! @breif dont really know what this is used for
//! @todo Ok this does not seem to refresh the view at all, but maybe some parts of the view, dont know realy
void GraphicsView::resetBackgroundBrush()
{
    this->setBackgroundBrush(mBackgroundColor);
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
        //Here we set A0, Landscape and Fullpage among other things to make sure that components get large enough to be treeted as vector graphics
        //Some bug or "feature" makes small objects be converted to bitmaps (ugly)
        //! @todo Try to find out why this happens (se comment above)
        QPrinter *printer = new QPrinter(QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A0);
        printer->setOrientation(QPrinter::Landscape);
        printer->setFullPage(true);
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(fileName);
        QPainter *painter = new QPainter(printer);
        this->render(painter);
        painter->end();
    }
}

