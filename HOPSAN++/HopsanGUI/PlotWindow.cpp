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

#include <cstring>
#include <limits>
#include <complex>

#include "Widgets/PlotWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/ProjectTabWidget.h"
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

#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"

#include "Dependencies/BarChartPlotter/barchartplotter.h"
#include "Dependencies/BarChartPlotter/axisbase.h"

const double DBLMAX = std::numeric_limits<double>::max();

//! @brief Constructor for the plot window, where plots are displayed.
//! @param plotVariableTree is a pointer to the variable tree from where the plot window was created
//! @param parent is a pointer to the main window
PlotWindow::PlotWindow(PlotVariableTree *plotVariableTree, MainWindow *parent)
    : QMainWindow(parent)

{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_MouseTracking, true);

    setWindowTitle("Hopsan Plot Window");
    //setAcceptDrops(false);
    //setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setPalette(gConfig.getPalette());

    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    resize(sw*0.7, sh*0.7);   //Resize plot window to 70% of screen height and width
    int w = this->size().width();
    int h = this->size().height();
    int x = (sw - w)/2;
    int y = (sh - h)/2;
    move(x, y);       //Move plot window to center of screen

    mpPlotVariableTree = plotVariableTree;

        //Create the toolbar and its buttons
    mpToolBar = new QToolBar(this);

    mpNewPlotButton = new QAction(this);
    mpNewPlotButton->setToolTip("Create New Plot");
    mpNewPlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
    connect(mpNewPlotButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpZoomButton = new QAction(this);
    mpZoomButton->setToolTip("Zoom (Z)");
    mpZoomButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Zoom.png"));
    mpZoomButton->setCheckable(true);
    mpZoomButton->setShortcut(QKeySequence("z"));
    connect(mpZoomButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

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

    mpExportToXmlAction = new QAction("Export to Extensible Markup Language File (.xml)", mpToolBar);
    mpExportToCsvAction = new QAction("Export to Comma-Separeted Values File (.csv)", mpToolBar);
    mpExportToMatlabAction = new QAction("Export to Matlab Script File (.m)", mpToolBar);
    mpExportToGnuplotAction = new QAction("Export to gnuplot data file(.dat)", mpToolBar);

    mpExportMenu = new QMenu(mpToolBar);
    mpExportMenu->addAction(mpExportToXmlAction);
    mpExportMenu->addAction(mpExportToCsvAction);
    mpExportMenu->addAction(mpExportToMatlabAction);
    mpExportMenu->addAction(mpExportToGnuplotAction);

    mpExportButton = new QToolButton(mpToolBar);
    mpExportButton->setToolTip("Export Plot Tab");
    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ExportPlot.png"));
    mpExportButton->setMenu(mpExportMenu);
    mpExportButton->setPopupMode(QToolButton::InstantPopup);
    mpExportButton->setMouseTracking(true);

    mpExportPdfAction = new QAction("Export to PDF", mpToolBar);
    mpExportPngAction = new QAction("Export to PNG", mpToolBar);

    mpExportGfxMenu = new QMenu(mpToolBar);
    mpExportGfxMenu->addAction(mpExportPdfAction);
    mpExportGfxMenu->addAction(mpExportPngAction);

    mpExportGfxButton = new QToolButton(mpToolBar);
    mpExportGfxButton->setToolTip("Export to Graphics File");
    mpExportGfxButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ExportGfx.png"));
    mpExportGfxButton->setMenu(mpExportGfxMenu);
    mpExportGfxButton->setPopupMode(QToolButton::InstantPopup);
    mpExportGfxButton->setMouseTracking(true);

    mpLoadFromXmlButton = new QAction(this);
    mpLoadFromXmlButton->setToolTip("Import Plot");
    mpLoadFromXmlButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Open.png"));
    connect(mpLoadFromXmlButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

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

    mpShowCurveInfoButton = new QAction(this);
    mpShowCurveInfoButton->setCheckable(true);
    mpShowCurveInfoButton->setChecked(true);
    mpShowCurveInfoButton->setToolTip("Toggle Variable Lists");
    mpShowCurveInfoButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowLists.png"));
    connect(mpShowCurveInfoButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpShowLegendsAction = new QAction(this);
    mpShowLegendsAction->setCheckable(true);
    mpShowLegendsAction->setChecked(true);
    mpShowLegendsAction->setToolTip("Toggle Legends");
    mpShowLegendsAction->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotLegends.png"));
    connect(mpShowLegendsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpShowPlotWidgetButton = new QAction(this);
    mpShowPlotWidgetButton->setCheckable(true);
    mpShowPlotWidgetButton->setChecked(true);
    mpShowPlotWidgetButton->setToolTip("Toggle Curve Controls");
    mpShowPlotWidgetButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowCurves.png"));
    connect(mpShowPlotWidgetButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpNewWindowFromTabButton = new QAction(this);
    mpNewWindowFromTabButton->setToolTip("Create Plot Window From Tab");
    mpNewWindowFromTabButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-OpenTabInNewPlotWindow.png"));
    connect(mpNewWindowFromTabButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpResetXVectorButton = new QAction(this);
    mpResetXVectorButton->setToolTip("Reset Time Vector");
    mpResetXVectorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    mpResetXVectorButton->setEnabled(false);
    connect(mpResetXVectorButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpBodePlotButton = new QAction(this);
    mpBodePlotButton->setToolTip("Transfer Function Analysis");
    mpBodePlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-TransferFunctionAnalysis.png"));
    connect(mpBodePlotButton, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpToolBar->addAction(mpNewPlotButton);
    mpToolBar->addAction(mpLoadFromXmlButton);
    mpToolBar->addAction(mpSaveButton);
    mpToolBar->addWidget(mpExportButton);
    mpToolBar->addWidget(mpExportGfxButton);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpZoomButton);
    mpToolBar->addAction(mpPanButton);
    mpToolBar->addAction(mpGridButton);
    mpToolBar->addAction(mpBackgroundColorButton);
    mpToolBar->addAction(mpResetXVectorButton);
    mpToolBar->addAction(mpBodePlotButton);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpShowCurveInfoButton);
    mpToolBar->addAction(mpShowPlotWidgetButton);
    mpToolBar->addAction(mpShowLegendsAction);
    mpToolBar->addAction(mpNewWindowFromTabButton);
    mpToolBar->setMouseTracking(true);

    addToolBar(mpToolBar);

    mpPlotTabs = new PlotTabWidget(this);
    this->addPlotTab();

    //Initialize the help message popup
    mpHelpPopup = new QWidget(this);
    mpHelpPopup->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mpHelpPopupIcon = new QLabel();
    mpHelpPopupIcon->setMouseTracking(true);
    mpHelpPopupIcon->setPixmap(QPixmap(QString(ICONPATH) + "Hopsan-Info.png"));
    mpHelpPopupLabel = new QLabel();
    mpHelpPopupLabel->setMouseTracking(true);
    mpHelpPopupGroupBoxLayout = new QHBoxLayout(mpHelpPopup);
    mpHelpPopupGroupBoxLayout->addWidget(mpHelpPopupIcon);
    mpHelpPopupGroupBoxLayout->addWidget(mpHelpPopupLabel);
    mpHelpPopupGroupBoxLayout->setContentsMargins(3,3,3,3);
    mpHelpPopupGroupBox = new QGroupBox(mpHelpPopup);
    mpHelpPopupGroupBox->setLayout(mpHelpPopupGroupBoxLayout);
    mpHelpPopupGroupBox->setMouseTracking(true);
    mpHelpPopupLayout = new QHBoxLayout(mpHelpPopup);
    mpHelpPopupLayout->addWidget(mpHelpPopupGroupBox);
    mpHelpPopup->setLayout(mpHelpPopupLayout);
    mpHelpPopup->setMouseTracking(true);
    mpHelpPopup->setBaseSize(50,30);
    mpHelpPopup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    mpHelpPopup->setStyleSheet("QGroupBox { background-color : rgba(255,255,224,255); } QLabel { margin : 0px; } ");
    mpHelpPopup->hide();
    mpHelpPopupTimer = new QTimer(this);
    connect(mpHelpPopupTimer, SIGNAL(timeout()), mpHelpPopup, SLOT(hide()));

    mpPlotInfoLayout = new QVBoxLayout();
    mpPlotInfoWidget = new QWidget();
    mpPlotInfoWidget->setLayout(mpPlotInfoLayout);
    mpPlotInfoWidget->hide();

    this->setDockOptions(QMainWindow::AllowNestedDocks);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotTabs,0,0,2,4);
    mpLayout->addWidget(mpHelpPopup, 0,0,1,4);
    mpLayout->addWidget(mpPlotInfoWidget, 2, 0, 1, 4);

    //Set the correct position of the help popup message in the central widget
    mpLayout->setColumnMinimumWidth(0,5);
    mpLayout->setColumnStretch(0,0);
    mpLayout->setColumnStretch(1,1);
    mpLayout->setRowMinimumHeight(0,25);
    mpLayout->setRowStretch(0,0);
    mpLayout->setRowStretch(1,1);
    mpLayout->setRowStretch(2,0);

    QWidget *pCentralWidget = new QWidget(this);
    pCentralWidget->setLayout(mpLayout);
    this->setCentralWidget(pCentralWidget);

    PlotTreeWidget *pLocalPlotWidget = new PlotTreeWidget(gpMainWindow);
    QDockWidget *pPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    pPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, pPlotWidgetDock);
    pPlotWidgetDock->setWidget(pLocalPlotWidget);
    pLocalPlotWidget->mpPlotVariableTree->updateList();

        //Establish signal and slots connections
    connect(mpNewPlotButton,                    SIGNAL(triggered()),        this,               SLOT(addPlotTab()));
    connect(mpLoadFromXmlButton,                SIGNAL(triggered()),        this,               SLOT(loadFromXml()));
    connect(mpSaveButton,                       SIGNAL(triggered()),        this,               SLOT(saveToXml()));
    connect(mpBodePlotButton,                   SIGNAL(triggered()),        this,               SLOT(createBodePlot()));
    connect(mpNewWindowFromTabButton,           SIGNAL(triggered()),        this,               SLOT(createPlotWindowFromTab()));
    connect(gpMainWindow->getOptionsDialog(),   SIGNAL(paletteChanged()),   this,               SLOT(updatePalette()));
    connect(mpShowPlotWidgetButton,             SIGNAL(toggled(bool)),      pPlotWidgetDock,    SLOT(setVisible(bool)));
    connect(mpShowLegendsAction,                SIGNAL(toggled(bool)),      this,               SLOT(setLegendsVisible(bool)));

        //Hide lists and curve areas by default if screen size is small
    if(sh*sw <= 800*1280)
    {
        mpShowCurveInfoButton->toggle();
        //mpShowCurvesButton->toggle();
    }

    this->setMouseTracking(true);
}


void PlotWindow::changeXVector(QVector<double> xarray, QString componentName, QString portName, QString dataName, QString dataUnit)
{
    getCurrentPlotTab()->changeXVector(xarray, componentName, portName, dataName, dataUnit, FIRSTPLOT);
}


void PlotWindow::addPlotTab(QString requestedName)
{
    PlotTab *mpNewTab = new PlotTab(this);

    QString tabName;
    QString numString;
    if(requestedName.isEmpty())
    {
        for(int tabID=0; true; ++tabID)
        {
            numString.setNum(tabID);
            bool found = false;
            for(int i=0; i<mpPlotTabs->count(); ++i)
            {
                if(mpPlotTabs->tabText(i) == "Plot " + numString)
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
        tabName = requestedName;
    }

    mpPlotTabs->addTab(mpNewTab, tabName);

    mpPlotTabs->setCurrentIndex(mpPlotTabs->count()-1);
}


PlotTabWidget *PlotWindow::getPlotTabWidget()
{
    return mpPlotTabs;
}


PlotTab *PlotWindow::getCurrentPlotTab()
{
    return qobject_cast<PlotTab *>(mpPlotTabs->currentWidget());
}




//! @brief Shows the help popup message for 5 seconds with specified message.
//! Any message already being shown will be replaced. Messages can be hidden in advance by calling mpHelpPopup->hide().
//! @param message String with text so show in message
void PlotWindow::showHelpPopupMessage(QString message)
{
    if(gConfig.getShowPopupHelp())
    {
        mpHelpPopupLabel->setText(message);
        mpHelpPopup->show();
        mpHelpPopupTimer->stop();
        mpHelpPopupTimer->start(5000);
    }
}


//! @brief Hides the help popup message
void PlotWindow::hideHelpPopupMessage()
{
    mpHelpPopup->hide();
}


//! @brief Creates a new plot curve from a plot variable in current container object and adds it to the current plot tab
//! @param generation Generation of plot data
//! @param componentName Name of component where variable is located
//! @param portName Name of port where variable is located
//! @param dataName Name of variable
//! @param dataUnit Unit of variable
void PlotWindow::addPlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY, QString modelPath, QColor desiredColor)
{
    if(dataUnit.isEmpty()) { dataUnit = gConfig.getDefaultUnit(dataName); }
    PlotCurve *pTempCurve = new PlotCurve(generation, componentName, portName, dataName, dataUnit, axisY, modelPath, getCurrentPlotTab());
    getCurrentPlotTab()->addCurve(pTempCurve, desiredColor);
    pTempCurve->updatePlotInfoVisibility();
}


void PlotWindow::addBarChart(QStandardItemModel *pItemModel)
{
    getCurrentPlotTab()->addBarChart(pItemModel);
}


//! @brief Saves the plot window to XML
//! All generations of all open curves will be saved, together with all cosmetic information about the plot window.
void PlotWindow::saveToXml()
{
        //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    filePath = QFileDialog::getSaveFileName(this, tr("Save Plot Window Description to XML"),
                                            fileDialogSaveDir.currentPath(),
                                            tr("Plot Window Description File (*.xml)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel


        //Write to xml file
    QDomDocument domDocument;
    QDomElement xmlRootElement = domDocument.createElement("hopsanplot");
    domDocument.appendChild(xmlRootElement);

    QDomElement dateElement = appendDomElement(xmlRootElement,"date");      //Append date tag
    QDate date = QDate::currentDate();
    dateElement.setAttribute("year", date.year());
    dateElement.setAttribute("month", date.month());
    dateElement.setAttribute("day", date.day());

    QDomElement timeElement = appendDomElement(xmlRootElement,"time");      //Append time tag
    QTime time = QTime::currentTime();
    timeElement.setAttribute("hour", time.hour());
    timeElement.setAttribute("minute", time.minute());
    timeElement.setAttribute("second", time.second());

        //Add tab elements
    for(int i=0; i<mpPlotTabs->count(); ++i)
    {
        QDomElement tabElement = appendDomElement(xmlRootElement,"plottab");
        if(mpPlotTabs->getTab(i)->isGridVisible())
        {
            tabElement.setAttribute("grid", "true");
        }
        else
        {
            tabElement.setAttribute("grid", "false");
        }
        tabElement.setAttribute("color", makeRgbString(mpPlotTabs->getTab(i)->getPlot()->canvasBackground().color()));

        if(mpPlotTabs->getTab(i)->mHasSpecialXAxis)
        {
            QDomElement specialXElement = appendDomElement(tabElement,"specialx");
            specialXElement.setAttribute("generation",  mpPlotTabs->getTab(i)->mVectorXGeneration);
            specialXElement.setAttribute("component",   mpPlotTabs->getTab(i)->mVectorXComponent);
            specialXElement.setAttribute("port",        mpPlotTabs->getTab(i)->mVectorXPortName);
            specialXElement.setAttribute("data",        mpPlotTabs->getTab(i)->mVectorXDataName);
            specialXElement.setAttribute("unit",        mpPlotTabs->getTab(i)->mVectorXDataUnit);
            specialXElement.setAttribute("model",       mpPlotTabs->getTab(i)->mVectorXModelPath);
        }

            //Add curve elements
        for(int j=0; j<mpPlotTabs->getTab(i)->getCurves().size(); ++j)
        {
            QDomElement curveElement = appendDomElement(tabElement,"curve");
            curveElement.setAttribute("generation", mpPlotTabs->getTab(i)->getCurves().at(j)->getGeneration());
            curveElement.setAttribute("component",  mpPlotTabs->getTab(i)->getCurves().at(j)->getComponentName());
            curveElement.setAttribute("port",       mpPlotTabs->getTab(i)->getCurves().at(j)->getPortName());
            curveElement.setAttribute("data",       mpPlotTabs->getTab(i)->getCurves().at(j)->getDataName());
            curveElement.setAttribute("unit",       mpPlotTabs->getTab(i)->getCurves().at(j)->getDataUnit());
            curveElement.setAttribute("axis",       mpPlotTabs->getTab(i)->getCurves().at(j)->getAxisY());
            curveElement.setAttribute("width",      mpPlotTabs->getTab(i)->getCurves().at(j)->getCurvePtr()->pen().width());
            curveElement.setAttribute("color",      makeRgbString(mpPlotTabs->getTab(i)->getCurves().at(j)->getCurvePtr()->pen().color()));
            curveElement.setAttribute("model",      mpPlotTabs->getTab(i)->getCurves().at(j)->getContainerObjectPtr()->getModelFileInfo().filePath());
        }
    }

    appendRootXMLProcessingInstruction(domDocument);

    //Save to file
    const int IndentSize = 4;
    QFile xmlsettings(filePath);
    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << filePath;
        return;
    }
    QTextStream out(&xmlsettings);
    domDocument.save(out, IndentSize);
}


//! @brief Loads a plot window from XML
void PlotWindow::loadFromXml()
{
    gpMainWindow->mpPlotWidget->loadFromXml();
}


void PlotWindow::performFrequencyAnalysis(PlotCurve *curve)
{
    mpFrequencyAnalysisCurve = curve;

    QLabel *pInfoLabel = new QLabel(tr("This will generate a frequency spectrum. Using more log samples will increase accuracy of the results."));
    pInfoLabel->setWordWrap(true);
    pInfoLabel->setFixedWidth(300);

    mpFrequencyAnalysisDialog = new QDialog(this);
    mpFrequencyAnalysisDialog->setWindowTitle("Generate Frequency Spectrum");

    mpLogScaleCheckBox = new QCheckBox("Use log scale");
    mpLogScaleCheckBox->setChecked(true);

    mpPowerSpectrumCheckBox = new QCheckBox("Power spectrum");
    mpPowerSpectrumCheckBox->setChecked(false);

    QPushButton *pCancelButton = new QPushButton("Cancel");
    QPushButton *pNextButton = new QPushButton("Go!");

    QGridLayout *pFrequencyAnalysisDialogLayout = new QGridLayout(mpFrequencyAnalysisDialog);
    pFrequencyAnalysisDialogLayout->addWidget(pInfoLabel,               0, 0, 1, 2);
    pFrequencyAnalysisDialogLayout->addWidget(mpLogScaleCheckBox,       1, 0, 1, 2);
    pFrequencyAnalysisDialogLayout->addWidget(mpPowerSpectrumCheckBox,  2, 0, 1, 2);
    pFrequencyAnalysisDialogLayout->addWidget(pCancelButton,            3, 0, 1, 1);
    pFrequencyAnalysisDialogLayout->addWidget(pNextButton,              3, 1, 1, 1);

    mpFrequencyAnalysisDialog->setLayout(pFrequencyAnalysisDialogLayout);

    mpFrequencyAnalysisDialog->show();

    connect(pCancelButton, SIGNAL(clicked()), mpFrequencyAnalysisDialog, SLOT(close()));
    connect(pNextButton, SIGNAL(clicked()), this, SLOT(performFrequencyAnalysisFromDialog()));
}


void PlotWindow::performFrequencyAnalysisFromDialog()
{
    mpFrequencyAnalysisDialog->close();

    addPlotTab();
    getCurrentPlotTab()->getPlot()->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
    getCurrentPlotTab()->updateLabels();
    PlotCurve *pNewCurve = new PlotCurve(mpFrequencyAnalysisCurve->getGeneration(), mpFrequencyAnalysisCurve->getComponentName(), mpFrequencyAnalysisCurve->getPortName(), mpFrequencyAnalysisCurve->getDataName(), mpFrequencyAnalysisCurve->getDataUnit(), mpFrequencyAnalysisCurve->getAxisY(), mpFrequencyAnalysisCurve->getContainerObjectPtr()->getModelFileInfo().filePath(), getCurrentPlotTab(), FIRSTPLOT, FREQUENCYANALYSIS);
    getCurrentPlotTab()->addCurve(pNewCurve);
    pNewCurve->toFrequencySpectrum();
    pNewCurve->updatePlotInfoVisibility();
    //! @todo Make logged axis an option for user
    if(mpLogScaleCheckBox->isChecked())
    {
        getCurrentPlotTab()->getPlot(FIRSTPLOT)->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
        getCurrentPlotTab()->getPlot(FIRSTPLOT)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
    }
    getCurrentPlotTab()->rescaleToCurves();
}


void PlotWindow::createBodePlot()
{
    mpCreateBodeDialog = new QDialog(this);
    mpCreateBodeDialog->setWindowTitle("Transfer Function Analysis");

    QGroupBox *pInputGroupBox = new QGroupBox(tr("Input Variable"));
    QVBoxLayout *pInputGroupBoxLayout = new QVBoxLayout;
    pInputGroupBoxLayout->addStretch(1);
    for(int i=0; i<getCurrentPlotTab()->getNumberOfCurves(FIRSTPLOT); ++i)
    {
        QRadioButton *radio = new QRadioButton(getCurrentPlotTab()->getCurves().at(i)->getComponentName() + ", " +
                                               getCurrentPlotTab()->getCurves().at(i)->getPortName() + ", " +
                                               getCurrentPlotTab()->getCurves().at(i)->getDataName());
        mBodeInputButtonToCurveMap.insert(radio, getCurrentPlotTab()->getCurves().at(i));
        pInputGroupBoxLayout->addWidget(radio);
    }
    pInputGroupBox->setLayout(pInputGroupBoxLayout);

    QGroupBox *pOutputGroupBox = new QGroupBox(tr("Output Variable"));
    QVBoxLayout *pOutputGroupBoxLayout = new QVBoxLayout;
    pOutputGroupBoxLayout->addStretch(1);
    for(int i=0; i<getCurrentPlotTab()->getNumberOfCurves(FIRSTPLOT); ++i)
    {
        QRadioButton *radio = new QRadioButton(getCurrentPlotTab()->getCurves().at(i)->getComponentName() + ", " +
                                               getCurrentPlotTab()->getCurves().at(i)->getPortName() + ", " +
                                               getCurrentPlotTab()->getCurves().at(i)->getDataName());
        mBodeOutputButtonToCurveMap.insert(radio, getCurrentPlotTab()->getCurves().at(i));
        pOutputGroupBoxLayout->addWidget(radio);
    }
    pOutputGroupBox->setLayout(pOutputGroupBoxLayout);

    double maxFreq = (getCurrentPlotTab()->getCurves(FIRSTPLOT).first()->getTimeVector().size()+1)/getCurrentPlotTab()->getCurves(FIRSTPLOT).first()->getTimeVector().last()/2;
    QLabel *pMaxFrequencyLabel = new QLabel("Maximum frequency in bode plot:");
    QLabel *pMaxFrequencyValue = new QLabel();
    QLabel *pMaxFrequencyUnit = new QLabel("Hz");
    pMaxFrequencyValue->setNum(maxFreq);
    mpMaxFrequencySlider = new QSlider(this);
    mpMaxFrequencySlider->setOrientation(Qt::Horizontal);
    mpMaxFrequencySlider->setMinimum(0);
    mpMaxFrequencySlider->setMaximum(maxFreq);
    mpMaxFrequencySlider->setValue(maxFreq);
    connect(mpMaxFrequencySlider, SIGNAL(valueChanged(int)), pMaxFrequencyValue, SLOT(setNum(int)));

    QHBoxLayout *pSliderLayout = new QHBoxLayout();
    pSliderLayout->addWidget(mpMaxFrequencySlider);
    pSliderLayout->addWidget(pMaxFrequencyValue);
    pSliderLayout->addWidget(pMaxFrequencyUnit);

    QPushButton *pCancelButton = new QPushButton("Cancel");
    QPushButton *pNextButton = new QPushButton("Go!");

    QGridLayout *pBodeDialogLayout = new QGridLayout;
    pBodeDialogLayout->addWidget(pInputGroupBox, 0, 0, 1, 2);
    pBodeDialogLayout->addWidget(pOutputGroupBox, 1, 0, 1, 2);
    pBodeDialogLayout->addWidget(pMaxFrequencyLabel, 2, 0, 1, 2);
    pBodeDialogLayout->addLayout(pSliderLayout, 3, 0, 1, 2);
    pBodeDialogLayout->addWidget(pCancelButton, 4, 0, 1, 1);
    pBodeDialogLayout->addWidget(pNextButton, 4, 1, 1, 1);

    mpCreateBodeDialog->setLayout(pBodeDialogLayout);

    mpCreateBodeDialog->show();

    connect(pCancelButton, SIGNAL(clicked()), mpCreateBodeDialog, SLOT(close()));
    connect(pNextButton, SIGNAL(clicked()), this, SLOT(createBodePlotFromDialog()));
    //connect(pNextButton, SIGNAL(clicked()), pCreateBodeDialog, SLOT(close()));
}


void PlotWindow::createBodePlotFromDialog()
{
    PlotCurve *inputCurve = 0;
    PlotCurve *outputCurve = 0;
    QMap<QRadioButton *, PlotCurve *>::iterator it;
    for(it=mBodeInputButtonToCurveMap.begin(); it!=mBodeInputButtonToCurveMap.end(); ++it)
    {
        if(it.key()->isChecked())
            inputCurve = it.value();
    }
    for(it=mBodeOutputButtonToCurveMap.begin(); it!=mBodeOutputButtonToCurveMap.end(); ++it)
    {
        if(it.key()->isChecked())
            outputCurve = it.value();
    }
    if(inputCurve == 0 || outputCurve == 0)
    {
        QMessageBox::warning(this, tr("Transfer Function Analysis Failed"), tr("Both input and output vectors must be selected."));
    }
    else if(inputCurve == outputCurve)
    {
        QMessageBox::warning(this, tr("Transfer Function Analysis Failed"), tr("Input and output vectors must be different."));
    }
    else
    {
        mpCreateBodeDialog->close();
        createBodePlot(inputCurve, outputCurve, mpMaxFrequencySlider->value());
    }
}


void PlotWindow::createBodePlot(PlotCurve *pInputCurve, PlotCurve *pOutputCurve, int Fmax)
{
    //Create temporary real vectors
    QVector<double> realYvector = pInputCurve->getDataVector();
    QVector<double> realXvector = pOutputCurve->getDataVector();

    //Abort and inform user if vectors are not of same size
    if(realXvector.size() != realYvector.size())
    {
        QMessageBox::warning(gpMainWindow, gpMainWindow->tr("Wrong Vector Size"),
                             "Input and output vector must be of same size.");
        return;
    }

    //Reduce vector size if they are not equal to an even potential of 2, and inform user
    int n = pow(2, int(log2(realXvector.size())));
    if(n != realXvector.size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(realXvector.size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindow, gpMainWindow->tr("Wrong Vector Size"),
                                 "Size of data vector must be an even power of 2. Number of log samples was reduced from " + oldString + " to " + newString + ".");
        reduceVectorSize(realXvector, n);
        reduceVectorSize(realYvector, n);
    }

    //Create complex vectors
    QVector< std::complex<double> > Y = realToComplex(realYvector);
    QVector< std::complex<double> > X = realToComplex(realXvector);

    //Apply the fourier transforms
    FFT(X);
    FFT(Y);

    //Divide the fourier transform elementwise and take their absolute value
    QVector< std::complex<double> > G;
    QVector<double> vRe;
    QVector<double> vIm;
    QVector<double> vImNeg;
    QVector<double> vBodeGain;
    QVector<double> vBodePhase;

    double phaseCorrection=0;
    QVector<double> vBodePhaseUncorrected;
    for(int i=0; i<Y.size()/2; ++i)
    {
        if(Y.at(i) == std::complex<double>(0,0))        //Check for division by zero
        {
            G.append(G[i-1]);    //! @todo This is not a good solution, and what if i=0?
        }
        else
        {
            G.append(X.at(i)/Y.at(i));                  //G(iw) = FFT(Y(iw))/FFT(X(iw))
        }
        if(i!=0)
        {
            vRe.append(G[i].real());
            vIm.append(G[i].imag());
            vImNeg.append(-G[i].imag());
            vBodeGain.append(20*log10(sqrt(G[i].real()*G[i].real() + G[i].imag()*G[i].imag())));  //Gain: abs(G) = sqrt(R^2 + X^2)
            vBodePhaseUncorrected.append(atan2(G[i].imag(), G[i].real())*180/3.14159265);          //Phase: arg(G) = arctan(X/R)

            // Correct the phase plot to make it continous (because atan2 is limited from -180 to +180)
            if(vBodePhaseUncorrected.size() > 1)
            {
                if(vBodePhaseUncorrected.last() > 170 && vBodePhaseUncorrected[vBodePhaseUncorrected.size()-2] < -170)
                    phaseCorrection -= 360;
                else if(vBodePhaseUncorrected.last() < -170 && vBodePhaseUncorrected[vBodePhaseUncorrected.size()-2] > 170)
                    phaseCorrection += 360;
            }
            vBodePhase.append(vBodePhaseUncorrected.last() + phaseCorrection);
        }
    }


    QVector<double> F;
    double stoptime = pInputCurve->getTimeVector().last();
    for(int i=1; i<G.size(); ++i)
    {
        F.append((i+1)/stoptime);
        if(F.last() >= Fmax) break;
    }
    vBodeGain.resize(F.size());
    vBodePhase.resize(F.size());


    addPlotTab("Nyquist Plot");
    PlotCurve *pNyquistCurve1 = new PlotCurve(pOutputCurve->getGeneration(), pOutputCurve->getComponentName(), pOutputCurve->getPortName(), pOutputCurve->getDataName(),
                                        pOutputCurve->getDataUnit(), pOutputCurve->getAxisY(), pOutputCurve->getContainerObjectPtr()->getModelFileInfo().filePath(),
                                        getCurrentPlotTab(), FIRSTPLOT, NYQUIST);
    getCurrentPlotTab()->addCurve(pNyquistCurve1);
    pNyquistCurve1->setData(vIm, vRe);
    pNyquistCurve1->updatePlotInfoVisibility();
    PlotCurve *pNyquistCurve2 = new PlotCurve(pOutputCurve->getGeneration(), pOutputCurve->getComponentName(), pOutputCurve->getPortName(), pOutputCurve->getDataName(),
                                        pOutputCurve->getDataUnit(), pOutputCurve->getAxisY(), pOutputCurve->getContainerObjectPtr()->getModelFileInfo().filePath(),
                                        getCurrentPlotTab(), FIRSTPLOT, NYQUIST);
    getCurrentPlotTab()->addCurve(pNyquistCurve2);
    pNyquistCurve2->setData(vImNeg, vRe);
    pNyquistCurve2->updatePlotInfoVisibility();
    getCurrentPlotTab()->getPlot()->replot();
    getCurrentPlotTab()->rescaleToCurves();

    addPlotTab("Bode Diagram");
    PlotCurve *pGainCurve = new PlotCurve(pOutputCurve->getGeneration(), pOutputCurve->getComponentName(), pOutputCurve->getPortName(), pOutputCurve->getDataName(),
                                          pOutputCurve->getDataUnit(), pOutputCurve->getAxisY(), pOutputCurve->getContainerObjectPtr()->getModelFileInfo().filePath(),
                                          getCurrentPlotTab(), FIRSTPLOT, BODEGAIN);
    getCurrentPlotTab()->addCurve(pGainCurve);
    pGainCurve->setData(vBodeGain, F);
    pGainCurve->updatePlotInfoVisibility();

    PlotCurve *pPhaseCurve = new PlotCurve(pOutputCurve->getGeneration(), pOutputCurve->getComponentName(), pOutputCurve->getPortName(), pOutputCurve->getDataName(),
                                          pOutputCurve->getDataUnit(), pOutputCurve->getAxisY(), pOutputCurve->getContainerObjectPtr()->getModelFileInfo().filePath(),
                                          getCurrentPlotTab(), SECONDPLOT, BODEPHASE);
    getCurrentPlotTab()->addCurve(pPhaseCurve, QColor(), SECONDPLOT);
    pPhaseCurve->setData(vBodePhase, F);
    pPhaseCurve->updatePlotInfoVisibility();

    getCurrentPlotTab()->showPlot(SECONDPLOT, true);
    getCurrentPlotTab()->getPlot(FIRSTPLOT)->replot();
    getCurrentPlotTab()->getPlot(SECONDPLOT)->replot();

    //getCurrentPlotTab()->getPlot(FIRSTPLOT)->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
    getCurrentPlotTab()->setBottomAxisLogarithmic(true);

    getCurrentPlotTab()->rescaleToCurves();


    //Add a curve marker at the amplitude margin
    for(int i=1; i<vBodePhase.size(); ++i)
    {
        if(vBodePhase.at(i) < -180 && vBodePhase.at(i-1) > -180)
        {
            QString valueString;
            valueString.setNum(-vBodeGain.at(i));
            getCurrentPlotTab()->insertMarker(pGainCurve, F.at(i), vBodeGain.at(i), "Gain Margin = " + valueString + " dB", false);
            break;
        }
    }

    //Add a curve marker at the phase margin
    for(int i=1; i<vBodeGain.size(); ++i)
    {
        if(vBodeGain.at(i) < -0 && vBodeGain.at(i-1) > -0)
        {
            QString valueString;
            valueString.setNum(180.0+vBodePhase.at(i));
            getCurrentPlotTab()->insertMarker(pPhaseCurve, F.at(i), vBodePhase.at(i), "Phase Margin = " + valueString + trUtf8("°"), false);
            break;
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
        showHelpPopupMessage("Create a new empty plot tab.");
    }
    else if(pHoveredAction == mpBodePlotButton)
    {
        showHelpPopupMessage("Performs transfer function analysis to generate nyquist plot and bode diagram.");
    }
    else if(pHoveredAction == mpZoomButton)
    {
        showHelpPopupMessage("Enable zooming tool.");
    }
    else if(pHoveredAction == mpPanButton)
    {
        showHelpPopupMessage("Enable panning tool.");
    }
    else if(pHoveredAction == mpSaveButton)
    {
        showHelpPopupMessage("Save plot window to XML.");
    }
    else if(pHoveredAction == mpLoadFromXmlButton)
    {
        showHelpPopupMessage("Load plot window from XML.");
    }
    else if(pHoveredAction == mpGridButton)
    {
        showHelpPopupMessage("Toggle background grid.");
    }
    else if(pHoveredAction == mpBackgroundColorButton)
    {
        showHelpPopupMessage("Change background color.");
    }
    else if(pHoveredAction == mpNewWindowFromTabButton)
    {
        showHelpPopupMessage("Create new plot window from current tab.");
    }
    else if(pHoveredAction == mpResetXVectorButton)
    {
        showHelpPopupMessage("Reset X-vector to simulation time.");
    }
    else if(pHoveredAction == mpShowCurveInfoButton)
    {
        showHelpPopupMessage("Show/hide variable lists.");
    }
    else if(pHoveredAction == mpShowPlotWidgetButton)
    {
        showHelpPopupMessage("Show/hide plot curve control panel.");
    }
}


//! @brief Slot that closes all tabs with no curves, and then closes the entire plot window if it has no curves.
void PlotWindow::closeIfEmpty()
{
    int curves=0;
    for(int i=0; i<mpPlotTabs->count(); ++i)
    {
        int nCurvesInTab = 0;
        for(int plotId=0; plotId<2; ++plotId)
        {
            nCurvesInTab += mpPlotTabs->getTab(i)->getNumberOfCurves(HopsanPlotID(plotId));
        }

        if(nCurvesInTab == 0)
        {
            mpPlotTabs->closePlotTab(i);
            --i;
        }

        curves += nCurvesInTab;
    }

    if(curves == 0)
        close();
}


void PlotWindow::hideCurveInfo()
{
    mpShowCurveInfoButton->setChecked(false);
}


void PlotWindow::setLegendsVisible(bool value)
{
    mLegendsVisible = value;
    for(int i=0; i<mpPlotTabs->count(); ++i)
    {
        mpPlotTabs->getTab(i)->setLegendsVisible(value);
    }
}


void PlotWindow::mouseMoveEvent(QMouseEvent *event)
{
    hideHelpPopupMessage();

    QMainWindow::mouseMoveEvent(event);
}


//! @brief Reimplementation of close function for plot window. Notifies plot widget that window no longer exists.
void PlotWindow::close()
{
    gpMainWindow->mpPlotWidget->mpPlotVariableTree->reportClosedPlotWindow(this);
    QMainWindow::close();
}


//! @brief Slot that updates the palette to match the one used in main window
void PlotWindow::updatePalette()
{
    setPalette(gpMainWindow->palette());
}


//! @brief Creates a new plot window and adds the curves from current plot tab
void PlotWindow::createPlotWindowFromTab()
{
    PlotWindow *pPlotWindow = new PlotWindow(mpPlotVariableTree, gpMainWindow);
    pPlotWindow->show();
    for(int i=0; i<getCurrentPlotTab()->getCurves().size(); ++i)
    {
        pPlotWindow->addPlotCurve(getCurrentPlotTab()->getCurves().at(i)->getGeneration(), getCurrentPlotTab()->getCurves().at(i)->getComponentName(), getCurrentPlotTab()->getCurves().at(i)->getPortName(), getCurrentPlotTab()->getCurves().at(i)->getDataName(), getCurrentPlotTab()->getCurves().at(i)->getDataUnit(), getCurrentPlotTab()->getCurves().at(i)->getAxisY());
    }
}


//! @brief Constructor for plot info box
//! @param pParentPlotCurve pointer to parent plot curve
//! @param parent Pointer to parent widget
PlotInfoBox::PlotInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent)
    : QWidget(parent)
{
    mpParentPlotCurve = pParentPlotCurve;

    mpTitle = new QLabel(pParentPlotCurve->mComponentName+", "+pParentPlotCurve->mPortName+", "+pParentPlotCurve->mDataName+" ["+pParentPlotCurve->mDataUnit+"]",this);

    mpColorBlob = new QToolButton(this);
    QColor color = mpParentPlotCurve->mLineColor;
    QString redString, greenString, blueString;
    redString.setNum(color.red());
    greenString.setNum(color.green());
    blueString.setNum(color.blue());
    QString buttonStyle;
    buttonStyle.append("QToolButton			{ border: 1px solid gray;               border-style: outset;	border-radius: 0px;    	padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:pressed   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:checked		{ border: 1px solid gray;               border-style: inset;    border-radius: 0px;    	padding: 1px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:checked   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 0px;    	padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:unchecked   	{ border: 1px solid gray;               border-style: outset;   border-radius: 0px;     padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    mpColorBlob->setStyleSheet(buttonStyle);

    mpColorBlob->setFixedSize(20, 20);
    mpColorBlob->setCheckable(true);
    mpColorBlob->setChecked(false);

    mpColorButton = new QToolButton(this);
    mpColorButton->setToolTip("Select Line Color");
    mpColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LineColor.png"));
    mpColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpColorButton->setFixedSize(25, 25);

    mpFrequencyAnalysisButton = new QToolButton(this);
    mpFrequencyAnalysisButton->setToolTip("Frequency Analysis");
    mpFrequencyAnalysisButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-FrequencyAnalysis.png"));
    mpFrequencyAnalysisButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpFrequencyAnalysisButton->setFixedSize(25, 25);

    mpScaleButton = new QToolButton(this);
    mpScaleButton->setToolTip("Scale Curve");
    mpScaleButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-PlotCurveScale.png"));
    mpScaleButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpScaleButton->setFixedSize(25, 25);

    mpSizeLabel = new QLabel(tr("Line Width: "));
    mpSizeLabel->setAcceptDrops(false);
    mpSizeSpinBox = new QSpinBox(this);
    mpSizeSpinBox->setAcceptDrops(false);
    mpSizeSpinBox->setRange(1,10);
    mpSizeSpinBox->setSingleStep(1);
    mpSizeSpinBox->setValue(2);
    mpSizeSpinBox->setSuffix(" pt");

    mpPreviousButton = new QToolButton(this);
    mpPreviousButton->setToolTip("Previous Generation");
    mpPreviousButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepLeft.png"));
    mpPreviousButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpNextButton = new QToolButton(this);
    mpNextButton->setToolTip("Next Generation");
    mpNextButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepRight.png"));

    mpGenerationLabel = new QLabel(this);
    QFont tempFont = mpGenerationLabel->font();
    tempFont.setBold(true);
    mpGenerationLabel->setFont(tempFont);

    mpAutoUpdateCheckBox = new QCheckBox("Auto Update");
    mpAutoUpdateCheckBox->setChecked(true);

    mpCloseButton = new QToolButton(this);
    mpCloseButton->setToolTip("Next Generation");
    mpCloseButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
    mpCloseButton->setFixedSize(20, 20);

    QLabel *pDummy = new QLabel((" "), this);       //This is used to avoid "stretching" the icons with the window

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpColorBlob,                0,  0);
    mpLayout->addWidget(mpTitle,                    0,  1);
    mpLayout->addWidget(mpGenerationLabel,          0,  2);
    mpLayout->addWidget(mpPreviousButton,           0,  3);
    mpLayout->addWidget(mpNextButton,               0,  4);
    mpLayout->addWidget(mpAutoUpdateCheckBox,       0,  5);
    mpLayout->addWidget(mpFrequencyAnalysisButton,  0,  6);
    mpLayout->addWidget(mpScaleButton,              0,  7);
    mpLayout->addWidget(mpSizeSpinBox,              0,  8);
    mpLayout->addWidget(mpColorButton,              0,  9);
    mpLayout->addWidget(mpCloseButton,              0,  10);
    mpLayout->addWidget(pDummy,                     0,  11);


    mpLayout->setColumnStretch(0, 0);
    mpLayout->setColumnStretch(1, 0);
    mpLayout->setColumnStretch(2, 0);
    mpLayout->setColumnStretch(3, 0);
    mpLayout->setColumnStretch(4, 0);
    mpLayout->setColumnStretch(5, 0);
    mpLayout->setColumnStretch(6, 0);
    mpLayout->setColumnStretch(7, 0);
    mpLayout->setColumnStretch(8, 0);
    mpLayout->setColumnStretch(9, 0);
    mpLayout->setColumnStretch(10, 0);
    mpLayout->setColumnStretch(11, 1);

    setLayout(mpLayout);

    connect(mpColorBlob,                SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setActive(bool)));
    connect(mpPreviousButton,           SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setPreviousGeneration()));
    connect(mpNextButton,               SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setNextGeneration()));
    connect(mpAutoUpdateCheckBox,       SIGNAL(toggled(bool)),  mpParentPlotCurve,  SLOT(setAutoUpdate(bool)));
    connect(mpFrequencyAnalysisButton,  SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(performFrequencyAnalysis()));
}


//! @brief Constructor for the plot tab widget
//! @param parent Pointer to the plot window the plot tab widget belongs to
PlotTabWidget::PlotTabWidget(PlotWindow *parent)
    : QTabWidget(parent)
{
    mpParentPlotWindow = parent;
    setTabsClosable(true);
    setMouseTracking(true);

    //connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closePlotTab(int)));
    connect(this,SIGNAL(currentChanged(int)),SLOT(tabChanged()));
}


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
    return qobject_cast<PlotTab *>(currentWidget());
}


//! @brief Returns a pointer to plot tab with index i
//! @param i Index of plot tab
PlotTab *PlotTabWidget::getTab(int i)
{
    return qobject_cast<PlotTab *>(widget(i));
}


//! @brief Slot that updates all necessary things when plot tab changes
void PlotTabWidget::tabChanged()
{
    if(count() > 0) { this->show(); }
    else { this->hide(); }

    for(int i=0; i<count(); ++i)
    {
        disconnect(mpParentPlotWindow->mpZoomButton,                SIGNAL(toggled(bool)),  getTab(i),  SLOT(enableZoom(bool)));
        disconnect(mpParentPlotWindow->mpPanButton,                 SIGNAL(toggled(bool)),  getTab(i),  SLOT(enablePan(bool)));
        disconnect(mpParentPlotWindow->mpBackgroundColorButton,     SIGNAL(triggered()),      getTab(i),  SLOT(setBackgroundColor()));
        disconnect(mpParentPlotWindow->mpGridButton,                SIGNAL(toggled(bool)),  getTab(i),  SLOT(enableGrid(bool)));
        disconnect(mpParentPlotWindow->mpResetXVectorButton,        SIGNAL(triggered()),      getTab(i),  SLOT(resetXVector()));
        disconnect(mpParentPlotWindow->mpExportToCsvAction,         SIGNAL(triggered()),    getTab(i),  SLOT(exportToXml()));
        disconnect(mpParentPlotWindow->mpExportToXmlAction,         SIGNAL(triggered()),    getTab(i),  SLOT(exportToCsv()));
        disconnect(mpParentPlotWindow->mpExportToMatlabAction,      SIGNAL(triggered()),    getTab(i),  SLOT(exportToMatlab()));
        disconnect(mpParentPlotWindow->mpExportToGnuplotAction,     SIGNAL(triggered()),    getTab(i),  SLOT(exportToGnuplot()));
        disconnect(mpParentPlotWindow->mpExportPdfAction,           SIGNAL(triggered()),    getTab(i),  SLOT(exportToPdf()));
        disconnect(mpParentPlotWindow->mpExportPngAction,           SIGNAL(triggered()),    getTab(i),  SLOT(exportToPng()));
    }

    if(this->count() != 0)
    {
        if(getCurrentTab()->isSpecialPlot())
        {
            mpParentPlotWindow->mpZoomButton->setDisabled(true);
            mpParentPlotWindow->mpPanButton->setDisabled(true);
            mpParentPlotWindow->mpSaveButton->setDisabled(true);
            mpParentPlotWindow->mpExportToCsvAction->setDisabled(true);
            mpParentPlotWindow->mpExportToGnuplotAction->setDisabled(true);
            mpParentPlotWindow->mpExportToMatlabAction->setDisabled(true);
            mpParentPlotWindow->mpLoadFromXmlButton->setDisabled(true);
            mpParentPlotWindow->mpGridButton->setDisabled(true);
            mpParentPlotWindow->mpBackgroundColorButton->setDisabled(true);
            mpParentPlotWindow->mpNewWindowFromTabButton->setDisabled(true);
            mpParentPlotWindow->mpResetXVectorButton->setDisabled(true);
            mpParentPlotWindow->mpBodePlotButton->setDisabled(true);
            mpParentPlotWindow->mpExportPdfAction->setDisabled(true);
        }
        else
        {
            mpParentPlotWindow->mpZoomButton->setDisabled(false);
            mpParentPlotWindow->mpPanButton->setDisabled(false);
            mpParentPlotWindow->mpSaveButton->setDisabled(false);
            mpParentPlotWindow->mpExportToCsvAction->setDisabled(false);
            mpParentPlotWindow->mpExportToGnuplotAction->setDisabled(false);
            mpParentPlotWindow->mpExportToMatlabAction->setDisabled(false);
            mpParentPlotWindow->mpExportGfxButton->setDisabled(false);
            mpParentPlotWindow->mpLoadFromXmlButton->setDisabled(false);
            mpParentPlotWindow->mpGridButton->setDisabled(false);
            mpParentPlotWindow->mpBackgroundColorButton->setDisabled(false);
            mpParentPlotWindow->mpNewWindowFromTabButton->setDisabled(false);
            mpParentPlotWindow->mpResetXVectorButton->setDisabled(false);
            mpParentPlotWindow->mpBodePlotButton->setDisabled(false);
            mpParentPlotWindow->mpExportPdfAction->setDisabled(false);
            mpParentPlotWindow->mpZoomButton->setChecked(getCurrentTab()->mpZoomer[FIRSTPLOT]->isEnabled());
            mpParentPlotWindow->mpPanButton->setChecked(getCurrentTab()->mpPanner[FIRSTPLOT]->isEnabled());
            mpParentPlotWindow->mpGridButton->setChecked(getCurrentTab()->mpGrid[FIRSTPLOT]->isVisible());
            mpParentPlotWindow->mpResetXVectorButton->setEnabled(getCurrentTab()->mHasSpecialXAxis);
            mpParentPlotWindow->mpBodePlotButton->setEnabled(getCurrentTab()->getCurves(FIRSTPLOT).size() > 1);
        }

        connect(mpParentPlotWindow->mpZoomButton,               SIGNAL(toggled(bool)),  getCurrentTab(),    SLOT(enableZoom(bool)));
        connect(mpParentPlotWindow->mpPanButton,                SIGNAL(toggled(bool)),  getCurrentTab(),    SLOT(enablePan(bool)));
        connect(mpParentPlotWindow->mpBackgroundColorButton,    SIGNAL(triggered()),    getCurrentTab(),    SLOT(setBackgroundColor()));
        connect(mpParentPlotWindow->mpGridButton,               SIGNAL(toggled(bool)),  getCurrentTab(),    SLOT(enableGrid(bool)));
        connect(mpParentPlotWindow->mpResetXVectorButton,       SIGNAL(triggered()),    getCurrentTab(),    SLOT(resetXVector()));
        connect(mpParentPlotWindow->mpExportToXmlAction,        SIGNAL(triggered()),    getCurrentTab(),    SLOT(exportToXml()));
        connect(mpParentPlotWindow->mpExportToCsvAction,        SIGNAL(triggered()),    getCurrentTab(),    SLOT(exportToCsv()));
        connect(mpParentPlotWindow->mpExportToMatlabAction,     SIGNAL(triggered()),    getCurrentTab(),    SLOT(exportToMatlab()));
        connect(mpParentPlotWindow->mpExportToGnuplotAction,    SIGNAL(triggered()),    getCurrentTab(),    SLOT(exportToGnuplot()));
        connect(mpParentPlotWindow->mpExportPdfAction,          SIGNAL(triggered()),    getCurrentTab(),    SLOT(exportToPdf()));
        connect(mpParentPlotWindow->mpExportPngAction,          SIGNAL(triggered()),    getCurrentTab(),    SLOT(exportToPng()));
    }
}


//! @brief Constructor for plot tabs.
//! @param parent Pointer to the plot window the tab belongs to
PlotTab::PlotTab(PlotWindow *parent)
    : QWidget(parent)
{
    mpParentPlotWindow = parent;
    setAcceptDrops(true);
    setMouseTracking(true);
    mHasSpecialXAxis=false;
    mVectorXLabel = QString("Time [s]");
    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;
    mBottomAxisLogarithmic = false;
    mIsSpecialPlot = false;

        //Initiate default values for left y-axis
    mCurrentUnitsLeft.insert("Value", gConfig.getDefaultUnit("Value"));
    mCurrentUnitsLeft.insert("Pressure", gConfig.getDefaultUnit("Pressure"));
    mCurrentUnitsLeft.insert("Flow", gConfig.getDefaultUnit("Flow"));
    mCurrentUnitsLeft.insert("Position", gConfig.getDefaultUnit("Position"));
    mCurrentUnitsLeft.insert("Velocity", gConfig.getDefaultUnit("Velocity"));
    mCurrentUnitsLeft.insert("Force", gConfig.getDefaultUnit("Force"));
    mCurrentUnitsLeft.insert("Torque", gConfig.getDefaultUnit("Torque"));
    mCurrentUnitsLeft.insert("Angle", gConfig.getDefaultUnit("Angle"));
    mCurrentUnitsLeft.insert("Angular Velocity", gConfig.getDefaultUnit("Angular Velocity"));

        //Initiate default values for right y-axis
    mCurrentUnitsRight.insert("Value", gConfig.getDefaultUnit("Value"));
    mCurrentUnitsRight.insert("Pressure", gConfig.getDefaultUnit("Pressure"));
    mCurrentUnitsRight.insert("Flow", gConfig.getDefaultUnit("Flow"));
    mCurrentUnitsRight.insert("Position", gConfig.getDefaultUnit("Position"));
    mCurrentUnitsRight.insert("Velocity", gConfig.getDefaultUnit("Velocity"));
    mCurrentUnitsRight.insert("Force", gConfig.getDefaultUnit("Force"));
    mCurrentUnitsRight.insert("Torque", gConfig.getDefaultUnit("Torque"));
    mCurrentUnitsRight.insert("Angle", gConfig.getDefaultUnit("Angle"));
    mCurrentUnitsRight.insert("Angular Velocity", gConfig.getDefaultUnit("Angular Velocity"));

    mCurveColors << "Blue" << "Red" << "Green" << "Orange" << "Pink" << "Brown" << "Purple" << "Gray";

    for(int plotID=0; plotID<2; ++plotID)
    {
        //Plots
        mpPlot[plotID] = new QwtPlot(mpParentPlotWindow);
        mpPlot[plotID]->setMouseTracking(true);
        mpPlot[plotID]->setAcceptDrops(false);
        mpPlot[plotID]->setCanvasBackground(QColor(Qt::white));
        mpPlot[plotID]->setAutoReplot(true);

        //Panning Tool
        mpPanner[plotID] = new QwtPlotPanner(mpPlot[plotID]->canvas());
        mpPanner[plotID]->setMouseButton(Qt::LeftButton);
        mpPanner[plotID]->setEnabled(false);

        //Rubber Band Zoom
        mpZoomer[plotID] = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpPlot[plotID]->canvas());      //Zoomer for left y axis
        mpZoomer[plotID]->setMaxStackDepth(10000);
        mpZoomer[plotID]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomer[plotID]->setRubberBandPen(QColor(Qt::green));
        mpZoomer[plotID]->setTrackerMode(QwtPicker::ActiveOnly);
        mpZoomer[plotID]->setTrackerPen(QColor(Qt::white));
        mpZoomer[plotID]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        mpZoomer[plotID]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        mpZoomer[plotID]->setZoomBase(QRectF());
        mpZoomer[plotID]->setEnabled(false);

        mpZoomerRight[plotID] = new QwtPlotZoomer( QwtPlot::xTop, QwtPlot::yRight, mpPlot[plotID]->canvas());   //Zoomer for right y axis
        mpZoomerRight[plotID]->setMaxStackDepth(10000);
        mpZoomerRight[plotID]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[plotID]->setRubberBandPen(QColor(Qt::green));
        mpZoomerRight[plotID]->setTrackerMode(QwtPicker::ActiveOnly);
        mpZoomerRight[plotID]->setTrackerPen(QColor(Qt::white));
        mpZoomerRight[plotID]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        mpZoomerRight[plotID]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        mpZoomerRight[plotID]->setEnabled(false);

        //Wheel Zoom
        mpMagnifier[plotID] = new QwtPlotMagnifier(mpPlot[plotID]->canvas());
        mpMagnifier[plotID]->setAxisEnabled(QwtPlot::yLeft, true);
        mpMagnifier[plotID]->setAxisEnabled(QwtPlot::yRight, true);
        mpMagnifier[plotID]->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
        mpMagnifier[plotID]->setWheelFactor(1.1);
        mpMagnifier[plotID]->setEnabled(true);

        mpGrid[plotID] = new QwtPlotGrid;
        mpGrid[plotID]->enableXMin(true);
        mpGrid[plotID]->enableYMin(true);
        mpGrid[plotID]->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
        mpGrid[plotID]->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
        mpGrid[plotID]->attach(mpPlot[plotID]);
    }

    mpBarPlot = new QSint::BarChartPlotter(this);

        //Curve Marker Symbol
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setStyle(QwtSymbol::XCross);
    mpMarkerSymbol->setSize(10,10);

    QwtLegend *tempLegend = new QwtLegend();
    tempLegend->setAutoFillBackground(false);

    QList<QWidget *> tempList = tempLegend->findChildren<QWidget *>();
    for(int i=0; i<tempList.size(); ++i)
    {
        tempList.at(i)->setAutoFillBackground(false);
    }

    mpLayout = new QGridLayout(this);
    mpPlot[FIRSTPLOT]->insertLegend(tempLegend, QwtPlot::TopLegend);
    for(int plotID=0; plotID<2; ++plotID)
    {
        mpPlot[plotID]->setAutoFillBackground(false);
        mpLayout->addWidget(mpPlot[plotID]);
    }

    this->setLayout(mpLayout);

    for(int plotID=1; plotID<2; ++plotID)       //Hide all plots except first one by default
    {
        showPlot(HopsanPlotID(plotID), false);
    }
    mpBarPlot->setVisible(false);

    mpPlot[FIRSTPLOT]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


//! @brief Destructor for plot tab. Removes all curves before tab is deleted.
PlotTab::~PlotTab()
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        while(!mPlotCurvePtrs[plotID].empty())
        {
            removeCurve(mPlotCurvePtrs[plotID].last());
        }
    }
}


