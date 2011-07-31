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
#include "loadObjects.h"
#include "version.h"

#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include "qwt_plot_renderer.h"
//#include "qwt_double_rect.h"

const double DBLMAX = std::numeric_limits<double>::max();

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

    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    resize(sw*0.7, sh*0.7);   //Resize plot window to 70% of screen height and width
    int w = this->size().width();
    int h = this->size().height();
    int x = (sw - w)/2;
    int y = (sh - h)/2;
    move(x, y);       //Move plot window to center of screen

    mpPlotParameterTree = plotParameterTree;

        //Create the toolbar and its buttons
    mpToolBar = new QToolBar(this);

    mpNewPlotButton = new QToolButton(mpToolBar);
    mpNewPlotButton->setToolTip("Create New Plot");
    mpNewPlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));

    mpZoomButton = new QToolButton(mpToolBar);
    mpZoomButton->setToolTip("Zoom (Z)");
    mpZoomButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Zoom.png"));
    mpZoomButton->setCheckable(true);
    mpZoomButton->setShortcut(QKeySequence("z"));

    mpPanButton = new QToolButton(mpToolBar);
    mpPanButton->setToolTip("Pan (X)");
    mpPanButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Pan.png"));
    mpPanButton->setCheckable(true);
    mpPanButton->setShortcut(QKeySequence("x"));

    mpSaveButton = new QToolButton(mpToolBar);
    mpSaveButton->setToolTip("Save Plot Window Description File (.xml)");
    mpSaveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Save.png"));
    mpSaveButton->setShortcut(QKeySequence("Ctrl+s"));

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

    mpLoadFromXmlButton = new QToolButton(mpToolBar);
    mpLoadFromXmlButton->setToolTip("Import Plot");
    mpLoadFromXmlButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Open.png"));

    mpGridButton = new QToolButton(mpToolBar);
    mpGridButton->setToolTip("Show Grid (G)");
    mpGridButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Grid.png"));
    mpGridButton->setCheckable(true);
    mpGridButton->setChecked(true);
    mpGridButton->setShortcut(QKeySequence("g"));

    mpBackgroundColorButton = new QToolButton(mpToolBar);
    mpBackgroundColorButton->setToolTip("Select Canvas Color (C)");
    mpBackgroundColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-BackgroundColor.png"));
    mpBackgroundColorButton->setShortcut(QKeySequence("c"));

    mpShowListsButton = new QToolButton(mpToolBar);
    mpShowListsButton->setCheckable(true);
    mpShowListsButton->setChecked(true);
    mpShowListsButton->setToolTip("Toggle Parameter Lists");
    mpShowListsButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowLists.png"));

    mpShowCurvesButton = new QToolButton(mpToolBar);
    mpShowCurvesButton->setCheckable(true);
    mpShowCurvesButton->setChecked(true);
    mpShowCurvesButton->setToolTip("Toggle Curve Controls");
    mpShowCurvesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowPlotWindowCurves.png"));

    mpNewWindowFromTabButton = new QToolButton(mpToolBar);
    mpNewWindowFromTabButton->setToolTip("Create Plot Window From Tab");
    mpNewWindowFromTabButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-OpenTabInNewPlotWindow.png"));

    mpResetXVectorButton = new QToolButton(mpToolBar);
    mpResetXVectorButton->setToolTip("Reset Time Vector");
    mpResetXVectorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    mpResetXVectorButton->setEnabled(false);

    mpBodePlotButton = new QToolButton(mpToolBar);
    mpBodePlotButton->setToolTip("Create Bode Plot");
    mpBodePlotButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-TransferFunctionAnalysis.png"));

    mpToolBar->addWidget(mpNewPlotButton);
    mpToolBar->addWidget(mpLoadFromXmlButton);
    mpToolBar->addWidget(mpSaveButton);
    mpToolBar->addWidget(mpExportButton);
    mpToolBar->addWidget(mpExportGfxButton);
    mpToolBar->addSeparator();
    mpToolBar->addWidget(mpZoomButton);
    mpToolBar->addWidget(mpPanButton);
    mpToolBar->addWidget(mpGridButton);
    mpToolBar->addWidget(mpBackgroundColorButton);
    mpToolBar->addWidget(mpResetXVectorButton);
    mpToolBar->addWidget(mpBodePlotButton);
    mpToolBar->addSeparator();
    mpToolBar->addWidget(mpShowListsButton);
    mpToolBar->addWidget(mpShowCurvesButton);
    mpToolBar->addWidget(mpNewWindowFromTabButton);

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
    updateLists();

        //Establish signal and slots connections
    connect(mpNewPlotButton,                                        SIGNAL(clicked()),                                              this,               SLOT(addPlotTab()));
    connect(mpLoadFromXmlButton,                                    SIGNAL(clicked()),                                              this,               SLOT(loadFromXml()));
    connect(mpSaveButton,                                           SIGNAL(clicked()),                                              this,               SLOT(saveToXml()));
    connect(mpBodePlotButton,                                       SIGNAL(clicked()),                                              this,               SLOT(createBodePlot()));
    connect(mpShowListsButton,                                      SIGNAL(toggled(bool)),                                          mpComponentList,    SLOT(setVisible(bool)));
    connect(mpShowListsButton,                                      SIGNAL(toggled(bool)),                                          mpPortList,         SLOT(setVisible(bool)));
    connect(mpShowListsButton,                                      SIGNAL(toggled(bool)),                                          mpVariableList,     SLOT(setVisible(bool)));
    connect(mpShowListsButton,                                      SIGNAL(toggled(bool)),                                          mpComponentsLabel,  SLOT(setVisible(bool)));
    connect(mpShowListsButton,                                      SIGNAL(toggled(bool)),                                          mpPortsLabel,       SLOT(setVisible(bool)));
    connect(mpShowListsButton,                                      SIGNAL(toggled(bool)),                                          mpVariablesLabel,   SLOT(setVisible(bool)));
    connect(mpNewWindowFromTabButton,                               SIGNAL(clicked()),                                              this,               SLOT(createPlotWindowFromTab()));
    connect(mpComponentList,                                        SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,               SLOT(updatePortList()));
    connect(gpMainWindow->mpProjectTabs,                            SIGNAL(currentChanged(int)),                                    this,               SLOT(updateLists()));
    connect(gpMainWindow->mpProjectTabs->getCurrentContainer(),     SIGNAL(componentChanged()),                                     this,               SLOT(updateLists()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),           SIGNAL(simulationFinished()),                                   this,               SLOT(updateLists()),    Qt::UniqueConnection);
    connect(mpPortList,                                             SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,               SLOT(updateVariableList()));
    connect(mpVariableList,                                         SIGNAL(itemDoubleClicked(QListWidgetItem*)),                    this,               SLOT(addPlotCurveFromBoxes()));
    connect(gpMainWindow->getOptionsDialog(),                       SIGNAL(paletteChanged()),                                       this,               SLOT(updatePalette()));


        //Hide lists and curve areas by default if screen size is small
    if(sh*sw <= 800*1280)
    {
        mpShowListsButton->toggle();
        mpShowCurvesButton->toggle();
    }
}


