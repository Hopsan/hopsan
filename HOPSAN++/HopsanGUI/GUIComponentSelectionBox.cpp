//$Id$

#include "GUIComponentSelectionBox.h"
#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QObject>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>


GUIComponentSelectionBox::GUIComponentSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, QGraphicsItem *parent)
        : QGraphicsItemGroup(parent)
{
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


GUIComponentSelectionBox::~GUIComponentSelectionBox()
{
}

void GUIComponentSelectionBox::setActive()
{

    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mActivePen);
    }
}

void GUIComponentSelectionBox::setPassive()
{
    this->setVisible(false);
}

void GUIComponentSelectionBox::setHovered()
{
    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mHoverPen);
    }
}
