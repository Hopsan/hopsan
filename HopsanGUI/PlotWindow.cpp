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
//! @file   PlotWindow.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the PlotWindow class
//!
//$Id$

#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>
#include <QDateTime>
#include <QStandardItemModel>
#include <QtGlobal>
#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>

#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/PlotWidget2.h"
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
#include "Utilities/GUIUtilities.h"
#include "Dialogs/OptionsDialog.h"
#include "PlotWindow.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Configuration.h"
#include "loadFunctions.h"
#include "version_gui.h"
#include "ModelHandler.h"
#include "Utilities/HelpPopUpWidget.h"
#include "Utilities/XMLUtilities.h"

#include "qwt_plot.h"

#include "PlotHandler.h"


#include "dependencies/BarChartPlotter/barchartplotter.h"
#include "dependencies/BarChartPlotter/axisbase.h"


//=============================================================
#include "PlotTab.h"
#include "PlotArea.h"
#include "PlotCurve.h"

//! @brief Closes the plot tab with specified index
//! @param index Index of tab to close
void PlotTabWidget::closePlotTab(int index)
{
    PlotTab *theTab = getTab(index);
    theTab->close();
    delete(theTab);
}


//! @brief Returns a pointer to current plot tab
PlotTab *PlotTabWidget::getCurrentTab()
{
    return qobject_cast<PlotTab*>(currentWidget());
}


//! @brief Returns a pointer to plot tab with index i
//! @param i Index of plot tab
PlotTab *PlotTabWidget::getTab(const int i)
{
    return qobject_cast<PlotTab*>(widget(i));
}

void PlotTabWidget::closeAllTabs()
{
    while (count()>0)
    {
        closePlotTab(0);
    }
}


//! @brief Constructor for the plot tab widget
//! @param parent Pointer to the plot window the plot tab widget belongs to
PlotTabWidget::PlotTabWidget(PlotWindow *pParentPlotWindow)
    : QTabWidget(pParentPlotWindow)
{
    setTabsClosable(true);
    setMouseTracking(true);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closePlotTab(int)));
}


