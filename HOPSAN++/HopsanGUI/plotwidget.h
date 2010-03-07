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

private slots:
    void enableZoom(bool);
};


class VariablePlot : public QwtPlot
{
public:
    VariablePlot(QWidget *parent = 0);
};


class VariableList : public QListWidget
{
    Q_OBJECT
 public:
    VariableList(QWidget *parent = 0);
    QMap<QString, int> map;

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
