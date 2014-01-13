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
//! @file   PlotTab.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013
//!
//! @brief Contains a class for plot tabs
//!
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

//Other includes
#include <cmath>


//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "GUIObjects/GUIContainerObject.h"
#include "ModelHandler.h"
#include "PlotCurve.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Utilities/GUIUtilities.h"
#include "version_gui.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "PlotArea.h"

#include "qwt_plot_renderer.h"
#include "Dependencies/BarChartPlotter/axisbase.h"



const double in2mm = 25.4;

//! @brief Constructor for plot tabs.
//! @param parent Pointer to the plot window the tab belongs to
PlotTab::PlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow)
    : QWidget(pParentPlotTabWidget)
{
    mpParentPlotTabWidget = pParentPlotTabWidget;
    mpParentPlotWindow = pParentPlotWindow;
//    setAcceptDrops(true);
//    setMouseTracking(true);
//    mHasCustomXData=false;
//    mLeftAxisLogarithmic = false;
//    mRightAxisLogarithmic = false;
//    mBottomAxisLogarithmic = false;


//    mCurveColors << "Blue" << "Red" << "Green" << "Orange" << "Pink" << "Brown" << "Purple" << "Gray";
//    for(int i=0; i<mCurveColors.size(); ++i)
//    {
//        mUsedColorsCounter.append(0);
//    }

//    for(int plotID=0; plotID<2; ++plotID)
//    {
//        // Plots
//        mpQwtPlots[plotID] = new HopQwtPlot(this);
//        mpQwtPlots[plotID]->setMouseTracking(true);
//        mpQwtPlots[plotID]->setAcceptDrops(false);
//        mpQwtPlots[plotID]->setCanvasBackground(QColor(Qt::white));
//        mpQwtPlots[plotID]->setAutoReplot(true);

//        // Panning Tool
//        mpPanner[plotID] = new QwtPlotPanner(mpQwtPlots[plotID]->canvas());
//        mpPanner[plotID]->setMouseButton(Qt::LeftButton);
//        mpPanner[plotID]->setEnabled(false);

//        // Rubber Band Zoom
//        QPen rubberBandPen(Qt::green);
//        rubberBandPen.setWidth(2);

//        mpZoomerLeft[plotID] = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpQwtPlots[plotID]->canvas());      //Zoomer for left y axis
//        mpZoomerLeft[plotID]->setMaxStackDepth(10000);
//        mpZoomerLeft[plotID]->setRubberBand(QwtPicker::NoRubberBand);
//        mpZoomerLeft[plotID]->setRubberBandPen(rubberBandPen);
//        mpZoomerLeft[plotID]->setTrackerMode(QwtPicker::AlwaysOff);
//        mpZoomerLeft[plotID]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
//        mpZoomerLeft[plotID]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
//        mpZoomerLeft[plotID]->setEnabled(false);

//        mpZoomerRight[plotID] = new QwtPlotZoomer( QwtPlot::xTop, QwtPlot::yRight, mpQwtPlots[plotID]->canvas());   //Zoomer for right y axis
//        mpZoomerRight[plotID]->setMaxStackDepth(10000);
//        mpZoomerRight[plotID]->setRubberBand(QwtPicker::NoRubberBand);
//        mpZoomerRight[plotID]->setRubberBandPen(rubberBandPen);
//        mpZoomerRight[plotID]->setTrackerMode(QwtPicker::AlwaysOff);
//        mpZoomerRight[plotID]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
//        mpZoomerRight[plotID]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
//        mpZoomerRight[plotID]->setEnabled(false);

//        // Wheel Zoom
//        mpMagnifier[plotID] = new QwtPlotMagnifier(mpQwtPlots[plotID]->canvas());
//        mpMagnifier[plotID]->setAxisEnabled(QwtPlot::yLeft, true);
//        mpMagnifier[plotID]->setAxisEnabled(QwtPlot::yRight, true);
//        mpMagnifier[plotID]->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
//        mpMagnifier[plotID]->setWheelFactor(1.1);
//        mpMagnifier[plotID]->setMouseButton(Qt::NoButton, Qt::NoModifier);
//        mpMagnifier[plotID]->setEnabled(true);

//        // Grid
//        mpGrid[plotID] = new QwtPlotGrid;
//        mpGrid[plotID]->enableXMin(true);
//        mpGrid[plotID]->enableYMin(true);
//        mpGrid[plotID]->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
//        mpGrid[plotID]->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
//        mpGrid[plotID]->setZ(GridLinesZOrderType);
//        mpGrid[plotID]->attach(mpQwtPlots[plotID]);

//        // Init curve counters
//        mNumYlCurves[plotID] = 0;
//        mNumYrCurves[plotID] = 0;
//    }

//    // Attach lock boxes to first plot
//    mpXLockCheckBox = new QCheckBox(mpQwtPlots[0]->axisWidget(QwtPlot::xBottom));
//    mpXLockCheckBox->setCheckable(true);
//    mpXLockCheckBox->setToolTip("Lock the x-axis");
//    mpYLLockCheckBox = new QCheckBox(mpQwtPlots[0]->axisWidget(QwtPlot::yLeft));
//    mpYLLockCheckBox->setCheckable(true);
//    mpYLLockCheckBox->setToolTip("Lock the left y-axis");
//    mpYRLockCheckBox = new QCheckBox(mpQwtPlots[0]->axisWidget(QwtPlot::yRight));
//    mpYRLockCheckBox->setCheckable(true);
//    mpYRLockCheckBox->setToolTip("Lock the right y-axis");
//    connect(mpXLockCheckBox, SIGNAL(toggled(bool)), this, SLOT(axisLockHandler()));
//    connect(mpYLLockCheckBox, SIGNAL(toggled(bool)), this, SLOT(axisLockHandler()));
//    connect(mpYRLockCheckBox, SIGNAL(toggled(bool)), this, SLOT(axisLockHandler()));

//    // Connect the refresh signal for repositioning the lock boxes
//    connect(mpQwtPlots[0], SIGNAL(afterReplot()), this, SLOT(refreshLockCheckBoxPositions()));




//    // Legend Stuff
//    constructLegendSettingsDialog();

//    mpRightPlotLegend = new PlotLegend(QwtPlot::yRight);
//    mpRightPlotLegend->attach(this->getQwtPlot());
//    mpRightPlotLegend->setAlignment(Qt::AlignRight | Qt::AlignTop);
//    mpRightPlotLegend->setZ(LegendBelowCurveZOrderType);

//    mpLeftPlotLegend = new PlotLegend(QwtPlot::yLeft);
//    mpLeftPlotLegend->attach(this->getQwtPlot());
//    mpLeftPlotLegend->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    mpLeftPlotLegend->setZ(LegendBelowCurveZOrderType);


//    // Create the lock axis dialog
//    constructAxisSettingsDialog();
//    constructAxisLabelDialog();

    mpTabLayout = new QGridLayout(this);

    // Create the scroll area and widget/layout for curve info boxes
    QVBoxLayout *pCurveInfoLayout = new QVBoxLayout();
    pCurveInfoLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pCurveInfoLayout->setSpacing(1);
    pCurveInfoLayout->setMargin(1);
    QWidget *pCurveInfoWidget = new QWidget(this);
    pCurveInfoWidget->setAutoFillBackground(true);
    pCurveInfoWidget->setPalette(gpConfig->getPalette());
    pCurveInfoWidget->setLayout(pCurveInfoLayout);
    mpCurveInfoScrollArea = new QScrollArea(this);
    mpCurveInfoScrollArea->setWidget(pCurveInfoWidget);
    mpCurveInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpCurveInfoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpCurveInfoScrollArea->setPalette(gpConfig->getPalette());
    mpCurveInfoScrollArea->setMinimumHeight(110);
    mpParentPlotWindow->mpCurveInfoStack->addWidget(mpCurveInfoScrollArea);

//    for(int plotID=0; plotID<2; ++plotID)
//    {
//        mpQwtPlots[plotID]->setAutoFillBackground(true);
//        mpQwtPlots[plotID]->setPalette(gpConfig->getPalette());
//        mpTabLayout->addWidget(mpQwtPlots[plotID]);
//    }
    addPlotArea();

//    mpPainterWidget = new PainterWidget(this);
//    mpPainterWidget->clearRect();
//    mpTabLayout->addWidget(mpPainterWidget, 0, 0);

//    for(int plotID=1; plotID<2; ++plotID)       //Hide all plots except first one by default
//    {
//        showPlot(int(plotID), false);
//    }

//    mpQwtPlots[0]->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}


//! @brief Destructor for plot tab
PlotTab::~PlotTab()
{
    // Removes all areas before tab is deleted.
    while(!mPlotAreas.empty())
    {
        removePlotArea(0);
    }
}

void PlotTab::applyLegendSettings()
{

}

void PlotTab::applyTimeScalingSettings()
{

}


//! @todo currently only supports settings axis for top plot
void PlotTab::openAxisSettingsDialog()
{

}

void PlotTab::openAxisLabelDialog()
{

}

void PlotTab::openTimeScalingDialog()
{

}

//! @todo currently only supports settings axis for top plot
void PlotTab::applyAxisSettings()
{

}

void PlotTab::applyAxisLabelSettings()
{

}

//! @brief Toggles the axis lock on/off for the enabled axis
void PlotTab::toggleAxisLock()
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->toggleAxisLock();
    }
}