void PlotTab::setTabName(QString name)
{
    mpParentPlotWindow->mpPlotTabs->setTabText(mpParentPlotWindow->mpPlotTabs->indexOf(this), name);
}


void PlotTab::addBarChart(QStandardItemModel *pItemModel)
{
    mIsSpecialPlot = true;
    mpParentPlotWindow->mpZoomButton->setDisabled(true);
    mpParentPlotWindow->mpPanButton->setDisabled(true);
    mpParentPlotWindow->mpSaveButton->setDisabled(true);
    //mpParentPlotWindow->mpExportButton->setDisabled(true);
    mpParentPlotWindow->mpExportToCsvAction->setDisabled(true);
    mpParentPlotWindow->mpExportToGnuplotAction->setDisabled(true);
    mpParentPlotWindow->mpExportToMatlabAction->setDisabled(true);
    mpParentPlotWindow->mpLoadFromXmlButton->setDisabled(true);
    mpParentPlotWindow->mpGridButton->setDisabled(true);
    mpParentPlotWindow->mpBackgroundColorButton->setDisabled(true);
    mpParentPlotWindow->mpNewWindowFromTabButton->setDisabled(true);
    mpParentPlotWindow->mpResetXVectorButton->setDisabled(true);
    mpParentPlotWindow->mpBodePlotButton->setDisabled(true);
    mpParentPlotWindow->mpExportPdfAction->setDisabled(true);

    for(int i=0; i<2; ++i)
    {
        mpPlot[i]->setVisible(false);
    }
    mpBarPlot->setVisible(true);

    int min=0;
    int max=0;
    for(int c=0; c<pItemModel->columnCount(); ++c)
    {
        int componentMin = 0;
        int componentMax = 0;
        for(int r=0; r<pItemModel->rowCount(); ++r)
        {
            double data = pItemModel->data(pItemModel->index(r, c)).toInt();
            if(data > 0)
            {
                componentMax += data;
            }
            if(data < 0)
            {
                componentMin += data;
            }
        }

        min=std::min(min, componentMin);
        max=std::max(max, componentMax);
    }

    mpBarPlot->axisY()->setRanges(min, max);

    mpBarPlot->axisY()->setTicks(max/50, max/10);                     //Minor & major
    mpBarPlot->axisY()->setPen(QPen(Qt::darkGray));
    mpBarPlot->axisY()->setMinorTicksPen(QPen(Qt::gray));
    mpBarPlot->axisY()->setMajorTicksPen(QPen(Qt::darkGray));
    //mpBarPlot->axisY()->setMinorGridPen(QPen(Qt::gray));
    mpBarPlot->axisY()->setMajorGridPen(QPen(Qt::lightGray));
    mpBarPlot->axisY()->setTextColor(Qt::black);
    mpBarPlot->axisY()->setOffset(int(log10(max)+1)*10);
    //qDebug() << "Max = " << max << ", offset = " << mpBarPlot->axisY()->offset();

    mpBarPlot->axisX()->setPen(QPen(Qt::darkGray));
    mpBarPlot->axisX()->setMinorTicksPen(QPen(Qt::gray));
    mpBarPlot->axisX()->setMajorTicksPen(QPen(Qt::darkGray));
    mpBarPlot->axisX()->setMajorGridPen(QPen(Qt::lightGray));
    mpBarPlot->axisX()->setTextColor(Qt::black);
    mpBarPlot->axisX()->setFont(QFont("Calibri", 9));

    mpBarPlot->setBarSize(32, 128);
    mpBarPlot->setBarOpacity(0.9);

    QLinearGradient bg(0,0,0,1);
    bg.setCoordinateMode(QGradient::ObjectBoundingMode);
    bg.setColorAt(1, QColor(0xccccff));
    bg.setColorAt(0.0, Qt::white);
    mpBarPlot->setBackground(QBrush(bg));
    //mpBarPlot->setBackground(QColor("White"));

    mpBarPlot->setModel(pItemModel);

    mpLayout->addWidget(mpBarPlot);
}


