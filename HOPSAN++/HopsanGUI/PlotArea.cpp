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
#include "PlotTab.h"
#include "global.h"
#include "Configuration.h"
#include "Utilities/GUIUtilities.h"
#include "ModelHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "PlotWindow.h"
#include "Widgets/HcomWidget.h"

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


TimeScaleWidget::TimeScaleWidget(SharedVariablePtrT pTime, QWidget *pParent) : QWidget(pParent)
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

    mCurveColors << "Blue" << "Red" << "Green" << "Orange" << "Pink" << "Brown" << "Purple" << "Gray";
    for(int i=0; i<mCurveColors.size(); ++i)
    {
        mUsedColorsCounter.append(0);
    }

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
    if(mHasCustomXData)
    {
        if (pCurve->hasCustomXData())
        {
            //! @todo check that same unit
            qWarning("todo Check that same unit");
        }
        else
        {
            pCurve->setCustomXData(mpCustomXData);
        }
    }

    determineAddedCurveUnitOrScale(pCurve);

    pCurve->attach(mpQwtPlot);
    pCurve->setParentPlotArea(this);
    mPlotCurves.append(pCurve);
    connect(pCurve, SIGNAL(curveDataUpdated()), this, SLOT(rescaleAxesToCurves()));

    // Create a curve info box for this curve
    mPlotCurveControlBoxes.append(new PlotCurveControlBox(pCurve, this));
    mpParentPlotTab->mpCurveInfoScrollArea->widget()->layout()->addWidget(mPlotCurveControlBoxes.last());
    mPlotCurveControlBoxes.last()->show();



    for(int i=0; true; ++i)
    {
        bool stop=false;
        for(int j=0; j<mUsedColorsCounter.size(); ++j)
        {
            if(mUsedColorsCounter[j] == i)
            {
                pCurve->setLineColor(mCurveColors[j]);
                ++mUsedColorsCounter[j];
                stop=true;
                break;
            }
        }
        if(stop) break;
    }

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
    pCurve->setZ(CurveZOrderType);
    pCurve->setLineWidth(2);
    setLegendSymbol(mpLegendSymbolType->currentText());

    //! @todo maybe make it possible to rescale only selected axis, instead of always recscaling both of them
    rescaleAxesToCurves();
    updateAxisLabels();
    mpQwtPlot->replot();
    mpQwtPlot->updateGeometry();


    //! @todo FIXA /Peter
    //mpParentPlotWindow->mpBodePlotButton->setEnabled(mPlotCurvePtrs[0].size() > 1);
}

void PlotArea::setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc)
{
    SharedVariableDescriptionT pVarDesc(new VariableDescription(rVarDesc));
    setCustomXVectorForAll(createFreeVectorVariable(xArray, pVarDesc));
}

void PlotArea::setCustomXVectorForAll(SharedVariablePtrT pData)
{
    for(int i=0; i<mPlotCurves.size(); ++i)
    {
        if (!mPlotCurves[i]->hasCustomXData())
        {
            mPlotCurves[i]->setCustomXData(pData);
        }
    }
    rescaleAxesToCurves();
    setTabOnlyCustomXVector(pData);
}

