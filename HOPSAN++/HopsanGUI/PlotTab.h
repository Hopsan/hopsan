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

class HopQwtPlotCurve;
class PlotLegend;

//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotCurve;
    friend class PlotTabWidget;
    friend class PlotMarker;

public:
    PlotTab(PlotTabWidget *pParentPlotTabWidget);
    ~PlotTab();
    PlotTabWidget *mpParentPlotTabWidget;
    PlotWindow *mpParentPlotWindow;

    void setTabName(QString name);

    void addCurve(PlotCurve *curve, QColor desiredColor=QColor(), HopsanPlotID plotID=FIRSTPLOT);
    void rescaleToCurves();
    void removeCurve(PlotCurve *curve);
    void removeAllCurvesOnAxis(const int axis);
    QList<PlotCurve *> getCurves(HopsanPlotID plotID=FIRSTPLOT);
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    QwtPlot *getPlot(HopsanPlotID plotID=FIRSTPLOT);

    void showPlot(HopsanPlotID plotID, bool visible);
    int getNumberOfCurves(HopsanPlotID plotID);
    void update();
    void insertMarker(HopQwtPlotCurve *curve);
    void changeXVector(QVector<double> xarray, const VariableDescription &rVarDesc, HopsanPlotID plotID=FIRSTPLOT);
    void updateLabels();
    bool isGridVisible();
    void saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions);
    bool isSpecialPlot();
    void setBottomAxisLogarithmic(bool value);
    bool hasLogarithmicBottomAxis();
    void setLegendsVisible(bool value);
    void exportToCsv(QString fileName);

protected:
    void addBarChart(QStandardItemModel *pItemModel);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *);

public slots:
    void openLegendSettingsDialog();
    void openAxisSettingsDialog();
    void applyAxisSettings();
    void applyLegendSettings();
    void enableZoom(bool value);
    void enableArrow(bool value);
    void enablePan(bool value);
    void enableGrid(bool value);
    void setBackgroundColor();
    void resetXVector();
    void exportToXml();
    void exportToCsv();
    void exportToHvc(QString fileName="");
    void exportToMatlab();
    void exportToGnuplot();
    void exportToPdf();
    void exportToGraphics();
    //void applyGraphicsSettings();
    void exportToOldHop();
    void exportToPng();

    void insertMarker(PlotCurve *pCurve, double x, double y, QString altLabel=QString(), bool movable=true);
    void insertMarker(PlotCurve *pCurve, QPoint pos, bool movable=true);

private slots:
    QString updateXmlOutputTextInDialog();
    void saveToXml();

private:
    int getPlotIDFromCurve(PlotCurve *pCurve);
    void constructLegendSettingsDialog();
    void constructAxisSettingsDialog();
    void setLegendSymbol(const QString symStyle);

    QwtPlot *mpQwtPlots[2];
    QSint::BarChartPlotter *mpBarPlot;

    QGridLayout *mpLayouta;

    QList<PlotCurve *> mPlotCurvePtrs[2];
    PlotCurve *mpActivePlotCurve;
    QStringList mCurveColors;
    QStringList mUsedColors;
    QwtPlotGrid *mpGrid[2];
    QwtPlotZoomer *mpZoomer[2];
    QwtPlotZoomer *mpZoomerRight[2];
    QwtPlotMagnifier *mpMagnifier[2];
    QwtPlotPanner *mpPanner[2];
    QList<PlotMarker *> mMarkerPtrs[2];
    QMap<QString, QString> mCurrentUnitsLeft;
    QMap<QString, QString> mCurrentUnitsRight;
    QwtSymbol *mpMarkerSymbol;


    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;
    bool mBottomAxisLogarithmic;


    QRubberBand *mpHoverRect;

    bool mHasSpecialXAxis;
    QVector<double> mSpecialXVector;
    QString mSpecialXVectorLabel;
    SharedVariableDescriptionT mSpecialXVectorDescription;

    //Stuff used in export to xml dialog
    QDialog *mpExportXmlDialog;

    QSpinBox *mpXmlIndentationSpinBox;
    QCheckBox *mpIncludeTimeCheckBox;
    QCheckBox *mpIncludeDescriptionsCheckBox;
    QTextEdit *mpXmlOutputTextBox;

    bool mIsSpecialPlot;

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


    QDialog *mpGraphicsSettingsDialog;
    QSpinBox *mpGraphicsSize;
    QSpinBox *mpGraphicsSizeW;
    QSpinBox *mpGraphicsQuality;
    QComboBox *mpGraphicsForm;

    // Axis settings related member variables
    QDialog *mpSetAxisDialog;
    QCheckBox *mpXbSetLockCheckBox;
    QCheckBox *mpYLSetLockCheckBox;
    QCheckBox *mpYRSetLockCheckBox;
    QDoubleSpinBox *mpXminSpinBox;
    QDoubleSpinBox *mpXmaxSpinBox;
    QDoubleSpinBox *mpYLminSpinBox;
    QDoubleSpinBox *mpYLmaxSpinBox;
    QDoubleSpinBox *mpYRminSpinBox;
    QDoubleSpinBox *mpYRmaxSpinBox;
    typedef struct _AxisLimits
    {
        double xbMin, xbMax;
        double yLMin, yLMax;
        double yRMin, yRMax;
    }AxisLimitsT;
    AxisLimitsT mAxisLimits[2];     // Persistent axis limits
};

#endif // PLOTTAB_H
