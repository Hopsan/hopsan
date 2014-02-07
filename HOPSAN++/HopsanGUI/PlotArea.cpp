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
//! @file   PlotArea.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014
//!
//! @brief Contains a class for plot tabs areas
//!
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

#include <QLineEdit>
#include <QGridLayout>

#include "PlotArea.h"
#include "PlotCurve.h"
#include "PlotCurveControlBox.h"
#include "PlotTab.h"
#include "global.h"
#include "Configuration.h"
#include "Utilities/GUIUtilities.h"
#include "ModelHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "PlotWindow.h"
#include "MessageHandler.h"

//Qwt includes
#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_map.h"
#include "qwt_scale_widget.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include <qwt_dyngrid_layout.h>
#include "qwt_plot_zoomer.h"

#include <limits>
const double DoubleMax = std::numeric_limits<double>::max();
const double DoubleMin = std::numeric_limits<double>::min();
const double Double100Min = 100*DoubleMin;

//! @brief Rectangle painter widget, used for painting transparent rectangles when dragging things to plot tabs
class RectanglePainterWidget : public QWidget
{
public:
    RectanglePainterWidget(QWidget *parent=0);
    void createRect(int x, int y, int w, int h);
    void clearRect();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    int mX, mY, mWidth, mHeight;
    bool mErase;
};


RectanglePainterWidget::RectanglePainterWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}


//! @brief Creates a rectangle with specified dimensions
void RectanglePainterWidget::createRect(int x, int y, int w, int h)
{
    mX = x;
    mY = y;
    mWidth = w;
    mHeight = h;
    mErase=false;
    update();
}


//! @brief Removes any previously drawn rectangles
void RectanglePainterWidget::clearRect()
{
    mErase=true;
    update();
}


//! @brief Paint event, does the actual drawing
void RectanglePainterWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(!mErase)
    {
        QBrush  brush(Qt::blue);		// yellow solid pattern
        painter.setBrush( brush );		// set the yellow brush
        painter.setOpacity(0.5);
        painter.setPen( Qt::NoPen );		// do not draw outline
        painter.drawRect(mX,mY,mWidth,mHeight);	// draw filled rectangle
    }
}

//! @brief Extend interval min even if it is invalid
void HopQwtInterval::extendMin(const double value)
{
    setMinValue(qMin(value,minValue()));
}

//! @brief Extend interval max even if it is invalid
void HopQwtInterval::extendMax(const double value)
{
    setMaxValue(qMax(value,maxValue()));
}


void HopQwtPlot::replot()
{
    QwtPlot::replot();
    emit afterReplot();
}


TimeScaleWidget::TimeScaleWidget(SharedVectorVariableT pTime, QWidget *pParent) : QWidget(pParent)
{
    mpTime = pTime;

    QHBoxLayout *pHBoxLayout = new QHBoxLayout(this);
    mpTimeScaleComboBox = new QComboBox(this);
    mpTimeOffsetLineEdit = new QLineEdit(this);
    mpTimeOffsetLineEdit->setValidator(new QDoubleValidator(this));

    pHBoxLayout->addWidget(new QLabel("Time", this));
    pHBoxLayout->addWidget(new QLabel("Scale: ", this));
    pHBoxLayout->addWidget(mpTimeScaleComboBox);
    pHBoxLayout->addWidget(new QLabel("Offset: ", this));
    pHBoxLayout->addWidget(mpTimeOffsetLineEdit);

    // Dont do stuff if mpTime = NULL ptr
    if (mpTime)
    {
        // Populate time scale box and try to figure out current time unit
        //! @todo what if time = 0
        //! @todo would be nice if we could sort on scale size
        QMap<QString,double> units = gpConfig->getCustomUnits(TIMEVARIABLENAME);
        QString currUnit = mpTime->getPlotScaleDataUnit();
        if (currUnit.isEmpty())
        {
            currUnit = gpConfig->getDefaultUnit(TIMEVARIABLENAME);
        }
        QMap<QString,double>::iterator it;
        int ctr=0;
        for (it = units.begin(); it != units.end(); ++it)
        {
            mpTimeScaleComboBox->addItem(QString("%1 [%2]").arg(it.value()).arg(it.key()));
            if (currUnit == it.key())
            {
                mpTimeScaleComboBox->setCurrentIndex(ctr);
            }
            ++ctr;
        }

        // Set the current offset value
        mpTimeOffsetLineEdit->setText(QString("%1").arg(mpTime->getPlotOffset()));

        // Connect signals to update time scale and ofset when changing values
        connect(mpTimeScaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setVaules()));
        connect(mpTimeOffsetLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setVaules()));
    }
    else
    {
        mpTimeScaleComboBox->setDisabled(true);
        mpTimeOffsetLineEdit->setDisabled(true);
    }
}

void TimeScaleWidget::setScale(const QString &rUnitScale)
{
    mpTimeScaleComboBox->findText(rUnitScale, Qt::MatchContains);
    setVaules();
}

void TimeScaleWidget::setOffset(const QString &rOffset)
{
    mpTimeOffsetLineEdit->setText(rOffset);
    setVaules();
}

void TimeScaleWidget::setVaules()
{
    QString newUnit = extractBetweenFromQString(mpTimeScaleComboBox->currentText().split(" ").last(), '[', ']');
    QString newScaleStr = mpTimeScaleComboBox->currentText().split(" ")[0];
    mpTime->setCustomUnitScale(UnitScale(newUnit, newScaleStr));
    mpTime->setPlotOffset(mpTimeOffsetLineEdit->text().toDouble());
    emit valuesChanged();
    //! @todo this will aslo call all the updates again, need to be able to set scale and ofset separately or togheter
}

PlotArea::PlotArea(PlotTab *pParentPlotTab)
    : QWidget(pParentPlotTab)
{
    mpParentPlotTab = pParentPlotTab;
    QGridLayout *pLayout = new QGridLayout(this);

    setAcceptDrops(true);
    setMouseTracking(true);
    mHasCustomXData=false;
    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;
    mBottomAxisLogarithmic = false;

    // Plots
    mpQwtPlot = new HopQwtPlot(this);
    mpQwtPlot->setMouseTracking(true);
    mpQwtPlot->setAcceptDrops(false);
    mpQwtPlot->setCanvasBackground(QColor(Qt::white));
    mpQwtPlot->setAutoReplot(true);
    pLayout->addWidget(mpQwtPlot);
    mpQwtPlot->setAutoFillBackground(true);
    mpQwtPlot->setPalette(gpConfig->getPalette());

    // Panning Tool
    mpQwtPanner = new QwtPlotPanner(mpQwtPlot->canvas());
    mpQwtPanner->setMouseButton(Qt::LeftButton);
    mpQwtPanner->setEnabled(false);

    // Rubber Band Zoom
    QPen rubberBandPen(Qt::green);
    rubberBandPen.setWidth(2);

    mpQwtZoomerLeft = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpQwtPlot->canvas());      //Zoomer for left y axis
    mpQwtZoomerLeft->setMaxStackDepth(10000);
    mpQwtZoomerLeft->setRubberBand(QwtPicker::NoRubberBand);
    mpQwtZoomerLeft->setRubberBandPen(rubberBandPen);
    mpQwtZoomerLeft->setTrackerMode(QwtPicker::AlwaysOff);
    mpQwtZoomerLeft->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpQwtZoomerLeft->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    mpQwtZoomerLeft->setEnabled(false);

    mpQwtZoomerRight = new QwtPlotZoomer( QwtPlot::xTop, QwtPlot::yRight, mpQwtPlot->canvas());   //Zoomer for right y axis
    mpQwtZoomerRight->setMaxStackDepth(10000);
    mpQwtZoomerRight->setRubberBand(QwtPicker::NoRubberBand);
    mpQwtZoomerRight->setRubberBandPen(rubberBandPen);
    mpQwtZoomerRight->setTrackerMode(QwtPicker::AlwaysOff);
    mpQwtZoomerRight->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpQwtZoomerRight->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    mpQwtZoomerRight->setEnabled(false);

    // Wheel Zoom
    mpQwtMagnifier = new QwtPlotMagnifier(mpQwtPlot->canvas());
    mpQwtMagnifier->setAxisEnabled(QwtPlot::yLeft, true);
    mpQwtMagnifier->setAxisEnabled(QwtPlot::yRight, true);
    mpQwtMagnifier->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
    mpQwtMagnifier->setWheelFactor(1.1);
    mpQwtMagnifier->setMouseButton(Qt::NoButton, Qt::NoModifier);
    mpQwtMagnifier->setEnabled(true);

    // Grid
    mpQwtPlotGrid = new QwtPlotGrid;
    mpQwtPlotGrid->enableXMin(true);
    mpQwtPlotGrid->enableYMin(true);
    mpQwtPlotGrid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    mpQwtPlotGrid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
    mpQwtPlotGrid->setZ(GridLinesZOrderType);
    mpQwtPlotGrid->attach(mpQwtPlot);

    // Init curve counters
    mNumYlCurves = 0;
    mNumYrCurves = 0;

    // Attach lock boxes to plot
    mpXLockCheckBox = new QCheckBox(mpQwtPlot->axisWidget(QwtPlot::xBottom));
    mpXLockCheckBox->setCheckable(true);
    mpXLockCheckBox->setToolTip("Lock the x-axis");
    mpYLLockCheckBox = new QCheckBox(mpQwtPlot->axisWidget(QwtPlot::yLeft));
    mpYLLockCheckBox->setCheckable(true);
    mpYLLockCheckBox->setToolTip("Lock the left y-axis");
    mpYRLockCheckBox = new QCheckBox(mpQwtPlot->axisWidget(QwtPlot::yRight));
    mpYRLockCheckBox->setCheckable(true);
    mpYRLockCheckBox->setToolTip("Lock the right y-axis");
    connect(mpXLockCheckBox, SIGNAL(toggled(bool)), this, SLOT(axisLockHandler()));
    connect(mpYLLockCheckBox, SIGNAL(toggled(bool)), this, SLOT(axisLockHandler()));
    connect(mpYRLockCheckBox, SIGNAL(toggled(bool)), this, SLOT(axisLockHandler()));
    // Connect the refresh signal for repositioning the lock boxes
    connect(mpQwtPlot, SIGNAL(afterReplot()), this, SLOT(refreshLockCheckBoxPositions()));

    // Create the lock axis dialog
    constructAxisSettingsDialog();
    constructAxisLabelDialog();

    // Legend Stuff
    mpRightPlotLegend = new PlotLegend(QwtPlot::yRight);
    mpRightPlotLegend->attach(mpQwtPlot);
    mpRightPlotLegend->setAlignment(Qt::AlignRight | Qt::AlignTop);
    mpRightPlotLegend->setZ(LegendBelowCurveZOrderType);

    mpLeftPlotLegend = new PlotLegend(QwtPlot::yLeft);
    mpLeftPlotLegend->attach(mpQwtPlot);
    mpLeftPlotLegend->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    mpLeftPlotLegend->setZ(LegendBelowCurveZOrderType);

    constructLegendSettingsDialog();


    mpPainterWidget = new RectanglePainterWidget(this);
    mpPainterWidget->clearRect();
    pLayout->addWidget(mpPainterWidget,0,0); //Add on top of plot
}

