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
    void updateZoom();

protected:

    QVector<DragPort*> mvSVGPorts;
    QGraphicsSvgItem *mpSVGComponent;
    ModelObjectAppearance *mpCompAppearance;
    PortAppearanceMapT mPortAppearanceMap;

    QGraphicsView *mpView;
    double mViewScale;
    QGridLayout *mpMainLayout;

    QSlider *mpZoomSlider;
    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;
};


class DragPort : public QGraphicsWidget
{
    Q_OBJECT

public:
    DragPort(QString path);

    void setPosOnComponent(QGraphicsItem *component, double x, double y, double rot);
    QPointF getPosOnComponent(QGraphicsItem *component);

protected:
    QGraphicsSvgItem *mpSvg;
};


#endif // MOVEPORTSDIALOG_H