void PlotArea::removeCurve(PlotCurve *pCurve)
{
    // Remove any markes used by the curve
    for(int i=0; i<mPlotMarkers.size(); ++i)
    {
        if(mPlotMarkers[i]->getCurve() == pCurve)
        {
            mpQwtPlot->canvas()->removeEventFilter(mPlotMarkers[i]);
            mPlotMarkers[i]->detach();
            mPlotMarkers.removeAt(i);
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

    for(int i=0; i<mCurveColors.size(); ++i)
    {
        if(pCurve->pen().color() == mCurveColors[i])
        {
            --mUsedColorsCounter[i];
            break;
        }
    }

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


    pCurve->deleteLater();

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
    update();
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

const SharedVariablePtrT PlotArea::getCustomXData() const
{
    return mpCustomXData;
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

void PlotArea::update()
{
    mpQwtPlot->enableAxis(QwtPlot::yLeft, false);
    mpQwtPlot->enableAxis(QwtPlot::yRight, false);
    QList<PlotCurve *>::iterator cit;
    for(cit=mPlotCurves.begin(); cit!=mPlotCurves.end(); ++cit)
    {
        if(!mpQwtPlot->axisEnabled((*cit)->getAxisY())) { mpQwtPlot->enableAxis((*cit)->getAxisY()); }
        (*cit)->attach(mpQwtPlot);
    }

    // Update plotmarkers
    for(int i=0; i<mPlotMarkers.size(); ++i)
    {
        QPointF posF = mPlotMarkers[i]->value();
        double x = mpQwtPlot->transform(QwtPlot::xBottom, posF.x());
        double y = mpQwtPlot->transform(QwtPlot::yLeft, posF.y());
        QPoint pos = QPoint(x,y);
        PlotCurve *pCurve = mPlotMarkers[i]->getCurve();
        mPlotMarkers[i]->setXValue(pCurve->sample(pCurve->closestPoint(pos)).x());
        mPlotMarkers[i]->setYValue(mpQwtPlot->invTransform(QwtPlot::yLeft, mpQwtPlot->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));
        mPlotMarkers[i]->refreshLabel(pCurve->sample(pCurve->closestPoint(pos)).x(), mpQwtPlot->invTransform(QwtPlot::yLeft, mpQwtPlot->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));
        //!< @todo label text will be wrong if curve data has changed
        //!< @todo label text will be wrong if plot sclae or offset change
    }
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
    //! @todo FIXA /Peter
    //if(mPlotCurves[0].size() > 0 && mPlotCurves[0][0]->getCurveType() != PortVariableType) return;

    if (event->mimeData()->hasText())
    {
        // Create the hover rectangle (size will be changed by dragMoveEvent)
        mpPainterWidget->createRect(0,0,this->width(), this->height());
        event->acceptProposedAction();
    }
}

void PlotArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    mpPainterWidget->clearRect();

    // Don't accept drag events to FFT and Bode plots
    //! @todo FIXA /Peter
    //if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    QWidget::dragLeaveEvent(event);
}

void PlotArea::dragMoveEvent(QDragMoveEvent *event)
{
    //Don't accept drag events to FFT and Bode plots
    //! @todo FIXA /Peter
    //if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    QCursor cursor;
    if(this->mapFromGlobal(cursor.pos()).y() > getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y()+10 && getNumberOfCurves() >= 1)
    {
        mpPainterWidget->createRect(getQwtPlot()->canvas()->x(), getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y(), getQwtPlot()->canvas()->width(), getQwtPlot()->canvas()->height()*1.0/3.0);
        //mpParentPlotWindow->showHelpPopupMessage("Replace X-axis with selected variable.");
    }
    else if(this->mapFromGlobal(cursor.pos()).x() < getQwtPlot()->canvas()->x()+9 + getQwtPlot()->canvas()->width()/2)
    {
        mpPainterWidget->createRect(getQwtPlot()->canvas()->x(), getQwtPlot()->canvas()->y(), getQwtPlot()->canvas()->width()/2, getQwtPlot()->canvas()->height());
        //mpParentPlotWindow->showHelpPopupMessage("Add selected variable to left Y-axis.");
    }
    else
    {
        mpPainterWidget->createRect(getQwtPlot()->canvas()->x() + getQwtPlot()->canvas()->width()/2, getQwtPlot()->canvas()->y(), getQwtPlot()->canvas()->width()/2, getQwtPlot()->canvas()->height());
        //mpParentPlotWindow->showHelpPopupMessage("Add selected variable to right Y-axis.");
        //! @todo FIXA help popups /Peter
    }
    QWidget::dragMoveEvent(event);
}

void PlotArea::dropEvent(QDropEvent *event)
{
    QWidget::dropEvent(event);

    mpPainterWidget->clearRect();

    //Don't accept drag events to FFT and Bode plots
    //! @todo FIXA /Peter
    //if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    if (event->mimeData()->hasText())
    {
        QString mimeText = event->mimeData()->text();
        if(mimeText.startsWith("HOPSANPLOTDATA:"))
        {
            qDebug() << mimeText;
            QStringList fields = mimeText.split(":");
            if (fields.size() > 2)
            {
                QString &name = fields[1];
                bool parseOk = false;
                int gen = fields[2].toInt(&parseOk);
                if (!parseOk)
                {
                    gen = -1;
                }

                QCursor cursor;
                if(this->mapFromGlobal(cursor.pos()).y() > getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y()+10 && getNumberOfCurves() >= 1)
                {
                    setCustomXVectorForAll(gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getLogVariableDataPtr(name, gen));
                    //! @todo do we need to reset to default unit too ?
                }
                else if(this->mapFromGlobal(cursor.pos()).x() < getQwtPlot()->canvas()->x()+9 + getQwtPlot()->canvas()->width()/2)
                {
                    mpParentPlotTab->mpParentPlotWindow->addPlotCurve(gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getLogVariableDataPtr(name, gen), QwtPlot::yLeft);
                }
                else
                {
                    mpParentPlotTab->mpParentPlotWindow->addPlotCurve(gpModelHandler->getCurrentViewContainerObject()->getLogDataHandler()->getLogVariableDataPtr(name, gen), QwtPlot::yRight);
                }
            }
        }
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
    QMenu *pChangeUnitsMenu;
    QMenu *pInsertMarkerMenu;

    QAction *pSetRightAxisLogarithmic = 0;
    QAction *pSetLeftAxisLogarithmic = 0;
    QAction *pSetUserDefinedAxisLabels = 0;

    pYAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    pYAxisRightMenu = menu.addMenu(QString("Right Y Axis"));

    pYAxisLeftMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::yLeft));
    pYAxisRightMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::yRight));


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


    // Create menu for inserting curve markers
    pInsertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    for(int plotID=0; plotID<2; ++plotID)
    {
        for(itc=mPlotCurves.begin(); itc!=mPlotCurves.end(); ++itc)
        {
            QAction *pTempAction = pInsertMarkerMenu->addAction((*itc)->getCurveName());
            actionToCurveMap.insert(pTempAction, (*itc));
        }
    }

    // Create option for changing axis labels
    pSetUserDefinedAxisLabels = menu.addAction("Set userdefined axis labels");

    // ----- Wait for user to make a selection ----- //

    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    // ----- User has selected something -----  //



    // Check if user did not click on a menu item
    if(selectedAction == 0)
    {
        return;
    }


    // Change unit on selected curve
    if(selectedAction->parentWidget()->parentWidget() == pChangeUnitsMenu)
    {
        actionToCurveMap.find(selectedAction).value()->setCustomCurveDataUnit(selectedAction->text());
    }


    //Make axis logarithmic
    if (selectedAction == pSetRightAxisLogarithmic)
    {
        mRightAxisLogarithmic = !mRightAxisLogarithmic;
        if(mRightAxisLogarithmic)
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine(10));
            rescaleAxesToCurves();
            mpQwtPlot->replot();
            mpQwtPlot->updateGeometry();
        }
        else
        {
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
            rescaleAxesToCurves();
            mpQwtPlot->replot();
            mpQwtPlot->updateGeometry();
        }
    }
    else if (selectedAction == pSetLeftAxisLogarithmic)
    {
        mLeftAxisLogarithmic = !mLeftAxisLogarithmic;
        if(mLeftAxisLogarithmic)
        {
            qDebug() << "Logarithmic!";
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
            rescaleAxesToCurves();
            mpQwtPlot->replot();
            mpQwtPlot->updateGeometry();
        }
        else
        {
            qDebug() << "Linear!";
            mpQwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
            rescaleAxesToCurves();
            mpQwtPlot->replot();
            mpQwtPlot->updateGeometry();
        }
    }

    // Set user axes labels
    if (selectedAction == pSetUserDefinedAxisLabels)
    {
        openAxisLabelDialog();
    }


    //Insert curve marker
    if(selectedAction->parentWidget() == pInsertMarkerMenu)
    {
        insertMarker(actionToCurveMap.find(selectedAction).value(), event->pos());
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

        // Initialize values for X axis by using the first curve
        xAxisLim.setMinValue(mPlotCurves.first()->minXValue());
        xAxisLim.setMaxValue(mPlotCurves.first()->maxXValue());

        bool someoneHasCustomXdata = false;
        for(int i=0; i<mPlotCurves.size(); ++i)
        {
            // First check if some curve has a custom x-axis and plot does not
            someoneHasCustomXdata = someoneHasCustomXdata || mPlotCurves[i]->hasCustomXData();
            if (!mHasCustomXData && someoneHasCustomXdata)
            {
                //! @todo maybe should do this with signal slot instead, to avoid unesisarry checks all the time
                setTabOnlyCustomXVector(mPlotCurves[i]->getCustomXData());
            }

            if(mPlotCurves[i]->getAxisY() == QwtPlot::yLeft)
            {
                if(mLeftAxisLogarithmic)
                {
                    // Only consider positive values if logarithmic scaling (negative ones will be discarded by Qwt)
                    ylAxisLim.extendMin(qMax(mPlotCurves[i]->minYValue(), Double100Min));
                }
                else
                {
                    ylAxisLim.extendMin(mPlotCurves[i]->minYValue());
                }
                ylAxisLim.extendMax(mPlotCurves[i]->maxYValue());
            }

            if(mPlotCurves[i]->getAxisY() == QwtPlot::yRight)
            {
                if(mRightAxisLogarithmic)
                {
                    // Only consider positive values if logarithmic scaling (negative ones will be discarded by Qwt)
                    yrAxisLim.extendMin(qMax(mPlotCurves[i]->minYValue(), Double100Min));
                }
                else
                {
                    yrAxisLim.extendMin(mPlotCurves[i]->minYValue());
                }
                yrAxisLim.extendMax(mPlotCurves[i]->maxYValue());
            }

            // find min / max x-value
            xAxisLim.extendMin(mPlotCurves[i]->minXValue());
            xAxisLim.extendMax(mPlotCurves[i]->maxXValue());
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

    if (mPlotCurves.size()>0)
    {
        if(mPlotCurves.first()->getCurveType() == PortVariableType)
        {
            QStringList leftUnits, rightUnits;
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                // First decide new label
                QString newLabel;
                // If alias empty then use data name
                if (mPlotCurves[i]->getLogDataVariablePtr()->getAliasName().isEmpty())
                {

                    newLabel = QString("%1 [%2]").arg(mPlotCurves[i]->getDataName()).arg(mPlotCurves[i]->getCurrentUnit());
                }
                // Else use the alias name
                else
                {
                    newLabel = QString("%1 [%2]").arg(mPlotCurves[i]->getLogDataVariablePtr()->getAliasName()).arg(mPlotCurves[i]->getCurrentUnit());
                }

                // If new label is not already on the axis then we may want to add it
                if( ( (mPlotCurves[i]->getAxisY() == QwtPlot::yLeft)  && !leftUnits.contains(newLabel)) ||
                        ( (mPlotCurves[i]->getAxisY() == QwtPlot::yRight) && !rightUnits.contains(newLabel)) )
                {
                    // If label on axis is empty then set new text label
                    if(mpQwtPlot->axisTitle(mPlotCurves[i]->getAxisY()).isEmpty())
                    {
                        mpQwtPlot->setAxisTitle(mPlotCurves[i]->getAxisY(), QwtText(newLabel));
                    }
                    // else append new text to already existing text
                    else
                    {
                        QString currText = mpQwtPlot->axisTitle(mPlotCurves[i]->getAxisY()).text();
                        mpQwtPlot->setAxisTitle(mPlotCurves[i]->getAxisY(), QwtText(currText.append(", ").append(newLabel)));
                    }

                    // Remember the text we just added
                    if(mPlotCurves[i]->getAxisY() == QwtPlot::yLeft)
                    {
                        leftUnits.append(newLabel);
                    }
                    if(mPlotCurves[i]->getAxisY() == QwtPlot::yRight)
                    {
                        rightUnits.append(newLabel);
                    }
                }
            }
            if (mHasCustomXData)
            {
                // Check all curves to make sure if it is the same custom x on all
                QList<SharedVariablePtrT> customXdatas;
                //! @todo checking this stuff every time is stupid, this should be sorted out upon adding removin curves
                for(int i=0; i<mPlotCurves.size(); ++i)
                {
                    SharedVariablePtrT pX = mPlotCurves[i]->getCustomXData();
                    if (!pX.isNull() && !customXdatas.contains(pX))
                    {
                        customXdatas.append(mPlotCurves[i]->getCustomXData());
                    }
                }

                QString text;
                for (int i=0; i<customXdatas.size(); ++i)
                {
                    text.append(customXdatas[i]->getDataName() + QString(" [%1], ").arg(customXdatas[i]->getPlotScaleDataUnit()));
                }
                text.chop(2);
                mpQwtPlot->setAxisTitle(QwtPlot::xBottom, text);
            }
            else
            {
                // Ok since there is not custom x-data lets assume that all curves have the same x variable (usually time), lets ask the first one
                SharedVariablePtrT pTime = mPlotCurves.first()->getTimeVectorPtr();
                if (pTime)
                {
                    mpQwtPlot->setAxisTitle(QwtPlot::xBottom, pTime->getDataName()+QString(" [%1] ").arg(pTime->getActualPlotDataUnit()));
                }

                // Else no automatic x-label
            }
        }
        else if(mPlotCurves[0]->getCurveType() == FrequencyAnalysisType)
        {
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                mpQwtPlot->setAxisTitle(mPlotCurves[i]->getAxisY(), "Relative Magnitude [-]");
                mpQwtPlot->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
            }
        }
        else if(mPlotCurves[0]->getCurveType() == NyquistType)
        {
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                mpQwtPlot->setAxisTitle(mPlotCurves[i]->getAxisY(), "Im");
                mpQwtPlot->setAxisTitle(QwtPlot::xBottom, "Re");
            }
        }
        else if(mPlotCurves[0]->getCurveType() == BodeGainType)
        {
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                mpQwtPlot->setAxisTitle(mPlotCurves[i]->getAxisY(), "Magnitude [dB]");
                mpQwtPlot->setAxisTitle(QwtPlot::xBottom, QwtText());      //No label, because there will be a phase plot below with same label
            }
        }
        else if(mPlotCurves[0]->getCurveType() == BodePhaseType)
        {
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                mpQwtPlot->setAxisTitle(mPlotCurves[i]->getAxisY(), "Phase [deg]");
                mpQwtPlot->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
            }
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
}