PlotArea::~PlotArea()
{
    // Remove all curves
    while(!mPlotCurves.empty())
    {
        removeCurve(mPlotCurves.last());
    }
}


//! @brief Adds a plot curve to a plot tab
//! @param curve Pointer to the plot curve
//! @param desiredColor Desired color for curve (will override default colors)
void PlotArea::addCurve(PlotCurve *pCurve, QColor desiredColor)
{
    // Attach the curve to this plot area
    pCurve->attach(mpQwtPlot);
    mPlotCurves.append(pCurve);

    // Set some private members in the curve based on this current plot area
    pCurve->mpParentPlotArea = this;
    pCurve->mIncludeGenInTitle = mpIncludeGenInCurveTitle->isChecked();
    pCurve->mIncludeSourceInTitle = mpIncludeSourceInCurveTitle->isChecked();
    pCurve->setZ(CurveZOrderType);
    pCurve->setLineWidth(2);
    pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    setLegendSymbol(mpLegendSymbolType->currentText(), pCurve);

    // Refresh the curve
    pCurve->refreshCurveTitle();

    // Set custom xdata if one is already pressent in this tab
    if(mHasCustomXData)
    {
        if (pCurve->hasCustomXVariable())
        {
            //! @todo check that same unit
            qWarning("todo Check that same unit");
        }
        else
        {
            pCurve->setCustomXData(mCustomXData);
        }
    }

    // Determine what plot scales to use
    determineAddedCurveUnitOrScale(pCurve);

    // Determine plot curve color, if desired color is valid then use that color, else use the color selector the get the least common one
    if (desiredColor.isValid())
    {
        pCurve->setLineColor(desiredColor);
        mCurveColorSelector.incrementCurveColorCounter(desiredColor.name());
    }
    else
    {
        pCurve->setLineColor(mCurveColorSelector.getLeastCommonCurveColor());
    }

    // Enable axis and zoomers as needed
    // If this is the first curve on one of the axis, then then exis will just be enabled and we need to normalize the zoom (copy from the other curve)
    if (!mpQwtPlot->axisEnabled(pCurve->getAxisY()))
    {
        mpQwtPlot->enableAxis(pCurve->getAxisY());
        if (pCurve->getAxisY() == QwtPlot::yLeft)
        {
            if (mpQwtPlot->axisEnabled(QwtPlot::yRight))
            {
                mpQwtZoomerLeft->setZoomStack(mpQwtZoomerRight->zoomStack());
            }
        }
        if (pCurve->getAxisY() == QwtPlot::yRight)
        {
            if (mpQwtPlot->axisEnabled(QwtPlot::yLeft))
            {
                mpQwtZoomerRight->setZoomStack(mpQwtZoomerLeft->zoomStack());
            }
        }
    }

    // Count num curves by axis
    if (pCurve->getAxisY() == QwtPlot::yLeft)
    {
        ++mNumYlCurves;
    }
    else if (pCurve->getAxisY() == QwtPlot::yRight)
    {
        ++mNumYrCurves;
    }

    // Create a curve info box for this curve
    PlotCurveControlBox *pControlBox = new PlotCurveControlBox(pCurve, this);
    connect(pControlBox, SIGNAL(removeCurve(PlotCurve*)), this, SLOT(removeCurve(PlotCurve*)));
    mPlotCurveControlBoxes.append(pControlBox);
    mpParentPlotTab->mpCurveInfoScrollArea->widget()->layout()->addWidget(mPlotCurveControlBoxes.last());
    mPlotCurveControlBoxes.last()->show();


    // Connect som signals from the curve
    connect(pCurve, SIGNAL(curveDataUpdated()), this, SLOT(rescaleAxesToCurves()));
    connect(pCurve, SIGNAL(curveInfoUpdated()), this, SLOT(updateAxisLabels()));

    // Refresh and redraw the plot area
    //! @todo maybe make it possible to rescale only selected axis, instead of always recscaling both of them
    rescaleAxesToCurves();
    updateAxisLabels();
    updateWindowtitleModelName();
    replot();
}

void PlotArea::setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc)
{
    SharedVariableDescriptionT pVarDesc(new VariableDescription(rVarDesc));
    setCustomXVectorForAll(createFreeVectorVariable(xArray, pVarDesc));
}

void PlotArea::setCustomXVectorForAll(HopsanVariable data)
{
    for(int i=0; i<mPlotCurves.size(); ++i)
    {
        if (!mPlotCurves[i]->hasCustomXVariable())
        {
            mPlotCurves[i]->setCustomXData(data);
        }
    }
    rescaleAxesToCurves();
    setTabOnlyCustomXVector(data);
}

void PlotArea::removeCurve(PlotCurve *pCurve)
{
    // Remove any markes used by the curve
    for(int i=0; i<mPlotMarkers.size(); ++i)
    {
        if(mPlotMarkers[i]->getCurve() == pCurve)
        {
            removePlotMarker(mPlotMarkers[i]);
            --i;
        }
    }

    // Remove the plot curve info box used by the curve
    for(int i=0; i<mPlotCurveControlBoxes.size(); ++i)
    {
        if(mPlotCurveControlBoxes[i]->getCurve() == pCurve)
        {
            mPlotCurveControlBoxes[i]->hide();
            mPlotCurveControlBoxes[i]->deleteLater();
            mPlotCurveControlBoxes.removeAt(i);
            break;
        }
    }

    // Reduce the curve color counter for this curve color
    mCurveColorSelector.decrementCurveColorCounter(pCurve->pen().color());

    if (pCurve->getAxisY() == QwtPlot::yLeft)
    {
        --mNumYlCurves;
    }
    else if (pCurve->getAxisY() == QwtPlot::yRight)
    {
        --mNumYrCurves;
    }

    pCurve->detach();
    mPlotCurves.removeAll(pCurve);
    pCurve->mpParentPlotArea = 0;
    pCurve->disconnect();
    delete pCurve;

    // Reset timevector incase we had special x-axis set previously
    if (mPlotCurves.isEmpty() && mHasCustomXData)
    {
        resetXTimeVector();
    }

    // Reset zoom and remove axis locks if last curve was removed (makes no sense to keep it zoomed in)
    if(mPlotCurves.isEmpty())
    {
        mpXLockCheckBox->setChecked(false);
        mpYLLockCheckBox->setChecked(false);
        mpYRLockCheckBox->setChecked(false);
        resetZoom();
    }

    rescaleAxesToCurves();
    updateAxisLabels();
    updateWindowtitleModelName();
    replot();
}

void PlotArea::removeAllCurvesOnAxis(const int axis)
{
    // Make a copy of the curves, since remove will delete them
    QList<PlotCurve*> curvePtrs = getCurves();
    for(int c=0; c<curvePtrs.size(); ++c)
    {
        if(curvePtrs[c]->getAxisY() == axis)
        {
            removeCurve(curvePtrs[c]);
        }
    }
}

void PlotArea::removePlotMarker(PlotMarker *pMarker)
{
    if (pMarker)
    {
        mpQwtPlot->canvas()->removeEventFilter(pMarker);
        pMarker->hide();
        pMarker->detach();
        pMarker->deleteLater();
        mPlotMarkers.removeAll(pMarker);
    }
}

QList<PlotCurve *> &PlotArea::getCurves()
{
    return mPlotCurves;
}

void PlotArea::setActivePlotCurve(PlotCurve *pCurve)
{
    // Mark deactive all others
    //! @todo if only one can be active it should be enough to deactivate that one
    for(int i=0; i<mPlotCurves.size(); ++i)
    {
        if(mPlotCurves[i] != pCurve)
        {
            mPlotCurves[i]->markActive(false);
        }
    }
    // Mark active the one
    if (pCurve!=0)
    {
        pCurve->markActive(true);
    }

    // Remember active curve
    mpActivePlotCurve = pCurve;
}

PlotCurve *PlotArea::getActivePlotCurve()
{
    return mpActivePlotCurve;
}

QwtPlot *PlotArea::getQwtPlot()
{
    return mpQwtPlot;
}

const QStringList &PlotArea::getModelPaths() const
{
    return mModelPaths;
}

int PlotArea::getNumberOfCurves() const
{
    return mPlotCurves.size();
}

bool PlotArea::isArrowEnabled() const
{
    return !(isZoomEnabled() || isPanEnabled());
}

bool PlotArea::isZoomEnabled() const
{
    return mpQwtZoomerLeft->isEnabled();
}

bool PlotArea::isPanEnabled() const
{
    return mpQwtPanner->isEnabled();
}

bool PlotArea::isGridVisible() const
{
    return mpQwtPlotGrid->isVisible();
}

bool PlotArea::isZoomed() const
{
    //    uint l = mpZoomerLeft[plotId]->zoomRectIndex();
    //    uint r = mpZoomerRight[plotId]->zoomRectIndex();
    //    qDebug() << "id,l,r: " << plotId <<" "<< l <<" "<< r;
    return (mpQwtZoomerLeft->zoomRectIndex() > 0);// && (mpZoomerRight->zoomRectIndex() > 1);
}

bool PlotArea::hasCustomXData() const
{
    return mHasCustomXData;
}

const HopsanVariable PlotArea::getCustomXData() const
{
    return mCustomXData;
}

void PlotArea::setBottomAxisLogarithmic(bool value)
{
    mBottomAxisLogarithmic = value;
    if(value)
    {
        mpQwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
    }
    else
    {
        mpQwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    }
}

void PlotArea::setLeftAxisLogarithmic(bool value)
{
    mLeftAxisLogarithmic = value;
    if(value)
    {
        mpQwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
    }
    else
    {
        mpQwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
    }
}

void PlotArea::setRightAxisLogarithmic(bool value)
{
    mRightAxisLogarithmic = value;
    if(value)
    {
        mpQwtPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine(10));
    }
    else
    {
        mpQwtPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
    }
}

bool PlotArea::isBottomAxisLogarithmic() const
{
    return mBottomAxisLogarithmic;
}

bool PlotArea::isLeftAxisLogarithmic() const
{
    return mLeftAxisLogarithmic;
}

bool PlotArea::isRightAxisLogarithmic() const
{
    return mRightAxisLogarithmic;
}

void PlotArea::setAxisLimits(QwtPlot::Axis axis, const double min, const double max, bool lockAxis)
{
    mpQwtPlot->setAxisScale(axis, min, max);

    // Lock the axis if desired, but do not unlock it if not
    //! @todo maybe we should
    if (lockAxis)
    {
        switch (axis) {
        case QwtPlot::xBottom:
            mpXLockCheckBox->setChecked(true);
            break;
        case QwtPlot::yLeft:
            mpYLLockCheckBox->setChecked(true);
            break;
        case QwtPlot::yRight:
            mpYRLockCheckBox->setChecked(true);
            break;
        default:
            break;
            //Nothing for the other axis
        }
    }
}

