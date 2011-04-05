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

#include <cstring>

#include "Widgets/PlotWidget.h"
#include "PlotWindow.h"

#include "MainWindow.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUISystem.h"
#include "Configuration.h"
#include "loadObjects.h"
#include "Dialogs/OptionsDialog.h"

#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include "qwt_double_rect.h"


//! @brief Constructor for the plot window, where plots are displayed.
//! @param plotParameterTree is a pointer to the parameter tree from where the plot window was created
//! @param parent is a pointer to the main window
PlotWindow::PlotWindow(PlotParameterTree *plotParameterTree, MainWindow *parent)
    : QMainWindow(parent)

{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Hopsan Plot Window");
    setAcceptDrops(false);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setPalette(gConfig.getPalette());

    resize(1000,800);    //! @todo Maybe user should be allowed to change default plot window size, or someone will become annoyed...

    mpCurrentGUISystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    mpPlotParameterTree = plotParameterTree;

        //Create the toolbar and its buttons
    mpToolBar = new QToolBar(this);

    mpNewPlotButton = new QToolButton(mpToolBar);
    mpNewPlotButton->setToolTip("Create New Plot");
    mpNewPlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
    mpNewPlotButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpZoomButton = new QToolButton(mpToolBar);
    mpZoomButton->setToolTip("Zoom (Z)");
    mpZoomButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Zoom.png"));
    mpZoomButton->setCheckable(true);
    mpZoomButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpZoomButton->setShortcut(QKeySequence("z"));

    mpPanButton = new QToolButton(mpToolBar);
    mpPanButton->setToolTip("Pan (X)");
    mpPanButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Pan.png"));
    mpPanButton->setCheckable(true);
    mpPanButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpPanButton->setShortcut(QKeySequence("x"));

    mpSaveButton = new QToolButton(mpToolBar);
    mpSaveButton->setToolTip("Save Plot Window");
    mpSaveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Save.png"));
    mpSaveButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpSVGButton = new QToolButton(mpToolBar);
    mpSVGButton->setToolTip("Export to SVG");
    mpSVGButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SaveToSvg.png"));
    mpSVGButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpExportGNUPLOTButton = new QToolButton(mpToolBar);
    mpExportGNUPLOTButton->setToolTip("Export to GNUPLOT");
    mpExportGNUPLOTButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SaveToGnuPlot.png"));
    mpExportGNUPLOTButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpImportGNUPLOTButton = new QToolButton(mpToolBar);
    mpImportGNUPLOTButton->setToolTip("Import from GNUPLOT");
    mpImportGNUPLOTButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LoadGnuPlot.png"));
    mpImportGNUPLOTButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpGridButton = new QToolButton(mpToolBar);
    mpGridButton->setToolTip("Show Grid (G)");
    mpGridButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Grid.png"));
    mpGridButton->setCheckable(true);
    mpGridButton->setChecked(true);
    mpGridButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpGridButton->setShortcut(QKeySequence("g"));

    mpBackgroundColorButton = new QToolButton(mpToolBar);
    mpBackgroundColorButton->setToolTip("Select Canvas Color (C)");
    mpBackgroundColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-BackgroundColor.png"));
    mpBackgroundColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpBackgroundColorButton->setShortcut(QKeySequence("c"));

    mpShowListsButton = new QToolButton(mpToolBar);
    mpShowListsButton->setCheckable(true);
    mpShowListsButton->setChecked(true);
    mpShowListsButton->setToolTip("Toggle Parameter Lists");
    mpShowListsButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowLists.png"));
    mpShowListsButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpShowCurvesButton = new QToolButton(mpToolBar);
    mpShowCurvesButton->setCheckable(true);
    mpShowCurvesButton->setChecked(true);
    mpShowCurvesButton->setToolTip("Toggle Curve Controls");
    mpShowCurvesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowCurves.png"));
    mpShowCurvesButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpNewWindowFromTabButton = new QToolButton(mpToolBar);
    mpNewWindowFromTabButton->setToolTip("Create Plot Window From Tab");
    mpNewWindowFromTabButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-OpenTabInNewPlotWindow.png"));
    mpNewWindowFromTabButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    mpResetXVectorButton = new QToolButton(mpToolBar);
    mpResetXVectorButton->setToolTip("Reset Time Vector");
    mpResetXVectorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    mpResetXVectorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpResetXVectorButton->setEnabled(false);

    mpToolBar->addWidget(mpNewPlotButton);
    mpToolBar->addWidget(mpZoomButton);
    mpToolBar->addWidget(mpPanButton);
    mpToolBar->addWidget(mpSaveButton);
    mpToolBar->addWidget(mpSVGButton);
    mpToolBar->addWidget(mpExportGNUPLOTButton);
    mpToolBar->addWidget(mpImportGNUPLOTButton);
    mpToolBar->addSeparator();
    mpToolBar->addWidget(mpGridButton);
    mpToolBar->addWidget(mpBackgroundColorButton);
    mpToolBar->addWidget(mpShowListsButton);
    mpToolBar->addWidget(mpShowCurvesButton);
    mpToolBar->addWidget(mpNewWindowFromTabButton);
    mpToolBar->addWidget(mpResetXVectorButton);

    addToolBar(mpToolBar);

    mpPlotTabs = new PlotTabWidget(this);
    this->addPlotTab();
    mpComponentsLabel = new QLabel(tr("Components"));
    QFont boldFont = mpComponentsLabel->font();
    boldFont.setBold(true);
    mpComponentsLabel->setFont(boldFont);
    mpComponentsLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpPortsLabel = new QLabel(tr("Ports"));
    mpPortsLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpPortsLabel->setFont(boldFont);
    mpVariablesLabel = new QLabel(tr("Variables"));
    mpVariablesLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mpVariablesLabel->setFont(boldFont);

    mpComponentList = new QListWidget(this);
    mpComponentList->setMaximumHeight(100);
    mpComponentList->setPalette(QPalette(QColor("black"), QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("dimgray"), QColor("white")));

    mpPortList = new QListWidget(this);
    mpPortList->setMaximumHeight(100);
    mpPortList->setPalette(QPalette(QColor("black"), QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("dimgray"), QColor("white")));

    mpVariableList = new VariableListWidget(this);
    mpVariableList->setMaximumHeight(100);

    this->setDockOptions(QMainWindow::AllowNestedDocks);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotTabs,0,0,1,4);
    mpLayout->addWidget(mpComponentsLabel,1,0);
    mpLayout->addWidget(mpPortsLabel,1,1);
    mpLayout->addWidget(mpVariablesLabel,1,2);
    mpLayout->addWidget(mpComponentList,2,0);
    mpLayout->addWidget(mpPortList,2,1);
    mpLayout->addWidget(mpVariableList,2,2);

    mpLayout->setRowStretch(0,1);
    mpLayout->setRowStretch(1,0);
    mpLayout->setRowStretch(2,0);

    QWidget *pCentralWidget = new QWidget(this);
    pCentralWidget->setLayout(mpLayout);
    this->setCentralWidget(pCentralWidget);

    // Populate boxes
    QList< QMap< QString, QMap< QString, QMap<QString, QVector<double> > > > > plotData = gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData();
    mpComponentList->addItems(plotData.last().keys());
    mpComponentList->setCurrentItem(mpComponentList->item(0));
    updatePortList();

        //Establish signal and slots connections
    connect(mpNewPlotButton,                SIGNAL(clicked()),                                              this,               SLOT(addPlotTab()));
    connect(mpSaveButton,                   SIGNAL(clicked()),                                              this,               SLOT(saveToXml()));
    connect(mpSVGButton,                    SIGNAL(clicked()),                                              this,               SLOT(exportSVG()));
    connect(mpExportGNUPLOTButton,          SIGNAL(clicked()),                                              this,               SLOT(exportGNUPLOT()));
    connect(mpImportGNUPLOTButton,          SIGNAL(clicked()),                                              this,               SLOT(importGNUPLOT()));
    connect(mpShowListsButton,              SIGNAL(toggled(bool)),                                          mpComponentList,    SLOT(setVisible(bool)));
    connect(mpShowListsButton,              SIGNAL(toggled(bool)),                                          mpPortList,         SLOT(setVisible(bool)));
    connect(mpShowListsButton,              SIGNAL(toggled(bool)),                                          mpVariableList,     SLOT(setVisible(bool)));
    connect(mpNewWindowFromTabButton,       SIGNAL(clicked()),                                              this,               SLOT(createPlotWindowFromTab()));
    connect(mpComponentList,                SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,               SLOT(updatePortList()));
    connect(mpPortList,                     SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,               SLOT(updateVariableList()));
    connect(mpVariableList,                 SIGNAL(itemDoubleClicked(QListWidgetItem*)),                    this,               SLOT(addPlotCurveFromBoxes()));
    connect(gpMainWindow->mpOptionsDialog,  SIGNAL(paletteChanged()),                                       this,               SLOT(updatePalette()));
}


