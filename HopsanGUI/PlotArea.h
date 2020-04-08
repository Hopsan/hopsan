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
//! @file   PlotArea.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014
//!
//! @brief Contains a class for plot tabs areas
//!
//$Id$

#ifndef PLOTAREA_H
#define PLOTAREA_H

#include <QObject>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>

//! @todo cleanup this mess (move to cpp if possible)
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_magnifier.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#include "qwt_legend.h"
#include "qwt_plot_legenditem.h"

#include "common.h"
#include "LogVariable.h"
#include "PlotCurveStyle.h"

// Forward Declaration
class PlotCurve;
class PlotMarker;
class MultiPlotMarker;
class PlotTab;
class PlotLegend;
class PlotCurveControlBox;
class RectanglePainterWidget;

enum PlotTabZOrderT {GridLinesZOrderType, LegendBelowCurveZOrderType, CurveZOrderType, ActiveCurveZOrderType, LegendAboveCurveZOrderType, CurveMarkerZOrderType};

class HopQwtInterval : public QwtInterval
{
public:
    HopQwtInterval( double minValue, double maxValue ) : QwtInterval(minValue, maxValue) {}
    void extendMin(const double value);
    void extendMax(const double value);

    HopQwtInterval&operator=( const QwtInterval& rhs )
    {
        setInterval(rhs.minValue(), rhs.maxValue(), rhs.borderFlags());
        return *this;
    }
};

class HopQwtPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit HopQwtPlot(QWidget *pParent=nullptr) : QwtPlot(pParent) {}
public slots:
    void replot();
signals:
    void afterReplot();
    void sizeChanged(int width, int height);

protected:
     virtual void resizeEvent( QResizeEvent *e );
};



class CurveColorSelector
{
public:
    CurveColorSelector();
    QStringList getCurveColors() const;
    QColor getLeastCommonCurveColor();
    void incrementCurveColorCounter(const QColor &rColor);
    void decrementCurveColorCounter(const QColor &rColor);

private:
    QStringList mCurveColors;
    QVector<int> mUsedColorsCounters;
};

class PlotArea : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
    friend class PlotMarker;

signals:
    void refreshContainsDataFromModels();

public:
    PlotArea(PlotTab *pParentPlotTab);
    ~PlotArea();

    void addCurve(PlotCurve *pCurve, PlotCurveStyle style=PlotCurveStyle());
    void setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc, bool force=false);
    void setCustomXVectorForAll(SharedVectorVariableT data, bool force=false);
    void removeAllCurvesOnAxis(const int axis);
    void removeAllCurves();

    void removePlotMarker(PlotMarker *pMarker);

    QList<PlotCurve*> &getCurves();
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    HopQwtPlot *getQwtPlot();

    const QStringList &getModelPaths() const;

    int getNumberOfCurves() const;
    bool isArrowEnabled() const;
    bool isZoomEnabled() const;
    bool isPanEnabled() const;
    bool isGridVisible() const;
    bool isZoomed() const;

    bool hasCustomXData() const;
    const SharedVectorVariableT getCustomXData() const;

    void setBottomAxisLogarithmic(bool value);
    void setLeftAxisLogarithmic(bool value);
    void setRightAxisLogarithmic(bool value);
    bool isBottomAxisLogarithmic() const;
    bool isLeftAxisLogarithmic() const;
    bool isRightAxisLogarithmic() const;
    bool isAxisLogarithmic(const QwtPlot::Axis axis) const;

    void setAxisLimits(QwtPlot::Axis axis, const double min, const double max, const double step=0, bool lockAxis=true);
    void setAxisLocked(QwtPlot::Axis axis, bool lockAxis);
    void setAxisLabel(QwtPlot::Axis axis, const QString &rLabel);

    HopQwtInterval getAxisCurveLimits(const QwtPlot::Axis axis) const;

    void setLegendsVisible(bool value);

public slots:
    void rescaleAxesToCurves();
    void updateAxisLabels();
    void replot();

    void openLegendSettingsDialog();
    void openAxisSettingsDialog();
    void openAxisLabelDialog();
    void openTimeOffsetDialog();
    void applyAxisSettings();
    void applyAxisLabelSettings();
    void applyLegendSettings();
    void applyTimeScalingSettings();
    void toggleAxisLock();

    void enableGrid(bool value);
    void setBackgroundColor(const QColor &rColor);
    void resetXDataVector();

    void enableArrow();
    void enablePan();
    void enableZoom();
    void resetZoom();

    void shiftModelGenerationsDown();
    void shiftModelGenerationsUp();
    void updateCurvesToNewGenerations();

    void hideCurve(PlotCurve *pCurve);
    void showCurve(PlotCurve *pCurve);
    void removeCurve(PlotCurve *pCurve);

    void insertMultiMarker(QPoint pos);

    void insertMarker(PlotCurve *pCurve, double x, double y, QString altLabel=QString(), bool movable=true);
    void insertMarker(PlotCurve *pCurve, QPoint pos, bool movable=true);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