void PlotArea::setAxisLabel(QwtPlot::Axis axis, const QString &rLabel)
{
    mpUserDefinedLabelsCheckBox->setChecked(true);
    switch (axis) {
    case QwtPlot::xBottom:
        mpUserDefinedXLabel->setText(rLabel);
        break;
    case QwtPlot::yLeft:
        mpUserDefinedYlLabel->setText(rLabel);
        break;
    case QwtPlot::yRight:
        mpUserDefinedYrLabel->setText(rLabel);
        break;
    default:
        break;
        //Nothing for the other axis
    }
    updateAxisLabels();
}

void PlotArea::setLegendsVisible(bool value)
{
    if (value)
    {
        // Only turn on internal automatically
        mpLegendsEnabledCheckBox->setChecked(true);
    }
    else
    {
        mpLegendsEnabledCheckBox->setChecked(false);
    }
    applyLegendSettings();
}

void PlotArea::replot()
{
    // Enable axis depending on the number of curves
    mpQwtPlot->enableAxis(QwtPlot::yLeft, (mNumYlCurves > 0));
    mpQwtPlot->enableAxis(QwtPlot::yRight, (mNumYrCurves > 0));

    // Update plotmarkers
    updatePlotMarkers();

    // Replot the plot area
    mpQwtPlot->replot();
    mpQwtPlot->updateGeometry();
}

void PlotArea::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    this->rescaleAxesToCurves();
}

void PlotArea::dragEnterEvent(QDragEnterEvent *event)
{
    // Don't accept drag events to FFT and Bode plots
    if( mpParentPlotTab->getPlotTabType() == XYPlotType )
    {
        if (event->mimeData()->hasText())
        {
            // Create the hover rectangle (size will be changed by dragMoveEvent)
            mpPainterWidget->createRect(0,0,this->width(), this->height());
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void PlotArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    // Don't accept drag events to FFT and Bode plots
    if( mpParentPlotTab->getPlotTabType() == XYPlotType )
    {
        mpPainterWidget->clearRect();
        QWidget::dragLeaveEvent(event);
    }
    else
    {
       event->ignore();
    }
}

void PlotArea::dragMoveEvent(QDragMoveEvent *event)
{
    // Don't accept drag events to FFT and Bode plots
     if( mpParentPlotTab->getPlotTabType() == XYPlotType )
     {
         QCursor cursor;
         if(this->mapFromGlobal(cursor.pos()).y() > getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y()+10 && getNumberOfCurves() >= 1)
         {
             mpPainterWidget->createRect(getQwtPlot()->canvas()->x(), getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y(), getQwtPlot()->canvas()->width(), getQwtPlot()->canvas()->height()*1.0/3.0);
             mpParentPlotTab->showHelpPopupMessage("Replace X-axis with selected variable.");
         }
         else if(this->mapFromGlobal(cursor.pos()).x() < getQwtPlot()->canvas()->x()+9 + getQwtPlot()->canvas()->width()/2)
         {
             mpPainterWidget->createRect(getQwtPlot()->canvas()->x(), getQwtPlot()->canvas()->y(), getQwtPlot()->canvas()->width()/2, getQwtPlot()->canvas()->height());
             mpParentPlotTab->showHelpPopupMessage("Add selected variable to left Y-axis.");
         }
         else
         {
             mpPainterWidget->createRect(getQwtPlot()->canvas()->x() + getQwtPlot()->canvas()->width()/2, getQwtPlot()->canvas()->y(), getQwtPlot()->canvas()->width()/2, getQwtPlot()->canvas()->height());
             mpParentPlotTab->showHelpPopupMessage("Add selected variable to right Y-axis.");
         }
         QWidget::dragMoveEvent(event);
     }
     else
     {
         event->ignore();
     }
}

void PlotArea::dropEvent(QDropEvent *event)
{
    // Don't accept drag events to FFT and Bode plots
    if( mpParentPlotTab->getPlotTabType() == XYPlotType )
    {
        QWidget::dropEvent(event);
        mpPainterWidget->clearRect();

        if (event->mimeData()->hasText())
        {
            QString mimeText = event->mimeData()->text();
            if(mimeText.startsWith("HOPSANPLOTDATA:"))
            {
                qDebug() << mimeText;
                QStringList fields = mimeText.split(":");
                if (fields.size() > 3)
                {
                    QString &name = fields[1];
                    QString &model = fields[3];
                    bool parseOk = false;
                    int gen = fields[2].toInt(&parseOk);
                    if (!parseOk)
                    {
                        gen = -1;
                    }

                    //! @todo what about subsystems here
                    ContainerObject *pContainer = gpModelHandler->getTopLevelSystem(model);
                    // If we failed to find by modelpath (like for imported vars), then try the current view
                    if (!pContainer)
                    {
                        pContainer = gpModelHandler->getCurrentViewContainerObject();
                    }
                    if (pContainer)
                    {
                        HopsanVariable data = pContainer->getLogDataHandler()->getHopsanVariable(name, gen);
                        // If we have found data then add it to the plot
                        if (data)
                        {
                            QCursor cursor;
                            if(this->mapFromGlobal(cursor.pos()).y() > getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y()+10 && getNumberOfCurves() >= 1)
                            {
                                setCustomXVectorForAll(data);
                                //! @todo do we need to reset to default unit too ?
                            }
                            else if(this->mapFromGlobal(cursor.pos()).x() < getQwtPlot()->canvas()->x()+9 + getQwtPlot()->canvas()->width()/2)
                            {
                                //! @todo maybe go to tab only and let it handle main window updates
                                mpParentPlotTab->mpParentPlotWindow->addPlotCurve(data, QwtPlot::yLeft);
                            }
                            else
                            {
                                mpParentPlotTab->mpParentPlotWindow->addPlotCurve(data, QwtPlot::yRight);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        event->ignore();
    }
}

void PlotArea::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget::contextMenuEvent(event);

    //   return;
    if(mpQwtZoomerLeft->isEnabled())
    {
        return;
    }

    QMenu menu;
    QMenu *pYAxisRightMenu;
    QMenu *pYAxisLeftMenu;
    QMenu *pBottomAxisMenu;
    QMenu *pChangeUnitsMenu;
    QMenu *pInsertMarkerMenu;

    QAction *pSetRightAxisLogarithmic = 0;
    QAction *pSetLeftAxisLogarithmic = 0;
    QAction *pSetBottomAxisLogarithmic = 0;
    QAction *pSetUserDefinedAxisLabels = 0;

    pYAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    pYAxisRightMenu = menu.addMenu(QString("Right Y Axis"));
    pBottomAxisMenu = menu.addMenu(QString("Bottom Axis"));

    pYAxisLeftMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::yLeft));
    pYAxisRightMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::yRight));
    pBottomAxisMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::xBottom));

    // Create menu and actions for changing units
    pChangeUnitsMenu = menu.addMenu(QString("Change Units"));
    QMap<QAction *, PlotCurve *> actionToCurveMap;
    QMap<QString, double> unitMap;
    QList<PlotCurve *>::iterator itc;
    QMap<QString, double>::iterator itu;
    for(itc=mPlotCurves.begin(); itc!=mPlotCurves.end(); ++itc)
    {
        QMenu *pTempMenu = pChangeUnitsMenu->addMenu(QString((*itc)->getComponentName() + ", " + (*itc)->getPortName() + ", " + (*itc)->getDataName()));
        if ((*itc)->getDataName() == "Value")
        {
            QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit((*itc)->getDataOriginalUnit());
            if (pqs.size() > 0)
            {
                unitMap = gpConfig->getCustomUnits(pqs.first());
            }
        }
        else
        {
            unitMap = gpConfig->getCustomUnits((*itc)->getDataName());
        }

        for(itu=unitMap.begin(); itu!=unitMap.end(); ++itu)
        {
            QAction *pTempAction = pTempMenu->addAction(itu.key());
            actionToCurveMap.insert(pTempAction, (*itc));
        }
    }

    // Create actions for making axis logarithmic
    if(mpQwtPlot->axisEnabled(QwtPlot::yLeft))
    {
        pSetLeftAxisLogarithmic = pYAxisLeftMenu->addAction("Logarithmic Scale");
        pSetLeftAxisLogarithmic->setCheckable(true);
        pSetLeftAxisLogarithmic->setChecked(mLeftAxisLogarithmic);
    }
    if(mpQwtPlot->axisEnabled(QwtPlot::yRight))
    {
        pSetRightAxisLogarithmic = pYAxisRightMenu->addAction("Logarithmic Scale");
        pSetRightAxisLogarithmic->setCheckable(true);
        pSetRightAxisLogarithmic->setChecked(mRightAxisLogarithmic);
    }
    if(mpQwtPlot->axisEnabled(QwtPlot::xBottom))
    {
        pSetBottomAxisLogarithmic = pBottomAxisMenu->addAction("Logarithmic Scale");
        pSetBottomAxisLogarithmic->setCheckable(true);
        pSetBottomAxisLogarithmic->setChecked(mBottomAxisLogarithmic);
    }


    // Create menu for inserting curve markers
    pInsertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    for(itc=mPlotCurves.begin(); itc!=mPlotCurves.end(); ++itc)
    {
        QAction *pTempAction = pInsertMarkerMenu->addAction((*itc)->getCurveName());
        actionToCurveMap.insert(pTempAction, (*itc));
    }


    // Create option for changing axis labels
    pSetUserDefinedAxisLabels = menu.addAction("Set userdefined axis labels");

    // ----- Wait for user to make a selection ----- //
    QAction *pSelectedAction = menu.exec(QCursor::pos());

    // ----- User has selected something -----  //

    // Check if user did not click on a menu item
    if(pSelectedAction == 0)
    {
        return;
    }


    // Change unit on selected curve
    if(pSelectedAction->parentWidget()->parentWidget() == pChangeUnitsMenu)
    {
        actionToCurveMap.find(pSelectedAction).value()->setCustomCurveDataUnit(pSelectedAction->text());
    }


    // Make axis logarithmic
    if (pSelectedAction == pSetRightAxisLogarithmic)
    {
        mRightAxisLogarithmic = !mRightAxisLogarithmic;
        if(mRightAxisLogarithmic)
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine(10));
        }
        else
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
        }
        rescaleAxesToCurves();
        mpQwtPlot->replot();
        mpQwtPlot->updateGeometry();
    }
    else if (pSelectedAction == pSetLeftAxisLogarithmic)
    {
        mLeftAxisLogarithmic = !mLeftAxisLogarithmic;
        if(mLeftAxisLogarithmic)
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
        }
        else
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
        }
        rescaleAxesToCurves();
        mpQwtPlot->replot();
        mpQwtPlot->updateGeometry();
    }
    else if (pSelectedAction == pSetBottomAxisLogarithmic)
    {
        mBottomAxisLogarithmic = !mBottomAxisLogarithmic;
        if(mBottomAxisLogarithmic)
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
        }
        else
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
        }
        rescaleAxesToCurves();
        mpQwtPlot->replot();
        mpQwtPlot->updateGeometry();
    }

    // Set user axes labels
    if (pSelectedAction == pSetUserDefinedAxisLabels)
    {
        openAxisLabelDialog();
    }


    // Insert curve marker
    if(pSelectedAction->parentWidget() == pInsertMarkerMenu)
    {
        insertMarker(actionToCurveMap.find(pSelectedAction).value(), event->pos());
    }
}

