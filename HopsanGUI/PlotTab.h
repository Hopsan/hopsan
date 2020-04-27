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
//! @file   PlotTab.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013
//!
//! @brief Contains a class for plot tabs
//!
//$Id$

#ifndef PLOTTAB_H
#define PLOTTAB_H

#include <QTabWidget>
#include <QObject>
#include <QStandardItemModel>
#include <QtXml>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QScrollArea>
#include <QTextEdit>
#include <QLabel>

#include "common.h"

#include "LogVariable.h"
#include "PlotCurveStyle.h"

// Forward Declarations
class PlotWindow;
class PlotTabWidget;
class PlotCurve;
class PlotMarker;
class PlotLegend;
class PlotArea;
class HopQwtPlot;
namespace QSint{
class BarChartPlotter;
}



class PlotGraphicsExporter : public QObject
{
    Q_OBJECT

public:
    PlotGraphicsExporter();
    void openExportDialog();
    QString selectExportFilename();

    bool supportsImageFormat(QString format);

    QString imageFormat() const;
    QString imageFilename() const;
    double dpi() const;

    void setImageFilename(const QString &rFilename);
    void setImageFormat(QString suffix);
    void setImageSize(QString dimension, QString width, QString height);
    void setImageSize(QString dimension, double width, double height);
    void setImageDPI(QString dpi);
    void setImageDPI(double dpi);

    QSizeF calcSizeMM() const;

public slots:
    void setScreenSize(int width, int height);

signals:
    void exportImage();

protected:
    QPointer<QDialog> mpDialog;
    QStringList mSupportedFormats;
    QString mImageFilename;
    QString mImageFormat;
    QString mDimensionsUnit;
    double mDPI;
    QSizeF mSetSize;
    bool mKeepAspect;
    bool mUseScreenSize;

protected slots:
    void changedDialogSettings();

private:
    QComboBox *mpSetImageFormat;
    QComboBox *mpSetDimensionsUnit;
    QDoubleSpinBox *mpDPISpinBox;
    QDoubleSpinBox *mpSetWidthSpinBox;
    QDoubleSpinBox *mpSetHeightSpinBox;
    QCheckBox *mpKeepAspectRatioCheckBox;
    QCheckBox *mpUseScreenSizeCheckBox;

    QLabel *mpPixelSizeLabel;
    QSizeF mPixelSize;
    QSizeF mScreenSize;

    QString mPreviousDimensionsUnit;


    QSizeF calcSizePX(QString unit=QString()) const;
    void updateDialogSizeEdits();
    void rememberDialogValues();
};

//! @brief Plot tab types
enum PlotTabTypeT {XYPlotType, BodePlotType, BarchartPlotType};

//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotTabWidget;
    friend class PlotCurve;
    friend class PlotArea;

public:
    PlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow);
    ~PlotTab();

    void setTabName(QString name);
    virtual PlotTabTypeT getPlotTabType() const;
    virtual bool isBarPlot() const;

    PlotArea *getPlotArea(const int subPlotId=0);
    int getNumPlotAreas() const;
    HopQwtPlot *getQwtPlot(const int subPlotId=0);

    void addCurve(PlotCurve *pCurve, const int subPlotId=0);
    void addCurve(PlotCurve *pCurve, PlotCurveStyle style, const int subPlotId=0);
    void removeCurve(PlotCurve *curve);
    void removeAllCurvesOnAxis(const int axis);
    void removeAllCurves();
    void setCustomXVectorForAll(QVector<double> xarray, const VariableDescription &rVarDesc, int plotID=0, bool force=false);
    void setCustomXVectorForAll(SharedVectorVariableT data, int plotID=0, bool force=false);

    QList<PlotCurve*> &getCurves(int plotID=0);
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    int getNumberOfCurves(const int subPlotId=0) const;
    bool isEmpty() const;

    virtual void addBarChart(QStandardItemModel *pItemModel);

    bool isArrowEnabled() const;
    bool isZoomEnabled() const;
    bool isPanEnabled() const;
    bool isGridVisible() const;
    bool isZoomed(const int plotId) const;

    void setLegendsVisible(bool value);
    bool hasCustomXData() const;

    void update();

    void saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions);
    void exportToCsv(const QString fileName, const QTextStream::RealNumberNotation notation=QTextStream::SmartNotation, const int precision=6);
    void exportAsImage(const QString fileName, const QString fileType, const QString width, const QString height, const QString dim, const QString dpi);

    void showHelpPopupMessage(const QString &rMessage);

