/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   PlotWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains the PlotWidget and otehr plot related classes
//!
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
//#include <qwt_data.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>


class MainWindow;
class GUIContainerObject;
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
    PlotWindow *getPlotWindow(int number);
    void reportClosedPlotWindow(PlotWindow *window);

    //MainWindow *mpParentMainWindow;
    GUIContainerObject *mpCurrentContainer;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    QPointF dragStartPosition;

public slots:
    void updateList();
    PlotWindow *createPlotWindow(QTreeWidgetItem *item);

private:
    QList<QStringList> mAvailableParameters;
    QList<PlotWindow *> mOpenPlotWindows;
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