void PlotArea::rescaleAxesToCurves()
{
    // Set defaults when no axis available
    HopQwtInterval xAxisLim(0,10), ylAxisLim(0,10), yrAxisLim(0,10);

    // Cycle plots, ignore if no curves
    if(!mPlotCurves.empty())
    {
        // Init left/right min max
        if (mNumYlCurves > 0)
        {
            ylAxisLim.setInterval(DoubleMax,DoubleMin);
        }
        if (mNumYrCurves > 0)
        {
            yrAxisLim.setInterval(DoubleMax,DoubleMin);
        }

        // Initialize values for X axis
        xAxisLim.setInterval(DoubleMax, DoubleMin);

        bool someoneHasCustomXdata = false;
        for(int i=0; i<mPlotCurves.size(); ++i)
        {
            // First check if some curve has a custom x-axis and plot does not
            someoneHasCustomXdata = someoneHasCustomXdata || mPlotCurves[i]->hasCustomXVariable();
            if (!mHasCustomXData && someoneHasCustomXdata)
            {
                //! @todo maybe should do this with signal slot instead, to avoid unesisarry checks all the time
                setTabOnlyCustomXVector(mPlotCurves[i]->getSharedCustomXVariable());
            }

            if(mPlotCurves[i]->getAxisY() == QwtPlot::yLeft)
            {
                if(mLeftAxisLogarithmic)
                {
                    // Only consider positive (non-zero) values if logarithmic scaling is used
                    double min, max;
                    if (mPlotCurves[i]->minMaxPositiveNonZeroYValues(min, max))
                    {
                        ylAxisLim.extendMin(min);
                        ylAxisLim.extendMax(max);
                    }
                }
                else
                {
                    ylAxisLim.extendMin(mPlotCurves[i]->minYValue());
                    ylAxisLim.extendMax(mPlotCurves[i]->maxYValue());
                }
            }

            if(mPlotCurves[i]->getAxisY() == QwtPlot::yRight)
            {
                if(mRightAxisLogarithmic)
                {
                    // Only consider positive (non-zero) values if logarithmic scaling is used
                    double min, max;
                    if (mPlotCurves[i]->minMaxPositiveNonZeroYValues(min, max))
                    {
                        yrAxisLim.extendMin(min);
                        yrAxisLim.extendMax(max);
                    }
                }
                else
                {
                    yrAxisLim.extendMin(mPlotCurves[i]->minYValue());
                    yrAxisLim.extendMax(mPlotCurves[i]->maxYValue());
                }
            }

            // Find min / max x-value
            if (mBottomAxisLogarithmic)
            {
                // Only consider positive (non-zero) values if logarithmic scaling is used
                double min, max;
                if (mPlotCurves[i]->minMaxPositiveNonZeroXValues(min, max))
                {
                    xAxisLim.extendMin(min);
                    xAxisLim.extendMax(max);
                }
            }
            else
            {
                xAxisLim.extendMin(mPlotCurves[i]->minXValue());
                xAxisLim.extendMax(mPlotCurves[i]->maxXValue());
            }
        }

        if (mHasCustomXData && !someoneHasCustomXdata)
        {
            this->resetXTimeVector();
        }
    }

    // Fix incorrect or bad limit values
    if(!ylAxisLim.isValid())
    {
        ylAxisLim.setInterval(0,10);
    }
    if(!yrAxisLim.isValid())
    {
        yrAxisLim.setInterval(0,10);
    }

    const double sameLimFrac = 0.1;
    // Max and min must not be same value; if they are, decrease/increase
    if ( (ylAxisLim.width()) < Double100Min)
    {
        ylAxisLim.extendMax(ylAxisLim.maxValue()+qMax(qAbs(ylAxisLim.maxValue()) * sameLimFrac, Double100Min));
        ylAxisLim.extendMin(ylAxisLim.minValue()-qMax(qAbs(ylAxisLim.minValue()) * sameLimFrac, Double100Min));
    }

    if ( (yrAxisLim.width()) < Double100Min)
    {
        yrAxisLim.extendMax(yrAxisLim.maxValue()+qMax(qAbs(yrAxisLim.maxValue()) * sameLimFrac, Double100Min));
        yrAxisLim.extendMin(yrAxisLim.maxValue()-qMax(qAbs(yrAxisLim.minValue()) * sameLimFrac, Double100Min));
    }

    if ( (xAxisLim.width()) < Double100Min)
    {
        xAxisLim.extendMax(xAxisLim.maxValue()+qMax(qAbs(xAxisLim.maxValue()) * sameLimFrac, Double100Min));
        xAxisLim.extendMin(xAxisLim.minValue()-qMax(qAbs(xAxisLim.minValue()) * sameLimFrac, Double100Min));
    }

    // If plot has log scale, we use a different approach for calculating margins
    // (fixed margins would not make sense with a log scale)

    //! @todo In new qwt the type in the transform has been removed, Trying with dynamic cast instead
    if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlot->axisScaleEngine(QwtPlot::yLeft)))
    {
        ylAxisLim.setInterval(ylAxisLim.minValue()/2.0, ylAxisLim.maxValue()*2.0);
    }
    else
    {
        // For linear scale expand by 5%
        //! @todo no need to add 5% if sameLimFrac has been added above
        ylAxisLim.setInterval(ylAxisLim.minValue()-0.05*ylAxisLim.width(), ylAxisLim.maxValue()+0.05*ylAxisLim.width());
    }

    if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlot->axisScaleEngine(QwtPlot::yRight)))
    {
        yrAxisLim.setInterval(yrAxisLim.minValue()/2.0, yrAxisLim.maxValue()*2.0);
    }
    else
    {
        // For linear scale expand by 5%
        yrAxisLim.setInterval(yrAxisLim.minValue()-0.05*yrAxisLim.width(), yrAxisLim.maxValue()+0.05*yrAxisLim.width());
    }



    // Create the zoom base (original zoom) rectangle for the left and right axis
    QRectF baseZoomRect;
    baseZoomRect.setX(xAxisLim.minValue());
    baseZoomRect.setWidth(xAxisLim.width());

    // Scale the axes autoamtically if not locked
    if (!mpXLockCheckBox->isChecked())
    {
        mpQwtPlot->setAxisScale(QwtPlot::xBottom, xAxisLim.minValue(), xAxisLim.maxValue());
    }

    if (!mpYLLockCheckBox->isChecked())
    {
        rescaleAxisLimitsToMakeRoomForLegend(QwtPlot::yLeft, ylAxisLim);
        //! @todo befor setting we should check so that min max is resonable else hopsan will crash (example: Inf)
        mpQwtPlot->setAxisScale(QwtPlot::yLeft, ylAxisLim.minValue(), ylAxisLim.maxValue());
        baseZoomRect.setY(ylAxisLim.minValue());
        baseZoomRect.setHeight(ylAxisLim.width());
        mpQwtZoomerLeft->setZoomBase(baseZoomRect);
    }

    if (!mpYRLockCheckBox->isChecked())
    {
        rescaleAxisLimitsToMakeRoomForLegend(QwtPlot::yRight, yrAxisLim);
        //! @todo befor setting we should check so that min max is resonable else hopsan will crash (example: Inf)
        mpQwtPlot->setAxisScale(QwtPlot::yRight, yrAxisLim.minValue(), yrAxisLim.maxValue());
        baseZoomRect.setY(yrAxisLim.minValue());
        baseZoomRect.setHeight(yrAxisLim.width());
        mpQwtZoomerRight->setZoomBase(baseZoomRect);
    }

    //! @todo left only applies to left even if the right is overshadowed, problem is that if left, right are bottom and top calculated buffers will be different on each axis, this is a todo problem with legend buffer ofset

    refreshLockCheckBoxPositions();

    // Now call the actual refresh of the axes
    mpQwtPlot->updateAxes();
}

void PlotArea::toggleAxisLock()
{
    bool allLocked = false;
    // First check if they are locked
    if (mpQwtPlot->axisEnabled(QwtPlot::xBottom))
    {
        allLocked = mpXLockCheckBox->isChecked();
    }
    if (mpQwtPlot->axisEnabled(QwtPlot::yLeft))
    {
        allLocked *= mpYLLockCheckBox->isChecked();
    }
    if (mpQwtPlot->axisEnabled(QwtPlot::yRight))
    {
        allLocked *= mpYRLockCheckBox->isChecked();
    }

    // Now switch to the other state (but only if axis is enabled)
    if (mpQwtPlot->axisEnabled(QwtPlot::xBottom))
    {
        mpXLockCheckBox->setChecked(!allLocked);
    }
    if (mpQwtPlot->axisEnabled(QwtPlot::yLeft))
    {
        mpYLLockCheckBox->setChecked(!allLocked);
    }
    if (mpQwtPlot->axisEnabled(QwtPlot::yRight))
    {
        mpYRLockCheckBox->setChecked(!allLocked);
    }
}

