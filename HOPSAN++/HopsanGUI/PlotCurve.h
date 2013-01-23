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


//! @todo we should merge this with plotcurve, and plotcurve should inherit QwtPlotCurve (containing this info)
class HopQwtPlotCurve : public QwtPlotCurve
{
public:
    enum {LegendShowLineAndSymbol=QwtPlotCurve::LegendShowBrush+1};

    HopQwtPlotCurve(QString label);
    QList<QwtLegendData> legendData() const;
};


class PlotCurveInfoBox : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
public:
    PlotCurveInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent);
    void setLineColor(const QColor color);
    void updateInfo();

private:
    void refreshTitle();
    void refreshActive(bool active);
    PlotCurve *mpParentPlotCurve;
    QLabel *mpTitle;
    QToolButton *mpColorBlob;
    QToolButton *mpPreviousButton;
    QToolButton *mpNextButton;
    QLabel *mpGenerationLabel;

private slots:
    void actiavateCurve(bool active);

};





//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject
{
    Q_OBJECT
    friend class PlotCurveInfoBox;
    friend class PlotWindow;
public:

    PlotCurve(SharedLogVariableDataPtrT pData,
              int axisY=QwtPlot::yLeft,
              PlotTab *parent=0,
              HopsanPlotID plotID=FIRSTPLOT,
              HopsanPlotCurveType curveType=PORTVARIABLE);

    PlotCurve(const SharedVariableDescriptionT &rVarDesc,
              const QVector<double> &rXVector,
              const QVector<double> &rYVector,
              int axisY=QwtPlot::yLeft,
              PlotTab *parent=0,
              HopsanPlotID plotID=FIRSTPLOT,
              HopsanPlotCurveType curveType=PORTVARIABLE);
    ~PlotCurve();
    PlotTab *mpParentPlotTab;

    QString getCurveName() const;
    HopsanPlotCurveType getCurveType();
    int getAxisY();
    HopQwtPlotCurve *getQwtPlotCurvePtr();

    int getGeneration() const;
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();

    SharedLogVariableDataPtrT getLogDataVariablePtr(); //! @todo is this needed
    const SharedLogVariableDataPtrT getLogDataVariablePtr() const;
    QVector<double> getDataVector() const;
    const QVector<double> &getTimeVector() const;

    void setGeneration(int generation);
    void setDataUnit(QString unit);
    void setScaling(double scaleX, double scaleY, double offsetX, double offsetY);

    void setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData);

    void toFrequencySpectrum();

    void resetLegendSize();

public slots:

    void setLineWidth(int);
    void setLineStyle(QString lineStyle);
    void setLineSymbol(QString lineSymbol);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void openScaleDialog();
    void updatePlotCurveInfoVisibility();
    void updateScaleFromDialog();
    void updateToNewGeneration();
    void updatePlotInfoBox();
    void removeMe();
    void setPreviousGeneration();
    void setNextGeneration();
    void setAutoUpdate(bool value);
    void performFrequencyAnalysis();
    //void performSetAxis();
    void markActive(bool value);

private slots:
    void updateCurve();
    void updateCurveName();

private:
    SharedLogVariableDataPtrT mpData;
    PlotCurveInfoBox *mpPlotCurveInfoBox;

    HopsanPlotCurveType mCurveType;
    HopQwtPlotCurve *mpQwtPlotCurve;

    QColor mLineColor;
    QString mLineStyle;
    QString mLineSymbol;
    int mLineWidth;
    int mAxisY;

    bool mAutoUpdate;
    bool mHaveCustomData;
    bool mIsActive;
    double mScaleX;
    double mScaleY;
    double mOffsetX;
    double mOffsetY;

    QwtSymbol *mpCurveSymbol;

    QDoubleSpinBox *mpXScaleSpinBox;
    QDoubleSpinBox *mpXOffsetSpinBox;
    QDoubleSpinBox *mpYScaleSpinBox;
    QDoubleSpinBox *mpYOffsetSpinBox;

    void deleteCustomData();
    void connectDataSignals();
    void commonConstructorCode(int axisY, PlotTab *parent, HopsanPlotID plotID, HopsanPlotCurveType curveType);
};



class PlotMarker : public QObject, public QwtPlotMarker
{
    Q_OBJECT
public:
    PlotMarker(PlotCurve *pCurve, PlotTab *pPlotTab, QwtSymbol *markerSymbol);
    PlotCurve *getCurve();
    virtual bool eventFilter (QObject *, QEvent *);
    void setMovable(bool movable);

private:
    PlotCurve *mpCurve;
    PlotTab *mpPlotTab;
    bool mIsBeingMoved;
    bool mIsMovable;
    QwtSymbol *mpMarkerSymbol;
};

#endif // PLOTCURVE_H