void PlotWindow::addPlotTab()
{
    QString numString;
    numString.setNum(mpPlotTabs->count());
    PlotTab *mpNewTab = new PlotTab(this);
    mpPlotTabs->addTab(mpNewTab, "Plot " + numString);

    mpPlotTabs->setCurrentIndex(mpPlotTabs->count()-1);
}


void PlotWindow::updatePortList()
{
    if(mpComponentList->count() == 0) { return; }

    disconnect(mpPortList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(updateVariableList()));

    mpPortList->clear();
    QList< QMap< QString, QMap< QString, QMap<QString, QVector<double> > > > > plotData = gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData();
    mpPortList->addItems(plotData.last().find(mpComponentList->currentItem()->text()).value().keys());
    mpPortList->setCurrentItem(mpPortList->item(0));
    mpPortList->sortItems();

    updateVariableList();

    connect(mpPortList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(updateVariableList()));
}


void PlotWindow::updateVariableList()
{
    if(mpPortList->count() == 0) { return; }

    mpVariableList->clear();
    QList< QMap< QString, QMap< QString, QMap<QString, QVector<double> > > > > plotData = gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData();
    mpVariableList->addItems(plotData.last().find(mpComponentList->currentItem()->text()).value().find(mpPortList->currentItem()->text()).value().keys());
    mpVariableList->setCurrentItem(mpVariableList->item(0));
    mpVariableList->sortItems();
}


void PlotWindow::addPlotCurveFromBoxes()
{
    this->addPlotCurve(gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData().size()-1,
                       mpComponentList->currentItem()->text(), mpPortList->currentItem()->text(), mpVariableList->currentItem()->text(), "", QwtPlot::yLeft);
}


PlotTabWidget *PlotWindow::getPlotTabWidget()
{
    return mpPlotTabs;
}


PlotTab *PlotWindow::getCurrentPlotTab()
{
    return qobject_cast<PlotTab *>(mpPlotTabs->currentWidget());
}


//! @brief Slot that exports current plot to .svg format
void PlotWindow::exportSVG()
{
    //! @todo Re-implement
}


//! @brief Slot that exports a curve to GNUPLOT format
void PlotWindow::exportGNUPLOT()
{
    //! @todo Re-implement
}


//! @brief Slot that imports a curve from a file in GNUPLOT format
void PlotWindow::importGNUPLOT()
{
    //! @todo Re-implement
}


//! @brief Creates a new plot curve from a plot variable in current container object and adds it to the current plot tab
//! @param generation Generation of plot data
//! @param componentName Name of component where variable is located
//! @param portName Name of port where variable is located
//! @param dataName Name of variable
//! @param dataUnit Unit of variable
void PlotWindow::addPlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY)
{
    getCurrentPlotTab()->getPlot()->replot();
    qDebug() << "dataUnit = " << dataUnit;
    if(dataUnit.isEmpty()) { dataUnit = gConfig.getDefaultUnit(dataName); }
    qDebug() << "dataUnit = " << dataUnit;
    PlotCurve *pTempCurve = new PlotCurve(generation, componentName, portName, dataName, dataUnit, axisY, getCurrentPlotTab());
    getCurrentPlotTab()->addCurve(pTempCurve);
    pTempCurve->updatePlotInfoDockVisibility();
}


void PlotWindow::saveToXml()
{
    //! @todo Re-implement
}


bool PlotWindow::saveToHmpf(QString fileName)
{
    //! @todo Re-implement
    return true;
}


