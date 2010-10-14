/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$

#ifndef PlotWindow_H
#define PlotWindow_H


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_data.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_item.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_picker.h>
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


class MainWindow;
class VariablePlot;
class VariablePlotZoomer;
class PlotParameterTree;
class PlotWidget;
class GUISystem;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlotWindow(QVector<double> xarray, QVector<double> yarray, PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(QVector<double> xarray, QVector<double> yarray, QString title, QString xLabel, QString yLabel, QwtPlot::Axis axisY);
    void changeXVector(QVector<double> xarray, QString xLabel, QString componentName, QString portName, QString dataName);
    void insertMarker(QwtPlotCurve *curve);
    void setActiveMarker(QwtPlotMarker *marker);

    MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentGUISystem;

    QVector<QwtPlotCurve *> mpCurves;
    QList<QStringList> mCurveParameters;
    QStringList mSpecialXParameter;

    QwtPlot *mpVariablePlot;
    QwtPlotGrid *mpGrid;
    QwtSymbol *mpMarkerSymbol;
    QwtPlotMarker *mpActiveMarker;
    QVector <QwtPlotMarker *> mpMarkers;
    QHash <QwtPlotCurve *, QwtPlotMarker *> mCurveToMarkerMap;
    QHash <QwtPlotMarker *, QwtPlotCurve *> mMarkerToCurveMap;
    QwtPlotCurve *tempCurve;
    QwtPlotZoomer *mpZoomer;
    QwtPlotMagnifier *mpMagnifier;
    QwtPlotPanner *mpPanner;

    QToolBar *mpToolBar;
    QToolButton *mpZoomButton;
    QToolButton *mpPanButton;
    QToolButton *mpSVGButton;
    QToolButton *mpExportGNUPLOTButton;
    QToolButton *mpImportGNUPLOTButton;
    QToolButton *mpGridButton;
    QToolBar *mpSizeButton;
    QSpinBox *mpSizeSpinBox;
    QToolButton *mpColorButton;
    QToolButton *mpBackgroundColorButton;
    QCheckBox *mpHoldCheckBox;
    QLabel *mpSizeLabel;

    QRubberBand *mpHoverRect;
    //QPainter *mpHoverRect;

    int nCurves;
    QStringList mCurveColors;
    bool mHasSpecialXAxis;
    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;
    bool mHold;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void enableZoom(bool);
    void enablePan(bool);
    void exportSVG();
    void exportGNUPLOT();
    void importGNUPLOT();
    void enableGrid(bool);
    void setLineWidth(int);
    void setLineColor();
    void setBackgroundColor();
    void checkNewValues();
    void setHold(bool value);

private:
    PlotParameterTree *mpPlotParameterTree;
};


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
public:
    PlotParameterTree(MainWindow *parent = 0);
    QList<QStringList> mAvailableParameters;
    MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentSystem;
    void createPlotWindow(QString componentName, QString portName, QString dataName, QString dataUnit);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    QPoint dragStartPosition;

public slots:
    void updateList();
    void createPlotWindow(QTreeWidgetItem *item);

private:
    QList<QStringList> mFavoriteParameters;
};


class PlotWidget : public QWidget
{
    Q_OBJECT
public:
    PlotWidget(MainWindow *parent = 0);
    PlotParameterTree *mpPlotParameterTree;
private:
    MainWindow *mpParentMainWindow;
    QGridLayout *mpLayout;
};

#endif // PlotWindow_H
