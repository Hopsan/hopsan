//!
//! @file   GUIObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIObject class (The baseclass for all objects)
//!
//$Id$

#include <QtGui>

#include "GUIObject.h"
#include "GUIContainerObject.h"
#include "../GraphicsView.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../UndoStack.h"


//! @todo should not pSystem and pParent be teh same ?
GUIObject::GUIObject(QPoint pos, qreal rot, selectionStatus, GUIContainerObject *pParentContainer, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent)
{
    //Initi variables
    mpParentContainerObject = 0;
    mHmfTagName = HMF_OBJECTTAG;

    this->setParentContainerObject(pParentContainer);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);

    //Set position orientation and other appearance stuff
    //Initially we dont know the selection box size
    mpSelectionBox = new GUIObjectSelectionBox(0.0, 0.0, 0.0, 0.0, QPen(QColor("red"),2), QPen(QColor("darkRed"),2), this);
    mpSelectionBox->setZValue(13);
    mpSelectionBox->setPassive();
    this->setCenterPos(pos);
    this->rotateTo(rot);
    this->setAcceptHoverEvents(true);
    mIsFlipped = false;
}


////! @brief Destructor for GUI Objects
//GUIObject::~GUIObject()
//{

//}

void GUIObject::setParentContainerObject(GUIContainerObject *pParentContainer)
{
    if(mpParentContainerObject != 0)
    {
        //First clear the slot and then establish new connection
        disconnect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
    }
    mpParentContainerObject = pParentContainer;
    if(mpParentContainerObject != 0)
    {
        connect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()),Qt::UniqueConnection);
    }
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

void GUIObject::saveToDomElement(QDomElement &/*rDomElement*/){}  //! @todo nothing for now

GUIContainerObject *GUIObject::getParentContainerObject()
{
    return mpParentContainerObject;
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


//! @brief Defines what happens when mouse stops hovering the object
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

    QGraphicsWidget::mousePressEvent(event);    //This must be before the code! Otherwise old position will not be stored for this object!

        //Store old positions for all components, in case more than one is selected
    if(event->button() == Qt::LeftButton)
    {
        for(int i = 0; i < mpParentContainerObject->mSelectedGUIModelObjectsList.size(); ++i)
        {
            mpParentContainerObject->mSelectedGUIModelObjectsList[i]->mOldPos = mpParentContainerObject->mSelectedGUIModelObjectsList[i]->pos();

        }
        for(int i = 0; i < mpParentContainerObject->mSelectedGUIWidgetsList.size(); ++i)
        {
            mpParentContainerObject->mSelectedGUIWidgetsList[i]->mOldPos = mpParentContainerObject->mSelectedGUIWidgetsList[i]->pos();
        }
    }

        //Objects shall not be selectable while creating a connector
    if(mpParentContainerObject->getIsCreatingConnector())
    {
        setFlag(QGraphicsItem::ItemIsMovable, false); //Make the component not movable during connection
        setFlag(QGraphicsItem::ItemIsSelectable, false); //Make the component not selactable during connection

        this->setSelected(false);
        this->setActive(false);
    }
}


//! @brief Defines what happens if a mouse key is released while hovering an object
void GUIObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
        //Objects shall not be selectable while creating a connector
    if(mpParentContainerObject->getIsCreatingConnector())
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
            mpSelectionBox->setActive();
            connect(mpParentContainerObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            connect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsRight()), this, SLOT(rotate90cw()));
            connect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsLeft()), this, SLOT(rotate90ccw()));
            connect(mpParentContainerObject, SIGNAL(flipSelectedObjectsHorizontal()), this, SLOT(flipHorizontal()));
            connect(mpParentContainerObject, SIGNAL(flipSelectedObjectsVertical()), this, SLOT(flipVertical()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate90cw()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlE()), this, SLOT(rotate90ccw()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            disconnect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            connect(mpParentContainerObject, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            emit objectSelected();
        }
        else
        {
            disconnect(mpParentContainerObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            disconnect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsRight()), this, SLOT(rotate90cw()));
            disconnect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsLeft()), this, SLOT(rotate90ccw()));
            disconnect(mpParentContainerObject, SIGNAL(flipSelectedObjectsHorizontal()), this, SLOT(flipHorizontal()));
            disconnect(mpParentContainerObject, SIGNAL(flipSelectedObjectsVertical()), this, SLOT(flipVertical()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlR()), this, SLOT(rotate90cw()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlE()), this, SLOT(rotate90ccw()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            connect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            disconnect(mpParentContainerObject, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
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
        this->rotate90cw(NOUNDO);
    }
}

//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stsack
//! @see rotateTo(qreal angle);
void GUIObject::rotate90cw(undoStatus undoSettings)
{
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setRotation(this->rotation()+90);

    if (this->rotation() >= 359)
    {
        this->setRotation(0);
    }

    if(undoSettings == UNDO)
    {
        mpParentContainerObject->mUndoStack->registerRotatedObject(this->getName(), 90);
    }

    emit objectMoved();
}

//! @brief Rotates a component 90 degrees counter-clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stsack
//! @see rotateTo(qreal angle);
void GUIObject::rotate90ccw(undoStatus undoSettings)
{
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setRotation(this->rotation()-90);

    if (this->rotation() < 0)
    {
        this->setRotation(360+this->rotation());
    }

    if(undoSettings == UNDO)
    {
        mpParentContainerObject->mUndoStack->registerRotatedObject(this->getName(), -90);    //! @todo This will register a clockwise rotation, which will be bad...
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
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort(); //!< @todo If we have many objects selected this will probably call MANY updates of the viewport, maybe should do in some other way, same "problem" in other places
}


//! @brief Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void GUIObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void GUIObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort();
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
    mActivePen = activePen;
    mHoverPen = hoverPen;
    this->setPassive();

    //Create 8 small lines, if you want more or less lines, sync your changes
    //with the setSize() function
    QGraphicsLineItem *tempLine;
    for (int i=0; i<8; ++i)
    {
        tempLine = new QGraphicsLineItem(this);
        mLines.push_back(tempLine);
    }
    this->setSize(x1,y1,x2,y2);
}

void GUIObjectSelectionBox::setSize(qreal x1, qreal y1, qreal x2, qreal y2)
{
    qreal b = 6;
    qreal a = 6;
    x1 += -4;
    y1 += -4;
    x2 += 4;
    y2 += 4;

    mLines[0]->setLine(x1,y1+b,x1,y1);
    mLines[1]->setLine(x1,y1,x1+a,y1);
    mLines[2]->setLine(x2-a,y1,x2,y1);
    mLines[3]->setLine(x2,y1,x2,y1+b);
    mLines[4]->setLine(x1+a,y2,x1,y2);
    mLines[5]->setLine(x1,y2-b,x1,y2);
    mLines[6]->setLine(x2,y2-b,x2,y2);
    mLines[7]->setLine(x2,y2,x2-a,y2);
}

//! @brief Makes the box visible and makes it use "active" style
//! @see setPassive();
//! @see setHovered();
void GUIObjectSelectionBox::setActive()
{
    this->setVisible(true);
    for(int i=0; i<mLines.size(); ++i)
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
    for(int i=0; i<mLines.size(); ++i)
    {
        mLines[i]->setPen(mHoverPen);
    }
}