//! @brief Reimplementation of close function for plot window. Notifies plot widget that window no longer exists.
void PlotWindow::close()
{
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->reportClosedPlotWindow(this);
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
    PlotWindow *pPlotWindow = new PlotWindow(mpPlotParameterTree, gpMainWindow);
    pPlotWindow->show();
    for(size_t i=0; i<getCurrentPlotTab()->getCurves().size(); ++i)
    {
        pPlotWindow->addPlotCurve(getCurrentPlotTab()->getCurves().at(i)->getGeneration(), getCurrentPlotTab()->getCurves().at(i)->getComponentName(), getCurrentPlotTab()->getCurves().at(i)->getPortName(), getCurrentPlotTab()->getCurves().at(i)->getDataName(), getCurrentPlotTab()->getCurves().at(i)->getDataUnit(), getCurrentPlotTab()->getCurves().at(i)->getAxisY());
    }
}


//! @brief Constructor for variable list widget
//! @param parentPlotWindow Pointer to plot window
//! @param parent Pointer to parent widget
VariableListWidget::VariableListWidget(PlotWindow *parentPlotWindow, QWidget *parent)
    : QListWidget(parent)
{
    this->setDragEnabled(true);
    mpParentPlotWindow = parentPlotWindow;
}


//! @brief Initializes drag operations from variable list widget
void VariableListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }

    QString mimeText;
    mimeText = QString("HOPSANPLOTDATA " + addQuotes(mpParentPlotWindow->mpComponentList->currentItem()->text()) + " " + addQuotes(mpParentPlotWindow->mpPortList->currentItem()->text()) + " " + addQuotes(mpParentPlotWindow->mpVariableList->currentItem()->text()));
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(mimeText);
    drag->setMimeData(mimeData);
    drag->exec();
}


//! @brief Constructor for plot info box
//! @param pParentPlotCurve pointer to parent plot curve
//! @param parent Pointer to parent widget
PlotInfoBox::PlotInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent)
    : QWidget(parent)
{
    mpParentPlotCurve = pParentPlotCurve;

    mpColorBlob = new QToolButton(this);
    QColor color = mpParentPlotCurve->mLineColor;
    QString redString, greenString, blueString;
    redString.setNum(color.red());
    greenString.setNum(color.green());
    blueString.setNum(color.blue());
    mpColorBlob->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mpColorBlob->setFixedSize(20, 20);
    mpColorBlob->setCheckable(true);
    mpColorBlob->setChecked(false);

    mpColorButton = new QToolButton(this);
    mpColorButton->setToolTip("Select Line Color");
    mpColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LineColor.png"));
    mpColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpColorButton->setFixedSize(25, 25);

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

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpColorBlob,            0,  0);
    mpLayout->addWidget(mpGenerationLabel,      0,  1);
    mpLayout->addWidget(mpPreviousButton,       0,  2);
    mpLayout->addWidget(mpNextButton,           0,  3);
    mpLayout->addWidget(mpSizeSpinBox,          0,  4);
    mpLayout->addWidget(mpCloseButton,          0,  5);
    mpLayout->addWidget(mpColorButton,          1,  2);
    mpLayout->addWidget(mpScaleButton,          1,  3);
    mpLayout->addWidget(mpAutoUpdateCheckBox,   1,  4,1,2);

    setLayout(mpLayout);

    connect(mpColorBlob,            SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setActive(bool)));
    connect(mpPreviousButton,       SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setPreviousGeneration()));
    connect(mpNextButton,           SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setNextGeneration()));
    connect(mpAutoUpdateCheckBox,   SIGNAL(toggled(bool)),  mpParentPlotCurve,  SLOT(setAutoUpdate(bool)));
}


//! @brief Constructor for the plot tab widget
//! @param parent Pointer to the plot window the plot tab widget belongs to
PlotTabWidget::PlotTabWidget(PlotWindow *parent)
    : QTabWidget(parent)
{
    mpParentPlotWindow = parent;
    this->setTabsClosable(true);

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closePlotTab(int)));
    connect(this,SIGNAL(currentChanged(int)),SLOT(tabChanged()));
}


//! @brief Closes the plot tab with specified index
//! @param index Index of tab to close
void PlotTabWidget::closePlotTab(int index)
{
    qDebug() << "Tjipp";
    PlotTab *tempTab = mpParentPlotWindow->getCurrentPlotTab();
    qDebug() << "Tjipp 1;";
    tempTab->close();
    qDebug() << "Tjipp 2;";
    delete(tempTab);
    qDebug() << "Tjipp 3;";
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
    //! @todo Finish this
    if(count() > 0) { this->show(); }
    else { this->hide(); }

    for(int i=0; i<count(); ++i)
    {
        disconnect(mpParentPlotWindow->mpZoomButton,                SIGNAL(toggled(bool)),  getTab(i),  SLOT(enableZoom(bool)));
        disconnect(mpParentPlotWindow->mpPanButton,                 SIGNAL(toggled(bool)),  getTab(i),  SLOT(enablePan(bool)));
        disconnect(mpParentPlotWindow->mpBackgroundColorButton,     SIGNAL(clicked()),      getTab(i),  SLOT(setBackgroundColor()));
        disconnect(mpParentPlotWindow->mpGridButton,                SIGNAL(toggled(bool)),  getTab(i),  SLOT(enableGrid(bool)));
        disconnect(mpParentPlotWindow->mpResetXVectorButton,       SIGNAL(clicked()),      getCurrentTab(),    SLOT(resetXVector()));
    }

    if(this->count() != 0)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(getCurrentTab()->mpZoomer->isEnabled());
        mpParentPlotWindow->mpPanButton->setChecked(getCurrentTab()->mpPanner->isEnabled());
        mpParentPlotWindow->mpGridButton->setChecked(getCurrentTab()->mpGrid->isVisible());
        mpParentPlotWindow->mpResetXVectorButton->setEnabled(getCurrentTab()->mHasSpecialXAxis);

        connect(mpParentPlotWindow->mpZoomButton,               SIGNAL(toggled(bool)),  getCurrentTab(),    SLOT(enableZoom(bool)));
        connect(mpParentPlotWindow->mpPanButton,                SIGNAL(toggled(bool)),  getCurrentTab(),    SLOT(enablePan(bool)));
        connect(mpParentPlotWindow->mpBackgroundColorButton,    SIGNAL(clicked()),      getCurrentTab(),    SLOT(setBackgroundColor()));
        connect(mpParentPlotWindow->mpGridButton,               SIGNAL(toggled(bool)),  getCurrentTab(),    SLOT(enableGrid(bool)));
        connect(mpParentPlotWindow->mpResetXVectorButton,       SIGNAL(clicked()),      getCurrentTab(),    SLOT(resetXVector()));
    }


}


