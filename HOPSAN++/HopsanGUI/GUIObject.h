//$Id$

#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include <QGraphicsWidget>
#include <QGraphicsView>
#include <vector>
#include "ProjectTabWidget.h"

class GraphicsScene;
class GraphicsView;
class GUIConnector;
class QGraphicsSvgItem;
class GUIObjectDisplayName;
class HopsanEssentials;
class Component;
class GUIObjectSelectionBox;
class GUIPort;

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIObject(QPoint position, AppearanceData appearanceData, GraphicsScene *scene = 0, QGraphicsItem *parent = 0);
    ~GUIObject();

    void addConnector(GUIConnector *item);

    virtual QString getName();
    void refreshName();
    virtual void setName(QString name, bool doOnlyLocalRename=false);
    virtual QString getTypeName();

    int getPortNumber(GUIPort *port);
    int getNameTextPos();
    void setNameTextPos(int textPos);

    void showPorts(bool visible);
    GUIPort *getPort(int number);

    GraphicsScene *mpParentGraphicsScene;
    GraphicsView *mpParentGraphicsView;

    //Core interaction
    virtual Component* getHopsanCoreComponentPtr();
    virtual ComponentSystem* getHopsanCoreSystemComponentPtr();
    virtual void deleteInHopsanCore();

    enum { Type = UserType + 2 };
    int type() const;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void groupComponents(QList<QGraphicsItem*> compList);

signals:
    void componentMoved();
    void componentDeleted();
    void componentSelected();

public slots:
     void deleteMe();
     void rotate();
     void moveUp();
     void moveDown();
     void moveLeft();
     void moveRight();
     void flipVertical();
     void flipHorizontal();
     void hideName();
     void showName();
     void setIcon(bool useIso);

protected:
    QGraphicsSvgItem *mpIcon;
    bool mHasIsoIcon;
    GUIObjectDisplayName *mpNameText;
    GUIObjectSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;
    QString mIconPath;
    QString mIsoIconPath;
    //std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component
    QList<GUIPort*> mPortListPtrs;
    int mNameTextPos;
    bool mIconRotation;
    bool mIsFlipped;
    AppearanceData mAppearanceData;

protected slots:
    void fixTextPosition(QPointF pos);

};

class GUIObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT
private:
    GUIObject* mpParentGUIComponent;

public:
    GUIObjectDisplayName(GUIObject *pParent);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusOutEvent(QFocusEvent *event);

signals:
    void textMoved(QPointF pos);

};



class GUIObjectSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent = 0);
    ~GUIObjectSelectionBox();
    void setActive();
    void setPassive();
    void setHovered();

    GUIObject *mpParentGUIObject;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};


class GUIComponent : public GUIObject
{
    Q_OBJECT
public:
    GUIComponent(HopsanEssentials *hopsan, AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

    //Core interaction
    Component* getHopsanCoreComponentPtr();
    Component *mpCoreComponent;
    //

    QString getName();
    void setName(QString name, bool doOnlyLocalRename=false);
    QString getTypeName();
    void deleteInHopsanCore();

    enum { Type = UserType + 3 };
    int type() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

    QString mComponentTypeName;

public slots:

};

class GUISubsystem : public GUIObject
{
    Q_OBJECT
public:
    GUISubsystem(HopsanEssentials *hopsan, AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

    Component* getHopsanCoreComponentPtr();
    ComponentSystem* getHopsanCoreSystemComponentPtr();
    void deleteInHopsanCore();

    QString getName();
    QString getTypeName();
    void setName(QString newName, bool doOnlyCoreRename);
    void setTypeCQS(QString typestring);
    QString getTypeCQS();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

private:
    QString mModelFilePath;
    QString mGraphicsFilePath;
    bool   mIsEmbedded;
    ComponentSystem *mpCoreComponentSystem;
};

class GUISystemPort : public GUIObject
{
    Q_OBJECT
public:
    GUISystemPort(ComponentSystem* pCoreComponentSystem, Port* pCorePort, AppearanceData appearanceData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);
    QString getTypeName();
    void setName(QString newName, bool doOnlyCoreRename);
    QString getName();
private:
    ComponentSystem *mpCoreComponentSystem;
    Port *mpCorePort; //hmm kanske inte
    GUIPort *mpGuiPort;

};


class GUIGroup : public GUIObject
{
    Q_OBJECT
public:
    GUIGroup(QList<QGraphicsItem*> compList, AppearanceData appearanceData, GraphicsScene *scene, QGraphicsItem *parent = 0);

//    QString getName();
//    void setName(QString name, bool doOnlyLocalRename=false);

    enum { Type = UserType + 4 };
    int type() const;

protected:
    GraphicsScene *mpParentScene;
    GraphicsScene *mpGroupScene;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void showParent();

//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
//    void openParameterDialog();
//
//    QString mComponentTypeName;
//
//    GraphicsScene *mpGroupScene;
//
//public slots:
//     void deleteMe();
};

#endif // GUIOBJECT_H
