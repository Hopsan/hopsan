#ifndef MOVEPORTSDIALOG_H
#define MOVEPORTSDIALOG_H

#include <QtGui>
#include <QtCore>
#include <QFile>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>


class DragPort;

class MovePortsDialog : public QDialog
{
    Q_OBJECT

public:
    MovePortsDialog(QWidget *parent = 0);
    //~MovePortsWidget();

public slots:
    bool close();
    void updateZoom();

protected:

    DragPort *mpPort;
    QGraphicsSvgItem *mpComponent;

    QGraphicsView *mpView;
    double mViewScale;
    QGridLayout *mpMainLayout;

    QSlider *mpZoomSlider;
    QPushButton *mpOkButton;
};


class DragPort : public QGraphicsSvgItem
{
    Q_OBJECT

public:
    DragPort(QString path);

    void setPosOnComponent(QGraphicsItem *component, double x, double y);
    QPointF getPosOnComponent(QGraphicsItem *component);

protected:

};


#endif // MOVEPORTSDIALOG_H
