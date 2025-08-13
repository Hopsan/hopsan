/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

#include <QObject>
#include <QDockWidget>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>
#include <QString>
#include <QMainWindow>

#include "PlotTab.h"
#include "PlotCurveStyle.h"
#include "qwt_plot.h"
#include "LogVariable.h"

// Forward Declaration
class PlotCurve;
class PlotWindow;
class HelpPopUpWidget;

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

    QString getName() const;

    PlotTab *addPlotTab(const QString &rName, PlotTabTypeT type=XYPlotType);
    PlotTab *getCurrentPlotTab();
    PlotTabWidget *getPlotTabWidget(); //!< @todo should this really be needed

    void createBodePlot(SharedVectorVariableT var1, SharedVectorVariableT var2, int Fmax, bool bode=true, bool nyquist=false, WindowingFunctionEnumT windowFunction=RectangularWindow, double minTime=-std::numeric_limits<double>::max(), double maxTime=std::numeric_limits<double>::max());

    void showHelpPopupMessage(const QString &rMessage);

    PlotCurve* addPlotCurve(SharedVectorVariableT data, const QwtPlot::Axis axisY=QwtPlot::yLeft, PlotCurveStyle style=PlotCurveStyle());
    PlotCurve* addPlotCurve(SharedVectorVariableT xdata, SharedVectorVariableT ydata, const QwtPlot::Axis axisY=QwtPlot::yLeft, PlotCurveStyle style=PlotCurveStyle());
    void setXData(SharedVectorVariableT xdata, bool force=false);

    void toggleVariablesWidget(bool visible);
    void toggleCurveControls(bool visible);

signals:
    void windowClosed(PlotWindow *pWindow);

public slots:
    PlotTab *addPlotTab();
    void createPlotWindowFromTab();

    void saveToXml();
    void loadFromXml();

    void importPlo();
    void importCsv();

    void updatePalette();
    void hidePlotCurveControls();
    void setLegendsVisible(bool value);

    void closeIfEmpty();
    void closeAllTabs();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

protected slots:
    void changedTab();
    void showToolBarHelpPopup();

private:
    void refreshWindowTitle();
    void setModelPaths(const QStringList &rPaths);

    QString mName;
    QStringList mModelPaths;
    QPointF dragStartPosition;

    PlotTabWidget *mpPlotTabWidget;
    QDockWidget *mpLocalVariablesWidgetDock;
    QDockWidget *mpPlotCurveControlsDock;
    QStackedWidget *mpPlotCurveControlsStack;
    HelpPopUpWidget *mpHelpPopup;

    QToolBar *mpToolBar;
    QAction *mpNewPlotButton;
    QAction *mpArrowButton;
    QAction *mpLegendButton;
    QAction *mpZoomButton;
    QAction *mpOriginalZoomButton;
    QAction *mpPanButton;
    QAction *mpSaveButton;
    QToolButton *mpExportButton;
    QToolButton *mpImportButton;
    QAction *mpLoadFromXmlButton;
    QAction *mpGridButton;
    QAction *mpBackgroundColorButton;
    QAction *mpNewWindowFromTabButton;
    QAction *mpResetXVectorButton;
    QAction *mpAllGenerationsDown;
    QAction *mpAllGenerationsUp;
    QAction *mpBodePlotButton;
    QMenu *mpImportMenu;
    QMenu *mpExportMenu;
    QAction *mpExportToXmlAction;
    QAction *mpImportPloAction;
    QAction *mpImportCsvAction;
    QAction *mpExportToCsvAction;
    QAction *mpExportToHvcAction;
    QAction *mpExportToMatlabAction;
    QAction *mpExportToGnuplotAction;
    QAction *mpExportToOldHopAction;
    QAction *mpExportToHdf5Action;
    QAction *mpExportToGraphicsAction;
    QAction *mpLocktheAxis;
    QAction *mpToggleAxisLockButton;
    QAction *mpOpentimeScaleDialog;
};

#endif // PLOTWINDOW_H