void PlotArea::openLegendSettingsDialog()
{

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
            SharedVariablePtrT pTime = mPlotCurves[i]->getTimeVectorPtr();
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
        mPlotCurves[i]->setIncludeGenerationInTitle(mpIncludeGenInCurveTitle->isChecked());
        mPlotCurves[i]->refreshCurveTitle();
    }

    mpQwtPlot->insertLegend(NULL, QwtPlot::TopLegend);

    rescaleAxesToCurves();
}

void PlotArea::applyTimeScalingSettings()
{
    //    QString newUnit = extractBetweenFromQString(mpTimeScaleComboBox->currentText().split(" ").last(), '[', ']');
    //    QString newScaleStr = mpTimeScaleComboBox->currentText().split(" ")[0];
    //    double newScale = newScaleStr.toDouble();
    //    //! @todo make sure we have at least one curve here, also this is not correct since different curves may have different generation, should be able to ask the zoomer about this instead, or have some refresh zoom slot that handles all of it
    //    double oldScale = mPlotCurvePtrs[0][0]->getTimeVectorPtr()->getPlotScale();

    //    //! @todo this will only affect the generation for the first curve
    //    mPlotCurvePtrs[0][0]->getTimeVectorPtr()->setCustomUnitScale(UnitScale(newUnit, newScaleStr));
    //    mPlotCurvePtrs[0][0]->getTimeVectorPtr()->setPlotOffset(mpTimeOffsetLineEdit->text().toDouble());
    //    //! @todo this will aslo call all the updates again, need to be able to set scale and ofset separately or togheter

    //    //! @todo offset step size should follow scale change to make more sense, (when you click the spinbox), also for ydata scaling

    //    // Update zoom rectangle to new scale if zoomed
    //    if(isZoomed(0))
    //    {
    //        QRectF oldZoomRect = mpZoomerLeft[0]->zoomRect();
    //        QRectF newZoomRect = QRectF(oldZoomRect.x()*newScale/oldScale, oldZoomRect.y(), oldZoomRect.width()*newScale/oldScale, oldZoomRect.height());

    //        resetZoom();

    //        mpZoomerLeft[0]->zoom(newZoomRect);
    //        update();
    //    }

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

//! @brief Help function to set legend symbole style
//! @todo allways sets for all curves, maybe should only set for one
void PlotArea::setLegendSymbol(const QString symStyle)
{
    for(int j=0; j<mPlotCurves.size(); ++j)
    {
        mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendNoAttribute, false);
        mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowLine, false);
        mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowSymbol, false);
        mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowBrush, false);

        if( symStyle == "Rectangle")
        {
            mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendNoAttribute, true);
        }
        else if( symStyle == "Line")
        {
            mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowLine, true);
        }
        else if( symStyle == "Symbol")
        {
            mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowSymbol, true);
         }
        else if ( symStyle == "Line&Symbol")
        {
            mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowLine, true);
            mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowSymbol, true);
        }
        else if( symStyle == "Brush")
        {
            mPlotCurves[j]->setLegendAttribute( PlotCurve::LegendShowBrush, true);
        }

        // Fix legend size after possible change in style
        mPlotCurves[j]->resetLegendSize();
    }
}

