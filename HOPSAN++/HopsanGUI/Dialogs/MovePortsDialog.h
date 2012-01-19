#ifndef MOVEPORTSDIALOG_H
#define MOVEPORTSDIALOG_H

#include <QtGui>
#include <QtCore>
#include <QFile>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include "GUIPortAppearance.h"
#include "common.h"


class DragPort;
class ModelObject;
class Component;
class ModelObjectAppearance;


class MovePortsDialog : public QDialog
{
    Q_OBJECT

public:
    MovePortsDialog(ModelObjectAppearance *pComponentAppearance, graphicsType gfxType = USERGRAPHICS, QWidget *parent = 0);
    //~MovePortsWidget();

public slots:
    bool okButtonPressed();
    bool cancelButtonPressed();
    void updatePortXPos(QString x);
    void updatePortYPos(QString y);
    void updatePortInfo(QString portName, QString x, QString y);
    void updateZoom();

signals:
    void finished();

protected:
    QVector<DragPort*> mvSVGPorts;
    QGraphicsSvgItem *mpSVGComponent;
    ModelObjectAppearance *mpCompAppearance;
    PortAppearanceMapT *mpPortAppearanceMap;
    QMap<QString,DragPort*> mDragPortMap;

    QGraphicsView *mpView;
    double mViewScale;
    QGridLayout *mpMainLayout;

    QSlider *mpZoomSlider;
    QLabel *mpSelectedPortLabel;
    QLabel *mpPortNameLabel;
    QLabel *mpSelectedPortXLabel;
    QLineEdit *mpPortXLineEdit;
    QLabel *mpSelectedPortYLabel;
    QLineEdit *mpPortYLineEdit;
    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;
};


class DragPort : public QGraphicsWidget
{
    Q_OBJECT

public:
    DragPort(PortAppearance *appearance, QString name, QGraphicsItem *parentComponent);

    void setPosOnComponent(double x, double y, double rot);
    QPointF getPosOnComponent();

signals:
    void activePort(QString portName, QString x, QString y);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void portMoved();

    QGraphicsItem *mpParentComponent;
    QGraphicsSvgItem *mpSvg;
    QGraphicsTextItem *mpName;
};


#endif // MOVEPORTSDIALOG_H
