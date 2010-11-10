//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

#include "common.h"
#include "GUIPortAppearance.h"

//Forward declarations
class GUIModelObject;
class GUISystem;
class CoreSystemAccess;

enum portDirection {TOPBOTTOM, LEFTRIGHT};

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    GUIPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent = 0, CoreSystemAccess *pGUIRootSystem=0);

    QPointF getCenterPos();
    void updatePosition(qreal x, qreal y);
    void updatePositionByFraction(qreal x, qreal y);

    GUISystem *getParentSystem();
    GUIModelObject *getGuiModelObject();

    portDirection getPortDirection();
    qreal getPortHeading();

    void magnify(bool blowup);
    void hide();

    QString getName();
    void setDisplayName(const QString name);
    QString getGUIComponentName();

    bool getLastNodeData(QString dataName, double& rData);

    QString getPortType();
    QString getNodeType();

    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits);
    void setStartValueDataByNames(QVector<QString> names, QVector<double> values);

    void setIsConnected(bool isConnected);
    bool isConnected();

        //Public member variables
    GUISystem *mpParentSystem;
    GUIModelObject *mpParentGuiModelObject;
    QPointF rectPos;

public slots:
    void hideIfNotConnected(bool hidePortsActionTriggered);
    void setVisible(bool value);

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange( GraphicsItemChange change, const QVariant & value );

    void addPortGraphicsOverlay(QString filepath);
    void scalePortOverlay(qreal scalefactor);
    void openRightClickMenu(QPoint screenPos);

protected slots:
    void setPortOverlayScale(qreal scale);

    //protected slots:
public: //! @todo This was made public temporarly to test plot in Python
    bool plot(QString dataName, QString dataUnit=QString());
    void refreshPortOverlayPosition();

private:
    QColor myLineColor;
    qreal myLineWidth;

    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    CoreSystemAccess *mpGUIRootSystem;
    QGraphicsTextItem *mpPortLabel;
    qreal mMag;
    bool mIsMag;

    bool mIsConnected;

    GUIPortAppearance *mpPortAppearance;
    QString name;

    QGraphicsSvgItem* mpPortGraphicsOverlay;
};

QPointF getOffsetPointfromPort(GUIPort *pPort);

#endif // GUIPORT_H
