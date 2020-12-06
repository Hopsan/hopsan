/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   GUIObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIObject class (The baseclass for all objects)
//!
//$Id$

#include <QGraphicsSceneMouseEvent>

#include "GUIObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIWidgets.h"
#include "GraphicsView.h"
#include "Widgets/ModelWidget.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"


WorkspaceObject::WorkspaceObject(QPointF pos, double rot, SelectionStatusEnumT, SystemObject *pParentSystem, QGraphicsItem *pParent)
    : QGraphicsWidget(pParent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    setAcceptHoverEvents(true);

    setParentSystemObject(pParentSystem);
    if (mpParentSystemObject)
    {
        pParentSystem->getContainedScenePtr()->addItem(this);
    }

    // Set position orientation and other appearance stuff
    // Initially we do not know the selection box size
    mpSelectionBox = new WorkspaceObjectSelectionBox(0.0, 0.0, 0.0, 0.0, QPen(QColor("red"),2), QPen(QColor("darkRed"),2), this);
    mpSelectionBox->setZValue(SelectionboxZValue);
    mpSelectionBox->setPassive();
    setGeometry(0,0,0,0);
    setTransformOriginPoint(boundingRect().center());
    setCenterPos(pos);
    rotate(rot);
    rememberPos();
}


void WorkspaceObject::setParentSystemObject(SystemObject *pParentSystem)
{
    if(mpParentSystemObject)
    {
        // First clear the slot and then establish new connection
        disconnect(mpParentSystemObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
    }
    mpParentSystemObject = pParentSystem;
    if(mpParentSystemObject)
    {
        connect(mpParentSystemObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()), Qt::UniqueConnection);
    }
}


//! @brief Returns the position of the  WorkspaceObject center in scene coordinates
QPointF WorkspaceObject::getCenterPos()
{
    if (scene())
    {
        return sceneBoundingRect().center();
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
    if (scene())
    {
        // We translate pos by the same amount as the diff from our current center pos
        QPointF posDiff = cpos - sceneBoundingRect().center();
        setPos(pos()+posDiff);
    }
    else
    {
        setPos(cpos);
    }
}

//! @brief Returns the position stored with rememberPos
//! @see WorkspaceObject::rememberPos()
QPointF WorkspaceObject::getPreviousPos() const
{
    return mPreviousPos;
}

//! @brief Returns the objects parent ContainerObject or 0 if no parent container
SystemObject *WorkspaceObject::getParentSystemObject()
{
    return mpParentSystemObject;
}


//! @brief Slot that deselects the object
void WorkspaceObject::deselect()
{
    setSelected(false);
}


//! @brief Slot that selects the object
void WorkspaceObject::select()
{
    setSelected(true);
}


//! @brief Defines what happens when mouse starts hovering the object
void WorkspaceObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!isSelected())
    {
        mpSelectionBox->setHovered();
    }

    QGraphicsWidget::hoverEnterEvent(event);
}


//! @brief Defines what happens when mouse stops hovering the object
void WorkspaceObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!isSelected())
    {
        mpSelectionBox->setPassive();
    }

    QGraphicsWidget::hoverLeaveEvent(event);
}

void WorkspaceObject::refreshSelectionBoxSize()
{
    // Does nothing by default, should be overloaded
}


//! @brief Defines what happens if a mouse key is pressed while hovering an object
void WorkspaceObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mpParentSystemObject)
    {
        return;
    }

    // We need the following code her to renven all objects including widgets from being movable while a connection is being created

    // Objects shall not be selectable while creating a connector
    if(mpParentSystemObject->isCreatingConnector())
    {
        setFlag(QGraphicsItem::ItemIsMovable, false);    // Make the component not movable during connection
        setFlag(QGraphicsItem::ItemIsSelectable, false); // Make the component not selectable during connection

        setSelected(false);
        setActive(false);
    }
    // Else we can re-enable movement and selection
    else
    {
        setFlag(QGraphicsItem::ItemIsMovable, true);    // Make the component movable if not (it is not movable during creation of connector)
        setFlag(QGraphicsItem::ItemIsSelectable, true); // Make the component selectable if not (it is not selectable during creation of connector)
    }

    // Make sure current objects oldpos is changed (it may not be selected before being clicked)
    rememberPos();

    // Store old positions for all components, in case more than one is selected
    //! @todo this should be handled elsewhere
    if(mpParentSystemObject && event->button() == Qt::LeftButton )
    {
        for(int i=0; i<mpParentSystemObject->getSelectedGUIWidgetPtrs().size(); ++i)
        {
            mpParentSystemObject->getSelectedGUIWidgetPtrs()[i]->rememberPos();
        }
    }

    QGraphicsWidget::mousePressEvent(event);
}

void WorkspaceObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //! @todo This crashes if we forward the event after calling "replace component". Not really needed, but figure out why.
    QGraphicsWidget::mouseReleaseEvent(event);

    // Objects shall not be selected while creating a connector
    if(mpParentSystemObject && mpParentSystemObject->isCreatingConnector())
    {
        setSelected(false);
        setActive(false);
    }
}


//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant WorkspaceObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // If item selection changes then connect or disconnect the appropriate signals
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (isSelected() && mpParentSystemObject)
        {
            mpSelectionBox->setActive();

            connect(mpParentSystemObject,    SIGNAL(deleteSelected()),                   this, SLOT(deleteMe()));
            connect(mpParentSystemObject,    SIGNAL(rotateSelectedObjectsRight()),       this, SLOT(rotate90cw()));
            connect(mpParentSystemObject,    SIGNAL(rotateSelectedObjectsLeft()),        this, SLOT(rotate90ccw()));
            connect(mpParentSystemObject,    SIGNAL(flipSelectedObjectsHorizontal()),    this, SLOT(flipHorizontal()));
            connect(mpParentSystemObject,    SIGNAL(flipSelectedObjectsVertical()),      this, SLOT(flipVertical()));
            connect(mpParentSystemObject,    SIGNAL(deselectAllGUIObjects()),            this, SLOT(deselect()));
            disconnect(mpParentSystemObject, SIGNAL(selectAllGUIObjects()),              this, SLOT(select()));

            GraphicsView *pGraphicsView = mpParentSystemObject->mpModelWidget->getGraphicsView();
            connect(pGraphicsView,              SIGNAL(keyPressDelete()),                   this, SLOT(deleteMe()));
            connect(pGraphicsView,              SIGNAL(keyPressCtrlUp()),                   this, SLOT(moveUp()));
            connect(pGraphicsView,              SIGNAL(keyPressCtrlDown()),                 this, SLOT(moveDown()));
            connect(pGraphicsView,              SIGNAL(keyPressCtrlLeft()),                 this, SLOT(moveLeft()));
            connect(pGraphicsView,              SIGNAL(keyPressCtrlRight()),                this, SLOT(moveRight()));

            emit objectSelected();
        }
        else if(mpParentSystemObject)
        {
            disconnect(mpParentSystemObject, SIGNAL(deleteSelected()),                   this, SLOT(deleteMe()));
            disconnect(mpParentSystemObject, SIGNAL(rotateSelectedObjectsRight()),       this, SLOT(rotate90cw()));
            disconnect(mpParentSystemObject, SIGNAL(rotateSelectedObjectsLeft()),        this, SLOT(rotate90ccw()));
            disconnect(mpParentSystemObject, SIGNAL(flipSelectedObjectsHorizontal()),    this, SLOT(flipHorizontal()));
            disconnect(mpParentSystemObject, SIGNAL(flipSelectedObjectsVertical()),      this, SLOT(flipVertical()));
            disconnect(mpParentSystemObject, SIGNAL(deselectAllGUIObjects()),            this, SLOT(deselect()));
            connect(mpParentSystemObject,    SIGNAL(selectAllGUIObjects()),              this, SLOT(select()));

            GraphicsView *pGraphicsView = mpParentSystemObject->mpModelWidget->getGraphicsView();
            disconnect(pGraphicsView,           SIGNAL(keyPressDelete()),                   this, SLOT(deleteMe()));
            disconnect(pGraphicsView,           SIGNAL(keyPressCtrlUp()),                   this, SLOT(moveUp()));
            disconnect(pGraphicsView,           SIGNAL(keyPressCtrlDown()),                 this, SLOT(moveDown()));
            disconnect(pGraphicsView,           SIGNAL(keyPressCtrlLeft()),                 this, SLOT(moveLeft()));
            disconnect(pGraphicsView,           SIGNAL(keyPressCtrlRight()),                this, SLOT(moveRight()));

            mpSelectionBox->setPassive();
        }
    }

    QGraphicsWidget::itemChange(change, value);

    // Move component only horizontal, vertical or snap to original position if Ctrl is pressed
    if (mpParentSystemObject && (change == QGraphicsItem::ItemPositionHasChanged) && mEnableSnap)
    {
        GraphicsView *pGraphicsView = mpParentSystemObject->mpModelWidget->getGraphicsView();
        if(mpParentSystemObject && pGraphicsView->isCtrlKeyPressed() && pGraphicsView->isLeftMouseButtonPressed())
        {
            QPointF diff = this->pos()-mPreviousPos;
            if( diff.manhattanLength() < 0.5*SNAPDISTANCE)
            {
                setPos(mPreviousPos);
            }
            else if(fabs(x()-mPreviousPos.x()) > fabs(y()-mPreviousPos.y()))
            {
                setPos(x(), mPreviousPos.y());
            }
            else
            {
                setPos(mPreviousPos.x(), y());
            }
        }
    }

    return value;
}