//! @brief Constructor for plot tabs.
//! @param parent Pointer to the plot window the tab belongs to
PlotTab::PlotTab(PlotWindow *parent)
    : QWidget(parent)
{
    mpParentPlotWindow = parent;
    this->setAcceptDrops(true);
    mHasSpecialXAxis=false;
    mVectorXLabel = QString("Time [s]");

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

    mpPlot = new QwtPlot();
    mpPlot->setAcceptDrops(false);
    mpPlot->setCanvasBackground(QColor(Qt::white));
    mpPlot->setAutoReplot(true);

        //Panning Tool
    mpPanner = new QwtPlotPanner(mpPlot->canvas());
    mpPanner->setMouseButton(Qt::LeftButton);
    mpPanner->setEnabled(false);

        //Rubber Band Zoom
    mpZoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpPlot->canvas());      //Zoomer for left y axis
    mpZoomer->setMaxStackDepth(10000);
    mpZoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    mpZoomer->setRubberBand(QwtPicker::RectRubberBand);
    mpZoomer->setRubberBandPen(QColor(Qt::green));
    mpZoomer->setTrackerMode(QwtPicker::ActiveOnly);
    mpZoomer->setTrackerPen(QColor(Qt::white));
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    mpZoomer->setZoomBase(QwtDoubleRect());
    mpZoomer->setEnabled(false);

    mpZoomerRight = new QwtPlotZoomer( QwtPlot::xTop, QwtPlot::yRight, mpPlot->canvas());   //Zoomer for right y axis
    mpZoomerRight->setMaxStackDepth(10000);
    mpZoomerRight->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    mpZoomerRight->setRubberBandPen(QColor(Qt::green));
    mpZoomerRight->setTrackerMode(QwtPicker::ActiveOnly);
    mpZoomerRight->setTrackerPen(QColor(Qt::white));
    mpZoomerRight->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpZoomerRight->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    mpZoomer->setEnabled(false);

    //! @todo This doesn't work right now. Do we need a wheel zoom?
        //Wheel Zoom
    mpMagnifier = new QwtPlotMagnifier(mpPlot->canvas());
    mpMagnifier->setAxisEnabled(QwtPlot::yLeft, true);
    mpMagnifier->setAxisEnabled(QwtPlot::yRight, true);
    mpMagnifier->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
    mpMagnifier->setWheelFactor(1.1);
    mpMagnifier->setMouseButton(Qt::LeftButton);
    mpMagnifier->setEnabled(false);

        //Curve Marker
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    mpMarkerSymbol->setStyle(QwtSymbol::Ellipse);
    mpMarkerSymbol->setSize(10,10);
    mpActiveMarker = 0;

    mpGrid = new QwtPlotGrid;
    mpGrid->enableXMin(true);
    mpGrid->enableYMin(true);
    mpGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    mpGrid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    mpGrid->attach(mpPlot);

    QwtLegend *tempLegend = new QwtLegend();
    //tempLegend->setPalette(QPalette(QColor("black"), QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("black"), QColor("white"), QColor("white"), QColor("white")));
    tempLegend->setAutoFillBackground(false);

    QList<QWidget *> tempList = tempLegend->findChildren<QWidget *>();
    for(size_t i=0; i<tempList.size(); ++i)
    {
        tempList.at(i)->setAutoFillBackground(false);
    }
    mpPlot->insertLegend(tempLegend, QwtPlot::TopLegend);
    mpPlot->setAutoFillBackground(false);

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpPlot);
    this->setLayout(pLayout);

    mpPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


//! @brief Destructor for plot tab. Removes all curves before tab is deleted.
PlotTab::~PlotTab()
{
    while(!mPlotCurvePtrs.empty())
    {
        removeCurve(mPlotCurvePtrs.last());
    }
}


//! @brief Adds a plot curve to a plot tab
//! @param curve Pointer to the plot curve
void PlotTab::addCurve(PlotCurve *curve)
{
    if(mHasSpecialXAxis)
    {
        curve->getCurvePtr()->setData(mVectorX, curve->getDataVector());
    }

    mPlotCurvePtrs.append(curve);

    size_t i=0;
    while(mUsedColors.contains(mCurveColors.first()))
    {
        mCurveColors.append(mCurveColors.first());
        mCurveColors.pop_front();
        ++i;
        if(i>mCurveColors.size()) break;
    }
    mUsedColors.append(mCurveColors.first());
    mpPlot->enableAxis(curve->getAxisY());
    rescaleToCurves();
    updateLabels();
    mpPlot->replot();
    curve->setLineColor(mCurveColors.first());
    curve->setLineWidth(2);
    mpParentPlotWindow->addDockWidget(Qt::RightDockWidgetArea, curve->getPlotInfoDockWidget());
}


