//$Id$

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "GUIObject.h"
#include "GUISystem.h"
//#include "GUIComponent.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
//#include "ParameterDialog.h"
//#include "GUIPort.h"
//#include "GUIConnector.h"
#include "GUIUtilities.h"
#include "UndoStack.h"
#include "MessageWidget.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "LibraryWidget.h"
//#include "loadObjects.h"

//using namespace std;



//! @todo should not pSystem and pParent be teh same ?
GUIObject::GUIObject(QPoint pos, qreal rot, selectionStatus, GUISystem *pSystem, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent)
{
    //Initi variables
    mHmfTagName = HMF_OBJECTTAG;
    mpSelectionBox = 0;

    mpParentSystem = pSystem;
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);

    //Set position orientation and other appearance stuff
    this->setCenterPos(pos);
    this->rotateTo(rot);
    this->setAcceptHoverEvents(true);
    mIsFlipped = false;
}


//! @brief Destructor for GUI Objects
GUIObject::~GUIObject()
{

}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int GUIObject::type() const
{
    return Type;
}

QPointF GUIObject::getCenterPos()
{
    return QPointF(this->pos().x()+this->boundingRect().width()/2.0, this->pos().y()+this->boundingRect().height()/2.0);
}

void GUIObject::setCenterPos(QPointF pos)
{
    this->setPos(pos.x()-this->boundingRect().width()/2.0, pos.y()-this->boundingRect().height()/2.0);
}


//! @brief Slot that deselects the object
void GUIObject::deselect()
{
    this->setSelected(false);
}


//! @brief Slot that selects the object
void GUIObject::select()
{
    this->setSelected(true);
}


//! @brief Defines what happens when mouse starts hovering the object
void GUIObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setHovered();
    }

    QGraphicsWidget::hoverEnterEvent(event);
}


//! @bried Defines what happens when mouse stops hovering the object
void GUIObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setPassive();
    }

    QGraphicsWidget::hoverLeaveEvent(event);
}


//! @brief Defines what happens if a mouse key is pressed while hovering an object
void GUIObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setFlag(QGraphicsItem::ItemIsMovable, true); //Make the component movable if not
    setFlag(QGraphicsItem::ItemIsSelectable, true); //Make the component selactable if not

        //Store old positions for all components, in case more than one is selected
    if(event->button() == Qt::LeftButton)
    {
        for(size_t i = 0; i < mpParentSystem->mSelectedGUIObjectsList.size(); ++i)
        {
            mpParentSystem->mSelectedGUIObjectsList[i]->mOldPos = mpParentSystem->mSelectedGUIObjectsList[i]->pos();
        }
    }

        //Objects shall not be selectable while creating a connector
    if(mpParentSystem->getIsCreatingConnector())
    {
        setFlag(QGraphicsItem::ItemIsMovable, false); //Make the component not movable during connection
        setFlag(QGraphicsItem::ItemIsSelectable, false); //Make the component not selactable during connection

        this->setSelected(false);
        this->setActive(false);
    }

    QGraphicsWidget::mousePressEvent(event);
}


//! @brief Defines what happens if a mouse key is released while hovering an object
void GUIObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
        //Objects shall not be selectable while creating a connector
    if(mpParentSystem->getIsCreatingConnector())
    {
        this->setSelected(false);
        this->setActive(false);
    }

    QGraphicsWidget::mouseReleaseEvent(event);
}