void PlotArea::updateAxisLabels()
{
    mpQwtPlot->setAxisTitle(QwtPlot::xBottom, QwtText());
    mpQwtPlot->setAxisTitle(QwtPlot::yLeft, QwtText());
    mpQwtPlot->setAxisTitle(QwtPlot::yRight, QwtText());

    if (!mPlotCurves.empty())
    {
        QStringList leftLabels, rightLabels, bottomLabels;
        QList<SharedVectorVariableT> sharedBottomVars;
        for(int i=0; i<mPlotCurves.size(); ++i)
        {
            // First decide new y-axis label
            // If alias empty then use data name, else use the alias name
            QString newLabel;
            if (mPlotCurves[i]->getVariable()->getAliasName().isEmpty())
            {
                newLabel = QString("%1").arg(mPlotCurves[i]->getDataName());
            }
            else
            {
                newLabel = QString("%1").arg(mPlotCurves[i]->getVariable()->getAliasName());
            }

            // Add unit if it exists
            if (!mPlotCurves[i]->getCurrentUnit().isEmpty())
            {
                newLabel.append(QString(" [%1]").arg(mPlotCurves[i]->getCurrentUnit()));
            }

            // If new label is not already on the axis then we may want to add it
            // Check left axis
            if( (mPlotCurves[i]->getAxisY() == QwtPlot::yLeft) && !leftLabels.contains(newLabel) )
            {
                leftLabels.append(newLabel);
            }

            // Check right axis
            if( (mPlotCurves[i]->getAxisY() == QwtPlot::yRight) && !rightLabels.contains(newLabel) )
            {
                rightLabels.append(newLabel);
            }

            // Now decide new bottom axis label
            // Use custom x-axis if availible, else try to use the time or frequency vector (if set)
            SharedVectorVariableT pSharedXVector = mPlotCurves[i]->getSharedCustomXVariable();
            if (pSharedXVector.isNull())
            {
                pSharedXVector = mPlotCurves[i]->getSharedTimeOrFrequencyVariable();
            }
            QString bottomLabel;
            if (pSharedXVector.isNull())
            {
                bottomLabel = "Samples";
            }
            else if (!sharedBottomVars.contains(pSharedXVector))
            {
                //! @todo for custom x mayb check for alias name
                sharedBottomVars.append(pSharedXVector); // This one is used for faster comparison (often the curves share the same x-vector)
                bottomLabel = QString("%1").arg(pSharedXVector->getDataName());
                if (!pSharedXVector->getActualPlotDataUnit().isEmpty())
                {
                    bottomLabel.append(QString(" [%1]").arg(pSharedXVector->getActualPlotDataUnit()));
                }

            }
            if (!bottomLabel.isEmpty() && !bottomLabels.contains(bottomLabel))
            {
                bottomLabels.append(bottomLabel);
            }
        }

        // Set the actual axis labels
        mpQwtPlot->setAxisTitle(QwtPlot::xBottom, QwtText(bottomLabels.join(", ")));
        mpQwtPlot->setAxisTitle(QwtPlot::yLeft, QwtText(leftLabels.join(", ")));
        mpQwtPlot->setAxisTitle(QwtPlot::yRight, QwtText(rightLabels.join(", ")));
    }

    // If Usercustom labels exist overwrite automatic label
    if (mpUserDefinedLabelsCheckBox->isChecked())
    {
        if (!mpUserDefinedXLabel->text().isEmpty())
        {
            mpQwtPlot->setAxisTitle(QwtPlot::xBottom, QwtText(mpUserDefinedXLabel->text()));
        }
        if (!mpUserDefinedYlLabel->text().isEmpty())
        {
            mpQwtPlot->setAxisTitle(QwtPlot::yLeft, QwtText(mpUserDefinedYlLabel->text()));
        }
        if (!mpUserDefinedYrLabel->text().isEmpty())
        {
            mpQwtPlot->setAxisTitle(QwtPlot::yRight, QwtText(mpUserDefinedYrLabel->text()));
        }
    }

}

void PlotArea::openLegendSettingsDialog()
{
    mpLegendSettingsDialog->exec();
}

void PlotArea::openAxisSettingsDialog()
{
    // Set values before buttons are connected to avoid triggering rescale
    mpXminLineEdit->setText(QString("%1").arg(mpQwtPlot->axisInterval(QwtPlot::xBottom).minValue()));
    mpXmaxLineEdit->setText(QString("%1").arg(mpQwtPlot->axisInterval(QwtPlot::xBottom).maxValue()));

    mpYLminLineEdit->setText(QString("%1").arg(mpQwtPlot->axisInterval(QwtPlot::yLeft).minValue()));
    mpYLmaxLineEdit->setText(QString("%1").arg(mpQwtPlot->axisInterval(QwtPlot::yLeft).maxValue()));

    mpYRminLineEdit->setText(QString("%1").arg(mpQwtPlot->axisInterval(QwtPlot::yRight).minValue()));
    mpYRmaxLineEdit->setText(QString("%1").arg(mpQwtPlot->axisInterval(QwtPlot::yRight).maxValue()));

    mpXLockDialogCheckBox->setChecked(mpXLockCheckBox->isChecked());
    mpYLLockDialogCheckBox->setChecked(mpYLLockCheckBox->isChecked());
    mpYRLockDialogCheckBox->setChecked(mpYRLockCheckBox->isChecked());

    // Connect the buttons, to adjust whenever changes are made
    connect(mpXminLineEdit,         SIGNAL(textChanged(QString)),   this,           SLOT(applyAxisSettings()));
    connect(mpXmaxLineEdit,         SIGNAL(textChanged(QString)),   this,           SLOT(applyAxisSettings()));
    connect(mpYLminLineEdit,        SIGNAL(textChanged(QString)),   this,           SLOT(applyAxisSettings()));
    connect(mpYLmaxLineEdit,        SIGNAL(textChanged(QString)),   this,           SLOT(applyAxisSettings()));
    connect(mpYRminLineEdit,        SIGNAL(textChanged(QString)),   this,           SLOT(applyAxisSettings()));
    connect(mpYRmaxLineEdit,        SIGNAL(textChanged(QString)),   this,           SLOT(applyAxisSettings()));
    connect(mpXLockDialogCheckBox,  SIGNAL(toggled(bool)),          this,           SLOT(applyAxisSettings()));
    connect(mpYLLockDialogCheckBox, SIGNAL(toggled(bool)),          this,           SLOT(applyAxisSettings()));
    connect(mpYRLockDialogCheckBox, SIGNAL(toggled(bool)),          this,           SLOT(applyAxisSettings()));

    mpSetAxisDialog->exec();

    // Disconnect the buttons again to prevent triggering apply when data is changed in code
    disconnect(mpXminLineEdit,      0, 0, 0);
    disconnect(mpXmaxLineEdit,      0, 0, 0);
    disconnect(mpYLminLineEdit,     0, 0, 0);
    disconnect(mpYLmaxLineEdit,     0, 0, 0);
    disconnect(mpYRminLineEdit,     0, 0, 0);
    disconnect(mpYRmaxLineEdit,     0, 0, 0);
    disconnect(mpXLockDialogCheckBox,  0, 0, 0);
    disconnect(mpYLLockDialogCheckBox, 0, 0, 0);
    disconnect(mpYRLockDialogCheckBox, 0, 0, 0);
}

void PlotArea::openAxisLabelDialog()
{
    mpUserDefinedLabelsDialog->exec();
}

void PlotArea::openTimeScalingDialog()
{
    QDialog scaleDialog(this);
    scaleDialog.setWindowTitle("Change Time scaling and offset");

    // One for each generation, automatic sort on key
    QMap<int, TimeScaleWidget*> activeGenerations;
    //! @todo what if massive amount of generations
    for (int i=0; i<mPlotCurves.size(); ++i)
    {
        int gen = mPlotCurves[i]->getGeneration();
        if (!activeGenerations.contains(gen))
        {
            SharedVectorVariableT pTime = mPlotCurves[i]->getSharedTimeOrFrequencyVariable();
            //if (pTime)
            {
                TimeScaleWidget *pTimeScaleW = new TimeScaleWidget(pTime, &scaleDialog);
                connect(pTimeScaleW, SIGNAL(valuesChanged()), this, SLOT(updateAxisLabels()));
                activeGenerations.insert(gen, pTimeScaleW);
            }
        }
    }

    QGridLayout *pGridLayout = new QGridLayout(&scaleDialog);

    // Now push scale widgets into grid, in sorted order from map
    pGridLayout->addWidget(new QLabel("Changing a generation time scale or offset will affect all variables at generation in all plot windows!",&scaleDialog), 0, 0, 1, 2, Qt::AlignLeft);
    int row = 1;
    QMap<int, TimeScaleWidget*>::iterator it;
    for (it=activeGenerations.begin(); it!=activeGenerations.end(); ++it)
    {
        pGridLayout->addWidget(new QLabel(QString("Gen: %1").arg(it.key()+1), &scaleDialog), row, 0);
        pGridLayout->addWidget(it.value(), row, 1);
        ++row;
    }

    // Add button box
    QPushButton *pDoneButton = new QPushButton("Close", &scaleDialog);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);
    pGridLayout->addWidget(pButtonBox, row, 1);
    connect(pDoneButton,SIGNAL(clicked()),&scaleDialog,SLOT(close()));
    connect(pDoneButton,SIGNAL(clicked()),this,SLOT(updateAxisLabels())); //!< @todo this should ahppen directly when changing scale values

    scaleDialog.exec();
}

void PlotArea::applyAxisSettings()
{
    // Set the new axis limits
    mpQwtPlot->setAxisScale(QwtPlot::xBottom, mpXminLineEdit->text().toDouble(),  mpXmaxLineEdit->text().toDouble());
    mpQwtPlot->setAxisScale(QwtPlot::yLeft,   mpYLminLineEdit->text().toDouble(), mpYLmaxLineEdit->text().toDouble());
    mpQwtPlot->setAxisScale(QwtPlot::yRight,  mpYRminLineEdit->text().toDouble(), mpYRmaxLineEdit->text().toDouble());

    mpXLockCheckBox->setChecked(mpXLockDialogCheckBox->isChecked());
    mpYLLockCheckBox->setChecked(mpYLLockDialogCheckBox->isChecked());
    mpYRLockCheckBox->setChecked(mpYRLockDialogCheckBox->isChecked());
}

void PlotArea::applyAxisLabelSettings()
{
    updateAxisLabels();
}

void PlotArea::applyLegendSettings()
{
    // Show/change internal legneds
    if(mpLegendsEnabledCheckBox->isChecked())
    {
        mpRightPlotLegend->show();
        mpLeftPlotLegend->show();

        mpRightPlotLegend->setMaxColumns(mpLegendCols->value());
        mpLeftPlotLegend->setMaxColumns(mpLegendCols->value());

        setLegendSymbol(mpLegendSymbolType->currentText());

        mpRightPlotLegend->setBackgroundMode(PlotLegend::BackgroundMode(mpLegendBgType->currentIndex()));
        mpLeftPlotLegend->setBackgroundMode(PlotLegend::BackgroundMode(mpLegendBgType->currentIndex()));

        int alignL = mpLegendLPosition->currentIndex();
        int alignR = mpLegendRPosition->currentIndex();

        if ( alignL == 0 )
        {
            mpLeftPlotLegend->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        }
        else if ( alignL == 1 )
        {
            mpLeftPlotLegend->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
        }
        else
        {
            mpLeftPlotLegend->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        }

        if ( alignR == 0 )
        {
            mpRightPlotLegend->setAlignment(Qt::AlignTop | Qt::AlignRight);
        }
        else if ( alignR == 1 )
        {
            mpRightPlotLegend->setAlignment(Qt::AlignBottom | Qt::AlignRight);
        }
        else
        {
            mpRightPlotLegend->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        }

        QColor bgColor(mpLegendBgColor->currentText());
        //bgColor.setAlpha(128);
        mpRightPlotLegend->setBackgroundBrush(bgColor);
        mpLeftPlotLegend->setBackgroundBrush(bgColor);


        QFont fontl = mpLeftPlotLegend->font();
        fontl.setPointSize(mpLegendFontSize->value());
        mpLeftPlotLegend->setFont(fontl);

        QFont fontr = mpRightPlotLegend->font();
        fontr.setPointSize(mpLegendFontSize->value());
        mpRightPlotLegend->setFont(fontr);
    }
    else
    {
        mpRightPlotLegend->hide();
        mpLeftPlotLegend->hide();
    }

    // Set generation in title option
    //! @todo maybe this should be in a slot of its own
    for (int i=0; i<mPlotCurves.size(); ++i)
    {
        mPlotCurves[i]->setIncludeSourceInTitle(mpIncludeSourceInCurveTitle->isChecked());
        mPlotCurves[i]->setIncludeGenerationInTitle(mpIncludeGenInCurveTitle->isChecked());
        mPlotCurves[i]->refreshCurveTitle();
    }

    mpQwtPlot->insertLegend(NULL, QwtPlot::TopLegend);

    rescaleAxesToCurves();
}