//! @brief Constructor for the plot window, where plots are displayed.
//! @param plotVariableTree is a pointer to the variable tree from where the plot window was created
//! @param parent is a pointer to the main window
PlotWindow::PlotWindow(const QString name, QWidget *parent)
    : QMainWindow(parent)
{
    // Set name of Window
    mName = name;
    refreshWindowTitle();

    // Set Window attributes
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_MouseTracking, true);

    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    setPalette(gpConfig->getPalette());

    setStyleSheet(gpConfig->getStyleSheet());

    //setAcceptDrops(false);
    //setAttribute(Qt::WA_TransparentForMouseEvents, false);


    // Set size of Window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    resize(sw*0.7, sh*0.7);   //Resize plot window to 70% of screen height and width
    int w = this->size().width();
    int h = this->size().height();
    int x = (sw - w)/2;
    int y = (sh - h)/2;
    move(x, y);       //Move plot window to center of screen

    //Create the toolbar and its buttons
    mpToolBar = new QToolBar(this);

    mpNewPlotButton = new QAction(this);
    mpNewPlotButton->setToolTip("Create New Plot");
    mpNewPlotButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-NewPlot.svg"));
    connect(mpNewPlotButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpArrowButton = new QAction(this);
    mpArrowButton->setToolTip("Arrow (P)");
    mpArrowButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Pointer.svg"));
    mpArrowButton->setCheckable((true));
    mpArrowButton->setShortcut(QKeySequence("p"));
    //! @todo Check short cut if to confirm if there is a conflict or not
    connect(mpArrowButton, SIGNAL(hovered()),this, SLOT(showToolBarHelpPopup()));

    mpLegendButton = new QAction(this);
    mpLegendButton->setToolTip("Legend (L)");
    mpLegendButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowPlotLegends.svg"));
    //mpLegendButton->setCheckable((true));
    mpLegendButton->setShortcut(QKeySequence("p"));
    //! @todo Check short cut if to confirm if there is a conflict or not
    connect(mpLegendButton, SIGNAL(hovered()),this, SLOT(showToolBarHelpPopup()));

    mpZoomButton = new QAction(this);
    mpZoomButton->setToolTip("Zoom (Z)");
    mpZoomButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Zoom.svg"));
    mpZoomButton->setCheckable(true);
    mpZoomButton->setShortcut(QKeySequence("z"));
    connect(mpZoomButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpOriginalZoomButton = new QAction(this);
    mpOriginalZoomButton->setToolTip("Reset original Zoom");
    mpOriginalZoomButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Zoom100.svg"));


    mpPanButton = new QAction(this);
    mpPanButton->setToolTip("Pan (X)");
    mpPanButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Pan.svg"));
    mpPanButton->setCheckable(true);
    mpPanButton->setShortcut(QKeySequence("x"));
    connect(mpPanButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpSaveButton = new QAction(this);
    mpSaveButton->setToolTip("Save Plot Window Description File (.xml)");
    mpSaveButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Save.svg"));
    mpSaveButton->setShortcut(QKeySequence("Ctrl+s"));
    connect(mpSaveButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
 //   mpSaveButton->setDisabled(true);

    mpImportPloAction = new QAction(this);
    mpImportPloAction->setText("Import from Old Hopsan File (.plo)");
    mpImportPloAction->setToolTip("Import from Old Hopsan File (.plo)");

    mpImportCsvAction = new QAction(this);
    mpImportCsvAction->setText("Import from Comma-Separated Values File (.csv)");
    mpImportCsvAction->setToolTip("Import from Comma-Separated Values File (.csv)");

    mpImportMenu = new QMenu(mpToolBar);
    mpImportMenu->addAction(mpImportPloAction);
    mpImportMenu->addAction(mpImportCsvAction);

    mpImportButton = new QToolButton(mpToolBar);
    mpImportButton->setToolTip("Import Plot Data");
    mpImportButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ImportPlot.svg"));
    mpImportButton->setMenu(mpImportMenu);
    mpImportButton->setPopupMode(QToolButton::InstantPopup);
    mpImportButton->setMouseTracking(true);
    connect(mpImportPloAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToXmlAction = new QAction("Export to Extensible Markup Language File (.xml)", mpToolBar);
    mpExportToCsvAction = new QAction("Export to Comma-Separeted Values File (.csv)", mpToolBar);
    mpExportToHvcAction = new QAction("Export to Hopsan Validation Files (.hvc and .hvd)", mpToolBar);
    mpExportToMatlabAction = new QAction("Export to Matlab Script File (.m)", mpToolBar);
    mpExportToGnuplotAction = new QAction("Export to gnuplot data file(.dat)", mpToolBar);
    mpExportToOldHopAction = new QAction("Export to Hopsan Classic file(.plo)", mpToolBar);
    mpExportToHdf5Action = new QAction("Export to HDF5 file (.h5)", mpToolBar);

    mpExportMenu = new QMenu(mpToolBar);
    mpExportMenu->addAction(mpExportToXmlAction);
    mpExportMenu->addAction(mpExportToCsvAction);
    mpExportMenu->addAction(mpExportToHvcAction);
    mpExportMenu->addAction(mpExportToMatlabAction);
    mpExportMenu->addAction(mpExportToGnuplotAction);
    mpExportMenu->addAction(mpExportToOldHopAction);
    mpExportMenu->addAction(mpExportToHdf5Action);

    mpExportButton = new QToolButton(mpToolBar);
    mpExportButton->setToolTip("Export Plot Tab");
    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportPlot.svg"));
    mpExportButton->setMenu(mpExportMenu);
    mpExportButton->setPopupMode(QToolButton::InstantPopup);
    mpExportButton->setMouseTracking(true);

    mpExportToGraphicsAction = new QAction("Export as Graphics", mpToolBar);
    mpExportToGraphicsAction->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportGfx.svg"));
    mpExportToGraphicsAction->setToolTip("Export to Graphics File");

    mpLoadFromXmlButton = new QAction(this);
    mpLoadFromXmlButton->setToolTip("Import Plot");
    mpLoadFromXmlButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Open.svg"));
    connect(mpLoadFromXmlButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
  //  mpLoadFromXmlButton->setDisabled(true);

    mpGridButton = new QAction(this);
    mpGridButton->setToolTip("Show Grid (G)");
    mpGridButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Grid.svg"));
    mpGridButton->setCheckable(true);
    mpGridButton->setChecked(true);
    mpGridButton->setShortcut(QKeySequence("g"));
    connect(mpGridButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpBackgroundColorButton = new QAction(this);
    mpBackgroundColorButton->setToolTip("Select Canvas Color (C)");
    mpBackgroundColorButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-SelectColor.svg"));
    mpBackgroundColorButton->setShortcut(QKeySequence("c"));
    connect(mpBackgroundColorButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpLocktheAxis = new QAction(this);
    mpLocktheAxis->setToolTip("Set Axis Limits");
    mpLocktheAxis->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-AxisScale.svg"));
    connect(mpLocktheAxis, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpToggleAxisLockButton = new QAction(this);
    mpToggleAxisLockButton->setToolTip("Lock Axis To Current Limits");
    mpToggleAxisLockButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Lock.svg"));
    connect(mpToggleAxisLockButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpOpentimeScaleDialog = new QAction(this);
    mpOpentimeScaleDialog->setToolTip("Open Time-Scale Dialog");
    mpOpentimeScaleDialog->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-AxisTimeScale.svg"));


    mpNewWindowFromTabButton = new QAction(this);
    mpNewWindowFromTabButton->setToolTip("Create Plot Window From Tab");
    mpNewWindowFromTabButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-OpenTabInNewPlotWindow.svg"));
    connect(mpNewWindowFromTabButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpResetXVectorButton = new QAction(this);
    mpResetXVectorButton->setToolTip("Reset Time Vector");
    mpResetXVectorButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ResetTimeVector.svg"));
    connect(mpResetXVectorButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpBodePlotButton = new QAction(this);
    mpBodePlotButton->setToolTip("Transfer Function Analysis");
    mpBodePlotButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-TransferFunctionAnalysis.svg"));
    connect(mpBodePlotButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpAllGenerationsDown = new QAction(this);
    mpAllGenerationsDown->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-StepLeft.svg"));
    mpAllGenerationsDown->setToolTip("Shift model curve generations down");

    mpAllGenerationsUp = new QAction(this);
    mpAllGenerationsUp->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-StepRight.svg"));
    mpAllGenerationsUp->setToolTip("Shift model curve generations up");

    // Initialize the help message popup
    mpHelpPopup = new HelpPopUpWidget(this);

    // Setup PlotVariable List stuff
    PlotWidget2 *pLocalPlotWidget = new PlotWidget2(this);
    pLocalPlotWidget->setPreferedPlotWindow(this);
    mpLocalVariablesWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    mpLocalVariablesWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, mpLocalVariablesWidgetDock);
    mpLocalVariablesWidgetDock->setWidget(pLocalPlotWidget);
    if(gpModelHandler->count() != 0)
    {
        //pLocalPlotWidget->setLogDataHandler(gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()); //!< @todo not necessarily the same as where the plot data will come from if plot by script
        pLocalPlotWidget->setLogDataHandler(gpModelHandler->getCurrentLogDataHandler().data()); //!< @todo not necessarily the same as where the plot data will come from if plot by script
    }

    mpLocalVariablesWidgetDock->toggleViewAction()->setToolTip("Toggle Variable List");
    mpLocalVariablesWidgetDock->toggleViewAction()->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowPlotWindowVariableList.svg"));
    connect(mpLocalVariablesWidgetDock->toggleViewAction(), SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    // Setup CurveInfoBox stuff
    mpPlotCurveControlsStack = new QStackedWidget(this);

    mpPlotCurveControlsDock = new QDockWidget(tr("PlotCurve Settings"), this);
    mpPlotCurveControlsDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, mpPlotCurveControlsDock);
    mpPlotCurveControlsDock->setWidget(mpPlotCurveControlsStack);
    mpPlotCurveControlsDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    mpPlotCurveControlsDock->setPalette(gpConfig->getPalette());
    mpPlotCurveControlsDock->show();

    mpPlotCurveControlsDock->toggleViewAction()->setToolTip("Toggle Curve Controls");
    mpPlotCurveControlsDock->toggleViewAction()->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowPlotWindowCurveSettings.svg"));
    connect(mpPlotCurveControlsDock->toggleViewAction(), SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    // Populate toolbar with actions
    mpToolBar->addAction(mpNewPlotButton);
    mpToolBar->addAction(mpLoadFromXmlButton);
    mpToolBar->addAction(mpSaveButton);
    mpToolBar->addWidget(mpImportButton);
    mpToolBar->addWidget(mpExportButton);
    //mpToolBar->addWidget(mpExportGfxButton);
    mpToolBar->addAction(mpExportToGraphicsAction);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpArrowButton);
    mpToolBar->addAction(mpPanButton);
    mpToolBar->addAction(mpZoomButton);
    mpToolBar->addAction(mpOriginalZoomButton);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpResetXVectorButton);
    mpToolBar->addAction(mpBodePlotButton);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpAllGenerationsDown);
    mpToolBar->addAction(mpAllGenerationsUp);
    mpToolBar->addAction(mpOpentimeScaleDialog);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpGridButton);
    mpToolBar->addAction(mpBackgroundColorButton);
    mpToolBar->addAction(mpLegendButton);
    mpToolBar->addAction(mpLocktheAxis);
    mpToolBar->addAction(mpToggleAxisLockButton);
    mpToolBar->addAction(mpPlotCurveControlsDock->toggleViewAction());
    mpToolBar->addAction(mpLocalVariablesWidgetDock->toggleViewAction());
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpNewWindowFromTabButton);
    mpToolBar->setMouseTracking(true);
    addToolBar(mpToolBar);

    this->setDockOptions(QMainWindow::AllowNestedDocks);

    mpPlotTabWidget = new PlotTabWidget(this);

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpPlotTabWidget,0,0,2,4);
    pLayout->addWidget(mpHelpPopup, 0,0,1,4);

    //Set the correct position of the help popup message in the central widget
    pLayout->setColumnMinimumWidth(0,2);
    pLayout->setColumnStretch(0,0);
    pLayout->setColumnStretch(1,1);
    pLayout->setRowMinimumHeight(0,2);
    pLayout->setRowStretch(0,0);
    pLayout->setRowStretch(1,1);
    pLayout->setRowStretch(2,0);

    QWidget *pCentralWidget = new QWidget(this);
    pCentralWidget->setLayout(pLayout);
    this->setCentralWidget(pCentralWidget);

    // Establish toolbar button signal and slots connections
    connect(mpNewPlotButton,                    SIGNAL(triggered()),            this,               SLOT(addPlotTab()));
    connect(mpLoadFromXmlButton,                SIGNAL(triggered()),            this,               SLOT(loadFromXml()));
    connect(mpImportPloAction,                  SIGNAL(triggered()),            this,               SLOT(importPlo()));
    connect(mpImportCsvAction,                  SIGNAL(triggered()),            this,               SLOT(importCsv()));
    connect(mpSaveButton,                       SIGNAL(triggered()),            this,               SLOT(saveToXml()));
    connect(mpNewWindowFromTabButton,           SIGNAL(triggered()),            this,               SLOT(createPlotWindowFromTab()));
    connect(gpOptionsDialog,   SIGNAL(paletteChanged()),       this,               SLOT(updatePalette()));
    connect(mpPlotTabWidget,                    SIGNAL(currentChanged(int)),    this,               SLOT(changedTab()));

    // Hide curve settings area by default if screen size is to small
    if(sh*sw < 800*1280)
    {
        mpPlotCurveControlsDock->toggleViewAction()->toggle();
    }

    this->setMouseTracking(true);
}

PlotWindow::~PlotWindow()
{
    // Need to close tabs before window is closed and destructors are run
    mpPlotTabWidget->closeAllTabs();
}

PlotTab *PlotWindow::addPlotTab(const QString &rName, PlotTabTypeT type)
{
    PlotTab *pNewTab;
    switch(type)
    {
    case BodePlotType:
        pNewTab = new BodePlotTab(mpPlotTabWidget, this);
        break;
    case BarchartPlotType:
        pNewTab = new BarchartPlotTab(mpPlotTabWidget, this);
        break;
    default:
        // Else use an ordinary XYPlot tab type
        pNewTab = new PlotTab(mpPlotTabWidget, this);
    }

    QString tabName;
    if(rName.isEmpty())
    {
        QString numString;
        for(int tabID=0; true; ++tabID)
        {
            numString.setNum(tabID);
            bool found = false;
            for(int i=0; i<mpPlotTabWidget->count(); ++i)
            {
                if(mpPlotTabWidget->tabText(i) == "Plot " + numString)
                {
                    found=true;
                    break;
                }
            }
            if(!found)
                break;
        }
        tabName = "Plot " + numString;
    }
    else
    {
        tabName = rName;
    }

    mpPlotTabWidget->addTab(pNewTab, tabName);
    mpPlotTabWidget->setCurrentIndex(mpPlotTabWidget->count()-1);
    return pNewTab;
}


PlotTab *PlotWindow::addPlotTab()
{
    return addPlotTab(QString());
}


PlotTabWidget *PlotWindow::getPlotTabWidget()
{
    return mpPlotTabWidget;
}


PlotTab *PlotWindow::getCurrentPlotTab()
{
    return mpPlotTabWidget->getCurrentTab();
}


QString PlotWindow::getName() const
{
    return mName;
}


//! @brief Creates a new plot curve from a plot variable in current container object and adds it to the current plot tab
//! @param[in] pData Shared pointer to data that you want to plot
//! @param[in] axisY  0=left 1=right
//! @param[in] desiredColor The desired color
PlotCurve* PlotWindow::addPlotCurve(SharedVectorVariableT data, const QwtPlot::Axis axisY, PlotCurveStyle style)
{
    if (data)
    {
        // Remember which model it belongs to, and connect the closeWindow signal from the data handler
        // this makes it possible to auto close all plotwindows with data from a particular model(logdatahandler)
        if (data->getLogDataHandler())
        {
            connect(data->getLogDataHandler(), SIGNAL(closePlotsWithOwnedData()), this, SLOT(close()), Qt::UniqueConnection);
        }

        // Make sure that we have a tab
        if (!getCurrentPlotTab())
        {
            addPlotTab();
            changedTab();
        }

        // Create and add a curve
        PlotCurve *pTempCurve = new PlotCurve(data, axisY);
        getCurrentPlotTab()->addCurve(pTempCurve, style);
        return pTempCurve;
    }
    return 0;
}

PlotCurve *PlotWindow::addPlotCurve(SharedVectorVariableT xdata, SharedVectorVariableT ydata, const QwtPlot::Axis axisY, PlotCurveStyle style)
{
    PlotCurve *pCurve = addPlotCurve(ydata, axisY, style);
    if (pCurve)
    {
        pCurve->setCustomXData(xdata);
    }
    return pCurve;
}

void PlotWindow::setXData(SharedVectorVariableT xdata, bool force)
{
    PlotTab *pTab=getCurrentPlotTab();
    // Make sure that we have a tab
    if (!pTab)
    {
        pTab = addPlotTab();
        changedTab();
    }

    pTab->setCustomXVectorForAll(xdata, 0, force);
}

void PlotWindow::toggleVariablesWidget(bool visible)
{
    mpLocalVariablesWidgetDock->setVisible(visible);
}

void PlotWindow::toggleCurveControls(bool visible)
{
    mpPlotCurveControlsDock->setVisible(visible);
}


//! @brief Imports .Plo files from Old Hopsan
//! Imports Plot Data Only
void PlotWindow::importPlo()
{
    gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->importFromPlo();
}


void PlotWindow::importCsv()
{
    gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->importFromCSV_AutoFormat();
}


//! @brief Saves the plot window to XML
//! All generations of all open curves will be saved, together with all cosmetic information about the plot window.
void PlotWindow::saveToXml()
{
    //! @todo fixa
   //Open file dialog and initialize the file stream
   QDir fileDialogSaveDir;
   QString filePath = QFileDialog::getSaveFileName(this, tr("Save Plot Window Specification"),
                                                   gpConfig->getStringSetting(cfg::dir::plotwindow),
                                                   tr("Plot Window Description File (*.xml)"));
   if(filePath.isEmpty())
   {
       return;    //Don't save anything if user presses cancel
   }
   else
   {
       QFileInfo fileInfo = QFileInfo(filePath);
       gpConfig->setStringSetting(cfg::dir::plotwindow, fileInfo.absolutePath());
   }

   //Write to xml file
   QDomDocument domDocument;
   QDomElement xmlRootElement = domDocument.createElement(plotwindow::hopsanplot);
   domDocument.appendChild(xmlRootElement);

   //Add tab elements
   for(int i=0; i<mpPlotTabWidget->count(); ++i)
   {
       QDomElement tabElement = appendDomElement(xmlRootElement,plotwindow::plottab);
       if(mpPlotTabWidget->getTab(i)->isGridVisible())
       {
           tabElement.setAttribute(plotwindow::grid, "true");
       }
       else
       {
           tabElement.setAttribute(plotwindow::grid, "false");
       }
       tabElement.setAttribute(plotwindow::color, makeRgbString(mpPlotTabWidget->getTab(i)->getQwtPlot()->canvasBackground().color()));

       //Add curve elements
       for(int j=0; j<mpPlotTabWidget->getTab(i)->getCurves().size(); ++j)
       {
           auto curve = mpPlotTabWidget->getTab(i)->getCurves().at(j);
           QDomElement curveElement = appendDomElement(tabElement,plotwindow::curve);
           curveElement.setAttribute(plotwindow::component,  curve->getComponentName());
           curveElement.setAttribute(plotwindow::port,       curve->getPortName());
           curveElement.setAttribute(plotwindow::data,       curve->getDataName());
           curveElement.setAttribute(plotwindow::axis,       curve->getAxisY());
           curveElement.setAttribute(plotwindow::width,      curve->pen().width());
           curveElement.setAttribute(plotwindow::color,      makeRgbString(curve->pen().color()));
           if(curve->hasCustomXData()) {
               QDomElement xcurveElement = appendDomElement(curveElement, plotwindow::xcurve);
               auto xcurve = curve->getSharedCustomXVariable();
               xcurveElement.setAttribute(plotwindow::component, xcurve->getComponentName());
               xcurveElement.setAttribute(plotwindow::port, xcurve->getPortName());
               xcurveElement.setAttribute(plotwindow::data, xcurve->getDataName());
           }
       }
   }

   appendRootXMLProcessingInstruction(domDocument);

   //Save to file
   QFile xmlsettings(filePath);
   if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
   {
       qDebug() << "Failed to open file for writing: " << filePath;
       return;
   }
   QTextStream out(&xmlsettings);
   domDocument.save(out, XMLINDENTATION);
}


//! @brief Loads a plot window from XML
void PlotWindow::loadFromXml()
{
    QString filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Plot Window Specification)"),
                                            gpConfig->getStringSetting(cfg::dir::plotwindow),
                                            tr("Plot Window Description File (*.xml)"));

    if(filePath.isEmpty())
    {
        return;    //Don't save anything if user presses cancel
    }
    else
    {
        QFileInfo fileInfo = QFileInfo(filePath);
        gpConfig->setStringSetting(cfg::dir::plotwindow, fileInfo.absolutePath());
    }
    QFile file(filePath);

    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        gpMessageHandler->addErrorMessage("Cannot open file for reading: "+filePath);
        return;
    }

    qDebug() << "Kattunge";

    QXmlStreamReader reader(file.readAll());

    // Read root element
    while (reader.readNextStartElement()) {
        if(reader.name() == plotwindow::hopsanplot)
        {
            qDebug() << "Kattunge: Parsing \"hopsanplot\"";
            while(reader.readNextStartElement()) {
                qDebug() << "Element: " << reader.name();
                if(reader.name() == plotwindow::plottab) {
                    qDebug() << "Kattunge: Parsing \"plottab\"";
                    PlotTab* tab = PlotWindow::addPlotTab();
                    auto attributes = reader.attributes();
                    if(attributes.hasAttribute(plotwindow::grid)) {
                        tab->enableGrid(attributes.value(plotwindow::grid).toString() == "true");
                    }
                    if(attributes.hasAttribute(plotwindow::color)) {
                        double r,g,b;
                        parseRgbString(attributes.value(plotwindow::color).toString(), r, g, b);
                        tab->getPlotArea()->setBackgroundColor(QColor(r, g, b));
                    }

                    while(reader.readNextStartElement()) {
                        if(reader.name() == plotwindow::curve) {
                            qDebug() << "Kattunge: Parsing \"curve\"";
                            QString comp, port, data;
                            int axis=0, width=1;
                            double r=0, g=0, b=0;
                            auto attributes = reader.attributes();
                            if(attributes.hasAttribute(plotwindow::component)) {
                                comp = attributes.value(plotwindow::component).toString();
                            }
                            if(attributes.hasAttribute(plotwindow::port)) {
                                port = attributes.value(plotwindow::port).toString();
                            }
                            if(attributes.hasAttribute(plotwindow::data)) {
                                data = attributes.value(plotwindow::data).toString();
                            }
                            if(attributes.hasAttribute(plotwindow::axis)) {
                                axis = attributes.value(plotwindow::axis).toInt();
                            }
                            if(attributes.hasAttribute(plotwindow::width)) {
                                width = attributes.value(plotwindow::width).toInt();
                            }
                            if(attributes.hasAttribute(plotwindow::color)) {
                                parseRgbString(attributes.value(plotwindow::color).toString(), r, g, b);
                            }
                            QString fullName = makeFullVariableName(QStringList(), comp,port,data);
                            qDebug() << "Looking for variable: " << fullName;
                            auto pLogData = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler();
                            if(pLogData != nullptr) {
                                qDebug() << "Kattunge 1";
                                auto varData = pLogData->getVectorVariable(fullName);
                                if(varData != nullptr) {
                                    qDebug() << "Kattunge 2";
                                    auto pCurve = new PlotCurve(varData);
                                    if(pCurve != nullptr) {
                                        qDebug() << "Kattunge 3";
                                        pCurve->setYAxis(axis);
                                        pCurve->setLineWidth(width);
                                        pCurve->setLineColor(QColor(r,g,b));
                                        tab->addCurve(pCurve);
                                        while(reader.readNextStartElement()) {
                                            if(reader.name() == plotwindow::xcurve) {
                                                qDebug() << "Found xcurve";
                                                QString comp, port, data;
                                                auto attributes = reader.attributes();
                                                if(attributes.hasAttribute(plotwindow::component)) {
                                                    comp = attributes.value(plotwindow::component).toString();
                                                }
                                                if(attributes.hasAttribute(plotwindow::port)) {
                                                    port = attributes.value(plotwindow::port).toString();
                                                }
                                                if(attributes.hasAttribute(plotwindow::data)) {
                                                    data = attributes.value(plotwindow::data).toString();
                                                }
                                                QString fullName = makeFullVariableName(QStringList(), comp,port,data);
                                                pCurve->setCustomXData(fullName);
                                            }
                                        }
                                    }
                                }
                            }

                        }
                        reader.skipCurrentElement();
                    }   // "curve" element
                } // "plottab" element
                reader.skipCurrentElement();
            } // "hopsanplot" element
        }
    }
}


//! @brief Shows the help popup with a message for the currently hovered toolbar item
void PlotWindow::showToolBarHelpPopup()
{
    QCursor cursor;
    QAction *pHoveredAction = mpToolBar->actionAt(mpToolBar->mapFromGlobal(cursor.pos()));
    if(pHoveredAction == mpNewPlotButton)
    {
        mpHelpPopup->showHelpPopupMessage("Create a new empty plot tab.");
    }
    else if(pHoveredAction == mpBodePlotButton)
    {
        mpHelpPopup->showHelpPopupMessage("Performs transfer function analysis to generate nyquist plot and bode diagram.");
    }
    else if(pHoveredAction == mpArrowButton)
    {
        mpHelpPopup->showHelpPopupMessage("Enables to select/revert-back-to the Arrow Pointer.");
    }
    else if(pHoveredAction == mpZoomButton)
    {
        mpHelpPopup->showHelpPopupMessage("Enable zooming tool.");
    }
    else if(pHoveredAction == mpPanButton)
    {
        mpHelpPopup->showHelpPopupMessage("Enable panning tool.");
    }
    else if(pHoveredAction == mpSaveButton)
    {
        mpHelpPopup->showHelpPopupMessage("Save plot window to XML.");
    }
    else if(pHoveredAction == mpLoadFromXmlButton)
    {
        mpHelpPopup->showHelpPopupMessage("Load plot window from XML.");
    }
    else if(pHoveredAction == mpGridButton)
    {
        mpHelpPopup->showHelpPopupMessage("Toggle background grid.");
    }
    else if(pHoveredAction == mpBackgroundColorButton)
    {
        mpHelpPopup->showHelpPopupMessage("Change background color.");
    }
    else if(pHoveredAction == mpNewWindowFromTabButton)
    {
        mpHelpPopup->showHelpPopupMessage("Create new plot window from current tab.");
    }
    else if(pHoveredAction == mpResetXVectorButton)
    {
        mpHelpPopup->showHelpPopupMessage("Reset X-vector to simulation time.");
    }
//    else if(pHoveredAction == mpShowCurveInfoButton)
//    {
//        showHelpPopupMessage("Show/hide plot curve control panel.");
//    }
//    else if(pHoveredAction == mpShowPlotWidgetButton)
//    {
//        showHelpPopupMessage("Show/hide variable lists.");
//    }
//    else if(pHoveredAction == mpImportButton)
//    {
//        showHelpPopupMessage("Import Data from Old Hopsan.");
//    }
    else if(pHoveredAction == mpLegendButton)
    {
        mpHelpPopup->showHelpPopupMessage("Open the legend settings dialog.");
    }
    else if(pHoveredAction == mpLocktheAxis)
    {
        mpHelpPopup->showHelpPopupMessage("Open the lock axis dialog.");
    }
    else if(pHoveredAction == mpToggleAxisLockButton)
    {
        mpHelpPopup->showHelpPopupMessage("Lock all axes to current limits.");
    }
}


//! @brief Slot that closes all tabs with no curves, and then closes the entire plot window if it has no curves.
void PlotWindow::closeIfEmpty()
{
    // Close each plottab that is empty (no curves)
    int i=0;
    while (i<mpPlotTabWidget->count())
    {
        if(mpPlotTabWidget->getTab(i)->isEmpty())
        {
            mpPlotTabWidget->closePlotTab(i);
        }
        else
        {
            ++i;
        }
    }

    // Close the entire plot window if all tabs have been closed
    if(mpPlotTabWidget->count() == 0)
    {
        close();
    }
}

void PlotWindow::closeAllTabs()
{
    mpPlotTabWidget->closeAllTabs();
}


void PlotWindow::hidePlotCurveControls()
{
    mpPlotCurveControlsDock->toggleViewAction()->setChecked(false);
}


void PlotWindow::setLegendsVisible(bool value)
{
    for(int i=0; i<mpPlotTabWidget->count(); ++i)
    {
        mpPlotTabWidget->getTab(i)->setLegendsVisible(value);
    }
}


void PlotWindow::mouseMoveEvent(QMouseEvent *event)
{
    mpHelpPopup->hide();
    QMainWindow::mouseMoveEvent(event);
}


//! @brief Reimplementation of close function for plot window. Notifies others that window no longer exists.
void PlotWindow::closeEvent(QCloseEvent *event)
{
    emit windowClosed(this);
    QMainWindow::closeEvent(event);
}

void PlotWindow::changedTab()
{
    // First disconnect all current connections (in case the tab is being changed)
    disconnect(mpZoomButton,                SIGNAL(toggled(bool)),  0,  0);
    disconnect(mpOriginalZoomButton,        SIGNAL(triggered()),    0,  0);
    disconnect(mpArrowButton,               SIGNAL(toggled(bool)),  0,  0);
    disconnect(mpPanButton,                 SIGNAL(toggled(bool)),  0,  0);
    disconnect(mpBackgroundColorButton,     SIGNAL(triggered()),    0,  0);
    disconnect(mpGridButton,                SIGNAL(toggled(bool)),  0,  0);
    disconnect(mpResetXVectorButton,        SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToCsvAction,         SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToHvcAction,         SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToXmlAction,         SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToOldHopAction,      SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToHdf5Action,        SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToMatlabAction,      SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToGnuplotAction,     SIGNAL(triggered()),    0,  0);
    disconnect(mpExportToGraphicsAction,    SIGNAL(triggered()),    0,  0);
    disconnect(mpLegendButton,              SIGNAL(triggered()),    0,  0);
    disconnect(mpLocktheAxis,               SIGNAL(triggered()),    0,  0);
    disconnect(mpAllGenerationsDown,        SIGNAL(triggered()),    0,  0);
    disconnect(mpAllGenerationsUp,          SIGNAL(triggered()),    0,  0);
    disconnect(mpOpentimeScaleDialog,       SIGNAL(triggered()),    0,  0);
    disconnect(mpToggleAxisLockButton,      SIGNAL(triggered()),    0,  0);
    disconnect(mpBodePlotButton,            SIGNAL(triggered()),    0,  0);

    // If there are any tabs then show the widget and reestablish connections to the current tab
    if(mpPlotTabWidget->count() > 0)
    {
        mpPlotTabWidget->show();
        PlotTab* pCurrentTab = mpPlotTabWidget->getCurrentTab();

        if(pCurrentTab->isBarPlot())
        {
            mpArrowButton->setDisabled(true);
            mpZoomButton->setDisabled(true);
            mpOriginalZoomButton->setDisabled(true);
            mpPanButton->setDisabled(true);
            //mpSaveButton->setDisabled(true);
            mpExportToCsvAction->setDisabled(true);
            mpExportToHvcAction->setDisabled(true);
            mpExportToGnuplotAction->setDisabled(true);
            mpExportToOldHopAction->setDisabled(true);
            mpExportToHdf5Action->setDisabled(true);
            mpExportToMatlabAction->setDisabled(true);
            //mpLoadFromXmlButton->setDisabled(true);
            mpGridButton->setDisabled(true);
            mpBackgroundColorButton->setDisabled(true);
            mpNewWindowFromTabButton->setDisabled(true);
            mpResetXVectorButton->setDisabled(true);
            mpBodePlotButton->setDisabled(true);
            mpExportToGraphicsAction->setDisabled(true);
            mpAllGenerationsDown->setDisabled(true);
            mpAllGenerationsUp->setDisabled(true);
        }
        else
        {
            mpArrowButton->setDisabled(false);
            mpZoomButton->setDisabled(false);
            mpOriginalZoomButton->setDisabled(false);
            mpPanButton->setDisabled(false);
            mpImportPloAction->setDisabled(false);
            //mpSaveButton->setDisabled(false);
            mpExportToCsvAction->setDisabled(false);
            mpExportToHvcAction->setDisabled(false);
            mpExportToGnuplotAction->setDisabled(false);
            mpExportToOldHopAction->setDisabled(false);
            mpExportToHdf5Action->setDisabled(false);
            mpExportToMatlabAction->setDisabled(false);
            //mpLoadFromXmlButton->setDisabled(false);
            mpGridButton->setDisabled(false);
            mpBackgroundColorButton->setDisabled(false);
            mpNewWindowFromTabButton->setDisabled(false);
            mpResetXVectorButton->setDisabled(false);
            mpBodePlotButton->setDisabled(false);
            mpExportToGraphicsAction->setDisabled(false);
            mpAllGenerationsDown->setDisabled(false);
            mpAllGenerationsUp->setDisabled(false);
            mpZoomButton->setChecked(pCurrentTab->isZoomEnabled());
            mpPanButton->setChecked(pCurrentTab->isPanEnabled());
            mpGridButton->setChecked(pCurrentTab->isGridVisible());
            //mpResetXVectorButton->setEnabled(pCurrentTab->hasCustomXData());
            //mpBodePlotButton->setEnabled(pCurrentTab->getCurves(0).size() > 1); //!< @todo check this in some better way
        }

        connect(mpZoomButton,               SIGNAL(toggled(bool)),  pCurrentTab,    SLOT(enableZoom(bool)));
        connect(mpOriginalZoomButton,       SIGNAL(triggered()),    pCurrentTab,    SLOT(resetZoom()));
        connect(mpArrowButton,              SIGNAL(toggled(bool)),  pCurrentTab,    SLOT(enableArrow(bool)));
        connect(mpPanButton,                SIGNAL(toggled(bool)),  pCurrentTab,    SLOT(enablePan(bool)));
        connect(mpBackgroundColorButton,    SIGNAL(triggered()),    pCurrentTab,    SLOT(setBackgroundColor()));
        connect(mpGridButton,               SIGNAL(toggled(bool)),  pCurrentTab,    SLOT(enableGrid(bool)));
        connect(mpResetXVectorButton,       SIGNAL(triggered()),    pCurrentTab,    SLOT(resetXTimeVector()));
        connect(mpExportToXmlAction,        SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToXml()));
        connect(mpExportToCsvAction,        SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToCsv()));
        connect(mpExportToHvcAction,        SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToHvc()));
        connect(mpExportToMatlabAction,     SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToMatlab()));
        connect(mpExportToGnuplotAction,    SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToGnuplot()));
        connect(mpExportToOldHopAction,     SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToPLO()));
        connect(mpExportToHdf5Action,       SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToHDF5()));
        connect(mpExportToGraphicsAction,   SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToGraphics()));
        connect(mpLegendButton,             SIGNAL(triggered()),    pCurrentTab,    SLOT(openLegendSettingsDialog()));
        connect(mpLocktheAxis,              SIGNAL(triggered()),    pCurrentTab,    SLOT(openAxisSettingsDialog()));
        connect(mpOpentimeScaleDialog,      SIGNAL(triggered()),    pCurrentTab,    SLOT(openTimeOffsetDialog()));
        connect(mpAllGenerationsDown,       SIGNAL(triggered()),    pCurrentTab,    SLOT(shiftAllGenerationsDown()));
        connect(mpAllGenerationsUp,         SIGNAL(triggered()),    pCurrentTab,    SLOT(shiftAllGenerationsUp()));
        connect(mpToggleAxisLockButton,     SIGNAL(triggered()),    pCurrentTab,    SLOT(toggleAxisLock()));
        connect(mpBodePlotButton,           SIGNAL(triggered()),    pCurrentTab,    SLOT(openCreateBodePlotDialog()));

        // Set the plottab specific info layout
        mpPlotCurveControlsStack->setCurrentWidget(pCurrentTab->mpCurveInfoScrollArea);
    }
    else
    {
        mpPlotTabWidget->hide();
    }
}