//! @brief Slot that rotates an object to the desired angle (NOT registered in undo stack!)
//! @param angle Angle to rotate to
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @note Undo registration will not work for objects or widgets as they have no name
void WorkspaceObject::rotate(double angle, UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    if (!isLocallyLocked() && (getModelLockLevel()==NotLocked))
    {
        setTransformOriginPoint(boundingRect().center());
        if(mIsFlipped)
        {
            angle *= -1.0;
        }
        setRotation(normDeg360(rotation()+angle));
        emit objectMoved();
    }
}

//! @brief Rotates a component 90 degrees clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see rotate(double angle, undoStatus undoSettings)
void WorkspaceObject::rotate90cw(UndoStatusEnumT undoSettings)
{
    rotate(90, undoSettings);
}

//! @brief Rotates a component 90 degrees counter-clockwise
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see rotate(double angle, undoStatus undoSettings)
void WorkspaceObject::rotate90ccw(UndoStatusEnumT undoSettings)
{
    rotate(-90, undoSettings);
}



//! @brief Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void WorkspaceObject::moveUp()
{
    if (!isLocallyLocked() && (getModelLockLevel()==NotLocked))
    {
        setPos(pos().x(), mapFromScene(mapToScene(pos())).y()-1);
        mpParentSystemObject->mpModelWidget->getGraphicsView()->updateViewPort(); //!< @todo If we have many objects selected this will probably call MANY updates of the viewport, maybe should do in some other way, same "problem" in other places
    }
}


//! @brief Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void WorkspaceObject::moveDown()
{
    if (!isLocallyLocked() && (getModelLockLevel()==NotLocked))
    {
        setPos(pos().x(), mapFromScene(mapToScene(pos())).y()+1);
        mpParentSystemObject->mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


//! @brief Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void WorkspaceObject::moveLeft()
{
    if (!isLocallyLocked() && (getModelLockLevel()==NotLocked))
    {
        setPos(mapFromScene(mapToScene(pos())).x()-1, pos().y());
        mpParentSystemObject->mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


//! @brief Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void WorkspaceObject::moveRight()
{
    if (!isLocallyLocked() && (getModelLockLevel()==NotLocked))
    {
        setPos(mapFromScene(mapToScene(pos())).x()+1, pos().y());
        mpParentSystemObject->mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


void WorkspaceObject::rememberPos()
{
    mPreviousPos = pos();
}


bool WorkspaceObject::isFlipped()
{
    return mIsFlipped;
}

bool WorkspaceObject::isLocallyLocked() const
{
    return mIsLocked;
}


LocklevelEnumT WorkspaceObject::getModelLockLevel() const
{
    auto* pSystem = qobject_cast<const SystemObject*>(this);
    if((pSystem == nullptr) && mpParentSystemObject) {
        pSystem = mpParentSystemObject;
    }

    if (pSystem && pSystem->mpModelWidget) {
        return pSystem->mpModelWidget->getCurrentLockLevel();
    }

    return NotLocked;
}



//! @brief Constructor for GUI object selection box
//! @param x1 Initial X-coordinate of the top left corner of the parent component
//! @param y1 Initial Y-coordinate of the top left corner of the parent component
//! @param x2 Initial X-coordinate of the bottom right corner of the parent component
//! @param y2 Initial Y-coordinate of the bottom right corner of the parent component
//! @param activePen Width and color of the box when the parent component is selected.
//! @param hoverPen Width and color of the box when the parent component is hovered by the mouse cursor.
//! @param *parent Pointer to the parent object.
WorkspaceObjectSelectionBox::WorkspaceObjectSelectionBox(double x1, double y1, double x2, double y2, QPen activePen, QPen hoverPen, WorkspaceObject *parent)
        : QGraphicsItemGroup(parent)
{
    mActivePen = activePen;
    mHoverPen = hoverPen;
    setPassive();

    //Create 8 small lines, if you want more or less lines, sync your changes
    //with the setSize() function
    QGraphicsLineItem *tempLine;
    for (int i=0; i<8; ++i)
    {
        tempLine = new QGraphicsLineItem(this);
        mLines.push_back(tempLine);
    }
    setSize(x1,y1,x2,y2);
}

void WorkspaceObjectSelectionBox::setSize(double x1, double y1, double x2, double y2)
{
    prepareGeometryChange(); //don't know if this is actually necessary but lets call it anyway

    double b = 6;
    double a = 6;
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
    setVisible(true);
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
    setVisible(false);
}


//! @brief Makes the box visible and makes it use "hovered" style
//! @see setActive();
//! @see setPassive();
void WorkspaceObjectSelectionBox::setHovered()
{
    setVisible(true);
    for(int i=0; i<mLines.size(); ++i)
    {
        mLines[i]->setPen(mHoverPen);
    }
}