void PlotWindow::addPlotTab()
{
    QString numString;
    numString.setNum(mpPlotTabs->count());
    PlotTab *mpNewTab = new PlotTab(this);
    mpPlotTabs->addTab(mpNewTab, "Plot " + numString);

    mpPlotTabs->setCurrentIndex(mpPlotTabs->count()-1);
}


//! @brief Updates the lists at the bottom of the plot window
void PlotWindow::updateLists()
{
    qDebug() << "Update lists!";

        //Disconnect update functions from item change slots, to prevent new updates before this one is finished
    disconnect(mpComponentList,     SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,   SLOT(updatePortList()));
    disconnect(mpPortList,          SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,   SLOT(updateVariableList()));

        //Clear everything from the lists
    mpVariableList->clear();
    mpPortList->clear();
    mpComponentList->clear();

        //Fetch new data and add to the lists
    QList< QMap< QString, QMap< QString, QMap<QString, QVector<double> > > > > plotData = gpMainWindow->mpProjectTabs->getCurrentContainer()->getAllPlotData();
    if(!plotData.isEmpty())
    {
        mpComponentList->addItems(plotData.last().keys());
        mpComponentList->setCurrentItem(mpComponentList->item(0));
        updatePortList();
    }

        //Connect this slot with simulation finished signal from plot tab (in case it has changed)
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),   SIGNAL(simulationFinished()),   this,   SLOT(updateLists()),    Qt::UniqueConnection);

        //Reconnect update functions
    connect(mpComponentList,     SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,   SLOT(updatePortList()));
    connect(mpPortList,          SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),  this,   SLOT(updateVariableList()));
}