//! @brief Adds a plot curve to a plot tab
//! @param curve Pointer to the plot curve
//! @param desiredColor Desired color for curve (will override default colors)
void PlotTab::addCurve(PlotCurve *curve, QColor desiredColor, HopsanPlotID plotID)
{
    if(mHasSpecialXAxis)
    {
        curve->getCurvePtr()->setSamples(mVectorX, curve->getDataVector());
    }

    mPlotCurvePtrs[plotID].append(curve);

    if(desiredColor == QColor())
    {
        int i=0;
        while(mUsedColors.contains(mCurveColors.first()))
        {
            mCurveColors.append(mCurveColors.first());
            mCurveColors.pop_front();
            ++i;
            if(i>mCurveColors.size()) break;
        }
        mUsedColors.append(mCurveColors.first());
        curve->setLineColor(mCurveColors.first());
    }
    else
    {
        curve->setLineColor(desiredColor);
    }

    mpPlot[plotID]->enableAxis(curve->getAxisY());
    rescaleToCurves();
    updateLabels();
    mpPlot[plotID]->replot();

    curve->setLineWidth(2);

    //mpPlot[plotID]->

    mpParentPlotWindow->mpBodePlotButton->setEnabled(mPlotCurvePtrs[FIRSTPLOT].size() > 1);
}


