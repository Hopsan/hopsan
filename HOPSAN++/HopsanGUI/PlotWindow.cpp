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
#include <QtGui>

#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/PlotWidget.h"
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
#include "Utilities/GUIUtilities.h"
#include "Dialogs/OptionsDialog.h"
#include "GUIObjects/GUISystem.h"
#include "MainWindow.h"
#include "PlotWindow.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Configuration.h"
#include "loadFunctions.h"
#include "version_gui.h"
#include "ModelHandler.h"
#include "Utilities/HelpPopUpWidget.h"

#include "qwt_plot.h"

#include "PlotHandler.h"


#include "Dependencies/BarChartPlotter/barchartplotter.h"
#include "Dependencies/BarChartPlotter/axisbase.h"


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

    //setAcceptDrops(false);
    //setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setPalette(gpConfig->getPalette());

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
    mpNewPlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
    connect(mpNewPlotButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpArrowButton = new QAction(this);
    mpArrowButton->setToolTip("Arrow (P)");
    mpArrowButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Pointer.png"));
    mpArrowButton->setCheckable((true));
    mpArrowButton->setShortcut(QKeySequence("p"));
    //! @todo Check short cut if to confirm if there is a conflict or not
    connect(mpArrowButton, SIGNAL(hovered()),this, SLOT(showToolBarHelpPopup()));

    mpLegendButton = new QAction(this);
    mpLegendButton->setToolTip("Legend (L)");
    mpLegendButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotLegends.png"));
    //mpLegendButton->setCheckable((true));
    mpLegendButton->setShortcut(QKeySequence("p"));
    //! @todo Check short cut if to confirm if there is a conflict or not
    connect(mpLegendButton, SIGNAL(hovered()),this, SLOT(showToolBarHelpPopup()));

    mpZoomButton = new QAction(this);
    mpZoomButton->setToolTip("Zoom (Z)");
    mpZoomButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Zoom.png"));
    mpZoomButton->setCheckable(true);
    mpZoomButton->setShortcut(QKeySequence("z"));
    connect(mpZoomButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpOriginalZoomButton = new QAction(this);
    mpOriginalZoomButton->setToolTip("Reset original Zoom");
    mpOriginalZoomButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Zoom100.png"));


    mpPanButton = new QAction(this);
    mpPanButton->setToolTip("Pan (X)");
    mpPanButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Pan.png"));
    mpPanButton->setCheckable(true);
    mpPanButton->setShortcut(QKeySequence("x"));
    connect(mpPanButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpSaveButton = new QAction(this);
    mpSaveButton->setToolTip("Save Plot Window Description File (.xml)");
    mpSaveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Save.png"));
    mpSaveButton->setShortcut(QKeySequence("Ctrl+s"));
    connect(mpSaveButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mpSaveButton->setDisabled(true);

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
    mpImportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ImportPlot.png"));
    mpImportButton->setMenu(mpImportMenu);
    mpImportButton->setPopupMode(QToolButton::InstantPopup);
    mpImportButton->setMouseTracking(true);
    connect(mpImportPloAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToXmlAction = new QAction("Export to Extensible Markup Language File (.xml)", mpToolBar);
    mpExportToCsvAction = new QAction("Export to Comma-Separeted Values File (.csv)", mpToolBar);
    mpExportToHvcAction = new QAction("Export to Hopsan Validation Files (.hvc and .csv)", mpToolBar);
    mpExportToMatlabAction = new QAction("Export to Matlab Script File (.m)", mpToolBar);
    mpExportToGnuplotAction = new QAction("Export to gnuplot data file(.dat)", mpToolBar);
    mpExportToOldHopAction = new QAction("Export to Hopsan Classic file(.plo)", mpToolBar);

    mpExportMenu = new QMenu(mpToolBar);
    mpExportMenu->addAction(mpExportToXmlAction);
    mpExportMenu->addAction(mpExportToCsvAction);
    mpExportMenu->addAction(mpExportToHvcAction);
    mpExportMenu->addAction(mpExportToMatlabAction);
    mpExportMenu->addAction(mpExportToGnuplotAction);
    mpExportMenu->addAction(mpExportToOldHopAction);

    mpExportButton = new QToolButton(mpToolBar);
    mpExportButton->setToolTip("Export Plot Tab");
    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ExportPlot.png"));
    mpExportButton->setMenu(mpExportMenu);
    mpExportButton->setPopupMode(QToolButton::InstantPopup);
    mpExportButton->setMouseTracking(true);

    mpExportToGraphicsAction = new QAction("Export as Graphics", mpToolBar);
    mpExportToGraphicsAction->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ExportGfx.png"));
    mpExportToGraphicsAction->setToolTip("Export to Graphics File");

    mpLoadFromXmlButton = new QAction(this);
    mpLoadFromXmlButton->setToolTip("Import Plot");
    mpLoadFromXmlButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Open.png"));
    connect(mpLoadFromXmlButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mpLoadFromXmlButton->setDisabled(true);

    mpGridButton = new QAction(this);
    mpGridButton->setToolTip("Show Grid (G)");
    mpGridButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Grid.png"));
    mpGridButton->setCheckable(true);
    mpGridButton->setChecked(true);
    mpGridButton->setShortcut(QKeySequence("g"));
    connect(mpGridButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpBackgroundColorButton = new QAction(this);
    mpBackgroundColorButton->setToolTip("Select Canvas Color (C)");
    mpBackgroundColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-BackgroundColor.png"));
    mpBackgroundColorButton->setShortcut(QKeySequence("c"));
    connect(mpBackgroundColorButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpLocktheAxis = new QAction(this);
    mpLocktheAxis->setToolTip("Lock Axis");
    mpLocktheAxis->setIcon(QIcon(QString(ICONPATH) + "Hopsan-PlotCurveScale.png"));
    connect(mpLocktheAxis, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpToggleAxisLockButton = new QAction(this);
    mpToggleAxisLockButton->setToolTip("Lock Axis To Current Limits");
    mpToggleAxisLockButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Lock.png"));
    connect(mpToggleAxisLockButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpOpentimeScaleDialog = new QAction(this);
    mpOpentimeScaleDialog->setToolTip("Open Time-Scale Dialog");
    mpOpentimeScaleDialog->setIcon(QIcon(QString(ICONPATH) + "Hopsan-AxisTimeScale.png"));


    mpNewWindowFromTabButton = new QAction(this);
    mpNewWindowFromTabButton->setToolTip("Create Plot Window From Tab");
    mpNewWindowFromTabButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-OpenTabInNewPlotWindow.png"));
    connect(mpNewWindowFromTabButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpResetXVectorButton = new QAction(this);
    mpResetXVectorButton->setToolTip("Reset Time Vector");
    mpResetXVectorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    connect(mpResetXVectorButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpBodePlotButton = new QAction(this);
    mpBodePlotButton->setToolTip("Transfer Function Analysis");
    mpBodePlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-TransferFunctionAnalysis.png"));
    connect(mpBodePlotButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpAllGenerationsDown = new QAction(this);
    mpAllGenerationsDown->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepLeft.png"));
    mpAllGenerationsDown->setToolTip("Shift all curve generations down");

    mpAllGenerationsUp = new QAction(this);
    mpAllGenerationsUp->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepRight.png"));
    mpAllGenerationsUp->setToolTip("Shift all curve generations up");

    // Initialize the help message popup
    mpHelpPopup = new HelpPopUpWidget(this);

    // Setup PlotVariable List stuff
    PlotWidget *pLocalPlotWidget = new PlotWidget(this);
    pLocalPlotWidget->setPreferedPlotWindow(this);
    QDockWidget *pLocalPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    pLocalPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, pLocalPlotWidgetDock);
    pLocalPlotWidgetDock->setWidget(pLocalPlotWidget);
    if(gpModelHandler->count() != 0)
    {
        pLocalPlotWidget->setLogDataHandler(gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()); //!< @todo not necessarily the same as where the plot data will come from if plot by script
    }

    pLocalPlotWidgetDock->toggleViewAction()->setToolTip("Toggle Variable List");
    pLocalPlotWidgetDock->toggleViewAction()->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowVariableList.png"));
    connect(pLocalPlotWidgetDock->toggleViewAction(), SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

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
    mpPlotCurveControlsDock->toggleViewAction()->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowCurveSettings.png"));
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
    mpToolBar->addAction(pLocalPlotWidgetDock->toggleViewAction());
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
    connect(gpMainWindow->getOptionsDialog(),   SIGNAL(paletteChanged()),       this,               SLOT(updatePalette()));
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


void PlotWindow::setCustomXVector(QVector<double> xarray, const VariableDescription &rVarDesc)
{
    getCurrentPlotTab()->setCustomXVectorForAll(xarray, rVarDesc, 0);
}

void PlotWindow::setCustomXVector(SharedVectorVariableT pData)
{
    getCurrentPlotTab()->setCustomXVectorForAll(pData);
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
PlotCurve* PlotWindow::addPlotCurve(HopsanVariable data, const QwtPlot::Axis axisY, QColor desiredColor)
{
    if (data)
    {
        // Remember which model it belongs to, and connect the closeWindow signal from the data handler
        // this makes it possible to autoclose all plotwindows with data from a particular model(logdatahandler)
        if (data.mpVariable->getLogDataHandler())
        {
            connect(data.mpVariable->getLogDataHandler(), SIGNAL(closePlotsWithOwnedData()), this, SLOT(close()), Qt::UniqueConnection);
        }

        // Make sure that we have a tab
        if (!getCurrentPlotTab())
        {
            addPlotTab();
            changedTab();
        }

        // Create and add a curve
        PlotCurve *pTempCurve = new PlotCurve(data, axisY);
        getCurrentPlotTab()->addCurve(pTempCurve, desiredColor);
        return pTempCurve;
    }
    return 0;
}

PlotCurve *PlotWindow::addPlotCurve(HopsanVariable xdata, HopsanVariable ydata, const QwtPlot::Axis axisY, QColor desiredColor)
{
    PlotCurve *pCurve = addPlotCurve(ydata, axisY, desiredColor);
    if (pCurve)
    {
        pCurve->setCustomXData(xdata);
    }
    return pCurve;
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
//    //Open file dialog and initialize the file stream
//    QDir fileDialogSaveDir;
//    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Plot Window Description to XML"),
//                                                    gpConfig->getPlotWindowDir(),
//                                                    tr("Plot Window Description File (*.xml)"));
//    if(filePath.isEmpty())
//    {
//        return;    //Don't save anything if user presses cancel
//    }
//    else
//    {
//        QFileInfo fileInfo = QFileInfo(filePath);
//        gpConfig->setPlotWindowDir(fileInfo.absolutePath());
//    }

//    //Write to xml file
//    QDomDocument domDocument;
//    QDomElement xmlRootElement = domDocument.createElement("hopsanplot");
//    domDocument.appendChild(xmlRootElement);

//    QDomElement dateElement = appendDomElement(xmlRootElement,"date");      //Append date tag
//    QDate date = QDate::currentDate();
//    dateElement.setAttribute("year", date.year());
//    dateElement.setAttribute("month", date.month());
//    dateElement.setAttribute("day", date.day());

//    QDomElement timeElement = appendDomElement(xmlRootElement,"time");      //Append time tag
//    QTime time = QTime::currentTime();
//    timeElement.setAttribute("hour", time.hour());
//    timeElement.setAttribute("minute", time.minute());
//    timeElement.setAttribute("second", time.second());

//    //Add tab elements
//    for(int i=0; i<mpPlotTabWidget->count(); ++i)
//    {
//        QDomElement tabElement = appendDomElement(xmlRootElement,"plottab");
//        if(mpPlotTabWidget->getTab(i)->isGridVisible())
//        {
//            tabElement.setAttribute("grid", "true");
//        }
//        else
//        {
//            tabElement.setAttribute("grid", "false");
//        }
//        tabElement.setAttribute("color", makeRgbString(mpPlotTabWidget->getTab(i)->getQwtPlot()->canvasBackground().color()));

//        if(mpPlotTabWidget->getTab(i)->mHasCustomXData)
//        {
//            QDomElement specialXElement = appendDomElement(tabElement,"specialx");
//            //! @todo FIXA /Peter
//            //specialXElement.setAttribute("generation",  mpPlotTabs->getTab(i)->mVectorXGeneration);
////            specialXElement.setAttribute("component",   mpPlotTabWidget->getTab(i)->mSpecialXVectorDescription->mComponentName);
////            specialXElement.setAttribute("port",        mpPlotTabWidget->getTab(i)->mSpecialXVectorDescription->mPortName);
////            specialXElement.setAttribute("data",        mpPlotTabWidget->getTab(i)->mSpecialXVectorDescription->mDataName);
////            specialXElement.setAttribute("unit",        mpPlotTabWidget->getTab(i)->mSpecialXVectorDescription->mDataUnit);
////            specialXElement.setAttribute("model",       mpPlotTabWidget->getTab(i)->mSpecialXVectorDescription->mModelPath);
//        }

//        //Add curve elements
//        for(int j=0; j<mpPlotTabWidget->getTab(i)->getCurves().size(); ++j)
//        {
//            QDomElement curveElement = appendDomElement(tabElement,"curve");
//            //! @todo FIXA /Peter
//            //curveElement.setAttribute("generation", mpPlotTabs->getTab(i)->getCurves().at(j)->getGeneration());
//            curveElement.setAttribute("component",  mpPlotTabWidget->getTab(i)->getCurves().at(j)->getComponentName());
//            curveElement.setAttribute("port",       mpPlotTabWidget->getTab(i)->getCurves().at(j)->getPortName());
//            curveElement.setAttribute("data",       mpPlotTabWidget->getTab(i)->getCurves().at(j)->getDataName());
//            //curveElement.setAttribute("unit",       mpPlotTabWidget->getTab(i)->getCurves().at(j)->getDataUnit());
//            curveElement.setAttribute("axis",       mpPlotTabWidget->getTab(i)->getCurves().at(j)->getAxisY());
//            curveElement.setAttribute("width",      mpPlotTabWidget->getTab(i)->getCurves().at(j)->pen().width());
//            curveElement.setAttribute("color",      makeRgbString(mpPlotTabWidget->getTab(i)->getCurves().at(j)->pen().color()));
//            //curveElement.setAttribute("model",      mpPlotTabWidget->getTab(i)->getCurves().at(j)->getContainerObjectPtr()->getModelFileInfo().filePath());
//            //! @todo FIXA /Peter
//        }
//    }

//    appendRootXMLProcessingInstruction(domDocument);

//    //Save to file
//    QFile xmlsettings(filePath);
//    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        qDebug() << "Failed to open file for writing: " << filePath;
//        return;
//    }
//    QTextStream out(&xmlsettings);
//    domDocument.save(out, XMLINDENTATION);
}


//! @brief Loads a plot window from XML
void PlotWindow::loadFromXml()
{
    gpPlotWidget->loadFromXml();
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
    // Cloase each plottab that is empty (no curves)
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
            mpSaveButton->setDisabled(true);
            mpExportToCsvAction->setDisabled(true);
            mpExportToHvcAction->setDisabled(true);
            mpExportToGnuplotAction->setDisabled(true);
            mpExportToOldHopAction->setDisabled(true);
            mpExportToMatlabAction->setDisabled(true);
            mpLoadFromXmlButton->setDisabled(true);
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
            mpSaveButton->setDisabled(false);
            mpExportToCsvAction->setDisabled(false);
            mpExportToHvcAction->setDisabled(false);
            mpExportToGnuplotAction->setDisabled(false);
            mpExportToOldHopAction->setDisabled(false);
            mpExportToMatlabAction->setDisabled(false);
            mpLoadFromXmlButton->setDisabled(false);
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
        connect(mpExportToGraphicsAction,   SIGNAL(triggered()),    pCurrentTab,    SLOT(exportToGraphics()));
        connect(mpLegendButton,             SIGNAL(triggered()),    pCurrentTab,    SLOT(openLegendSettingsDialog()));
        connect(mpLocktheAxis,              SIGNAL(triggered()),    pCurrentTab,    SLOT(openAxisSettingsDialog()));
        connect(mpOpentimeScaleDialog,      SIGNAL(triggered()),    pCurrentTab,    SLOT(openTimeScalingDialog()));
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
    setPalette(gpMainWindow->palette());
}


//! @brief Creates a new plot window and adds the curves from current plot tab
void PlotWindow::createPlotWindowFromTab()
{
    //! @todo should be in tab instead and have signal slot
    PlotWindow *pPW = 0;
    for(int i=0; i<getCurrentPlotTab()->getCurves().size(); ++i)
    {
        HopsanVariable data(getCurrentPlotTab()->getCurves().at(i)->getVariableContainer(), getCurrentPlotTab()->getCurves().at(i)->getVariable());
        pPW = gpPlotHandler->plotDataToWindow(pPW, data, getCurrentPlotTab()->getCurves().at(i)->getAxisY());
    }
}


//! @todo should not run code on non bodeplot tabs
void PlotWindow::createBodePlot(SharedVectorVariableT var1, SharedVectorVariableT var2, int Fmax)
{
    SharedVectorVariableT pNyquist, pNyquistInv, pGain, pPhase;
    createBodeVariables(var1, var2, Fmax, pNyquist, pNyquistInv, pGain, pPhase);

    // Nyquist plot
    PlotTab *pNyquistTab = addPlotTab("Nyquist Plot");
    pNyquistTab->addCurve(new PlotCurve(pNyquist, QwtPlot::yLeft, NyquistType), "Blue");
    pNyquistTab->addCurve(new PlotCurve(pNyquistInv, QwtPlot::yLeft, NyquistType), "Blue");

    // Bode plot
    PlotTab *pBodeTab = addPlotTab("Bode Diagram", BodePlotType);
    PlotCurve *pGainCurve = new PlotCurve(pGain, QwtPlot::yLeft, BodeGainType);
    pBodeTab->getPlotArea(BodePlotTab::MagnitudePlot)->addCurve(pGainCurve);
    pBodeTab->getPlotArea(BodePlotTab::MagnitudePlot)->setBottomAxisLogarithmic(true);

    PlotCurve *pPhaseCurve = new PlotCurve(pPhase, QwtPlot::yLeft, BodePhaseType);
    pBodeTab->getPlotArea(BodePlotTab::PhasePlot)->addCurve(pPhaseCurve);
    pBodeTab->getPlotArea(BodePlotTab::PhasePlot)->setBottomAxisLogarithmic(true);

    pBodeTab->rescaleAxesToCurves();

    // Add a curve marker at the amplitude margin
    for(int i=1; i<pPhase->getDataSize(); ++i)
    {
        //! @todo a find crossing(s) function in variable should be nice
        if( (pPhase->peekData(i)<-180) && (pPhase->peekData(i-1)>-180) )
        {
            pBodeTab->getPlotArea(BodePlotTab::MagnitudePlot)->insertMarker(pGainCurve,
                                                                            pGain->getSharedTimeOrFrequencyVector()->peekData(i), pGain->peekData(i),
                                                                            QString("Gain Margin = %1 dB").arg(-pGain->peekData(i)),
                                                                            false);
            break;
        }
    }

    // Add a curve marker at the phase margin
    for(int i=1; i<pGain->getDataSize(); ++i)
    {
        //! @todo a find crossing(s) function in variable should be nice
        if( (pGain->peekData(i)<-0) && (pGain->peekData(i-1)>-0) )
        {
            pBodeTab->getPlotArea(BodePlotTab::PhasePlot)->insertMarker(pPhaseCurve,
                                                                        pPhase->getSharedTimeOrFrequencyVector()->peekData(i), pPhase->peekData(i),
                                                                        QString("Phase Margin = %1").arg(180.0+pPhase->peekData(i))+trUtf8("°"),
                                                                        false);
            break;
        }
    }

    //! @todo this should not happen here
    SharedVectorVariableT gainVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->defineNewVariable("bodegain");
    if(gainVar.isNull())
    {
        gainVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getVectorVariable("bodegain",-1);
    }
    gainVar.data()->assignFrom(pGain);

    SharedVectorVariableT phaseVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->defineNewVariable("bodephase");
    if(phaseVar.isNull())
    {
        phaseVar = gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getVectorVariable("bodegain",-1);
    }
    phaseVar.data()->assignFrom(pPhase);
}

void PlotWindow::showHelpPopupMessage(const QString &rMessage)
{
    mpHelpPopup->showHelpPopupMessage(rMessage);
}