void PlotWindow::refreshWindowTitle()
{
    if (mModelPaths.isEmpty())
    {
        setWindowTitle(mName);
    }
    else
    {
        QStringList models;
        for (int i=0; i<mModelPaths.size(); ++i)
        {
            QFileInfo file(mModelPaths[i]);
            models.append(file.fileName());
        }
        setWindowTitle(mName + " (" + models.join(", ") + ")");
    }
}

void PlotWindow::setModelPaths(const QStringList &rPaths)
{
    mModelPaths = rPaths;
    refreshWindowTitle();
}


//! @brief Slot that updates the palette to match the one used in main window
void PlotWindow::updatePalette()
{
    setPalette(gpMainWindowWidget->palette());
}


//! @brief Creates a new plot window and adds the curves from current plot tab
void PlotWindow::createPlotWindowFromTab()
{
    //! @todo should be in tab instead and have signal slot
    PlotWindow *pPW = 0;
    for(int i=0; i<getCurrentPlotTab()->getCurves().size(); ++i)
    {
        SharedVectorVariableT data = getCurrentPlotTab()->getCurves().at(i)->getSharedVectorVariable();
        pPW = gpPlotHandler->plotDataToWindow(pPW, data, getCurrentPlotTab()->getCurves().at(i)->getAxisY());
    }
}