//! @brief Rescales the axes and the zommers so that all plot curves will fit
void PlotTab::rescaleToCurves()
{
    double xMin, xMax, yMinLeft, yMaxLeft, yMinRight, yMaxRight;

    xMin=0;
    xMax=10;
    yMinLeft=0;
    yMaxLeft=10;
    yMinRight=0;
    yMaxRight=10;

    if(!mPlotCurvePtrs.empty())
    {
        bool foundFirstLeft = false;
        bool foundFirstRight = false;

        xMin=mPlotCurvePtrs.first()->getCurvePtr()->minXValue();
        xMax=mPlotCurvePtrs.first()->getCurvePtr()->maxXValue();

        for(size_t i=0; i<mPlotCurvePtrs.size(); ++i)
        {
            if(mPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yLeft)
            {
                if(foundFirstLeft == false)
                {
                    yMinLeft=mPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    yMaxLeft=mPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                    foundFirstLeft = true;
                }
                else
                {
                    if(mPlotCurvePtrs.at(i)->getCurvePtr()->minYValue() < yMinLeft)
                        yMinLeft=mPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    if(mPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue() > yMaxLeft)
                        yMaxLeft=mPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                }
            }

            if(mPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yRight)
            {
                if(foundFirstRight == false)
                {
                    yMinRight=mPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    yMaxRight=mPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                    foundFirstRight = true;
                }
                else
                {
                    if(mPlotCurvePtrs.at(i)->getCurvePtr()->minYValue() < yMinRight)
                        yMinRight=mPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    if(mPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue() > yMaxRight)
                        yMaxRight=mPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                }
            }

            if(mPlotCurvePtrs.at(i)->getCurvePtr()->minXValue() < xMin)
                xMin=mPlotCurvePtrs.at(i)->getCurvePtr()->minXValue();
            if(mPlotCurvePtrs.at(i)->getCurvePtr()->maxXValue() > xMax)
                xMax=mPlotCurvePtrs.at(i)->getCurvePtr()->maxXValue();

        }
    }

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

    double heightLeft = yMaxLeft-yMinLeft;
    double heightRight = yMaxRight-yMinRight;

    mpPlot->setAxisScale(QwtPlot::yLeft, yMinLeft-0.05*heightLeft, yMaxLeft+0.05*heightLeft);
    mpPlot->setAxisScale(QwtPlot::yRight, yMinRight-0.05*heightRight, yMaxRight+0.05*heightRight);
    mpPlot->setAxisScale(QwtPlot::xBottom, xMin, xMax);

    QwtDoubleRect tempDoubleRect;
    tempDoubleRect.setX(xMin);
    tempDoubleRect.setY(yMinLeft-0.05*heightLeft);
    tempDoubleRect.setWidth(xMax-xMin);
    tempDoubleRect.setHeight(yMaxLeft-yMinLeft+0.1*heightLeft);
    mpZoomer->setZoomBase(tempDoubleRect);

    QwtDoubleRect tempDoubleRect2;
    tempDoubleRect2.setX(xMin);
    tempDoubleRect2.setY(yMinRight-0.05*heightRight);
    tempDoubleRect2.setHeight(yMaxRight-yMinRight+0.1*heightRight);
    tempDoubleRect2.setWidth(xMax-xMin);
    mpZoomerRight->setZoomBase(tempDoubleRect2);
}


void PlotTab::removeCurve(PlotCurve *curve)
{
    for(size_t i=0; i<mUsedColors.size(); ++i)
    {
        if(curve->getCurvePtr()->pen().color() == mUsedColors.at(i))
        {
            mUsedColors.removeAt(i);
            break;
        }
    }

    curve->getCurvePtr()->detach();
    mPlotCurvePtrs.removeAll(curve);
    delete(curve);
    rescaleToCurves();
    updateLabels();
    mpPlot->replot();
}


//! @brief Changes the X vector of current plot tab to specified variable
//
void PlotTab::changeXVector(QVector<double> xArray, QString componentName, QString portName, QString dataName, QString dataUnit)
{
    mVectorX = xArray;

    for(size_t i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        mPlotCurvePtrs.at(i)->getCurvePtr()->setData(mVectorX, mPlotCurvePtrs.at(i)->getDataVector());
    }

    rescaleToCurves();
    mVectorXLabel = QString(dataName + " [" + dataUnit + "]");
    updateLabels();
    mpPlot->replot();
    mVectorX = xArray;
    mHasSpecialXAxis = true;
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(true);
}


void PlotTab::updateLabels()
{
    mpPlot->setAxisTitle(QwtPlot::yLeft, QwtText());
    mpPlot->setAxisTitle(QwtPlot::yRight, QwtText());

    QStringList leftUnits, rightUnits;
    for(size_t i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        QString newUnit = QString(mPlotCurvePtrs.at(i)->getDataName() + " [" + mPlotCurvePtrs.at(i)->getDataUnit() + "]");
        if( !(mPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yLeft && leftUnits.contains(newUnit)) && !(mPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yRight && rightUnits.contains(newUnit)) )
        {
            if(!mpPlot->axisTitle(mPlotCurvePtrs.at(i)->getAxisY()).isEmpty())
            {
                mpPlot->setAxisTitle(mPlotCurvePtrs.at(i)->getAxisY(), QwtText(QString(mpPlot->axisTitle(mPlotCurvePtrs.at(i)->getAxisY()).text().append(", "))));
            }
            mpPlot->setAxisTitle(mPlotCurvePtrs.at(i)->getAxisY(), QwtText(QString(mpPlot->axisTitle(mPlotCurvePtrs.at(i)->getAxisY()).text().append(newUnit))));
            if(mPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yLeft)
            {
                leftUnits.append(newUnit);
            }
            if(mPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yRight)
            {
                rightUnits.append(newUnit);
            }
        }
    }

    mpPlot->setAxisTitle(QwtPlot::xBottom, QwtText(mVectorXLabel));
}


void PlotTab::resetXVector()
{
    for(size_t i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        mPlotCurvePtrs.at(i)->getCurvePtr()->setData(mPlotCurvePtrs.at(i)->getTimeVector(), mPlotCurvePtrs.at(i)->getDataVector());
    }

    mVectorXLabel = QString("Time [s]");
    updateLabels();

    mHasSpecialXAxis = false;

    rescaleToCurves();
    mpPlot->replot();

    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


void PlotTab::enableZoom(bool value)
{
    if(mpParentPlotWindow->mpPanButton->isChecked() && value)
    {
        mpParentPlotWindow->mpPanButton->setChecked(false);
        mpPanner->setEnabled(false);
    }
    mpZoomer->setEnabled(value);
    mpZoomerRight->setEnabled(value);
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


void PlotTab::enablePan(bool value)
{
    if(mpParentPlotWindow->mpZoomButton->isChecked() && value)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        mpZoomer->setEnabled(false);
        mpZoomerRight->setEnabled(false);
    }
    mpPanner->setEnabled(value);
}


void PlotTab::enableGrid(bool value)
{
    mpGrid->setVisible(value);
}


void PlotTab::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(mpPlot->canvasBackground(), this);
    if (color.isValid())
    {
        mpPlot->setCanvasBackground(color);
        mpPlot->replot();
    }
}


QList<PlotCurve *> PlotTab::getCurves()
{
    return mPlotCurvePtrs;
}


void PlotTab::setActivePlotCurve(PlotCurve *pCurve)
{
    mpActivePlotCurve = pCurve;
}


PlotCurve *PlotTab::getActivePlotCurve()
{
    return mpActivePlotCurve;
}


QwtPlot *PlotTab::getPlot()
{
    return mpPlot;
}


int PlotTab::getNumberOfCurves()
{
    return mPlotCurvePtrs.size();
}