void PlotTab::openLegendSettingsDialog()
{
    //! @todo solve in some smarter way
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->openLegendSettingsDialog();
    }
}


void PlotTab::setTabName(QString name)
{
    mpParentPlotTabWidget->setTabText(mpParentPlotTabWidget->indexOf(this), name);
}

PlotTabTypeT PlotTab::getPlotTabType() const
{
    return XYPlotType;
}

PlotArea *PlotTab::getPlotArea(const int subPlotId)
{
    if (subPlotId < mPlotAreas.size())
    {
        return mPlotAreas[subPlotId];
    }
    return 0;
}


void PlotTab::addBarChart(QStandardItemModel *pItemModel)
{
    // Do nothing by default, should only be implemented in barchart plots
}



//! @brief Rescales the axes and the zoomers so that all plot curves will fit
void PlotTab::rescaleAxesToCurves()
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->rescaleAxesToCurves();
    }
}


//! @brief Removes a curve from the plot tab
//! @param curve Pointer to curve to remove
void PlotTab::removeCurve(PlotCurve *curve)
{
    //! @todo what if curve added multiple times, then maybe remove until no longer found
    int plotID = getPlotIDForCurve(curve);
    if (plotID >= 0)
    {
        mPlotAreas[plotID]->removeCurve(curve);
    }
}

