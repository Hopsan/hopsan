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
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

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
};

class TimeScaleWidget : public QWidget
{
    Q_OBJECT
public:
    TimeScaleWidget(SharedVariablePtrT pTime, QWidget *pParent=0);
    void setScale(const QString &rUnitScale);
    void setOffset(const QString &rOffset);

signals:
    void valuesChanged();

public slots:
    void setVaules();

private:
    SharedVariablePtrT mpTime;
    QComboBox *mpTimeScaleComboBox;
    QLineEdit *mpTimeOffsetLineEdit;


};

class PlotArea : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
    friend class PlotMarker;

public:
    PlotArea(PlotTab *pParentPlotTab);
    ~PlotArea();

    void addCurve(PlotCurve *pCurve, QColor desiredColor=QColor());
    void setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc);
    void setCustomXVectorForAll(SharedVariablePtrT pData);
    void removeCurve(PlotCurve *pCurve);
    void removeAllCurvesOnAxis(const int axis);

    QList<PlotCurve*> &getCurves();
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    QwtPlot *getQwtPlot();

    int getNumberOfCurves() const;
    bool isArrowEnabled() const;
    bool isZoomEnabled() const;
    bool isPanEnabled() const;
    bool isGridVisible() const;
    bool isZoomed() const;

    bool hasCustomXData() const;
    const SharedVariablePtrT getCustomXData() const;

    void setBottomAxisLogarithmic(bool value);
    void setLeftAxisLogarithmic(bool value);
    void setRightAxisLogarithmic(bool value);
    bool isBottomAxisLogarithmic() const;
    bool isLeftAxisLogarithmic() const;
    bool isRightAxisLogarithmic() const;

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
    void resetXTimeVector();

    void enableArrow();
    void enablePan();
    void enableZoom();
    void resetZoom();

    void shiftAllGenerationsDown();
    void shiftAllGenerationsUp();

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
    void setTabOnlyCustomXVector(SharedVariablePtrT pData);
    void determineAddedCurveUnitOrScale(PlotCurve *pCurve);
    void rescaleAxisLimitsToMakeRoomForLegend(const QwtPlot::Axis axisId, QwtInterval &rAxisLimits);
    void calculateLegendBufferOffsets(const QwtPlot::Axis axisId, double &rBottomOffset, double &rTopOffset);
    void updatePlotMarkers();

    PlotTab *mpParentPlotTab;

    HopQwtPlot *mpQwtPlot;
    QwtPlotGrid *mpQwtPlotGrid;
    QwtPlotZoomer *mpQwtZoomerLeft, *mpQwtZoomerRight;
    QwtPlotMagnifier *mpQwtMagnifier;
    QwtPlotPanner *mpQwtPanner;

    QList<PlotMarker*> mPlotMarkers;
    QList<PlotCurve*> mPlotCurves;
    PlotCurve *mpActivePlotCurve;
    quint32 mNumYlCurves, mNumYrCurves;
    QStringList mCurveColors;
    QList<int> mUsedColorsCounter;

    QList<PlotCurveControlBox*> mPlotCurveControlBoxes;
    RectanglePainterWidget *mpPainterWidget;

    // Custom X data axis variables
    bool mHasCustomXData;
    SharedVariablePtrT mpCustomXData;

    // Legend related member variables
    PlotLegend *mpLeftPlotLegend, *mpRightPlotLegend;
    QCheckBox *mpLegendsEnabledCheckBox;
    QCheckBox *mpIncludeGenInCurveTitle;
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
    void axisLockHandler();
};

class CustomXDataDropEdit : public QLineEdit
{
    Q_OBJECT
public:
    CustomXDataDropEdit(QWidget *pParent=0);

signals:
    void newXData(QString fullName);

protected:
    void dropEvent(QDropEvent *e);

};

class PlotCurveControlBox : public QWidget
{
    Q_OBJECT
public:
    PlotCurveControlBox(PlotCurve *pPlotCurve, PlotArea *pParentArea);
    PlotCurve *getCurve();

public slots:
    void updateInfo();
    void updateColor(const QColor color);
    void markActive(bool active);

private slots:
    void activateCurve(bool active);
    void setXData(QString fullName);
    void resetTimeVector();
    void setGeneration(const int gen);

private:
    void refreshTitle();
    PlotCurve *mpPlotCurve;
    PlotArea *mpPlotArea;
    QLabel *mpTitle, *mpGenerationLabel, *mpSourceLable;
    QToolButton *mpColorBlob;
    QSpinBox *mpGenerationSpinBox;
    CustomXDataDropEdit *mpCustomXDataDrop;
    QToolButton *mpResetTimeButton;
};

#endif // PLOTAREA_H
