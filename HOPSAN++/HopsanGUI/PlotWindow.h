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

#include "GUIObjects/GUIContainerObject.h"

class MainWindow;
class VariablePlot;
class VariablePlotZoomer;
class PlotParameterTree;
class PlotWidget;
class GUISystem;
class PlotTabWidget;
class PlotTab;
class PlotMarker;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotWidget;
    friend class VariableListWidget;
    friend class PlotTabWidget;     //! @todo Not nice...
    friend class PlotTab;           //! @todo Not nice at all...
public:
    enum PlotWindowItems {ShowAll, ShowOnlyLists, ShowOnlyCurves, ShowOnlyPlot};

    PlotWindow(PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit="", int axisY=QwtPlot::yLeft, QString modelPath = QString());
    void setGeneration(int gen);
    PlotTabWidget *getPlotTabWidget();
    PlotTab *getCurrentPlotTab();

    //MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentGUISystem;

    QToolButton *mpShowListsButton;     //! @todo Should not be public?
    QToolButton *mpShowCurvesButton;    //! @todo Should not be public?

signals:
    void curveAdded();

public slots:
    void addPlotTab();
    void updateLists();
    void updatePortList();
    void updateVariableList();
    void addPlotCurveFromBoxes();
    void importGNUPLOT();
    void close();
    void updatePalette();
    void createPlotWindowFromTab();
    void saveToXml();
    void loadFromXml();

private:
    QGridLayout *mpLayout;
    PlotParameterTree *mpPlotParameterTree;
    QPoint dragStartPosition;

    QToolBar *mpToolBar;

    QToolButton *mpNewPlotButton;
    QToolButton *mpZoomButton;
    QToolButton *mpPanButton;
    QToolButton *mpSaveButton;
    QToolButton *mpExportButton;
    QToolButton *mpExportGfxButton;
    QToolButton *mpLoadFromXmlButton;
    QToolButton *mpGridButton;
    QToolButton *mpBackgroundColorButton;
    QToolButton *mpNewWindowFromTabButton;
    QToolButton *mpResetXVectorButton;
    QMenu *mpExportMenu;
    QAction *mpExportToCsvAction;
    QAction *mpExportToMatlabAction;
    QAction *mpExportToGnuplotAction;
    QMenu *mpExportGfxMenu;
    QAction *mpExportPdfAction;
    QAction *mpExportPngAction;

    PlotTabWidget *mpPlotTabs;
    QLabel *mpComponentsLabel;
    QLabel *mpPortsLabel;
    QLabel *mpVariablesLabel;
    QListWidget *mpComponentList;
    QListWidget *mpPortList;
    QListWidget *mpVariableList;
};


class VariableListWidget : public QListWidget
{
    Q_OBJECT
public:
    VariableListWidget(PlotWindow *parentPlotWindow, QWidget *parent=0);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    PlotWindow *mpParentPlotWindow;
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
    QToolButton *mpColorBlob;
    QToolButton *mpPreviousButton;
    QToolButton *mpNextButton;
    QToolButton *mpCloseButton;

    QGridLayout *mpLayout;
    QSpinBox *mpSizeSpinBox;
    QToolButton *mpColorButton;
    QToolButton *mpScaleButton;
    QCheckBox *mpAutoUpdateCheckBox;
    QLabel *mpLabel;
    QLabel *mpSizeLabel;
    QLabel *mpGenerationLabel;
};





//! @brief Tab widget for plots in plot window
//! @todo Not sure if this is needed
class PlotTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    PlotTabWidget(PlotWindow *parent);
    PlotWindow *mpParentPlotWindow;
    PlotTab *getCurrentTab();
    PlotTab *getTab(int i);

private slots:
    void closePlotTab(int index);
    void tabChanged();
};


//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
    friend class PlotTabWidget;
    friend class PlotMarker;
