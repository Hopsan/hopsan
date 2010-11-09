//$Id$

#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsSvgItem>
#include <QPen>

#include <assert.h>
#include "common.h"
#include <QtXml>  //This one is only used for the virtual save function

class GUIObjectSelectionBox;
class GUISystem;


enum GUIObjectEnumT {GUIOBJECT=QGraphicsItem::UserType+1, GUIMODELOBJECT, GUICONTAINEROBJECT, GUISYSTEM, GUICOMPONENT, GUISYSTEMPORT, GUIGROUP, GUIGROUPPORT};

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIObject(QPoint pos, qreal rot, selectionStatus=DESELECTED, GUISystem *pSystem=0, QGraphicsItem *pParent=0);
    ~GUIObject();
    GUISystem *mpParentSystem; //!< @todo not public

    virtual QString getTypeName() {assert(false);} //Maybe sould not bee here
    virtual QString getName() {assert(false);} //Maybe sould not bee here

    virtual QPointF getCenterPos();
    virtual void setCenterPos(QPointF pos);

    virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString()){;} //! @todo nothing for now
    virtual void saveToDomElement(QDomElement &rDomElement){;}  //! @todo nothing for now
    virtual void loadFromHMF(QString modelFilePath=QString()) {assert(false);} //Only available in GUISubsystem for now

    bool isFlipped();

    enum { Type = GUIOBJECT };
    int type() const;

    QPointF mOldPos;

public slots:
    virtual void deleteMe();
    virtual void rotate(undoStatus undoSettings = UNDO);
    void rotateTo(qreal angle);
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    virtual void flipVertical(undoStatus undoSettings = UNDO){;} //!< @todo nothing for now
    virtual void flipHorizontal(undoStatus undoSettings = UNDO){;}  //!< @todo nothing for now
    void deselect();
    void select();

signals:
    void objectMoved();
    void objectDeleted();
    void objectSelected();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    QString mHmfTagName;
    GUIObjectSelectionBox *mpSelectionBox;
    bool mIsFlipped;
};



class GUIObjectSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent = 0);
    void setActive();
    void setPassive();
    void setHovered();

    GUIObject *mpParentGUIObject;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};



#endif // GUIOBJECT_H
