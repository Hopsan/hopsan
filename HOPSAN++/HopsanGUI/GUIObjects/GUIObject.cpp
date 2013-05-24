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
//! @file   GUIObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIObject class (The baseclass for all objects)
//!
//$Id$

#include <QtGui>

#include "GUIObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIWidgets.h"
#include "GraphicsView.h"
#include "Widgets/ProjectTabWidget.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include <cassert>


WorkspaceObject::WorkspaceObject(QPointF pos, qreal rot, SelectionStatusEnumT, ContainerObject *pParentContainer, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent)
{
    //Initi variables
    mpParentContainerObject = 0;
    mIsFlipped = false;
    mHmfTagName = HMF_OBJECTTAG;

    this->setParentContainerObject(pParentContainer);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    this->setAcceptHoverEvents(true);

    if (mpParentContainerObject != 0)
    {
        pParentContainer->getContainedScenePtr()->addItem(this);
    }

    //Set position orientation and other appearance stuff
    //Initially we dont know the selection box size
    mpSelectionBox = new WorkspaceObjectSelectionBox(0.0, 0.0, 0.0, 0.0, QPen(QColor("red"),2), QPen(QColor("darkRed"),2), this);
    mpSelectionBox->setZValue(SelectionboxZValue);
    mpSelectionBox->setPassive();
    this->setGeometry(0,0,0,0);
    this->setTransformOriginPoint(boundingRect().center());
    this->setCenterPos(pos);
    this->rotate(rot);
    updateOldPos();
}


void WorkspaceObject::setParentContainerObject(ContainerObject *pParentContainer)
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
int WorkspaceObject::type() const
{
    return Type;
}

//! @brief Returns the position of the  WorkspaceObject center in scene coordinates
QPointF WorkspaceObject::getCenterPos()
{
    if (this->scene() != 0)
    {
        return this->sceneBoundingRect().center();
    }
    else
    {
        return QPointF(0,0);
    }
}

//! @brief Set the position of the object so that it center is at given position
//! @param [in] cpos The center position in scene coordinates
void WorkspaceObject::setCenterPos(const QPointF cpos)
{
    if (this->scene() != 0)
    {
        // We translate pos by the same amout as the diff from our current center pos
        QPointF posDiff = cpos - this->sceneBoundingRect().center();
        this->setPos(this->pos()+posDiff);
    }
    else
    {
        setPos(cpos);
    }
}

void WorkspaceObject::saveToDomElement(QDomElement &/*rDomElement*/)
{
    // Nothing for now
}

void WorkspaceObject::loadFromHMF(QString /*modelFilePath=QString()*/)
{
    //Should only be available in SystemContainers for now
}

//! @brief Returns the objects parent ContainerObject or 0 if no parent container
ContainerObject *WorkspaceObject::getParentContainerObject()
{
    return mpParentContainerObject;
}


//! @brief Slot that deselects the object
void WorkspaceObject::deselect()
{
    this->setSelected(false);
}


//! @brief Slot that selects the object
void WorkspaceObject::select()
{
    this->setSelected(true);
}


//! @brief Defines what happens when mouse starts hovering the object
void WorkspaceObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setHovered();
    }

    QGraphicsWidget::hoverEnterEvent(event);
}


//! @brief Defines what happens when mouse stops hovering the object
void WorkspaceObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        mpSelectionBox->setPassive();
    }

    QGraphicsWidget::hoverLeaveEvent(event);
}


//! @brief Defines what happens if a mouse key is pressed while hovering an object
void WorkspaceObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(mpParentContainerObject == 0)
    {
        return;
    }

    setFlag(QGraphicsItem::ItemIsMovable, true); //Make the component movable if not (it is not movable during creation of connector)
    setFlag(QGraphicsItem::ItemIsSelectable, true); //Make the component selactable if not (it is not selectable during creation of connector)

        //Store old positions for all components, in case more than one is selected
    updateOldPos();  //Make sure current objects oldpos is changed (it may not be selected before being clicked)
    if(event->button() == Qt::LeftButton && mpParentContainerObject != 0)
    {
        for(int i = 0; i < mpParentContainerObject->getSelectedGUIWidgetPtrs().size(); ++i)
        {
            mpParentContainerObject->getSelectedGUIWidgetPtrs()[i]->setOldPos();
        }
    }

    QGraphicsWidget::mousePressEvent(event);

        //Objects shall not be selectable while creating a connector
    if(mpParentContainerObject->isCreatingConnector())
    {
        setFlag(QGraphicsItem::ItemIsMovable, false); //Make the component not movable during connection
        setFlag(QGraphicsItem::ItemIsSelectable, false); //Make the component not selactable during connection

        this->setSelected(false);
        this->setActive(false);
    }
}


//! @brief Defines what happens if a mouse key is released while hovering an object
void WorkspaceObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
        //Objects shall not be selectable while creating a connector
    if(mpParentContainerObject != 0 && mpParentContainerObject->isCreatingConnector())
    {
        this->setSelected(false);
        this->setActive(false);
    }

    //! @todo This crashes if we forward the event after calling "replace component". Not really needed, but figure out why.
    QGraphicsWidget::mouseReleaseEvent(event);
}


