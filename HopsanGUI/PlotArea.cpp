/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   PlotArea.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2014
//!
//! @brief Contains a class for plot tabs areas
//!
//$Id$

#include <QLineEdit>
#include <QGridLayout>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>

#include "PlotArea.h"
#include "PlotCurve.h"
#include "PlotCurveControlBox.h"
#include "PlotTab.h"
#include "global.h"
#include "Configuration.h"
#include "Utilities/GUIUtilities.h"
#include "ModelHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "PlotWindow.h"
#include "MessageHandler.h"
#include "Widgets/TimeOffsetWidget.h"

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

namespace  {

const double DoubleMax = std::numeric_limits<double>::max();
const double DoubleMin = std::numeric_limits<double>::min();
const double Double1000Min = 1000*DoubleMin;

inline int limitGen(int gen, int min, int max)
{
    return qMin(qMax(gen, min), max);
}

QString displayNameForData(const SharedVectorVariableT& data) {
    if (data->hasCustomLabel()) {
        return data->getCustomLabel();
    }
    else if (data->hasAliasName()) {
        return data->getAliasName();
    }
    else {
        return data->getFullVariableNameWithSeparator(", ");
    }
}

}

//! @brief Rectangle painter widget, used for painting transparent rectangles when dragging things to plot tabs
class RectanglePainterWidget : public QWidget
{
public:
    RectanglePainterWidget(QWidget *parent=nullptr);
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

void HopQwtPlot::resizeEvent(QResizeEvent *e)
{
    QwtPlot::resizeEvent(e);
    emit sizeChanged(width(), height());
}


PlotArea::PlotArea(PlotTab *pParentPlotTab)
    : QWidget(pParentPlotTab)
{
    mpParentPlotTab = pParentPlotTab;
    QGridLayout *pLayout = new QGridLayout(this);

    setAcceptDrops(true);
    setMouseTracking(true);
    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;
    mBottomAxisLogarithmic = false;
    mBottomAxisShowOnlySamples = false;

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
    removeAllCurves();
}



//! @brief Adds a plot curve to a plot tab
//! @param curve Pointer to the plot curve
//! @param desiredColor Desired color for curve (will override default colors)
void PlotArea::addCurve(PlotCurve *pCurve, PlotCurveStyle style)
{
    // Attach the curve to this plot area
    pCurve->attach(mpQwtPlot);
    mPlotCurves.append(pCurve);

    // Set some private members in the curve based on this current plot area
    pCurve->mpParentPlotArea = this;
    pCurve->mIncludeGenInTitle = mpIncludeGenInCurveTitle->isChecked();
    pCurve->mIncludeSourceInTitle = mpIncludeSourceInCurveTitle->isChecked();
    pCurve->setZ(CurveZOrderType);
    pCurve->setLineWidth(style.thickness);
    QPen pen = pCurve->pen();
    pen.setStyle(Qt::PenStyle(style.line_style));
    pCurve->setPen(pen);

    pCurve->setLineSymbol(PlotCurveStyle::toSymbolName(style.symbol));
    pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    setLegendSymbol(mpLegendSymbolType->currentText(), pCurve);

    // Show only samples
    if (mBottomAxisShowOnlySamples != pCurve->getShowVsSamples())
    {
        pCurve->setShowVsSamples(mBottomAxisShowOnlySamples);
    }

    // Refresh the curve
    pCurve->refreshCurveTitle();

    // Set custom xdata if one is already present in this tab
    if(hasCustomXData())
    {
        if (!pCurve->hasCustomXData())
        {
            SharedVectorVariableT xdata = switchVariableGeneration(mCustomXData, pCurve->getDataGeneration());
            pCurve->setCustomXData(xdata);
        }
    }

    // Determine what plot scales to use
    determineCurveTFUnitScale(pCurve);
    determineCurveDataUnitScale(pCurve);
    determineCurveXDataUnitScale(pCurve);

    // Determine plot curve color, if desired color is valid then use that color, else use the color selector the get the least common one
    if (style.color.isValid())
    {
        pCurve->setLineColor(style.color);
        mCurveColorSelector.incrementCurveColorCounter(style.color.name());
    }
    else
    {
        pCurve->setLineColor(mCurveColorSelector.getLeastCommonCurveColor());
    }

    // Enable axis and zoomers as needed
    if (!mpQwtPlot->axisEnabled(pCurve->getAxisY()))
    {
        mpQwtPlot->enableAxis(pCurve->getAxisY());

        // If this is the first curve on one of the axis, then then axis will just be enabled and we need to normalize the zoom (copy from the other curve)
        // unless the axis lock is set on that axis (from a previous plot)
        //! @todo this code is likely not working at all, when new curves are added everything is always auto rescaled (zoom is lost) (unless axis are locked), I believe this code can be removed /Peter
        if ( (pCurve->getAxisY() == QwtPlot::yLeft) && !mpYLLockCheckBox->isChecked() && mpQwtPlot->axisEnabled(QwtPlot::yRight) )
        {
            mpQwtZoomerLeft->setZoomStack(mpQwtZoomerRight->zoomStack());
        }
        else if ( (pCurve->getAxisY() == QwtPlot::yRight) && !mpYRLockCheckBox->isChecked() && mpQwtPlot->axisEnabled(QwtPlot::yLeft) )
        {
            mpQwtZoomerRight->setZoomStack(mpQwtZoomerLeft->zoomStack());
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
    connect(pControlBox, SIGNAL(hideCurve(PlotCurve*)), this, SLOT(hideCurve(PlotCurve*)));
    connect(pControlBox, SIGNAL(showCurve(PlotCurve*)), this, SLOT(showCurve(PlotCurve*)));
    pControlBox->setSizeValue(style.thickness);
    pControlBox->setSymbol(style.symbol);
    pControlBox->setLineType(style.line_style);
    mPlotCurveControlBoxes.append(pControlBox);
    mpParentPlotTab->mpCurveInfoScrollArea->widget()->layout()->addWidget(mPlotCurveControlBoxes.last());

    mPlotCurveControlBoxes.last()->show();


    // Connect some signals from the curve
    connect(pCurve, SIGNAL(curveDataUpdated()), this, SLOT(rescaleAxesToCurves()));
    connect(pCurve, SIGNAL(customXDataChanged(PlotCurve*)), this, SLOT(determineCurveXDataUnitScale(PlotCurve*)));
    connect(pCurve, SIGNAL(customXDataChanged(PlotCurve*)), this, SLOT(refreshPlotAreaCustomXData()));
    connect(pCurve, SIGNAL(curveInfoUpdated()), this, SLOT(updateAxisLabels()));
    connect(pCurve, SIGNAL(dataRemoved(PlotCurve*)), this, SLOT(removeCurve(PlotCurve*)));

    // Connect signals from the curve data logdatahandler (to trigger update whenever new data becomes available)
    LogDataHandler2 *pLDH = pCurve->getSharedVectorVariable()->getLogDataHandler();
    if (pLDH)
    {
        // Unique connection will prevent multiple connections to same log data handler
        // Note! We connect here but we never disconnect, but that is OK, most of the time only data from same handler will be present
        // if that is not the case, well then update will be triggered more often, but who cares (not me)
        connect(pLDH, SIGNAL(dataAdded()), this, SLOT(updateCurvesToNewGenerations()), Qt::UniqueConnection);
    }

    //Unlock and reset zoom if adding first curve to an empty area
    if(getNumberOfCurves() == 1) {
        mpXLockCheckBox->setChecked(false);
        mpYLLockCheckBox->setChecked(false);
        mpYRLockCheckBox->setChecked(false);
        resetZoom();
    }

    // Refresh and redraw the plot area
    //! @todo maybe make it possible to rescale only selected axis, instead of always rescaling both of them
    rescaleAxesToCurves();
    updateAxisLabels();
    updateWindowtitleModelName();
    replot();

    //Update multi plot markers
    for(int i=0; i<mMultiPlotMarkers.size(); ++i)
    {
        mMultiPlotMarkers[i]->addMarker(pCurve);
    }
}

void PlotArea::setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc, bool force)
{
    SharedVariableDescriptionT pVarDesc(new VariableDescription(rVarDesc));
    setCustomXVectorForAll(createFreeVectorVariable(xArray, pVarDesc), force);
}

void PlotArea::setCustomXVectorForAll(SharedVectorVariableT data, bool force)
{
    for(int i=0; i<mPlotCurves.size(); ++i)
    {
        if (force || !mPlotCurves[i]->hasCustomXData())
        {
            mPlotCurves[i]->setCustomXData(data);
            determineCurveXDataUnitScale(mPlotCurves[i]);
        }
    }
    rescaleAxesToCurves();
}

void PlotArea::removeCurve(PlotCurve *pCurve)
{
    // Remove any markers used by the curve
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


    //Update multi plot markers
    for(int i=0; i<mMultiPlotMarkers.size(); ++i)
    {
        mMultiPlotMarkers[i]->removeMarker(pCurve);
    }

    pCurve->detach();
    mPlotCurves.removeAll(pCurve);
    pCurve->mpParentPlotArea = nullptr;
    pCurve->disconnect();
    delete pCurve;

    // Reset time vector in case we had special x-axis set previously
    refreshPlotAreaCustomXData();
    rescaleAxesToCurves();
    updateAxisLabels();
    updateWindowtitleModelName();
    replot();
}

void PlotArea::insertMultiMarker(QPoint pos)
{
    mMultiPlotMarkers.append(new MultiPlotMarker(pos, this));
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

void PlotArea::removeAllCurves()
{
    // Remove all curves
    while(!mPlotCurves.empty())
    {
        removeCurve(mPlotCurves.last());
    }
}

void PlotArea::hideCurve(PlotCurve *pCurve)
{
    // Hide any markers used by the curve
    //! @todo maybe support hiding markers belonging to curve, for now discard them
    for(int i=0; i<mPlotMarkers.size(); ++i)
    {
        if(mPlotMarkers[i]->getCurve() == pCurve)
        {
            removePlotMarker(mPlotMarkers[i]);
            --i;
        }
    }

//    // Remove the plot curve info box used by the curve
//    for(int i=0; i<mPlotCurveControlBoxes.size(); ++i)
//    {
//        if(mPlotCurveControlBoxes[i]->getCurve() == pCurve)
//        {
//            mPlotCurveControlBoxes[i]->hide();
//            mPlotCurveControlBoxes[i]->deleteLater();
//            mPlotCurveControlBoxes.removeAt(i);
//            break;
//        }
//    }

    // Reduce the curve color counter for this curve color
//    mCurveColorSelector.decrementCurveColorCounter(pCurve->pen().color());

    if (pCurve->getAxisY() == QwtPlot::yLeft)
    {
        --mNumYlCurves;
    }
    else if (pCurve->getAxisY() == QwtPlot::yRight)
    {
        --mNumYrCurves;
    }


    // Update multi plot markers
    for(int i=0; i<mMultiPlotMarkers.size(); ++i)
    {
        mMultiPlotMarkers[i]->removeMarker(pCurve);
    }

    pCurve->detach();
    mHiddenPlotCurves.append(pCurve);
    mPlotCurves.removeAll(pCurve);
    //pCurve->mpParentPlotArea = 0;
    //pCurve->disconnect();
    //delete pCurve;

    // Reset time vector in case we had special x-axis set previously
    refreshPlotAreaCustomXData();
    rescaleAxesToCurves();
    updateAxisLabels();
    updateWindowtitleModelName();
    replot();
}

void PlotArea::showCurve(PlotCurve *pCurve)
{
    int idx=-1;
    for (int i=0; i<mHiddenPlotCurves.size(); ++i)
    {
        if (mHiddenPlotCurves[i] == pCurve)
        {
            idx = i;
        }
    }
    if (idx < 0)
    {
        return;
    }

    pCurve->attach(mpQwtPlot);
    mPlotCurves.append(pCurve);

    if (pCurve->getAxisY() == QwtPlot::yLeft)
    {
        ++mNumYlCurves;
    }
    else if (pCurve->getAxisY() == QwtPlot::yRight)
    {
        ++mNumYrCurves;
    }

    mHiddenPlotCurves.removeAt(idx);

    rescaleAxesToCurves();
    updateAxisLabels();
    updateWindowtitleModelName();
    replot();
}

void PlotArea::removePlotMarker(PlotMarker *pMarker)
{
    if (pMarker)
    {
        for(int i=0; i<mMultiPlotMarkers.size(); ++i)
        {
            MultiPlotMarker *mm = mMultiPlotMarkers[i];
            if(mm->mPlotMarkerPtrs.contains(pMarker))
            {
                while(!mm->mPlotMarkerPtrs.isEmpty())
                {
                    pMarker = mm->mPlotMarkerPtrs.first();
                    mpQwtPlot->canvas()->removeEventFilter(pMarker);
                    pMarker->hide();
                    pMarker->detach();
                    pMarker->deleteLater();
                    mm->mPlotMarkerPtrs.removeAll(pMarker);
                    mPlotMarkers.removeAll(pMarker);
                }

                mm->mpDummyMarker->hide();
                mm->mpDummyMarker->detach();
                delete(mm->mpDummyMarker);

                mMultiPlotMarkers.removeAll(mm);
                mm->deleteLater();
                return;
            }
        }
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
    // Mark deactivate all others
    //! @todo if only one can be active it should be enough to deactivate that one
    for(int i=0; i<mPlotCurves.size(); ++i)
    {
        if(mPlotCurves[i] != pCurve)
        {
            mPlotCurves[i]->markActive(false);
        }
    }
    // Mark active the one
    if (pCurve!=nullptr)
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

HopQwtPlot *PlotArea::getQwtPlot()
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
    return !mCustomXData.isNull();
}

const SharedVectorVariableT PlotArea::getCustomXData() const
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

bool PlotArea::isAxisLogarithmic(const QwtPlot::Axis axis) const
{
    switch (axis) {
    case QwtPlot::xBottom:
        return isBottomAxisLogarithmic();
    case QwtPlot::yLeft:
        return isLeftAxisLogarithmic();
    case QwtPlot::yRight:
        return isRightAxisLogarithmic();
    default:
        return false;
    }
}

void PlotArea::setAxisLimits(QwtPlot::Axis axis, const double min, const double max, const double step, bool lockAxis)
{
    mpQwtPlot->setAxisScale(axis, min, max, step);

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

void PlotArea::setAxisLocked(QwtPlot::Axis axis, bool lockAxis)
{
    switch (axis)
    {
    case QwtPlot::xBottom:
        mpXLockCheckBox->setChecked(lockAxis);
        break;
    case QwtPlot::yLeft:
        mpYLLockCheckBox->setChecked(lockAxis);
        break;
    case QwtPlot::yRight:
        mpYRLockCheckBox->setChecked(lockAxis);
        break;
    default:
        break;
        //Nothing for the other axis
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

//! @brief Get the min and max values of the curves on the current axis, (linear or logarithmic values)
HopQwtInterval PlotArea::getAxisCurveLimits(const QwtPlot::Axis axis) const
{
    HopQwtInterval curveLimits(DoubleMax,-DoubleMax);
    bool axisLog = isAxisLogarithmic(axis);

    for (int i=0; i<mPlotCurves.size(); ++i)
    {
        if(mPlotCurves[i]->getAxisY() == axis)
        {
            if(axisLog)
            {
                // Only consider positive (non-zero) values if logarithmic scaling is used
                double min, max;
                if (mPlotCurves[i]->minMaxPositiveNonZeroYValues(min, max))
                {
                    curveLimits.extendMin(min);
                    curveLimits.extendMax(max);
                }
            }
            else
            {
                curveLimits.extendMin(mPlotCurves[i]->minYValue());
                curveLimits.extendMax(mPlotCurves[i]->maxYValue());
            }
        }
    }

    return curveLimits;
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
    // Unless we are zoomed, try to readjust the axis (to add auto offset for legend)
    if (!isZoomed())
    {
        rescaleAxesToCurves();
    }
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
                    SystemObject *pContainer = gpModelHandler->getTopLevelSystem(model);
                    // If we failed to find by modelpath (like for imported vars), then try the current view
                    if (!pContainer)
                    {
                        pContainer = gpModelHandler->getCurrentViewContainerObject();
                    }
                    if (pContainer)
                    {
                        SharedVectorVariableT data = pContainer->getLogDataHandler()->getVectorVariable(name, gen);
                        // If we have found data then add it to the plot
                        if (data)
                        {
                            QCursor cursor;
                            if(this->mapFromGlobal(cursor.pos()).y() > getQwtPlot()->canvas()->height()*2.0/3.0+getQwtPlot()->canvas()->y()+10 && getNumberOfCurves() >= 1)
                            {
                                setAxisLocked(QwtPlot::xBottom, false);
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
    QMenu *pResetUnitsMenu;
    QMenu *pInsertMarkerMenu;

    QAction *pSetRightAxisLogarithmic = nullptr;
    QAction *pSetLeftAxisLogarithmic = nullptr;
    QAction *pSetBottomAxisLogarithmic = nullptr;
    QAction *pSetBottomAxisShowSamples = nullptr;
    QAction *pSetUserDefinedAxisLabels = nullptr;

    pYAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    pYAxisRightMenu = menu.addMenu(QString("Right Y Axis"));
    pBottomAxisMenu = menu.addMenu(QString("Bottom Axis"));

    pYAxisLeftMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::yLeft));
    pYAxisRightMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::yRight));
    pBottomAxisMenu->setEnabled(mpQwtPlot->axisEnabled(QwtPlot::xBottom));

    // Create menu and actions for changing units
    pChangeUnitsMenu = menu.addMenu(QString("Change Units"));
    pResetUnitsMenu = menu.addMenu(QString("Reset Units"));
    QMap<QAction *, PlotCurve *> dataActionToCurveMap;
    QMap<QAction *, QVector<PlotCurve *>> timeorfreqActionToCurveMap;
    QMap<QAction *, QVector<PlotCurve *>> customxActionToCurveMap;
    QList<PlotCurve *>::iterator itc;
    QList<UnitConverter>::iterator itu;

    QMap<VectorVariable*, QVector<PlotCurve*>> unique_time_or_frequency_variables_curve_map;
    QMap<VectorVariable*, QVector<PlotCurve*>> unique_customx_variables_curve_map;
    for(itc=mPlotCurves.begin(); itc!=mPlotCurves.end(); ++itc)
    {
        PlotCurve *pCurve = *itc;
        QList<UnitConverter> unitScales;

        QAction *pTempResetUnitAction = pResetUnitsMenu->addAction(pCurve->getCurveName());
        dataActionToCurveMap.insert(pTempResetUnitAction, pCurve);

        QMenu *pTempChangeUnitMenu = pChangeUnitsMenu->addMenu(pCurve->getCurveName());
        if (pCurve->getDataQuantity().isEmpty())
        {
            QStringList pqs = gpConfig->getQuantitiesForUnit(pCurve->getDataUnit());
            if (pqs.size() == 1)
            {
                gpConfig->getUnitScales(pqs.first(), unitScales);
            }
        }
        else
        {
            gpConfig->getUnitScales(pCurve->getDataQuantity(), unitScales);
        }

        for(itu=unitScales.begin(); itu!=unitScales.end(); ++itu)
        {
            QAction *pTempAction = pTempChangeUnitMenu->addAction((*itu).mUnit);
            dataActionToCurveMap.insert(pTempAction, pCurve);
        }

        auto tf = pCurve->getSharedTimeOrFrequencyVariable();
        if (tf) {
            unique_time_or_frequency_variables_curve_map[tf.data()].append(pCurve);
        }
        auto cx = pCurve->getSharedCustomXVariable();
        if (cx) {
            unique_customx_variables_curve_map[cx.data()].append(pCurve);
        }
    }

    // Create change unit enteries for time or frequency data
    if (!unique_time_or_frequency_variables_curve_map.isEmpty()) {
        pResetUnitsMenu->addSeparator();
        pChangeUnitsMenu->addSeparator();
    }
    for (const auto& curveList : unique_time_or_frequency_variables_curve_map) {

        auto pFirstCurve = curveList.first();

        QList<UnitConverter> unitScales;
        QString dataName = pFirstCurve->getSharedTimeOrFrequencyVariable()->getDataName();
        QString dataQuantity = pFirstCurve->getSharedTimeOrFrequencyVariable()->getDataQuantity();

        QAction *pTempResetUnitAction = pResetUnitsMenu->addAction(dataName);
        for (const auto& pCurve : curveList) {
            timeorfreqActionToCurveMap[pTempResetUnitAction].append(pCurve);
        }

        QMenu *pTempChangeUnitMenu = pChangeUnitsMenu->addMenu(dataName);
        if (dataQuantity.isEmpty())
        {
            QStringList pqs = gpConfig->getQuantitiesForUnit(pFirstCurve->getSharedTimeOrFrequencyVariable()->getDataUnit());
            if (pqs.size() == 1)
            {
                gpConfig->getUnitScales(pqs.first(), unitScales);
            }
        }
        else
        {
            gpConfig->getUnitScales(dataQuantity, unitScales);
        }

        for(itu=unitScales.begin(); itu!=unitScales.end(); ++itu)
        {
            QAction *pTempAction = pTempChangeUnitMenu->addAction((*itu).mUnit);
            for (const auto& pCurve : curveList) {
                timeorfreqActionToCurveMap[pTempAction].append(pCurve);
            }
        }
    }

    // Create change unit enteries for custom X-data
    if (!unique_customx_variables_curve_map.isEmpty()) {
        pResetUnitsMenu->addSeparator();
        pChangeUnitsMenu->addSeparator();
    }
    for (const auto& curveList : unique_customx_variables_curve_map) {

        auto pFirstCurve = curveList.first();

        QList<UnitConverter> unitScales;
        QString dataName = displayNameForData(pFirstCurve->getSharedCustomXVariable());
        QString dataQuantity = pFirstCurve->getSharedCustomXVariable()->getDataQuantity();

        QAction *pTempResetUnitAction = pResetUnitsMenu->addAction(dataName);
        for (const auto& pCurve : curveList) {
            customxActionToCurveMap[pTempResetUnitAction].append(pCurve);
        }

        QMenu *pTempChangeUnitMenu = pChangeUnitsMenu->addMenu(dataName);
        if (dataQuantity.isEmpty())
        {
            QStringList pqs = gpConfig->getQuantitiesForUnit(pFirstCurve->getSharedCustomXVariable()->getDataUnit());
            if (pqs.size() == 1)
            {
                gpConfig->getUnitScales(pqs.first(), unitScales);
            }
        }
        else
        {
            gpConfig->getUnitScales(dataQuantity, unitScales);
        }

        for(itu=unitScales.begin(); itu!=unitScales.end(); ++itu)
        {
            QAction *pTempAction = pTempChangeUnitMenu->addAction((*itu).mUnit);
            for (const auto& pCurve : curveList) {
                customxActionToCurveMap[pTempAction].append(pCurve);
            }
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

        pSetBottomAxisShowSamples = pBottomAxisMenu->addAction("Show Samples");
        pSetBottomAxisShowSamples->setCheckable(true);
        pSetBottomAxisShowSamples->setChecked(mBottomAxisShowOnlySamples);
    }

    QAction *pInsertMultiMarkerAction = menu.addAction("Insert multi-marker");

    // Create menu for inserting curve markers
    pInsertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    for(itc=mPlotCurves.begin(); itc!=mPlotCurves.end(); ++itc)
    {
        QAction *pTempAction = pInsertMarkerMenu->addAction((*itc)->getCurveName());
        dataActionToCurveMap.insert(pTempAction, (*itc));
    }


    // Create option for changing axis labels
    pSetUserDefinedAxisLabels = menu.addAction("Set userdefined axis labels");

    // ----- Wait for user to make a selection ----- //
    QAction *pSelectedAction = menu.exec(QCursor::pos());

    // ----- User has selected something -----  //

    // Check if user did not click on a menu item
    if(pSelectedAction == nullptr)
    {
        return;
    }


    // Change unit on selected curve
    if(pSelectedAction->parentWidget()->parentWidget() == pChangeUnitsMenu)
    {
        auto it = dataActionToCurveMap.find(pSelectedAction);
        if (it != dataActionToCurveMap.end()) {
            if(it.value()->getCurrentPlotUnit() != pSelectedAction->text()) {
                setAxisLocked((QwtPlot::Axis)it.value()->getAxisY(), false); //Reset lock axis setting, but only if unit is different from existing unit
            }
            it.value()->setCurveDataUnitScale(pSelectedAction->text());
        }

        auto lit = timeorfreqActionToCurveMap.find(pSelectedAction);
        if (lit != timeorfreqActionToCurveMap.end()) {
            for (auto pCurve : *lit) {
                if(pCurve->getCurrentTFPlotUnit() != pSelectedAction->text()) {
                    setAxisLocked(QwtPlot::xBottom, false); //Reset lock axis setting, but only if unit is different from existing unit
                }
                pCurve->setCurveTFUnitScale(pSelectedAction->text());
            }
        }

        auto cxlit = customxActionToCurveMap.find(pSelectedAction);
        if (cxlit != customxActionToCurveMap.end()) {
            for (auto pCurve : *cxlit) {
                if(pCurve->getCurrentXPlotUnit() != pSelectedAction->text()) {
                    setAxisLocked(QwtPlot::xBottom, false); //Reset lock axis setting, but only if unit is different from existing unit
                }
                pCurve->setCurveCustomXDataUnitScale(pSelectedAction->text());
            }
        }
    }

    // Reset unit on selected curve
    if(pSelectedAction->parentWidget() == pResetUnitsMenu)
    {
        auto it = dataActionToCurveMap.find(pSelectedAction);
        if (it != dataActionToCurveMap.end()) {
            it.value()->resetCurveDataUnitScale();
            it.value()->setCurveExtraDataScaleAndOffset(1,0);
        }

        auto lit = timeorfreqActionToCurveMap.find(pSelectedAction);
        if (lit != timeorfreqActionToCurveMap.end()) {
            for (auto pCurve : *lit) {
                pCurve->resetCurveTFUnitScale();
            }
        }

        auto cxlit = customxActionToCurveMap.find(pSelectedAction);
        if (cxlit != customxActionToCurveMap.end()) {
            for (auto pCurve : *cxlit) {
                pCurve->resetCurveCustomXDataUnitScale();
            }
        }
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
    else if (pSelectedAction == pSetBottomAxisShowSamples)
    {
        mBottomAxisShowOnlySamples = !mBottomAxisShowOnlySamples;

        for(PlotCurve *pCurve : mPlotCurves)
        {
            pCurve->setShowVsSamples(mBottomAxisShowOnlySamples);
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
        insertMarker(dataActionToCurveMap.find(pSelectedAction).value(), event->pos());
    }


    // Insert multi-curve marker
    if(pSelectedAction == pInsertMultiMarkerAction)
    {
        insertMultiMarker(event->pos());
    }
}

void PlotArea::rescaleAxesToCurves()
{
    // Set defaults when no axis available
    HopQwtInterval xAxisLim(0.0, 1.0), ylAxisLim(-1.0, 1.0), yrAxisLim(-1.0, 1.0);

    // Cycle plots, ignore if no curves
    if(!mPlotCurves.empty())
    {
        // Init left/right min max
        if (mNumYlCurves > 0)
        {
            ylAxisLim = getAxisCurveLimits(QwtPlot::yLeft);
        }
        if (mNumYrCurves > 0)
        {
            yrAxisLim = getAxisCurveLimits(QwtPlot::yRight);
        }

        // Initialize values for X axis
        xAxisLim.setInterval(DoubleMax, -DoubleMax);
        for(int i=0; i<mPlotCurves.size(); ++i)
        {
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
    }

    // Fix incorrect or bad limit values
    if(!ylAxisLim.isValid())
    {
        ylAxisLim.setInterval(-1.0, 1.0);
    }
    if(!yrAxisLim.isValid())
    {
        yrAxisLim.setInterval(-1.0, 1.0);
    }

    const double sameLimFrac = 0.1;
    bool leftSameLimitEnlargeApplied=false, rightSameLimitEnlargeApplied=false;
    // Max and min must not be same value; if they are, decrease/increase
    if ( (ylAxisLim.width()) < Double1000Min)
    {
        ylAxisLim.extendMax(ylAxisLim.maxValue()+qMax(qAbs(ylAxisLim.maxValue()) * sameLimFrac, Double1000Min));
        ylAxisLim.extendMin(ylAxisLim.minValue()-qMax(qAbs(ylAxisLim.minValue()) * sameLimFrac, Double1000Min));
        leftSameLimitEnlargeApplied=true;
    }

    if ( (yrAxisLim.width()) < Double1000Min)
    {
        yrAxisLim.extendMax(yrAxisLim.maxValue()+qMax(qAbs(yrAxisLim.maxValue()) * sameLimFrac, Double1000Min));
        yrAxisLim.extendMin(yrAxisLim.minValue()-qMax(qAbs(yrAxisLim.minValue()) * sameLimFrac, Double1000Min));
        rightSameLimitEnlargeApplied=true;
    }

    if ( (xAxisLim.width()) < Double1000Min)
    {
        xAxisLim.extendMax(xAxisLim.maxValue()+qMax(qAbs(xAxisLim.maxValue()) * sameLimFrac, Double1000Min));
        xAxisLim.extendMin(xAxisLim.minValue()-qMax(qAbs(xAxisLim.minValue()) * sameLimFrac, Double1000Min));
    }

    // If plot has log scale, we use a different approach for calculating margins
    // (fixed margins would not make sense with a log scale)

    // Now enlarge the Left and Right axis limits to get some margin around values but only if the sameLimit enlargement has not already been applied
    const double linearEnlargeFrac=0.05; // 5%
    if (!leftSameLimitEnlargeApplied)
    {
        if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlot->axisScaleEngine(QwtPlot::yLeft)))
        {
            ylAxisLim.setInterval(ylAxisLim.minValue()/2.0, ylAxisLim.maxValue()*2.0);
        }
        else
        {
            // For linear scale expand by linearEnlargeFrac
            ylAxisLim.setInterval(ylAxisLim.minValue()-linearEnlargeFrac*ylAxisLim.width(), ylAxisLim.maxValue()+linearEnlargeFrac*ylAxisLim.width());
        }
    }

    if (!rightSameLimitEnlargeApplied)
    {
        if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlot->axisScaleEngine(QwtPlot::yRight)))
        {
            yrAxisLim.setInterval(yrAxisLim.minValue()/2.0, yrAxisLim.maxValue()*2.0);
        }
        else
        {
            // For linear scale expand by linearEnlargeFrac
            yrAxisLim.setInterval(yrAxisLim.minValue()-linearEnlargeFrac*yrAxisLim.width(), yrAxisLim.maxValue()+linearEnlargeFrac*yrAxisLim.width());
        }
    }

    // Scale the axes automatically if not locked
    if (!mpXLockCheckBox->isChecked())
    {
        mpQwtPlot->setAxisScale(QwtPlot::xBottom, xAxisLim.minValue(), xAxisLim.maxValue());
    }

    setSmartYAxisLimits(QwtPlot::yLeft, ylAxisLim);
    setSmartYAxisLimits(QwtPlot::yRight, yrAxisLim);

    refreshLockCheckBoxPositions();

    // Now call the actual refresh of the axes
    mpQwtPlot->updateAxes();
}

void PlotArea::refreshPlotAreaCustomXData()
{
    bool someoneHasCustomXdata = false;
    // loop in reverse to activate last added as current
    for(int i=int(mPlotCurves.size())-1; i>=0; --i)
    {
        // First check if some curve has a custom x-axis and plot does not
        if (mPlotCurves[i]->hasCustomXData())
        {
            // Set plot-area global custom x-data
            mCustomXData = mPlotCurves[i]->getSharedCustomXVariable();
            someoneHasCustomXdata = true;
            break;
        }
    }

    // If no one has custom data, then reset it
    if (!someoneHasCustomXdata)
    {
        mCustomXData = SharedVectorVariableT();
    }
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
        for(PlotCurve *pPlotCurve : mPlotCurves)
        {
            // First decide new y-axis label
            // Use quantity if it exists else go with the data name
            QString newLabel = pPlotCurve->getDataQuantity();
            if (newLabel.isEmpty())
            {
                newLabel = pPlotCurve->getDataName();
            }

            // Add unit if it exists
            QString yUnit = pPlotCurve->getCurrentPlotUnit();
            if (!yUnit.isEmpty())
            {
                newLabel.append(QString(" [%1]").arg(yUnit));
            }

            // If new label is not already on the axis then we may want to add it
            // Check left axis
            if( (pPlotCurve->getAxisY() == QwtPlot::yLeft) && !leftLabels.contains(newLabel) )
            {
                leftLabels.append(newLabel);
            }

            // Check right axis
            if( (pPlotCurve->getAxisY() == QwtPlot::yRight) && !rightLabels.contains(newLabel) )
            {
                rightLabels.append(newLabel);
            }

            // Now decide new bottom axis label
            // Use custom x-axis if available, else try to use the time or frequency vector (if set), but also check for showVSsamples
            QString bottomLabel;

            if (pPlotCurve->getShowVsSamples())
            {
                bottomLabel = "Samples";
            }
            else
            {
                bool customX=true;
                SharedVectorVariableT pSharedXVector = pPlotCurve->getSharedCustomXVariable();
                // If not custom data exist, try time or frequency vector
                if (pSharedXVector.isNull())
                {
                    customX = false;
                    pSharedXVector = pPlotCurve->getSharedTimeOrFrequencyVariable();
                }

                if (pSharedXVector.isNull())
                {
                    bottomLabel = "Samples";
                }
                else if (!sharedBottomVars.contains(pSharedXVector))
                {
                    sharedBottomVars.append(pSharedXVector); // This one is used for faster comparison (often the curves share the same x-vector)
                    // Use alias if it exist else data quantity and if that is empty go with data name
                    if (pSharedXVector->hasAliasName())
                    {
                        bottomLabel = pSharedXVector->getAliasName();
                    }
                    else
                    {
                        bottomLabel = pSharedXVector->getDataQuantity();
                        if (bottomLabel.isEmpty())
                        {
                            bottomLabel = pSharedXVector->getDataName();
                        }
                    }

                    if(customX)
                    {
                        QString xUS = pPlotCurve->getCurrentXPlotUnit();
                        if (!xUS.isEmpty())
                        {
                            bottomLabel.append(QString(" [%1]").arg(xUS));
                        }
                    }
                    // handle time or frequency vector
                    else
                    {
                        UnitConverter tfUS = pPlotCurve->getCurveTFUnitScale();
                        if (!tfUS.isEmpty())
                        {
                            bottomLabel.append(QString(" [%1]").arg(tfUS.mUnit));
                        }
                    }
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

    // If User custom labels exist overwrite automatic label
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

void PlotArea::openTimeOffsetDialog()
{
    QDialog offsetDialog(this);
    offsetDialog.setWindowTitle("Change Time Offset");

    // Maps for all generations and log data handlers in plot
    // Normally if all variables come from the same model, its the same log data handler
    //! @todo But if you mix variables from different models the "last" at a specific generation will overwrite here
    QMap<int, LogDataHandler2*> generations;

    // Go through every curve and collect each generation
    for (PlotCurve* pCurve : mPlotCurves)
    {
        generations.insert(pCurve->getDataGeneration(), pCurve->getSharedVectorVariable()->getLogDataHandler());
    }

    QGridLayout *pGridLayout = new QGridLayout(&offsetDialog);
    pGridLayout->addWidget(new QLabel("Changing generation time offset will affect all curves in all plots",&offsetDialog), 0, 0, 1, 2, Qt::AlignLeft);
    int row = 1;
    for (int gen : generations.uniqueKeys())
    {
        // Now create an editor widget for each unique generation
        TimeOffsetWidget *pTimeScaleW = new TimeOffsetWidget(gen, generations.value(gen), &offsetDialog);
        connect(pTimeScaleW, SIGNAL(valuesChanged()), this, SLOT(updateAxisLabels()));
        pGridLayout->addWidget(new QLabel(QString("Gen: %1").arg(gen+1), &offsetDialog), row, 0);
        pGridLayout->addWidget(pTimeScaleW, row, 1);
        ++row;
    }

    // Add button box
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal,&offsetDialog);
    pButtonBox->addButton(QDialogButtonBox::Ok);
    pGridLayout->addWidget(pButtonBox, row, 1);
    connect(pButtonBox,SIGNAL(accepted()),&offsetDialog,SLOT(close()));
    connect(pButtonBox,SIGNAL(accepted()),this,SLOT(updateAxisLabels()));

    offsetDialog.exec();
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
    // Show/change internal legends
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

    mpQwtPlot->insertLegend(nullptr, QwtPlot::TopLegend);

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

void PlotArea::shiftModelGenerationsDown()
{
    // Working on copy since mPlotCurves will be modified during loop
    QList<PlotCurve*> curves = mPlotCurves;
    QList<PlotCurve*> curvesHidden;
    for (PlotCurve *pCurve : curves)
    {
        bool rc = pCurve->autoDecrementModelSourceGeneration();
        if (!rc)
        {
            hideCurve(pCurve);
            curvesHidden.append(pCurve);
        }
    }

    // Working on copy since mHiddenPlotCurves will be modified during loop
    curves = mHiddenPlotCurves;
    for (PlotCurve *pCurve : curves)
    {
        // Prevent decrementing curves we did just hide again
        if (curvesHidden.contains(pCurve))
        {
            continue;
        }
        // Attempt to show previously hidden curves
        if (pCurve->isAutoUpdating())
        {
            bool rc = pCurve->autoDecrementModelSourceGeneration();
            if (rc)
            {
                showCurve(pCurve);
            }
        }
    }
}

void PlotArea::shiftModelGenerationsUp()
{
    // Working on copy since mPlotCurves will be modified during loop
    QList<PlotCurve*> curves = mPlotCurves;
    QList<PlotCurve*> curvesHidden;
    for (PlotCurve *pCurve : curves)
    {
        bool rc = pCurve->autoIncrementModelSourceGeneration();
        if (!rc)
        {
            hideCurve(pCurve);
            curvesHidden.append(pCurve);
        }
    }

    // Working on copy since mHiddenPlotCurves will be modified during loop
    curves = mHiddenPlotCurves;
    for (PlotCurve *pCurve : curves)
    {
        // Prevent decrementing curves we did just hide again
        if (curvesHidden.contains(pCurve))
        {
            continue;
        }
        // Attempt to show previously hidden curves
        bool rc = pCurve->autoIncrementModelSourceGeneration();
        if (rc)
        {
            showCurve(pCurve);
        }
    }
}

void PlotArea::updateCurvesToNewGenerations()
{
    for (PlotCurve *pCurve : mPlotCurves)
    {
        pCurve->updateToNewGeneration();
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

    connect(pCurve, SIGNAL(curveDataUpdated()), pMarker, SLOT(updatePosition()));
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

    // We do not need to refresh left y axis since lock box will be in 0,0 always, but we add space
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
    // Spinbox will be very wide on Windows (to allow dblmax as value) so we ignore its width size hint and let the combo boxes decide the column width
    mpLegendLeftOffset->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mpLegendLeftOffset->setDecimals(2);
    mpLegendLeftOffset->setSingleStep(0.1);
    mpLegendLeftOffset->setValue(0);
    mpLegendLeftOffset->setDisabled(mpLegendsAutoOffsetCheckBox->isChecked());

    mpLegendRightOffset = new QDoubleSpinBox(this);
    mpLegendRightOffset->setRange(-DoubleMax, DoubleMax);
    mpLegendRightOffset->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
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
    legendBoxLayout->addWidget( new QLabel( "Legends on/off: " ), row, 0, 1, 1, Qt::AlignRight );
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

    r=3;c=3;
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

//! @brief Help function to set legend symbols style for all curves
void PlotArea::setLegendSymbol(const QString symStyle)
{
    for(int j=0; j<mPlotCurves.size(); ++j)
    {
        setLegendSymbol(symStyle, mPlotCurves[j]);
    }
}

//! @brief Help function to set legend symbols style for one particular curve
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

void PlotArea::determineCurveDataUnitScale(PlotCurve *pCurve)
{
    const QString originalDataUnit = pCurve->getDataUnit();
    QString dataQuantity = pCurve->getDataQuantity();

    // Figure out the desired default unit (set in configuration) for the data behind this curve
    QString desiredDefaultUnit;
    if ( dataQuantity.isEmpty() )
    {
        // Figure out compatible default unit based on the original data unit
        QStringList pqs = gpConfig->getQuantitiesForUnit(originalDataUnit);
        if (pqs.size() > 1)
        {
            gpMessageHandler->addWarningMessage(QString("Unit %1 is associated to multiple physical quantities, default unit selection may be incorrect").arg(originalDataUnit));
        }
        if (pqs.size() == 1)
        {
            dataQuantity = pqs.first();
            desiredDefaultUnit = gpConfig->getDefaultUnit(dataQuantity);
        }
    }
    else
    {
        desiredDefaultUnit = gpConfig->getDefaultUnit(dataQuantity);
    }

    // Use the default unit if it is not the same as the original unit
    if (!desiredDefaultUnit.isEmpty() && (desiredDefaultUnit != originalDataUnit) )
    {
        UnitConverter us;
        gpConfig->getUnitScale(dataQuantity, desiredDefaultUnit, us);
        pCurve->setCurveDataUnitScale(us);
    }

    // If all curves on the same axis has the same custom unit, assign this unit to the new curve as well
    // But only if there is an original unit to begin with otherwise we should scale something with unknown original unit (bad)
    if (!originalDataUnit.isEmpty())
    {
        QString customUnit;
        for(int i=0; i<mPlotCurves.size(); ++i)
        {
            // Skip checking the curve we are adding, and only check for curves on the same axis as we are adding to
            if ( (mPlotCurves[i] != pCurve) && (mPlotCurves[i]->getAxisY() == pCurve->getAxisY()) )
            {
                if( customUnit.isEmpty() )
                {
                    // Assign custom unit on first occurrence
                    customUnit = mPlotCurves[i]->getCurrentPlotUnit();
                }
                else if(customUnit != mPlotCurves[i]->getCurrentPlotUnit())
                {
                    // Unit is different between the other curves, so we do not want to use it
                    customUnit = QString();
                    break;
                }
            }
        }
        // If we have found a custom unit that is shared among the other curves, then set that custom scale
        // but only if it is different from the current unit, (we do not want a custom curve scale 1)
        if( !customUnit.isEmpty()  && (customUnit != pCurve->getCurrentPlotUnit()) )
        {
            pCurve->setCurveDataUnitScale(customUnit);
        }
    }
}

void PlotArea::determineCurveXDataUnitScale(PlotCurve *pCurve)
{
    if (pCurve->hasCustomXData())
    {
        const QString originalXDataUnit = pCurve->getSharedCustomXVariable()->getDataUnit();
        QString xDataQuantity = pCurve->getSharedCustomXVariable()->getDataQuantity();

        // Figure out the desired default unit (set in configuration) for the data behind this curve
        QString desiredDefaultUnit;
        // If quantity is not set, try to lookup based on original unit
        if ( xDataQuantity.isEmpty() ) {
            // Figure out compatible default unit based on the original data unit
            QStringList pqs = gpConfig->getQuantitiesForUnit(originalXDataUnit);
            if (pqs.size() > 1)
            {
                gpMessageHandler->addWarningMessage(QString("Unit %1 is associated to multiple physical quantities, default unit selection may be incorrect").arg(originalXDataUnit));
            }
            if (pqs.size() == 1)
            {
                xDataQuantity = pqs.first();
                desiredDefaultUnit = gpConfig->getDefaultUnit(xDataQuantity);
            }
        }
        else {
            desiredDefaultUnit = gpConfig->getDefaultUnit(xDataQuantity);
        }

        // Use the default unit if it is not the same as the original unit
        if (!desiredDefaultUnit.isEmpty() && (desiredDefaultUnit != originalXDataUnit) ) {
            pCurve->setCurveCustomXDataUnitScale(gpConfig->getUnitScaleUC(xDataQuantity, desiredDefaultUnit));
        }

        // If all curves on the same axis has the same custom unit, assign this unit to the new curve as well
        // But only if there is an original unit to begin with otherwise we would scale something with unknown original unit (bad)
        if (!originalXDataUnit.isEmpty())
        {
            QMap<QString, QStringList> currentQuantityCustomUnits;
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                // Skip checking the curve we are adding
                if ( mPlotCurves[i] != pCurve) {

                    auto uc = mPlotCurves[i]->getCurveCustomXDataUnitScale();
                    if (uc.mQuantity.isEmpty()) {
                        uc.mQuantity = xDataQuantity.isEmpty() ? "NoQuantity" : xDataQuantity;
                    }
                    if (!uc.isEmpty() && !uc.mUnit.isEmpty() ) {
                        QStringList& rUnitsForQuantity = currentQuantityCustomUnits[uc.mQuantity];
                        if (!rUnitsForQuantity.contains(uc.mUnit)) {
                            rUnitsForQuantity.append(uc.mUnit);
                        }
                    }
                }
            }

            // In case multiple quantities are plotted, lookup unit for the correct one
            auto it = currentQuantityCustomUnits.find(pCurve->getSharedCustomXVariable()->getDataQuantity());
            if (it != currentQuantityCustomUnits.end() && !it->isEmpty()) {
                // Hopefully the same unit is used for all data with this quantity (size 1)
                // but in case not, use the unit from the latest added curve
                QString customUnit = it->last();

                // Only set if it is different from the current unit, (we do not want a custom curve scale 1)
                if( !customUnit.isEmpty() && (customUnit != pCurve->getCurrentXPlotUnit()) ) {
                    pCurve->setCurveCustomXDataUnitScale(customUnit);
                }
            }
        }
    }
}

void PlotArea::determineCurveTFUnitScale(PlotCurve *pCurve)
{
    SharedVectorVariableT tfVar = pCurve->getSharedTimeOrFrequencyVariable();
    if (tfVar)
    {
        QString defaultTfUnit = gpConfig->getDefaultUnit(tfVar->getDataName());

        // Use the default unit if it is not the same as the original unit
        if (!defaultTfUnit.isEmpty() && (defaultTfUnit != tfVar->getDataUnit()) )
        {
            UnitConverter us;
            gpConfig->getUnitScale(tfVar->getDataName(), defaultTfUnit, us);
            pCurve->setCurveTFUnitScale(us);
        }

        // If all curves on the same axis has the same custom unit, assign this unit to the new curve as well
        // But only if there is an original unit to begin with otherwise we should scale something with unknown original unit (bad)
        if (!tfVar->getDataUnit().isEmpty())
        {
            QString customUnit;
            for(int i=0; i<mPlotCurves.size(); ++i)
            {
                // Skip checking the curve we are adding, and only check for curves on the same axis as we are adding to
                if ( (mPlotCurves[i] != pCurve) && (mPlotCurves[i]->getAxisY() == pCurve->getAxisY()) )
                {
                    if( customUnit.isEmpty() )
                    {
                        // Assign custom unit on first occurrence
                        customUnit = mPlotCurves[i]->getCurveTFUnitScale().mUnit;
                    }
                    else if(customUnit != mPlotCurves[i]->getCurveTFUnitScale().mUnit)
                    {
                        // Unit is different between the other curves, so we do not want to use it
                        customUnit = QString();
                        break;
                    }
                }
            }
            // If we have found a custom unit that is shared among the other curves, then set that custom scale
            if( !customUnit.isEmpty() )
            {
                UnitConverter us;
                gpConfig->getUnitScale(tfVar->getDataName(), customUnit, us);
                pCurve->setCurveTFUnitScale(us);
            }
        }
    }
}

//! @brief This help function will append bottom or top space to make room for legend, space will be appended to input axis limits
//! @param[in] axisId The axis to append to
//! @param[in] rAxisLimits The initial axis limits to append to
void PlotArea::setSmartYAxisLimits(const QwtPlot::Axis axisId, QwtInterval axisLimits)
{
    // check abort conditions
    if ( (axisId == QwtPlot::yLeft) && (mNumYlCurves == 0 || mpYLLockCheckBox->isChecked()) )
    {
        return;
    }
    else if ( (axisId == QwtPlot::yRight) && (mNumYrCurves == 0 || mpYRLockCheckBox->isChecked()) )
    {
        return;
    }

    //! @todo only works for top buffer right now
    if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlot->axisScaleEngine(axisId)))
    {
        //! @todo what should happen here ?
        mpQwtPlot->setAxisAutoScale(axisId, true);
    }
    else
    {
        // Curves range
        const double cr = axisLimits.width();

        // Find largest legend height in pixels
        double lht, lhb;
        calculateLegendBufferOffsets(axisId, lhb, lht);

        // Axis height
        const double ah = mpQwtPlot->axisWidget(axisId)->size().height();

        // Remove legend and margin height from axis height, what remains is the height for the curves
        // Divide with the curves value range to get the scale
        double s = (ah-(lht+lhb))/cr; //[px/unit]

        // Don't try to change axis limits if legend is higher then the axis that will look strange and risk crashing Hopsan when axis limit -> inf
        if (s > 0)
        {
            //qDebug() << "s: " << s;
            s = qMax(s,1e-100); // Limit to prevent div by 0

            // Calculate new axis range for current axis height given the scale
            const double ar = ah/s;

            axisLimits.setMaxValue(axisLimits.minValue() + ar - lhb/s);
            axisLimits.setMinValue(axisLimits.minValue() - lhb/s);
        }
    }

    //! @todo before setting we should check so that min max is reasonable else hopsan will crash (example: Inf)
    // Set the new axis value
    mpQwtPlot->setAxisScale(axisId, axisLimits.minValue(), axisLimits.maxValue());
    // Create the zoom base (original zoom) rectangle for the left or right axis
    QRectF baseZoomRect;
    baseZoomRect.setX(mpQwtPlot->axisInterval(QwtPlot::xBottom).minValue());
    baseZoomRect.setWidth(mpQwtPlot->axisInterval(QwtPlot::xBottom).width());
    baseZoomRect.setY(axisLimits.minValue());
    baseZoomRect.setHeight(axisLimits.width());
    if (axisId == QwtPlot::yLeft)
    {
        mpQwtZoomerLeft->setZoomBase(baseZoomRect);
    }
    else if (axisId == QwtPlot::yRight)
    {
        mpQwtZoomerRight->setZoomBase(baseZoomRect);
    }
    //! @todo left only applies to left even if the right is overshadowed, problem is that if left, right are bottom and top calculated buffers will be different on each axis, this is a todo problem with legend buffer offset
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
    //! @todo even if a legend is empty it seems to be visible and the borderDistance will be added, this causes unnecessary space when on top or bottom (and the other legend is not)

    // Figure out vertical alignment, by bitwise masking
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
        //!< @todo label text will be wrong if plot scale or offset change
    }
}

void PlotArea::updateWindowtitleModelName()
{
    //! @todo instead of string, maybe should use shared pointers (in the data variables to avoid duplicating the string for variables from same model) /Peter
    mModelPaths.clear();
    for(PlotCurve *pCurve : mPlotCurves) {
        const QString &name = pCurve->getSharedVectorVariable()->getModelPath();
        if (!mModelPaths.contains(name) && !name.isEmpty()) {
            mModelPaths.append(name);
        }
    }
    emit refreshContainsDataFromModels();
}

void PlotArea::getLowestHighestGeneration(int &rLowest, int &rHighest)
{
    // We assume here that all curves come from the same log data handler
    SharedVectorVariableT pData;
    if (mPlotCurves.empty())
    {
        if (mHiddenPlotCurves.empty())
        {
            // nothing
        }
        else
        {
            pData = mHiddenPlotCurves.front()->getSharedVectorVariable();
        }
    }
    else
    {
        pData = mPlotCurves.front()->getSharedVectorVariable();
    }

    if (pData)
    {
        LogDataHandler2 *pLDH = pData->getLogDataHandler();
        if (pLDH)
        {
            rLowest = pLDH->getLowestGenerationNumber();
            rHighest = pLDH->getHighestGenerationNumber();
            return;
        }
    }

    // We get here on failure
    rLowest = -1;
    rHighest = -1;
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

void PlotArea::resetXDataVector()
{
    // Remove any custom x-data
    for(PlotCurve *pCurve : mPlotCurves)
    {
        if (pCurve->hasCustomXData())
        {
            pCurve->setCustomXData(SharedVectorVariableT());
        }
    }
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