void PlotTab::removeAllCurvesOnAxis(const int axis)
{
    QList<PlotCurve*> curvePtrs = getCurves();
    for(int c=0; c<curvePtrs.size(); ++c)
    {
        if(curvePtrs[c]->getAxisY() == axis)
        {
            removeCurve(curvePtrs.at(c));
        }
    }
}


//! @brief Changes the X vector of current plot tab to specified variable
//! @param xArray New data for X-axis
//! @param componentName Name of component from which new data origins
//! @param portName Name of port form which new data origins
//! @param dataName Data name (physical quantity) of new data
//! @param dataUnit Unit of new data
void PlotTab::setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc, int plotID)
{
    if (plotID < mPlotAreas.size())
    {
        mPlotAreas[plotID]->setCustomXVectorForAll(xArray,rVarDesc);
    }
}

void PlotTab::setCustomXVectorForAll(SharedVariablePtrT pData, int plotID)
{
    if (plotID < mPlotAreas.size())
    {
        mPlotAreas[plotID]->setCustomXVectorForAll(pData);
    }
}


//! @brief Updates labels on plot axes
void PlotTab::updateAxisLabels()
{
    for(int plotID=0; plotID<2; ++plotID)
    {

    }
}

bool PlotTab::isGridVisible() const
{
    return mPlotAreas.first()->isGridVisible();
}


void PlotTab::resetXTimeVector()
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        if (pArea->hasCustomXData())
        {
            pArea->resetXTimeVector();
        }
    }
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


//! @brief Slot that opens a dialog from where user can export current plot tab to a XML file
void PlotTab::exportToXml()
{

    //Open a dialog where text and font can be selected
    mpExportXmlDialog = new QDialog(this);
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


//! @brief Slot that exports plot tab to a specified comma-separated values file (.csv)
void PlotTab::exportToCsv()
{
    //Open file dialog and initialize the file stream
    QString filePath;
    QFileInfo fileInfo;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To CSV File"),
                                            gpConfig->getPlotDataDir(),
                                            tr("Comma-separated values files (*.csv)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());

    exportToCsv(filePath);
}


//! @brief Exports plot tab to comma-separated value file with specified filename
//! @param fileName File name
void PlotTab::exportToCsv(QString fileName)
{
    QFile file;
    file.setFileName(fileName);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + fileName);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    //! @todo this function should use export functions in the log data handler instead
    // Cycle plot curves
    //! @todo this seems to write column ordered data, we need to support column or row ordered data
    if (getPlotTabType() != XYPlotType)
    {
        gpTerminalWidget->mpConsole->printWarningMessage("Will only export from first sub-plot");
    }
    //! @todo make sure that csv can export from multiple sub plots (but how)

    PlotArea *pPlotArea = mPlotAreas.first();
    QList<PlotCurve*> curves = pPlotArea->getCurves();

    QString dummy;
    if(pPlotArea->hasCustomXData())
    {
        //! @todo how to handle this with multiple xvectors per curve
        //! @todo take into account wheter cached or not, Should have some smart auto function for this in the data object

        QVector<double> xvec = pPlotArea->getCustomXData()->getDataVectorCopy(); //! @todo shoudl direct access if not in cache
        for(int i=0; i<xvec.size(); ++i)
        {
            fileStream << xvec[i];
            for(int j=0; j<curves.size(); ++j)
            {
                fileStream << ", " << curves[j]->getLogDataVariablePtr()->peekData(i,dummy);
            }
            fileStream << "\n";
        }
    }
    else
    {
        QVector<double> time = curves.first()->getTimeVectorPtr()->getDataVectorCopy();
        for(int i=0; i<time.size(); ++i)
        {
            fileStream << time[i];
            for(int j=0; j<curves.size(); ++j)
            {
                fileStream << ", " << curves[j]->getLogDataVariablePtr()->peekData(i,dummy);
            }
            fileStream << "\n";
        }
    }

    file.close();
}

void PlotTab::exportToHvc(QString fileName)
{
    // Can not use other than xy plot for hvc data
    if (getPlotTabType() != XYPlotType)
    {
        return;
    }
    // Abort if no curves
    if (mPlotAreas.first()->getNumberOfCurves() == 0)
    {
        return;
    }

    QFileInfo fileInfo;
    if (fileName.isEmpty())
    {
        // Open file dialog and initialize the file stream
        QString filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To CSV File"),
                                                        gpConfig->getPlotDataDir(),
                                                        tr("HopsanValidationCfg (*.hvc)"));
        if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
        fileInfo.setFile(filePath);
    }

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + fileName);
        return;
    }

    // Save the csv data
    QString csvFileName=fileInfo.baseName()+".csv";
    this->exportToCsv(fileInfo.absolutePath()+"/"+csvFileName);

    qDebug() << fileInfo.absoluteFilePath();
    qDebug() << fileInfo.absolutePath()+"/"+csvFileName;


    // Save HVC xml data
    QDomDocument doc;
    QDomElement hvcroot = doc.createElement("hopsanvalidationconfiguration");
    doc.appendChild(hvcroot);
    hvcroot.setAttribute("hvcversion", "0.1");

    QList<PlotCurve*> curves = mPlotAreas.first()->getCurves();
    QString modelPath = relativePath(curves.first()->getLogDataVariablePtr()->getLogDataHandler()->getParentContainerObject()->getModelFileInfo(), QDir(fileInfo.absolutePath()));
    QDomElement validation = appendDomElement(hvcroot, "validation");
    validation.setAttribute("date", QDateTime::currentDateTime().toString("yyyyMMdd"));
    validation.setAttribute("time", QDateTime::currentDateTime().toString("hhmmss"));
    validation.setAttribute("hopsanguiversion", HOPSANGUIVERSION);
    validation.setAttribute("hopsancoreversion", gHopsanCoreVersion);
    appendDomTextNode(validation, "modelfile", modelPath);
    appendDomTextNode(validation, "parameterset", "");

    // Cycle plot curves
    for (int i=0; i<curves.size(); ++i)
    {
        QDomElement component = appendDomElement(validation, "component");
        component.setAttribute("name", curves[i]->getComponentName());

        QDomElement port = appendDomElement(component, "port");
        port.setAttribute("name", curves[i]->getPortName());

        QDomElement variable = appendDomElement(port, "variable");
        variable.setAttribute("name", curves[i]->getDataName());

        appendDomTextNode(variable, "csvfile", csvFileName);
        appendDomIntegerNode(variable, "column", i+1);

        appendDomValueNode(variable, "tolerance", 0.01);
    }

    QTextStream hvcFileStream(&file);
    appendRootXMLProcessingInstruction(doc); //The xml "comment" on the first line
    doc.save(hvcFileStream, 2);
    file.close();
}