void PlotTab::update()
{
    //mpPlot->clear();
    QList<PlotCurve *>::iterator cit;
    for(cit=mPlotCurvePtrs.begin(); cit!=mPlotCurvePtrs.end(); ++cit)
    {
        (*cit)->getCurvePtr()->attach(mpPlot);
    }
    mpPlot->replot();
}


//! @brief Inserts a curve marker at the specified curve
//! @param curve is a pointer to the specified curve
void PlotTab::insertMarker(QwtPlotCurve *curve)
{
    //! @todo Re-implement
}


//! @brief Changes the active marker (the on that can be moved around)
//! @param[in] marker is a pointer to the marker that shall be activated
void PlotTab::setActiveMarker(QwtPlotMarker *marker)
{
    //! @todo Re-implement
}


//! @brief Defines what happens when used drags something into the plot window
void PlotTab::dragEnterEvent(QDragEnterEvent *event)
{
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
    QCursor cursor;
    if(this->mapFromGlobal(cursor.pos()).y() > this->height()/2 && getNumberOfCurves() >= 1)
    {
        mpHoverRect->setGeometry(getPlot()->canvas()->x()+9, getPlot()->canvas()->height()/2+getPlot()->canvas()->y()+10, getPlot()->canvas()->width(), getPlot()->canvas()->height()/2);
    }
    else if(this->mapFromGlobal(cursor.pos()).x() < this->width()/2)
    {
        mpHoverRect->setGeometry(getPlot()->canvas()->x()+9, getPlot()->canvas()->y()+9, getPlot()->canvas()->width()/2, getPlot()->canvas()->height());
    }
    else
    {
        mpHoverRect->setGeometry(getPlot()->canvas()->x()+9 + getPlot()->canvas()->width()/2, getPlot()->canvas()->y()+9, getPlot()->canvas()->width()/2, getPlot()->canvas()->height());
    }
    QWidget::dragMoveEvent(event);
}


//! @brief Defines what happens when user drags something out from the plot window.
void PlotTab::dragLeaveEvent(QDragLeaveEvent *event)
{
    delete(mpHoverRect);
    QWidget::dragLeaveEvent(event);
}


//! @brief Defines what happens when user drops something in the plot window
void PlotTab::dropEvent(QDropEvent *event)
{
    QWidget::dropEvent(event);

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
            if(mpParentPlotWindow->mapFromGlobal(cursor.pos()).y() > mpParentPlotWindow->height()/2 && getNumberOfCurves() >= 1)
            {
                changeXVector(gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotData(gpMainWindow->mpProjectTabs->getCurrentContainer()->getNumberOfPlotGenerations()-1, componentName, portName, dataName), componentName, portName, dataName, gConfig.getDefaultUnit(dataName));
            }
            else if(mpParentPlotWindow->mapFromGlobal(cursor.pos()).x() < mpParentPlotWindow->width()/2)
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

    if(this->mpZoomer->isEnabled())
    {
        return;
    }

    QMenu menu;

    QMenu *yAxisRightMenu;
    QMenu *yAxisLeftMenu;
    QMenu *changeUnitsMenu;
//    QMenu *insertMarkerMenu;
//    QMenu *selectMarkerMenu;

//    QMenu *changeUnitMenuRight;
//    QMenu *removeCurveMenu;

    QAction *setRightAxisLogarithmic;
    QAction *setLeftAxisLogarithmic;

    yAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    yAxisRightMenu = menu.addMenu(QString("Right Y Axis"));

    changeUnitsMenu = menu.addMenu(QString("Change Units"));

    QMap<QString, double> unitMap;
    QList<PlotCurve *>::iterator itc;
    QMap<QString, double>::iterator itu;
    for(itc=mPlotCurvePtrs.begin(); itc!=mPlotCurvePtrs.end(); ++itc)
    {
        QMenu *pTempMenu = changeUnitsMenu->addMenu(QString((*itc)->getComponentName() + ", " + (*itc)->getPortName() + ", " + (*itc)->getDataName()));
        unitMap = gConfig.getCustomUnits((*itc)->getDataName());
        for(itu=unitMap.begin(); itu!=unitMap.end(); ++itu)
        {
            QAction *pTempAction = pTempMenu->addAction(itu.key());
        }
    }


        //Create actions for making axis logarithmic
    if(mpPlot->axisEnabled(QwtPlot::yLeft))
    {
        setLeftAxisLogarithmic = yAxisLeftMenu->addAction("Logarithmic Scale");
        setLeftAxisLogarithmic->setCheckable(true);
        setLeftAxisLogarithmic->setChecked(mLeftAxisLogarithmic);
    }
    if(mpPlot->axisEnabled(QwtPlot::yRight))
    {
        setRightAxisLogarithmic = yAxisRightMenu->addAction("Logarithmic Scale");
        setRightAxisLogarithmic->setCheckable(true);
        setRightAxisLogarithmic->setChecked(mRightAxisLogarithmic);
    }


//        //Create menu for insereting curve markers
//    insertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
//    QMap <QAction *, QwtPlotCurve *> actionToCurveMap;
//    QAction *tempAction;
//    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
//    {
//        tempAction = insertMarkerMenu->addAction(this->mPlotCurvePtrs[i]->getCurvePtr()->title().text());
//        actionToCurveMap.insert(tempAction, mPlotCurvePtrs[i]->getCurvePtr());
//        if(mCurveToMarkerMap.contains(mPlotCurvePtrs[i]->getCurvePtr()))
//        {
//           tempAction->setDisabled(true);
//        }
//    }


//        //Create menu for selecting curve markers
//    selectMarkerMenu = menu.addMenu(QString("Change Active Marker"));
//    QMap <QAction *, QwtPlotMarker *> actionToMarkerMap;
//    if(mpMarkers.size() < 2)
//    {
//        selectMarkerMenu->setDisabled(true);    //Disable the select marker menu if there are less than two markers
//    }
//    else
//    {
//        for(int i=0; i<mpMarkers.size(); ++i)
//        {
//            tempAction = selectMarkerMenu->addAction(mMarkerToCurveMap.value(mpMarkers[i])->title().text());
//            actionToMarkerMap.insert(tempAction, mpMarkers[i]);
//            if(mpActiveMarker == mpMarkers[i])
//            {
//                QFont tempFont = tempAction->font();
//                tempFont.setBold(true);
//                tempAction->setFont(tempFont);
//            }
//        }
//    }


//        //Create menu for removing curves
//    removeCurveMenu = menu.addMenu(QString("Remove Plot Curve"));
//    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
//    {
//        tempAction = removeCurveMenu->addAction(mPlotCurvePtrs[i]->getCurvePtr()->title().text());
//        actionToCurveMap.insert(tempAction, mPlotCurvePtrs[i]->getCurvePtr());
//    }




    // ----- Wait for user to make a selection ----- //

    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    // ----- User has selected something -----  //



        // Check if user did not click on a menu item
    if(selectedAction == 0)
    {
        return;
    }


//        // Change unit on left axis
//    if((selectedAction->parentWidget() == changeUnitMenuLeft) && (gConfig.getCustomUnits(physicalQuantityLeft).contains(selectedAction->text())))
//    {
//        //this->setUnit(QwtPlot::yLeft, physicalQuantityLeft, selectedAction->text());
//    }


//        // Change unit on right axis
//    if((selectedAction->parentWidget() == changeUnitMenuRight) && (gConfig.getCustomUnits(physicalQuantityRight).contains(selectedAction->text())))
//    {
//        //this->setUnit(QwtPlot::yRight, physicalQuantityRight, selectedAction->text());
//    }


        //Make axis logarithmic
    if (selectedAction == setRightAxisLogarithmic)
    {
        mRightAxisLogarithmic = !mRightAxisLogarithmic;
        if(mRightAxisLogarithmic)
        {
            mpPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLog10ScaleEngine);
        }
        else
        {
            mpPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
        }
    }
    else if (selectedAction == setLeftAxisLogarithmic)
    {
        mLeftAxisLogarithmic = !mLeftAxisLogarithmic;
        if(mLeftAxisLogarithmic)
        {
            mpPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
        }
        else
        {
            mpPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
        }
    }


