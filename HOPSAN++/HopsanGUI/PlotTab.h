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
//! @file   PlotTab.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013
//!
//! @brief Contains a class for plot tabs
//!
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

#ifndef PLOTTAB_H
#define PLOTTAB_H

#include <QTabWidget>
#include <QObject>
#include <QStandardItemModel>
#include <QtXml>


#include "Dependencies/BarChartPlotter/barchartplotter.h"
#include "common.h"

#include "LogVariable.h"

//Forward Declarations
class PlotWindow;
class PlotTabWidget;
class PlotCurve;
class PlotMarker;
class PlotLegend;
class PlotArea;
class QwtPlot;

//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotTabWidget;
    //friend class PlotMarker;
    friend class PlotCurve;
    friend class PlotArea;

public:
    PlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow);
    ~PlotTab();
    PlotTabWidget *mpParentPlotTabWidget;   //!< @todo should not be public
    PlotWindow *mpParentPlotWindow;         //!< @todo should not be public

    void setTabName(QString name);
    virtual PlotTabTypeT getPlotTabType() const;

    PlotArea *getPlotArea(const int subPlotId=0);
    QwtPlot *getQwtPlot(const int subPlotId=0);

    void addCurve(PlotCurve *pCurve, int subPlotId=0);
    void removeCurve(PlotCurve *curve);
    void removeAllCurvesOnAxis(const int axis);
    void setCustomXVectorForAll(QVector<double> xarray, const VariableDescription &rVarDesc, int plotID=0);
    void setCustomXVectorForAll(SharedVariablePtrT pData, int plotID=0);

    QList<PlotCurve*> &getCurves(int plotID=0);
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    int getNumberOfCurves(const int subPlotId=0) const;

    virtual void addBarChart(QStandardItemModel *pItemModel);

    bool isArrowEnabled() const;
    bool isZoomEnabled() const;
    bool isPanEnabled() const;
    bool isGridVisible() const;
    bool isZoomed(const int plotId) const;

    bool hasCustomXData() const;

    void setLegendsVisible(bool value);

    void update();

    // PlotTab only
    int getNumPlotAreas() const;
    virtual bool isBarPlot() const;

    void saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions);
    void exportToCsv(QString fileName);

public slots:
    void rescaleAxesToCurves();
    void toggleAxisLock();
    void updateAxisLabels();

    void openLegendSettingsDialog();
    void openAxisSettingsDialog();
    void openAxisLabelDialog();
    void openTimeScalingDialog();
    void applyAxisSettings();
    void applyAxisLabelSettings();
    void applyLegendSettings();
    void applyTimeScalingSettings();

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
    void exportToGraphics();

    void shiftAllGenerationsDown();
    void shiftAllGenerationsUp();

protected slots:
    QString updateXmlOutputTextInDialog();

    // PlotTab only
    void saveToXml();
    void exportImage();
    void changedGraphicsExportSettings();

protected:
    PlotArea *addPlotArea();
    void removePlotArea(const int id);
    int getPlotIDForCurve(PlotCurve *pCurve);

    void setTabOnlyCustomXVector(SharedVariablePtrT pData, int plotID=0);

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
    QCheckBox *mpKeepAspectRatio;
    QComboBox *mpImageDimUnit;
    QDoubleSpinBox *mpImageSetWidth;
    QDoubleSpinBox *mpImageSetHeight;
    QLabel *mpPixelSizeLabel;
    QSizeF mImagePixelSize;
    QDoubleSpinBox *mpImageDPI;
    QComboBox *mpImageFormat;
    QString mPreviousImageUnit;
    QSizeF calcMMSize() const;
    QSizeF calcPXSize(QString unit=QString()) const;
    void updateGraphicsExportSizeEdits();
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
