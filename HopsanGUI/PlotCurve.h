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
//! @file   PlotCurve.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010
//!
//! @brief Contains a class for plot curves
//!
//$Id$

#ifndef PLOTCURVE_H
#define PLOTCURVE_H

#include "qwt_legend_data.h"
#include "qwt_plot_legenditem.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_intervalcurve.h"

#include <QLabel>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>

#include "LogVariable.h"
#include "common.h"

// Forward Declaration
class PlotArea;
class PlotCurve;

enum {AxisIdRole=QwtLegendData::UserRole+1};

class PlotLegend : public QwtPlotLegendItem
{
private:
    QwtPlot::Axis mAxis;
    int mnItems;

public:
    PlotLegend(QwtPlot::Axis axisId);
    void updateLegend( const QwtPlotItem *plotItem, const QList<QwtLegendData> &data );
};


enum HopsanPlotCurveTypeEnumT {PortVariableType, FrequencyAnalysisType, NyquistType, BodeGainType, BodePhaseType, GeneralType};

//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject, public QwtPlotCurve
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotArea;
public:
    enum {LegendShowLineAndSymbol=QwtPlotCurve::LegendShowBrush+1};

    PlotCurve(HopsanVariable data, const QwtPlot::Axis axisY=QwtPlot::yLeft, const HopsanPlotCurveTypeEnumT curveType=PortVariableType);
    ~PlotCurve();

    void setIncludeGenerationInTitle(bool doit);
    void setIncludeSourceInTitle(bool doit);
    QString getCurveName() const;
    QString getCurveName(bool includeGeneration, bool includeSourceFile) const;
    HopsanPlotCurveTypeEnumT getCurveType();
    int getAxisY();

    PlotArea *getParentPlotArea() const;

    QVector<double> getVariableDataCopy() const;
    const HopsanVariable getHopsanVariable() const;
    const HopsanVariable getCustomXHopsanVariable() const;
    const SharedVectorVariableContainerT getSharedVectorVariableContainer() const;
    const SharedVectorVariableT getSharedVectorVariable() const;
    const SharedVectorVariableT getSharedTimeOrFrequencyVariable() const;
    const SharedVectorVariableT getSharedCustomXVariable() const;
    bool hasCustomXData() const;
    bool getShowVsSamples() const;

    bool minMaxPositiveNonZeroYValues(double &rMin, double &rMax);
    bool minMaxPositiveNonZeroXValues(double &rMin, double &rMax);

    int getGeneration() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataOriginalUnit() const;
    const QString &getDataCustomPlotUnit() const;
    const QString &getCurrentUnit() const;
    const QString &getDataModelPath() const;
    VariableSourceTypeT getDataSource() const;
    bool hasCustomDataPlotScale() const;

    bool hasCustomCurveDataPlotScale() const;
    void setCustomCurveDataUnit(const QString &rUnit);
    void setCustomCurveDataUnit(const QString &rUnit, double scale);
    void removeCustomCurveDataUnit();
    void setTimePlotScalingAndOffset(double scale, double offset);
    void setLocalCurvePlotScaleAndOffset(const double scale, const double offset);
    void setDataPlotOffset(const double offset);

    void setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData);
    void setCustomXData(const VariableDescription &rVarDesc, const QVector<double> &rvXdata);
    void setCustomXData(HopsanVariable data);
    void setCustomXData(const QString fullName);
    void setShowVsSamples(bool tf);

    bool isAutoUpdating() const;
    QColor getLineColor() const;
    void resetLegendSize();

    // Qwt overloaded function
    QList<QwtLegendData> legendData() const;
    QRectF boundingRect() const;

signals:
    void curveDataUpdated();
    void curveInfoUpdated();
    void customXDataChanged();
    void colorChanged(QColor);
    void markedActive(bool);

public slots:
    bool setGeneration(const int generation);
    bool setNonImportedGeneration(const int gen);
    void gotoPreviousGeneration();
    void gotoNextGeneration();

    void setLineWidth(int);
    void setLineStyle(QString lineStyle);
    void setLineSymbol(QString lineSymbol);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void openScaleDialog();
    void updateTimePlotScaleFromDialog();
    void updateLocalPlotScaleAndOffsetFromDialog();
    void updateDataPlotOffsetFromDialog();
    void updateToNewGeneration();

    void refreshCurveTitle();
    void setAutoUpdate(bool value);
    void openFrequencyAnalysisDialog();
    void markActive(bool value);

private slots:
    void updateCurve();
    void updateCurveName();

private:
    // Private member functions
    void deleteCustomData();
    void connectDataSignals();
    void connectCustomXDataSignals();
    void disconnectDataSignals();
    void disconnectCustomXDataSignals();

    // Curve data
    HopsanPlotCurveTypeEnumT mCurveType;
    HopsanVariable mData;
    HopsanVariable mCustomXdata;
    bool mHaveCustomData;
    bool mShowVsSamples;

    UnitScale mCustomCurveDataUnitScale;
    double mLocalAdditionalCurveScale;
    double mLocalAdditionalCurveOffset;

    // Curve properties settings
    bool mAutoUpdate, mIsActive, mIncludeGenInTitle, mIncludeSourceInTitle;
    QwtPlot::Axis mAxisY;
    QComboBox *mpTimeScaleComboBox;
    QDoubleSpinBox *mpTimeOffsetSpinBox;
    QLineEdit *mpLocalCurveScaleLineEdit;
    QLineEdit *mpLocalCurveOffsetLineEdit;
    QLineEdit *mpDataPlotOffsetLineEdit;
    PlotArea *mpParentPlotArea;

    // Line properties
    QColor mLineColor;
    QString mLineStyle;
    QString mLineSymbol;
    QwtSymbol *mpCurveSymbol;
    int mLineWidth;
    int mCurveSymbolSize;
};


//! @brief Class for plot markers
class PlotMarker : public QObject, public QwtPlotMarker
{
    Q_OBJECT
public:
    PlotMarker(PlotCurve *pCurve, PlotArea *pPlotArea);
    PlotCurve *getCurve();
    void setMovable(bool movable);

    // Overloaded virtual methods
    virtual bool eventFilter (QObject *object, QEvent *event);

signals:
    void idxChanged(int);
    void highlighted(bool);

public slots:
    void refreshLabel(const double x, const double y);
    void refreshLabel(const QString &label);
    void setColor(QColor color);

private:
    void highlight(bool tf);

    PlotCurve *mpCurve;
    PlotArea *mpPlotArea;
    QwtSymbol *mpMarkerSymbol;
    bool mIsHighlighted;
    bool mIsBeingMoved;
    bool mIsMovable;
};


//! @brief Class for vertical line with one plot marker per curve
class MultiPlotMarker : public QObject
{
    Q_OBJECT
public:
    MultiPlotMarker(QPoint pos, PlotArea *pPlotArea);
    void addMarker(PlotCurve* pCurve);
    void removeMarker(PlotCurve* pCurve);
    QList<PlotMarker*> mPlotMarkerPtrs;
    QwtPlotMarker *mpDummyMarker;       //Used to display the vertical line
public slots:
    void highlight(bool tf);
private slots:
    void moveAll(int idx);

};

#endif // PLOTCURVE_H
