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
//! @file   PlotWindow.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the PlotWindow class
//!
//$Id$

#ifndef PlotWindow_H
#define PlotWindow_H

#include <QtGui>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include "qwt_legend.h"

#include "qwt_plot_legenditem.h"

#include <QObject>
#include <QFile>
#include <QObject>
#include <QString>



#include "GUIObjects/GUIContainerObject.h"
#include "Dependencies/BarChartPlotter/barchartplotter.h"
#include "Dependencies/BarChartPlotter/axisbase.h"

class MainWindow;
class PlotVariableTree;
class PlotTreeWidget;
class SystemContainer;
class PlotTabWidget;
class PlotTab;
class PlotMarker;
class PlotCurve;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotTreeWidget;                //! @todo Should plot window really be friend with everything?
    friend class VariableListWidget;
    friend class PlotTabWidget;
    friend class PlotTab;
    friend class PlotCurve;
public:
    PlotWindow(PlotVariableTree *PlotVariableTree, MainWindow *parent);
    ~PlotWindow();
    void addPlotCurve(LogVariableData *pData, int axisY=QwtPlot::yLeft, QString modelPath = QString(), QColor desiredColor=QColor());
    void addBarChart(QStandardItemModel *pItemModel);

    PlotTabWidget *getPlotTabWidget();
    PlotTab *getCurrentPlotTab();
    void showHelpPopupMessage(QString message);
    void hideHelpPopupMessage();
    //SystemContainer *mpCurrentGUISystem;

signals:
    void curveAdded();

public slots:
    void changeXVector(QVector<double> xarray, VariableDescription &rVarDesc);
    void addPlotTab(QString requestedName=QString());
    void updatePalette();
    void createPlotWindowFromTab();
    void saveToXml();
    void ImportPlo();

    void loadFromXml();
    void performFrequencyAnalysis(PlotCurve *curve);
    void performFrequencyAnalysisFromDialog();


    void showFrequencyAnalysisHelp();
    void createBodePlot();
    void createBodePlotFromDialog();
    void createBodePlot(PlotCurve *pInputCurve, PlotCurve *pOutputCurve, int Fmax);
    void showToolBarHelpPopup();
    void closeIfEmpty();
    void hideCurveInfo();
    void setLegendsVisible(bool value);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QGridLayout *mpLayout;
    QGridLayout *mpInfoBoxLayout;
    PlotVariableTree *mpPlotVariableTree;
    QPointF dragStartPosition;

    QToolBar *mpToolBar;

    QAction *mpNewPlotButton;
    QAction *mpArrowButton;
    QAction *mpLegendButton;
    QAction *mpZoomButton;
    QAction *mpPanButton;
    QAction *mpSaveButton;
    QToolButton *mpExportButton;
    QToolButton *mpExportGfxButton;
    QAction *mpLoadFromXmlButton;
    QAction *mpGridButton;
    QAction *mpBackgroundColorButton;
    QAction *mpNewWindowFromTabButton;
    QAction *mpResetXVectorButton;

    QAction *mpShowCurveInfoButton;
    QAction *mpShowPlotWidgetButton;

//    QAction *mpShowLegendsAction;
    QAction *mpBodePlotButton;
    QMenu *mpExportMenu;
    QAction *mpExportToXmlAction;
    QAction *mpImportClassicData;
    QAction *mpExportToCsvAction;
    QAction *mpExportToHvcAction;
    QAction *mpExportToMatlabAction;
    QAction *mpExportToGnuplotAction;
    QAction *mpExportToOldHopAction;
    QMenu *mpExportGfxMenu;
    QAction *mpExportPdfAction;
    QAction *mpExportPngAction;
    QAction *mpExportToGraphicsAction;
    QAction *mpLocktheAxis;

    PlotTabWidget *mpPlotTabs;

    QMap<QRadioButton *, PlotCurve *> mBodeInputButtonToCurveMap;
    QMap<QRadioButton *, PlotCurve *> mBodeOutputButtonToCurveMap;

    //Help popup
    QWidget *mpHelpPopup;
    QLabel *mpHelpPopupIcon;
    QLabel *mpHelpPopupLabel;
    QHBoxLayout *mpHelpPopupLayout;
    QGroupBox *mpHelpPopupGroupBox;
    QHBoxLayout *mpHelpPopupGroupBoxLayout;
    QTimer *mpHelpPopupTimer;

    QDialog *mpCreateBodeDialog;
    QSlider *mpMaxFrequencySlider;



    QDialog *mpFrequencyAnalysisDialog;
    QCheckBox *mpLogScaleCheckBox;
    QCheckBox *mpPowerSpectrumCheckBox;
    PlotCurve *mpFrequencyAnalysisCurve;

    //QWidget *mpPlotCurveInfoWidget;
    //QScrollArea *mpPlotCurveInfoScrollArea;
    QVBoxLayout *mpPlotCurveInfoLayout;
    QAbstractItemModel *model;
};



#endif // PLOTWINDOW_H