//        //Insert curve marker
//    QMap<QAction *, QwtPlotCurve *>::iterator it;
//    if(selectedAction->parentWidget() == insertMarkerMenu)
//    {
//        for(it = actionToCurveMap.begin(); it!=actionToCurveMap.end(); ++it)
//        {
//            if(selectedAction == it.key())
//            {
//                this->insertMarker(it.value());
//            }
//        }
//    }

//        //Change active curve marker
//    QMap<QAction *, QwtPlotMarker *>::iterator itm;
//    for(itm = actionToMarkerMap.begin(); itm!=actionToMarkerMap.end(); ++itm)
//    {
//        if(selectedAction == itm.key())
//        {
//            this->setActiveMarker(itm.value());
//        }
//    }


//        //Remove plot curve
//    if(selectedAction->parentWidget() == removeCurveMenu)
//    {
//        for(it = actionToCurveMap.begin(); it!=actionToCurveMap.end(); ++it)
//        {
//            if(selectedAction == it.key())
//            {
//                it.value()->detach();
//                int i;
//                for(i=0; i<mPlotCurvePtrs.size(); ++i)
//                {
//                    if(mPlotCurvePtrs[i]->getCurvePtr() == it.value())
//                    {
//                        break;
//                    }
//                }
//                mpCurves.remove(i);
//                delete(it.value());
//            }
//        }
//        mpPlot->replot();
//    }
}


//! @todo Why is this here?
void PlotTab::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "Mouse released!";
    QWidget::mouseReleaseEvent(event);
}


class PlotInfoBox;


PlotCurve::PlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY, PlotTab *parent)
{
    mpParentPlotTab = parent;
    mGeneration = generation;
    mComponentName = componentName;
    mPortName = portName;
    mDataName = dataName;
    if(dataUnit.isEmpty())
    {
        mDataUnit = gConfig.getDefaultUnit(dataName);//mpParentPlotTab->mCurrentUnitsLeft.find(dataName).value();
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

    mDataVector = gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotData(generation, componentName, portName, dataName);
    mTimeVector = gpMainWindow->mpProjectTabs->getCurrentContainer()->getTimeVector(generation);

    mpCurve = new QwtPlotCurve(QString(mComponentName+", "+mPortName+", "+mDataName));
    updateCurve();
    mpCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpCurve->setAxis(QwtPlot::xBottom, axisY);
    mpCurve->attach(parent->getPlot());

    mpPlotInfoBox = new PlotInfoBox(this, mpParentPlotTab);
    updatePlotInfoBox();
    mpPlotInfoBox->mpSizeSpinBox->setValue(2);
    mpPlotInfoDockWidget = new QDockWidget(mComponentName+", "+mPortName+", "+mDataName+" ["+mDataUnit+"]", mpParentPlotTab->mpParentPlotWindow);
    mpPlotInfoDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    mpPlotInfoDockWidget->setMaximumHeight(100);
    mpPlotInfoDockWidget->setWidget(mpPlotInfoBox);
    mpPlotInfoDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mpPlotInfoDockWidget->setMinimumWidth(mpPlotInfoDockWidget->windowTitle().length()*6);
    mpPlotInfoDockWidget->hide();

    connect(mpPlotInfoBox->mpSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setLineWidth(int)));
    connect(mpPlotInfoBox->mpColorButton, SIGNAL(clicked()), this, SLOT(setLineColor()));
    connect(mpPlotInfoBox->mpScaleButton, SIGNAL(clicked()), this, SLOT(openScaleDialog()));
    connect(mpParentPlotTab->mpParentPlotWindow->getPlotTabWidget(), SIGNAL(currentChanged(int)), this, SLOT(updatePlotInfoDockVisibility()));
    connect(mpParentPlotTab->mpParentPlotWindow->mpShowCurvesButton, SIGNAL(toggled(bool)), SLOT(updatePlotInfoDockVisibility()));
    connect(mpPlotInfoBox->mpCloseButton, SIGNAL(clicked()), this, SLOT(removeMe()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));
}


PlotCurve::~PlotCurve()
{
    mpPlotInfoDockWidget->hide();
    delete(mpPlotInfoBox);
    delete(mpPlotInfoDockWidget);
}


int PlotCurve::getGeneration()
{
    return mGeneration;
}


QwtPlotCurve *PlotCurve::getCurvePtr()
{
    return mpCurve;
}


QDockWidget *PlotCurve::getPlotInfoDockWidget()
{
    return mpPlotInfoDockWidget;
}


QString PlotCurve::getComponentName()
{
    return mComponentName;
}


QString PlotCurve::getPortName()
{
    return mPortName;
}


QString PlotCurve::getDataName()
{
    return mDataName;
}