//! @brief Rescales the axes and the zommers so that all plot curves will fit
void PlotTab::rescaleToCurves()
{
    //Cycle plots and rescale each of them
    for(int plotID=0; plotID<2; ++plotID)
    {
        double xMin, xMax, yMinLeft, yMaxLeft, yMinRight, yMaxRight;

        xMin=0;         //Min value for X axis
        xMax=10;        //Max value for X axis
        yMinLeft=0;     //Min value for left Y axis
        yMaxLeft=10;    //Max value for left Y axis
        yMinRight=0;    //Min value for right Y axis
        yMaxRight=10;   //Max value for right Y axis

        //Cycle plots
        if(!mPlotCurvePtrs[plotID].empty())
        {
            bool foundFirstLeft = false;        //Tells that first left axis curve was found
            bool foundFirstRight = false;       //Tells that first right axis curve was found

            //Initialize values for X axis by using the first curve
            xMin=mPlotCurvePtrs[plotID].first()->getCurvePtr()->minXValue();
            xMax=mPlotCurvePtrs[plotID].first()->getCurvePtr()->maxXValue();

            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yLeft)
                {
                    if(foundFirstLeft == false)     //First left-axis curve, use min and max Y values as initial values
                    {
                        yMinLeft=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minYValue();
                        yMaxLeft=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxYValue();
                        foundFirstLeft = true;
                    }
                    else    //Compare min/max Y value with previous and change if the new one is smaller/larger
                    {
                        if(mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minYValue() < yMinLeft)
                            yMinLeft=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minYValue();
                        if(mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxYValue() > yMaxLeft)
                            yMaxLeft=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxYValue();
                    }
                }

                if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yRight)
                {
                    if(foundFirstRight == false)    //First right-axis curve, use min and max Y values as initial values
                    {
                        yMinRight=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minYValue();
                        yMaxRight=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxYValue();
                        foundFirstRight = true;
                    }
                    else    //Compare min/max Y value with previous and change if the new one is smaller/larger
                    {
                        if(mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minYValue() < yMinRight)
                            yMinRight=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minYValue();
                        if(mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxYValue() > yMaxRight)
                            yMaxRight=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxYValue();
                    }
                }

                //Compare min/max X value with previous and change if the new one is smaller/larger
                if(mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minXValue() < xMin)
                    xMin=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->minXValue();
                if(mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxXValue() > xMax)
                    xMax=mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->maxXValue();

            }
        }

        //Max and min must not be same value; if they are, decrease/increase them by one
        if(yMaxLeft == yMinLeft)
        {
            yMaxLeft = yMaxLeft+1;
            yMinLeft = yMinLeft-1;
        }
        if(yMaxRight == yMinRight)
        {
            yMaxRight = yMaxRight+1;
            yMinRight = yMinRight-1;
        }

        //Calculate heights (used for calculating margins at top and bottom
        double heightLeft = yMaxLeft-yMinLeft;
        double heightRight = yMaxRight-yMinRight;

        //If plot has log scale, we use a different approach for calculating margins
        //(fixed margins would not make sense with a log scale)
        if(mpPlot[plotID]->axisScaleEngine(QwtPlot::yLeft)->transformation()->type() == QwtScaleTransformation::Log10)
        {
            heightLeft = 0;
            yMaxLeft = yMaxLeft*2;
            yMinLeft = yMinLeft/2;
        }
        if(mpPlot[plotID]->axisScaleEngine(QwtPlot::yRight)->transformation()->type() == QwtScaleTransformation::Log10)
        {
            heightRight = 0;
            yMaxRight = yMaxRight*2;
            yMinRight = yMinRight/2;
        }

        //Scale the axes
        mpPlot[plotID]->setAxisScale(QwtPlot::yLeft, yMinLeft-0.05*heightLeft, yMaxLeft+0.05*heightLeft);
        mpPlot[plotID]->setAxisScale(QwtPlot::yRight, yMinRight-0.05*heightRight, yMaxRight+0.05*heightRight);
        mpPlot[plotID]->setAxisScale(QwtPlot::xBottom, xMin, xMax);
        mpPlot[plotID]->updateAxes();

        //Scale the zoom base (maximum zoom)
        QRectF tempDoubleRect;
        tempDoubleRect.setX(xMin);
        tempDoubleRect.setY(yMinLeft-0.05*heightLeft);
        tempDoubleRect.setWidth(xMax-xMin);
        tempDoubleRect.setHeight(yMaxLeft-yMinLeft+0.1*heightLeft);
        mpZoomer[plotID]->setZoomBase(tempDoubleRect);

        QRectF tempDoubleRect2;
        tempDoubleRect2.setX(xMin);
        tempDoubleRect2.setY(yMinRight-0.05*heightRight);
        tempDoubleRect2.setHeight(yMaxRight-yMinRight+0.1*heightRight);
        tempDoubleRect2.setWidth(xMax-xMin);
        mpZoomerRight[plotID]->setZoomBase(tempDoubleRect2);
    }

    //Curve Marker
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setStyle(QwtSymbol::XCross);
    mpMarkerSymbol->setSize(10,10);
}