void PlotArea::applyTimeScalingSettings()
{
    updateAxisLabels();
}

void PlotArea::enableZoom()
{
    // Make sure disabled
    mpQwtPanner->setEnabled(false);

    // Enable zoomer, (note! Right zoomer is slave to the left one)
    mpQwtZoomerLeft->setEnabled(true);
    mpQwtZoomerLeft->setRubberBand(QwtPicker::RectRubberBand);
    mpQwtZoomerRight->setEnabled(true);
}

void PlotArea::resetZoom()
{
    //    qDebug() << "Zoom stack L0: " << mpZoomerLeft[0]->zoomStack().size();
    //    qDebug() << "Zoom stack R0: " << mpZoomerRight[0]->zoomStack().size();
    if (!mpYLLockCheckBox->isChecked() && !mpXLockCheckBox->isChecked())
    {
        mpQwtZoomerLeft->zoom(0);
    }
    if (!mpYRLockCheckBox->isChecked() && !mpXLockCheckBox->isChecked())
    {
        mpQwtZoomerRight->zoom(0);
    }
    rescaleAxesToCurves();
}

void PlotArea::shiftAllGenerationsDown()
{
    PlotCurve *pCurve;
    Q_FOREACH(pCurve, mPlotCurves)
    {
        pCurve->setPreviousGeneration();
    }
}

void PlotArea::shiftAllGenerationsUp()
{
    PlotCurve *pCurve;
    Q_FOREACH(pCurve, mPlotCurves)
    {
        pCurve->setNextGeneration();
    }
}

//! @brief Inserts a curve marker at the specified curve
//! @param pCurve is a pointer to the specified curve
void PlotArea::insertMarker(PlotCurve *pCurve, double x, double y, QString altLabel, bool movable)
{
    PlotMarker *pMarker = new PlotMarker(pCurve, this);
    mPlotMarkers.append(pMarker);

    pMarker->attach(mpQwtPlot);
    pMarker->setXValue(x);
    pMarker->setYValue(y);

    if (altLabel.isEmpty())
    {
        pMarker->refreshLabel(x, y);
    }
    else
    {
        pMarker->refreshLabel(altLabel);
    }

    mpQwtPlot->canvas()->installEventFilter(pMarker);
    mpQwtPlot->canvas()->setMouseTracking(true);
    pMarker->setMovable(movable);
}

//! @brief Inserts a curve marker at the specified curve
//! @param pCurve is a pointer to the specified curve
void PlotArea::insertMarker(PlotCurve *pCurve, QPoint pos, bool movable)
{
//    PlotMarker *pMarker = new PlotMarker(pCurve, this);
//    mMarkerPtrs[plotID].append(pMarker);

//    pMarker->attach(mpQwtPlots[plotID]);
//    QCursor cursor;
//    pMarker->setXValue(pCurve->sample(pCurve->closestPoint(pos)).x());
//    pMarker->setYValue(mpQwtPlot->invTransform(QwtPlot::yLeft, mpQwtPlot->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));

//    pMarker->refreshLabel(pCurve->sample(pCurve->closestPoint(pos)).x(),
//                          pCurve->sample(pCurve->closestPoint(mpQwtPlots[plotID]->canvas()->mapFromGlobal(cursor.pos()))).y());

//    mpQwtPlots[plotID]->canvas()->installEventFilter(pMarker);
//    mpQwtPlots[plotID]->canvas()->setMouseTracking(true);
//    pMarker->setMovable(movable);


    double x = pCurve->sample(pCurve->closestPoint(pos)).x();
    double y = mpQwtPlot->invTransform(QwtPlot::yLeft, mpQwtPlot->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y()));
    //double y2 = pCurve->sample(pCurve->closestPoint(mpQwtPlots[plotID]->canvas()->mapFromGlobal(cursor.pos()))).y()
    insertMarker(pCurve, x, y, QString(), movable);
}

void PlotArea::refreshLockCheckBoxPositions()
{
    const int space = 2;

    // Calculate placement for time loc box
    QFont font = mpQwtPlot->axisFont(QwtPlot::xBottom); //Assume same font on all axes
    mpXLockCheckBox->move(0,mpQwtPlot->axisScaleDraw(QwtPlot::xBottom)->extent(font)+space);

    // We do not need to refresh left y axis since lock box will be in 0,0 allways, but we add space
    mpYLLockCheckBox->move(-space,0);

    // Calculate placement for right axis lock box
    mpYRLockCheckBox->move(mpQwtPlot->axisScaleDraw(QwtPlot::yRight)->extent(font)+space,0);
}

void PlotArea::axisLockHandler()
{
    mpQwtMagnifier->setAxisEnabled(QwtPlot::xBottom, !mpXLockCheckBox->isChecked());
    mpQwtPanner->setAxisEnabled(QwtPlot::xBottom, !mpXLockCheckBox->isChecked());

    mpQwtMagnifier->setAxisEnabled(QwtPlot::yLeft, !mpYLLockCheckBox->isChecked());
    mpQwtPanner->setAxisEnabled(QwtPlot::yLeft, !mpYLLockCheckBox->isChecked());

    mpQwtMagnifier->setAxisEnabled(QwtPlot::yRight, !mpYRLockCheckBox->isChecked());
    mpQwtPanner->setAxisEnabled(QwtPlot::yRight, !mpYRLockCheckBox->isChecked());
}