//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant WorkspaceObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected() && mpParentContainerObject != 0)
        {
            mpSelectionBox->setActive();
            connect(mpParentContainerObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            connect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsRight()), this, SLOT(rotate90cw()));
            connect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsLeft()), this, SLOT(rotate90ccw()));
            connect(mpParentContainerObject, SIGNAL(flipSelectedObjectsHorizontal()), this, SLOT(flipHorizontal()));
            connect(mpParentContainerObject, SIGNAL(flipSelectedObjectsVertical()), this, SLOT(flipVertical()));
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            disconnect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            connect(mpParentContainerObject, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            emit objectSelected();
        }
        else if(mpParentContainerObject != 0)
        {
            disconnect(mpParentContainerObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
            disconnect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsRight()), this, SLOT(rotate90cw()));
            disconnect(mpParentContainerObject, SIGNAL(rotateSelectedObjectsLeft()), this, SLOT(rotate90ccw()));
            disconnect(mpParentContainerObject, SIGNAL(flipSelectedObjectsHorizontal()), this, SLOT(flipHorizontal()));
            disconnect(mpParentContainerObject, SIGNAL(flipSelectedObjectsVertical()), this, SLOT(flipVertical()));
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlUp()), this, SLOT(moveUp()));
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlDown()), this, SLOT(moveDown()));
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlLeft()), this, SLOT(moveLeft()));
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressCtrlRight()), this, SLOT(moveRight()));
            connect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
            disconnect(mpParentContainerObject, SIGNAL(deselectAllGUIObjects()), this, SLOT(deselect()));
            mpSelectionBox->setPassive();
        }
    }

    QGraphicsWidget::itemChange(change, value);

    // Move component only horizontal, vertical or snap to original position if Ctrl is pressed
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        if(mpParentContainerObject != 0 && mpParentContainerObject->mpModelWidget->getGraphicsView()->isCtrlKeyPressed() && mpParentContainerObject->mpModelWidget->getGraphicsView()->isLeftMouseButtonPressed())
        {
            QPointF diff = this->pos()-mOldPos;
            if( diff.manhattanLength() < SNAPDISTANCE)
            {
                this->setPos(mOldPos);
            }
            else if(fabs(x()-mOldPos.x()) > fabs(y()-mOldPos.y()))
            {
                this->setPos(x(), mOldPos.y());
            }
            else
            {
                this->setPos(mOldPos.x(), y());
            }
        }
    }

    return value;
}


//! @brief Slot that rotates an object to the desired angle (NOT registered in undo stack!)
//! @param angle Angle to rotate to
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @note Undo registration will not work for obejcts or widgets as they have no name
void WorkspaceObject::rotate(qreal angle, UndoStatusEnumT /*undoSettings*/)
{
    this->setTransformOriginPoint(this->boundingRect().center());
    if(mIsFlipped)
    {
        angle *= -1;
    }
    this->setRotation(normDeg360(this->rotation()+angle));
    emit objectMoved();
}

//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see rotate(qreal angle, undoStatus undoSettings)
void WorkspaceObject::rotate90cw(UndoStatusEnumT undoSettings)
{
    this->rotate(90, undoSettings);
}

//! @brief Rotates a component 90 degrees counter-clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see rotate(qreal angle, undoStatus undoSettings)
void WorkspaceObject::rotate90ccw(UndoStatusEnumT undoSettings)
{
    this->rotate(-90, undoSettings);
}



//! @brief Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void WorkspaceObject::moveUp()
{
    //qDebug() << "Move up!";
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()-1);
    mpParentContainerObject->mpModelWidget->getGraphicsView()->updateViewPort(); //!< @todo If we have many objects selected this will probably call MANY updates of the viewport, maybe should do in some other way, same "problem" in other places
}


//! @brief Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void WorkspaceObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
    mpParentContainerObject->mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void WorkspaceObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
    mpParentContainerObject->mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void WorkspaceObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
    mpParentContainerObject->mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Tells the component to ask its parent to delete it
//! @todo The name of the function is silly
//! @todo will not work with gui only objects like textboxes, as they dont have unique names
void WorkspaceObject::deleteMe()
{
    //Should not be used
    assert(false);
}


void WorkspaceObject::updateOldPos()
{
    mOldPos = this->pos();
}


bool WorkspaceObject::isFlipped()
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
WorkspaceObjectSelectionBox::WorkspaceObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, WorkspaceObject *parent)
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

void WorkspaceObjectSelectionBox::setSize(qreal x1, qreal y1, qreal x2, qreal y2)
{
    this->prepareGeometryChange(); //dont know if this is actually necessary but lets call it anyway

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
void WorkspaceObjectSelectionBox::setActive()
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
void WorkspaceObjectSelectionBox::setPassive()
{
    this->setVisible(false);
}


//! @brief Makes the box visible and makes it use "hovered" style
//! @see setActive();
//! @see setPassive();
void WorkspaceObjectSelectionBox::setHovered()
{
    this->setVisible(true);
    for(int i=0; i<mLines.size(); ++i)
    {
        mLines[i]->setPen(mHoverPen);
    }
}
