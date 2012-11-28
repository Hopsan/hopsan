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
//! @file   PlotWindow.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the PlotWindow class
//!
//$Id$

#ifndef PlotWindow_H
#define PlotWindow_H

#include <QtGui>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include "qwt_legend.h"

//#include "hop_qwt_plot_legenditem.h"
#include "qwt_plot_legenditem.h"

#include <QObject>
#include <QFile>
#include <QObject>
#include <QString>



#include "GUIObjects/GUIContainerObject.h"
#include "Dependencies/BarChartPlotter/barchartplotter.h"
#include "Dependencies/BarChartPlotter/axisbase.h"

class MainWindow;
class PlotVariableTree;
class PlotTreeWidget;
class SystemContainer;
class PlotTabWidget;
class PlotTab;
class PlotMarker;
class PlotCurve;

enum {AxisIdRole=QwtLegendData::UserRole+1};
//! @todo Merge this class with PlotLegend
class HopQwtPlotLegendItem : public QwtPlotLegendItem
{
private:
    QwtPlot::Axis mAxis;
    int mnItems;

public:
    HopQwtPlotLegendItem(QwtPlot::Axis axisId) :
        QwtPlotLegendItem()
    {
        mAxis = axisId;
    }

    void updateLegend( const QwtPlotItem *plotItem, const QList<QwtLegendData> &data )
    {
        // Use only those curve pointers that should belong to this particular legend
        QList<QwtLegendData> myData;
        for (int i=0; i<data.size(); ++i)
        {
            if (data[i].value(AxisIdRole) == mAxis)
            {
                myData.push_back(data[i]);
            }
        }

        QwtPlotLegendItem::updateLegend( plotItem, myData );
    }
};

class PlotLegend : public HopQwtPlotLegendItem
{
public:
    PlotLegend(QwtPlot::Axis axisId);
};


//! @todo we should merge this with plotcurve, and plotcurve should inherit QwtPlotCurve (containing this info)
class HopQwtPlotCurve : public QwtPlotCurve
{
public:
    HopQwtPlotCurve(QString label);
    QList<QwtLegendData> legendData() const;

};

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotTreeWidget;                //! @todo Should plot window really be friend with everything?
    friend class VariableListWidget;
    friend class PlotTabWidget;
    friend class PlotTab;
    friend class PlotCurve;
public:
    PlotWindow(PlotVariableTree *PlotVariableTree, MainWindow *parent);
    ~PlotWindow();
    void addPlotCurve(LogVariableData *pData, int axisY=QwtPlot::yLeft, QString modelPath = QString(), QColor desiredColor=QColor());
    void addBarChart(QStandardItemModel *pItemModel);

    PlotTabWidget *getPlotTabWidget();
    PlotTab *getCurrentPlotTab();
    void showHelpPopupMessage(QString message);
    void hideHelpPopupMessage();
    //SystemContainer *mpCurrentGUISystem;

signals:
    void curveAdded();

public slots:
    void changeXVector(QVector<double> xarray, VariableDescription &rVarDesc);
    void addPlotTab(QString requestedName=QString());
    void updatePalette();
    void createPlotWindowFromTab();
    void saveToXml();
    void ImportPlo();

    void loadFromXml();
    void performFrequencyAnalysis(PlotCurve *curve);
    void performFrequencyAnalysisFromDialog();


    void showFrequencyAnalysisHelp();
    void createBodePlot();
    void createBodePlotFromDialog();
    void createBodePlot(PlotCurve *pInputCurve, PlotCurve *pOutputCurve, int Fmax);
    void showToolBarHelpPopup();
    void closeIfEmpty();
    void hideCurveInfo();
    void setLegendsVisible(bool value);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QGridLayout *mpLayout;
    QGridLayout *mpInfoBoxLayout;
    PlotVariableTree *mpPlotVariableTree;
    QPointF dragStartPosition;

    QToolBar *mpToolBar;

    QAction *mpNewPlotButton;
    QAction *mpArrowButton;
    QAction *mpLegendButton;
    QAction *mpZoomButton;
    QAction *mpPanButton;
    QAction *mpSaveButton;
    QToolButton *mpExportButton;
    QToolButton *mpExportGfxButton;
    QAction *mpLoadFromXmlButton;
    QAction *mpGridButton;
    QAction *mpBackgroundColorButton;
    QAction *mpNewWindowFromTabButton;
    QAction *mpResetXVectorButton;

    QAction *mpShowCurveInfoButton;
    QAction *mpShowPlotWidgetButton;

