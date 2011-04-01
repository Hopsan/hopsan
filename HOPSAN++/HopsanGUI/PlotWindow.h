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

class MainWindow;
class VariablePlot;
class VariablePlotZoomer;
class PlotParameterTree;
class PlotWidget;
class GUISystem;
class PlotTabWidget;
class PlotTab;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotWidget;
    friend class VariableListWidget;
public:
    PlotWindow(PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit="", int axisY=QwtPlot::yLeft);
    void setGeneration(int gen);
    PlotTabWidget *getPlotTabWidget();
    PlotTab *getCurrentPlotTab();

    //MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentGUISystem;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void closeEvent(QCloseEvent *);

signals:
    void curveAdded();

public slots:
    void addPlotTab();
    void updatePortList();
    void updateVariableList();
    void addPlotCurveFromBoxes();
    void discardGeneration();
    void discardOldestGeneration();
    void importGNUPLOT();
    void enableGrid(bool);
    void enableZoom(bool);
    void enablePan(bool);
    void setBackgroundColor();
    void exportSVG();
    void exportGNUPLOT();
    void saveToXml();
    bool saveToHmpf(QString fileName);
    void close();
    void updatePalette();

private:
    QGridLayout *mpLayout;
    PlotParameterTree *mpPlotParameterTree;
    QPoint dragStartPosition;

    QToolBar *mpToolBar;
    QToolButton *mpNewPlotButton;
    QToolButton *mpZoomButton;
    QToolButton *mpPanButton;
    QToolButton *mpSaveButton;
    QToolButton *mpSVGButton;
    QToolButton *mpExportGNUPLOTButton;
    QToolButton *mpImportGNUPLOTButton;
    QToolButton *mpGridButton;
    QToolButton *mpBackgroundColorButton;

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
    //QSpinBox *mpGenerationSpinBox;
    QToolButton *mpColorButton;
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

private slots:
    void closePlotTab(int index);
    void tabChanged();
};


//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
    friend class PlotCurve;
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
    void setActiveMarker(QwtPlotMarker *marker);
    void changeXVector(QVector<double> xarray, QString componentName, QString portName, QString dataName, QString dataUnit);

    QVector<double> mVectorX;       //! @todo Should be private

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

public slots:
    void enableZoom(bool value);
    void enablePan(bool value);
    void enableGrid(bool value);
    void setBackgroundColor(QColor color);

private:
    QwtPlot *mpPlot;

    bool mHasLeftCurve;
    bool mHasRightCurve;

    QList<PlotCurve *> mPlotCurvePtrs;
    PlotCurve *mpActivePlotCurve;
    QVector<double> mSpecialXAxis;
    QColor mBackgroundColor;
    bool mShowGrid();
    QStringList mCurveColors;
    QStringList mUsedColors;
    QwtPlotGrid * mpGrid;
    QVector <QwtPlotMarker *> mpMarkers;
    QHash <QwtPlotCurve *, QwtPlotMarker *> mCurveToMarkerMap;
    QHash <QwtPlotMarker *, QwtPlotCurve *> mMarkerToCurveMap;
    QwtPlotCurve *tempCurve;
    QwtPlotZoomer *mpZoomer;
    QwtPlotZoomer *mpZoomerRight;
    QwtPlotMagnifier *mpMagnifier;
    QwtPlotPanner *mpPanner;

    QMap<QString, QString> mCurrentUnitsLeft;
    QMap<QString, QString> mCurrentUnitsRight;
    QwtSymbol *mpMarkerSymbol;
    QwtPlotMarker *mpActiveMarker;


    //QList< QList< QVector<double> > > mVectorY;

    QString mUnitLeft;
    QString mUnitRight;

    bool mHasSpecialXAxis;
    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;

    bool mAutoUpdate;

    QRubberBand *mpHoverRect;
};


//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject
{
    Q_OBJECT
    friend class PlotInfoBox;
public:
    PlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit="", int axisY=QwtPlot::yLeft, PlotTab *parent=0);
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
    void setGeneration(int generation);
    void setDataUnit();

public slots:
    void setLineWidth(int);
    void setLineColor(QColor color);
    void setLineColor(QString colorName=QString());
    void updateAtTabChange();
    void updateToNewGeneration();
    void updatePlotInfoBox();
    void removeMe();
    void setPreviousGeneration();
    void setNextGeneration();
    void setAutoUpdate(bool value);

private slots:
    void setActive(bool value);

private:
    QwtPlotCurve *mpCurve;
    int mGeneration;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QVector<double> mDataVector;
    QVector<double> mTimeVector;
    QColor mLineColor;
    int mLineWidth;
    int mAxisY;
    QDockWidget *mpPlotInfoDockWidget;
    PlotInfoBox *mpPlotInfoBox;
    bool mAutoUpdate;
};




#endif // PLOTWINDOW_H
