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
#include <GUIConnector.h>

#include "common.h"

class QDomElement;
class WorkspaceObjectSelectionBox;
class SystemObject;

enum WorkspaceObjectEnumT {WorkspaceObjectType=QGraphicsItem::UserType+1, WidgetType, ModelObjectType, SystemObjectType, ComponentType, ScopeComponentType,
                           SystemPortObjectType, AnimatedObjectType};

class WorkspaceObject : public QGraphicsWidget
{
    Q_OBJECT

public:
    WorkspaceObject(QPointF pos, double rot, SelectionStatusEnumT=Deselected, SystemObject *pParentSystem=0, QGraphicsItem *pParent=0);

    virtual void setParentSystemObject(SystemObject *pParentSystem);
    virtual SystemObject *getParentSystemObject();

    // Position methods
    QPointF getCenterPos();
    void setCenterPos(const QPointF cpos);
    QPointF getPreviousPos() const;

    // Load and save methods
    virtual void loadFromDomElement(QDomElement domElement) = 0;
    virtual void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel) = 0;

    // Other methods
    bool isFlipped();
    LocklevelEnumT getModelLockLevel() const;
    bool isLocallyLocked() const;

    // Type info
    virtual int type() const override = 0;
    virtual QString getHmfTagName() const = 0;

public slots:
    virtual void flipVertical(UndoStatusEnumT undoSettings=Undo) = 0;
    virtual void flipHorizontal(UndoStatusEnumT undoSettings=Undo) = 0;
    virtual void rotate(double angle, UndoStatusEnumT undoSettings=Undo);
    void rotate90cw(UndoStatusEnumT undoSettings=Undo);
    void rotate90ccw(UndoStatusEnumT undoSettings=Undo);

    void rememberPos();

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void deselect();
    void select();

    //! @brief Tell the parent container to delete this object
    virtual void deleteMe(UndoStatusEnumT undoSettings=Undo) = 0;

signals:
    void objectMoved();
    void objectDeleted();
    void objectSelected();

protected:
    // Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    virtual void refreshSelectionBoxSize();

    // Protected members
    SystemObject *mpParentSystemObject = nullptr;
    bool mIsFlipped = false;
    bool mEnableSnap = true;
    bool mIsLocked = false;
    WorkspaceObjectSelectionBox *mpSelectionBox = nullptr;
    QPointF mPreviousPos;
    QMap<QString, QVector<Connector*> > mConnectionsBeforeReconfigure;
};



class WorkspaceObjectSelectionBox : public QGraphicsItemGroup
{
public:
    WorkspaceObjectSelectionBox(double x1, double y1, double x2, double y2, QPen activePen, QPen hoverPen, WorkspaceObject *parent = 0);
    void setSize(double x1, double y1, double x2, double y2);

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
