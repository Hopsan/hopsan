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
//! @file   PlutCurve.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010
//!
//! @brief Contains a class for plot curves
//!
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

#ifndef PLOTCURVE_H
#define PLOTCURVE_H

#include "qwt_legend_data.h"
#include "qwt_plot_legenditem.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"

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
class PlotTab;
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

class CurveInfoBox : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
public:
    CurveInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent);
    void setLineColor(const QColor color);
    void updateInfo();

private:
    void refreshTitle();
    void refreshActive(bool active);
    PlotCurve *mpParentPlotCurve;
    QLabel *mpTitle;
    QToolButton *mpColorBlob;
    QSpinBox *mpGenerationSpinBox;
    QLabel *mpGenerationLabel;
    CustomXDataDropEdit *mpCustomXDataDrop;
    QToolButton *mpResetTimeButton;

private slots:
    void actiavateCurve(bool active);
    void setXData(QString fullName);
    void resetTimeVector();
    void setGeneration(const int gen);
};


//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject, public QwtPlotCurve
{
    Q_OBJECT
    friend class CurveInfoBox;
    friend class PlotWindow;
public:
    enum {LegendShowLineAndSymbol=QwtPlotCurve::LegendShowBrush+1};

    PlotCurve(SharedLogVariableDataPtrT pData,
              int axisY=QwtPlot::yLeft,
              PlotTab *parent=0,
              HopsanPlotIDEnumT plotID=FirstPlot,
              HopsanPlotCurveTypeEnumT curveType=PortVariableType);

    PlotCurve(const VariableDescription &rVarDesc,
              const QVector<double> &rXVector,
              const QVector<double> &rYVector,
              int axisY=QwtPlot::yLeft,
              PlotTab *parent=0,
              HopsanPlotIDEnumT plotID=FirstPlot,
              HopsanPlotCurveTypeEnumT curveType=PortVariableType);
    ~PlotCurve();

    QString getCurveName() const;
    HopsanPlotCurveTypeEnumT getCurveType();
    int getAxisY();

    SharedLogVariableDataPtrT getLogDataVariablePtr(); //! @todo is this needed
    const SharedLogVariableDataPtrT getLogDataVariablePtr() const;
    QVector<double> getDataVector() const;
    const SharedLogVariableDataPtrT getTimeVectorPtr() const;
    bool hasCustomXData() const;
    const SharedLogVariableDataPtrT getCustomXData() const;

    int getGeneration() const;
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();

    void setCustomDataUnit(const QString unit);
    void setCustomDataUnit(const QString unit, double scale);
    void setTimePlotScalingAndOffset(double scale, double offset);
    void setValuePlotScalingAndOffset(double scale, double offset);

    void setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData);
    void setCustomXData(const VariableDescription &rVarDesc, const QVector<double> &rvXdata);
    void setCustomXData(SharedLogVariableDataPtrT pData);
    void setCustomXData(const QString fullName);

    void toFrequencySpectrum(const bool doPowerSpectrum=false);
    void resetLegendSize();

    // Qwt overloaded function
    QList<QwtLegendData> legendData() const;

signals:
    void curveDataUpdated();

public slots:
    bool setGeneration(int generation);
    void setPreviousGeneration();
    void setNextGeneration();

    void setLineWidth(int);
    void setLineStyle(QString lineStyle);
    void setLineSymbol(QString lineSymbol);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void openScaleDialog();
    void updateTimePlotScaleFromDialog();
    void updateValuePlotScaleFromDialog();
    void updateToNewGeneration();
    void updatePlotInfoBox();
    void removeMe();

    void setAutoUpdate(bool value);
    void performFrequencyAnalysis();
    void markActive(bool value);

private slots:
    void updateCurve();
    void updateCurveName();

private:
    // Private member functions
    void deleteCustomData();
    void connectDataSignals();
    void commonConstructorCode(int axisY, PlotTab *parent, HopsanPlotIDEnumT plotID, HopsanPlotCurveTypeEnumT curveType);

    // Curve data
    HopsanPlotCurveTypeEnumT mCurveType;
    SharedLogVariableDataPtrT mpData;
    SharedLogVariableDataPtrT mpCustomXdata;
    bool mHaveCustomData;

    QString mCustomDataUnit;
    double mCustomDataUnitScale;

    // Curve properties settings
    CurveInfoBox *mpPlotCurveInfoBox;
    bool mAutoUpdate;
    bool mIsActive;
    int mAxisY;
    QDoubleSpinBox *mpXScaleSpinBox;
    QDoubleSpinBox *mpXOffsetSpinBox;
    QDoubleSpinBox *mpYScaleSpinBox;
    QDoubleSpinBox *mpYOffsetSpinBox;
    PlotTab *mpParentPlotTab;

    // Line properties
    QColor mLineColor;
    QString mLineStyle;
    QString mLineSymbol;
    QwtSymbol *mpCurveSymbol;
    int mLineWidth;
    int mCurveSymbolSize;
};



class PlotMarker : public QObject, public QwtPlotMarker
{
    Q_OBJECT
public:
    PlotMarker(PlotCurve *pCurve, PlotTab *pPlotTab);
    PlotCurve *getCurve();
    virtual bool eventFilter (QObject *, QEvent *);
    void setMovable(bool movable);

public slots:
    void refreshLabel(const double x, const double y);
    void refreshLabel(const QString &label);

private:
    PlotCurve *mpCurve;
    PlotTab *mpPlotTab;
    QwtSymbol *mpMarkerSymbol;
    Qt::Alignment mLabelAlignment;
    int mMarkerSize;
    bool mIsBeingMoved;
    bool mIsMovable;
};

#endif // PLOTCURVE_H
