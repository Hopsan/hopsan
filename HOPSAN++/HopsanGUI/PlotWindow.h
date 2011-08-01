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

#include "GUIObjects/GUIContainerObject.h"

class MainWindow;
class PlotParameterTree;
class PlotWidget;
class GUISystem;
class PlotTabWidget;
class PlotTab;
class PlotMarker;
class PlotCurve;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotWidget;                //! @todo Should plot window really be friend with everything?
    friend class VariableListWidget;
    friend class PlotTabWidget;
    friend class PlotTab;
    friend class PlotCurve;
public:
    PlotWindow(PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit="", int axisY=QwtPlot::yLeft, QString modelPath = QString());
    void setGeneration(int gen);
    PlotTabWidget *getPlotTabWidget();
    PlotTab *getCurrentPlotTab();

    GUISystem *mpCurrentGUISystem;

signals:
    void curveAdded();

public slots:
    void addPlotTab();
    void updateLists();
    void updatePortList();
    void updateVariableList();
    void addPlotCurveFromBoxes();
    void close();
    void updatePalette();
    void createPlotWindowFromTab();
    void saveToXml();
    void loadFromXml();
    void performFrequencyAnalysis(PlotCurve *curve);
    void createBodePlot();
    void createBodePlotFromDialog();
    void createBodePlot(PlotCurve *pInputCurve, PlotCurve *pOutputCurve);

private:
    QGridLayout *mpLayout;
    PlotParameterTree *mpPlotParameterTree;
    QPointF dragStartPosition;

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
    QToolButton *mpShowListsButton;
    QToolButton *mpShowCurvesButton;
    QToolButton *mpBodePlotButton;
    QMenu *mpExportMenu;
    QAction *mpExportToXmlAction;
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

    QMap<QRadioButton *, PlotCurve *> mBodeInputButtonToCurveMap;
    QMap<QRadioButton *, PlotCurve *> mBodeOutputButtonToCurveMap;
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
    QToolButton *mpFrequencyAnalysisButton;
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

private slots:
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
public:
    PlotTab(PlotWindow *parent);
    ~PlotTab();
    PlotWindow *mpParentPlotWindow;

    void addCurve(PlotCurve *curve, HopsanPlotID plotID=FIRSTPLOT);
    void rescaleToCurves();
    void removeCurve(PlotCurve *curve);
    QList<PlotCurve *> getCurves(HopsanPlotID plotID=FIRSTPLOT);
    void setActivePlotCurve(PlotCurve *pCurve);
    PlotCurve *getActivePlotCurve();
    QwtPlot *getPlot(HopsanPlotID plotID=FIRSTPLOT);
    void showPlot(HopsanPlotID plotID, bool visible);
    int getNumberOfCurves(HopsanPlotID plotID);
    void update();
    void insertMarker(QwtPlotCurve *curve);
    void changeXVector(QVector<double> xarray, QString componentName, QString portName, QString dataName, QString dataUnit, HopsanPlotID plotID=FIRSTPLOT);
    void updateLabels();
    bool isGridVisible();
    void saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions);

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
    void exportToXml();
    void exportToCsv();
    void exportToMatlab();
    void exportToGnuplot();
    void exportToPdf();
    void exportToPng();
    void insertMarker(PlotCurve *pCurve, QPoint pos);

private slots:
    QString updateXmlOutputTextInDialog();
    void saveToXml();

private:
    QwtPlot *mpPlot[2];

    QList<PlotCurve *> mPlotCurvePtrs[2];
    PlotCurve *mpActivePlotCurve;
    QStringList mCurveColors;
    QStringList mUsedColors;
    QwtPlotGrid *mpGrid[2];
    QwtPlotZoomer *mpZoomer[2];
    QwtPlotZoomer *mpZoomerRight[2];
    QwtPlotMagnifier *mpMagnifier[2];
    QwtPlotPanner *mpPanner[2];
    QList<PlotMarker *> mMarkerPtrs;
    QMap<QString, QString> mCurrentUnitsLeft;
    QMap<QString, QString> mCurrentUnitsRight;
    QwtSymbol *mpMarkerSymbol;

    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;

    QRubberBand *mpHoverRect;

    bool mHasSpecialXAxis;
    QVector<double> mVectorX;
    QString mVectorXLabel;

    QString mVectorXModelPath;
    QString mVectorXComponent;
    QString mVectorXPortName;
    QString mVectorXDataName;
    QString mVectorXDataUnit;
    int mVectorXGeneration;

    //Stuff used in export to xml dialog
    QDialog *mpExportXmlDialog;
    QSpinBox *mpXmlIndentationSpinBox;
    QCheckBox *mpIncludeTimeCheckBox;
    QCheckBox *mpIncludeDescriptionsCheckBox;
    QTextEdit *mpXmlOutputTextBox;
};


//! @brief Class describing a plot curve in plot window
class PlotCurve : public QObject
{
    Q_OBJECT
    friend class PlotInfoBox;
public:
    PlotCurve(int generation, QString componentName, QString portName, QString dataName, QString dataUnit="", int axisY=QwtPlot::yLeft, QString modelPath="", PlotTab *parent=0, HopsanPlotID plotID=FIRSTPLOT, HopsanPlotCurveType curveType=PORTVARIABLE);
    ~PlotCurve();
    PlotTab *mpParentPlotTab;

    int getGeneration();
    HopsanPlotCurveType getCurveType();
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
    void setData(QVector<double> vData, QVector<double> vTime);
    void toFrequencySpectrum();

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
    void removeIfNotConnected();
    void setPreviousGeneration();
    void setNextGeneration();
    void setAutoUpdate(bool value);
    void performFrequencyAnalysis();

private slots:
    void setActive(bool value);
    void updateCurve();

private:
    HopsanPlotCurveType mCurveType;
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