void PlotArea::setTabOnlyCustomXVector(SharedVariablePtrT pData)
{
    mHasCustomXData = true;
    mpCustomXData = pData;

    updateAxisLabels();
    update();

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
                gpTerminalWidget->mpConsole->printWarningMessage(QString("Unit %1 is associated to multiple physical quantities, default unit selection may be incorrect").arg(dataUnit));
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

//! @todo rename
void PlotArea::resetXTimeVector()
{
    mHasCustomXData = false;
    mpCustomXData = SharedVariablePtrT(0);

    PlotCurve *pCurve;
    Q_FOREACH(pCurve, mPlotCurves)
    {
        pCurve->setCustomXData(SharedVariablePtrT(0)); //Remove any custom x-data
    }

    rescaleAxesToCurves();
    updateAxisLabels();
    update();
}


CustomXDataDropEdit::CustomXDataDropEdit(QWidget *pParent)
    : QLineEdit(pParent)
{
    //Nothing
}

void CustomXDataDropEdit::dropEvent(QDropEvent *e)
{
    QLineEdit::dropEvent(e);
    QString mimeText = e->mimeData()->text();
    if(mimeText.startsWith("HOPSANPLOTDATA:"))
    {
        QStringList fields = mimeText.split(":");
        if (fields.size() > 2)
        {
            // We do not want to include gen here, as the curve should decide for it self what gen to use
            emit newXData(fields[1]);
        }
    }
    else
    {
        emit newXData(mimeText);
    }
}


//! @brief Constructor for plot info box
//! @param pParentPlotCurve pointer to parent plot curve
//! @param pParent Pointer to parent widget
PlotCurveControlBox::PlotCurveControlBox(PlotCurve *pPlotCurve, PlotArea *pParentArea)
    : QWidget(pParentArea)
{
    mpPlotArea = pParentArea;
    mpPlotCurve = pPlotCurve;
    connect(mpPlotCurve, SIGNAL(curveInfoUpdated()), this, SLOT(updateInfo()));
    connect(mpPlotCurve, SIGNAL(colorChanged(QColor)), this, SLOT(updateColor(QColor)));
    connect(mpPlotCurve, SIGNAL(markedActive(bool)), this, SLOT(markActive(bool)));

    mpColorBlob = new QToolButton(this);
    mpColorBlob->setFixedSize(20,20);
    mpColorBlob->setCheckable(true);
    mpColorBlob->setChecked(false);
    updateColor(mpPlotCurve->getLineColor());

    mpTitle = new QLabel(this);
    mpTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpTitle->setAlignment(Qt::AlignHCenter);
    refreshTitle();

    mpCustomXDataDrop = new CustomXDataDropEdit(this);
    mpCustomXDataDrop->setToolTip("Drag and Drop here to set Custom XData Vector");
    mpResetTimeButton = new QToolButton(this);
    mpResetTimeButton->setToolTip("Reset Time Vector");
    mpResetTimeButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    mpResetTimeButton->setEnabled(false);

    mpGenerationSpinBox = new QSpinBox(this);
    mpGenerationSpinBox->setToolTip("Change generation");

    mpGenerationLabel = new QLabel(this);
    mpGenerationLabel->setToolTip("Available generations");
    QFont tempFont = mpGenerationLabel->font();
    tempFont.setBold(true);
    mpGenerationLabel->setFont(tempFont);

    QCheckBox *pAutoUpdateCheckBox = new QCheckBox("Auto Update");
    pAutoUpdateCheckBox->setChecked(true);

    QToolButton *pColorButton = new QToolButton(this);
    pColorButton->setToolTip("Select Line Color");
    pColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LineColor.png"));

    QToolButton *pFrequencyAnalysisButton = new QToolButton(this);
    pFrequencyAnalysisButton->setToolTip("Frequency Analysis");
    pFrequencyAnalysisButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-FrequencyAnalysis.png"));

    QToolButton *pScaleButton = new QToolButton(this);
    pScaleButton->setToolTip("Scale Curve");
    pScaleButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-PlotCurveScale.png"));

    QLabel *pSizeLabel = new QLabel(tr("Line Width: "));
    pSizeLabel->setAcceptDrops(false);
    QSpinBox *pSizeSpinBox = new QSpinBox(this);
    pSizeSpinBox->setAcceptDrops(false);
    pSizeSpinBox->setRange(1,10);
    pSizeSpinBox->setSingleStep(1);
    pSizeSpinBox->setValue(2);
    pSizeSpinBox->setSuffix(" pt");

    // New Combo Box for Line Style
    QComboBox *pLineStyleCombo = new QComboBox;
    pLineStyleCombo->addItem(tr("Solid Line"));
    pLineStyleCombo->addItem(tr("Dash Line"));
    pLineStyleCombo->addItem(tr("Dot Line"));
    pLineStyleCombo->addItem(tr("Dash Dot Line"));
    pLineStyleCombo->addItem(tr("Dash Dot Dot Line"));
    pLineStyleCombo->addItem(tr("No Curve")); //CustomDashLine

    // New Combo Box for Symbol Style
    QComboBox *pLineSymbol = new QComboBox;
    pLineSymbol->addItem(tr("None"));
    pLineSymbol->addItem(tr("Cross"));
    pLineSymbol->addItem(tr("Ellipse"));
    pLineSymbol->addItem(tr("XCross"));
    pLineSymbol->addItem(tr("Triangle"));
    pLineSymbol->addItem(tr("Rectangle"));
    pLineSymbol->addItem(tr("Diamond"));
    pLineSymbol->addItem(tr("Down Triangle"));
    pLineSymbol->addItem(tr("Up Triangle"));
    pLineSymbol->addItem(tr("Right Triangle"));
    pLineSymbol->addItem(tr("Hexagon"));
    pLineSymbol->addItem(tr("Horizontal Line"));
    pLineSymbol->addItem(tr("Vertical Line"));
    pLineSymbol->addItem(tr("Star 1"));
    pLineSymbol->addItem(tr("Star 2"));
    //mpLineSymbol->addItem(tr("Dots"));


    QToolButton *pCloseButton = new QToolButton(this);
    pCloseButton->setToolTip("Remove Curve");
    pCloseButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));

    QLabel *pDummy = new QLabel("", this);

    QHBoxLayout *pInfoBoxLayout = new QHBoxLayout(this);
    pInfoBoxLayout->addWidget(mpColorBlob);
    pInfoBoxLayout->addWidget(mpTitle);
    pInfoBoxLayout->addWidget(mpCustomXDataDrop);
    pInfoBoxLayout->addWidget(mpResetTimeButton);
    pInfoBoxLayout->addWidget(mpGenerationSpinBox);
    pInfoBoxLayout->addWidget(mpGenerationLabel);
    pInfoBoxLayout->addWidget(pAutoUpdateCheckBox);
    pInfoBoxLayout->addWidget(pFrequencyAnalysisButton);
    pInfoBoxLayout->addWidget(pScaleButton);
    pInfoBoxLayout->addWidget(pSizeSpinBox);
    pInfoBoxLayout->addWidget(pColorButton);
    pInfoBoxLayout->addWidget(pLineStyleCombo);
    pInfoBoxLayout->addWidget(pLineSymbol);
    pInfoBoxLayout->addWidget(pCloseButton);
    pInfoBoxLayout->addWidget(pDummy); // This one must be here to prevent colorblob from having a very small clickable area, (really strange)

    setLayout(pInfoBoxLayout);

    connect(mpColorBlob,               SIGNAL(clicked(bool)),       this,               SLOT(activateCurve(bool)));
    connect(mpCustomXDataDrop,         SIGNAL(newXData(QString)),   this,               SLOT(setXData(QString)));
    connect(mpResetTimeButton,         SIGNAL(clicked()),           this,               SLOT(resetTimeVector()));
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,               SLOT(setGeneration(int)));
    connect(pAutoUpdateCheckBox,       SIGNAL(toggled(bool)),       mpPlotCurve,  SLOT(setAutoUpdate(bool)));
    connect(pFrequencyAnalysisButton,  SIGNAL(clicked(bool)),       mpPlotCurve,  SLOT(performFrequencyAnalysis()));
    connect(pColorButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(setLineColor()));
    connect(pScaleButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(openScaleDialog()));
    connect(pCloseButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(removeMe()));
    connect(pSizeSpinBox,    SIGNAL(valueChanged(int)),             mpPlotCurve,  SLOT(setLineWidth(int)));
    connect(pLineStyleCombo, SIGNAL(currentIndexChanged(QString)),  mpPlotCurve,  SLOT(setLineStyle(QString)));
    connect(pLineSymbol,     SIGNAL(currentIndexChanged(QString)),  mpPlotCurve,  SLOT(setLineSymbol(QString)));

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if(mpPlotCurve->getCurveType() != PortVariableType)
    {
        pAutoUpdateCheckBox->setDisabled(true);
        mpGenerationSpinBox->setDisabled(true);
        pFrequencyAnalysisButton->setDisabled(true);
    }

    setPalette(gpConfig->getPalette());
}

