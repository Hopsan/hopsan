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
#include <QObject>
#include <QString>

#include <qwt_plot.h>
#include "LogVariable.h"

// Forward Declaration
class PlotTab;
class PlotCurve;
class PlotWindow;


//! @brief Tab widget for plots in plot window
class PlotTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    PlotTabWidget(PlotWindow *pParentPlotWindow);
    PlotTab *getCurrentTab();
    PlotTab *getTab(const int i);
    void closeAllTabs();

public slots:
    void closePlotTab(int index);
};

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotTab;

public:
    PlotWindow(const QString name, QWidget *parent);
    ~PlotWindow();
    PlotCurve* addPlotCurve(SharedLogVariableDataPtrT pData, int axisY=QwtPlot::yLeft, QColor desiredColor=QColor());
    void addBarChart(QStandardItemModel *pItemModel);

    PlotTabWidget *getPlotTabWidget();
    PlotTab *getCurrentPlotTab();

    void showHelpPopupMessage(QString message);
    void hideHelpPopupMessage();

    QString getName() const;

signals:
    void curveAdded();
    void windowClosed(PlotWindow *pWindow);

public slots:
    void setCustomXVector(QVector<double> xarray, const VariableDescription &rVarDesc);
    void setCustomXVector(SharedLogVariableDataPtrT pData);
    void addPlotTab(QString requestedName=QString());
    void updatePalette();
    void createPlotWindowFromTab();
    void saveToXml();
    void importPlo();

    void loadFromXml();
    void performFrequencyAnalysis(PlotCurve *curve);
    void performFrequencyAnalysisFromDialog();


    void showFrequencyAnalysisHelp();
    void createBodePlot();
    void createBodePlotFromDialog();
    void createBodePlot(PlotCurve *pInputCurve, PlotCurve *pOutputCurve, int Fmax);
    void showToolBarHelpPopup();
    void closeIfEmpty();
    void hidePlotCurveInfo();
    void setLegendsVisible(bool value);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

protected slots:
    void changedTab();

private:
    void refreshWindowTitle();

    QString mName;
    QString mModelName;

    QGridLayout *mpLayout;
    QGridLayout *mpInfoBoxLayout;
    QPointF dragStartPosition;

    QToolBar *mpToolBar;

    QAction *mpNewPlotButton;
    QAction *mpArrowButton;
    QAction *mpLegendButton;
    QAction *mpZoomButton;
    QAction *mpOriginalZoomButton;
    QAction *mpPanButton;
    QAction *mpSaveButton;
    QToolButton *mpExportButton;
    QToolButton *mpExportGfxButton;
    QAction *mpLoadFromXmlButton;
    QAction *mpGridButton;
    QAction *mpBackgroundColorButton;
    QAction *mpNewWindowFromTabButton;
    QAction *mpResetXVectorButton;
    QAction *mpAllGenerationsDown;
    QAction *mpAllGenerationsUp;

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

    QDockWidget *mpCurveInfoDock;
    QStackedWidget *mpCurveInfoStack;

    PlotTabWidget *mpPlotTabWidget;

    QMap<QRadioButton*, PlotCurve*> mBodeInputButtonToCurveMap;
    QMap<QRadioButton*, PlotCurve*> mBodeOutputButtonToCurveMap;

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

    QAbstractItemModel *model;
};



#endif // PLOTWINDOW_H