void PlotArea::constructLegendSettingsDialog()
{
    mpLegendSettingsDialog = new QDialog(this);
    mpLegendSettingsDialog->setWindowTitle("Legend Controls");
    mpLegendSettingsDialog->setWindowModality(Qt::WindowModal);

    mpLegendFontSize = new QSpinBox(this);
    mpLegendFontSize->setRange(1,100);
    mpLegendFontSize->setSingleStep(1);
    mpLegendFontSize->setValue(11);

    mpLegendCols = new QSpinBox(this);
    mpLegendCols->setRange(1,100);
    mpLegendCols->setSingleStep(1);
    mpLegendCols->setValue(1);

    mpLegendsAutoOffsetCheckBox = new QCheckBox(this);
    mpLegendsAutoOffsetCheckBox->setCheckable(true);
    mpLegendsAutoOffsetCheckBox->setChecked(true);

    mpLegendLeftOffset = new QDoubleSpinBox(this);
    mpLegendLeftOffset->setRange(-DoubleMax, DoubleMax);
    mpLegendLeftOffset->setDecimals(2);
    mpLegendLeftOffset->setSingleStep(0.1);
    mpLegendLeftOffset->setValue(0);
    mpLegendLeftOffset->setDisabled(mpLegendsAutoOffsetCheckBox->isChecked());

    mpLegendRightOffset = new QDoubleSpinBox(this);
    mpLegendRightOffset->setRange(-DoubleMax, DoubleMax);
    mpLegendRightOffset->setDecimals(2);
    mpLegendRightOffset->setSingleStep(0.1);
    mpLegendRightOffset->setValue(0);
    mpLegendRightOffset->setDisabled(mpLegendsAutoOffsetCheckBox->isChecked());

    mpLegendsEnabledCheckBox = new QCheckBox(this);
    mpLegendsEnabledCheckBox->setCheckable(true);
    mpLegendsEnabledCheckBox->setChecked(true);

    mpIncludeGenInCurveTitle = new QCheckBox(this);
    mpIncludeGenInCurveTitle->setCheckable(true);
    mpIncludeGenInCurveTitle->setChecked(true);

    mpIncludeSourceInCurveTitle = new QCheckBox(this);
    mpIncludeSourceInCurveTitle->setCheckable(true);
    mpIncludeSourceInCurveTitle->setChecked(false);

    mpLegendLPosition = new QComboBox(this);
    mpLegendLPosition->addItem("Top");
    mpLegendLPosition->addItem("Bottom");
    mpLegendLPosition->addItem("Centre");

    mpLegendRPosition = new QComboBox(this);
    mpLegendRPosition->addItem("Top");
    mpLegendRPosition->addItem("Bottom");
    mpLegendRPosition->addItem("Centre");

    mpLegendBgColor = new QComboBox(this);
    mpLegendBgColor->addItem("White");
    mpLegendBgColor->addItem("Red");
    mpLegendBgColor->addItem("Blue");
    mpLegendBgColor->addItem("Black");
    mpLegendBgColor->addItem("Maroon");
    mpLegendBgColor->addItem("Gray");
    mpLegendBgColor->addItem("LightSalmon");
    mpLegendBgColor->addItem("SteelBlue");
    mpLegendBgColor->addItem("Yellow");
    mpLegendBgColor->addItem("Gray");
    mpLegendBgColor->addItem("Fuchsia");
    mpLegendBgColor->addItem("PaleGreen");
    mpLegendBgColor->addItem("PaleTurquoise");
    mpLegendBgColor->addItem("Cornsilk");
    mpLegendBgColor->addItem("HotPink");
    mpLegendBgColor->addItem("Peru");
    mpLegendBgColor->addItem("Pink");

    mpLegendBgType = new QComboBox(this);
    mpLegendBgType->addItem("Legends", PlotLegend::LegendBackground);
    mpLegendBgType->addItem("Items", PlotLegend::ItemBackground);

    mpLegendSymbolType = new QComboBox(this);
    mpLegendSymbolType->addItem("Line&Symbol", PlotCurve::LegendShowLineAndSymbol);
    mpLegendSymbolType->addItem("Line", PlotCurve::LegendShowLine );
    mpLegendSymbolType->addItem("Symbol", PlotCurve::LegendShowSymbol );
    mpLegendSymbolType->addItem("Rectangle", PlotCurve::LegendNoAttribute );

    QGridLayout *legendBoxLayout = new QGridLayout();

    const int blankline = 12;
    int row = 0;
    legendBoxLayout->addWidget( new QLabel( "Legends on/off: " ), row, 0 );
    legendBoxLayout->addWidget( mpLegendsEnabledCheckBox, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Size: " ), row, 0, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendFontSize, row, 1 );
    legendBoxLayout->addWidget( new QLabel( "Columns: " ), row, 2, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendCols, row, 3 );
    row++;
    legendBoxLayout->addWidget( new QLabel( "Symbol Type: " ), row, 0, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendSymbolType, row, 1 );
    row++;
    legendBoxLayout->addWidget( new QLabel( "Include generation in curve name: " ), row, 0, 1, 2, Qt::AlignRight );
    legendBoxLayout->addWidget( mpIncludeGenInCurveTitle, row, 2 );
    row++;
    legendBoxLayout->addWidget( new QLabel( "Include source in curve name: " ), row, 0, 1, 2, Qt::AlignRight );
    legendBoxLayout->addWidget( mpIncludeSourceInCurveTitle, row, 2 );


    row++;
    legendBoxLayout->setRowMinimumHeight(row, blankline);
    row++;
    legendBoxLayout->addWidget( new QLabel( "Legend background" ), row, 0, 1, 4, Qt::AlignHCenter);
    row++;
    legendBoxLayout->addWidget( new QLabel( "Color: "), row, 0, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendBgColor, row, 1 );
    legendBoxLayout->addWidget( new QLabel( "Type: " ), row, 2, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendBgType, row, 3 );

    row++;
    legendBoxLayout->setRowMinimumHeight(row, blankline);
    row++;
    legendBoxLayout->addWidget( new QLabel( "Vertical legend position" ), row, 0, 1, 4, Qt::AlignHCenter);
    row++;
    legendBoxLayout->addWidget( new QLabel( "Left: " ), row, 0, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendLPosition, row, 1 );
    legendBoxLayout->addWidget( new QLabel( "Right: " ), row, 2, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendRPosition, row, 3 );

    row++;
    legendBoxLayout->setRowMinimumHeight(row, blankline);
    row++;
    legendBoxLayout->addWidget( new QLabel( "Axis compensation for legend height" ), row, 0, 1, 4, Qt::AlignHCenter);
    row++;
    legendBoxLayout->addWidget( new QLabel( "Left: " ), row, 0, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendLeftOffset, row, 1 );
    legendBoxLayout->addWidget( new QLabel( "Right: " ), row, 2, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendRightOffset, row, 3 );
    row++;
    legendBoxLayout->addWidget( new QLabel( "Auto Offset: " ), row, 0, 1, 1, Qt::AlignRight );
    legendBoxLayout->addWidget( mpLegendsAutoOffsetCheckBox, row, 1 );

    row++;
    QPushButton *pFinishedLegButton = new QPushButton("Close", mpLegendSettingsDialog);
    QDialogButtonBox *pFinishedLegButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pFinishedLegButtonBox->addButton(pFinishedLegButton, QDialogButtonBox::ActionRole);
    legendBoxLayout->addWidget( pFinishedLegButton, row, 3 );

    mpLegendSettingsDialog->setLayout(legendBoxLayout);

    connect(mpLegendFontSize, SIGNAL(valueChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendCols, SIGNAL(valueChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendsEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(mpIncludeGenInCurveTitle, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(mpIncludeSourceInCurveTitle, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(mpLegendBgType, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendSymbolType, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendLPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendRPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendBgColor, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendLeftOffset, SIGNAL(valueChanged(double)), this, SLOT(applyLegendSettings()));
    connect(mpLegendRightOffset, SIGNAL(valueChanged(double)), this, SLOT(applyLegendSettings()));
    connect(mpLegendsAutoOffsetCheckBox, SIGNAL(toggled(bool)), mpLegendLeftOffset, SLOT(setDisabled(bool)));
    connect(mpLegendsAutoOffsetCheckBox, SIGNAL(toggled(bool)), mpLegendRightOffset, SLOT(setDisabled(bool)));
    connect(mpLegendsAutoOffsetCheckBox, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(pFinishedLegButton, SIGNAL(clicked()), mpLegendSettingsDialog, SLOT(close()));
}

void PlotArea::constructAxisSettingsDialog()
{
    mpSetAxisDialog = new QDialog(this);
    mpSetAxisDialog->setWindowTitle("Set Axis Limits");

    mpXLockDialogCheckBox = new QCheckBox("Locked Limits");
    mpXLockDialogCheckBox->setCheckable(true);
    mpXLockDialogCheckBox->setChecked(false);
    mpYLLockDialogCheckBox = new QCheckBox("Locked Limits");
    mpYLLockDialogCheckBox->setCheckable(true);
    mpYLLockDialogCheckBox->setChecked(false);
    mpYRLockDialogCheckBox = new QCheckBox("Locked Limits");
    mpYRLockDialogCheckBox->setCheckable(true);
    mpYRLockDialogCheckBox->setChecked(false);

    QDoubleValidator *pDoubleValidator = new QDoubleValidator(mpSetAxisDialog);

    mpXminLineEdit = new QLineEdit(mpSetAxisDialog);
    mpXminLineEdit->setValidator(pDoubleValidator);

    mpXmaxLineEdit = new QLineEdit(mpSetAxisDialog);
    mpXmaxLineEdit->setValidator(pDoubleValidator);

    mpYLminLineEdit = new QLineEdit(mpSetAxisDialog);
    mpYLminLineEdit->setValidator(pDoubleValidator);

    mpYLmaxLineEdit = new QLineEdit(mpSetAxisDialog);
    mpYLmaxLineEdit->setValidator(pDoubleValidator);

    mpYRminLineEdit = new QLineEdit(mpSetAxisDialog);
    mpYRminLineEdit->setValidator(pDoubleValidator);

    mpYRmaxLineEdit = new QLineEdit(mpSetAxisDialog);
    mpYRmaxLineEdit->setValidator(pDoubleValidator);


    QPushButton *pFinishedButton = new QPushButton("Done", mpSetAxisDialog);
    QDialogButtonBox *pFinishedButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pFinishedButtonBox->addButton(pFinishedButton, QDialogButtonBox::ActionRole);

    QGridLayout *pAxisLimitsDialogLayout = new QGridLayout(mpSetAxisDialog);


    int r,c;
    r=0;c=0;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("Left Y Axis")),r,c,1,2, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("max")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYLmaxLineEdit, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("min")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYLminLineEdit, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpYLLockDialogCheckBox, r, c, 1, 2, Qt::AlignCenter);
    pAxisLimitsDialogLayout->setColumnMinimumWidth(c+2, 20);

    r=0;c=6;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("Right Y Axis")),r,c,1,2, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("max")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYRmaxLineEdit, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("min")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYRminLineEdit, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpYRLockDialogCheckBox, r, c, 1, 2, Qt::AlignCenter);

    r=3,c=3;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("X Axis")),r,c,1,2, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("min")), r, c, Qt::AlignCenter);
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("max")), r, c+1, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpXminLineEdit, r, c);
    pAxisLimitsDialogLayout->addWidget(mpXmaxLineEdit, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpXLockDialogCheckBox, r, c, 1, 2, Qt::AlignCenter);
    pAxisLimitsDialogLayout->setColumnMinimumWidth(c+2, 20);

    r=6;c=7;
    pAxisLimitsDialogLayout->addWidget(pFinishedButtonBox, r, c, 1, 2);

    mpSetAxisDialog->setLayout(pAxisLimitsDialogLayout);

    // Connect persistent connections
    connect(pFinishedButton,    SIGNAL(clicked()),              mpSetAxisDialog, SLOT(close()));

}

void PlotArea::constructAxisLabelDialog()
{
    mpUserDefinedLabelsDialog = new QDialog(this);
    mpUserDefinedLabelsDialog->setWindowTitle("Set custom axis labels");

    mpUserDefinedLabelsCheckBox = new QCheckBox("Activate user defined labels");
    mpUserDefinedLabelsCheckBox->setCheckable(true);
    mpUserDefinedLabelsCheckBox->setChecked(true);

    mpUserDefinedXLabel = new QLineEdit();
    mpUserDefinedYlLabel = new QLineEdit();
    mpUserDefinedYrLabel = new QLineEdit();

    QGridLayout *pGridLayout = new QGridLayout(mpUserDefinedLabelsDialog);

    pGridLayout->addWidget(new QLabel(tr("Left axis")), 0, 0);
    pGridLayout->addWidget(mpUserDefinedYlLabel, 0, 1);
    pGridLayout->addWidget(new QLabel(tr("Right axis")), 0, 4);
    pGridLayout->addWidget(mpUserDefinedYrLabel, 0, 5);
    pGridLayout->addWidget(new QLabel(tr("Bottom axis")), 1, 2);
    pGridLayout->addWidget(mpUserDefinedXLabel, 1, 3);

    pGridLayout->addWidget(mpUserDefinedLabelsCheckBox, 2, 0, 1, 3);

    QPushButton *pFinishedButton = new QPushButton("Done", mpSetAxisDialog);
    QDialogButtonBox *pFinishedButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pFinishedButtonBox->addButton(pFinishedButton, QDialogButtonBox::ActionRole);

    pGridLayout->addWidget(pFinishedButtonBox, 3, 5);

    connect(pFinishedButton, SIGNAL(clicked()), this, SLOT(applyAxisLabelSettings()));
    connect(pFinishedButton, SIGNAL(clicked()), mpUserDefinedLabelsDialog, SLOT(close()));
}

//! @brief Help function to set legend symbole style for all curves
void PlotArea::setLegendSymbol(const QString symStyle)
{
    for(int j=0; j<mPlotCurves.size(); ++j)
    {
        setLegendSymbol(symStyle, mPlotCurves[j]);
    }
}

//! @brief Help function to set legend symbole style for one particualr curve
void PlotArea::setLegendSymbol(const QString symStyle, PlotCurve *pCurve)
{
    pCurve->setLegendAttribute( PlotCurve::LegendNoAttribute, false);
    pCurve->setLegendAttribute( PlotCurve::LegendShowLine, false);
    pCurve->setLegendAttribute( PlotCurve::LegendShowSymbol, false);
    pCurve->setLegendAttribute( PlotCurve::LegendShowBrush, false);

    if( symStyle == "Rectangle")
    {
        pCurve->setLegendAttribute( PlotCurve::LegendNoAttribute, true);
    }
    else if( symStyle == "Line")
    {
        pCurve->setLegendAttribute( PlotCurve::LegendShowLine, true);
    }
    else if( symStyle == "Symbol")
    {
        pCurve->setLegendAttribute( PlotCurve::LegendShowSymbol, true);
     }
    else if ( symStyle == "Line&Symbol")
    {
        pCurve->setLegendAttribute( PlotCurve::LegendShowLine, true);
        pCurve->setLegendAttribute( PlotCurve::LegendShowSymbol, true);
    }
    else if( symStyle == "Brush")
    {
        pCurve->setLegendAttribute( PlotCurve::LegendShowBrush, true);
    }

    // Fix legend size after possible change in style
    pCurve->resetLegendSize();
}

void PlotArea::setTabOnlyCustomXVector(HopsanVariable data)
{
    mHasCustomXData = true; //!< @todo we could get rid of this bool, and check data directly
    mCustomXData = data;

    updateAxisLabels();
    replot();

}