//! @brief Slot that exports plot tab to a specified matlab script file (.m)
//! @todo this function should use export functions in the log data handler instead
void PlotTab::exportToMatlab()
{
    // Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To MATLAB File"),
                                            gpConfig->getPlotDataDir(),
                                            tr("MATLAB script file (*.m)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

    // Write initial comment
    fileStream << "% MATLAB File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";

    // Cycle plot areas
    int nTotCurves=0;
    for (int a=0; a<mPlotAreas.size(); ++a)
    {
        PlotArea *pArea = mPlotAreas[a];
        QList<PlotCurve*> curves = pArea->getCurves();

        // Cycle plot curves
        for(int c=0; c<curves.size(); ++c)
        {
            fileStream << "x" << c+nTotCurves << "=[";                                         //Write time/X vector
            if(pArea->hasCustomXData())
            {
                //! @todo need smart function to autoselect copy or direct access depending on cached or not (also in other places)
                QVector<double> xvec = pArea->getCustomXData()->getDataVectorCopy();
                for(int j=0; j<xvec.size(); ++j)
                {
                    if(j>0) fileStream << ",";
                    fileStream << xvec[j];
                }
            }
            else
            {
                //! @todo what if not timevector then this will crash
                QVector<double> time = curves[c]->getTimeVectorPtr()->getDataVectorCopy();
                for(int j=0; j<time.size(); ++j)
                {
                    if(j>0) fileStream << ",";
                    fileStream << time[j];
                }
            }
            fileStream << "];\n";

            fileStream << "y" << c+nTotCurves << "=[";                                             //Write data vector
            QVector<double> data=curves[c]->getDataVectorCopy();
            for(int k=0; k<data.size(); ++k)
            {
                if(k>0) fileStream << ",";
                fileStream << data[k];
            }
            fileStream << "];\n";
        }
        // Increment number for next plot area
        nTotCurves += curves.size();

    }

    // Write plot functions
    nTotCurves=0;
    for (int a=0; a<mPlotAreas.size(); ++a)
    {
        PlotArea *pArea = mPlotAreas[a];
        QList<PlotCurve*> curves = pArea->getCurves();

        QStringList matlabColors;
        matlabColors << "r" << "g" << "b" << "c" << "m" << "y";
        fileStream << "hold on\n";
        fileStream << "subplot("<< mPlotAreas.size() << ",1," << a << ")\n";
        for(int c=0; c<curves.size(); ++c)
        {
            if( (curves[c]->getAxisY() == QwtPlot::yLeft && pArea->isLeftAxisLogarithmic()) ||
                (curves[c]->getAxisY() == QwtPlot::yRight && pArea->isRightAxisLogarithmic()) )
            {
                if(pArea->isBottomAxisLogarithmic())
                    fileStream << "loglog";
                else
                    fileStream << "semilogy";
            }
            else
            {
                if(pArea->isBottomAxisLogarithmic())
                    fileStream << "semilogx";
                else
                    fileStream << "plot";
            }
            fileStream << "(x" << c+nTotCurves << ",y" << c+nTotCurves << ",'-" << matlabColors[c%6] << "','linewidth'," << curves[c]->pen().width() << ")\n";
        }
        // Increment number for next plot area
        nTotCurves += curves.size();
    }


    file.close();
}


//! @brief Slot that exports plot tab to specified gnuplot file  (.dat)
//! @todo this function should use export functions in the log data handler instead
void PlotTab::exportToGnuplot()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To gnuplot File"),
                                            gpConfig->getPlotDataDir(),
                                            tr("gnuplot file (*.dat)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

    if (getPlotTabType() != XYPlotType)
    {
        gpTerminalWidget->mpConsole->printWarningMessage("Will only export from first sub-plot");
    }
    //! @todo make sure that csv can export from multiple sub plots (but how)

    QList<PlotCurve*> curves = mPlotAreas.first()->getCurves();

    // Write initial comment
    fileStream << "# gnuplot File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";
    fileStream << "# T";
    for(int i=0; i<curves.size(); ++i)
    {
        fileStream << "                  Y" << i;
    }
    fileStream << "\n";

    // Write time and data vectors
    QString dummy, err;
    QVector<double> time = curves.first()->getTimeVectorPtr()->getDataVectorCopy();
    for(int i=0; i<time.size(); ++i)
    {
        dummy.setNum(time[i]);
        fileStream << dummy;
        for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }

        for(int k=0; k<curves.size(); ++k)
        {
            dummy.setNum(curves[k]->getLogDataVariablePtr()->peekData(i,err));
            fileStream << dummy;
            for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }
        }
        fileStream << "\n";
    }

    file.close();
}