private:
    void constructLegendSettingsDialog();
    void constructAxisSettingsDialog();
    void constructAxisLabelDialog();
    void setLegendSymbol(const QString symStyle);
    void setLegendSymbol(const QString symStyle, PlotCurve *pCurve);
    void determineCurveDataUnitScale(PlotCurve *pCurve);
    void determineCurveTFUnitScale(PlotCurve *pCurve);
    void setSmartYAxisLimits(const QwtPlot::Axis axisId, QwtInterval axisLimits);
    void calculateLegendBufferOffsets(const QwtPlot::Axis axisId, double &rBottomOffset, double &rTopOffset);
    void updatePlotMarkers();
    void updateWindowtitleModelName();
    void getLowestHighestGeneration(int &rLowest, int &rHighest);

    PlotTab *mpParentPlotTab;

    HopQwtPlot *mpQwtPlot;
    QwtPlotGrid *mpQwtPlotGrid;
    QwtPlotZoomer *mpQwtZoomerLeft, *mpQwtZoomerRight;
    QwtPlotMagnifier *mpQwtMagnifier;
    QwtPlotPanner *mpQwtPanner;

    QList<PlotMarker*> mPlotMarkers;
    QList<MultiPlotMarker*> mMultiPlotMarkers;
    QList<PlotCurve*> mPlotCurves, mHiddenPlotCurves;
    PlotCurve *mpActivePlotCurve;
    quint32 mNumYlCurves, mNumYrCurves;

    CurveColorSelector mCurveColorSelector;

    QStringList mModelPaths;

    QList<PlotCurveControlBox*> mPlotCurveControlBoxes;
    RectanglePainterWidget *mpPainterWidget;

    // Custom X data axis variables
    SharedVectorVariableT mCustomXData;

    // Legend related member variables
    PlotLegend *mpLeftPlotLegend, *mpRightPlotLegend;
    QCheckBox *mpLegendsEnabledCheckBox;
    QCheckBox *mpIncludeGenInCurveTitle;
    QCheckBox *mpIncludeSourceInCurveTitle;
    QCheckBox *mpLegendsAutoOffsetCheckBox;
    QDialog *mpLegendSettingsDialog;
    QComboBox *mpLegendLPosition;
    QComboBox *mpLegendRPosition;
    QComboBox *mpLegendBgType;
    QComboBox *mpLegendBgColor;
    QComboBox *mpLegendSymbolType;
    QSpinBox *mpLegendCols;
    QDoubleSpinBox *mpLegendLeftOffset;
    QDoubleSpinBox *mpLegendRightOffset;
    QSpinBox *mpLegendFontSize;

    // Axis properties
    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;
    bool mBottomAxisLogarithmic;
    bool mBottomAxisShowOnlySamples;
    QCheckBox *mpXLockCheckBox;
    QCheckBox *mpYLLockCheckBox;
    QCheckBox *mpYRLockCheckBox;

    // Axis settings related member variables
    QDialog *mpSetAxisDialog;
    QCheckBox *mpXLockDialogCheckBox;
    QCheckBox *mpYLLockDialogCheckBox;
    QCheckBox *mpYRLockDialogCheckBox;
    QLineEdit *mpXminLineEdit;
    QLineEdit *mpXmaxLineEdit;
    QLineEdit *mpYLminLineEdit;
    QLineEdit *mpYLmaxLineEdit;
    QLineEdit *mpYRminLineEdit;
    QLineEdit *mpYRmaxLineEdit;
    QDialog *mpUserDefinedLabelsDialog;
    QLineEdit *mpUserDefinedXLabel;
    QLineEdit *mpUserDefinedYlLabel;
    QLineEdit *mpUserDefinedYrLabel;
    QCheckBox *mpUserDefinedLabelsCheckBox;

private slots:
    void refreshLockCheckBoxPositions();
    void refreshPlotAreaCustomXData();
    void determineCurveXDataUnitScale(PlotCurve *pCurve);
    void axisLockHandler();
};

#endif // PLOTAREA_H
