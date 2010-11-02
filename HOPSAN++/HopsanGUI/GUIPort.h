//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

#include "common.h"
#include "GUIPortAppearance.h"

//Forward declarations
class GUIObject;
class GUIModelObject;
class GUISystem;
class CoreSystemAccess;
class GraphicsView;

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    GUIPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent = 0, CoreSystemAccess *pGUIRootSystem=0);
    GUISystem *mpParentSystem;
    GUIModelObject *mpParentGuiModelObject;

    QPointF getCenterPos();
    void updatePosition(qreal x, qreal y);
    void updatePositionByFraction(qreal x, qreal y);
    GUISystem *getParentSystem();
    GUIModelObject *getGuiModelObject();
    void magnify(bool blowup);
    portDirection getPortDirection();
    void setPortDirection(portDirection direction);
    qreal getPortHeading();
    void hide();

    QString getName();
    void setDisplayName(const QString name);
    QString getGUIComponentName();

    QPointF rectPos;
    int getPortNumber();

    bool getLastNodeData(QString dataName, double& rData);

    QString getPortType();
    QString getNodeType();

    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits);
    void setStartValueDataByNames(QVector<QString> names, QVector<double> values);

    bool isConnected;

public slots:
    void hideIfNotConnected(bool hidePortsActionTriggered);
    void setVisible(bool value);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);


    //protected slots:
public: //! @todo This was made public temporarly to test plot in Python
    void plot(QString dataName, QString dataUnit=QString());

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

private:
    QColor myLineColor;
    qreal myLineWidth;

    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    CoreSystemAccess *mpGUIRootSystem;
    QGraphicsTextItem *mpPortLabel;
    qreal mMag;
    bool mIsMag;

    GUIPortAppearance *mpPortAppearance;
//    qreal mXpos;
//    qreal mYpos;
    QString name;

};

#endif // GUIPORT_H