//! @brief Updates the port list
void PlotWindow::updatePortList()
{
    qDebug() << "Update port lists!";
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


//! @brief Updates the variable list
void PlotWindow::updateVariableList()
{
    qDebug() << "Update variable lists!";
    if(mpComponentList->count() == 0 || mpPortList->count() == 0) { return; }

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


//! @brief Creates a new plot curve from a plot variable in current container object and adds it to the current plot tab
//! @param generation Generation of plot data
//! @param componentName Name of component where variable is located
//! @param portName Name of port where variable is located
//! @param dataName Name of variable
//! @param dataUnit Unit of variable
void PlotWindow::addPlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY, QString modelPath)
{
    if(dataUnit.isEmpty()) { dataUnit = gConfig.getDefaultUnit(dataName); }
    PlotCurve *pTempCurve = new PlotCurve(generation, componentName, portName, dataName, dataUnit, axisY, modelPath, getCurrentPlotTab());
    getCurrentPlotTab()->addCurve(pTempCurve);
    pTempCurve->updatePlotInfoDockVisibility();
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
    addPlotTab();
    getCurrentPlotTab()->getPlot()->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
    getCurrentPlotTab()->updateLabels();
    PlotCurve *pNewCurve = new PlotCurve(curve->getGeneration(), curve->getComponentName(), curve->getPortName(), curve->getDataName(), curve->getDataUnit(), curve->getAxisY(), curve->getContainerObjectPtr()->getModelFileInfo().filePath(), getCurrentPlotTab());
    getCurrentPlotTab()->addCurve(pNewCurve);
    pNewCurve->toFrequencySpectrum();
    pNewCurve->updatePlotInfoDockVisibility();
}


void PlotWindow::createBodePlot()
{
    QDialog *pCreateBodeDialog = new QDialog(this);
    pCreateBodeDialog->setWindowTitle("Create Bode Plot");

    QGroupBox *pInputGroupBox = new QGroupBox(tr("Input Variable"));
    QVBoxLayout *pInputGroupBoxLayout = new QVBoxLayout;
    pInputGroupBoxLayout->addStretch(1);
    for(int i=0; i<getCurrentPlotTab()->getNumberOfCurves(); ++i)
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
    for(int i=0; i<getCurrentPlotTab()->getNumberOfCurves(); ++i)
    {
        QRadioButton *radio = new QRadioButton(getCurrentPlotTab()->getCurves().at(i)->getComponentName() + ", " +
                                               getCurrentPlotTab()->getCurves().at(i)->getPortName() + ", " +
                                               getCurrentPlotTab()->getCurves().at(i)->getDataName());
        mBodeOutputButtonToCurveMap.insert(radio, getCurrentPlotTab()->getCurves().at(i));
        pOutputGroupBoxLayout->addWidget(radio);
    }
    pOutputGroupBox->setLayout(pOutputGroupBoxLayout);

    QPushButton *pCancelButton = new QPushButton("Cancel");
    QPushButton *pNextButton = new QPushButton("Go!");

    QGridLayout *pBodeDialogLayout = new QGridLayout;
    pBodeDialogLayout->addWidget(pInputGroupBox, 0, 0, 1, 2);
    pBodeDialogLayout->addWidget(pOutputGroupBox, 1, 0, 1, 2);
    pBodeDialogLayout->addWidget(pCancelButton, 2, 0, 1, 1);
    pBodeDialogLayout->addWidget(pNextButton, 2, 1, 1, 1);

    pCreateBodeDialog->setLayout(pBodeDialogLayout);

    pCreateBodeDialog->show();

    connect(pNextButton, SIGNAL(clicked()), this, SLOT(createBodePlotFromDialog()));
    connect(pNextButton, SIGNAL(clicked()), pCreateBodeDialog, SLOT(close()));
}


void PlotWindow::createBodePlotFromDialog()
{
    PlotCurve *inputCurve;
    PlotCurve *outputCurve;
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
    createBodePlot(inputCurve, outputCurve);
}


void PlotWindow::createBodePlot(PlotCurve *pInputCurve, PlotCurve *pOutputCurve)
{
    //! @todo Make sure data vector is 2^n

    //Create a complex vector
    QVector< std::complex<double> > vInputComplex = realToComplex(pInputCurve->getDataVector());
    QVector< std::complex<double> > vOutputComplex = realToComplex(pOutputCurve->getDataVector());

    //Apply the fourier transform
    FFT(vInputComplex);
    FFT(vOutputComplex);

    //Divide the fourier transform elementwise and take their absolute value
    QVector< std::complex<double> > vTF;
    QVector<double> vBodeGain;
    QVector<double> vBodePhase;

    for(int i=0; i<vInputComplex.size()/2; ++i)
    {
        if(vInputComplex.at(i) == std::complex<double>(0,0))        //Check for division by zero
        {
            vTF.append(vTF[i-1]);    //! @todo This is not a good solution
        }
        else
        {
            vTF.append(vOutputComplex.at(i)/vInputComplex.at(i));
        }
        if(i!=0)
        {
            vBodeGain.append(sqrt(vTF[i].real()*vTF[i].real() + vTF[i].imag()*vTF[i].imag()));
            vBodePhase.append(atan(vTF[i].real()/vTF[i].imag())*180/3.14159265);
        }
    }


    QVector<double> vFrequency;
    double stoptime = pInputCurve->getTimeVector().last();
    for(int i=1; i<vTF.size(); ++i)
    {
        vFrequency.append((i+1)/stoptime);
    }


    addPlotTab();
    PlotCurve *pGainCurve = new PlotCurve(pOutputCurve->getGeneration(), pOutputCurve->getComponentName(), pOutputCurve->getPortName(), pOutputCurve->getDataName(),
                                          pOutputCurve->getDataUnit(), pOutputCurve->getAxisY(), pOutputCurve->getContainerObjectPtr()->getModelFileInfo().filePath(),
                                          getCurrentPlotTab());
    getCurrentPlotTab()->addCurve(pGainCurve);
    pGainCurve->setData(vBodeGain, vFrequency);
    pGainCurve->updatePlotInfoDockVisibility();

    PlotCurve *pPhaseCurve = new PlotCurve(pOutputCurve->getGeneration(), pOutputCurve->getComponentName(), pOutputCurve->getPortName(), pOutputCurve->getDataName(),
                                          pOutputCurve->getDataUnit(), pOutputCurve->getAxisY(), pOutputCurve->getContainerObjectPtr()->getModelFileInfo().filePath(),
                                          getCurrentPlotTab(), true);
    getCurrentPlotTab()->addCurve(pPhaseCurve, true);
    pPhaseCurve->setData(vBodePhase, vFrequency);
    pPhaseCurve->updatePlotInfoDockVisibility();

    getCurrentPlotTab()->showSecondPlot(true);
    getCurrentPlotTab()->getPlot()->replot();

    getCurrentPlotTab()->rescaleToCurves();

    //getCurrentPlotTab()->getPlot()->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
    getCurrentPlotTab()->getPlot()->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
    getCurrentPlotTab()->getSecondPlot()->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);

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
    for(int i=0; i<getCurrentPlotTab()->getCurves().size(); ++i)
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
    QString buttonStyle;
    buttonStyle.append("QToolButton			{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:pressed   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:checked		{ border: 1px solid gray;               border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:checked   	{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
    buttonStyle.append("QToolButton:hover:unchecked   	{ border: 1px solid gray;               border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(" + redString + "," + greenString + "," + blueString + ") } ");
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

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpColorBlob,                0,  0);
    mpLayout->addWidget(mpGenerationLabel,          0,  1);
    mpLayout->addWidget(mpPreviousButton,           0,  2);
    mpLayout->addWidget(mpNextButton,               0,  3);
    mpLayout->addWidget(mpSizeSpinBox,              0,  4);
    mpLayout->addWidget(mpCloseButton,              0,  5);
    mpLayout->addWidget(mpFrequencyAnalysisButton,  1,  1);
    mpLayout->addWidget(mpColorButton,              1,  2);
    mpLayout->addWidget(mpScaleButton,              1,  3);
    mpLayout->addWidget(mpAutoUpdateCheckBox,       1,  4,  1,  2);

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
    this->setTabsClosable(true);

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()));
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
        disconnect(mpParentPlotWindow->mpBackgroundColorButton,     SIGNAL(clicked()),      getTab(i),  SLOT(setBackgroundColor()));
        disconnect(mpParentPlotWindow->mpGridButton,                SIGNAL(toggled(bool)),  getTab(i),  SLOT(enableGrid(bool)));
        disconnect(mpParentPlotWindow->mpResetXVectorButton,        SIGNAL(clicked()),      getTab(i),  SLOT(resetXVector()));
        disconnect(mpParentPlotWindow->mpExportToCsvAction,         SIGNAL(triggered()),    getTab(i),  SLOT(exportToXml()));
        disconnect(mpParentPlotWindow->mpExportToXmlAction,         SIGNAL(triggered()),    getTab(i),  SLOT(exportToCsv()));
        disconnect(mpParentPlotWindow->mpExportToMatlabAction,      SIGNAL(triggered()),    getTab(i),  SLOT(exportToMatlab()));
        disconnect(mpParentPlotWindow->mpExportToGnuplotAction,     SIGNAL(triggered()),    getTab(i),  SLOT(exportToGnuplot()));
        disconnect(mpParentPlotWindow->mpExportPdfAction,           SIGNAL(triggered()),    getTab(i),  SLOT(exportToPdf()));
        disconnect(mpParentPlotWindow->mpExportPngAction,           SIGNAL(triggered()),    getTab(i),  SLOT(exportToPng()));
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

    mpSecondPlot = new QwtPlot();
    mpSecondPlot->setAcceptDrops(false);
    mpSecondPlot->setCanvasBackground(QColor(Qt::white));
    mpSecondPlot->setAutoReplot(true);


        //Panning Tool
    mpPanner = new QwtPlotPanner(mpPlot->canvas());
    mpPanner->setMouseButton(Qt::LeftButton);
    mpPanner->setEnabled(false);

        //Rubber Band Zoom
    mpZoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpPlot->canvas());      //Zoomer for left y axis
    mpZoomer->setMaxStackDepth(10000);
    mpZoomer->setRubberBand(QwtPicker::NoRubberBand);
    mpZoomer->setRubberBandPen(QColor(Qt::green));
    mpZoomer->setTrackerMode(QwtPicker::ActiveOnly);
    mpZoomer->setTrackerPen(QColor(Qt::white));
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    mpZoomer->setZoomBase(QRectF());
    mpZoomer->setEnabled(false);

    mpZoomerRight = new QwtPlotZoomer( QwtPlot::xTop, QwtPlot::yRight, mpPlot->canvas());   //Zoomer for right y axis
    mpZoomerRight->setMaxStackDepth(10000);
    mpZoomerRight->setRubberBand(QwtPicker::NoRubberBand);
    mpZoomerRight->setRubberBandPen(QColor(Qt::green));
    mpZoomerRight->setTrackerMode(QwtPicker::ActiveOnly);
    mpZoomerRight->setTrackerPen(QColor(Qt::white));
    mpZoomerRight->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpZoomerRight->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    mpZoomerRight->setEnabled(false);

        //Wheel Zoom
    mpMagnifier = new QwtPlotMagnifier(mpPlot->canvas());
    mpMagnifier->setAxisEnabled(QwtPlot::yLeft, true);
    mpMagnifier->setAxisEnabled(QwtPlot::yRight, true);
    mpMagnifier->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
    mpMagnifier->setWheelFactor(1.1);
    mpMagnifier->setEnabled(true);

        //Curve Marker Symbol
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setStyle(QwtSymbol::XCross);
    mpMarkerSymbol->setSize(10,10);

    mpGrid = new QwtPlotGrid;
    mpGrid->enableXMin(true);
    mpGrid->enableYMin(true);
    mpGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    mpGrid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    mpGrid->attach(mpPlot);

    mpSecondGrid = new QwtPlotGrid;
    mpSecondGrid->enableXMin(true);
    mpSecondGrid->enableYMin(true);
    mpSecondGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    mpSecondGrid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    mpSecondGrid->attach(mpSecondPlot);

    QwtLegend *tempLegend = new QwtLegend();
    //tempLegend->setPalette(QPalette(QColor("black"), QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("black"), QColor("white"), QColor("white"), QColor("white")));
    tempLegend->setAutoFillBackground(false);

    QList<QWidget *> tempList = tempLegend->findChildren<QWidget *>();
    for(int i=0; i<tempList.size(); ++i)
    {
        tempList.at(i)->setAutoFillBackground(false);
    }
    mpPlot->insertLegend(tempLegend, QwtPlot::TopLegend);
    mpPlot->setAutoFillBackground(false);

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpPlot);
    pLayout->addWidget(mpSecondPlot);
    this->setLayout(pLayout);

    showSecondPlot(false);

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
void PlotTab::addCurve(PlotCurve *curve, bool addToSecondPlot)
{
    if(mHasSpecialXAxis)
    {
        curve->getCurvePtr()->setSamples(mVectorX, curve->getDataVector());
    }

    if(addToSecondPlot)
    {
        mSecondPlotCurvePtrs.append(curve);
    }
    else
    {
        mPlotCurvePtrs.append(curve);
    }

    int i=0;
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
    double xMinSecond, xMaxSecond, yMinLeftSecond, yMaxLeftSecond, yMinRightSecond, yMaxRightSecond;

    xMin=0;
    xMax=10;
    yMinLeft=0;
    yMaxLeft=10;
    yMinRight=0;
    yMaxRight=10;

    xMinSecond=0;
    xMaxSecond=10;
    yMinLeftSecond=0;
    yMaxLeftSecond=10;
    yMinRightSecond=0;
    yMaxRightSecond=10;

    if(!mPlotCurvePtrs.empty())
    {
        bool foundFirstLeft = false;
        bool foundFirstRight = false;

        xMin=mPlotCurvePtrs.first()->getCurvePtr()->minXValue();
        xMax=mPlotCurvePtrs.first()->getCurvePtr()->maxXValue();

        for(int i=0; i<mPlotCurvePtrs.size(); ++i)
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


    if(!mSecondPlotCurvePtrs.empty())
    {
        bool foundFirstLeft = false;
        bool foundFirstRight = false;

        xMinSecond=mSecondPlotCurvePtrs.first()->getCurvePtr()->minXValue();
        xMaxSecond=mSecondPlotCurvePtrs.first()->getCurvePtr()->maxXValue();

        for(int i=0; i<mSecondPlotCurvePtrs.size(); ++i)
        {
            if(mSecondPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yLeft)
            {
                if(foundFirstLeft == false)
                {
                    yMinLeftSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    yMaxLeftSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                    foundFirstLeft = true;
                }
                else
                {
                    if(mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minYValue() < yMinLeftSecond)
                        yMinLeftSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    if(mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue() > yMaxLeftSecond)
                        yMaxLeftSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                }
            }

            if(mSecondPlotCurvePtrs.at(i)->getAxisY() == QwtPlot::yRight)
            {
                if(foundFirstRight == false)
                {
                    yMinRightSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    yMaxRightSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                    foundFirstRight = true;
                }
                else
                {
                    if(mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minYValue() < yMinRightSecond)
                        yMinRightSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minYValue();
                    if(mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue() > yMaxRightSecond)
                        yMaxRightSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxYValue();
                }
            }

            if(mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minXValue() < xMinSecond)
                xMinSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->minXValue();
            if(mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxXValue() > xMaxSecond)
                xMaxSecond=mSecondPlotCurvePtrs.at(i)->getCurvePtr()->maxXValue();

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

    if(yMaxLeftSecond == yMinLeftSecond)
    {
        yMaxLeftSecond = yMaxLeftSecond+1;
        yMinLeftSecond = yMinLeftSecond-1;
    }
    if(yMaxRightSecond == yMinRightSecond)
    {
        yMaxRightSecond = yMaxRightSecond+1;
        yMinRightSecond = yMinRightSecond-1;
    }


    double heightLeft = yMaxLeft-yMinLeft;
    double heightRight = yMaxRight-yMinRight;

    double heightLeftSecond = yMaxLeftSecond-yMinLeftSecond;
    double heightRightSecond = yMaxRightSecond-yMinRightSecond;

    mpPlot->setAxisScale(QwtPlot::yLeft, yMinLeft-0.05*heightLeft, yMaxLeft+0.05*heightLeft);
    mpPlot->setAxisScale(QwtPlot::yRight, yMinRight-0.05*heightRight, yMaxRight+0.05*heightRight);
    mpPlot->setAxisScale(QwtPlot::xBottom, xMin, xMax);

    qDebug() << "yMin = " << yMinLeft << ", yMax = " << yMaxLeft;

    mpSecondPlot->setAxisScale(QwtPlot::yLeft, yMinLeftSecond-0.05*heightLeftSecond, yMaxLeftSecond+0.05*heightLeftSecond);
    mpSecondPlot->setAxisScale(QwtPlot::yRight, yMinRightSecond-0.05*heightRightSecond, yMaxRightSecond+0.05*heightRightSecond);
    mpSecondPlot->setAxisScale(QwtPlot::xBottom, xMinSecond, xMaxSecond);

    QRectF tempDoubleRect;
    tempDoubleRect.setX(xMin);
    tempDoubleRect.setY(yMinLeft-0.05*heightLeft);
    tempDoubleRect.setWidth(xMax-xMin);
    tempDoubleRect.setHeight(yMaxLeft-yMinLeft+0.1*heightLeft);
    mpZoomer->setZoomBase(tempDoubleRect);

    QRectF tempDoubleRect2;
    tempDoubleRect2.setX(xMin);
    tempDoubleRect2.setY(yMinRight-0.05*heightRight);
    tempDoubleRect2.setHeight(yMaxRight-yMinRight+0.1*heightRight);
    tempDoubleRect2.setWidth(xMax-xMin);
    mpZoomerRight->setZoomBase(tempDoubleRect2);

    //! @todo Uncomment this when zoomer is fixed for secondary plot
//    QRectF tempDoubleRect3;
//    tempDoubleRect3.setX(xMinSecond);
//    tempDoubleRect3.setY(yMinSecondLeft-0.05*heightLeftSecond);
//    tempDoubleRect3.setWidth(xMaxSecond-xMinSecond);
//    tempDoubleRect3.setHeight(yMaxSecondLeft-yMinSecondLeft+0.1*heightLeftSecond);
//    mpZoomer->setZoomBase(tempDoubleRect3);

//    QRectF tempDoubleRect4;
//    tempDoubleRect4.setX(xMinSecond);
//    tempDoubleRect4.setY(yMinSecondRight-0.05*heightRightSecond);
//    tempDoubleRect4.setHeight(yMaxSecondRight-yMinSecondRight+0.1*heightRightSecond);
//    tempDoubleRect4.setWidth(xMaxSecond-xMinSecond);
//    mpZoomerRight->setZoomBase(tempDoubleRect4);

            //Curve Marker
    mpMarkerSymbol = new QwtSymbol();
    //mpMarkerSymbol->setPen(QPen(QBrush(Color(Qt::red)), 3));
    mpMarkerSymbol->setStyle(QwtSymbol::XCross);
    mpMarkerSymbol->setSize(10,10);
}


//! @brief Removes a curve from the plot tab
//! @param curve Pointer to curve to remove
void PlotTab::removeCurve(PlotCurve *curve)
{
    for(int i=0; i<mMarkerPtrs.size(); ++i)
    {
        if(mMarkerPtrs.at(i)->getCurve() == curve)
        {
            mpPlot->canvas()->removeEventFilter(mMarkerPtrs.at(i));
            mMarkerPtrs.at(i)->detach();
            //delete(mMarkerPtrs.at(i));
            mMarkerPtrs.removeAt(i);
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
    mPlotCurvePtrs.removeAll(curve);
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
void PlotTab::changeXVector(QVector<double> xArray, QString componentName, QString portName, QString dataName, QString dataUnit)
{
    mVectorX = xArray;

    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        mPlotCurvePtrs.at(i)->getCurvePtr()->setSamples(mVectorX, mPlotCurvePtrs.at(i)->getDataVector());
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
    mHasSpecialXAxis = true;
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(true);
}


void PlotTab::updateLabels()
{
    mpPlot->setAxisTitle(QwtPlot::yLeft, QwtText());
    mpPlot->setAxisTitle(QwtPlot::yRight, QwtText());

    QStringList leftUnits, rightUnits;
    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
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


bool PlotTab::isGridVisible()
{
    return mpGrid->isVisible();
}


void PlotTab::resetXVector()
{
    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        mPlotCurvePtrs.at(i)->getCurvePtr()->setSamples(mPlotCurvePtrs.at(i)->getTimeVector(), mPlotCurvePtrs.at(i)->getDataVector());
    }

    mVectorXLabel = QString("Time [s]");
    updateLabels();

    mHasSpecialXAxis = false;

    rescaleToCurves();
    mpPlot->replot();

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
                                            fileDialogSaveDir.currentPath(),
                                            tr("Comma-separated values (*.csv)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

        //Cycle plot curves
    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        fileStream << "x" << i;                                         //Write time/X vector
        if(mHasSpecialXAxis)
        {
            for(int j=0; j<mVectorX.size(); ++j)
            {
                fileStream << "," << mVectorX[j];
            }
        }
        else
        {
            for(int j=0; j<mPlotCurvePtrs[i]->getTimeVector().size(); ++j)
            {
                fileStream << "," << mPlotCurvePtrs[i]->getTimeVector()[j];
            }
        }
        fileStream << "\n";

        fileStream << "y" << i;                                             //Write data vector
        for(int k=0; k<mPlotCurvePtrs[i]->getDataVector().size(); ++k)
        {
            fileStream << "," << mPlotCurvePtrs[i]->getDataVector()[k];
        }
        fileStream << "\n";
    }

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
                                            fileDialogSaveDir.currentPath(),
                                            tr("MATLAB script file (*.m)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
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
    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
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
            for(int j=0; j<mPlotCurvePtrs[i]->getTimeVector().size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mPlotCurvePtrs[i]->getTimeVector()[j];
            }
        }
        fileStream << "]\n";

        fileStream << "y" << i << "=[";                                             //Write data vector
        for(int k=0; k<mPlotCurvePtrs[i]->getDataVector().size(); ++k)
        {
            if(k>0) fileStream << ",";
            fileStream << mPlotCurvePtrs[i]->getDataVector()[k];
        }
        fileStream << "]\n";
    }

        //Write plot functions
    QStringList matlabColors;
    matlabColors << "r" << "g" << "b" << "c" << "m" << "y";
    fileStream << "hold on\n";
    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        fileStream << "plot(x" << i << ",y" << i << ",'-" << matlabColors[i%6] << "','linewidth'," << mPlotCurvePtrs[i]->getCurvePtr()->pen().width() << ")\n";
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
                                            fileDialogSaveDir.currentPath(),
                                            tr("gnuplot file (*.dat)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
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
    for(int i=0; i<mPlotCurvePtrs.size(); ++i)
    {
        fileStream << "                  Y" << i;
    }
    fileStream << "\n";

        //Write time and data vectors
    QString dummy;
    for(int i=0; i<mPlotCurvePtrs.first()->getTimeVector().size(); ++i)
    {
        dummy.setNum(mPlotCurvePtrs.first()->getTimeVector()[i]);
        fileStream << dummy;
        for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }

        for(int k=0; k<mPlotCurvePtrs.size(); ++k)
        {
            dummy.setNum(mPlotCurvePtrs[k]->getDataVector()[i]);
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
     QString fileName = QFileDialog::getSaveFileName(this, "Export File Name", QString(), "Portable Document Format (*.pdf)");
    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;

        QPrinter *printer = new QPrinter(QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A4);
        printer->setOrientation(QPrinter::Landscape);
        printer->setFullPage(false);
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(fileName);
        renderer.renderTo(mpPlot,*printer);
    }
}


//! @brief Slot that exports plot tab as bitmap to specified .png file
void PlotTab::exportToPng()
{
    QString fileName = QFileDialog::getSaveFileName(
       this, "Export File Name", QString(),
       "Portable Network Graphics (*.png)");

    QPixmap pixmap(mpPlot->width(), mpPlot->height());
    pixmap.fill();
    QwtPlotRenderer renderer;
    renderer.renderTo(mpPlot, pixmap);

    pixmap.save( fileName );
}


void PlotTab::enableZoom(bool value)
{
    if(mpParentPlotWindow->mpPanButton->isChecked() && value)
    {
        mpParentPlotWindow->mpPanButton->setChecked(false);
        mpPanner->setEnabled(false);
    }
    mpZoomer->setEnabled(value);
    if(value)   { mpZoomer->setRubberBand(QwtPicker::RectRubberBand); }
    else        { mpZoomer->setRubberBand(QwtPicker::NoRubberBand); }
    mpZoomerRight->setEnabled(value);
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


void PlotTab::enablePan(bool value)
{
    if(mpParentPlotWindow->mpZoomButton->isChecked() && value)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        mpZoomer->setEnabled(false);
        mpZoomer->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight->setEnabled(false);
    }
    mpPanner->setEnabled(value);
}


void PlotTab::enableGrid(bool value)
{
    mpGrid->setVisible(value);
    mpSecondGrid->setVisible(value);
}


void PlotTab::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(mpPlot->canvasBackground().color(), this);
    if (color.isValid())
    {
        mpPlot->setCanvasBackground(color);
        mpPlot->replot();
        mpSecondPlot->setCanvasBackground(color);
        mpSecondPlot->replot();
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


QwtPlot *PlotTab::getSecondPlot()
{
    return mpSecondPlot;
}


void PlotTab::showSecondPlot(bool visible)
{
    mpSecondPlot->setVisible(visible);
}


int PlotTab::getNumberOfCurves()
{
    return mPlotCurvePtrs.size();
}


void PlotTab::update()
{
    mpPlot->enableAxis(QwtPlot::yLeft, false);
    mpPlot->enableAxis(QwtPlot::yRight, false);
    QList<PlotCurve *>::iterator cit;
    for(cit=mPlotCurvePtrs.begin(); cit!=mPlotCurvePtrs.end(); ++cit)
    {
        if(!mpPlot->axisEnabled((*cit)->getAxisY())) { mpPlot->enableAxis((*cit)->getAxisY()); }
        (*cit)->getCurvePtr()->attach(mpPlot);
    }

    for(int i=0; i<mMarkerPtrs.size(); ++i)
    {
        QPointF posF = mMarkerPtrs.at(i)->value();
        double x = mpPlot->transform(QwtPlot::xBottom, posF.x());
        double y = mpPlot->transform(QwtPlot::yLeft, posF.y());
        QPoint pos = QPoint(x,y);
        QwtPlotCurve *pCurve = mMarkerPtrs.at(i)->getCurve()->getCurvePtr();
        mMarkerPtrs.at(i)->setXValue(pCurve->sample(pCurve->closestPoint(pos)).x());
        mMarkerPtrs.at(i)->setYValue(mpPlot->invTransform(QwtPlot::yLeft, mpPlot->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));
    }
    mpPlot->replot();
}


//! @brief Inserts a curve marker at the specified curve
//! @param curve is a pointer to the specified curve
void PlotTab::insertMarker(PlotCurve *pCurve, QPoint pos)
{
    qDebug() << "Inserting curve marker for " << pCurve->getComponentName() << ", " << pCurve->getPortName() << ", " << pCurve->getDataName();

    mpMarkerSymbol->setPen(QPen(pCurve->getCurvePtr()->pen().brush().color(), 3));
    PlotMarker *tempMarker = new PlotMarker(pCurve, this, *mpMarkerSymbol);
    mMarkerPtrs.append(tempMarker);

    tempMarker->attach(mpPlot);
    QCursor cursor;
    tempMarker->setXValue(pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(pos)).x());
    tempMarker->setYValue(mpPlot->invTransform(QwtPlot::yLeft, mpPlot->transform(pCurve->getCurvePtr()->yAxis(), pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(pos)).y())));

    QString xString;
    QString yString;
    double x = pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(pos)).x();
    double y = pCurve->getCurvePtr()->sample(pCurve->getCurvePtr()->closestPoint(mpPlot->canvas()->mapFromGlobal(cursor.pos()))).y();
    xString.setNum(x);
    yString.setNum(y);
    QwtText tempLabel;
    tempLabel.setText("("+xString+", "+yString+")");
    tempLabel.setText("("+xString+", "+yString+")");
    tempLabel.setColor(pCurve->getCurvePtr()->pen().brush().color());
    tempLabel.setBackgroundBrush(QColor(255,255,255,220));
    tempLabel.setFont(QFont("Calibri", 12, QFont::Normal));
    tempMarker->setLabel(tempLabel);
    tempMarker->setLabelAlignment(Qt::AlignTop);

    mpPlot->canvas()->installEventFilter(tempMarker);
    mpPlot->canvas()->setMouseTracking(true);
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

        //Cycle plot curves and write data tags
    for(int j=0; j<mPlotCurvePtrs[0]->getTimeVector().size(); ++j)
    {
        QDomElement dataTag = appendDomElement(rDomElement, "data");

        if(mHasSpecialXAxis)        //Special x-axis, replace time with x-data
        {
            dataTag.setAttribute(mVectorXDataName, mVectorX[j]);
        }
        else                        //X-axis = time
        {
            dataTag.setAttribute("time", mPlotCurvePtrs[0]->getTimeVector()[j]);
        }

        //Write variable tags for each variable
        for(int i=0; i<mPlotCurvePtrs.size(); ++i)
        {
            QString numTemp;
            numTemp.setNum(i);
            QDomElement varTag = appendDomElement(dataTag, mPlotCurvePtrs[i]->getDataName()+numTemp);
            QString valueString;
            valueString.setNum(mPlotCurvePtrs[i]->getDataVector()[j]);
            QDomText value = varTag.ownerDocument().createTextNode(valueString);
            varTag.appendChild(value);

            if(descriptions)
            {
                varTag.setAttribute("component", mPlotCurvePtrs[i]->getComponentName());
                varTag.setAttribute("port", mPlotCurvePtrs[i]->getPortName());
                varTag.setAttribute("type", mPlotCurvePtrs[i]->getDataName());
                varTag.setAttribute("unit", mPlotCurvePtrs[i]->getDataUnit());
            }
        }
    }
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
    for(int i=0; i<10; ++i)
    {
        display.append(lines[i]);
        display.append("\n");
    }
    for(int k=0; k<mpXmlIndentationSpinBox->value(); ++k) display.append(" ");
    display.append("...\n");
    display.append(lines[lines.size()-2]);
    display.append(lines[lines.size()-1]);

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

    qDebug() << display;
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
                                            fileDialogSaveDir.currentPath(),
                                            tr("Extensible Markup Language (*.xml)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
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
    QMenu *insertMarkerMenu;

    QAction *setRightAxisLogarithmic = 0;
    QAction *setLeftAxisLogarithmic = 0;


    yAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    yAxisRightMenu = menu.addMenu(QString("Right Y Axis"));

    yAxisLeftMenu->setEnabled(mpPlot->axisEnabled(QwtPlot::yLeft));
    yAxisRightMenu->setEnabled(mpPlot->axisEnabled(QwtPlot::yRight));

        //Create menu and actions for changing units
    changeUnitsMenu = menu.addMenu(QString("Change Units"));
    QMap<QAction *, PlotCurve *> actionToCurveMap;
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
            actionToCurveMap.insert(pTempAction, (*itc));
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


        //Create menu for insereting curve markers
    insertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    for(itc=mPlotCurvePtrs.begin(); itc!=mPlotCurvePtrs.end(); ++itc)
    {
        QAction *pTempAction = insertMarkerMenu->addAction(QString((*itc)->getComponentName() + ", " + (*itc)->getPortName() + ", " + (*itc)->getDataName()));
        actionToCurveMap.insert(pTempAction, (*itc));
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
PlotCurve::PlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY, QString modelPath, PlotTab *parent, bool addToSecondPlot)
{
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
            if(gpMainWindow->mpProjectTabs->getTab(i)->getSystem()->getModelFileInfo().filePath() == modelPath)
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
    mTimeVector = mpContainerObject->getTimeVector(generation);

        //Create the actual curve
    mpCurve = new QwtPlotCurve(QString(mComponentName+", "+mPortName+", "+mDataName));
    updateCurve();
    mpCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpCurve->setYAxis(axisY);
    if(addToSecondPlot)
    {
        mpCurve->attach(parent->getSecondPlot());
    }
    else
    {
        mpCurve->attach(parent->getPlot());
    }
    qDebug() << "1";
        //Create the plot info box
    mpPlotInfoBox = new PlotInfoBox(this, mpParentPlotTab);
            qDebug() << "2";
    updatePlotInfoBox();
            qDebug() << "3";
    mpPlotInfoBox->mpSizeSpinBox->setValue(2);

    mpPlotInfoDockWidget = new QDockWidget(mComponentName+", "+mPortName+", "+mDataName+" ["+mDataUnit+"]", mpParentPlotTab->mpParentPlotWindow);
    mpPlotInfoDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    mpPlotInfoDockWidget->setMaximumHeight(100);
    mpPlotInfoDockWidget->setWidget(mpPlotInfoBox);
    mpPlotInfoDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mpPlotInfoDockWidget->setMinimumWidth(mpPlotInfoDockWidget->windowTitle().length()*6);
    mpPlotInfoDockWidget->hide();

        //Create connections
    connect(mpPlotInfoBox->mpSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setLineWidth(int)));
    connect(mpPlotInfoBox->mpColorButton, SIGNAL(clicked()), this, SLOT(setLineColor()));
    connect(mpPlotInfoBox->mpScaleButton, SIGNAL(clicked()), this, SLOT(openScaleDialog()));
    connect(mpParentPlotTab->mpParentPlotWindow->getPlotTabWidget(), SIGNAL(currentChanged(int)), this, SLOT(updatePlotInfoDockVisibility()));
    connect(mpParentPlotTab->mpParentPlotWindow->mpShowCurvesButton, SIGNAL(toggled(bool)), SLOT(updatePlotInfoDockVisibility()));
    connect(mpPlotInfoBox->mpCloseButton, SIGNAL(clicked()), this, SLOT(removeMe()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));
    connect(mpContainerObject, SIGNAL(objectDeleted()), this, SLOT(removeMe()));
    connect(mpContainerObject->getGUIModelObject(mComponentName), SIGNAL(objectDeleted()), this, SLOT(removeMe()));
    connect(mpContainerObject->getGUIModelObject(mComponentName), SIGNAL(nameChanged()), this, SLOT(removeMe()));
    connect(mpContainerObject, SIGNAL(connectorRemoved()), this, SLOT(removeIfNotConnected()));
}


//! @brief Destructor for plot curves
//! Deletes the info box and its dock widgets before the curve is removed.
PlotCurve::~PlotCurve()
{
    mpContainerObject->decrementOpenPlotCurves();
    mpPlotInfoDockWidget->hide();
    delete(mpPlotInfoBox);
    delete(mpPlotInfoDockWidget);
}


//! @brief Returns the current generation a plot curve is representing
int PlotCurve::getGeneration()
{
    return mGeneration;
}


//! @brief Returns a pointer to the actual Qwt curve in a plot curve object
QwtPlotCurve *PlotCurve::getCurvePtr()
{
    return mpCurve;
}


//! @brief Returns a pointer to the plot info dock of a plot curve
QDockWidget *PlotCurve::getPlotInfoDockWidget()
{
    return mpPlotInfoDockWidget;
}


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
GUIContainerObject *PlotCurve::getContainerObjectPtr()
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
        mTimeVector = mpContainerObject->getTimeVector(mGeneration);
    else
        mTimeVector = mpParentPlotTab->mVectorX;

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
    for(int i=0; i<n/2; ++i)        //FFT is symmetric, so only use first half
    {
        //! @todo Should we compensate for only using haft the vector, to maintain energy density?
        mDataVector.append(real(vComplex[i]*conj(vComplex[i]))/n);
    }

    //Create the x vector (frequency)
    double max = mTimeVector.last();
    mTimeVector.clear();
    for(int i=0; i<n/2; ++i)
    {
        mTimeVector.append(double(i)/max);
    }

    mDataName = "Value";
    mDataUnit = "-";

    updateCurve();
    mpParentPlotTab->changeXVector(mTimeVector, "", "", "Frequency", "Hz");
    mpParentPlotTab->update();
    updatePlotInfoBox();

    mpPlotInfoBox->mpNextButton->setDisabled(true);
    mpPlotInfoBox->mpPreviousButton->setDisabled(true);
    mpPlotInfoBox->mpFrequencyAnalysisButton->setDisabled(true);
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


//! @brief Tells the parent plot tab of a curve to remove it
void PlotCurve::removeMe()
{
    mpParentPlotTab->removeCurve(this);
}


//! @brief Slot that checks that the plotted port is still connected, and removes the curve if not
void PlotCurve::removeIfNotConnected()
{
    if(!mpContainerObject->getGUIModelObject(mComponentName)->getPort(mPortName)->isConnected())
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
    mpPlotInfoBox->mpPreviousButton->setEnabled(mGeneration > 0 && mpContainerObject->getNumberOfPlotGenerations() > 1);
    mpPlotInfoBox->mpNextButton->setEnabled(mGeneration < mpContainerObject->getNumberOfPlotGenerations()-1 && mpContainerObject->getNumberOfPlotGenerations() > 1);

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
        for(int i=0; i<mTimeVector.size(); ++i)
        {
            tempX.append(mpParentPlotTab->mVectorX[i]*mScaleX + mOffsetX);
            tempY.append(mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    else
    {
        for(int i=0; i<mTimeVector.size(); ++i)
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
}


//! @brief Event filter for plot markers
//! This will interrupt events from plot canvas, to enable using mouse and key events for modifying markers.
//! @returns True if event was interrupted, false if its propagation shall continue
//! @param object Pointer to the object the event belongs to (in this case the plot canvas)
//! @param ev ent Event to be interrupted
bool PlotMarker::eventFilter(QObject *object, QEvent *event)
{

        // Key press events, used to initiate moving of a marker if mouse cursor is close enough
    if (event->type() == QEvent::MouseButtonPress)
    {
        qDebug() << "mousePressEvent()";
        QCursor cursor;
        QPointF midPoint;
        midPoint.setX(mpPlotTab->getPlot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(mpPlotTab->getPlot()->transform(QwtPlot::yLeft, value().y()));

        if(!mpPlotTab->mpZoomer->isEnabled() && !mpPlotTab->mpPanner->isEnabled())
        {
            if((mpPlotTab->getPlot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
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
        midPoint.setX(mpPlotTab->getPlot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(mpPlotTab->getPlot()->transform(QwtPlot::yLeft, value().y()));
        if((mpPlotTab->getPlot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
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
            double x = mpCurve->getCurvePtr()->sample(mpCurve->getCurvePtr()->closestPoint(mpPlotTab->getPlot()->canvas()->mapFromGlobal(cursor.pos()))).x();
            double y = mpCurve->getCurvePtr()->sample(mpCurve->getCurvePtr()->closestPoint(mpPlotTab->getPlot()->canvas()->mapFromGlobal(cursor.pos()))).y();
            setXValue(x);
            setYValue(mpPlotTab->getPlot()->invTransform(QwtPlot::yLeft, mpPlotTab->getPlot()->transform(mpCurve->getCurvePtr()->yAxis(), y)));

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
            midPoint.setX(mpPlotTab->getPlot()->transform(QwtPlot::xBottom, value().x()));
            midPoint.setY(mpPlotTab->getPlot()->transform(mpCurve->getCurvePtr()->yAxis(), value().y()));
            if((mpPlotTab->getPlot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
            {
                plot()->canvas()->removeEventFilter(this);
                mpPlotTab->mMarkerPtrs.removeAll(this);
                this->hide();           // This will only hide and inactivate the marker. Deleting it seem to make program crash.
                this->detach();
                return true;
            }
        }
        return false;
    }
    return false;
}


//! @brief Returns a pointer to the curve a plot marker belongs to
PlotCurve *PlotMarker::getCurve()
{
    return mpCurve;
}
