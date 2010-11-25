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
class GUIContainerObject;
class GUIConnector;


enum GUIObjectEnumT {GUIOBJECT=QGraphicsItem::UserType+1, GUIMODELOBJECT, GUICONTAINEROBJECT, GUISYSTEM, GUICOMPONENT, GUISYSTEMPORT, GUIGROUP, GUIGROUPPORT};

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT

public:
    GUIObject(QPoint pos, qreal rot, selectionStatus=DESELECTED, GUIContainerObject *pSystem=0, QGraphicsItem *pParent=0);
    ~GUIObject();

    //Name methods
    virtual QString getTypeName() {assert(false);} //Maybe sould not bee here
    virtual QString getName() {assert(false);} //Maybe sould not bee here

    //Position methods
    virtual QPointF getCenterPos();
    virtual void setCenterPos(QPointF pos);

    //Load and save methods
    //virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString()){;} //! @todo nothing for now
    virtual void saveToDomElement(QDomElement &rDomElement){;}  //! @todo nothing for now
    virtual void loadFromHMF(QString modelFilePath=QString()) {assert(false);} //Only available in GUISubsystem for now

    bool isFlipped();

    enum { Type = GUIOBJECT };
    int type() const;

    //Public members
    GUIContainerObject *mpParentContainerObject; //!< @todo not public
    QPointF mOldPos;

public slots:
    virtual void flipVertical(undoStatus undoSettings = UNDO){;} //!< @todo nothing for now
    virtual void flipHorizontal(undoStatus undoSettings = UNDO){;}  //!< @todo nothing for now
    virtual void deleteMe();
    virtual void rotate(undoStatus undoSettings = UNDO);

    void rotateTo(qreal angle);
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
    GUIObjectSelectionBox *mpSelectionBox;
};



class GUIObjectSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

public:
    GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent = 0);

    //Selection/active methods
    void setActive();
    void setPassive();
    void setHovered();

    //Public members
    GUIObject *mpParentGUIObject;

protected:
    //Protected members
    std::vector<QGraphicsLineItem*> mLines;

    QPen mActivePen;
    QPen mHoverPen;

};



#endif // GUIOBJECT_H