//! @brief Removes a curve from the plot tab
//! @param curve Pointer to curve to remove
void PlotTab::removeCurve(PlotCurve *curve)
{
    int plotID = getPlotIDFromCurve(curve);

    for(int i=0; i<mMarkerPtrs[plotID].size(); ++i)
    {
        if(mMarkerPtrs[plotID].at(i)->getCurve() == curve)
        {
            mpPlot[plotID]->canvas()->removeEventFilter(mMarkerPtrs[plotID].at(i));
            mMarkerPtrs[plotID].at(i)->detach();
            mMarkerPtrs[plotID].removeAt(i);
            --i;
        }
    }

    for(int i=0; i<mUsedColors.size(); ++i)
    {
        if(curve->getCurvePtr()->pen().color() == mUsedColors.at(i))
        {
            mUsedColors.removeAt(i);
            break;
        }
    }

    curve->getCurvePtr()->detach();
    for(int plotID=0; plotID<2; ++plotID)
    {
        mPlotCurvePtrs[plotID].removeAll(curve);
    }
    delete(curve);
    rescaleToCurves();
    updateLabels();
    update();
}


//! @brief Changes the X vector of current plot tab to specified variable
//! @param xArray New data for X-axis
//! @param componentName Name of component from which new data origins
//! @param portName Name of port form which new data origins
//! @param dataName Data name (physical quantity) of new data
//! @param dataUnit Unit of new data
void PlotTab::changeXVector(QVector<double> xArray, QString componentName, QString portName, QString dataName, QString dataUnit, HopsanPlotID plotID)
{
    mHasSpecialXAxis = true;

    mVectorX = xArray;

    for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
    {
        mPlotCurvePtrs[plotID].at(i)->getCurvePtr()->setSamples(mVectorX, mPlotCurvePtrs[plotID].at(i)->getDataVector());
        mPlotCurvePtrs[plotID].at(i)->setDataUnit(mPlotCurvePtrs[plotID].at(i)->getDataUnit());
    }

    rescaleToCurves();

    mVectorXModelPath = gpMainWindow->mpProjectTabs->getCurrentContainer()->getModelFileInfo().filePath();
    mVectorXComponent = componentName;
    mVectorXPortName = portName;
    mVectorXDataName = dataName;
    mVectorXDataUnit = dataUnit;
    mVectorXGeneration = gpMainWindow->mpProjectTabs->getCurrentContainer()->getNumberOfPlotGenerations()-1;

    mVectorXLabel = QString(dataName + " [" + dataUnit + "]");
    updateLabels();
    update();
    mVectorX = xArray;
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(true);
}


//! @brief Updates labels on plot axes
void PlotTab::updateLabels()
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        mpPlot[plotID]->setAxisTitle(QwtPlot::yLeft, QwtText());
        mpPlot[plotID]->setAxisTitle(QwtPlot::yRight, QwtText());

        if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == PORTVARIABLE)
        {
            QStringList leftUnits, rightUnits;
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                QString newUnit = QString(mPlotCurvePtrs[plotID].at(i)->getDataName() + " [" + mPlotCurvePtrs[plotID].at(i)->getDataUnit() + "]");
                if( !(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yLeft && leftUnits.contains(newUnit)) && !(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yRight && rightUnits.contains(newUnit)) )
                {
                    if(!mpPlot[plotID]->axisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY()).isEmpty())
                    {
                        mpPlot[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), QwtText(QString(mpPlot[plotID]->axisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY()).text().append(", "))));
                    }
                    mpPlot[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), QwtText(QString(mpPlot[plotID]->axisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY()).text().append(newUnit))));
                    if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yLeft)
                    {
                        leftUnits.append(newUnit);
                    }
                    if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yRight)
                    {
                        rightUnits.append(newUnit);
                    }
                }
            }
            mpPlot[plotID]->setAxisTitle(QwtPlot::xBottom, QwtText(mVectorXLabel));
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == FREQUENCYANALYSIS)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpPlot[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Relative Magnitude [-]");
                mpPlot[plotID]->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == NYQUIST)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpPlot[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Im");
                mpPlot[plotID]->setAxisTitle(QwtPlot::xBottom, "Re");
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == BODEGAIN)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpPlot[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Magnitude [dB]");
                mpPlot[plotID]->setAxisTitle(QwtPlot::xBottom, QwtText());      //No label, because there will be a phase plot bellow with same label
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == BODEPHASE)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpPlot[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Phase [deg]");
                mpPlot[plotID]->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
            }
        }
    }
}


bool PlotTab::isGridVisible()
{
    return mpGrid[FIRSTPLOT]->isVisible();
}


void PlotTab::resetXVector()
{
    mHasSpecialXAxis = false;

    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
    {
        mPlotCurvePtrs[FIRSTPLOT].at(i)->getCurvePtr()->setSamples(mPlotCurvePtrs[FIRSTPLOT].at(i)->getTimeVector(), mPlotCurvePtrs[FIRSTPLOT].at(i)->getDataVector());
        mPlotCurvePtrs[FIRSTPLOT].at(i)->setDataUnit(mPlotCurvePtrs[FIRSTPLOT].at(i)->getDataUnit());
    }

    mVectorXLabel = QString("Time [s]");
    updateLabels();
    update();

    rescaleToCurves();
    mpPlot[FIRSTPLOT]->replot();

    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


//! @brief Slot that opens a dialog from where user can export current plot tab to a XML file
void PlotTab::exportToXml()
{

        //Open a dialog where text and font can be selected
    mpExportXmlDialog = new QDialog(gpMainWindow);
    mpExportXmlDialog->setWindowTitle("Export Plot Tab To XML");

    QLabel *pXmlIndentationLabel = new QLabel("Indentation: ");

    mpXmlIndentationSpinBox = new QSpinBox(this);
    mpXmlIndentationSpinBox->setRange(0,100);
    mpXmlIndentationSpinBox->setValue(2);

    mpIncludeTimeCheckBox = new QCheckBox("Include date && time");
    mpIncludeTimeCheckBox->setChecked(true);

    mpIncludeDescriptionsCheckBox = new QCheckBox("Include variable descriptions");
    mpIncludeDescriptionsCheckBox->setChecked(true);

    QLabel *pOutputLabel = new QLabel("Output data:");

    mpXmlOutputTextBox = new QTextEdit();
    mpXmlOutputTextBox->toHtml();
    mpXmlOutputTextBox->setReadOnly(true);
    mpXmlOutputTextBox->setMinimumSize(700, 210);

    QPushButton *pDoneInDialogButton = new QPushButton("Export");
    QPushButton *pCancelInDialogButton = new QPushButton("Cancel");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout();
    pDialogLayout->addWidget(pXmlIndentationLabel,          0, 0);
    pDialogLayout->addWidget(mpXmlIndentationSpinBox,       0, 1);
    pDialogLayout->addWidget(mpIncludeTimeCheckBox,         2, 0, 1, 2);
    pDialogLayout->addWidget(mpIncludeDescriptionsCheckBox, 3, 0, 1, 2);
    pDialogLayout->addWidget(pOutputLabel,                  4, 0, 1, 2);
    pDialogLayout->addWidget(mpXmlOutputTextBox,            5, 0, 1, 4);
    pDialogLayout->addWidget(pButtonBox,                    6, 2, 1, 2);

    mpExportXmlDialog->setLayout(pDialogLayout);

    connect(mpXmlIndentationSpinBox,        SIGNAL(valueChanged(int)),  this,               SLOT(updateXmlOutputTextInDialog()));
    connect(mpIncludeTimeCheckBox,          SIGNAL(toggled(bool)),      this,               SLOT(updateXmlOutputTextInDialog()));
    connect(mpIncludeDescriptionsCheckBox,  SIGNAL(toggled(bool)),      this,               SLOT(updateXmlOutputTextInDialog()));
    connect(pDoneInDialogButton,            SIGNAL(clicked()),          this,               SLOT(saveToXml()));
    connect(pCancelInDialogButton,          SIGNAL(clicked()),          mpExportXmlDialog,  SLOT(close()));

    updateXmlOutputTextInDialog();
    mpExportXmlDialog->open();
}


//! @brief Slot that exports plot tab to a specified comma-separated value file (.csv)
void PlotTab::exportToCsv()
{
        //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To CSV File"),
                                            gConfig.getPlotDataDir(),
                                            tr("Comma-separated values (*.csv)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file


        //Cycle plot curves
    if(mHasSpecialXAxis)
    {
        for(int i=0; i<mVectorX.size(); ++i)
        {
            fileStream << mVectorX[i];
            for(int j=0; j<mPlotCurvePtrs[FIRSTPLOT].size(); ++j)
            {
                fileStream << ", " << mPlotCurvePtrs[FIRSTPLOT][j]->getDataVector()[i];
            }
            fileStream << "\n";
        }
    }
    else
    {
        for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT][0]->getTimeVector().size(); ++i)
        {
            fileStream << mPlotCurvePtrs[FIRSTPLOT][0]->getTimeVector()[i];
            for(int j=0; j<mPlotCurvePtrs[FIRSTPLOT].size(); ++j)
            {
                fileStream << ", " << mPlotCurvePtrs[FIRSTPLOT][j]->getDataVector()[i];
            }
            fileStream << "\n";
        }
    }


//        //Cycle plot curves
//    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
//    {
//        fileStream << "x" << i;                                         //Write time/X vector
//        if(mHasSpecialXAxis)
//        {
//            for(int j=0; j<mVectorX.size(); ++j)
//            {
//                fileStream << "," << mVectorX[j];
//            }
//        }
//        else
//        {
//            for(int j=0; j<mPlotCurvePtrs[FIRSTPLOT][i]->getTimeVector().size(); ++j)
//            {
//                fileStream << "," << mPlotCurvePtrs[FIRSTPLOT][i]->getTimeVector()[j];
//            }
//        }
//        fileStream << "\n";

//        fileStream << "y" << i;                                             //Write data vector
//        for(int k=0; k<mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector().size(); ++k)
//        {
//            fileStream << "," << mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector()[k];
//        }
//        fileStream << "\n";
//    }

    file.close();
}


//! @brief Slot that exports plot tab to a specified matlab script file (.m)
void PlotTab::exportToMatlab()
{
        //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To MATLAB File"),
                                            gConfig.getPlotDataDir(),
                                            tr("MATLAB script file (*.m)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

        //Write initial comment
    fileStream << "% MATLAB File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";

        //Cycle plot curves
    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
    {
        fileStream << "x" << i << "=[";                                         //Write time/X vector
        if(mHasSpecialXAxis)
        {
            for(int j=0; j<mVectorX.size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mVectorX[j];
            }
        }
        else
        {
            for(int j=0; j<mPlotCurvePtrs[FIRSTPLOT][i]->getTimeVector().size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mPlotCurvePtrs[FIRSTPLOT][i]->getTimeVector()[j];
            }
        }
        fileStream << "];\n";

        fileStream << "y" << i << "=[";                                             //Write data vector
        for(int k=0; k<mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector().size(); ++k)
        {
            if(k>0) fileStream << ",";
            fileStream << mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector()[k];
        }
        fileStream << "];\n";
    }

        //Cycle plot curves
    for(int i=0; i<mPlotCurvePtrs[SECONDPLOT].size(); ++i)
    {
        fileStream << "x" << i+mPlotCurvePtrs[FIRSTPLOT].size() << "=[";                                         //Write time/X vector
        if(mHasSpecialXAxis)
        {
            for(int j=0; j<mVectorX.size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mVectorX[j];
            }
        }
        else
        {
            for(int j=0; j<mPlotCurvePtrs[SECONDPLOT][i]->getTimeVector().size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mPlotCurvePtrs[SECONDPLOT][i]->getTimeVector()[j];
            }
        }
        fileStream << "];\n";

        fileStream << "y" << i+mPlotCurvePtrs[FIRSTPLOT].size() << "=[";                                             //Write data vector
        for(int k=0; k<mPlotCurvePtrs[SECONDPLOT][i]->getDataVector().size(); ++k)
        {
            if(k>0) fileStream << ",";
            fileStream << mPlotCurvePtrs[SECONDPLOT][i]->getDataVector()[k];
        }
        fileStream << "];\n";
    }

        //Write plot functions
    QStringList matlabColors;
    matlabColors << "r" << "g" << "b" << "c" << "m" << "y";
    fileStream << "hold on\n";
    fileStream << "subplot(2,1,1)\n";
    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
    {
        if((mPlotCurvePtrs[FIRSTPLOT][i]->getAxisY() == QwtPlot::yLeft && mLeftAxisLogarithmic) || (mPlotCurvePtrs[FIRSTPLOT][i]->getAxisY() == QwtPlot::yRight && mRightAxisLogarithmic))
        {
            if(mBottomAxisLogarithmic)
                fileStream << "loglog";
            else
                fileStream << "semilogy";
        }
        else
        {
            if(mBottomAxisLogarithmic)
                fileStream << "semilogx";
            else
                fileStream << "plot";
        }
        fileStream << "(x" << i << ",y" << i << ",'-" << matlabColors[i%6] << "','linewidth'," << mPlotCurvePtrs[FIRSTPLOT][i]->getCurvePtr()->pen().width() << ")\n";
    }
    if(mPlotCurvePtrs[SECONDPLOT].size() > 0)
    {
        fileStream << "subplot(2,1,2)\n";
        for(int i=0; i<mPlotCurvePtrs[SECONDPLOT].size(); ++i)
        {
            if((mPlotCurvePtrs[SECONDPLOT][i]->getAxisY() == QwtPlot::yLeft && mLeftAxisLogarithmic) || (mPlotCurvePtrs[SECONDPLOT][i]->getAxisY() == QwtPlot::yRight && mRightAxisLogarithmic))
            {
                if(mBottomAxisLogarithmic)
                    fileStream << "loglog";
                else
                    fileStream << "semilogy";
            }
            else
            {
                if(mBottomAxisLogarithmic)
                    fileStream << "semilogx";
                else
                    fileStream << "plot";
            }
            fileStream << "(x" << i+mPlotCurvePtrs[FIRSTPLOT].size() << ",y" << i+mPlotCurvePtrs[FIRSTPLOT].size() << ",'-" << matlabColors[i%6] << "','linewidth'," << mPlotCurvePtrs[SECONDPLOT][i]->getCurvePtr()->pen().width() << ")\n";
        }
    }

    file.close();
}


//! @brief Slot that exports plot tab to specified gnuplot file  (.dat)
void PlotTab::exportToGnuplot()
{
        //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To gnuplot File"),
                                            gConfig.getPlotDataDir(),
                                            tr("gnuplot file (*.dat)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

        //Write initial comment
    fileStream << "# gnuplot File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";
    fileStream << "# T";
    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
    {
        fileStream << "                  Y" << i;
    }
    fileStream << "\n";

        //Write time and data vectors
    QString dummy;
    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].first()->getTimeVector().size(); ++i)
    {
        dummy.setNum(mPlotCurvePtrs[FIRSTPLOT].first()->getTimeVector()[i]);
        fileStream << dummy;
        for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }

        for(int k=0; k<mPlotCurvePtrs[FIRSTPLOT].size(); ++k)
        {
            dummy.setNum(mPlotCurvePtrs[FIRSTPLOT][k]->getDataVector()[i]);
            fileStream << dummy;
            for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }
        }
        fileStream << "\n";
    }

    file.close();
}


//! @brief Slot that exports plot tab as vector graphics to specified .pdf file
void PlotTab::exportToPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export File Name", gConfig.getPlotGfxDir(), "Portable Document Format (*.pdf)");
    if ( !fileName.isEmpty() )
    {
        QFileInfo file(fileName);
        gConfig.setPlotGfxDir(file.absolutePath());

        QwtPlotRenderer renderer;

        QPrinter *printer = new QPrinter(QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A4);
        printer->setOrientation(QPrinter::Landscape);
        printer->setFullPage(false);
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(fileName);
        renderer.renderTo(mpPlot[FIRSTPLOT],*printer);
    }
}


//! @brief Slot that exports plot tab as bitmap to specified .png file
void PlotTab::exportToPng()
{
    QString fileName = QFileDialog::getSaveFileName(
       this, "Export File Name", gConfig.getPlotGfxDir(),
       "Portable Network Graphics (*.png)");

    if(!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        gConfig.setPlotGfxDir(file.absolutePath());

        if(mpBarPlot->isVisible())
        {
            QPixmap pixmap = QPixmap::grabWidget(this);
            pixmap.save(fileName);
        }
        else
        {
            QPixmap pixmap(mpPlot[FIRSTPLOT]->width(), mpPlot[FIRSTPLOT]->height());
            pixmap.fill();
            QwtPlotRenderer renderer;
            renderer.renderTo(mpPlot[FIRSTPLOT], pixmap);
            pixmap.save(fileName);
        }
    }
}


