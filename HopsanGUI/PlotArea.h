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
    explicit HopQwtPlot(QWidget *pParent=0) : QwtPlot(pParent) {}
public slots:
    void replot();
signals:
    void afterReplot();
    void sizeChanged(int width, int height);

protected:
     virtual void resizeEvent( QResizeEvent *e );
};

class TimeOrFrequencyScaleWidget : public QWidget
{
    Q_OBJECT
public:
    TimeOrFrequencyScaleWidget(const QList<PlotCurve*> &rPlotCurves, QWidget *pParent=0);
//    void setScale(const QString &rUnitScale);
//    void setOffset(const QString &rOffset);

signals:
    void valuesChanged();

public slots:
    void setVaules();

private:
    QList<PlotCurve*> mPlotCurves;
    QString mQuantity;
    QComboBox *mpScaleComboBox;
    QLineEdit *mpOffsetLineEdit;
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

    void addCurve(PlotCurve *pCurve, QColor desiredColor=QColor(), int thickness=2, int type=1);
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
    void openTimeScalingDialog();
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

    void shiftAllGenerationsDown();
    void shiftAllGenerationsUp();
    void updateCurvesToNewGenerations();

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
    void determineAddedCurveDataUnitScale(PlotCurve *pCurve);
    void determineAddedCurveTFUnitScale(PlotCurve *pCurve);
    void setSmartYAxisLimits(const QwtPlot::Axis axisId, QwtInterval axisLimits);
    void calculateLegendBufferOffsets(const QwtPlot::Axis axisId, double &rBottomOffset, double &rTopOffset);
    void updatePlotMarkers();
    void updateWindowtitleModelName();

    PlotTab *mpParentPlotTab;

    HopQwtPlot *mpQwtPlot;
    QwtPlotGrid *mpQwtPlotGrid;
    QwtPlotZoomer *mpQwtZoomerLeft, *mpQwtZoomerRight;
    QwtPlotMagnifier *mpQwtMagnifier;
    QwtPlotPanner *mpQwtPanner;

    QList<PlotMarker*> mPlotMarkers;
    QList<MultiPlotMarker*> mMultiPlotMarkers;
    QList<PlotCurve*> mPlotCurves;
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
    void axisLockHandler();
};

#endif // PLOTAREA_H