//    QAction *mpShowLegendsAction;
    QAction *mpBodePlotButton;
    QMenu *mpExportMenu;
    QAction *mpExportToXmlAction;
    QAction *mpImportClassicData;
    QAction *mpExportToCsvAction;
    QAction *mpExportToHvcAction;
    QAction *mpExportToMatlabAction;
    QAction *mpExportToGnuplotAction;
    QAction *mpExportToOldHopAction;
    QMenu *mpExportGfxMenu;
    QAction *mpExportPdfAction;
    QAction *mpExportPngAction;
    QAction *mpExportToGraphicsAction;
    QAction *mpLocktheAxis;

    PlotTabWidget *mpPlotTabs;

    QMap<QRadioButton *, PlotCurve *> mBodeInputButtonToCurveMap;
    QMap<QRadioButton *, PlotCurve *> mBodeOutputButtonToCurveMap;

    //Help popup
    QWidget *mpHelpPopup;
    QLabel *mpHelpPopupIcon;
    QLabel *mpHelpPopupLabel;
    QHBoxLayout *mpHelpPopupLayout;
    QGroupBox *mpHelpPopupGroupBox;
    QHBoxLayout *mpHelpPopupGroupBoxLayout;
    QTimer *mpHelpPopupTimer;

    QDialog *mpCreateBodeDialog;
    QSlider *mpMaxFrequencySlider;



    QDialog *mpFrequencyAnalysisDialog;
    QCheckBox *mpLogScaleCheckBox;
    QCheckBox *mpPowerSpectrumCheckBox;
    PlotCurve *mpFrequencyAnalysisCurve;

    QWidget *mpPlotInfoWidget;
    QScrollArea *mpPlotInfoScrollArea;
    QVBoxLayout *mpPlotInfoLayout;
    QAbstractItemModel *model;
};


class PlotCurve;


class PlotInfoBox : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
public:
    PlotInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent);
    PlotCurve *mpParentPlotCurve;

private:
    QLabel *mpTitle;
    QToolButton *mpColorBlob;
    QToolButton *mpPreviousButton;
    QToolButton *mpNextButton;
    QToolButton *mpCloseButton;

    QGridLayout *mpInfBoxLayout;
    QSpinBox *mpSizeSpinBox;
    QComboBox *mpLineStyleCombo;
    QComboBox *mpLineSymbol;
    QToolButton *mpColorButton;
    QToolButton *mpFrequencyAnalysisButton;
    //QToolButton *mpSetAxisButton;
    QToolButton *mpScaleButton;
    QCheckBox *mpAutoUpdateCheckBox;
    QLabel *mpLabel;
    QLabel *mpSizeLabel;
    QLabel *mpGenerationLabel;

};


//! @brief Tab widget for plots in plot window
class PlotTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    PlotTabWidget(PlotWindow *parent);
    PlotWindow *mpParentPlotWindow;
    PlotTab *getCurrentTab();
    PlotTab *getTab(int i);

public slots:
    void closePlotTab(int index);
    void tabChanged();

};


//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotWindow;
    friend class PlotCurve;
    friend class PlotTabWidget;
    friend class PlotMarker;
    friend class QItemSelectionModel;
