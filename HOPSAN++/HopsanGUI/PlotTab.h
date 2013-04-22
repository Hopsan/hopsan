#ifndef PLOTTAB_H
#define PLOTTAB_H

#include <QTabWidget>
#include <QObject>
#include <QStandardItemModel>
#include <QtXml>

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
#include "Dependencies/BarChartPlotter/barchartplotter.h"
#include "Dependencies/BarChartPlotter/axisbase.h"
#include "common.h"

#include "LogVariable.h"

//Forward Declarations
class PlotWindow;
class PlotTabWidget;
class PlotCurve;
class PlotMarker;
class PlotLegend;
class PainterWidget;

class HopQwtPlot : public QwtPlot
{
    Q_OBJECT
public:
    HopQwtPlot(QWidget *pParent=0);
signals:
    void resized();
protected:
    void resizeEvent( QResizeEvent *e );
};

//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotCurve;
    friend class PlotTabWidget;
    friend class PlotMarker;

public:
    PlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow);
    ~PlotTab();
    PlotTabWidget *mpParentPlotTabWidget;   //!< @todo should not be public
    PlotWindow *mpParentPlotWindow;         //!< @todo should not be public

    void setTabName(QString name);

    void addCurve(PlotCurve *curve, QColor desiredColor=QColor(), HopsanPlotIDEnumT plotID=FirstPlot);
    void setCustomXVectorForAll(QVector<double> xarray, const VariableDescription &rVarDesc, HopsanPlotIDEnumT plotID=FirstPlot);
    void setCustomXVectorForAll(SharedLogVariableDataPtrT pData, HopsanPlotIDEnumT plotID=FirstPlot);
    void removeCurve(PlotCurve *curve);
    void removeAllCurvesOnAxis(const int axis);

    QList<PlotCurve *> getCurves(HopsanPlotIDEnumT plotID=FirstPlot);
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    QwtPlot *getPlot(HopsanPlotIDEnumT plotID=FirstPlot);

    int getNumberOfCurves(HopsanPlotIDEnumT plotID);
    bool isGridVisible();
    bool isSpecialPlot();
    bool hasLogarithmicBottomAxis();
    bool isZoomed(const int plotId) const;

    void showPlot(HopsanPlotIDEnumT plotID, bool visible);
    void setBottomAxisLogarithmic(bool value);
    void setLegendsVisible(bool value);

    void update();
    void updateLabels();

    void saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions);
    void exportToCsv(QString fileName);

protected:
    void addBarChart(QStandardItemModel *pItemModel);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *);

public slots:
    void rescaleAxesToCurves();

    void openLegendSettingsDialog();
    void openAxisSettingsDialog();
    void applyAxisSettings();
    void applyLegendSettings();
    void enableZoom(bool value);
    void resetZoom();
    void enableArrow(bool value);
    void enablePan(bool value);
    void enableGrid(bool value);
    void setBackgroundColor();
    void resetXTimeVector();
    void exportToXml();
    void exportToCsv();
    void exportToHvc(QString fileName="");
    void exportToMatlab();
    void exportToGnuplot();
    void exportToPdf();
    void exportToGraphics();
    void exportToOldHop();
    void exportToPng();

    void shiftAllGenerationsDown();
    void shiftAllGenerationsUp();

    void insertMarker(PlotCurve *pCurve, double x, double y, QString altLabel=QString(), bool movable=true);
    void insertMarker(PlotCurve *pCurve, QPoint pos, bool movable=true);

private slots:
    QString updateXmlOutputTextInDialog();
    void saveToXml();

    void exportImage();
    void changedGraphicsExportSettings();

private:
    int getPlotIDFromCurve(PlotCurve *pCurve);
    void constructLegendSettingsDialog();
    void constructAxisSettingsDialog();
    void setLegendSymbol(const QString symStyle);
    void setTabOnlyCustomXVector(SharedLogVariableDataPtrT pData, HopsanPlotIDEnumT plotID=FirstPlot);

    QGridLayout *mpTabLayout;
    QSint::BarChartPlotter *mpBarPlot;

    QScrollArea *mpCurveInfoScrollArea;

    QwtPlot *mpQwtPlots[2];
    QList<PlotCurve *> mPlotCurvePtrs[2];
    QwtPlotGrid *mpGrid[2];
    QwtPlotZoomer *mpZoomerLeft[2];
    QwtPlotZoomer *mpZoomerRight[2];
    QwtPlotMagnifier *mpMagnifier[2];
    QwtPlotPanner *mpPanner[2];
    QList<PlotMarker *> mMarkerPtrs[2];
    PlotCurve *mpActivePlotCurve;
    QStringList mCurveColors;
    QStringList mUsedColors;
    PainterWidget *mpPainterWidget;

    bool mIsSpecialPlot;

    // Custom X data axis variables
    bool mHasCustomXData;
    QString mCustomXDataLabel;
    SharedLogVariableDataPtrT mpCustomXData;

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


    // Legend related member variables
    QwtLegend *mpExternalLegend;
    PlotLegend *mpLeftPlotLegend, *mpRightPlotLegend;
    QCheckBox *mpLegendsInternalEnabledCheckBox;
    QCheckBox *mpLegendsExternalEnabledCheckBox;
    QCheckBox *mpLegendsAutoOffsetCheckBox;
    //QCheckBox *mpLegendsOffYREnabledCheckBox;
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

    // Axis settings related member variables
    QDialog *mpSetAxisDialog;
    QCheckBox *mpXAutoCheckBox;
    QCheckBox *mpYLAutoCheckBox;
    QCheckBox *mpYRAutoCheckBox;
    QDoubleSpinBox *mpXminSpinBox;
    QDoubleSpinBox *mpXmaxSpinBox;
    QDoubleSpinBox *mpYLminSpinBox;
    QDoubleSpinBox *mpYLmaxSpinBox;
    QDoubleSpinBox *mpYRminSpinBox;
    QDoubleSpinBox *mpYRmaxSpinBox;

    typedef struct _AxisLimits // Persistent axis limits
    {
        double min, max;
    }AxisLimitsT;
    AxisLimitsT mXAxisLimits[2];
    AxisLimitsT mYLAxisLimits[2];
    AxisLimitsT mYRAxisLimits[2];

    void rescaleAxistoIncludeLegendBufferOffset(const int plotId, const QwtPlot::Axis axisId, const AxisLimitsT &rLegendBufferOffset, AxisLimitsT &rAxisLimits);
    void calculateLegendBufferOffsets(const int plotId, AxisLimitsT &rLeftLegendOffset, AxisLimitsT &rRightLegendOffset);
};


class PainterWidget : public QWidget
{
public:
    PainterWidget(QWidget *parent=0);
    void createRect(int x, int y, int w, int h);
    void clearRect();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    int mX, mY, mWidth, mHeight;
    bool mErase;
};


#endif // PLOTTAB_H
