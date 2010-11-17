#ifndef GUIMODELOBJECT_H
#define GUIMODELOBJECT_H

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"

class GUIConnector;
class GUIModelObjectDisplayName;
class GUIPort;
class GUISystem;

class GUIModelObject : public GUIObject
{
    Q_OBJECT

public:
    GUIModelObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, GUIContainerObject *system = 0, QGraphicsItem *parent = 0);
    ~GUIModelObject();

    //Name methods
    virtual QString getName();
    virtual void refreshDisplayName();
    //virtual void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    virtual void setDisplayName(QString name);
    virtual QString getTypeName();
    virtual int getNameTextPos();
    virtual void setNameTextPos(int textPos);

    //CQS methods
    virtual QString getTypeCQS() {assert(false); return "";} //Only available in GUISystemComponent adn GuiComponent for now
    virtual void setTypeCQS(QString typestring) {assert(false);} //Only available in GUISystemComponent

    //Appearance methods
    virtual GUIModelObjectAppearance* getAppearanceData();
    virtual void refreshAppearance();

    //Parameter methods
    virtual QVector<QString> getParameterNames();
    virtual QString getParameterUnit(QString name) {assert(false); return "";} //Only availible in GUIComponent for now
    virtual QString getParameterDescription(QString name) {assert(false); return "";} //Only availible in GUIComponent for now
    virtual double getParameterValue(QString name);
    virtual void setParameterValue(QString name, double value);

    //Load and save methods
    virtual void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    virtual void saveToDomElement(QDomElement &rDomElement);
    virtual void loadFromHMF(QString modelFilePath=QString()) {assert(false);} //Only available in GUISubsystem for now

    //Connector methods
    void rememberConnector(GUIConnector *item);
    void forgetConnector(GUIConnector *item);

    //Port methods
    void showPorts(bool visible);
    GUIPort *getPort(QString name);
    QList<GUIPort*> &getPortListPtrs();

    //Public members
    GUIModelObjectDisplayName *mpNameText;

    QList<GUIConnector*> getGUIConnectorPtrs();

    enum { Type = GUIMODELOBJECT };
    int type() const;

public slots:
    void deleteMe();
    void rotate(undoStatus undoSettings = UNDO);
    //! @todo flip should work on all gui objects
    void flipVertical(undoStatus undoSettings = UNDO);
    void flipHorizontal(undoStatus undoSettings = UNDO);
    void hideName();
    void showName();
    void setIcon(graphicsType);

signals:
    void groupSelected(QPointF pt);

protected:
    //Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    //Save and load methods
    virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement);

    //Group methods
    //virtual void groupComponents(QList<QGraphicsItem*> compList);

    //Port methods
    virtual void createPorts() {assert(false);} //Only availible in GUIComponent for now

    //Protected members
    GUIModelObjectAppearance mGUIModelObjectAppearance;

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

};


class GUIModelObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT

public:
    GUIModelObjectDisplayName(GUIModelObject *pParent);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);

protected:
    //Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    //Protected members
    GUIModelObject* mpParentGUIModelObject;
};

#endif // GUIMODELOBJECT_H