public:
    PlotTab(PlotWindow *parent);
    ~PlotTab();
    PlotWindow *mpParentPlotWindow;

    void setTabName(QString name);

    void addBarChart(QStandardItemModel *pItemModel);
    void addCurve(PlotCurve *curve, QColor desiredColor=QColor(), HopsanPlotID plotID=FIRSTPLOT);
    void rescaleToCurves();
    void removeCurve(PlotCurve *curve);
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
    VariableDescription mSpecialXVectorDescription;
    QString mSpecialXVectorModelPath; //!< @todo Maybe modelpath should be part of the description

    //Stuff used in export to xml dialog
    QDialog *mpExportXmlDialog;

    QSpinBox *mpXmlIndentationSpinBox;
    QCheckBox *mpIncludeTimeCheckBox;
    QCheckBox *mpIncludeDescriptionsCheckBox;
    QTextEdit *mpXmlOutputTextBox;

    bool mIsSpecialPlot;

    // Legend related member variables
    QwtLegend *mpExternalLegend;
    HopQwtPlotLegendItem *mpLeftPlotLegend, *mpRightPlotLegend;
    QCheckBox *mpLegendsInternalEnabledCheckBox;
    QCheckBox *mpLegendsExternalEnabledCheckBox;
    QCheckBox *mpLegendsOffEnabledCheckBox;
    //QCheckBox *mpLegendsOffYREnabledCheckBox;
    QDialog *mpLegendSettingsDialog;
    QComboBox *mpLegendLPosition;
    QComboBox *mpLegendRPosition;
    QComboBox *mpLegendBg;
    QComboBox *mpLegendSym;
    QSpinBox *mpLegendCol;
    QDoubleSpinBox *mpLegendOff;
    //QDoubleSpinBox *mpLegendOffYR;
    QSpinBox *mpLegendSize;
    QComboBox *mpLegendBlob;

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
    double bufferoffset;
    double bufferoffsetYR;
};


//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject
{
    Q_OBJECT
    friend class PlotInfoBox;
    friend class PlotWindow;
public:

    PlotCurve(LogVariableData *pData,
              int axisY=QwtPlot::yLeft,
              QString modelPath="",
              PlotTab *parent=0,
              HopsanPlotID plotID=FIRSTPLOT,
              HopsanPlotCurveType curveType=PORTVARIABLE);

    PlotCurve(const VariableDescription &rVarDesc,
              const QVector<double> &rXVector,
              const QVector<double> &rYVector,
              int axisY=QwtPlot::yLeft,
              QString modelPath="",
              PlotTab *parent=0,
              HopsanPlotID plotID=FIRSTPLOT,
              HopsanPlotCurveType curveType=PORTVARIABLE);
    ~PlotCurve();
    PlotTab *mpParentPlotTab;

    QString getCurveName() const;
    HopsanPlotCurveType getCurveType();
    int getAxisY();
    HopQwtPlotCurve *getQwtPlotCurvePtr();
    //QDockWidget *getPlotInfoDockWidget();

    LogVariableData *getPlotLogDataVariable();
    int getGeneration() const;
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();


    const QVector<double> &getDataVector() const;
    const QVector<double> &getTimeVector() const;
    ContainerObject *getContainerObjectPtr();

    void setGeneration(int generation);
    void setDataUnit(QString unit);
    void setScaling(double scaleX, double scaleY, double offsetX, double offsetY);

    void setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData);

    void toFrequencySpectrum();

public slots:

    void setLineWidth(int);
    void setLineStyle(QString);
    void setLineSymbol(QString);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void openScaleDialog();
    void updatePlotInfoVisibility();
    void updateScaleFromDialog();
    void updateToNewGeneration();
    void updatePlotInfoBox();
    void removeMe();
    void removeIfNotConnected();
    void setPreviousGeneration();
    void setNextGeneration();
    void setAutoUpdate(bool value);
    void performFrequencyAnalysis();
    //void performSetAxis();

private slots:
    void setActive(bool value);
    void updateCurve();
    void updateCurveName();

private:
    LogVariableData *mpData;
    PlotInfoBox *mpPlotInfoBox;

    HopsanPlotCurveType mCurveType;
    HopQwtPlotCurve *mpQwtPlotCurve;

    ContainerObject *mpContainerObject;

    QColor mLineColor;
    QString mLineStyle;
    QString mLineSymbol;
    int mLineWidth;
    int mAxisY;

    bool mAutoUpdate;
    bool mHaveCustomData;
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
    void commonConstructorCode(int axisY, QString modelPath, PlotTab *parent, HopsanPlotID plotID, HopsanPlotCurveType curveType);
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



#endif // PLOTWINDOW_H