PlotCurve *PlotCurveControlBox::getCurve()
{
    return mpPlotCurve;
}

void PlotCurveControlBox::updateColor(const QColor color)
{
    QString buttonStyle;

    // Update color blob in plot info box
    buttonStyle.append(QString("QToolButton                 { border: 1px solid gray;           border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:unchecked		{ border: 1px solid gray;           border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:checked         { border: 1px solid gray;           border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150); border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover           { border: 2px solid gray;           border-style: outset;   border-radius: 10px;    padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover:unchecked { border: 1px solid gray;           border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover:checked   { border: 2px solid rgb(70,70,150); border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover:pressed   { border: 2px solid rgb(70,70,150); border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));

    //! @todo need to fix the syle so that it is shown which is activated

    mpColorBlob->setStyleSheet(buttonStyle);
}

//! @brief Updates buttons and text in plot info box to correct values
void PlotCurveControlBox::updateInfo()
{
    // Enable/diable generation buttons
    const int lowGen = mpPlotCurve->getLogDataVariablePtr()->getLowestGeneration();
    const int highGen = mpPlotCurve->getLogDataVariablePtr()->getHighestGeneration();
    const int gen = mpPlotCurve->getGeneration();
    const int nGen = mpPlotCurve->getLogDataVariablePtr()->getNumGenerations();
    disconnect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,  SLOT(setGeneration(int))); //Need to temporarily disconnect to avoid loop
    mpGenerationSpinBox->setRange(lowGen+1, highGen+1);
    mpGenerationSpinBox->setValue(gen+1);
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,  SLOT(setGeneration(int)));
    mpGenerationSpinBox->setEnabled(nGen > 1);

    // Set generation number strings
    //! @todo this will show strange when we have deleted old generations, maybe we should reassign all generations when we delete old data (costly)
    mpGenerationLabel->setText(QString("[%1,%2]").arg(lowGen+1).arg(highGen+1));

    // Update curve name
    refreshTitle();

    // Update Xdata
    if (mpPlotCurve->hasCustomXData())
    {
        mpCustomXDataDrop->setText(mpPlotCurve->getCustomXData()->getFullVariableName());
        if (mpPlotCurve->getTimeVectorPtr())
        {
            mpResetTimeButton->setEnabled(true);
        }
    }
    else
    {
        mpCustomXDataDrop->setText("");
        mpResetTimeButton->setEnabled(false);
    }
}

