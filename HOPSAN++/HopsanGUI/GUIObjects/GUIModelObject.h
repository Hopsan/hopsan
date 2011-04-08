//!
//! @file   GUIModelObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIModelObject class (The baseclass for all objects representing model parts)
//!
//$Id$

#ifndef GUIMODELOBJECT_H
#define GUIMODELOBJECT_H

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"
#include <QGraphicsSvgItem>

class GUIConnector;
class GUIModelObjectDisplayName;
class GUIPort;
class GUISystem;

class GUIModelObject : public GUIObject
{
    Q_OBJECT

public:
    GUIModelObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType graphics = USERGRAPHICS, GUIContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~GUIModelObject();

    virtual void setParentContainerObject(GUIContainerObject *pParentContainer);

    //Name methods
    virtual void setName(QString name);
    virtual QString getName();
    virtual void refreshDisplayName();
    virtual void setDisplayName(QString name);
    virtual QString getTypeName();
    virtual int getNameTextPos();
    virtual void setNameTextPos(int textPos);

    //CQS methods
    virtual QString getTypeCQS(){return "hasNoCqsType";} //Overloaded in GUISystem and GUIComponent
    //virtual void setTypeCQS(QString /*typestring*/) {assert(false);} //Only available in GUISystemComponent

    //Appearance methods
    virtual GUIModelObjectAppearance* getAppearanceData();
    virtual void refreshAppearance();

    //Help methods
    QString getHelpPicture();
    QString getHelpText();

    //Parameter methods
    virtual QVector<QString> getParameterNames();
    virtual QString getParameterUnit(QString /*name*/) {assert(false); return "";} //Only availible in GUIComponent for now
    virtual QString getParameterDescription(QString /*name*/) {assert(false); return "";} //Only availible in GUIComponent for now
    virtual double getParameterValue(QString name);
    virtual QString getParameterValueTxt(QString name);
    virtual bool setParameterValue(QString name, QString valueTxt);
    virtual QString getStartValueTxt(QString portName, QString variable);
    virtual bool setStartValue(QString portName, QString variable, QString sysParName);
    //Load and save methods
    virtual void saveToDomElement(QDomElement &rDomElement);
    virtual void loadFromHMF(QString /*modelFilePath=QString()*/) {assert(false);} //Only available in GUISystem for now
    virtual void loadFromDomElement(QDomElement &/*rDomElement*/) {assert(false);} //Only available in GUISystem for now
    virtual void setModelFileInfo(QFile &rFile);

    //Connector methods
    QList<GUIConnector*> getGUIConnectorPtrs();
    void rememberConnector(GUIConnector *item);
    void forgetConnector(GUIConnector *item);

    //Port methods
    void showPorts(bool visible);
    GUIPort *getPort(QString name);
    QList<GUIPort*> &getPortListPtrs();

    //Public members
    GUIModelObjectDisplayName *mpNameText;

    enum { Type = GUIMODELOBJECT };
    int type() const;

    //Temporary - these belong in container object, but they must be here because of the load function
    bool mPortsHidden;
    bool mNamesHidden;

public slots:
    void deleteMe();
    void rotate90cw(undoStatus undoSettings = UNDO);
    void rotate90ccw(undoStatus undoSettings = UNDO);
    //! @todo maybe flip should work on all gui objects
    void flipVertical(undoStatus undoSettings = UNDO);
    void flipHorizontal(undoStatus undoSettings = UNDO);
    void hideName(undoStatus undoSettings = NOUNDO);
    void showName(undoStatus undoSettings = NOUNDO);
    void setIcon(graphicsType);

signals:

protected:
    //Protected methods
    virtual void openPropertiesDialog(){}
    virtual QAction *buildBaseContextMenu(QMenu &rMenue, QGraphicsSceneContextMenuEvent* pEvent);

    //Reimplemented Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    //Save and load methods
    virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement);

    //Port methods
    virtual void createPorts() {assert(false);} //Need to be overloaded

    //Protected members
    GUIModelObjectAppearance mGUIModelObjectAppearance;

    double mTextOffset;
    int mNameTextPos;

    graphicsType mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;
    QString mLastIconPath;

    QList<GUIPort*> mPortListPtrs;
    QList<GUIConnector*> mGUIConnectorPtrs;

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
