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

    PlotCurve(SharedVectorVariableT data, const QwtPlot::Axis axisY=QwtPlot::yLeft, const HopsanPlotCurveTypeEnumT curveType=PortVariableType);
    ~PlotCurve();

    void setIncludeGenerationInTitle(bool doit);
    void setIncludeSourceInTitle(bool doit);
    QString getCurveName() const;
    QString getCurveName(bool includeGeneration, bool includeSourceFile) const;
    HopsanPlotCurveTypeEnumT getCurveType();
    int getAxisY();

    PlotArea *getParentPlotArea() const;

    QVector<double> getVariableDataCopy() const;
    const SharedVectorVariableT getSharedVectorVariable() const;
    const SharedVectorVariableT getSharedTimeOrFrequencyVariable() const;
    const SharedVectorVariableT getSharedCustomXVariable() const;
    bool hasCustomXData() const;

    bool minMaxPositiveNonZeroYValues(double &rMin, double &rMax);
    bool minMaxPositiveNonZeroXValues(double &rMin, double &rMax);

    bool isCurveGenerationValid() const;
    int getCurveGeneration() const;
    int getDataGeneration() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;
    const QString &getDataQuantity() const;
    const QString &getDataModelPath() const;
    QString getDataFullName() const;
    QString getDataSmartName() const;
    QString getAliasName() const;
    VariableSourceTypeT getDataSource() const;

    bool hasCurveDataUnitScale() const;
    void setCurveDataUnitScale(const QString &rUnit);
    void setCurveDataUnitScale(const UnitConverter &rUS);
    const UnitConverter getCurveDataUnitScale() const;
    void resetCurveDataUnitScale();

    bool hasCurveCustomXDataUnitScale() const;
    void setCurveCustomXDataUnitScale(const QString &rUnit);
    void setCurveCustomXDataUnitScale(const UnitConverter &rUS);
    UnitConverter getCurveCustomXDataUnitScale() const;
    void resetCurveCustomXDataUnitScale();

    void setCurveExtraDataScaleAndOffset(const double scale, const double offset);

    QString getCurrentPlotUnit() const;
    QString getCurrentXPlotUnit() const;
    QString getCurrentTFPlotUnit() const;

    UnitConverter getCurveTFUnitScale() const;
    void setCurveTFUnitScale(const UnitConverter &us);
    void setCurveTFUnitScale(const QString& unit);
    void resetCurveTFUnitScale();

    void setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData);
    void setCustomXData(const VariableDescription &rVarDesc, const QVector<double> &rvXdata);
    void setCustomXData(SharedVectorVariableT data);
    void setCustomXData(const QString fullName);

    bool getShowVsSamples() const;
    void setShowVsSamples(bool tf);

    bool isAutoUpdating() const;
    bool isInverted() const;
    QColor getLineColor() const;
    void resetLegendSize();

    // Qwt overloaded function
    QList<QwtLegendData> legendData() const;
    QRectF boundingRect() const;

signals:
    void curveDataUpdated();
    void curveInfoUpdated();
    void customXDataChanged(PlotCurve *pCurve);
    void colorChanged(QColor);
    void markedActive(bool);
    void dataRemoved(PlotCurve *pCurve);

public slots:
    bool setGeneration(const int generation);
    bool setNonImportedGeneration(const int generation);
    bool autoDecrementModelSourceGeneration();
    bool autoIncrementModelSourceGeneration();

    void setLineWidth(int);
    void setLineStyle(QString lineStyle);
    void setLineSymbol(QString lineSymbol);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void openScaleDialog();
    void updateCurveExtraDataScaleAndOffsetFromDialog();
    void updateToNewGeneration();

    void refreshCurveTitle();
    void setAutoUpdate(bool value);
    void setInvertPlot(bool tf);
    void openFrequencyAnalysisDialog();
    void markActive(bool value);

private slots:
    void updateCurve();
    void updateCurveName();
    void dataIsBeingRemoved();
    void customXDataIsBeingRemoved();

private:
    // Private member functions
    void deleteCustomData();
    void connectDataSignals();
    void connectCustomXDataSignals();
    void disconnectDataSignals();
    void disconnectCustomXDataSignals();

    // Curve data
    HopsanPlotCurveTypeEnumT mCurveType;
    SharedVectorVariableT mData;
    SharedVectorVariableT mCustomXdata;
    bool mHaveCustomData;
    bool mShowVsSamples;

    // Curve scale
    UnitConverter mCurveCustomXDataUnitScale;
    UnitConverter mCurveDataUnitScale;
    UnitConverter mCurveTFUnitScale;
    double mCurveExtraDataScale;
    double mCurveExtraDataOffset;

    // Curve set generation
    int mSetGeneration;
    bool mSetGenerationIsValid;

    // Curve properties settings
    bool mAutoUpdate, mIsActive, mIncludeGenInTitle, mIncludeSourceInTitle;
    QwtPlot::Axis mAxisY;
    QComboBox *mpTimeScaleComboBox;
    QDoubleSpinBox *mpTimeOffsetSpinBox;
    QLineEdit *mpCurveExtraDataScaleLineEdit;
    QLineEdit *mpCurveExtraDataOffsetLineEdit;
    QLineEdit *mpCurveDataOffsetLineEdit;
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
    void updatePosition();

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
    void updatePosition();
private slots:
    void moveAll(int idx);

};

#endif // PLOTCURVE_H