QString PlotCurve::getDataUnit()
{
    return mDataUnit;
}


int PlotCurve::getAxisY()
{
    return mAxisY;
}


QVector<double> PlotCurve::getDataVector()
{
    return mDataVector;
}


QVector<double> PlotCurve::getTimeVector()
{
    return mTimeVector;
}


void PlotCurve::setGeneration(int generation)
{
    mGeneration = generation;
    mDataVector = gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotData(mGeneration, mComponentName, mPortName, mDataName);
    if(mpParentPlotTab->mVectorX.size() == 0)
        mTimeVector = gpMainWindow->mpProjectTabs->getCurrentContainer()->getTimeVector(mGeneration);
    else
        mTimeVector = mpParentPlotTab->mVectorX;

    updateCurve();
    mpParentPlotTab->update();
    updatePlotInfoBox();
}


void PlotCurve::setDataUnit(QString unit)
{
    mDataUnit = unit;
    updateCurve();
}


void PlotCurve::setScaling(double scaleX, double scaleY, double offsetX, double offsetY)
{
    mScaleX=scaleX;
    mScaleY=scaleY;
    mOffsetX=offsetX;
    mOffsetY=offsetY;
    updateCurve();
}


void PlotCurve::setPreviousGeneration()
{
    if(mGeneration>0)       //This check should not really be necessary since button is disabled anyway, but just to be sure...
        setGeneration(mGeneration-1);
}


void PlotCurve::setNextGeneration()
{
    if(mGeneration<gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getNumberOfPlotGenerations()-1)       //This check should not really be necessary since button is disabled anyway, but just to be sure...
        setGeneration(mGeneration+1);
}


void PlotCurve::setLineWidth(int lineWidth)
{
    mLineWidth = lineWidth;
    QPen tempPen = mpCurve->pen();
    tempPen.setWidth(lineWidth);
    mpCurve->setPen(tempPen);
}


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
    mpPlotInfoBox->mpColorBlob->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
}


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


void PlotCurve::openScaleDialog()
{
    QDialog *pScaleDialog = new QDialog(mpParentPlotTab->mpParentPlotWindow);
    pScaleDialog->setWindowTitle("Change Curve Scale");

    QLabel *pXScaleLabel = new QLabel("Time Axis Scale: ", pScaleDialog);
    mpXScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXScaleSpinBox->setRange(-1000000000000000, 1000000000000000);
    mpXScaleSpinBox->setDecimals(10);
    mpXScaleSpinBox->setSingleStep(0.1);
    mpXScaleSpinBox->setValue(mScaleX);

    QLabel *pXOffsetLabel = new QLabel("Time Axis Offset: ", pScaleDialog);
    mpXOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXOffsetSpinBox->setDecimals(10);
    mpXOffsetSpinBox->setRange(-1000000000000000, 1000000000000000);
    mpXOffsetSpinBox->setSingleStep(0.1);
    mpXOffsetSpinBox->setValue(mOffsetX);

    QLabel *pYScaleLabel = new QLabel("Y-Axis Scale: ", pScaleDialog);
    mpYScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYScaleSpinBox->setSingleStep(0.1);
    mpYScaleSpinBox->setDecimals(10);
    mpYScaleSpinBox->setRange(-1000000000000000, 1000000000000000);
    mpYScaleSpinBox->setValue(mScaleY);

    QLabel *pYOffsetLabel = new QLabel("Y-Axis Offset: ", pScaleDialog);
    mpYOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYOffsetSpinBox->setDecimals(10);
    mpYOffsetSpinBox->setRange(-1000000000000000, 1000000000000000);
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


void PlotCurve::updateScaleFromDialog()
{
    setScaling(mpXScaleSpinBox->value(), mpYScaleSpinBox->value(), mpXOffsetSpinBox->value(), mpYOffsetSpinBox->value());
    mpParentPlotTab->rescaleToCurves();
}


void PlotCurve::updatePlotInfoDockVisibility()
{
    if(mpParentPlotTab == mpParentPlotTab->mpParentPlotWindow->getCurrentPlotTab() && mpParentPlotTab->mpParentPlotWindow->mpShowCurvesButton->isChecked())
    {
        mpPlotInfoDockWidget->show();
    }
    else
    {
        mpPlotInfoDockWidget->hide();
    }
}


void PlotCurve::removeMe()
{
    mpParentPlotTab->removeCurve(this);
}


void PlotCurve::updateToNewGeneration()
{
    if(mAutoUpdate)     //Only change the generation if auto update is on
        setGeneration(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getNumberOfPlotGenerations()-1);
    updatePlotInfoBox();    //Update the plot info box regardless of auto update setting, to show number of available generations correctly
}


void PlotCurve::updatePlotInfoBox()
{
    mpPlotInfoBox->mpPreviousButton->setEnabled(mGeneration > 0 && gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getNumberOfPlotGenerations() > 1);
    mpPlotInfoBox->mpNextButton->setEnabled(mGeneration < gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getNumberOfPlotGenerations()-1 && gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getNumberOfPlotGenerations() > 1);

    QString numString1, numString2;
    numString1.setNum(mGeneration+1);
    numString2.setNum(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getNumberOfPlotGenerations());
    mpPlotInfoBox->mpGenerationLabel->setText(numString1 + "(" + numString2 + ")");
}


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
//        mpPlotInfoBox->setPalette(QPalette(QColor(240,240,240), QColor(240,240,240)));
        mpPlotInfoBox->setAutoFillBackground(false);
        mpPlotInfoBox->mpColorBlob->setChecked(false);
    }
}


void PlotCurve::updateCurve()
{
    double unitScale = gConfig.getCustomUnits(mDataName).find(mDataUnit).value();
    QVector<double> tempX;
    QVector<double> tempY;
    if(mpParentPlotTab->mHasSpecialXAxis)
    {
        for(size_t i=0; i<mTimeVector.size(); ++i)
        {
            tempX.append(mpParentPlotTab->mVectorX[i]*mScaleX + mOffsetX);
            tempY.append(mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    else
    {
        for(size_t i=0; i<mTimeVector.size(); ++i)
        {
            tempX.append(mTimeVector[i]*mScaleX + mOffsetX);
            tempY.append(mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    mpCurve->setData(tempX, tempY);
}


void PlotCurve::setAutoUpdate(bool value)
{
    mAutoUpdate = value;
}
