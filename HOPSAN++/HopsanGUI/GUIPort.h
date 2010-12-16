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
class GUIContainerObject;

enum portDirection {TOPBOTTOM, LEFTRIGHT};

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    GUIPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent = 0);

    QPointF getCenterPos();
    void updatePosition(qreal x, qreal y);
    void updatePositionByFraction(qreal x, qreal y);

    GUIContainerObject *getParentContainerObjectPtr();
    GUIModelObject *getGuiModelObject();

    portDirection getPortDirection();
    qreal getPortHeading();

    void magnify(bool blowup);
    void show();
    void hide();

    QString getName();
    void setDisplayName(const QString name);
    QString getGuiModelObjectName();

    bool getLastNodeData(QString dataName, double& rData);

    QString getPortType();
    QString getNodeType();

    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits);
    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits);
//    void setStartValueDataByNames(QVector<QString> names, QVector<double> values);
    bool setStartValueDataByNames(QVector<QString> names, QVector<QString> valuesTxt);

    void setIsConnected(bool isConnected);
    bool isConnected();

        //Public member variables
    GUIContainerObject *mpParentContainerObject;
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
//    QVariant itemChange( GraphicsItemChange change, const QVariant & value );

    void addPortGraphicsOverlay(QString filepath);

    void openRightClickMenu(QPoint screenPos);

protected slots:
    void setPortOverlayScale(qreal scale);

    //protected slots:
public: //! @todo This was made public temporarly to test plot in Python
    bool plot(QString dataName, QString dataUnit=QString());
    void refreshPortOverlayPosition();
    void refreshPortGraphics();
    void refreshPortGraphics(QString cqsType, QString portType, QString nodeType);

private:
    void setPortOverlayIconScale();

    QColor myLineColor;
    qreal myLineWidth;

    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;

    qreal mMag;
    qreal mOverlaySetScale;
    bool mIsMagnified;
    bool mIsConnected;

    GUIPortAppearance *mpPortAppearance;
    QString mName;

    QGraphicsTextItem *mpPortLabel;
    QGraphicsSvgItem* mpPortGraphicsOverlay;
};

QPointF getOffsetPointfromPort(GUIPort *pStartPortGUIPort, GUIPort *pEndPort);

#endif // GUIPORT_H