void PlotTab::exportToGraphics()
{
    QDialog *pGraphicsSettingsDialog = new QDialog(mpParentPlotWindow);
    pGraphicsSettingsDialog->setWindowTitle("Graphic Export");
    pGraphicsSettingsDialog->setWindowModality(Qt::WindowModal);

    mpImageDimUnit = new QComboBox();
    mpImageDimUnit->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mpImageDimUnit->addItem("px");
    mPreviousImageUnit = mpImageDimUnit->currentText();
    mpImageDimUnit->addItem("mm");
    mpImageDimUnit->addItem("cm");
    mpImageDimUnit->addItem("in");
    connect(mpImageDimUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(changedGraphicsExportSettings()));

    mpImageSetWidth = new QDoubleSpinBox(this);
    mpImageSetWidth->setDecimals(0);
    mpImageSetWidth->setRange(1,10000);
    mpImageSetWidth->setSingleStep(1);
    mpImageSetWidth->setValue(getQwtPlot(0)->width());
    connect(mpImageSetWidth, SIGNAL(editingFinished()), this, SLOT(changedGraphicsExportSettings()));

    mpImageSetHeight = new QDoubleSpinBox(this);
    mpImageSetHeight->setDecimals(0);
    mpImageSetHeight->setRange(1,10000);
    mpImageSetHeight->setSingleStep(1);
    mpImageSetHeight->setValue(getQwtPlot(0)->height());
    connect(mpImageSetHeight, SIGNAL(editingFinished()), this, SLOT(changedGraphicsExportSettings()));

    mpPixelSizeLabel = new QLabel(QString("%1X%2").arg(getQwtPlot(0)->width()).arg(getQwtPlot(0)->height()));
    mImagePixelSize = QSize(getQwtPlot(0)->width(), getQwtPlot(0)->height());


    mpImageDPI = new QDoubleSpinBox(this);
    mpImageDPI->setDecimals(0);
    mpImageDPI->setRange(1,10000);
    mpImageDPI->setSingleStep(1);
    mpImageDPI->setValue(96);
    connect(mpImageDPI, SIGNAL(editingFinished()), this, SLOT(changedGraphicsExportSettings()));

    // Vector
    mpImageFormat = new QComboBox();
    mpImageFormat->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mpImageFormat->addItem("png");
    mpImageFormat->addItem("pdf");
    mpImageFormat->addItem("svg");
    mpImageFormat->addItem("ps");
    mpImageFormat->addItem("jpeg");

    mpKeepAspectRatio = new QCheckBox("Keep Aspect Ratio");
    mpKeepAspectRatio->setChecked(true);

    int r=0;
    QGridLayout *pLayout = new QGridLayout();
    pLayout->addWidget(new QLabel("Format:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageFormat,r,1);
    pLayout->addWidget(new QLabel("Px:"),r,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpPixelSizeLabel,r,3);
    ++r;
    pLayout->addWidget(new QLabel("Dimension Unit:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageDimUnit,r,1);
    pLayout->addWidget(new QLabel("Width:"),r,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageSetWidth,r,3);
    pLayout->addWidget(new QLabel("Height:"),r+1,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageSetHeight,r+1,3);
    ++r;
    pLayout->addWidget(new QLabel("DPI:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageDPI,r,1);
    mpImageDPI->setDisabled(true);
    ++r;
    pLayout->addWidget(mpKeepAspectRatio,r,1);
    ++r;

    QPushButton *pExportButton = new QPushButton("Export");
    pExportButton->setAutoDefault(false);
    pLayout->addWidget(pExportButton,r,0);
    connect(pExportButton, SIGNAL(clicked()), this, SLOT(exportImage()));
    QPushButton *pCloseButton = new QPushButton("Close");
    pCloseButton->setAutoDefault(false);
    pLayout->addWidget(pCloseButton,r,5);
    connect(pCloseButton, SIGNAL(clicked()), pGraphicsSettingsDialog, SLOT(close()));

    pGraphicsSettingsDialog->setLayout(pLayout);
    pGraphicsSettingsDialog->exec();
}


void PlotTab::exportToPLO()
{
    // Open file dialog and initialize the file stream
    QString filePath;
    QFileInfo fileInfo;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To OldHopsan Format File"),
                                            gpConfig->getPlotDataDir(),
                                            tr("Hopsan Classic file (*.PLO)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());

    if (getPlotTabType() != XYPlotType)
    {
        gpTerminalWidget->mpConsole->printWarningMessage("Will only export from first sub-plot");
    }
    //! @todo make sure that csv can export from multiple sub plots (but how)

    QVector<SharedVariablePtrT> variables;
    for(int c=0; c<getCurves(0).size(); ++c)
    {
        variables.append(getCurves(0)[c]->getLogDataVariablePtr());
    }

    //! @todo this assumes that all curves belong to the same model
    getCurves(0).first()->getLogDataVariablePtr()->getLogDataHandler()->exportToPlo(filePath, variables);
}

void PlotTab::shiftAllGenerationsDown()
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->shiftAllGenerationsDown();
    }
}

void PlotTab::shiftAllGenerationsUp()
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->shiftAllGenerationsUp();
    }
}


void PlotTab::enableZoom(bool value)
{
    if(value)
    {
        mpParentPlotWindow->mpArrowButton->setChecked(false);
        mpParentPlotWindow->mpPanButton->setChecked(false);

        PlotArea *pArea;
        Q_FOREACH(pArea, mPlotAreas)
        {
            pArea->enableZoom();
        }
    }

    //! @todo FIXA / Peter
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}

void PlotTab::resetZoom()
{
    //! @todo only the one selected
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->resetZoom();
    }
}

void PlotTab::enableArrow(bool value)
{
    if(value)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        mpParentPlotWindow->mpPanButton->setChecked(false);
        PlotArea *pArea;
        Q_FOREACH(pArea, mPlotAreas)
        {
            pArea->enableArrow();
        }
    }
}


void PlotTab::enablePan(bool value)
{
    if(value)
    {
        mpParentPlotWindow->mpArrowButton->setChecked(false);
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        PlotArea *pArea;
        Q_FOREACH(pArea, mPlotAreas)
        {
            pArea->enablePan();
        }

    }
}


void PlotTab::enableGrid(bool value)
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->enableGrid(value);
    }
}