public slots:
    void rescaleAxesToCurves();
    void toggleAxisLock();
    void updateAxisLabels();

    void openLegendSettingsDialog();
    void openAxisSettingsDialog();
    void openAxisLabelDialog();
    void openTimeOffsetDialog();

    void openCreateBodePlotDialog();

    void openFrequencyAnalysisDialog(PlotCurve *pCurve);
    void showFrequencyAnalysisHelp();

    void enableArrow(bool value);
    void enablePan(bool value);
    void enableZoom(bool value);
    void resetZoom();

    void enableGrid(bool value);

    void setBackgroundColor();
    void resetXTimeVector();

    void exportToXml();
    void exportToCsv();
    void exportToHvc(QString fileName="");
    void exportToMatlab();
    void exportToGnuplot();
    void exportToPLO();
    void exportToHDF5();
    void exportToGraphics();

    void shiftAllGenerationsDown();
    void shiftAllGenerationsUp();

protected slots:
    void updateWindowtitleModelNames();
    QString updateXmlOutputTextInDialog();
    void saveToXml();
    void exportImage();
    void exportImageSelectFile();
    void updateMaximumBodeFreqHz(int value);
    void updateMaximumBodeFreqRadSec(int value);
    void updateWindowingMinMaxTime();
    void updateBodeWindowingMinMaxTime();

protected:
    PlotArea *addPlotArea();
    void removePlotArea(const int id);
    int getPlotIDForCurve(PlotCurve *pCurve);

    PlotTabWidget *mpParentPlotTabWidget;
    PlotWindow *mpParentPlotWindow;

    QGridLayout *mpTabLayout;
    QList<PlotArea*> mPlotAreas;
    QScrollArea *mpCurveInfoScrollArea;

    //Stuff used in export to xml dialog
    QDialog *mpExportXmlDialog;
    QSpinBox *mpXmlIndentationSpinBox;
    QCheckBox *mpIncludeTimeCheckBox;
    QCheckBox *mpIncludeDescriptionsCheckBox;
    QTextEdit *mpXmlOutputTextBox;

    // Export graphics settings
    PlotGraphicsExporter mGraphicsExporter;

    //Bode plot dialog
    QDoubleSpinBox *mpBodeWindowingMinTimeSpinBox;
    QDoubleSpinBox *mpBodeWindowingMaxTimeSpinBox;
    QSpinBox *mpMaxFrequencyHzSpinBox;
    QSpinBox *mpMaxFrequencyRadSecSpinBox;

    //Frequency spectrum dialog
    QDoubleSpinBox *mpWindowingMinTimeSpinBox;
    QDoubleSpinBox *mpWindowingMaxTimeSpinBox;
};

class BodePlotTab : public PlotTab
{
    Q_OBJECT
public:
    enum {MagnitudePlot, PhasePlot};
    BodePlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow);
    virtual PlotTabTypeT getPlotTabType() const;
};

class BarchartPlotTab : public PlotTab
{
    Q_OBJECT
public:
    BarchartPlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow);
    virtual PlotTabTypeT getPlotTabType() const;
    virtual bool isBarPlot() const;
    virtual void addBarChart(QStandardItemModel *pItemModel);

private:
    QSint::BarChartPlotter *mpBarPlot;

};

#endif // PLOTTAB_H