void PlotCurveControlBox::refreshTitle()
{
    mpTitle->setText(mpPlotCurve->getCurveName() + " ["+mpPlotCurve->getCurrentUnit()+"]");
}

void PlotCurveControlBox::markActive(bool active)
{
    //! @todo this is not visible (marking doesnt show)
    mpColorBlob->setChecked(active);
    if (active)
    {
        setAutoFillBackground(true);
        setPalette(gpConfig->getPalette());
    }
    else
    {
        setAutoFillBackground(false);
    }
}

void PlotCurveControlBox::activateCurve(bool active)
{
    if(active)
    {
        mpPlotArea->setActivePlotCurve(mpPlotCurve);
    }
    else
    {
        mpPlotArea->setActivePlotCurve(0);
    }
}

void PlotCurveControlBox::setXData(QString fullName)
{
    mpPlotCurve->setCustomXData(fullName);
}

void PlotCurveControlBox::resetTimeVector()
{
    mpPlotCurve->setCustomXData(0);
}

void PlotCurveControlBox::setGeneration(const int gen)
{
    // Since info box begins counting at 1 we need to subtract one, but we do not want underflow as that would set latest generation (-1)
    if (!mpPlotCurve->setGeneration(qMax(gen-1,0)))
    {
        mpGenerationSpinBox->setPrefix("NA<");
        mpGenerationSpinBox->setSuffix(">");
    }
    else
    {
        mpGenerationSpinBox->setPrefix("");
        mpGenerationSpinBox->setSuffix("");
    }
}