//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant GUIObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected())
        {
            mpParentSystem->mSelectedGUIObjectsList.append(this);
            mpSelectionBox->setActive();
            connect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            disconnect(mpParentSystem, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            connect(mpParentSystem, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            emit objectSelected();
        }
        else
        {
            mpParentSystem->mSelectedGUIObjectsList.removeAll(this);
            disconnect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            connect(mpParentSystem, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            disconnect(mpParentSystem, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            mpSelectionBox->setPassive();
        }
    }
//    else if (change == QGraphicsItem::ItemPositionHasChanged)
//    {
//        emit componentMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.

//            //Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
//        if(mpParentSystem != 0 && mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mSnapping && !mpParentSystem->mIsCreatingConnector)
//        {
//                //Vertical snap
//            if( (mpGUIConnectorPtrs.size() == 1) &&
//                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
//                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
//                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) )
//            {
//                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
//                {
//                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
//                }
//                else
//                {
//                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
//                }
//            }
////            else if( (mpGUIConnectorPtrs.size() == 2) &&
////                     (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
////                     (mpGUIConnectorPtrs.last()->getNumberOfLines() < 4) &&
////                     ( ( (this->rotation() == 0 || this->rotation() == 180) &&
////                       (mPortListPtrs.first()->pos().y() == mPortListPtrs.last()->pos().y()) ) ||
////                       ( (this->rotation() == 90 || this->rotation() == 270) &&
////                       (mPortListPtrs.first()->pos().x() == mPortListPtrs.last()->pos().x()) ) ) &&
////                     !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
////                     !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 1) &&
////                     (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) < SNAPDISTANCE) &&
////                     (abs(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x()) > 0.0) &&
////                     !(mpGUIConnectorPtrs.last()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.last()->getNumberOfLines() == 2) &&
////                     !(mpGUIConnectorPtrs.last()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.last()->getNumberOfLines() > 1) &&
////                     (abs(mpGUIConnectorPtrs.last()->mPoints.first().x() - mpGUIConnectorPtrs.last()->mPoints.last().x()) < SNAPDISTANCE) &&
////                     (abs(mpGUIConnectorPtrs.last()->mPoints.first().x() - mpGUIConnectorPtrs.last()->mPoints.last().x()) > 0.0) )
////            {
////                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
////                {
////                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.last().x() - mpGUIConnectorPtrs.first()->mPoints.first().x(), 0);
////                }
////                else
////                {
////                    this->moveBy(mpGUIConnectorPtrs.first()->mPoints.first().x() - mpGUIConnectorPtrs.first()->mPoints.last().x(), 0);
////                }
////            }

//                //Horizontal snap
//            if( (mpGUIConnectorPtrs.size() == 1) &&
//                (mpGUIConnectorPtrs.first()->getNumberOfLines() < 4) &&
//                !(mpGUIConnectorPtrs.first()->isFirstAndLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() == 2) &&
//                !(mpGUIConnectorPtrs.first()->isFirstOrLastDiagonal() && mpGUIConnectorPtrs.first()->getNumberOfLines() > 2) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) < SNAPDISTANCE) &&
//                (abs(mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y()) > 0.0) )
//            {
//                if(this->mpGUIConnectorPtrs.first()->getStartPort()->mpParentGuiObject == this)
//                {
//                    this->moveBy(0, mpGUIConnectorPtrs.first()->mPoints.last().y() - mpGUIConnectorPtrs.first()->mPoints.first().y());
//                }
//                else
//                {
//                    this->moveBy(0, mpGUIConnectorPtrs.first()->mPoints.first().y() - mpGUIConnectorPtrs.first()->mPoints.last().y());
//                }
//            }
//        }
//    }

    return  QGraphicsWidget::itemChange(change, value);
}


//! @brief Slot that rotates the object to a desired angle (NOT registered in undo stack!)
//! @param angle Angle to rotate to
//! @see rotate(undoStatus undoSettings)
//! @todo Add option to register this in undo stack - someone will want to do this sooner or later anyway
void GUIObject::rotateTo(qreal angle)
{
    while(this->rotation() != angle)
    {
        this->rotate(NOUNDO);
    }
}

//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stsack
//! @see rotateTo(qreal angle);
void GUIObject::rotate(undoStatus undoSettings)
{
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setRotation(this->rotation()+90);

    if (this->rotation() == 360)
    {
        this->setRotation(0);
    }

    if(undoSettings == UNDO)
    {
        mpParentSystem->mUndoStack->registerRotatedObject(this);
    }

    emit objectMoved();
}


//! @brief Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveUp()
{
    //qDebug() << "Move up!";
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()-1);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort(); //!< @todo If we have many objects selected this will probably call MANY updates of the viewport, maybe should do in some other way, same "problem" in other places
}


//! @brief Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void GUIObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void GUIObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Tells the component to ask its parent to delete it
//! @todo The name of the function is silly
//! @todo will not work with gui only objects like textboxes, as they ont have unique names
void GUIObject::deleteMe()
{
    //Should not be used
    assert(false);
}


bool GUIObject::isFlipped()
{
    return mIsFlipped;
}



//! @brief Constructor for GUI object selection box
//! @param x1 Initial X-coordinate of the top left corner of the parent component
//! @param y1 InitialY-coordinate of the top left corner of the parent component
//! @param x2 InitialX-coordinate of the bottom right corner of the parent component
//! @param y2 InitialY-coordinate of the bottom right corner of the parent component
//! @param activePen Width and color of the box when the parent component is selected.
//! @param hoverPen Width and color of the box when the parent component is hovered by the mouse cursor.
//! @param *parent Pointer to the parent object.
GUIObjectSelectionBox::GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent)
        : QGraphicsItemGroup(parent)
{
    mpParentGUIObject = parent;
    qreal b = 5;
    qreal a = 5;
    x1 = x1-3;
    y1 = y1-3;
    x2 = x2+3;
    y2 = y2+3;

    mActivePen = activePen;
    mHoverPen = hoverPen;
    this->setPassive();

    QGraphicsLineItem *tempLine = new QGraphicsLineItem(x1,y1+b,x1,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y1,x1+a,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2-a,y1,x2,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y1,x2,y1+b,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1+a,y2,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y2-b,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2-b,x2,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2,x2-a,y2,this);
    mLines.push_back(tempLine);
}


//! @brief Makes the box visible and makes it use "active" style
//! @see setPassive();
//! @see setHovered();
void GUIObjectSelectionBox::setActive()
{

    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(mActivePen);
    }
}


//! @brief Makes the box invisible
//! @see setActive();
//! @see setHovered();
void GUIObjectSelectionBox::setPassive()
{
    this->setVisible(false);
}


//! @brief Makes the box visible and makes it use "hovered" style
//! @see setActive();
//! @see setPassive();
void GUIObjectSelectionBox::setHovered()
{
    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(mHoverPen);
    }
}