void PlotArea::determineAddedCurveUnitOrScale(PlotCurve *pCurve)
{
    // If a custom plotunit is set then use that
    const QString &custPlotUnit = pCurve->getDataCustomPlotUnit();
    if (custPlotUnit.isEmpty())
    {
        // Else use the default unit for this curve, unless it is a "Value" with an actual unit set
        const QString &dataUnit = pCurve->getDataOriginalUnit();
        QString defaultUnit;
        if ( pCurve->getDataName() != "Value" )
        {
            defaultUnit = gpConfig->getDefaultUnit(pCurve->getDataName());
        }
        else
        {
            QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit(dataUnit);
            //! @todo if same unit exist in multiple places we have a problem
            if (pqs.size() > 1)
            {
                gpMessageHandler->addWarningMessage(QString("Unit %1 is associated to multiple physical quantities, default unit selection may be incorrect").arg(dataUnit));
            }
            if (pqs.size() == 1)
            {
                defaultUnit = gpConfig->getDefaultUnit(pqs.first());
            }
        }


        if (!defaultUnit.isEmpty() && (defaultUnit != dataUnit) )
        {
            pCurve->setCustomCurveDataUnit(defaultUnit);
        }
        //! @todo maybe this elseif can be removed in the future
        else if (pCurve->getDataCustomPlotUnit() == "-")
        {
            pCurve->setCustomCurveDataUnit(defaultUnit);
        }

        // If all curves on the same axis has the same custom unit, assign this unit to the new curve as well
        QString customUnit;
        for(int i=0; i<mPlotCurves.size(); ++i)
        {
            if(mPlotCurves[i]->getAxisY() == pCurve->getAxisY())
            {
                if(customUnit.isEmpty())
                {
                    customUnit = mPlotCurves[i]->getCurrentUnit();
                }
                else if(customUnit != mPlotCurves[i]->getCurrentUnit())  //Unit is different between the other curves, so don't use it
                {
                    customUnit = QString();
                    break;
                }
            }
        }

        // If we have found a custom unit and the original unit is not empty then try to set custom unit
        if(!customUnit.isEmpty() && !pCurve->getDataOriginalUnit().isEmpty())
        {
            pCurve->setCustomCurveDataUnit(customUnit);
        }
    }
    // Else the given plot unit in the data will be used
}

void PlotArea::rescaleAxisLimitsToMakeRoomForLegend(const QwtPlot::Axis axisId, QwtInterval &rAxisLimits)
{
    //! @todo only works for top buffer right now
    if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlot->axisScaleEngine(axisId)))
    {
        //! @todo what should happen here ?
        mpQwtPlot->setAxisAutoScale(axisId, true);
    }
    else
    {
        // Curves range
        const double cr = rAxisLimits.width();

        // Find largest legend height in pixels
        double lht, lhb;
        calculateLegendBufferOffsets(axisId, lhb, lht);

        // Axis height
        const double ah = mpQwtPlot->axisWidget(axisId)->size().height();

        // Remove legend and margin height from axis height, what remains is the height for the curves
        // Divid with the curves value range to get the scale
        double s = (ah-(lht+lhb))/cr; //[px/unit]

        // Dont try to change axis limits if legend is higher then teh axis that will look strange and risk krashing Hopsan when axis limit -> inf
        if (s > 0)
        {
            //qDebug() << "s: " << s;
            s = qMax(s,1e-100); // Limit to prevent div by 0

            // Calculate new axis range for current axis height given the scale
            const double ar = ah/s;

            rAxisLimits.setMaxValue(rAxisLimits.minValue() + ar - lhb/s);
            rAxisLimits.setMinValue(rAxisLimits.minValue() - lhb/s);
        }
    }
}

//! @todo only works for linear scale right now, need to check for log scale also
void PlotArea::calculateLegendBufferOffsets(const QwtPlot::Axis axisId, double &rBottomOffset, double &rTopOffset)
{
    double leftLegendHeight=0, rightLegendHeight=0;
    if (mpLeftPlotLegend->isVisible())
    {
        leftLegendHeight = mpLeftPlotLegend->geometry(mpQwtPlot->geometry()).height() + mpLeftPlotLegend->borderDistance();
    }
    if (mpRightPlotLegend->isVisible())
    {
        rightLegendHeight = mpRightPlotLegend->geometry(mpQwtPlot->geometry()).height() + mpRightPlotLegend->borderDistance();
    }
    //! @todo even if a legend is empty it seems to be visible and the borderDistance will be added, this causes unecssary space when on top or bottom (and the other legend is not)

    // Figure out vertical alginemnt, by bitwise masking
    Qt::Alignment lva = mpLeftPlotLegend->alignment() & Qt::AlignVertical_Mask;
    Qt::Alignment rva = mpRightPlotLegend->alignment() & Qt::AlignVertical_Mask;

    rBottomOffset = rTopOffset = 0;
    if(mpLegendsAutoOffsetCheckBox->isChecked())
    {
        if ( (lva == Qt::AlignTop) && (rva == Qt::AlignTop) )
        {
           rTopOffset = qMax(leftLegendHeight,rightLegendHeight);
        }
        else if ( (lva == Qt::AlignBottom) && (rva == Qt::AlignBottom) )
        {
            rBottomOffset = qMax(leftLegendHeight,rightLegendHeight);
        }
        else if ( (lva == Qt::AlignBottom) && (rva == Qt::AlignTop) )
        {
            if (axisId == QwtPlot::yLeft)
            {
                rBottomOffset = leftLegendHeight;
                rTopOffset = rightLegendHeight;
            }
            else
            {
                rBottomOffset = rightLegendHeight;
                rTopOffset = leftLegendHeight;
            }
        }
        else if ( (lva == Qt::AlignTop) && (rva == Qt::AlignBottom) )
        {
            if (axisId == QwtPlot::yLeft)
            {
                rBottomOffset = rightLegendHeight;
                rTopOffset = leftLegendHeight;
            }
            else
            {
                rBottomOffset = leftLegendHeight;
                rTopOffset = rightLegendHeight;
            }
        }
        else if ( (lva == Qt::AlignVCenter) && (axisId == QwtPlot::yRight)  )
        {
            if (rva == Qt::AlignTop)
            {
                rTopOffset = rightLegendHeight;
            }
            else if (rva == Qt::AlignBottom)
            {
                rBottomOffset = rightLegendHeight;
            }
        }
        else if ( (rva == Qt::AlignVCenter) && (axisId == QwtPlot::yLeft)  )
        {
            if (lva == Qt::AlignTop)
            {
                rTopOffset = leftLegendHeight;
            }
            else if (lva == Qt::AlignBottom)
            {
                rBottomOffset = leftLegendHeight;
            }
        }
    }
    else
    {
        if (axisId == QwtPlot::yLeft)
        {
            if (lva == Qt::AlignTop)
            {
                rTopOffset = mpLegendLeftOffset->value()*leftLegendHeight;
            }
            else if (lva == Qt::AlignBottom)
            {
                rBottomOffset = mpLegendLeftOffset->value()*leftLegendHeight;
            }
        }
        else if (axisId == QwtPlot::yRight)
        {
            if (rva == Qt::AlignTop)
            {
                rTopOffset = mpLegendLeftOffset->value()*leftLegendHeight;
            }
            else if (rva == Qt::AlignBottom)
            {
                rBottomOffset = mpLegendLeftOffset->value()*leftLegendHeight;
            }
        }
        //! @todo Center? than what to do
    }
}

void PlotArea::updatePlotMarkers()
{
    for(int i=0; i<mPlotMarkers.size(); ++i)
    {
        PlotCurve *pCurve = mPlotMarkers[i]->getCurve();
        QPointF posF = mPlotMarkers[i]->value();
        double x = mpQwtPlot->transform(QwtPlot::xBottom, posF.x());
        double y = mpQwtPlot->transform(QwtPlot::yLeft, posF.y());
        QPointF closestPoint = pCurve->sample(pCurve->closestPoint(QPoint(x,y)));

        mPlotMarkers[i]->setXValue(closestPoint.x());
        mPlotMarkers[i]->setYValue(mpQwtPlot->invTransform(QwtPlot::yLeft, mpQwtPlot->transform(pCurve->yAxis(), closestPoint.y())));
        mPlotMarkers[i]->refreshLabel(closestPoint.x(), mpQwtPlot->invTransform(QwtPlot::yLeft, mpQwtPlot->transform(pCurve->yAxis(), closestPoint.y())));
        //!< @todo label text will be wrong if curve data has changed
        //!< @todo label text will be wrong if plot sclae or offset change
    }
}

void PlotArea::updateWindowtitleModelName()
{
    //! @todo instead of string, maybe should use shared pointers (in the data variables to avoid duplicating the string for variables from same model) /Peter
    mModelPaths.clear();
    foreach(PlotCurve *pCurve, mPlotCurves)
    {
        const QString &name = pCurve->getVariable()->getModelPath();
        if (!mModelPaths.contains(name) && !name.isEmpty())
        {
            mModelPaths.append(name);
        }
    }
    emit refreshContainsDataFromModels();
}

void PlotArea::enableArrow()
{
    mpQwtZoomerLeft->setEnabled(false);
    mpQwtZoomerLeft->setRubberBand(QwtPicker::NoRubberBand);
    mpQwtZoomerRight->setEnabled(false);
    mpQwtPanner->setEnabled(false);
}

void PlotArea::enablePan()
{
    mpQwtZoomerLeft->setEnabled(false);
    mpQwtZoomerLeft->setRubberBand(QwtPicker::NoRubberBand);
    mpQwtZoomerRight->setEnabled(false);
    mpQwtPanner->setEnabled(true);
}

void PlotArea::enableGrid(bool value)
{
    mpQwtPlotGrid->setVisible(value);
}

void PlotArea::setBackgroundColor(const QColor &rColor)
{
    if (rColor.isValid())
    {
        mpQwtPlot->setCanvasBackground(rColor);

        //! @todo should have some functions for these two or three, they are used in many places
        mpQwtPlot->replot();
        mpQwtPlot->updateGeometry();
    }
}

//! @todo renamethis function
void PlotArea::resetXTimeVector()
{
    mHasCustomXData = false;
    mCustomXData = HopsanVariable();

    PlotCurve *pCurve;
    Q_FOREACH(pCurve, mPlotCurves)
    {
        pCurve->setCustomXData(mCustomXData); //Remove any custom x-data
    }

    rescaleAxesToCurves();
    updateAxisLabels();
    replot();
}





CurveColorSelector::CurveColorSelector()
{
    // Init colors
    mCurveColors << "Blue" << "Red" << "Green" << "Orange" << "Pink" << "Brown" << "Purple" << "Gray";

    // Init color counters
    mUsedColorsCounters.reserve(mCurveColors.size());
    for(int i=0; i<mCurveColors.size(); ++i)
    {
        mUsedColorsCounters.append(0);
    }
}

QColor CurveColorSelector::getLeastCommonCurveColor()
{
    // Find the curve that has been used to least amount of times
    for(int i=0; ; ++i)
    {
        // We check each curve in order
        for(int c=0; c<mUsedColorsCounters.size(); ++c)
        {
            if(mUsedColorsCounters[c] == i)
            {
                ++mUsedColorsCounters[c];
                return mCurveColors[c];
            }
        }
    }
}

void CurveColorSelector::decrementCurveColorCounter(const QColor &rColor)
{
    for (int c=0; c<mCurveColors.size(); ++c)
    {
        if (rColor==mCurveColors[c])
        {
            --mUsedColorsCounters[c];
            break;
        }
    }
}

void CurveColorSelector::incrementCurveColorCounter(const QColor &rColor)
{
    for (int c=0; c<mCurveColors.size(); ++c)
    {
        if (rColor==mCurveColors[c])
        {
            ++mUsedColorsCounters[c];
            break;
        }
    }
}
