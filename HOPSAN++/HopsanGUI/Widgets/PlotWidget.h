//$Id$

#ifndef PlotWidget_H
#define PlotWidget_H

#include <QGridLayout>
#include <iostream>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVector>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHash>
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
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_data.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>


class MainWindow;
class GUISystem;
class PlotWindow;
class PlotWidget;
class PlotParameterTree;


class PlotParameterItem : public QTreeWidgetItem
{
public:
    PlotParameterItem(QString componentName, QString portName, QString dataName, QString dataUnit, QTreeWidgetItem *parent = 0);
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();

private:
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
};


class PlotParameterTree : public QTreeWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotWidget;
public:
    PlotParameterTree(MainWindow *parent = 0);
    PlotWindow *createPlotWindow(QString componentName, QString portName, QString dataName, QString dataUnit);
    PlotWindow *createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit);

    //MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentSystem;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    QPoint dragStartPosition;

public slots:
    void updateList();
    PlotWindow *createPlotWindow(QTreeWidgetItem *item);

private:
    QList<QStringList> mAvailableParameters;
};


class PlotWidget : public QWidget
{
    Q_OBJECT
public:
    PlotWidget(MainWindow *parent = 0);
    PlotParameterTree *mpPlotParameterTree;

public slots:
    void loadFromXml();

private:
    //MainWindow *mpParentMainWindow;
    QPushButton *mpLoadButton;
    QGridLayout *mpLayout;
};

#endif // PlotWidget_H
