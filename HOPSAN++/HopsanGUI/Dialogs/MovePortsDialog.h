#ifndef MOVEPORTSDIALOG_H
#define MOVEPORTSDIALOG_H

#include <QtGui>
#include <QtCore>
#include <QFile>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include "GUIPortAppearance.h"


class DragPort;
class ModelObject;
class Component;
class ModelObjectAppearance;

class MovePortsDialog : public QDialog
{
    Q_OBJECT

public:
    MovePortsDialog(Component *pGUIComponent, QWidget *parent = 0);
    //~MovePortsWidget();

public slots:
    bool close();
    void updateZoom();

protected:

    QVector<DragPort*> mvPorts;
    QGraphicsSvgItem *mpComponent;
    ModelObjectAppearance *mpCompAppearance;
    PortAppearanceMapT mPortAppearanceMap;

    QGraphicsView *mpView;
    double mViewScale;
    QGridLayout *mpMainLayout;

    QSlider *mpZoomSlider;
    QPushButton *mpOkButton;
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