void PlotTab::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(mPlotAreas.first()->getQwtPlot()->canvasBackground().color(), this);
    if (color.isValid())
    {
        PlotArea *pArea;
        Q_FOREACH(pArea, mPlotAreas)
        {
            pArea->setBackgroundColor(color);
        }
    }
}

//! @todo what if I want all curves from all subplots
QList<PlotCurve*> &PlotTab::getCurves(int plotID)
{
    return mPlotAreas[plotID]->getCurves();
}



QwtPlot *PlotTab::getQwtPlot(const int subPlotId)
{
    return mPlotAreas[subPlotId]->getQwtPlot();
}

void PlotTab::addCurve(PlotCurve *pCurve, int subPlotId)
{
    if (subPlotId < mPlotAreas.size())
    {
        mPlotAreas[subPlotId]->addCurve(pCurve);
    }
}


int PlotTab::getNumberOfCurves(const int subPlotId) const
{
    if (subPlotId < mPlotAreas.size())
    {
        return mPlotAreas[subPlotId]->getNumberOfCurves();
    }
    return 0;
}

bool PlotTab::isArrowEnabled() const
{
    return mPlotAreas.first()->isArrowEnabled();
}

bool PlotTab::isZoomEnabled() const
{
    return mPlotAreas.first()->isZoomEnabled();
}

bool PlotTab::isPanEnabled() const
{
    return mPlotAreas.first()->isPanEnabled();
}


void PlotTab::update()
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->update();
    }
}