void PlotTab::enableZoom(bool value)
{
    if(mpParentPlotWindow->mpPanButton->isChecked() && value)
    {
        mpParentPlotWindow->mpPanButton->setChecked(false);
        mpPanner[FIRSTPLOT]->setEnabled(false);
        mpPanner[SECONDPLOT]->setEnabled(false);
    }
    mpZoomer[FIRSTPLOT]->setEnabled(value);
    if(value)   { mpZoomer[FIRSTPLOT]->setRubberBand(QwtPicker::RectRubberBand); }
    else        { mpZoomer[FIRSTPLOT]->setRubberBand(QwtPicker::NoRubberBand); }
    mpZoomerRight[FIRSTPLOT]->setEnabled(value);
    mpZoomer[SECONDPLOT]->setEnabled(value);
    if(value)   { mpZoomer[SECONDPLOT]->setRubberBand(QwtPicker::RectRubberBand); }
    else        { mpZoomer[SECONDPLOT]->setRubberBand(QwtPicker::NoRubberBand); }
    mpZoomerRight[SECONDPLOT]->setEnabled(value);
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


void PlotTab::enablePan(bool value)
{
    if(mpParentPlotWindow->mpZoomButton->isChecked() && value)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        mpZoomer[FIRSTPLOT]->setEnabled(false);
        mpZoomer[FIRSTPLOT]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[FIRSTPLOT]->setEnabled(false);
        mpZoomer[SECONDPLOT]->setEnabled(false);
        mpZoomer[SECONDPLOT]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[SECONDPLOT]->setEnabled(false);
    }
    mpPanner[FIRSTPLOT]->setEnabled(value);
    mpPanner[SECONDPLOT]->setEnabled(value);
}


void PlotTab::enableGrid(bool value)
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        mpGrid[plotID]->setVisible(value);
    }
}


void PlotTab::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(mpPlot[FIRSTPLOT]->canvasBackground().color(), this);
    if (color.isValid())
    {
        mpPlot[FIRSTPLOT]->setCanvasBackground(color);
        mpPlot[FIRSTPLOT]->replot();
        mpPlot[SECONDPLOT]->setCanvasBackground(color);
        mpPlot[SECONDPLOT]->replot();
    }
}


QList<PlotCurve *> PlotTab::getCurves(HopsanPlotID plotID)
{
    return mPlotCurvePtrs[plotID];
}


void PlotTab::setActivePlotCurve(PlotCurve *pCurve)
{
    mpActivePlotCurve = pCurve;
}


PlotCurve *PlotTab::getActivePlotCurve()
{
    return mpActivePlotCurve;
}


QwtPlot *PlotTab::getPlot(HopsanPlotID plotID)
{
    return mpPlot[plotID];
}


void PlotTab::showPlot(HopsanPlotID plotID, bool visible)
{
    mpPlot[plotID]->setVisible(visible);
}


int PlotTab::getNumberOfCurves(HopsanPlotID plotID)
{
    return mPlotCurvePtrs[plotID].size();
}


void PlotTab::update()
{
    for(int plotID=0; plotID<1; ++plotID)
    {
        mpPlot[plotID]->enableAxis(QwtPlot::yLeft, false);
        mpPlot[plotID]->enableAxis(QwtPlot::yRight, false);
        QList<PlotCurve *>::iterator cit;
        for(cit=mPlotCurvePtrs[plotID].begin(); cit!=mPlotCurvePtrs[plotID].end(); ++cit)
        {
            if(!mpPlot[plotID]->axisEnabled((*cit)->getAxisY())) { mpPlot[plotID]->enableAxis((*cit)->getAxisY()); }
            (*cit)->getCurvePtr()->attach(mpPlot[plotID]);
        }

        for(int i=0; i<mMarkerPtrs[plotID].size(); ++i)
        {
            QPointF posF = mMarkerPtrs[plotID].at(i)->value();
            double x = mpPlot[plotID]->transform(QwtPlot::xBottom, posF.x());
            double y = mpPlot[plotID]->transform(QwtPlot::yLeft, posF.y());
            QPoint pos = QPoint(x,y);
            QwtPlotCurve *pCurve = mMarkerPtrs[plotID].at(i)->getCurve()->getCurvePtr();
            mMarkerPtrs[plotID].at(i)->setXValue(pCurve->sample(pCurve->closestPoint(pos)).x());
            mMarkerPtrs[plotID].at(i)->setYValue(mpPlot[plotID]->invTransform(QwtPlot::yLeft, mpPlot[plotID]->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));
        }
        mpPlot[plotID]->replot();
    }
}


void PlotTab::insertMarker(PlotCurve *pCurve, double x, double y, QString altLabel, bool movable)
{
    qDebug() << "x and y = " << x << ", " << y;

    int plotID = getPlotIDFromCurve(pCurve);

    mpMarkerSymbol->setPen(QPen(pCurve->getCurvePtr()->pen().brush().color(), 3));
    PlotMarker *tempMarker = new PlotMarker(pCurve, this, *mpMarkerSymbol);
    mMarkerPtrs[plotID].append(tempMarker);

    tempMarker->attach(mpPlot[plotID]);
    QCursor cursor;
    tempMarker->setXValue(x);
    tempMarker->setYValue(y);

    QString xString;
    QString yString;
    xString.setNum(x);
    yString.setNum(y);
    QwtText tempLabel;
    if(altLabel != QString())
    {
        tempLabel.setText(altLabel);
    }
    else
    {
        tempLabel.setText("("+xString+", "+yString+")");
    }
    tempLabel.setColor(pCurve->getCurvePtr()->pen().brush().color());
    tempLabel.setBackgroundBrush(QColor(255,255,255,220));
    tempLabel.setFont(QFont("Calibri", 12, QFont::Normal));
    tempMarker->setLabel(tempLabel);
    tempMarker->setLabelAlignment(Qt::AlignTop);

    mpPlot[plotID]->canvas()->installEventFilter(tempMarker);
    mpPlot[plotID]->canvas()->setMouseTracking(true);

    tempMarker->setMovable(movable);
}


//! @brief Inserts a curve marker at the specified curve
//! @param curve is a pointer to the specified curve
void PlotTab::insertMarker(PlotCurve *pCurve, QPoint pos, bool movable)
{
    int plotID = getPlotIDFromCurve(pCurve);

    mpMarkerSymbol->setPen(QPen(pCurve->getCurvePtr()->pen().brush().color(), 3));
    PlotMarker *tempMarker = new PlotMarker(pCurve, this, *mpMarkerSymbol);
    mMarkerPtrs[plotID].append(tempMarker);

    tempMarker->attach(mpPlot[plotID]);
    QCursor cursor;
    tempMarker->setXValue(pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(pos)).x());
    tempMarker->setYValue(mpPlot[plotID]->invTransform(QwtPlot::yLeft, mpPlot[plotID]->transform(pCurve->getCurvePtr()->yAxis(), pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(pos)).y())));

    QString xString;
    QString yString;
    double x = pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(pos)).x();
    double y = pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(mpPlot[plotID]->canvas()->mapFromGlobal(cursor.pos()))).y();
    xString.setNum(x);
    yString.setNum(y);
    QwtText tempLabel;
    tempLabel.setText("("+xString+", "+yString+")");
    tempLabel.setColor(pCurve->getCurvePtr()->pen().brush().color());
    tempLabel.setBackgroundBrush(QColor(255,255,255,220));
    tempLabel.setFont(QFont("Calibri", 12, QFont::Normal));
    tempMarker->setLabel(tempLabel);
    tempMarker->setLabelAlignment(Qt::AlignTop);

    mpPlot[plotID]->canvas()->installEventFilter(tempMarker);
    mpPlot[plotID]->canvas()->setMouseTracking(true);

    tempMarker->setMovable(movable);
}


//! @brief Saves the current tab to a DOM element (XML)
//! @param rDomElement Reference to the dom element to save to
//! @param dateTime Tells whether or not date and time should be included
//! @param descriptions Tells whether or not variable descriptions shall be included
void PlotTab::saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions)
{
    if(dateTime)
    {
        QDateTime datetime;
        rDomElement.setAttribute("datetime", datetime.currentDateTime().toString(Qt::ISODate));
    }

    if(mpBarPlot->isVisible())
    {
        QAbstractItemModel *model = mpBarPlot->model();

        for(int c=0; c<model->columnCount(); ++c)
        {
            double losses = model->data(model->index(0, c)).toInt() - model->data(model->index(1, c)).toInt();;

            QDomElement dataTag = appendDomElement(rDomElement, "data");
            QDomElement varTag = appendDomElement(dataTag, "losses");
            QString valueString;
            valueString.setNum(losses);
            QDomText value = varTag.ownerDocument().createTextNode(valueString);
            varTag.appendChild(value);

            if(descriptions)
            {
                varTag.setAttribute("component", model->headerData(c, Qt::Horizontal).toString());
            }
        }
    }
    else
    {

            //Cycle plot curves and write data tags
        for(int j=0; j<mPlotCurvePtrs[FIRSTPLOT][0]->getTimeVector().size(); ++j)
        {
            QDomElement dataTag = appendDomElement(rDomElement, "data");

            if(mHasSpecialXAxis)        //Special x-axis, replace time with x-data
            {
                setQrealAttribute(dataTag, mVectorXDataName, mVectorX[j], 10, 'g');
            }
            else                        //X-axis = time
            {
                setQrealAttribute(dataTag, "time", mPlotCurvePtrs[FIRSTPLOT][0]->getTimeVector()[j], 10, 'g');
            }

            //Write variable tags for each variable
            for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
            {
                QString numTemp;
                numTemp.setNum(i);
                QDomElement varTag = appendDomElement(dataTag, mPlotCurvePtrs[FIRSTPLOT][i]->getDataName()+numTemp);
                QString valueString;
                valueString.setNum(mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector()[j]);
                QDomText value = varTag.ownerDocument().createTextNode(valueString);
                varTag.appendChild(value);

                if(descriptions)
                {
                    varTag.setAttribute("component", mPlotCurvePtrs[FIRSTPLOT][i]->getComponentName());
                    varTag.setAttribute("port", mPlotCurvePtrs[FIRSTPLOT][i]->getPortName());
                    varTag.setAttribute("type", mPlotCurvePtrs[FIRSTPLOT][i]->getDataName());
                    varTag.setAttribute("unit", mPlotCurvePtrs[FIRSTPLOT][i]->getDataUnit());
                }
            }
        }
    }
}


bool PlotTab::isSpecialPlot()
{
    return mIsSpecialPlot;
}


void PlotTab::setBottomAxisLogarithmic(bool value)
{
    mBottomAxisLogarithmic = value;
    if(value)
    {
        getPlot(FIRSTPLOT)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
        getPlot(SECONDPLOT)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
    }
    else
    {
        getPlot(FIRSTPLOT)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
        getPlot(SECONDPLOT)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    }

}


bool PlotTab::hasLogarithmicBottomAxis()
{
    return mBottomAxisLogarithmic;
}


void PlotTab::setLegendsVisible(bool value)
{
    for(int c=0; c<mPlotCurvePtrs[FIRSTPLOT].size(); ++c)
    {
        mPlotCurvePtrs[FIRSTPLOT].at(c)->getCurvePtr()->setItemAttribute(QwtPlotItem::Legend, value);
    }

    for(int c=0; c<mPlotCurvePtrs[FIRSTPLOT].size(); ++c)
    {
        mPlotCurvePtrs[FIRSTPLOT].at(c)->getCurvePtr()->setItemAttribute(QwtPlotItem::Legend, value);
    }

    update();
}


//! @brief Private slot that updates the xml preview field in the export to xml dialog
QString PlotTab::updateXmlOutputTextInDialog()
{
    QDomDocument domDocument;
    QDomElement element = domDocument.createElement("hopsanplotdata");
    domDocument.appendChild(element);
    this->saveToDomElement(element, mpIncludeTimeCheckBox->isChecked(), mpIncludeDescriptionsCheckBox->isChecked());
    QString output = domDocument.toString(mpXmlIndentationSpinBox->value());

    QStringList lines = output.split("\n");

    //We want the first 10 lines and the last 2 from the xml output
    QString display;
    for(int i=0; i<10 && i<lines.size(); ++i)
    {
        display.append(lines[i]);
        display.append("\n");
    }
    for(int k=0; k<mpXmlIndentationSpinBox->value(); ++k) display.append(" ");
    if(lines.size() > 9)
    {
        display.append("...\n");
        display.append(lines[lines.size()-2]);
        display.append(lines[lines.size()-1]);
    }


    display.replace(" ", "&nbsp;");
    display.replace(">", "!!!GT!!!");
    display.replace("<", "<font color=\"saddlebrown\">&lt;");
    display.replace("!!!GT!!!","</font><font color=\"saddlebrown\">&gt;</font>");
    display.replace("\n", "<br>\n");
    display.replace("&lt;?xml", "&lt;?xml</font>");
    display.replace("&lt;data", "&lt;data</font>");

    display.replace("0&nbsp;", "0</font>&nbsp;");
    display.replace("1&nbsp;", "1</font>&nbsp;");
    display.replace("2&nbsp;", "2</font>&nbsp;");
    display.replace("3&nbsp;", "3</font>&nbsp;");
    display.replace("4&nbsp;", "4</font>&nbsp;");
    display.replace("5&nbsp;", "5</font>&nbsp;");
    display.replace("6&nbsp;", "6</font>&nbsp;");
    display.replace("7&nbsp;", "7</font>&nbsp;");
    display.replace("8&nbsp;", "8</font>&nbsp;");
    display.replace("9&nbsp;", "9</font>&nbsp;");

    display.replace("&lt;hopsanplotdata", "&lt;hopsanplotdata</font>");
    display.replace("&lt;losses", "&lt;losses</font>");
    display.replace("&nbsp;version=", "&nbsp;version=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;encoding=", "&nbsp;encoding=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;component=", "&nbsp;component=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;port=", "&nbsp;port=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;type=", "&nbsp;type=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;unit=", "&nbsp;unit=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;time=", "&nbsp;time=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;datetime=", "&nbsp;datetime=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("\"&nbsp;", "\"</font>&nbsp;");

    display.replace("&nbsp;", "<font face=\"Consolas\">&nbsp;</font>");
    display.replace("</font></font>", "</font>");

    mpXmlOutputTextBox->setText(display);

    return output;
}


//! @brief Private slot that opens a file dialog and saves the current tab to a specified XML file
//! @note Don't call this directly, call exportToXml() first and it will subsequently call this slot
void PlotTab::saveToXml()
{
        //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To XML File"),
                                            gConfig.getPlotDataDir(),
                                            tr("Extensible Markup Language (*.xml)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    QDomDocument domDocument;
    QDomElement element = domDocument.createElement("hopsanplotdata");
    domDocument.appendChild(element);
    this->saveToDomElement(element, mpIncludeTimeCheckBox->isChecked(), mpIncludeDescriptionsCheckBox->isChecked());
    appendRootXMLProcessingInstruction(domDocument);

    QTextStream fileStream(&file);
    domDocument.save(fileStream, mpXmlIndentationSpinBox->value());
    file.close();

    mpExportXmlDialog->close();
}


int PlotTab::getPlotIDFromCurve(PlotCurve *pCurve)
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        if(mPlotCurvePtrs[plotID].contains(pCurve))
            return plotID;
    }
    assert(false);      //Plot curve has no plot ID (should never happen)
    return -1;
}


//! @brief Defines what happens when used drags something into the plot window
void PlotTab::dragEnterEvent(QDragEnterEvent *event)
{
    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PORTVARIABLE) return;

    if (event->mimeData()->hasText())
    {
            //Create the hover rectangle (size will be changed by dragMoveEvent)
        mpHoverRect = new QRubberBand(QRubberBand::Rectangle,this);
        mpHoverRect->setGeometry(0, 0, this->width(), this->height());
        mpHoverRect->setWindowOpacity(1);
        mpHoverRect->show();

        event->acceptProposedAction();
    }
}


