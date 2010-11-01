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

//class ProjectTabWidget;
class GraphicsScene;
//class GraphicsView;
class GUIConnector;
class GUIModelObjectDisplayName;
class GUIObjectSelectionBox;
class GUIPort;
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

    enum { Type = GUIOBJECT };
    int type() const;

public slots:
    void deleteMe();
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
    QPointF mOldPos;
};


class GUIModelObject : public GUIObject
{
    Q_OBJECT
public:
    GUIModelObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, GUISystem *system = 0, QGraphicsItem *parent = 0);

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

    virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    virtual void saveToDomElement(QDomElement &rDomElement);
    virtual void loadFromHMF(QString modelFilePath=QString()) {assert(false);} //Only available in GUISubsystem for now

    enum { Type = GUIMODELOBJECT };
    int type() const;

    //Public Vaariables
    GUIModelObjectDisplayName *mpNameText;

public slots:
    void rotate(undoStatus undoSettings = UNDO);
    //! @todo flip should work on all ui objects
    void flipVertical(undoStatus undoSettings = UNDO);
    void flipHorizontal(undoStatus undoSettings = UNDO);
    void hideName();
    void showName();
    void setIcon(graphicsType);


protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    virtual void saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement);

    void groupComponents(QList<QGraphicsItem*> compList);
    virtual void createPorts() {assert(false);} //Only availible in GUIComponent for now

    //Protected Variables
    AppearanceData mAppearanceData;

    double mTextOffset;
    int mNameTextPos;

    graphicsType mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;

    QList<GUIPort*> mPortListPtrs;
    QList<GUIConnector*> mpGUIConnectorPtrs;

    QGraphicsLineItem *mpTempLine;

protected slots:
    void fixTextPosition(QPointF pos);

private:

};

class GUIModelObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT
public:
    GUIModelObjectDisplayName(GUIModelObject *pParent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    GUIModelObject* mpParentGUIModelObject;
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

class GUIContainerObject : public GUIModelObject
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
