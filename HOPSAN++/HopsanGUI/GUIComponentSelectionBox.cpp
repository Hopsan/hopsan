//$Id$

#include "GUIComponentSelectionBox.h"
#include "GUIComponent.h"

#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QObject>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>


//! Constructor.
//! @param x1 is the x-coordinate of the top left corner of the parent component.
//! @param y1 is the y-coordinate of the top left corner of the parent component.
//! @param x2 is the x-coordinate of the bottom right corner of the parent component.
//! @param y2 is the y-coordinate of the bottom right corner of the parent component.
//! @param activePen defines the width and color of the box when the parent component is selected.
//! @param hoverPen defines the width and color of the box when the parent component is hovered by the mouse cursor.
//! @param *parent defines the parent object.
GUIComponentSelectionBox::GUIComponentSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIComponent *parent)
        : QGraphicsItemGroup(parent)
{
    mpParentGUIComponent = parent;

    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;

    QGraphicsLineItem *tempLine = new QGraphicsLineItem(x1,y1+5,x1,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y1,x1+5,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2-5,y1,x2,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y1,x2,y1+5,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1+5,y2,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y2-5,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2-5,x2,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2,x2-5,y2,this);
    mLines.push_back(tempLine);
}


//! Destructor.
GUIComponentSelectionBox::~GUIComponentSelectionBox()
{
}


//! Tells the box to become visible and use active style.
//! @see setPassive();
//! @see setHovered();
void GUIComponentSelectionBox::setActive()
{

    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mActivePen);
    }
}


//! Tells the box to become invisible.
//! @see setActive();
//! @see setHovered();
void GUIComponentSelectionBox::setPassive()
{
    this->setVisible(false);
}


//! Tells the box to become visible and use hovered style.
//! @see setActive();
//! @see setPassive();
void GUIComponentSelectionBox::setHovered()
{
    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mHoverPen);
    }
}