//! @brief Defines what happens when user is dragging something in the plot window.
void PlotTab::dragMoveEvent(QDragMoveEvent *event)
{
    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PORTVARIABLE) return;

    QCursor cursor;
    if(this->mapFromGlobal(cursor.pos()).y() > getPlot()->canvas()->height()/2+getPlot()->canvas()->y()+10 && getNumberOfCurves(FIRSTPLOT) >= 1)
    {
        mpHoverRect->setGeometry(getPlot()->canvas()->x()+9, getPlot()->canvas()->height()/2+getPlot()->canvas()->y()+10, getPlot()->canvas()->width(), getPlot()->canvas()->height()/2);
        mpParentPlotWindow->showHelpPopupMessage("Replace X-axis with selected variable.");
    }
    else if(this->mapFromGlobal(cursor.pos()).x() < getPlot()->canvas()->x()+9 + getPlot()->canvas()->width()/2)
    {
        mpHoverRect->setGeometry(getPlot()->canvas()->x()+9, getPlot()->canvas()->y()+9, getPlot()->canvas()->width()/2, getPlot()->canvas()->height());
        mpParentPlotWindow->showHelpPopupMessage("Add selected variable to left Y-axis.");
    }
    else
    {
        mpHoverRect->setGeometry(getPlot()->canvas()->x()+9 + getPlot()->canvas()->width()/2, getPlot()->canvas()->y()+9, getPlot()->canvas()->width()/2, getPlot()->canvas()->height());
        mpParentPlotWindow->showHelpPopupMessage("Add selected variable to right Y-axis.");
    }
    QWidget::dragMoveEvent(event);
}


//! @brief Defines what happens when user drags something out from the plot window.
void PlotTab::dragLeaveEvent(QDragLeaveEvent *event)
{
    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PORTVARIABLE) return;

    delete(mpHoverRect);
    QWidget::dragLeaveEvent(event);
}


//! @brief Defines what happens when user drops something in the plot window
void PlotTab::dropEvent(QDropEvent *event)
{
    QWidget::dropEvent(event);

    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PORTVARIABLE) return;

    if (event->mimeData()->hasText())
    {
        delete(mpHoverRect);

        QString mimeText = event->mimeData()->text();
        QTextStream mimeStream;
        mimeStream.setString(&mimeText);

        QString discardedText;
        QString componentName;
        QString portName;
        QString dataName;

        if(mimeText.startsWith("HOPSANPLOTDATA"))
        {
            qDebug() << mimeText;

            discardedText = readName(mimeStream);
            componentName = readName(mimeStream);
            portName = readName(mimeStream);
            dataName = readName(mimeStream);

            QCursor cursor;
            if(this->mapFromGlobal(cursor.pos()).y() > getPlot()->canvas()->height()/2+getPlot()->canvas()->y()+10 && getNumberOfCurves(FIRSTPLOT) >= 1)
            {
                changeXVector(gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotData(gpMainWindow->mpProjectTabs->getCurrentContainer()->getNumberOfPlotGenerations()-1, componentName, portName, dataName), componentName, portName, dataName, gConfig.getDefaultUnit(dataName));
            }
            else if(this->mapFromGlobal(cursor.pos()).x() < getPlot()->canvas()->x()+9 + getPlot()->canvas()->width()/2)
            {
                mpParentPlotWindow->addPlotCurve(gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData().size()-1, componentName, portName, dataName, "", QwtPlot::yLeft);
            }
            else
            {
                mpParentPlotWindow->addPlotCurve(gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData().size()-1, componentName, portName, dataName, "", QwtPlot::yRight);
            }
        }
    }
}


//! @brief Handles the right-click menu in the plot tab
void PlotTab::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget::contextMenuEvent(event);

    if(this->mpZoomer[FIRSTPLOT]->isEnabled())
    {
        return;
    }

    QMenu menu;

    QMenu *yAxisRightMenu;
    QMenu *yAxisLeftMenu;
    QMenu *changeUnitsMenu;
    QMenu *insertMarkerMenu;

    QAction *setRightAxisLogarithmic = 0;
    QAction *setLeftAxisLogarithmic = 0;


    yAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    yAxisRightMenu = menu.addMenu(QString("Right Y Axis"));

    yAxisLeftMenu->setEnabled(mpPlot[FIRSTPLOT]->axisEnabled(QwtPlot::yLeft));
    yAxisRightMenu->setEnabled(mpPlot[FIRSTPLOT]->axisEnabled(QwtPlot::yRight));

        //Create menu and actions for changing units
    changeUnitsMenu = menu.addMenu(QString("Change Units"));
    QMap<QAction *, PlotCurve *> actionToCurveMap;
    QMap<QString, double> unitMap;
    QList<PlotCurve *>::iterator itc;
    QMap<QString, double>::iterator itu;
    for(itc=mPlotCurvePtrs[FIRSTPLOT].begin(); itc!=mPlotCurvePtrs[FIRSTPLOT].end(); ++itc)
    {
        QMenu *pTempMenu = changeUnitsMenu->addMenu(QString((*itc)->getComponentName() + ", " + (*itc)->getPortName() + ", " + (*itc)->getDataName()));
        unitMap = gConfig.getCustomUnits((*itc)->getDataName());
        for(itu=unitMap.begin(); itu!=unitMap.end(); ++itu)
        {
            QAction *pTempAction = pTempMenu->addAction(itu.key());
            actionToCurveMap.insert(pTempAction, (*itc));
        }
    }


        //Create actions for making axis logarithmic
    if(mpPlot[FIRSTPLOT]->axisEnabled(QwtPlot::yLeft))
    {
        setLeftAxisLogarithmic = yAxisLeftMenu->addAction("Logarithmic Scale");
        setLeftAxisLogarithmic->setCheckable(true);
        setLeftAxisLogarithmic->setChecked(mLeftAxisLogarithmic);
    }
    if(mpPlot[FIRSTPLOT]->axisEnabled(QwtPlot::yRight))
    {
        setRightAxisLogarithmic = yAxisRightMenu->addAction("Logarithmic Scale");
        setRightAxisLogarithmic->setCheckable(true);
        setRightAxisLogarithmic->setChecked(mRightAxisLogarithmic);
    }


        //Create menu for insereting curve markers
    insertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    for(int plotID=0; plotID<2; ++plotID)
    {
        for(itc=mPlotCurvePtrs[plotID].begin(); itc!=mPlotCurvePtrs[plotID].end(); ++itc)
        {
            QAction *pTempAction = insertMarkerMenu->addAction((*itc)->getCurveName());
            actionToCurveMap.insert(pTempAction, (*itc));
        }
    }



    // ----- Wait for user to make a selection ----- //

    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    // ----- User has selected something -----  //



        // Check if user did not click on a menu item
    if(selectedAction == 0)
    {
        return;
    }


        // Change unit on selected curve
    if(selectedAction->parentWidget()->parentWidget() == changeUnitsMenu)
    {
        actionToCurveMap.find(selectedAction).value()->setDataUnit(selectedAction->text());
    }


        //Make axis logarithmic
    if (selectedAction == setRightAxisLogarithmic)
    {
        mRightAxisLogarithmic = !mRightAxisLogarithmic;
        if(mRightAxisLogarithmic)
        {
            mpPlot[FIRSTPLOT]->setAxisScaleEngine(QwtPlot::yRight, new QwtLog10ScaleEngine);
            rescaleToCurves();
            mpPlot[FIRSTPLOT]->replot();
        }
        else
        {
            mpPlot[FIRSTPLOT]->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
            rescaleToCurves();
            mpPlot[FIRSTPLOT]->replot();
        }
    }
    else if (selectedAction == setLeftAxisLogarithmic)
    {
        mLeftAxisLogarithmic = !mLeftAxisLogarithmic;
        if(mLeftAxisLogarithmic)
        {
            qDebug() << "Logarithmic!";
            mpPlot[FIRSTPLOT]->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
            rescaleToCurves();
            mpPlot[FIRSTPLOT]->replot();
        }
        else
        {
            qDebug() << "Linear!";
            mpPlot[FIRSTPLOT]->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
            rescaleToCurves();
            mpPlot[FIRSTPLOT]->replot();
        }
    }


        //Insert curve marker
    if(selectedAction->parentWidget() == insertMarkerMenu)
    {
        insertMarker(actionToCurveMap.find(selectedAction).value(), event->pos());
    }

}


class PlotInfoBox;


//! @brief Constructor for plot curves.
//! @param generation Generation of plot data to use
//! @param componentName Name of component where plot data is located
//! @param portName Name of port where plot data is located
//! @param dataName Name of physical quantity to use (e.g. "Pressure", "Velocity"...)
//! @param dataUnit Name of unit to show data in
//! @param axisY Which Y-axis to use (QwtPlot::yLeft or QwtPlot::yRight)
//! @param parent Pointer to plot tab which curve shall be created it
PlotCurve::PlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY, QString modelPath, PlotTab *parent, HopsanPlotID plotID, HopsanPlotCurveType curveType)
{
    mCurveType = curveType;

        //Set all member variables
    mpParentPlotTab = parent;
    if(modelPath.isEmpty())
    {
        mpContainerObject = gpMainWindow->mpProjectTabs->getCurrentContainer();
    }
    else
    {
        for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
        {
            if(gpMainWindow->mpProjectTabs->getTab(i)->getTopLevelSystem()->getModelFileInfo().filePath() == modelPath)
            {
                mpContainerObject = gpMainWindow->mpProjectTabs->getContainer(i);
                break;
            }
        }
    }
    assert(!mpContainerObject == 0);        //Container not found, should never happen! Caller to the function has supplied a model name that does not exist.

    mpContainerObject->incrementOpenPlotCurves();
    mGeneration = generation;
    mComponentName = componentName;
    mPortName = portName;
    mDataName = dataName;
    if(dataUnit.isEmpty())
    {
        mDataUnit = gConfig.getDefaultUnit(dataName);   //Apply default unit if not specified
    }
    else
    {
        mDataUnit = dataUnit;
    }
    mAxisY = axisY;
    mAutoUpdate = true;
    mScaleX = 1.0;
    mScaleY = 1.0;
    mOffsetX = 0.0;
    mOffsetY = 0.0;

        //Get data from container object
    mDataVector = mpContainerObject->getPlotData(generation, componentName, portName, dataName);
    mTimeVector = mpContainerObject->getTimeVector(generation, componentName, portName);

        //Create the actual curve
    mpCurve = new QwtPlotCurve(QString(mComponentName+", "+mPortName+", "+mDataName));
    updateCurve();
    mpCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpCurve->setYAxis(axisY);
    mpCurve->attach(parent->getPlot(plotID));

        //Create the plot info box
    mpPlotInfoBox = new PlotInfoBox(this, mpParentPlotTab);
    updatePlotInfoBox();
    mpPlotInfoBox->mpSizeSpinBox->setValue(2);

    mpParentPlotTab->mpParentPlotWindow->mpPlotInfoLayout->addWidget(mpPlotInfoBox);

    if(curveType != PORTVARIABLE)
    {
        setAutoUpdate(false);
        mpPlotInfoBox->mpAutoUpdateCheckBox->setDisabled(true);
        mpPlotInfoBox->mpNextButton->setDisabled(true);
        mpPlotInfoBox->mpPreviousButton->setDisabled(true);
        mpPlotInfoBox->mpFrequencyAnalysisButton->setDisabled(true);
    }

    mpCurve->setItemAttribute(QwtPlotItem::Legend, mpParentPlotTab->mpParentPlotWindow->mLegendsVisible);

        //Create connections
    connect(mpPlotInfoBox->mpSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setLineWidth(int)));
    connect(mpPlotInfoBox->mpColorButton, SIGNAL(clicked()), this, SLOT(setLineColor()));
    connect(mpPlotInfoBox->mpScaleButton, SIGNAL(clicked()), this, SLOT(openScaleDialog()));
    connect(mpParentPlotTab->mpParentPlotWindow->getPlotTabWidget(), SIGNAL(currentChanged(int)), this, SLOT(updatePlotInfoVisibility()));
    connect(mpParentPlotTab->mpParentPlotWindow->mpShowCurveInfoButton, SIGNAL(toggled(bool)), SLOT(updatePlotInfoVisibility()));
    connect(mpPlotInfoBox->mpCloseButton, SIGNAL(clicked()), this, SLOT(removeMe()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));
    connect(gpMainWindow->mpProjectTabs,SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));
    connect(mpContainerObject, SIGNAL(objectDeleted()), this, SLOT(removeMe()));
    connect(mpContainerObject, SIGNAL(objectDeleted()), mpParentPlotTab->mpParentPlotWindow, SLOT(closeIfEmpty()), Qt::UniqueConnection);
    connect(mpContainerObject->getModelObject(mComponentName), SIGNAL(objectDeleted()), this, SLOT(removeMe()));
    connect(mpContainerObject->getModelObject(mComponentName), SIGNAL(nameChanged()), this, SLOT(removeMe()));
    connect(mpContainerObject, SIGNAL(connectorRemoved()), this, SLOT(removeIfNotConnected()));
}


//! @brief Destructor for plot curves
//! Deletes the info box and its dock widgets before the curve is removed.
PlotCurve::~PlotCurve()
{
    mpContainerObject->decrementOpenPlotCurves();
    delete(mpPlotInfoBox);
    //delete(mpPlotInfoDockWidget);
}


//! @brief Returns the current generation a plot curve is representing
int PlotCurve::getGeneration()
{
    return mGeneration;
}

QString PlotCurve::getCurveName()
{
    if(mCurveType == PORTVARIABLE)
        return QString(mComponentName + ", " + mPortName + ", " + mDataName);
    else if(mCurveType == FREQUENCYANALYSIS)
        return "Frequency Spectrum";
    else if(mCurveType == NYQUIST)
        return "Nyquist Plot";
    else if(mCurveType == BODEGAIN)
        return "Magnitude Plot";
    else if(mCurveType == BODEPHASE)
        return "Phase Plot";
    else
        return "Unnamed Curve";
}


//! @brief Returns the type of the curve
HopsanPlotCurveType PlotCurve::getCurveType()
{
    return mCurveType;
}


//! @brief Returns a pointer to the actual Qwt curve in a plot curve object
QwtPlotCurve *PlotCurve::getCurvePtr()
{
    return mpCurve;
}


////! @brief Returns a pointer to the plot info dock of a plot curve
//QDockWidget *PlotCurve::getPlotInfoDockWidget()
//{
//    return mpPlotInfoDockWidget;
//}


//! @brief Returns the name of the component a plot curve is created from
QString PlotCurve::getComponentName()
{
    return mComponentName;
}


//! @brief Returns the name of the port a plot curve is created from
QString PlotCurve::getPortName()
{
    return mPortName;
}


//! @brief Returns the data name (physical quantity) of a plot curve
QString PlotCurve::getDataName()
{
    return mDataName;
}


//! @brief Returns the current data unit of a plot curve
QString PlotCurve::getDataUnit()
{
    return mDataUnit;
}


//! @brief Tells which Y-axis a plot curve is assigned to
int PlotCurve::getAxisY()
{
    return mAxisY;
}


//! @brief Returns the (unscaled) data vector of a plot curve
QVector<double> PlotCurve::getDataVector()
{
    return mDataVector;
}


//! @brief Returns the (unscaled) time vector of a plot curve
//! This returns the TIME vector, NOT any special X-axes if they are used.
QVector<double> PlotCurve::getTimeVector()
{
    return mTimeVector;
}


//! @brief Returns a pointer to the container object a curve origins from
ContainerObject *PlotCurve::getContainerObjectPtr()
{
    return mpContainerObject;
}


//! @brief Sets the generation of a plot curve
//! Updates the data to specified generation, and updates plot info box.
//! @param genereation Genereation to use
void PlotCurve::setGeneration(int generation)
{
    mGeneration = generation;
    mDataVector = mpContainerObject->getPlotData(mGeneration, mComponentName, mPortName, mDataName);
    if(mpParentPlotTab->mVectorX.size() == 0)
        mTimeVector = mpContainerObject->getTimeVector(mGeneration, mComponentName, mPortName);
    else
    {
        mpParentPlotTab->mVectorX = mpContainerObject->getPlotData(mGeneration, mpParentPlotTab->mVectorXComponent,
                                                                   mpParentPlotTab->mVectorXPortName, mpParentPlotTab->mVectorXDataName);
        mTimeVector = mpParentPlotTab->mVectorX;
    }

    updateCurve();
    mpParentPlotTab->update();
    updatePlotInfoBox();
}


//! @brief Sets the unit of a plot curve
//! @param unit Name of new unit
void PlotCurve::setDataUnit(QString unit)
{
    mDataUnit = unit;
    updateCurve();
    mpParentPlotTab->updateLabels();
    mpParentPlotTab->rescaleToCurves();
    mpParentPlotTab->update();
}


//! @brief Sets the scaling of a plot curve
//! @param scaleX Scale factor for X-axis
//! @param scaleY Scale factor for Y-axis
//! @param offsetX Offset value for X-axis
//! @param offsetY Offset value for Y-axis
void PlotCurve::setScaling(double scaleX, double scaleY, double offsetX, double offsetY)
{
    mScaleX=scaleX;
    mScaleY=scaleY;
    mOffsetX=offsetX;
    mOffsetY=offsetY;
    updateCurve();
}