public:
    PlotTab(PlotWindow *parent);
    ~PlotTab();
    PlotWindow *mpParentPlotWindow;

    void addCurve(PlotCurve *curve);
    void rescaleToCurves();
    void removeCurve(PlotCurve *curve);
    QList<PlotCurve *> getCurves();
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    QwtPlot *getPlot();
    int getNumberOfCurves();
    void update();
    void insertMarker(QwtPlotCurve *curve);
    void changeXVector(QVector<double> xarray, QString componentName, QString portName, QString dataName, QString dataUnit);
    void updateLabels();
    bool isGridVisible();

    bool mHasSpecialXAxis;          //! @todo Should be private
    QVector<double> mVectorX;       //! @todo Should be private
    QString mVectorXLabel;          //! @todo Should be private

    QString mVectorXModelPath;      //! @todo Should be private
    QString mVectorXComponent;      //! @todo Should be private
    QString mVectorXPortName;       //! @todo Should be private
    QString mVectorXDataName;       //! @todo Should be private
    QString mVectorXDataUnit;       //! @todo Should be private
    int mVectorXGeneration;         //! @todo Should be private

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *);

public slots:
    void enableZoom(bool value);
    void enablePan(bool value);
    void enableGrid(bool value);
    void setBackgroundColor();
    void resetXVector();
    void exportToCsv();
    void exportToMatlab();
    void exportToGnuplot();
    void exportToPdf();
    void exportToPng();
    void insertMarker(PlotCurve *pCurve, QPoint pos);

private:
    QwtPlot *mpPlot;

    QList<PlotCurve *> mPlotCurvePtrs;
    PlotCurve *mpActivePlotCurve;
    bool mShowGrid();
    QStringList mCurveColors;
    QStringList mUsedColors;
    QwtPlotGrid * mpGrid;
    QwtPlotZoomer *mpZoomer;
    QwtPlotZoomer *mpZoomerRight;
    QwtPlotMagnifier *mpMagnifier;
    QwtPlotPanner *mpPanner;
    QList<PlotMarker *> mMarkerPtrs;
    QMap<QString, QString> mCurrentUnitsLeft;
    QMap<QString, QString> mCurrentUnitsRight;
    QwtSymbol *mpMarkerSymbol;

    QString mUnitLeft;
    QString mUnitRight;

    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;

    QRubberBand *mpHoverRect;
};


//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject
{
    Q_OBJECT
    friend class PlotInfoBox;
public:
    PlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit="", int axisY=QwtPlot::yLeft, QString modelPath="", PlotTab *parent=0);
    ~PlotCurve();
    PlotTab *mpParentPlotTab;

    int getGeneration();
    QwtPlotCurve *getCurvePtr();
    QDockWidget *getPlotInfoDockWidget();
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();
    int getAxisY();
    QVector<double> getDataVector();
    QVector<double> getTimeVector();
    GUIContainerObject *getContainerObjectPtr();
    void setGeneration(int generation);
    void setDataUnit(QString unit);
    void setScaling(double scaleX, double scaleY, double offsetX, double offsetY);

public slots:
    void setLineWidth(int);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void openScaleDialog();
    void updatePlotInfoDockVisibility();
    void updateScaleFromDialog();
    void updateToNewGeneration();
    void updatePlotInfoBox();
    void removeMe();
    void setPreviousGeneration();
    void setNextGeneration();
    void setAutoUpdate(bool value);

private slots:
    void setActive(bool value);
    void updateCurve();

private:
    QwtPlotCurve *mpCurve;
    int mGeneration;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    GUIContainerObject *mpContainerObject;
    QVector<double> mDataVector;
    QVector<double> mTimeVector;
    QColor mLineColor;
    int mLineWidth;
    int mAxisY;
    QDockWidget *mpPlotInfoDockWidget;
    PlotInfoBox *mpPlotInfoBox;
    bool mAutoUpdate;
    double mScaleX;
    double mScaleY;
    double mOffsetX;
    double mOffsetY;

    QDoubleSpinBox *mpXScaleSpinBox;
    QDoubleSpinBox *mpXOffsetSpinBox;
    QDoubleSpinBox *mpYScaleSpinBox;
    QDoubleSpinBox *mpYOffsetSpinBox;
};



class PlotMarker : public QObject, public QwtPlotMarker
{
    Q_OBJECT
public:
    PlotMarker(PlotCurve *pCurve, PlotTab *pPlotTab, QwtSymbol markerSymbol);
    PlotCurve *getCurve();
    virtual bool eventFilter (QObject *, QEvent *);

private:
    PlotCurve *mpCurve;
    PlotTab *mpPlotTab;
    bool mIsBeingMoved;
    QwtSymbol mMarkerSymbol;
};



#endif // PLOTWINDOW_H
