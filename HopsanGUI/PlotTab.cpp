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
//! @file   PlotTab.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013
//!
//! @brief Contains a class for plot tabs
//!
//$Id$

//Other includes
#include <cmath>
#include <QDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QAction>
#include <QColorDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QMessageBox>


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
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
#include "PlotArea.h"
#include "Utilities/HelpPopUpWidget.h"

#include "qwt_plot_renderer.h"
#include "dependencies/BarChartPlotter/barchartplotter.h"
#include "dependencies/BarChartPlotter/axisbase.h"



const double in2mm = 25.4;

//! @brief Constructor for plot tabs.
//! @param parent Pointer to the plot window the tab belongs to
PlotTab::PlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow)
    : QWidget(pParentPlotTabWidget)
{
    mpParentPlotTabWidget = pParentPlotTabWidget;
    mpParentPlotWindow = pParentPlotWindow;

    mpTabLayout = new QGridLayout(this);
    mpTabLayout->setContentsMargins(0,0,0,0);

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
    mpParentPlotWindow->mpPlotCurveControlsStack->addWidget(mpCurveInfoScrollArea);

    addPlotArea();

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(&mGraphicsExporter, SIGNAL(exportImage()), this, SLOT(exportImageSelectFile()));
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

//! @todo currently only supports settings axis for top plot
void PlotTab::openAxisSettingsDialog()
{
    if (!mPlotAreas.empty())
    {
        mPlotAreas[0]->openAxisSettingsDialog();
    }
}

//! @todo currently only supports settings axis for top plot
void PlotTab::openAxisLabelDialog()
{
    if (!mPlotAreas.empty())
    {
        mPlotAreas[0]->openAxisLabelDialog();
    }
}

//! @todo currently only supports settings axis for top plot
void PlotTab::openTimeOffsetDialog()
{
    if (!mPlotAreas.empty())
    {
        mPlotAreas[0]->openTimeOffsetDialog();
    }
}

//! @brief Toggles the axis lock on/off for the enabled axis
void PlotTab::toggleAxisLock()
{
    for(PlotArea *pArea : mPlotAreas) {
        pArea->toggleAxisLock();
    }
}

void PlotTab::openLegendSettingsDialog()
{
    //! @todo solve in some smarter way
    for(PlotArea *pArea : mPlotAreas) {
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
    Q_UNUSED(pItemModel);
    // Do nothing by default, should only be implemented in bar chart plots
}



//! @brief Rescales the axes and the zoomers so that all plot curves will fit
void PlotTab::rescaleAxesToCurves()
{
    for(PlotArea *pArea : mPlotAreas) {
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

void PlotTab::removeAllCurves()
{
    for (int i=0; i<mPlotAreas.size(); ++i)
    {
        mPlotAreas[i]->removeAllCurves();
    }
}


//! @brief Changes the X vector of current plot tab to specified variable
//! @param xArray New data for X-axis
//! @param componentName Name of component from which new data origins
//! @param portName Name of port form which new data origins
//! @param dataName Data name (physical quantity) of new data
//! @param dataUnit Unit of new data
void PlotTab::setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc, int plotID, bool force)
{
    if (plotID < mPlotAreas.size())
    {
        mPlotAreas[plotID]->setCustomXVectorForAll(xArray, rVarDesc, force);
    }
}

void PlotTab::setCustomXVectorForAll(SharedVectorVariableT data, int plotID, bool force)
{
    if (plotID < mPlotAreas.size())
    {
        mPlotAreas[plotID]->setCustomXVectorForAll(data, force);
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
    for(PlotArea *pArea : mPlotAreas) {
        if (pArea->hasCustomXData()) {
            pArea->resetXDataVector();
        }
    }
//    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
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
                                            gpConfig->getStringSetting(cfg::dir::plotdata),
                                            tr("Comma-separated values files (*.csv)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setStringSetting(cfg::dir::plotdata, fileInfo.absolutePath());

    exportToCsv(filePath);
}


//! @brief Exports plot tab to comma-separated value file with specified filename
//! @param fileName File name
void PlotTab::exportToCsv(const QString fileName, const QTextStream::RealNumberNotation notation, const int precision)
{
    QFile file;
    file.setFileName(fileName);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMessageHandler->addErrorMessage("Failed to open file for writing: " + fileName);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    fileStream.setRealNumberNotation(notation);
    fileStream.setRealNumberPrecision(precision);

    //! @todo this function should use export functions in the log data handler instead
    // Cycle plot curves
    //! @todo this seems to write column ordered data, we need to support column or row ordered data
    if (getPlotTabType() != XYPlotType)
    {
        gpMessageHandler->addWarningMessage("Will only export from first sub-plot");
    }
    //! @todo make sure that csv can export from multiple sub plots (but how)

    PlotArea *pPlotArea = mPlotAreas.first();
    QList<PlotCurve*> curves = pPlotArea->getCurves();

    QString dummy;
    if(pPlotArea->hasCustomXData())
    {
        //! @todo how to handle this with multiple xvectors per curve
        //! @todo take into account whether cached or not, Should have some smart auto function for this in the data object

        QVector<double> xvec = pPlotArea->getCustomXData()->getDataVectorCopy(); //! @todo should direct access if not in cache
        for(int i=0; i<xvec.size(); ++i)
        {
            fileStream << xvec[i];
            for(int j=0; j<curves.size(); ++j)
            {
                fileStream << ", " << curves[j]->getSharedVectorVariable()->peekData(i,dummy);
            }
            fileStream << "\n";
        }
    }
    else
    {
        QVector<double> time = curves.first()->getSharedTimeOrFrequencyVariable()->getDataVectorCopy();
        for(int i=0; i<time.size(); ++i)
        {
            fileStream << time[i];
            for(int j=0; j<curves.size(); ++j)
            {
                fileStream << ", " << curves[j]->getSharedVectorVariable()->peekData(i,dummy);
            }
            fileStream << "\n";
        }
    }

    file.close();
}

void PlotTab::exportAsImage(const QString fileName, const QString fileType, const QString width, const QString height, const QString dim, const QString dpi)
{
    if (!mGraphicsExporter.supportsImageFormat(fileType))
    {
        gpMessageHandler->addErrorMessage(QString("Filetype: %1 not supported!").arg(fileType));
        return;
    }
    mGraphicsExporter.setImageFilename(fileName);
    mGraphicsExporter.setImageFormat(fileType);
    if (!width.isEmpty() && !height.isEmpty())
    {
        mGraphicsExporter.setImageSize(dim, width, height);
    }
    if (!dpi.isEmpty())
    {
        mGraphicsExporter.setImageDPI(dpi);
    }
    exportImage();
}

void PlotTab::showHelpPopupMessage(const QString &rMessage)
{
    mpParentPlotWindow->showHelpPopupMessage(rMessage);
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

    QFileInfo hvcFileInfo;
    if (fileName.isEmpty())
    {
        // Open file dialog and initialize the file stream
        QString filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To HVC and HVD"),
                                                        gpConfig->getStringSetting(cfg::dir::plotdata),
                                                        tr("HopsanValidationCfg (*.hvc)"));
        if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
        hvcFileInfo.setFile(filePath);
    }

    QString hvdFileName=hvcFileInfo.baseName()+".hvd";

    // Save HVC xml data
    QDomDocument doc;
    QDomElement hvcroot = doc.createElement("hopsanvalidationconfiguration");
    doc.appendChild(hvcroot);
    hvcroot.setAttribute("hvcversion", "0.2");

    QList<PlotCurve*> curves = mPlotAreas.first()->getCurves();
    QString modelPath = relativePath(curves.first()->getSharedVectorVariable()->getLogDataHandler()->getParentModel()->getTopLevelSystemContainer()->getModelFileInfo(),
                                     QDir(hvcFileInfo.canonicalPath()));
    QDomElement validation = appendDomElement(hvcroot, "validation");
    validation.setAttribute("date", QDateTime::currentDateTime().toString("yyyyMMdd"));
    validation.setAttribute("time", QDateTime::currentDateTime().toString("hhmmss"));
    validation.setAttribute("hopsanguiversion", HOPSANGUIVERSION);
    validation.setAttribute("hopsancoreversion", gHopsanCoreVersion);
    appendDomTextNode(validation, "modelfile", modelPath);
    appendDomTextNode(validation, "parameterset", "");
    appendDomTextNode(validation, "hvdfile", hvdFileName);

    // Cycle plot curves
    for (int i=0; i<curves.size(); ++i)
    {
        QDomElement variable = appendDomElement(validation, "variable");
        variable.setAttribute("name", curves[i]->getDataFullName());

        //! @todo this will only support one timevector
        appendDomIntegerNode(variable, "timecolumn", 0);
        appendDomIntegerNode(variable, "column", i+1);
        appendDomValueNode(variable, "tolerance", 0.01);
    }

    appendRootXMLProcessingInstruction(doc); //The xml "comment" on the first line
    bool savedOK = saveXmlFile(hvcFileInfo.absoluteFilePath(), gpMessageHandler, [&](){return doc;});
    if (!savedOK) {
        return;
    }

    // Save the csv data
    //! @todo this will only support one timevector
    this->exportToCsv(hvcFileInfo.absolutePath()+"/"+hvdFileName, QTextStream::ScientificNotation);

    qDebug() << hvcFileInfo.absoluteFilePath();
    qDebug() << hvcFileInfo.absolutePath()+"/"+hvdFileName;
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
                                            gpConfig->getStringSetting(cfg::dir::plotdata),
                                            tr("MATLAB script file (*.m)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setStringSetting(cfg::dir::plotdata, fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMessageHandler->addErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

    // Write initial comment
    fileStream << "% MATLAB File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";

    // Cycle plot areas
    int nTotCurves=0;
    QStringList addedVariablesX, addedVariablesY;
    for(const auto &pArea : qAsConst(mPlotAreas)) {
        // Cycle plot curves
        for(const auto &pCurve : qAsConst(pArea->getCurves())) {
            if(pArea->hasCustomXData())
            {
                //! @todo need smart function to auto select copy or direct access depending on cached or not (also in other places)
                QString varName = pArea->getCustomXData()->getSmartName("_");
                if(!addedVariablesX.contains(varName)) {
                    fileStream << varName << "=[";
                    QVector<double> xvec = pArea->getCustomXData()->getDataVectorCopy();
                    for(int i=0; i<xvec.size(); ++i) {
                        if(i>0) fileStream << ",";
                        fileStream << xvec[i];
                    }
                    fileStream << "];\n";
                }
                addedVariablesX.append(varName);
            }
            else
            {
                //! @todo what if not timevector then this will crash
                QString varName = pCurve->getSharedTimeOrFrequencyVariable()->getSmartName("_");
                if(!addedVariablesX.contains(varName)) {
                    fileStream << varName << "=[";
                    QVector<double> time = pCurve->getSharedTimeOrFrequencyVariable()->getDataVectorCopy();
                    for(int i=0; i<time.size(); ++i) {
                        if(i>0) fileStream << ",";
                        fileStream << time[i];
                    }
                    fileStream << "];\n";
                }
                addedVariablesX.append(varName);
            }

            //Check if this variable is plotted multiple times with different generations
            bool includeGenerationInVariableName = false;
            for(const auto &pCurve2 : qAsConst(pArea->getCurves())) {
                QString name1 = pCurve->getSharedVectorVariable()->getFullVariableName();
                QString name2 = pCurve2->getSharedVectorVariable()->getFullVariableName();
                if(name1 == name2) {
                    if(pCurve->getCurveGeneration() != pCurve2->getCurveGeneration()) {
                        includeGenerationInVariableName = true;
                        break;
                    }
                }
            }

            QString varName = pCurve->getSharedVectorVariable()->getSmartName("_");
            if(includeGenerationInVariableName) {
                varName.append("_"+QString::number(pCurve->getCurveGeneration()));
            }

            addedVariablesY.append(varName);
            fileStream << varName << "=[";                                          //Write data vector
            QVector<double> data=pCurve->getVariableDataCopy();
            for(int i=0; i<data.size(); ++i) {
                if(i>0) fileStream << ",";
                fileStream << data[i];
            }
            fileStream << "];\n";
            nTotCurves++;
        }
        // Increment number for next plot area
    }

    // Write plot functions
    fileStream << "figure\n";
    nTotCurves=0;
    for (int a=0; a<mPlotAreas.size(); ++a)
    {
        PlotArea *pArea = mPlotAreas[a];
        QList<PlotCurve*> curves = pArea->getCurves();

        QStringList matlabColors;
        matlabColors << "r" << "g" << "b" << "c" << "m" << "y";
        fileStream << "subplot("<< mPlotAreas.size() << ",1," << a+1 << ")\n";
        fileStream << "hold on\n";
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
            fileStream << "(" << addedVariablesX[c] << "," << addedVariablesY[c] << ",'-" << matlabColors[c%6] << "','linewidth'," << curves[c]->pen().width() << ")\n";
        }
        fileStream << "hold off\n";
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
                                            gpConfig->getStringSetting(cfg::dir::plotdata),
                                            tr("gnuplot file (*.dat)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setStringSetting(cfg::dir::plotdata, fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMessageHandler->addErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

    if (getPlotTabType() != XYPlotType)
    {
        gpMessageHandler->addWarningMessage("Will only export from first sub-plot");
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
    QVector<double> time = curves.first()->getSharedTimeOrFrequencyVariable()->getDataVectorCopy();
    for(int i=0; i<time.size(); ++i)
    {
        dummy.setNum(time[i]);
        fileStream << dummy;
        for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }

        for(int k=0; k<curves.size(); ++k)
        {
            dummy.setNum(curves[k]->getSharedVectorVariable()->peekData(i,err));
            fileStream << dummy;
            for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }
        }
        fileStream << "\n";
    }

    file.close();
}

void PlotTab::exportToGraphics()
{
    //! @todo this only works with one plot area in plot (same problem in export (render) code
    HopQwtPlot *pPlot = getQwtPlot(0);
    if (pPlot)
    {
        mGraphicsExporter.setScreenSize(pPlot->width(), pPlot->height());
        connect( pPlot, SIGNAL(sizeChanged(int,int)), &mGraphicsExporter, SLOT(setScreenSize(int,int)));
        mGraphicsExporter.openExportDialog();
        disconnect( pPlot, SIGNAL(sizeChanged(int,int)), &mGraphicsExporter, SLOT(setScreenSize(int,int)));
    }
}


void PlotTab::exportToPLO()
{
    // Open file dialog and initialize the file stream
    QString filePath;
    QFileInfo fileInfo;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To OldHopsan Format File"),
                                            gpConfig->getStringSetting(cfg::dir::plotdata),
                                            tr("Hopsan Classic file (*.PLO)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setStringSetting(cfg::dir::plotdata, fileInfo.absolutePath());

    if (getPlotTabType() != XYPlotType)
    {
        gpMessageHandler->addWarningMessage("Will only export from first sub-plot");
    }
    //! @todo make sure that csv can export from multiple sub plots (but how)

    QList<SharedVectorVariableT> variables;
    for(int c=0; c<getCurves(0).size(); ++c)
    {
        variables.append(getCurves(0)[c]->getSharedVectorVariable());
    }

    //! @todo this assumes that all curves belong to the same model
    getCurves(0).first()->getSharedVectorVariable()->getLogDataHandler()->exportToPlo(filePath, variables);
}

void PlotTab::exportToHDF5()
{
    // Open file dialog and initialize the file stream
    QString filePath;
    QFileInfo fileInfo;
    filePath = QFileDialog::getSaveFileName(this, tr("Export HDF5 Format File"),
                                            gpConfig->getStringSetting(cfg::dir::plotdata),
                                            tr("Hopsan Classic file (*.h5)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setStringSetting(cfg::dir::plotdata, fileInfo.absolutePath());

    if (getPlotTabType() != XYPlotType)
    {
        gpMessageHandler->addWarningMessage("Will only export from first sub-plot");
    }

    QList<SharedVectorVariableT> variables;
    QList<SharedVectorVariableT> xvariables;
    QList<SharedVectorVariableT> timevariables;
    QList<PlotCurve*> curves = getCurves(0);
    for(PlotCurve *pCurve : curves) {
        SharedVectorVariableT pVar, pToF, pCX;
        pVar = pCurve->getSharedVectorVariable();
        pToF = pCurve->getSharedTimeOrFrequencyVariable();
        pCX  = pCurve->getSharedCustomXVariable();

        variables.append(pVar);
        if (pCX && !xvariables.contains(pCX))
        {
            xvariables.append(pCX);
        }
        if (pToF && !timevariables.contains(pToF))
        {
            timevariables.append(pToF);
        }
    }
    //! @todo we might also want to save plot curve information in hdf5, so that plots can be reporduced outside

    if (!curves.isEmpty())
    {
        //! @todo this assumes that all curves belong to the same model
        curves.first()->getSharedVectorVariable()->getLogDataHandler()->exportToHDF5(filePath, variables+timevariables+xvariables);
    }
}

void PlotTab::shiftAllGenerationsDown()
{
    for(PlotArea *pArea : mPlotAreas) {
        pArea->shiftModelGenerationsDown();
    }
}

void PlotTab::shiftAllGenerationsUp()
{
    for(PlotArea *pArea : mPlotAreas) {
        pArea->shiftModelGenerationsUp();
    }
}

void PlotTab::updateWindowtitleModelNames()
{
    QStringList names;
    for(PlotArea *pArea : mPlotAreas) {
        names.append(pArea->getModelPaths());
    }
    mpParentPlotWindow->setModelPaths(names);
}


void PlotTab::enableZoom(bool value)
{
    if(value) {
        mpParentPlotWindow->mpArrowButton->setChecked(false);
        mpParentPlotWindow->mpPanButton->setChecked(false);

        for(PlotArea *pArea : mPlotAreas) {
            pArea->enableZoom();
        }
    }
}

void PlotTab::resetZoom()
{
    //! @todo only the one selected
    for(PlotArea *pArea : mPlotAreas) {
        pArea->resetZoom();
    }
}

void PlotTab::enableArrow(bool value)
{
    if(value) {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        mpParentPlotWindow->mpPanButton->setChecked(false);
        for(PlotArea *pArea : mPlotAreas) {
            pArea->enableArrow();
        }
    }
}


void PlotTab::enablePan(bool value)
{
    if(value) {
        mpParentPlotWindow->mpArrowButton->setChecked(false);
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        for(PlotArea *pArea : mPlotAreas) {
            pArea->enablePan();
        }

    }
}


void PlotTab::enableGrid(bool value)
{
    for(PlotArea *pArea : mPlotAreas) {
        pArea->enableGrid(value);
    }
}


void PlotTab::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(mPlotAreas.first()->getQwtPlot()->canvasBackground().color(), this);
    if (color.isValid()) {
        for(PlotArea *pArea : mPlotAreas) {
            pArea->setBackgroundColor(color);
        }
    }
}

//! @todo what if I want all curves from all subplots
QList<PlotCurve*> &PlotTab::getCurves(int plotID)
{
    return mPlotAreas[plotID]->getCurves();
}



HopQwtPlot *PlotTab::getQwtPlot(const int subPlotId)
{
    return mPlotAreas[subPlotId]->getQwtPlot();
}

void PlotTab::addCurve(PlotCurve *pCurve, const int subPlotId)
{
    if (subPlotId < mPlotAreas.size()) {
        mPlotAreas[subPlotId]->addCurve(pCurve);
    }
}

void PlotTab::addCurve(PlotCurve *pCurve, PlotCurveStyle style, const int subPlotId)
{
    if (subPlotId < mPlotAreas.size()) {
        mPlotAreas[subPlotId]->addCurve(pCurve, style);
    }
}


int PlotTab::getNumberOfCurves(const int subPlotId) const
{
    if (subPlotId < mPlotAreas.size()) {
        return mPlotAreas[subPlotId]->getNumberOfCurves();
    }
    return 0;
}

bool PlotTab::isEmpty() const
{
    //! @todo this needs to be overloaded for other types of plots that have no curves
    int sum=0;
    for(PlotArea *pArea : mPlotAreas) {
        sum+=pArea->getNumberOfCurves();
    }
    return sum==0;
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
    for(PlotArea *pArea : mPlotAreas) {
        pArea->replot();
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
    for(PlotArea *pArea : mPlotAreas) {
        if (pArea->hasCustomXData()) {
            return true;
        }
    }
    return false;
}


void PlotTab::setLegendsVisible(bool value)
{
    for(PlotArea *pArea : mPlotAreas) {
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
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To XML File"),
                                            gpConfig->getStringSetting(cfg::dir::plotdata),
                                            tr("Extensible Markup Language (*.xml)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gpConfig->setStringSetting(cfg::dir::plotdata, fileInfo.absolutePath());

    QDomDocument domDocument;
    QDomElement element = domDocument.createElement("hopsanplotdata");
    domDocument.appendChild(element);
    this->saveToDomElement(element, mpIncludeTimeCheckBox->isChecked(), mpIncludeDescriptionsCheckBox->isChecked());
    appendRootXMLProcessingInstruction(domDocument);
    bool savedOK = saveXmlFile(fileInfo.filePath(), gpMessageHandler, [&](){return domDocument;}, mpXmlIndentationSpinBox->value());
    if (!savedOK) {
        return;
    }

    mpExportXmlDialog->close();
}

void PlotTab::exportImage()
{
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground,true);
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame,true);
    renderer.renderDocument(getQwtPlot(0), mGraphicsExporter.imageFilename(), mGraphicsExporter.calcSizeMM(), int(mGraphicsExporter.dpi()));
    //! @todo should work for all subplots in plot

    // The QwtPlotRenderer code does not check if the file was successfully written, so we can not know for sure.
    // Lets at least check if the file exists.
    QFileInfo fi(mGraphicsExporter.imageFilename());
    if (!fi.exists()) {
        gpMessageHandler->addErrorMessage(QString("Could not export plot to image: '%1'").arg(mGraphicsExporter.imageFilename()), "Plot export");
    }
}

void PlotTab::exportImageSelectFile()
{
    QString selectedFilePath = mGraphicsExporter.selectExportFilename();
    if (!selectedFilePath.isEmpty()) {
        exportImage();
    }
}

//! @brief Updates maximum frequency in Bode dialog for Hz
//! @param [in] value Maximum frequency in rad/s
void PlotTab::updateMaximumBodeFreqHz(int value)
{
    mpMaxFrequencyHzSpinBox->blockSignals(true);
    mpMaxFrequencyHzSpinBox->setValue(int(value/(2.0*M_PI)));
    mpMaxFrequencyHzSpinBox->blockSignals(false);
}

//! @brief Updates maximum frequency in Bode dialog for rad/s
//! @param [in] value Maximum frequency in Hz
void PlotTab::updateMaximumBodeFreqRadSec(int value)
{
    mpMaxFrequencyRadSecSpinBox->blockSignals(true);
    mpMaxFrequencyRadSecSpinBox->setValue(int(value*2.0*M_PI));
    mpMaxFrequencyRadSecSpinBox->blockSignals(false);
}

//! @brief Updates minimum and maximum limits of windowing time limits in frequency analysis dialog
void PlotTab::updateWindowingMinMaxTime()
{
    mpWindowingMinTimeSpinBox->setMaximum(mpWindowingMaxTimeSpinBox->value());
    mpWindowingMaxTimeSpinBox->setMinimum(mpWindowingMinTimeSpinBox->value());
}

//! @brief Updates minimum and maximum limits of windowing time limits in frequency analysis dialog
void PlotTab::updateBodeWindowingMinMaxTime()
{
    mpBodeWindowingMinTimeSpinBox->setMaximum(mpBodeWindowingMaxTimeSpinBox->value());
    mpBodeWindowingMaxTimeSpinBox->setMinimum(mpBodeWindowingMinTimeSpinBox->value());
}

PlotArea *PlotTab::addPlotArea()
{
    PlotArea *pArea = new PlotArea(this);
    mPlotAreas.append(pArea);
    mpTabLayout->addWidget(pArea);
    connect(pArea, SIGNAL(refreshContainsDataFromModels()), this, SLOT(updateWindowtitleModelNames()));
    return pArea;
}

void PlotTab::removePlotArea(const int id)
{
    if (id < mPlotAreas.size())
    {
        mPlotAreas[id]->disconnect();
        mPlotAreas[id]->deleteLater();
        mPlotAreas.removeAt(id);
    }
}


int PlotTab::getPlotIDForCurve(PlotCurve *pCurve)
{
    for(int i=0; i<mPlotAreas.size(); ++i) {
        if (mPlotAreas[i]->getCurves().contains(pCurve)) {
            return i;
        }
    }
    // Plot curve has no plot ID (should never happen)
    qFatal("Plot curve has no plot ID (should never happen)");
    Q_ASSERT(false);
    return -1;
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

//! @todo maybe this function belongs in the plot area
void PlotTab::openFrequencyAnalysisDialog(PlotCurve *pCurve)
{
    QLabel *pInfoLabel = new QLabel(tr("This will generate a frequency spectrum. Using more log samples will increase accuracy of the results."));
    pInfoLabel->setWordWrap(true);
    pInfoLabel->setFixedWidth(300);

    QDialog *pDialog = new QDialog(this);
    pDialog->setWindowTitle("Generate Frequency Spectrum");

    QCheckBox *pLogScaleCheckBox = new QCheckBox("Use log scale", this);
    pLogScaleCheckBox->setChecked(false);

    QLabel *pSpectrumTypeLabel = new QLabel("Type:",this);

    QComboBox *pSpectrumTypeComboBox = new QComboBox(this);
    pSpectrumTypeComboBox->addItem("Power spectrum");
    pSpectrumTypeComboBox->addItem("Energy spectrum");
    pSpectrumTypeComboBox->addItem("RMS spectrum");
    pSpectrumTypeComboBox->setCurrentIndex(0);

    QGroupBox *pWindowingGroupBox = new QGroupBox("Windowing", pDialog);
    QGridLayout *pWindowingLayout = new QGridLayout(pWindowingGroupBox);

    QLabel *pWindowingLabel = new QLabel("Windowing function:",pDialog);
    QComboBox *pWindowingComboBox = new QComboBox(pDialog);
    pWindowingComboBox->addItems(QStringList() << "Rectangular" << "Hann Function" << "Flat Top (ISO 18431-2)");
    pWindowingComboBox->setCurrentIndex(0);

    double minX = pCurve->getSharedTimeOrFrequencyVariable()->minOfData();
    double maxX = pCurve->getSharedTimeOrFrequencyVariable()->maxOfData();

    QLabel *pMinTimeLabel = new QLabel("Min time: ", pDialog);

    double xdiff = maxX-minX;
    if(0 == xdiff) {
        gpMessageHandler->addErrorMessage("Minimum and maximum value of X-vector must not be equal when creating frequency spectrum");
        return;
    }

    mpWindowingMinTimeSpinBox = new QDoubleSpinBox(this);
    mpWindowingMinTimeSpinBox->setMinimum(minX);
    mpWindowingMinTimeSpinBox->setMaximum(maxX);
    mpWindowingMinTimeSpinBox->setValue(minX);
    mpWindowingMinTimeSpinBox->setDecimals(2-qFloor(log10(xdiff)));
    mpWindowingMinTimeSpinBox->setSingleStep(pow(10,qFloor(log10(xdiff))-1));

    QLabel *pMaxTimeLabel = new QLabel("Max time: ", pDialog);

    mpWindowingMaxTimeSpinBox = new QDoubleSpinBox(this);
    mpWindowingMaxTimeSpinBox->setMinimum(minX);
    mpWindowingMaxTimeSpinBox->setMaximum(maxX);
    mpWindowingMaxTimeSpinBox->setValue(maxX);
    mpWindowingMaxTimeSpinBox->setDecimals(2-qFloor(log10(xdiff)));
    mpWindowingMaxTimeSpinBox->setSingleStep(pow(10,qFloor(log10(xdiff))-1));

    connect(mpWindowingMinTimeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateWindowingMinMaxTime()));
    connect(mpWindowingMaxTimeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateWindowingMinMaxTime()));

    pWindowingLayout->addWidget(pWindowingLabel,            0, 0, 1, 2);
    pWindowingLayout->addWidget(pWindowingComboBox,         0, 0, 1, 2);
    pWindowingLayout->addWidget(pMinTimeLabel,              1, 0, 1, 1);
    pWindowingLayout->addWidget(mpWindowingMinTimeSpinBox,  1, 1, 1, 1);
    pWindowingLayout->addWidget(pMaxTimeLabel,              1, 2, 1, 1);
    pWindowingLayout->addWidget(mpWindowingMaxTimeSpinBox,  1, 3, 1, 1);

    QPushButton *pCancelButton = new QPushButton("Cancel");
    QPushButton *pNextButton = new QPushButton("Go!");

    // Toolbar
    QAction *pHelpAction = new QAction("Show Context Help", this);
    pHelpAction->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Help.svg"));
    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pHelpAction);

    QGridLayout *pLayout = new QGridLayout(pDialog);
    int row=0;
    pLayout->addWidget(pInfoLabel,               row++, 0, 1, 4);
    pLayout->addWidget(pLogScaleCheckBox,        row++, 0, 1, 4);
    pLayout->addWidget(pSpectrumTypeLabel,       row,   0, 1, 1);
    pLayout->addWidget(pSpectrumTypeComboBox,    row++, 1, 1, 3);
    pLayout->addWidget(pWindowingGroupBox,       row++, 2, 1, 2);
    pLayout->addWidget(pToolBar,                 row, 0, 1, 1);
    pLayout->addWidget(new QWidget(pDialog),     row, 1, 1, 1);
    pLayout->addWidget(pCancelButton,            row, 2, 1, 1);
    pLayout->addWidget(pNextButton,              row++, 3, 1, 1);
    pLayout->setColumnStretch(1, 1);

    pDialog->setLayout(pLayout);
    pDialog->setPalette(gpConfig->getPalette());

    connect(pCancelButton, SIGNAL(clicked()), pDialog, SLOT(close()));
    connect(pNextButton, SIGNAL(clicked()), pDialog, SLOT(accept()));
    connect(pHelpAction, SIGNAL(triggered()), this, SLOT(showFrequencyAnalysisHelp()));

    int rc = pDialog->exec();
    if (rc == QDialog::Accepted)
    {
        WindowingFunctionEnumT function;
        if(pWindowingComboBox->currentIndex() == 0) {
            function = RectangularWindow;
        }
        else if(pWindowingComboBox->currentIndex() == 1) {
            function = HannWindow;
        }
        else if(pWindowingComboBox->currentIndex() == 2) {
            function = FlatTopWindow;
        }

        double minTime = mpWindowingMinTimeSpinBox->value();
        double maxTime = mpWindowingMaxTimeSpinBox->value();
        FrequencySpectrumEnumT type;
        switch(pSpectrumTypeComboBox->currentIndex()) {
            case 0:
                type = PowerSpectrum;
                break;
            case 1:
                type = EnergySpectrum;
                break;
            case 2:
                type = RMSSpectrum;
                break;
        }
        SharedVectorVariableT pNewVar = pCurve->getSharedVectorVariable()->toFrequencySpectrum(SharedVectorVariableT(), type, function, minTime, maxTime);
        if (pNewVar) {
            PlotTab *pTab = mpParentPlotWindow->addPlotTab();
            pTab->addCurve(new PlotCurve(pNewVar, QwtPlot::yLeft, FrequencyAnalysisType));
            pTab->setTabName("Frequency Spectrum");

            if(pLogScaleCheckBox->isChecked())
            {
                pTab->getPlotArea(0)->setBottomAxisLogarithmic(true); //!< @todo maybe need a set all logarithmic function
                pTab->getPlotArea(0)->setLeftAxisLogarithmic(true);
                pTab->rescaleAxesToCurves(); //!< @todo maybe we should not need to call this here
            }
        }
        else {
            gpMessageHandler->addErrorMessage("Failed to create frequency spectrum");
        }
    }
    pDialog->deleteLater();
}

void PlotTab::showFrequencyAnalysisHelp()
{
    gpHelpPopupWidget->openContextHelp("userFrequencyAnalysisPage.html");
}

void PlotTab::openCreateBodePlotDialog()
{
    // Abort for empty tabs
    if (isEmpty())
    {
        return;
    }

    QDialog *pBodeDialog = new QDialog(this);
    pBodeDialog->setWindowTitle("Transfer Function Analysis");

    QMap<QRadioButton*, PlotCurve*> bodeInputButtonToCurveMap;
    QMap<QRadioButton*, PlotCurve*> bodeOutputButtonToCurveMap;

    QGroupBox *pInputGroupBox = new QGroupBox(tr("Input Variable"));
    QVBoxLayout *pInputGroupBoxLayout = new QVBoxLayout;
    pInputGroupBoxLayout->addStretch(1);
    for(int i=0; i<getNumberOfCurves(0); ++i)
    {
        QString curveName = getCurves(0).at(i)->getComponentName() + ", " +
                            getCurves(0).at(i)->getPortName() + ", " +
                            getCurves(0).at(i)->getDataName();
        QString aliasName = getCurves(0).at(i)->getAliasName();
        if(!aliasName.isEmpty()) {
            curveName.append(" ("+aliasName+")");
        }
        QRadioButton *radio = new QRadioButton(curveName);
        bodeInputButtonToCurveMap.insert(radio, getCurves(0).at(i));
        pInputGroupBoxLayout->addWidget(radio);
    }
    pInputGroupBox->setLayout(pInputGroupBoxLayout);

    QGroupBox *pOutputGroupBox = new QGroupBox(tr("Output Variable"));
    QVBoxLayout *pOutputGroupBoxLayout = new QVBoxLayout;
    pOutputGroupBoxLayout->addStretch(1);
    for(int i=0; i<getNumberOfCurves(0); ++i)
    {
        QString curveName = getCurves(0).at(i)->getComponentName() + ", " +
                            getCurves(0).at(i)->getPortName() + ", " +
                            getCurves(0).at(i)->getDataName();
        QString aliasName = getCurves(0).at(i)->getAliasName();
        if(!aliasName.isEmpty()) {
            curveName.append(" ("+aliasName+")");
        }
        QRadioButton *radio = new QRadioButton(curveName);
        bodeOutputButtonToCurveMap.insert(radio, getCurves(0).at(i));
        pOutputGroupBoxLayout->addWidget(radio);
    }
    pOutputGroupBox->setLayout(pOutputGroupBoxLayout);

    SharedVectorVariableT pTimeVector = getCurves(0).first()->getSharedTimeOrFrequencyVariable();
    if (pTimeVector)
    {
        const double dataSize = pTimeVector->getDataSize()+1;
        const double stopTime = pTimeVector->last();
        const double maxFreq = dataSize/stopTime/2;

        QGroupBox *pWindowingGroupBox = new QGroupBox("Windowing", pBodeDialog);
        QGridLayout *pWindowingLayout = new QGridLayout(pWindowingGroupBox);

        QLabel *pWindowingLabel = new QLabel("Windowing function:",pBodeDialog);
        QComboBox *pWindowingComboBox = new QComboBox(pBodeDialog);
        pWindowingComboBox->addItems(QStringList() << "Rectangular" << "Hann Function" << "Flat Top (ISO 18431-2)");
        pWindowingComboBox->setCurrentIndex(0);

        double minX = pTimeVector->minOfData();
        double maxX = pTimeVector->maxOfData();

        QLabel *pMinTimeLabel = new QLabel("Min time: ", pBodeDialog);

        double xdiff = maxX-minX;
        if(0 == xdiff) {
            gpMessageHandler->addErrorMessage("Minimum and maximum value of X-vector must not be equal when creating Bode plot");
            return;
        }

        mpBodeWindowingMinTimeSpinBox = new QDoubleSpinBox(pBodeDialog);
        mpBodeWindowingMinTimeSpinBox->setMinimum(minX);
        mpBodeWindowingMinTimeSpinBox->setMaximum(maxX);
        mpBodeWindowingMinTimeSpinBox->setValue(minX);
        mpBodeWindowingMinTimeSpinBox->setDecimals(2-qFloor(log10(xdiff)));
        mpBodeWindowingMinTimeSpinBox->setSingleStep(pow(10,qFloor(log10(xdiff))-1));

        QLabel *pMaxTimeLabel = new QLabel("Max time: ", pBodeDialog);

        mpBodeWindowingMaxTimeSpinBox = new QDoubleSpinBox(pBodeDialog);
        mpBodeWindowingMaxTimeSpinBox->setMinimum(minX);
        mpBodeWindowingMaxTimeSpinBox->setMaximum(maxX);
        mpBodeWindowingMaxTimeSpinBox->setValue(maxX);
        mpBodeWindowingMaxTimeSpinBox->setDecimals(2-qFloor(log10(xdiff)));
        mpBodeWindowingMaxTimeSpinBox->setSingleStep(pow(10,qFloor(log10(xdiff))-1));

        connect(mpBodeWindowingMinTimeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateBodeWindowingMinMaxTime()));
        connect(mpBodeWindowingMaxTimeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateBodeWindowingMinMaxTime()));

        pWindowingLayout->addWidget(pWindowingLabel,            0, 0, 1, 2);
        pWindowingLayout->addWidget(pWindowingComboBox,         0, 0, 1, 2);
        pWindowingLayout->addWidget(pMinTimeLabel,              1, 0, 1, 1);
        pWindowingLayout->addWidget(mpBodeWindowingMinTimeSpinBox,  1, 1, 1, 1);
        pWindowingLayout->addWidget(pMaxTimeLabel,              1, 2, 1, 1);
        pWindowingLayout->addWidget(mpBodeWindowingMaxTimeSpinBox,  1, 3, 1, 1);

        QGroupBox *pMaxFrequencyGroupBox = new QGroupBox("Maximum frequency in bode plot:",this);
        QLabel *pMaxFrequencyHzUnit = new QLabel("Hz");
        QLabel *pMaxFrequencyRadSecUnit = new QLabel("rad/s");
        mpMaxFrequencyHzSpinBox = new QSpinBox(this);
        mpMaxFrequencyHzSpinBox->setMinimum(0);
        mpMaxFrequencyHzSpinBox->setMaximum(int(maxFreq));
        mpMaxFrequencyHzSpinBox->setValue(int(maxFreq));
        mpMaxFrequencyHzSpinBox->setSingleStep(1);
        mpMaxFrequencyRadSecSpinBox = new QSpinBox(this);
        mpMaxFrequencyRadSecSpinBox->setMinimum(0);
        mpMaxFrequencyRadSecSpinBox->setMaximum(int(maxFreq*2.0*M_PI));
        mpMaxFrequencyRadSecSpinBox->setValue(int(maxFreq*2.0*M_PI));
        mpMaxFrequencyRadSecSpinBox->setSingleStep(1);
        connect(mpMaxFrequencyHzSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateMaximumBodeFreqRadSec(int)));
        connect(mpMaxFrequencyRadSecSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateMaximumBodeFreqHz(int)));

        QHBoxLayout *pSliderLayout = new QHBoxLayout(pMaxFrequencyGroupBox);
        pSliderLayout->addWidget(mpMaxFrequencyHzSpinBox);
        pSliderLayout->addWidget(pMaxFrequencyHzUnit);
        pSliderLayout->addWidget(mpMaxFrequencyRadSecSpinBox);
        pSliderLayout->addWidget(pMaxFrequencyRadSecUnit);

        QCheckBox *pBodeCheckBox = new QCheckBox("Create Bode Plot", pBodeDialog);
        QCheckBox *pNyquistCheckBox = new QCheckBox("Create Nyquist Plot", pBodeDialog);
        pBodeCheckBox->setChecked(true);
        pNyquistCheckBox->setChecked(false);
        connect(pBodeCheckBox, SIGNAL(toggled(bool)), mpMaxFrequencyHzSpinBox, SLOT(setEnabled(bool)));
        connect(pBodeCheckBox, SIGNAL(toggled(bool)), mpMaxFrequencyRadSecSpinBox, SLOT(setEnabled(bool)));
        connect(pBodeCheckBox, SIGNAL(toggled(bool)), pMaxFrequencyGroupBox, SLOT(setEnabled(bool)));
        connect(pBodeCheckBox, SIGNAL(toggled(bool)), pMaxFrequencyHzUnit, SLOT(setEnabled(bool)));
        connect(pBodeCheckBox, SIGNAL(toggled(bool)), pMaxFrequencyRadSecUnit, SLOT(setEnabled(bool)));

        QPushButton *pCancelButton = new QPushButton("Cancel");
        QPushButton *pNextButton = new QPushButton("Go!");
        pNextButton->setDefault(true);

        // Toolbar
        QAction *pHelpAction = new QAction("Show Context Help", this);
        pHelpAction->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Help.svg"));
        QToolBar *pToolBar = new QToolBar(this);
        pToolBar->addAction(pHelpAction);

        QGridLayout *pBodeDialogLayout = new QGridLayout;
        int row=0;
        pBodeDialogLayout->addWidget(pInputGroupBox,        row++, 0, 1, 3);
        pBodeDialogLayout->addWidget(pOutputGroupBox,       row++, 0, 1, 3);
        pBodeDialogLayout->addWidget(pWindowingGroupBox,    row++, 0, 1, 3);
        pBodeDialogLayout->addWidget(pMaxFrequencyGroupBox, row++, 0, 1, 3);
        pBodeDialogLayout->addWidget(pBodeCheckBox,         row++, 0, 1, 3);
        pBodeDialogLayout->addWidget(pNyquistCheckBox,      row++, 0, 1, 3);
        pBodeDialogLayout->addWidget(pToolBar,              row,   0, 1, 1);
        pBodeDialogLayout->addWidget(pCancelButton,         row,   1, 1, 1);
        pBodeDialogLayout->addWidget(pNextButton,           row,   2, 1, 1);

        pBodeDialog->setLayout(pBodeDialogLayout);
        pBodeDialog->setPalette(gpConfig->getPalette());

        connect(pCancelButton, SIGNAL(clicked()), pBodeDialog, SLOT(close()));
        connect(pNextButton, SIGNAL(clicked()), pBodeDialog, SLOT(accept()));
        connect(pHelpAction, SIGNAL(triggered()), this, SLOT(showFrequencyAnalysisHelp()));

        int rc = pBodeDialog->exec();
        if (rc == QDialog::Accepted)
        {
            PlotCurve *pInputCurve=nullptr, *pOutputCurve=nullptr;
            QMap<QRadioButton *, PlotCurve *>::iterator it;
            for(it=bodeInputButtonToCurveMap.begin(); it!=bodeInputButtonToCurveMap.end(); ++it)
            {
                if(it.key()->isChecked())
                {
                    pInputCurve = it.value();
                    break;
                }
            }
            for(it=bodeOutputButtonToCurveMap.begin(); it!=bodeOutputButtonToCurveMap.end(); ++it)
            {
                if(it.key()->isChecked())
                {
                    pOutputCurve = it.value();
                    break;
                }
            }
            //! @todo maybe check that both are time vectors and that both have same time
            if(pInputCurve == nullptr || pOutputCurve == nullptr)
            {
                QMessageBox::warning(this, tr("Transfer Function Analysis Failed"), tr("Both input and output vectors must be selected."));
            }
            else if(pInputCurve == pOutputCurve)
            {
                QMessageBox::warning(this, tr("Transfer Function Analysis Failed"), tr("Input and output vectors must be different."));
            }
            else
            {
                WindowingFunctionEnumT function;
                if(pWindowingComboBox->currentIndex() == 0) {
                    function = RectangularWindow;
                }
                else if(pWindowingComboBox->currentIndex() == 1) {
                    function = HannWindow;
                }
                else/* if(pWindowingComboBox->currentIndex() == 2)*/ {
                    function = FlatTopWindow;
                }


                double minTime = mpBodeWindowingMinTimeSpinBox->value();
                double maxTime = mpBodeWindowingMaxTimeSpinBox->value();

                mpParentPlotWindow->createBodePlot(pInputCurve->getSharedVectorVariable(), pOutputCurve->getSharedVectorVariable(), mpMaxFrequencyHzSpinBox->value(), pBodeCheckBox->isChecked(), pNyquistCheckBox->isChecked(), function, minTime, maxTime);
            }
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Transfer Function Analysis Failed"), tr("No time vector available"));
    }

    pBodeDialog->deleteLater();
}




void PlotGraphicsExporter::changedDialogSettings()
{
    // Save dialog values
    rememberDialogValues();

    mpSetWidthSpinBox->setDisabled(mpUseScreenSizeCheckBox->isChecked());
    mpSetHeightSpinBox->setDisabled(mpUseScreenSizeCheckBox->isChecked());

    // Recalculate values for setSize boxes if unit changes
    if (mPreviousDimensionsUnit != mpSetDimensionsUnit->currentText())
    {
        updateDialogSizeEdits();
        mPreviousDimensionsUnit = mpSetDimensionsUnit->currentText();
        mPixelSize = calcSizePX(); // Set new pxSize
    }
    else if (mpUseScreenSizeCheckBox->isChecked())
    {
        mpSetDimensionsUnit->setCurrentIndex(0);
        mPixelSize = mScreenSize;
    }
    else if (mpKeepAspectRatioCheckBox->isChecked())
    {
        // Calc new actual pixel resolution
        QSizeF pxSize = calcSizePX();

        // Adjust size according to AR
        double ar = mPixelSize.width() / mPixelSize.height();
        // See which one changed
        if ( fabs(pxSize.width() - mPixelSize.width()) > fabs(pxSize.height() - mPixelSize.height()) )
        {
            pxSize.rheight() = pxSize.width() * 1.0/ar;
        }
        else
        {
            pxSize.rwidth() = pxSize.height() * ar;
        }
        mPixelSize = pxSize; // Set new pxSize
        updateDialogSizeEdits();
    }
    else
    {
        mPixelSize = calcSizePX(); // Set new pxSize
    }

    // Update the pixel size label
    mpPixelSizeLabel->setText(QString("%1X%2").arg(round(mPixelSize.width())).arg(round(mPixelSize.height())));
}

PlotGraphicsExporter::PlotGraphicsExporter()
{
    mSupportedFormats << "png" << "pdf" << "svg" << "ps" << "jpeg";

    mImageFormat = gpConfig->getStringSetting(cfg::plotgfximageformat);
    mDimensionsUnit = gpConfig->getStringSetting(cfg::plotgfxdimensionunit);
    mDPI = gpConfig->getDoubleSetting(cfg::plotgfxdpi);
    mSetSize = gpConfig->getPlotGfxSize();
    mKeepAspect = gpConfig->getBoolSetting(cfg::plotgfxkeepaspect);
    mUseScreenSize = gpConfig->getBoolSetting(cfg::plotgfxusescreensize);
}

void PlotGraphicsExporter::openExportDialog()
{
    if(mpDialog && mpDialog->isVisible())
    {
        mpDialog->activateWindow();
        return;
    }

    mpDialog = new QDialog();
    mpDialog->setWindowTitle("Plot Graphics Export");
    mpDialog->setWindowModality(Qt::WindowModal);
    mpDialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    mpSetImageFormat = new QComboBox(mpDialog);
    mpSetImageFormat->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    for(const QString &format : mSupportedFormats) {
        mpSetImageFormat->addItem(format);
    }
    mpSetImageFormat->setCurrentIndex(mpSetImageFormat->findText(mImageFormat, Qt::MatchExactly));
    connect(mpSetImageFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(changedDialogSettings()));

    mpSetDimensionsUnit = new QComboBox(mpDialog);
    mpSetDimensionsUnit->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mpSetDimensionsUnit->addItem("px");
    mpSetDimensionsUnit->addItem("mm");
    mpSetDimensionsUnit->addItem("cm");
    mpSetDimensionsUnit->addItem("in");
    mpSetDimensionsUnit->setCurrentIndex(mpSetDimensionsUnit->findText(mDimensionsUnit, Qt::MatchExactly));
    mPreviousDimensionsUnit = mpSetDimensionsUnit->currentText();
    connect(mpSetDimensionsUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(changedDialogSettings()));

    mpSetWidthSpinBox = new QDoubleSpinBox(mpDialog);
    mpSetWidthSpinBox->setDecimals(0);
    mpSetWidthSpinBox->setRange(1,10000);
    mpSetWidthSpinBox->setSingleStep(1);
    mpSetWidthSpinBox->setValue(mSetSize.width());
    connect(mpSetWidthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changedDialogSettings()));

    mpSetHeightSpinBox = new QDoubleSpinBox(mpDialog);
    mpSetHeightSpinBox->setDecimals(0);
    mpSetHeightSpinBox->setRange(1,10000);
    mpSetHeightSpinBox->setSingleStep(1);
    mpSetHeightSpinBox->setValue(mSetSize.height());
    connect(mpSetHeightSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changedDialogSettings()));

    mpDPISpinBox = new QDoubleSpinBox(mpDialog);
    mpDPISpinBox->setDecimals(0);
    mpDPISpinBox->setRange(1,10000);
    mpDPISpinBox->setSingleStep(1);
    mpDPISpinBox->setValue(mDPI);
    connect(mpDPISpinBox, SIGNAL(valueChanged(double)), this, SLOT(changedDialogSettings()));

    mpKeepAspectRatioCheckBox = new QCheckBox("Keep Aspect Ratio",mpDialog);
    mpKeepAspectRatioCheckBox->setChecked(mKeepAspect);
    connect(mpKeepAspectRatioCheckBox, SIGNAL(toggled(bool)), this, SLOT(changedDialogSettings()));

    mpUseScreenSizeCheckBox = new QCheckBox("Use Screen Size",mpDialog);
    mpUseScreenSizeCheckBox->setChecked(mUseScreenSize);
    connect(mpUseScreenSizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(changedDialogSettings()));

    mPixelSize = calcSizePX();
    mpPixelSizeLabel = new QLabel(QString("%1X%2").arg(mPixelSize.width()).arg(mPixelSize.height()),mpDialog);

    int r=0;
    QGridLayout *pLayout = new QGridLayout(mpDialog);
    pLayout->addWidget(new QLabel("Format:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpSetImageFormat,r,1);
    pLayout->addWidget(new QLabel("Px:"),r,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpPixelSizeLabel,r,3);
    ++r;
    pLayout->addWidget(new QLabel("Dimension Unit:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpSetDimensionsUnit,r,1);
    pLayout->addWidget(new QLabel("Width:"),r,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpSetWidthSpinBox,r,3);
    pLayout->addWidget(new QLabel("Height:"),r+1,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpSetHeightSpinBox,r+1,3);
    ++r;
    pLayout->addWidget(new QLabel("DPI:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpDPISpinBox,r,1);
    mpDPISpinBox->setDisabled(true);
    ++r;
    pLayout->addWidget(mpKeepAspectRatioCheckBox,r,1);
    pLayout->addWidget(mpUseScreenSizeCheckBox,r,2,1,2);
    ++r;

    QPushButton *pExportButton = new QPushButton("Apply Export", mpDialog);
    pExportButton->setToolTip("Export but keep dialog open");
    pExportButton->setAutoDefault(false);
    pLayout->addWidget(pExportButton,r,0);
    connect(pExportButton, SIGNAL(clicked()), this, SIGNAL(exportImage()));

    QPushButton *pExportCloseButton = new QPushButton("Export", mpDialog);
    pExportCloseButton->setToolTip("Export and close dialog");
    pExportCloseButton->setAutoDefault(false);
    pLayout->addWidget(pExportCloseButton,r,3);
    connect(pExportCloseButton, SIGNAL(clicked()), this, SIGNAL(exportImage()));
    connect(pExportCloseButton, SIGNAL(clicked()), mpDialog, SLOT(close()));

    QPushButton *pCloseButton = new QPushButton("Close", mpDialog);
    pCloseButton->setToolTip("Close dialog without exporting");
    pCloseButton->setAutoDefault(false);
    pLayout->addWidget(pCloseButton,r,4);
    connect(pCloseButton, SIGNAL(clicked()), mpDialog, SLOT(close()));

    mpDialog->exec();
    mpDialog->deleteLater();
    mpDialog = nullptr;
}

QString PlotGraphicsExporter::selectExportFilename()
{
    QString fileFilter;
    if (mImageFormat == "pdf")
    {
        fileFilter = "Portable Document Format (*.pdf)";
    }
    else if (mImageFormat == "ps")
    {
        fileFilter = "PostScript Format (*.ps)";
    }
    else if (mImageFormat == "svg")
    {
        fileFilter = "Scalable Vector Graphics (*.svg)";
    }
    else if (mImageFormat == "png")
    {
        fileFilter = "Portable Network Graphics (*.png)";
    }
    else if (mImageFormat == "jpeg")
    {
        fileFilter = "Joint Photographic Experts Group (*.jpg)";
    }

    fileFilter.append(";;all (*.*)");

    QString path = gpConfig->getStringSetting(cfg::dir::plotgfx);
    if (!mImageFilename.isEmpty())
    {
        QFileInfo file(mImageFilename);
        path = file.canonicalPath();
    }

    mImageFilename = QFileDialog::getSaveFileName(mpDialog, "Choose Export Filename", path, fileFilter);
    if(!mImageFilename.isEmpty())
    {
        QFileInfo file(mImageFilename);
        gpConfig->setStringSetting(cfg::dir::plotgfx, file.canonicalPath());
    }
    return mImageFilename;
}

bool PlotGraphicsExporter::supportsImageFormat(QString format)
{
    return mSupportedFormats.contains(format.toLower());
}

QString PlotGraphicsExporter::imageFormat() const
{
    return mImageFormat;
}

QString PlotGraphicsExporter::imageFilename() const
{
    return mImageFilename;
}

double PlotGraphicsExporter::dpi() const
{
    return mDPI;
}

void PlotGraphicsExporter::setImageFilename(const QString &rFilename)
{
    mImageFilename = rFilename;
}

void PlotGraphicsExporter::setImageFormat(QString suffix)
{
    suffix = suffix.toLower();
    if (supportsImageFormat(suffix))
    {
        mImageFormat = suffix;
    }
}

void PlotGraphicsExporter::setImageSize(QString dimension, QString width, QString height)
{
   setImageSize(dimension, width.toDouble(), height.toDouble());
}

void PlotGraphicsExporter::setImageSize(QString dimension, double width, double height)
{
    dimension = dimension.toLower();
    if (dimension == "px" || dimension == "mm" || dimension == "cm" || dimension == "in")
    {
        mDimensionsUnit = dimension;
        mSetSize = QSizeF(qMax(width,1.0), qMax(height,1.0));
        //! @todo what about keep aspect ratio
    }
}

void PlotGraphicsExporter::setImageDPI(QString dpi)
{
    setImageDPI(dpi.toDouble());
}

void PlotGraphicsExporter::setImageDPI(double dpi)
{
    // make sure dpi wont be to low (like zero) which would lead to div by 0
    mDPI = qMax(dpi, 1.0);
}

QSizeF PlotGraphicsExporter::calcSizeMM() const
{
    QSizeF pxSize = calcSizePX();
    const double pxToMM = 1.0/mDPI*in2mm ;
    return QSizeF(pxSize.width()*pxToMM,pxSize.height()*pxToMM);
}

void PlotGraphicsExporter::setScreenSize(int width, int height)
{
    mScreenSize.setWidth(width);
    mScreenSize.setHeight(height);
    if (mpDialog)
    {
        changedDialogSettings();
    }
}

QSizeF PlotGraphicsExporter::calcSizePX(QString unit) const
{
    if (unit.isEmpty())
    {
        unit = mDimensionsUnit;
    }

    QSizeF pxSize;
    if ( unit == "px")
    {
        pxSize = mSetSize;
    }
    else if (unit == "mm")
    {
        const double mmToPx = 1.0/in2mm * mDPI;
        pxSize = QSizeF(mSetSize.width()*mmToPx, mSetSize.height()*mmToPx);
    }
    else if (unit == "cm")
    {
        const double cmToPx = 10.0/in2mm * mDPI;
        pxSize = QSizeF(mSetSize.width()*cmToPx, mSetSize.height()*cmToPx);
    }
    else if (unit == "in")
    {
        pxSize = QSizeF(mSetSize.width()*mDPI, mSetSize.height()*mDPI);
    }

    //! @todo round to int, ceil or floor, handle truncation
    return pxSize;
}

void PlotGraphicsExporter::updateDialogSizeEdits()
{
    QSizeF newSize;
    mpDPISpinBox->setDisabled(false);
    if (mpSetDimensionsUnit->currentText() == "px")
    {
        newSize.setWidth(round(mPixelSize.width()));
        newSize.setHeight(round(mPixelSize.height()));
        mpSetWidthSpinBox->setDecimals(0);
        mpSetHeightSpinBox->setDecimals(0);
        mpDPISpinBox->setDisabled(true);
    }
    else if (mpSetDimensionsUnit->currentText() == "mm")
    {
        const double px2mm = 1.0/mDPI*in2mm;
        newSize = mPixelSize*px2mm;
        mpSetWidthSpinBox->setDecimals(2);
        mpSetHeightSpinBox->setDecimals(2);
    }
    else if (mpSetDimensionsUnit->currentText() == "cm")
    {
        const double px2cm = 1.0/(10*mDPI)*in2mm;
        newSize = mPixelSize*px2cm;
        mpSetWidthSpinBox->setDecimals(3);
        mpSetHeightSpinBox->setDecimals(3);
    }
    else if (mpSetDimensionsUnit->currentText() == "in")
    {
        const double px2in = 1.0/mDPI;
        newSize = mPixelSize*px2in;
        mpSetWidthSpinBox->setDecimals(3);
        mpSetHeightSpinBox->setDecimals(3);
    }

    mpSetWidthSpinBox->blockSignals(true);
    mpSetHeightSpinBox->blockSignals(true);
    mpSetWidthSpinBox->setValue(newSize.width());
    mpSetHeightSpinBox->setValue(newSize.height());
    mpSetWidthSpinBox->blockSignals(false);
    mpSetHeightSpinBox->blockSignals(false);
    rememberDialogValues();
}

void PlotGraphicsExporter::rememberDialogValues()
{
    mImageFormat = mpSetImageFormat->currentText();
    mDimensionsUnit = mpSetDimensionsUnit->currentText();
    mSetSize = QSizeF(mpSetWidthSpinBox->value(), mpSetHeightSpinBox->value());
    mDPI = mpDPISpinBox->value();
    mUseScreenSize = mpUseScreenSizeCheckBox->isChecked();
    mKeepAspect = mpKeepAspectRatioCheckBox->isChecked();

    gpConfig->setStringSetting(cfg::plotgfximageformat, mImageFormat);
    gpConfig->setStringSetting(cfg::plotgfxdimensionunit, mDimensionsUnit);
    gpConfig->setPlotGfxSize(mSetSize);
    gpConfig->setDoubleSetting(cfg::plotgfxdpi, mDPI);
    gpConfig->setBoolSetting(cfg::plotgfxusescreensize, mUseScreenSize);
    gpConfig->setBoolSetting(cfg::plotgfxkeepaspect, mKeepAspect);
}
