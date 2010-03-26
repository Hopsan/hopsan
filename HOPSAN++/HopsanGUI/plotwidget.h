//$Id$

#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_data.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_item.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_picker.h>
#include <QGridLayout>
#include <iostream>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVector>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QToolBar>
#include <QToolButton>
#include <QMainWindow>
#include <QColor>
#include <QMouseEvent>
#include <QApplication>
#include <QDragMoveEvent>
#include <qwt_legend.h>
#include <QFileDialog>
#include <QSvgGenerator>


class VariablePlot;

class PlotWidget : public QMainWindow
{
    Q_OBJECT
public:
    PlotWidget(QVector<double> xarray, QVector<double> yarray, QWidget *parent = 0);

    QwtPlotCurve *mpCurve;
    VariablePlot *mpVariablePlot;

    QwtPlotZoomer *zoomer;
    QwtPlotPanner *panner;

    QToolButton *btnZoom;
    QToolButton *btnPan;
    QToolButton *btnSVG;

private slots:
    void enableZoom(bool);
    void enablePan(bool);
    void exportSVG();
};


class VariablePlot : public QwtPlot
{
public:
    VariablePlot(QWidget *parent = 0);

protected:
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

};


class VariableList : public QListWidget
{
    Q_OBJECT
 public:
    VariableList(QWidget *parent = 0);
    QMap<QString, int> map;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    QPoint dragStartPosition;

 private slots:
    void createPlot(QListWidgetItem *item);

};


class VariableListDialog : public QWidget
{
    Q_OBJECT
public:
    VariableListDialog(QWidget *parent = 0);
};

#endif // PLOTWIDGET_H