void PlotCurve::setData(QVector<double> vData, QVector<double> vTime)
{
    mDataVector = vData;
    mTimeVector = vTime;
    updateCurve();
}


//! @brief Converts the plot curve to its frequency spectrum by using FFT
void PlotCurve::toFrequencySpectrum()
{
    //Vector size has to be an even potential of 2.
    //Calculate largets potential that is smaller than or equal to the vector size.
    int n = pow(2, int(log2(mDataVector.size())));
    if(n != mDataVector.size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(mDataVector.size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindow, gpMainWindow->tr("Wrong Vector Size"),
                                 "Size of data vector must be an even power of 2. Number of log samples was reduced from " + oldString + " to " + newString + ".");
        reduceVectorSize(mDataVector, n);
        reduceVectorSize(mTimeVector, n);
    }

    //Create a complex vector
    QVector< std::complex<double> > vComplex = realToComplex(mDataVector);

    //Apply the fourier transform
    FFT(vComplex);

    //Scalar multiply complex vector with its conjugate, and divide it with its size
    mDataVector.clear();
    for(int i=1; i<n/2; ++i)        //FFT is symmetric, so only use first half
    {
        if(mpParentPlotTab->mpParentPlotWindow->mpPowerSpectrumCheckBox->isChecked())
        {
            mDataVector.append(real(vComplex[i]*conj(vComplex[i]))/n);
        }
        else
        {
            mDataVector.append(sqrt(vComplex[i].real()*vComplex[i].real() + vComplex[i].imag()*vComplex[i].imag()));
        }
    }

    //Create the x vector (frequency)
    double max = mTimeVector.last();
    mTimeVector.clear();
    for(int i=1; i<n/2; ++i)
    {
        mTimeVector.append(double(i)/max);
    }

    mDataName = "Value";
    mDataUnit = "-";

    updateCurve();
    mpParentPlotTab->changeXVector(mTimeVector, "", "", "Frequency", "Hz");
    mpParentPlotTab->update();
    updatePlotInfoBox();
}


//! @brief Changes a curve to the previous available gneraetion of its data
void PlotCurve::setPreviousGeneration()
{
    //if(mGeneration>0)       //This check should not really be necessary since button is disabled anyway, but just to be sure...
    if(mGeneration>0 && mpContainerObject->componentHasPlotGeneration(mGeneration-1, mComponentName))
        setGeneration(mGeneration-1);
}


//! @brief Changes a curve to the next available generation of its data
void PlotCurve::setNextGeneration()
{
    if(mGeneration<mpContainerObject->getNumberOfPlotGenerations()-1)       //This check should not really be necessary since button is disabled anyway, but just to be sure...
        setGeneration(mGeneration+1);
}


//! @brief Sets the line width of a plot curve
//! @param lineWidth Line width to give curve
void PlotCurve::setLineWidth(int lineWidth)
{
    mLineWidth = lineWidth;
    QPen tempPen = mpCurve->pen();
    tempPen.setWidth(lineWidth);
    mpCurve->setPen(tempPen);
}


//! @brief Sets the color of a line
//! @brief color Color to give the line.
void PlotCurve::setLineColor(QColor color)
{
    mLineColor = color;
    QPen tempPen = mpCurve->pen();
    tempPen.setColor(color);
    mpCurve->setPen(tempPen);

    //Update color blob in plot info box
    QString redString, greenString, blueString;
    redString.setNum(color.red());
    greenString.setNum(color.green());
    blueString.setNum(color.blue());
    QString buttonStyle;
    buttonStyle.append("QToolButton			{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:pressed   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:checked		{ border: 1px solid gray;               border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:checked   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:unchecked   	{ border: 1px solid gray;               border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    mpPlotInfoBox->mpColorBlob->setStyleSheet(buttonStyle);
}


//! @brief Sets the color of a line
//! @param colorName Svg name of the color
//! @see setLineColor(QColor color)
void PlotCurve::setLineColor(QString colorName)
{
    QColor color;
    if(colorName.isEmpty())
    {
        color = QColorDialog::getColor(mpCurve->pen().color(), gpMainWindow);
        if (!color.isValid()) { return; }
    }
    else
    {
        color = QColor(colorName);
    }
    setLineColor(color);
}


//! @brief Opens the scaling dialog for a plot curve
void PlotCurve::openScaleDialog()
{
    QDialog *pScaleDialog = new QDialog(mpParentPlotTab->mpParentPlotWindow);
    pScaleDialog->setWindowTitle("Change Curve Scale");

    QLabel *pXScaleLabel = new QLabel("Time Axis Scale: ", pScaleDialog);
    mpXScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXScaleSpinBox->setRange(-DBLMAX, DBLMAX);
    mpXScaleSpinBox->setDecimals(10);
    mpXScaleSpinBox->setSingleStep(0.1);
    mpXScaleSpinBox->setValue(mScaleX);

    QLabel *pXOffsetLabel = new QLabel("Time Axis Offset: ", pScaleDialog);
    mpXOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXOffsetSpinBox->setDecimals(10);
    mpXOffsetSpinBox->setRange(-DBLMAX, DBLMAX);
    mpXOffsetSpinBox->setSingleStep(0.1);
    mpXOffsetSpinBox->setValue(mOffsetX);

    QLabel *pYScaleLabel = new QLabel("Y-Axis Scale: ", pScaleDialog);
    mpYScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYScaleSpinBox->setSingleStep(0.1);
    mpYScaleSpinBox->setDecimals(10);
    mpYScaleSpinBox->setRange(-DBLMAX, DBLMAX);
    mpYScaleSpinBox->setValue(mScaleY);

    QLabel *pYOffsetLabel = new QLabel("Y-Axis Offset: ", pScaleDialog);
    mpYOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYOffsetSpinBox->setDecimals(10);
    mpYOffsetSpinBox->setRange(-DBLMAX, DBLMAX);
    mpYOffsetSpinBox->setSingleStep(0.1);
    mpYOffsetSpinBox->setValue(mOffsetY);

    QPushButton *pDoneButton = new QPushButton("Done", pScaleDialog);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(pScaleDialog);
    pDialogLayout->addWidget(pXScaleLabel,0,0);
    pDialogLayout->addWidget(mpXScaleSpinBox,0,1);
    pDialogLayout->addWidget(pXOffsetLabel,1,0);
    pDialogLayout->addWidget(mpXOffsetSpinBox,1,1);
    pDialogLayout->addWidget(pYScaleLabel,2,0);
    pDialogLayout->addWidget(mpYScaleSpinBox,2,1);
    pDialogLayout->addWidget(pYOffsetLabel,3,0);
    pDialogLayout->addWidget(mpYOffsetSpinBox,3,1);
    pDialogLayout->addWidget(pButtonBox,4,0,1,2);
    pScaleDialog->setLayout(pDialogLayout);
    pScaleDialog->show();

    connect(pDoneButton,SIGNAL(clicked()),pScaleDialog,SLOT(close()));
    connect(mpXScaleSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
    connect(mpXOffsetSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
    connect(mpYScaleSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
    connect(mpYOffsetSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
}


//! @brief Updates the scaling of a plot curve form values in scaling dialog
void PlotCurve::updateScaleFromDialog()
{
    setScaling(mpXScaleSpinBox->value(), mpYScaleSpinBox->value(), mpXOffsetSpinBox->value(), mpYOffsetSpinBox->value());
    mpParentPlotTab->rescaleToCurves();
}


//! @brief Shows or hides plot info dock
//! Changes visibility depending on whether or not the tab is currently open, and whether or not the hide plot info dock setting is activated.
void PlotCurve::updatePlotInfoVisibility()
{
    if(mpParentPlotTab == mpParentPlotTab->mpParentPlotWindow->getCurrentPlotTab() && mpParentPlotTab->mpParentPlotWindow->mpShowCurveInfoButton->isChecked())
    {
        //mpParentPlotTab->mpParentPlotWindow->addDockWidget(Qt::BottomDockWidgetArea, mpPlotInfoDockWidget, Qt::Vertical);
        mpParentPlotTab->mpParentPlotWindow->mpPlotInfoLayout->addWidget(mpPlotInfoBox);
        mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->show();
    }
    else
    {
        mpParentPlotTab->mpParentPlotWindow->mpPlotInfoLayout->removeWidget(mpPlotInfoBox);
        mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        if(mpParentPlotTab->mpParentPlotWindow->mpPlotInfoLayout->isEmpty())
        {
            mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->hide();
        }
    }
}


//! @brief Tells the parent plot tab of a curve to remove it
void PlotCurve::removeMe()
{
    mpParentPlotTab->removeCurve(this);
}


//! @brief Slot that checks that the plotted port is still connected, and removes the curve if not
void PlotCurve::removeIfNotConnected()
{
    if(!mpContainerObject->getModelObject(mComponentName)->getPort(mPortName)->isConnected())
    {
        removeMe();
    }
}

//! @brief Updates a plot curve to the most recent available generation of its data
void PlotCurve::updateToNewGeneration()
{
    if(mAutoUpdate)     //Only change the generation if auto update is on
        setGeneration(mpContainerObject->getNumberOfPlotGenerations()-1);
    updatePlotInfoBox();    //Update the plot info box regardless of auto update setting, to show number of available generations correctly
    mpParentPlotTab->rescaleToCurves();
}


//! @brief Updates buttons and text in plot info box to correct values
void PlotCurve::updatePlotInfoBox()
{
    mpPlotInfoBox->mpPreviousButton->setEnabled(mGeneration > 0 && mpContainerObject->getNumberOfPlotGenerations() > 1 && mAutoUpdate);
    mpPlotInfoBox->mpNextButton->setEnabled(mGeneration < mpContainerObject->getNumberOfPlotGenerations()-1 && mpContainerObject->getNumberOfPlotGenerations() > 1 && mAutoUpdate);

    QString numString1, numString2;
    numString1.setNum(mGeneration+1);
    numString2.setNum(mpContainerObject->getNumberOfPlotGenerations());
    mpPlotInfoBox->mpGenerationLabel->setText(numString1 + "(" + numString2 + ")");
}


//! @brief Activates (highlights) the plot curve
//! This will also de-activate any other active plot curve.
void PlotCurve::setActive(bool value)
{
    if(value)
    {
        setLineWidth(mpPlotInfoBox->mpSizeSpinBox->value()+1);
        mpPlotInfoBox->setPalette(QPalette(QColor("lightgray"), QColor("lightgray")));
        mpPlotInfoBox->setAutoFillBackground(true);

        for(int i=0; i<mpParentPlotTab->getCurves().size(); ++i)
        {
            if(mpParentPlotTab->getCurves().at(i) != this)
            {
                mpParentPlotTab->getCurves().at(i)->setActive(false);
            }
        }
        mpParentPlotTab->setActivePlotCurve(this);
    }
    else
    {
        setLineWidth(mpPlotInfoBox->mpSizeSpinBox->value());
        mpPlotInfoBox->setAutoFillBackground(false);
        mpPlotInfoBox->mpColorBlob->setChecked(false);
    }
}


//! @brief Updates the values of a curve
//! Updates a curve with regard to special X-axis, units and scaling.
void PlotCurve::updateCurve()
{
    double unitScale = gConfig.getCustomUnits(mDataName).find(mDataUnit).value();
    QVector<double> tempX;
    QVector<double> tempY;
    if(mpParentPlotTab->mHasSpecialXAxis)
    {
        for(int i=0; i<mpParentPlotTab->mVectorX.size() && i<mDataVector.size(); ++i)
        {
            tempX.append(mpParentPlotTab->mVectorX[i]*mScaleX + mOffsetX);
            tempY.append(mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    else
    {
        for(int i=0; i<mTimeVector.size() && i<mDataVector.size(); ++i)
        {
            tempX.append(mTimeVector[i]*mScaleX + mOffsetX);
            tempY.append(mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    mpCurve->setSamples(tempX, tempY);
}


//! @brief Sets auto update flag for a plot curve
//! If this is activated, plot will automatically change to latest plot generation after next simulation.
void PlotCurve::setAutoUpdate(bool value)
{
    mAutoUpdate = value;
}


void PlotCurve::performFrequencyAnalysis()
{
    mpParentPlotTab->mpParentPlotWindow->performFrequencyAnalysis(this);
}


//! @brief Constructor for plot markers
//! @param pCurve Pointer to curve the marker belongs to
//! @param pPlotTab Plot tab the marker is located in
//! @param markerSymbol The symbol the marker shall use
PlotMarker::PlotMarker(PlotCurve *pCurve, PlotTab *pPlotTab, QwtSymbol markerSymbol)
    : QwtPlotMarker()
{
    mpCurve = pCurve;
    mpPlotTab = pPlotTab;
    mIsBeingMoved = false;
    mMarkerSymbol = markerSymbol;
    setSymbol(&mMarkerSymbol);
    mIsMovable = true;
}


//! @brief Event filter for plot markers
//! This will interrupt events from plot canvas, to enable using mouse and key events for modifying markers.
//! @returns True if event was interrupted, false if its propagation shall continue
//! @param object Pointer to the object the event belongs to (in this case the plot canvas)
//! @param ev ent Event to be interrupted
bool PlotMarker::eventFilter(QObject */*object*/, QEvent *event)
{
    if(!mIsMovable)
        return false;

        // Mouse press events, used to initiate moving of a marker if mouse cursor is close enough
    if (event->type() == QEvent::MouseButtonPress)
    {
        QCursor cursor;
        QPointF midPoint;
        midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(this->plot()->transform(QwtPlot::yLeft, value().y()));

        if(!mpPlotTab->mpZoomer[FIRSTPLOT]->isEnabled() && !mpPlotTab->mpPanner[FIRSTPLOT]->isEnabled())
        {
            if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
            {
                mIsBeingMoved = true;
                return true;
            }
        }
    }

        // Mouse move (hover) events, used to change marker color or move marker if cursor is close enough.
    else if (event->type() == QEvent::MouseMove)
    {
        bool retval = false;
        QCursor cursor;
        QPointF midPoint;
        midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(this->plot()->transform(QwtPlot::yLeft, value().y()));
        if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
        {
            mMarkerSymbol.setPen(QPen(mpCurve->getCurvePtr()->pen().brush().color().lighter(165), 3));
            this->setSymbol(&mMarkerSymbol);
            this->plot()->replot();
            retval=true;
        }
        else
        {
            if(!mIsBeingMoved)
            {
                mMarkerSymbol.setPen(QPen(mpCurve->getCurvePtr()->pen().brush().color(), 3));
                this->setSymbol(&mMarkerSymbol);
                this->plot()->replot();
            }
        }

        if(mIsBeingMoved)
        {
            double x = mpCurve->getCurvePtr()->sample(mpCurve->getCurvePtr()->closestPoint(this->plot()->canvas()->mapFromGlobal(cursor.pos()))).x();
            double y = mpCurve->getCurvePtr()->sample(mpCurve->getCurvePtr()->closestPoint(this->plot()->canvas()->mapFromGlobal(cursor.pos()))).y();
            setXValue(x);
            setYValue(this->plot()->invTransform(QwtPlot::yLeft, this->plot()->transform(mpCurve->getCurvePtr()->yAxis(), y)));

            QString xString;
            QString yString;
            xString.setNum(x);
            yString.setNum(y);
            QwtText tempLabel;
            tempLabel.setText("("+xString+", "+yString+")");
            tempLabel.setColor(mpCurve->getCurvePtr()->pen().brush().color());
            tempLabel.setBackgroundBrush(QColor(255,255,255,220));
            tempLabel.setFont(QFont("Calibri", 12, QFont::Normal));
            setLabel(tempLabel);
        }
        return retval;
    }

        //Mouse release event, will stop moving marker
    else if (event->type() == QEvent::MouseButtonRelease && mIsBeingMoved == true)
    {
        mIsBeingMoved = false;
        return false;
    }

        //!Keypress event, will delete marker if delete key is pressed
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Delete)
        {
            QCursor cursor;
            QPointF midPoint;
            midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
            midPoint.setY(this->plot()->transform(mpCurve->getCurvePtr()->yAxis(), value().y()));
            if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
            {
                plot()->canvas()->removeEventFilter(this);
                for(int plotID=0; plotID<2; ++plotID)
                {
                    mpPlotTab->mMarkerPtrs[plotID].removeAll(this);     //Cycle all plots and remove the marker if it is found
                }
                this->hide();           // This will only hide and inactivate the marker. Deleting it seem to make program crash.
                this->detach();
                return true;
            }
        }
        return false;
    }
    return false;
}


void PlotMarker::setMovable(bool movable)
{
    mIsMovable = movable;
}


//! @brief Returns a pointer to the curve a plot marker belongs to
PlotCurve *PlotMarker::getCurve()
{
    return mpCurve;
}
