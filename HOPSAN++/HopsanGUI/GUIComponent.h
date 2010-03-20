//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <QGraphicsWidget>
#include <QGraphicsView>
#include <vector>
#include "GUIComponent.h"
#include "ProjectTabWidget.h"

class GraphicsScene;
class GraphicsView;
class GUIConnector;
class QGraphicsSvgItem;
class GUIComponentNameTextItem;
class HopsanEssentials;
class Component;
class GUIComponentSelectionBox;
class GUIPort;

class GUIComponent : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIComponent(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);
    //GUIComponent(HopsanEssentials *hopsan, const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIComponent();
    //QGraphicsView *getParentView();
    void addConnector(GUIConnector *item);

    void refreshName();
    QString getName();
    void setName(QString name);
    QString getTypeName();

    int getPortNumber(GUIPort *);
    int getNameTextPos();
    void setNameTextPos(int);

    void showPorts(bool visible);
    GUIPort *getPort(int number);

    GraphicsScene *mpParentGraphicsScene;
    GraphicsView *mpParentGraphicsView;

    enum { Type = UserType + 1 };
    int type() const;

    //Core interaction
    Component *mpCoreComponent;
    //

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    //void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    //void keyPressEvent( QKeyEvent *event );
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openParameterDialog();
    void groupComponents(QList<QGraphicsItem*> compList);

signals:
    void componentMoved();
    void componentDeleted();

public slots:
     void deleteMe();
     void rotate();
     void moveUp();
     void moveDown();
     void moveLeft();
     void moveRight();

private:
    QGraphicsSvgItem *mpIcon;
    GUIComponentNameTextItem *mpNameText;
    GUIComponentSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;
    //std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component

    QList<GUIPort*> mPortListPtrs;
    QString mComponentTypeName;
    int mNameTextPos;
    bool mIconRotation;

private slots:
    void fixTextPosition(QPointF pos);

};


class GUIComponentNameTextItem : public QGraphicsTextItem
{
    Q_OBJECT
private:
    GUIComponent* mpParentGUIComponent;

public:
    GUIComponentNameTextItem(GUIComponent *pParent);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusOutEvent(QFocusEvent *event);

signals:
    void textMoved(QPointF pos);

};



class GUIComponentSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIComponentSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIComponent *parent = 0);
    ~GUIComponentSelectionBox();
    void setActive();
    void setPassive();
    void setHovered();

    GUIComponent *mpParentGUIComponent;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};

#endif // GUICOMPONENT_H
