//$Id$

#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsSvgItem>
#include <QPen>
#include <QtXml>

#include "common.h"

#include "AppearanceData.h"
#include <assert.h>

class ProjectTabWidget;
class GraphicsScene;
class GraphicsView;
class GUIConnector;
class GUIObjectDisplayName;
class Component;
class GUIObjectSelectionBox;
class GUIPort;
class GUISystem;
class GUIComponent;

enum GUIObjectEnumT {GUIOBJECT=QGraphicsItem::UserType+1, GUISYSTEM, GUICOMPONENT, GUISYSTEMPORT, GUIGROUP, GUIGROUPPORT};

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, GUISystem *system = 0, QGraphicsItem *parent = 0);
    ~GUIObject();

    void rememberConnector(GUIConnector *item);
    void forgetConnector(GUIConnector *item);

    QList<GUIConnector*> getGUIConnectorPtrs();

    virtual QString getName();
    void refreshDisplayName();
    //virtual void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    void setDisplayName(QString name);
    virtual QString getTypeName();
    virtual QString getTypeCQS() {assert(false); return "";} //Only available in GUISystemComponent adn GuiComponent for now
    virtual void setTypeCQS(QString typestring) {assert(false);} //Only available in GUISystemComponent

    AppearanceData* getAppearanceData();
    void refreshAppearance();

    int getNameTextPos();
    void setNameTextPos(int textPos);

    void showPorts(bool visible);
    GUIPort *getPort(QString name);
    QList<GUIPort*> &getPortListPtrs();

    virtual QVector<QString> getParameterNames();
    virtual QString getParameterUnit(QString name) {assert(false); return "";} //Only availible in GUIComponent for now
    virtual QString getParameterDescription(QString name) {assert(false); return "";} //Only availible in GUIComponent for now
    virtual double getParameterValue(QString name);
    virtual void setParameterValue(QString name, double value);

    GUISystem *mpParentSystem;

    virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    virtual void saveToDomElement(QDomElement &rDomElement);
    virtual void loadFromHMF(QString modelFilePath=QString()) {assert(false);} //Only available in GUISubsystem for now

    enum { Type = GUIOBJECT };
    int type() const;
    GUIObjectDisplayName *mpNameText;
    //QHash<QString, GUIPort*> mGuiPortPtrMap;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    virtual void saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement);

signals:
    void componentMoved();
    void componentDeleted();
    void componentSelected();

public slots:
     void deleteMe();
     void rotate(undoStatus undoSettings = UNDO);
     void rotateTo(qreal angle);
     void moveUp();
     void moveDown();
     void moveLeft();
     void moveRight();
     void flipVertical(undoStatus undoSettings = UNDO);
     void flipHorizontal(undoStatus undoSettings = UNDO);
     void hideName();
     void showName();
     void setIcon(graphicsType);
     void deselect();
     void select();


protected:
    QList<GUIConnector*> mpGUIConnectorPtrs;

    void groupComponents(QList<QGraphicsItem*> compList);
    QGraphicsSvgItem *mpIcon;

    GUIObjectSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;

    int mNameTextPos;
    graphicsType mIconType;
    bool mIconRotation;
    bool mIsFlipped;
    AppearanceData mAppearanceData;
    QPointF mOldPos;

    QList<GUIPort*> mPortListPtrs;

    virtual void createPorts() {assert(false);} //Only availible in GUIComponent for now

protected slots:
    void fixTextPosition(QPointF pos);

private:

};

class GUIObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT
private:
    GUIObject* mpParentGUIObject;

public:
    GUIObjectDisplayName(GUIObject *pParent);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);
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

class GUIContainerObject : public GUIObject
{
    Q_OBJECT
public:
    enum CONTAINERSTATUS {CLOSED, OPEN, ROOT};
    GUIContainerObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUISystem *system=0, QGraphicsItem *parent = 0);
    void makeRootSystem();
    virtual void updateExternalPortPositions();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y);

protected:
    CONTAINERSTATUS getContainerStatus();
    CONTAINERSTATUS mContainerStatus;

};













#endif // GUIOBJECT_H