//! @todo should not run code on non bodeplot tabs
void PlotWindow::createBodePlot(SharedVectorVariableT var1, SharedVectorVariableT var2, int Fmax, bool bode, bool nyquist, WindowingFunctionEnumT windowFunction, double minTime, double maxTime)
{
    SharedVectorVariableT pNyquist, pNyquistInv, pGain, pPhase;
    createBodeVariables(var1, var2, Fmax, pNyquist, pNyquistInv, pGain, pPhase, windowFunction, minTime, maxTime);

    // Nyquist plot
    if(nyquist) {
        PlotTab *pNyquistTab = addPlotTab("Nyquist Plot");
        PlotCurveStyle blueStyle;
        blueStyle.color = QColor("Blue");
        pNyquistTab->addCurve(new PlotCurve(pNyquist, QwtPlot::yLeft, NyquistType), blueStyle);
        pNyquistTab->addCurve(new PlotCurve(pNyquistInv, QwtPlot::yLeft, NyquistType), blueStyle);
    }

    // Bode plot
    if(bode) {
        PlotTab *pBodeTab = addPlotTab("Bode Diagram", BodePlotType);
        PlotCurve *pGainCurve = new PlotCurve(pGain, QwtPlot::yLeft, BodeGainType);
        pBodeTab->getPlotArea(BodePlotTab::MagnitudePlot)->addCurve(pGainCurve);
        pBodeTab->getPlotArea(BodePlotTab::MagnitudePlot)->setBottomAxisLogarithmic(true);
        pGainCurve->setCurveDataUnitScale(gpConfig->getUnitScaleUC(pGain->getDataQuantity(), "dB"));

        PlotCurve *pPhaseCurve = new PlotCurve(pPhase, QwtPlot::yLeft, BodePhaseType);
        pBodeTab->getPlotArea(BodePlotTab::PhasePlot)->addCurve(pPhaseCurve);
        pBodeTab->getPlotArea(BodePlotTab::PhasePlot)->setBottomAxisLogarithmic(true);
        pPhaseCurve->setCurveDataUnitScale(gpConfig->getUnitScaleUC(pPhaseCurve->getDataQuantity(), "deg"));

        pBodeTab->rescaleAxesToCurves();

        //! @todo this should not happen here
        SharedVectorVariableT gainVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->defineNewVectorVariable("bodegain");
        if(gainVar.isNull())
        {
            gainVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getVectorVariable("bodegain",-1);
        }
        gainVar.data()->assignFrom(pGain);

        SharedVectorVariableT phaseVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->defineNewVectorVariable("bodephase");
        if(phaseVar.isNull())
        {
            phaseVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getVectorVariable("bodegain",-1);
        }
        phaseVar.data()->assignFrom(pPhase);
    }
}

void PlotWindow::showHelpPopupMessage(const QString &rMessage)
{
    mpHelpPopup->showHelpPopupMessage(rMessage);
}
