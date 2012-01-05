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
//! @file   GUIObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIObject class (The baseclass for all objects)
//!
//$Id$

#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPen>

#include <cassert>
#include "common.h"

class QDomElement;
class WorkspaceObjectSelectionBox;
class ContainerObject;

enum GUIObjectEnumT {WORKSPACEOBJECT=QGraphicsItem::UserType+1, MODELOBJECT, CONTAINEROBJECT, SYSTEMCONTAINER, COMPONENT, CONTAINERPORT, GROUPCONTAINER};

class WorkspaceObject : public QGraphicsWidget
{
    Q_OBJECT

public:
    WorkspaceObject(QPointF pos, qreal rot, selectionStatus=DESELECTED, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);

    virtual void setParentContainerObject(ContainerObject *pParentContainer);
    virtual ContainerObject *getParentContainerObject();

    //Name methods
    virtual QString getTypeName() {assert(false);} //Maybe sould not bee here
    virtual QString getName() {assert(false);} //Maybe sould not bee here

    //Position methods
    QPointF getCenterPos();
    void setCenterPos(const QPointF cpos);

    //Load and save methods
    virtual void saveToDomElement(QDomElement &/*rDomElement*/);
    virtual void loadFromHMF(QString /*modelFilePath=QString()*/) {assert(false);} //Only available in GUISubsystem for now

    //Other methods
    bool isFlipped();

    enum { Type = WORKSPACEOBJECT };
    int type() const;

    //Public members
    ContainerObject *mpParentContainerObject;

public slots:
    virtual void flipVertical(undoStatus /*undoSettings = UNDO*/){} //!< @todo nothing for now
    virtual void flipHorizontal(undoStatus /*undoSettings = UNDO*/){}  //!< @todo nothing for now
    virtual void deleteMe();
    virtual void rotate(qreal angle, undoStatus undoSettings=UNDO);
    void rotate90cw(undoStatus undoSettings=UNDO);
    void rotate90ccw(undoStatus undoSettings=UNDO);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void deselect();
    void select();

signals:
    void objectMoved();
    void objectDeleted();
    void objectSelected();

protected:
    //Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    //Protected members
    QString mHmfTagName;
    bool mIsFlipped;
    WorkspaceObjectSelectionBox *mpSelectionBox;
    QPointF mOldPos;
};



class WorkspaceObjectSelectionBox : public QGraphicsItemGroup
{
public:
    WorkspaceObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, WorkspaceObject *parent = 0);
    void setSize(qreal x1, qreal y1, qreal x2, qreal y2);

    //Selection/active methods
    void setActive();
    void setPassive();
    void setHovered();

protected:
    //Protected members
    QVector<QGraphicsLineItem*> mLines;

    QPen mActivePen;
    QPen mHoverPen;
};



#endif // GUIOBJECT_H