int PlotTab::getNumPlotAreas() const
{
    return mPlotAreas.size();
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

    //! @todo FIXA /Peter
//    if(mpBarPlot->isVisible())
//    {
//        QAbstractItemModel *model = mpBarPlot->model();

//        for(int c=0; c<model->columnCount(); ++c)
//        {
//            double losses = model->data(model->index(0, c)).toInt() - model->data(model->index(1, c)).toInt();;

//            QDomElement dataTag = appendDomElement(rDomElement, "data");
//            QDomElement varTag = appendDomElement(dataTag, "losses");
//            QString valueString;
//            valueString.setNum(losses);
//            QDomText value = varTag.ownerDocument().createTextNode(valueString);
//            varTag.appendChild(value);

//            if(descriptions)
//            {
//                varTag.setAttribute("component", model->headerData(c, Qt::Horizontal).toString());
//            }
//        }
//    }
//    else
//    {

//        //Cycle plot curves and write data tags
//        QString dummy;
//        QVector<double> time = mPlotCurvePtrs[0].first()->getTimeVectorPtr()->getDataVectorCopy();
//        for(int j=0; j<time.size(); ++j)
//        {
//            QDomElement dataTag = appendDomElement(rDomElement, "data");

//            if(mHasCustomXData)        //Special x-axis, replace time with x-data
//            {
//                setQrealAttribute(dataTag, mpCustomXData->getDataName(), mpCustomXData->peekData(j,dummy), 10, 'g');
//            }
//            else                        //X-axis = time
//            {
//                setQrealAttribute(dataTag, "time", time[j], 10, 'g');
//            }

//            //Write variable tags for each variable
//            for(int i=0; i<mPlotCurvePtrs[0].size(); ++i)
//            {
//                QString numTemp;
//                numTemp.setNum(i);
//                QDomElement varTag = appendDomElement(dataTag, mPlotCurvePtrs[0][i]->getDataName()+numTemp);
//                QString valueString;
//                valueString.setNum(mPlotCurvePtrs[0][i]->getDataVectorCopy()[j]);
//                QDomText value = varTag.ownerDocument().createTextNode(valueString);
//                varTag.appendChild(value);

//                if(descriptions)
//                {
//                    varTag.setAttribute("component", mPlotCurvePtrs[0][i]->getComponentName());
//                    varTag.setAttribute("port", mPlotCurvePtrs[0][i]->getPortName());
//                    varTag.setAttribute("type", mPlotCurvePtrs[0][i]->getDataName());
//                    varTag.setAttribute("unit", mPlotCurvePtrs[0][i]->getDataCustomPlotUnit());
//                }
//            }
//        }
//    }
}


bool PlotTab::isBarPlot() const
{
    return false;
}




bool PlotTab::hasCustomXData() const
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        if (pArea->hasCustomXData())
        {
            return true;
        }
    }
    return false;
}


