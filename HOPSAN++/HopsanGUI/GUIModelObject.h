#ifndef GUIMODELOBJECT_H
#define GUIMODELOBJECT_H

#include "GUIObject.h"
#include "AppearanceData.h"

class GUIConnector;
class GUIModelObjectDisplayName;
class GUIPort;
class GUISystem;

class GUIModelObject : public GUIObject
{
    Q_OBJECT
public:
    GUIModelObject(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, GUISystem *system = 0, QGraphicsItem *parent = 0);
    ~GUIModelObject();

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

    void deleteMe();

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
    //! @todo flip should work on all gui objects
    void flipVertical(undoStatus undoSettings = UNDO);
    void flipHorizontal(undoStatus undoSettings = UNDO);
    void hideName();
    void showName();
    void setIcon(graphicsType);


protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
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
    void snapNameTextPosition(QPointF pos);
    void calcNameTextPositions(QVector<QPointF> &rPts);
    void setNameTextScale(qreal scale);

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

#endif // GUIMODELOBJECT_H
