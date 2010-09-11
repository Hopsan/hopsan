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
#include <QListWidgetItem>
#include <QListWidget>
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


class MainWindow;
class VariablePlot;
class VariableList;
class VariableListDialog;
class GUISystem;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
public:
    PlotWindow(QVector<double> xarray, QVector<double> yarray, VariableList *variableList, MainWindow *parent);
    void addPlotCurve(QVector<double> xarray, QVector<double> yarray, QString title, QString xLabel, QString yLabel, QwtPlot::Axis axisY);
    void changeXVector(QVector<double> xarray, QString xLabel);

    QVector <QwtPlotCurve *> mpCurves;
    QwtPlotCurve *tempCurve;
    VariablePlot *mpVariablePlot;
    MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentGUISystem;

    QwtPlotZoomer *mpZoomer;
    QwtPlotPanner *mpPanner;
    QwtPlotGrid *mpGrid;

    QToolBar *mpToolBar;
    QToolButton *mpZoomButton;
    QToolButton *mpPanButton;
    QToolButton *mpSVGButton;
    QToolButton *mpGNUPLOTButton;
    QToolButton *mpGridButton;
    QToolBar *mpSizeButton;
    QSpinBox *mpSizeSpinBox;
    QToolButton *mpColorButton;
    QToolButton *mpBackgroundColorButton;
    QLabel *mpSizeLabel;

    QRubberBand *mpHoverRect;

    int nCurves;
    QStringList mCurveColors;
    bool mHasSpecialXAxis;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void enableZoom(bool);
    void enablePan(bool);
    void exportSVG();
    void exportGNUPLOT();
    void enableGrid(bool);
    void setSize(int);
    void setColor();
    void setBackgroundColor();

private:
    VariableList *mpVariableList;
};


class VariablePlot : public QwtPlot
{
public:
    VariablePlot(QWidget *parent = 0);
    void setCurve(QwtPlotCurve *pCurve);
    QwtPlotCurve *getCurve();

private:
    QwtPlotCurve *mpCurve;
};


class VariableList : public QTreeWidget
{
    Q_OBJECT
public:
    VariableList(MainWindow *parent = 0);
    //QHash<QString, int> map;
    QHash< QString, QVector<double> > xMap;
    QHash< QString, QVector<double> > yMap;
    QHash< QString, QString > yLabelMap;
    MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentSystem;

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    QPoint dragStartPosition;

 private slots:
    void updateList();
    void createPlot(QTreeWidgetItem *item);
};


class SelectedVariableList : public VariableList
{
    Q_OBJECT
public:
    SelectedVariableList(MainWindow *parent = 0);
    QHash< QString, QVector<double> > xMap;
    QHash< QString, QVector<double> > yMap;
    MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentSystem;

protected:
    //virtual void mousePressEvent(QMouseEvent *event);
    //virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    QPoint dragStartPosition;

private slots:
    //void createPlot(QListWidgetItem *item);
};


class VariableListDialog : public QWidget
{
    Q_OBJECT
public:
    VariableListDialog(MainWindow *parent = 0);
    VariableList *mpVariableList;
private:
    MainWindow *mpParentMainWindow;
    QPushButton *plotButton;
};

#endif // PlotWindow_H