void PlotTab::setLegendsVisible(bool value)
{
    PlotArea *pArea;
    Q_FOREACH(pArea, mPlotAreas)
    {
        pArea->setLegendsVisible(value);
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
                                            gpConfig->getPlotDataDir(),
                                            tr("Extensible Markup Language (*.xml)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
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

void PlotTab::exportImage()
{
    QString fileName, fileFilter;
    if (mpImageFormat->currentText() == "pdf")
    {
        fileFilter = "Portable Document Format (*.pdf)";
    }
    else if (mpImageFormat->currentText() == "ps")
    {
        fileFilter = "PostScript Format (*.ps)";
    }
    else if (mpImageFormat->currentText() == "svg")
    {
        fileFilter = "Scalable Vector Graphics (*.svg)";
    }
    else if (mpImageFormat->currentText() == "png")
    {
        fileFilter = "Portable Network Graphics (*.png)";
    }
    else if (mpImageFormat->currentText() == "jpeg")
    {
        fileFilter = "Joint Photographic Experts Group (*.jpg)";
    }

    fileName = QFileDialog::getSaveFileName(this, "Export File Name", gpConfig->getPlotGfxDir(), fileFilter);

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground,true);
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame,true);
    renderer.renderDocument(getQwtPlot(0),fileName,calcMMSize(),mpImageDPI->value());
    //! @todo should work for all subplots in plot
}

void PlotTab::changedGraphicsExportSettings()
{

    // Recalculate values for setSize boxes if unit changes
    if (mPreviousImageUnit != mpImageDimUnit->currentText())
    {
        updateGraphicsExportSizeEdits();
        mPreviousImageUnit = mpImageDimUnit->currentText();

        mImagePixelSize = calcPXSize(); // Set new pxSize
    }
    else if (mpKeepAspectRatio->isChecked())
    {
        // Calc new actual pixel resolution
         QSizeF pxSize = calcPXSize();

        // Adjust size according to AR
        double ar = mImagePixelSize.width() / mImagePixelSize.height();
        // See which one changed
        if ( fabs(pxSize.width() - mImagePixelSize.width()) > fabs(pxSize.height() - mImagePixelSize.height()) )
        {
            pxSize.rheight() = pxSize.width() * 1/ar;
        }
        else
        {
            pxSize.rwidth() = pxSize.height() * ar;
        }
        mImagePixelSize = pxSize; // Set new pxSize

        updateGraphicsExportSizeEdits();
    }
    else
    {
        mImagePixelSize = calcPXSize(); // Set new pxSize
    }

    mpPixelSizeLabel->setText(QString("%1X%2").arg(round(mImagePixelSize.width())).arg(round(mImagePixelSize.height())));
}

PlotArea *PlotTab::addPlotArea()
{
    PlotArea *pArea = new PlotArea(this);
    mPlotAreas.append(pArea);
    mpTabLayout->addWidget(pArea);
    return pArea;
}

void PlotTab::removePlotArea(const int id)
{
    if (id < mPlotAreas.size())
    {
        mPlotAreas[id]->deleteLater();
        mPlotAreas.removeAt(id);
    }
}


int PlotTab::getPlotIDForCurve(PlotCurve *pCurve)
{
    for (int i=0; i<mPlotAreas.size(); ++i)
    {
        if (mPlotAreas[i]->getCurves().contains(pCurve))
        {
            return i;
        }
    }
    // Plot curve has no plot ID (should never happen)
    qFatal("Plot curve has no plot ID (should never happen)");
    Q_ASSERT(false);
    return -1;
}



void PlotTab::setTabOnlyCustomXVector(SharedVariablePtrT pData, int /*plotID*/)
{


    mpParentPlotWindow->mpResetXVectorButton->setEnabled(true);
}




QSizeF PlotTab::calcMMSize() const
{
    QSizeF pxSize = calcPXSize();
    const double pxToMM = 1.0/mpImageDPI->value()*in2mm ;
    return QSizeF(pxSize.width()*pxToMM,pxSize.height()*pxToMM);
}

QSizeF PlotTab::calcPXSize(QString unit) const
{
    if (unit.isEmpty())
    {
        unit = mpImageDimUnit->currentText();
    }

    QSizeF pxSize;
    if ( unit == "px")
    {
        pxSize = QSizeF(mpImageSetWidth->value(), mpImageSetHeight->value());
    }
    else if (unit == "mm")
    {
        const double mmToPx = 1.0/in2mm * mpImageDPI->value();
        pxSize = QSizeF(mpImageSetWidth->value()*mmToPx,mpImageSetHeight->value()*mmToPx);
    }
    else if (unit == "cm")
    {
        const double cmToPx = 10.0/in2mm * mpImageDPI->value();
        pxSize = QSizeF(mpImageSetWidth->value()*cmToPx,mpImageSetHeight->value()*cmToPx);
    }
    else if (unit == "in")
    {
        pxSize = QSizeF(mpImageSetWidth->value()*mpImageDPI->value(), mpImageSetHeight->value()*mpImageDPI->value());
    }

    //! @todo round to int, ceil or floor, handle truncation
    return pxSize;
}

void PlotTab::updateGraphicsExportSizeEdits()
{
    QSizeF newSize;
    mpImageDPI->setDisabled(false);
    if (mpImageDimUnit->currentText() == "px")
    {
        newSize.setWidth(round(mImagePixelSize.width()));
        newSize.setHeight(round(mImagePixelSize.height()));
        mpImageSetWidth->setDecimals(0);
        mpImageSetHeight->setDecimals(0);
        mpImageDPI->setDisabled(true);
    }
    else if (mpImageDimUnit->currentText() == "mm")
    {
        const double px2mm = 1.0/mpImageDPI->value()*in2mm;
        newSize = mImagePixelSize*px2mm;
        mpImageSetWidth->setDecimals(2);
        mpImageSetHeight->setDecimals(2);
    }
    else if (mpImageDimUnit->currentText() == "cm")
    {
        const double px2cm = 1.0/(10*mpImageDPI->value())*in2mm;
        newSize = mImagePixelSize*px2cm;
        mpImageSetWidth->setDecimals(3);
        mpImageSetHeight->setDecimals(3);
    }
    else if (mpImageDimUnit->currentText() == "in")
    {
        const double px2in = 1.0/(mpImageDPI->value());
        newSize = mImagePixelSize*px2in;
        mpImageSetWidth->setDecimals(3);
        mpImageSetHeight->setDecimals(3);
    }

    mpImageSetWidth->setValue(newSize.width());
    mpImageSetHeight->setValue(newSize.height());
}


BodePlotTab::BodePlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow)
    : PlotTab(pParentPlotTabWidget, pParentPlotWindow)
{
    addPlotArea();
}

PlotTabTypeT BodePlotTab::getPlotTabType() const
{
    return BodePlotType;
}


BarchartPlotTab::BarchartPlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow)
    : PlotTab(pParentPlotTabWidget, pParentPlotWindow)
{
    removePlotArea(0);
    mpBarPlot = new QSint::BarChartPlotter(this);
    //mpBarPlot->setVisible(true);
}

PlotTabTypeT BarchartPlotTab::getPlotTabType() const
{
    return BarchartPlotType;
}

bool BarchartPlotTab::isBarPlot() const
{
    return true;
}

void BarchartPlotTab::addBarChart(QStandardItemModel *pItemModel)
{
    double min=0;
    double max=0;
    for(int c=0; c<pItemModel->columnCount(); ++c)
    {
        double componentMin = 0;
        double componentMax = 0;
        for(int r=0; r<pItemModel->rowCount(); ++r)
        {
            double data = pItemModel->data(pItemModel->index(r, c)).toDouble();
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

    mpTabLayout->addWidget(mpBarPlot,0,0);
}
