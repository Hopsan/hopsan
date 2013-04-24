#include "PlotTab.h"

#include "PlotWindow.h"
#include "Configuration.h"
#include "PlotCurve.h"

#include "GUIObjects/GUIContainerObject.h"

#include "Utilities/GUIUtilities.h"
#include <limits>

#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
//#include "qwt_legend_item.h"
#include "qwt_legend.h"
//#include "q_layout.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include <qwt_dyngrid_layout.h>

#include "MainWindow.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/HcomWidget.h"

#include "version_gui.h"

#include <cmath>

const double DoubleMax = std::numeric_limits<double>::max();
const double DoubleEps = std::numeric_limits<double>::epsilon();
const double in2mm = 25.4;

//! @brief Constructor for plot tabs.
//! @param parent Pointer to the plot window the tab belongs to
PlotTab::PlotTab(PlotTabWidget *pParentPlotTabWidget, PlotWindow *pParentPlotWindow)
    : QWidget(pParentPlotTabWidget)
{
    mpParentPlotTabWidget = pParentPlotTabWidget;
    mpParentPlotWindow = pParentPlotWindow;
    setAcceptDrops(true);
    setMouseTracking(true);
    mHasCustomXData=false;
    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;
    mBottomAxisLogarithmic = false;
    mIsSpecialPlot = false;

    mCurveColors << "Blue" << "Red" << "Green" << "Orange" << "Pink" << "Brown" << "Purple" << "Gray";

    for(int plotID=0; plotID<2; ++plotID)
    {
        //Plots
        mpQwtPlots[plotID] = new HopQwtPlot(this);
        mpQwtPlots[plotID]->setMouseTracking(true);
        mpQwtPlots[plotID]->setAcceptDrops(false);
        mpQwtPlots[plotID]->setCanvasBackground(QColor(Qt::white));
        mpQwtPlots[plotID]->setAutoReplot(true);
        connect(mpQwtPlots[plotID], SIGNAL(resized()), this, SLOT(rescaleAxesToCurves()));

        //Panning Tool
        mpPanner[plotID] = new QwtPlotPanner(mpQwtPlots[plotID]->canvas());
        mpPanner[plotID]->setMouseButton(Qt::LeftButton);
        mpPanner[plotID]->setEnabled(false);

        //Rubber Band Zoom
        QPen rubberBandPen(Qt::green);
        rubberBandPen.setWidth(2);

        mpZoomerLeft[plotID] = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpQwtPlots[plotID]->canvas());      //Zoomer for left y axis
        mpZoomerLeft[plotID]->setMaxStackDepth(10000);
        mpZoomerLeft[plotID]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerLeft[plotID]->setRubberBandPen(rubberBandPen);
        mpZoomerLeft[plotID]->setTrackerMode(QwtPicker::AlwaysOff);
        mpZoomerLeft[plotID]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        mpZoomerLeft[plotID]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        mpZoomerLeft[plotID]->setZoomBase(QRectF());
        mpZoomerLeft[plotID]->setEnabled(false);

        mpZoomerRight[plotID] = new QwtPlotZoomer( QwtPlot::xTop, QwtPlot::yRight, mpQwtPlots[plotID]->canvas());   //Zoomer for right y axis
        mpZoomerRight[plotID]->setMaxStackDepth(10000);
        mpZoomerRight[plotID]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[plotID]->setRubberBandPen(rubberBandPen);
        mpZoomerRight[plotID]->setTrackerMode(QwtPicker::AlwaysOff);
        mpZoomerRight[plotID]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        mpZoomerRight[plotID]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        mpZoomerRight[plotID]->setEnabled(false);

        //Wheel Zoom
        mpMagnifier[plotID] = new QwtPlotMagnifier(mpQwtPlots[plotID]->canvas());
        mpMagnifier[plotID]->setAxisEnabled(QwtPlot::yLeft, true);
        mpMagnifier[plotID]->setAxisEnabled(QwtPlot::yRight, true);
        mpMagnifier[plotID]->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
        mpMagnifier[plotID]->setWheelFactor(1.1);
        mpMagnifier[plotID]->setMouseButton(Qt::NoButton, Qt::NoModifier);
        mpMagnifier[plotID]->setEnabled(true);

        mpGrid[plotID] = new QwtPlotGrid;
        mpGrid[plotID]->enableXMin(true);
        mpGrid[plotID]->enableYMin(true);
        mpGrid[plotID]->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
        mpGrid[plotID]->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
        mpGrid[plotID]->attach(mpQwtPlots[plotID]);
    }

    mpBarPlot = new QSint::BarChartPlotter(this);

    // Legend Stuff
    constructLegendSettingsDialog();

    mpExternalLegend = 0; //No external legend by default

    mpRightPlotLegend = new PlotLegend(QwtPlot::yRight);
    mpRightPlotLegend->attach(this->getPlot());
    mpRightPlotLegend->setAlignment(Qt::AlignRight);

    mpLeftPlotLegend = new PlotLegend(QwtPlot::yLeft);
    mpLeftPlotLegend->attach(this->getPlot());
    mpLeftPlotLegend->setAlignment(Qt::AlignLeft);


    // Create the lock axis dialog
    constructAxisSettingsDialog();

    mpTabLayout = new QGridLayout(this);

    // Create the scroll area and widget/layout for curve info boxes
    QVBoxLayout *pCurveInfoLayout = new QVBoxLayout();
    pCurveInfoLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pCurveInfoLayout->setSpacing(1);
    pCurveInfoLayout->setMargin(1);
    QWidget *pCurveInfoWidget = new QWidget(this);
    pCurveInfoWidget->setAutoFillBackground(true);
    pCurveInfoWidget->setPalette(gConfig.getPalette());
    pCurveInfoWidget->setLayout(pCurveInfoLayout);
    mpCurveInfoScrollArea = new QScrollArea(this);
    mpCurveInfoScrollArea->setWidget(pCurveInfoWidget);
    mpCurveInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpCurveInfoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpCurveInfoScrollArea->setPalette(gConfig.getPalette());
    mpCurveInfoScrollArea->setMinimumHeight(110);
    mpParentPlotWindow->mpCurveInfoStack->addWidget(mpCurveInfoScrollArea);


    for(int plotID=0; plotID<2; ++plotID)
    {
        mpQwtPlots[plotID]->setAutoFillBackground(true);
        mpQwtPlots[plotID]->setPalette(gConfig.getPalette());
        mpTabLayout->addWidget(mpQwtPlots[plotID]);
    }

    mpPainterWidget = new PainterWidget(this);
    mpPainterWidget->clearRect();
    mpTabLayout->addWidget(mpPainterWidget, 0, 0);

    connect(mpQwtPlots[FirstPlot], SIGNAL(legendClicked(QwtPlotItem*)), this, SLOT(updateLegend(QwtPlotItem *)));//QwtPlotItem *, bool)));

    for(int plotID=1; plotID<2; ++plotID)       //Hide all plots except first one by default
    {
        showPlot(HopsanPlotIDEnumT(plotID), false);
    }
    mpBarPlot->setVisible(false);

    mpQwtPlots[FirstPlot]->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}


//! @brief Destructor for plot tab. Removes all curves before tab is deleted.
PlotTab::~PlotTab()
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        while(!mPlotCurvePtrs[plotID].empty())
        {
            removeCurve(mPlotCurvePtrs[plotID].last());
        }
    }
}

void PlotTab::applyLegendSettings()
{
    // Show/change internal legneds
    if(mpLegendsInternalEnabledCheckBox->isChecked())
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

    // Handle external legend
    if (mpLegendsExternalEnabledCheckBox->isChecked())
    {
        if (mpExternalLegend == 0)
        {
            mpExternalLegend = new QwtLegend();
            mpExternalLegend->setAutoFillBackground(false);
            mpExternalLegend->setFrameStyle(QFrame::NoFrame | QFrame::Sunken);
            mpExternalLegend->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            mpQwtPlots[FirstPlot]->insertLegend(mpExternalLegend, QwtPlot::TopLegend);
        }

        QFont font = mpExternalLegend->font();
        font.setPointSize(mpLegendFontSize->value());
        mpExternalLegend->setFont(font);

        QString symStyle = mpLegendSymbolType->currentText();
        setLegendSymbol(symStyle);
    }
    else
    {
        mpQwtPlots[FirstPlot]->insertLegend(NULL, QwtPlot::TopLegend);
        // Since it is deleted set ptr to NULL
        mpExternalLegend = 0;
    }

    // Decide if we should hide or show manual legend offset controls
    if (mpLegendsAutoOffsetCheckBox->isChecked())
    {
        mpLegendLeftOffset->setDisabled(true);
        mpLegendRightOffset->setDisabled(true);
    }
    else
    {
        mpLegendLeftOffset->setDisabled(false);
        mpLegendRightOffset->setDisabled(false);
    }

    rescaleAxesToCurves();
}


//! @todo currently only supports settings axis for top plot
void PlotTab::openAxisSettingsDialog()
{
    mpXminSpinBox->setValue(mXAxisLimits[FirstPlot].min);
    mpXmaxSpinBox->setValue(mXAxisLimits[FirstPlot].max);

    mpYLminSpinBox->setValue(mYLAxisLimits[FirstPlot].min);
    mpYLmaxSpinBox->setValue(mYLAxisLimits[FirstPlot].max);

    mpYRminSpinBox->setValue(mYRAxisLimits[FirstPlot].min);
    mpYRmaxSpinBox->setValue(mYRAxisLimits[FirstPlot].max);

    mpSetAxisDialog->exec();
}

//! @todo currently only supports settings axis for top plot
void PlotTab::applyAxisSettings()
{
    //If a box is not checked then use manual settings AND remember the value since we do not know how to ask for it later
    if(!mpXAutoCheckBox->isChecked())
    {
        this->getPlot(FirstPlot)->setAxisScale(QwtPlot::xBottom, mpXminSpinBox->value(),mpXmaxSpinBox->value());
        mXAxisLimits[FirstPlot].min = mpXminSpinBox->value();
        mXAxisLimits[FirstPlot].max = mpXmaxSpinBox->value();
    }

    if(!mpYLAutoCheckBox->isChecked())
    {
        this->getPlot(FirstPlot)->setAxisScale(QwtPlot::yLeft, mpYLminSpinBox->value(),mpYLmaxSpinBox->value());
        mYLAxisLimits[FirstPlot].min = mpYLminSpinBox->value();
        mYLAxisLimits[FirstPlot].max = mpYLmaxSpinBox->value();
    }

    if(!mpYRAutoCheckBox->isChecked())
    {
        this->getPlot(FirstPlot)->setAxisScale(QwtPlot::yRight, mpYRminSpinBox->value(),mpYRmaxSpinBox->value());
        mYRAxisLimits[FirstPlot].min = mpYRminSpinBox->value();
        mYRAxisLimits[FirstPlot].max = mpYRmaxSpinBox->value();
    }

    // If anyone of the boxes are checked we call rescale in case we just unchecked it as it needs to auto refresh
    if (mpXAutoCheckBox->isChecked() || mpYLAutoCheckBox->isChecked() || mpYRAutoCheckBox->isChecked())
    {
        this->rescaleAxesToCurves();
    }
}


void PlotTab::openLegendSettingsDialog()
{
    mpLegendSettingsDialog->exec();
}


void PlotTab::setTabName(QString name)
{
    mpParentPlotTabWidget->setTabText(mpParentPlotTabWidget->indexOf(this), name);
}


void PlotTab::addBarChart(QStandardItemModel *pItemModel)
{
    mIsSpecialPlot = true;

    for(int i=0; i<2; ++i)
    {
        mpQwtPlots[i]->setVisible(false);
    }
    mpBarPlot->setVisible(true);

    double min=0;
    double max=0;
    for(int c=0; c<pItemModel->columnCount(); ++c)
    {
        double componentMin = 0;
        double componentMax = 0;
        for(int r=0; r<pItemModel->rowCount(); ++r)
        {
            double data = pItemModel->data(pItemModel->index(r, c)).toDouble();
            if(data > 0)
            {
                componentMax += data;
            }
            if(data < 0)
            {
                componentMin += data;
            }
        }

        min=std::min(min, componentMin);
        max=std::max(max, componentMax);
    }

    mpBarPlot->axisY()->setRanges(min, max);

    mpBarPlot->axisY()->setTicks(max/50, max/10);                     //Minor & major
    mpBarPlot->axisY()->setPen(QPen(Qt::darkGray));
    mpBarPlot->axisY()->setMinorTicksPen(QPen(Qt::gray));
    mpBarPlot->axisY()->setMajorTicksPen(QPen(Qt::darkGray));
    //mpBarPlot->axisY()->setMinorGridPen(QPen(Qt::gray));
    mpBarPlot->axisY()->setMajorGridPen(QPen(Qt::lightGray));
    mpBarPlot->axisY()->setTextColor(Qt::black);
    mpBarPlot->axisY()->setOffset(int(log10(max)+1)*10);
    //qDebug() << "Max = " << max << ", offset = " << mpBarPlot->axisY()->offset();

    mpBarPlot->axisX()->setPen(QPen(Qt::darkGray));
    mpBarPlot->axisX()->setMinorTicksPen(QPen(Qt::gray));
    mpBarPlot->axisX()->setMajorTicksPen(QPen(Qt::darkGray));
    mpBarPlot->axisX()->setMajorGridPen(QPen(Qt::lightGray));
    mpBarPlot->axisX()->setTextColor(Qt::black);
    mpBarPlot->axisX()->setFont(QFont("Calibri", 9));

    mpBarPlot->setBarSize(32, 128);
    mpBarPlot->setBarOpacity(0.9);

    QLinearGradient bg(0,0,0,1);
    bg.setCoordinateMode(QGradient::ObjectBoundingMode);
    bg.setColorAt(1, QColor(0xccccff));
    bg.setColorAt(0.0, Qt::white);
    mpBarPlot->setBackground(QBrush(bg));
    //mpBarPlot->setBackground(QColor("White"));

    mpBarPlot->setModel(pItemModel);

    mpTabLayout->addWidget(mpBarPlot);
}


//! @brief Adds a plot curve to a plot tab
//! @param curve Pointer to the plot curve
//! @param desiredColor Desired color for curve (will override default colors)
void PlotTab::addCurve(PlotCurve *pCurve, QColor desiredColor, HopsanPlotIDEnumT plotID)
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

    // Use the default unit for this curve, unless it is a "Value" with an actual unit set
    QString defaultUnit = gConfig.getDefaultUnit(pCurve->getDataName());
    if ( pCurve->getDataName() != "Value" && (defaultUnit != pCurve->getDataUnit()) )
    {
        pCurve->setCustomDataUnit(defaultUnit);
    }
    else if (pCurve->getDataUnit() == "-")
    {
        pCurve->setCustomDataUnit(defaultUnit);
    }

    // If all curves on the same axis has the same custom unit, assign this unit to the new curve as well
    QString customUnit;
    for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
    {
        if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == pCurve->getAxisY())
        {
            if(customUnit.isEmpty())
            {
                customUnit = mPlotCurvePtrs[plotID].at(i)->getDataUnit();
            }
            else if(customUnit != mPlotCurvePtrs[plotID].at(i)->getDataUnit())  //Unit is different between the other curves, so don't use it
            {
                customUnit = QString();
                break;
            }
        }
    }
    if(!customUnit.isEmpty())
    {
        pCurve->setCustomDataUnit(customUnit);
    }


    mPlotCurvePtrs[plotID].append(pCurve);
    connect(pCurve, SIGNAL(curveDataUpdated()), this, SLOT(rescaleAxesToCurves()));

    if(desiredColor == QColor())
    {
        int i=0;
        while(mUsedColors.contains(mCurveColors.first()))
        {
            mCurveColors.append(mCurveColors.first());
            mCurveColors.pop_front();
            ++i;
            if(i>mCurveColors.size()) break;
        }
        mUsedColors.append(mCurveColors.first());
        pCurve->setLineColor(mCurveColors.first());
    }
    else
    {
        pCurve->setLineColor(desiredColor);
    }

    mpQwtPlots[plotID]->enableAxis(pCurve->getAxisY());
    rescaleAxesToCurves();
    updateLabels();
    pCurve->setZ(1000);
    pCurve->setLineWidth(2);

    setLegendSymbol(mpLegendSymbolType->currentText());

    mpQwtPlots[plotID]->replot();
    mpQwtPlots[plotID]->updateGeometry();

    mpParentPlotWindow->mpBodePlotButton->setEnabled(mPlotCurvePtrs[FirstPlot].size() > 1);
}


//! @brief Rescales the axes and the zommers so that all plot curves will fit
void PlotTab::rescaleAxesToCurves()
{
    //! @todo get freeze in here if x or y data vectors are empty (that has to be fixed)
    //Cycle plots and rescale each of them
    for(int plotID=0; plotID<2; ++plotID)
    {
        // Ignore if zoomed
        if (!isZoomed(plotID))
        {
            AxisLimitsT xAxisLim, ylAxisLim, yrAxisLim;

            // Cycle plots, ignore if no curves
            if(!mPlotCurvePtrs[plotID].empty())
            {
                xAxisLim.min=0;           //Min value for X axis
                xAxisLim.max=10;          //Max value for X axis
                ylAxisLim.min=DoubleMax;     //Min value for left Y axis
                ylAxisLim.max=-DoubleMax;    //Max value for left Y axis
                yrAxisLim.min=DoubleMax;     //Min value for right Y axis
                yrAxisLim.max=-DoubleMax;    //Max value for right Y axis

                // Initialize values for X axis by using the first curve
                xAxisLim.min=mPlotCurvePtrs[plotID].first()->minXValue();
                xAxisLim.max=mPlotCurvePtrs[plotID].first()->maxXValue();

                bool someoneHasCustomXdata = false;
                for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
                {
                    // First check if some curve hav a custom x-axis and plot does not
                    someoneHasCustomXdata = someoneHasCustomXdata || mPlotCurvePtrs[plotID].at(i)->hasCustomXData();
                    if (!mHasCustomXData && someoneHasCustomXdata)
                    {
                        //! @todo maybe should do this with signal slot instead, to avoid unesisarry checks all the time
                        setTabOnlyCustomXVector(mPlotCurvePtrs[plotID].at(i)->getCustomXData());
                    }

                    if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yLeft)
                    {
                        if(mLeftAxisLogarithmic)
                        {
                            //Only consider positive values if logarithmic scaling (negative ones will be discarded by Qwt)
                            double minVal = qMax(mPlotCurvePtrs[plotID].at(i)->minYValue(), DoubleEps);
                            ylAxisLim.min = qMin(ylAxisLim.min, minVal);
                        }
                        else
                        {
                            ylAxisLim.min = qMin( ylAxisLim.min, mPlotCurvePtrs[plotID].at(i)->minYValue());
                        }
                        ylAxisLim.max = qMax(ylAxisLim.max, mPlotCurvePtrs[plotID].at(i)->maxYValue());
                    }

                    if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yRight)
                    {
                        if(mRightAxisLogarithmic)
                        {
                            //Only consider positive values if logarithmic scaling (negative ones will be discarded by Qwt)
                            double minVal = qMax(mPlotCurvePtrs[plotID].at(i)->minYValue(), DoubleEps);
                            yrAxisLim.min = qMin(yrAxisLim.min, minVal);
                        }
                        else
                        {
                            yrAxisLim.min = qMin(yrAxisLim.min, mPlotCurvePtrs[plotID].at(i)->minYValue());
                        }
                        yrAxisLim.max = qMax(yrAxisLim.max, mPlotCurvePtrs[plotID].at(i)->maxYValue());
                    }

                    // find min / max x-value
                    xAxisLim.min = qMin(xAxisLim.min, mPlotCurvePtrs[plotID].at(i)->minXValue());
                    xAxisLim.max = qMax(xAxisLim.max, mPlotCurvePtrs[plotID].at(i)->maxXValue());
                }

                if (mHasCustomXData && !someoneHasCustomXdata)
                {
                    this->resetXTimeVector();
                }
            }
            else    //No curves
            {
                xAxisLim.min=0;         //Min value for X axis
                xAxisLim.max=10;        //Max value for X axis
                ylAxisLim.min=0;     //Min value for left Y axis
                ylAxisLim.max=10;    //Max value for left Y axis
                yrAxisLim.min=0;    //Min value for right Y axis
                yrAxisLim.max=10;   //Max value for right Y axis
            }

            // Fix incorrect or bad limit values
            if(ylAxisLim.min > ylAxisLim.max)
            {
                ylAxisLim.min = 0;
                ylAxisLim.max = 10;
            }
            if(yrAxisLim.min > yrAxisLim.max)
            {
                yrAxisLim.min = 0;
                yrAxisLim.max = 10;
            }

            // Max and min must not be same value; if they are, decrease/increase
            if(ylAxisLim.max == 0 && ylAxisLim.min == 0)
            {
                ylAxisLim.max = 1;
                ylAxisLim.min = -1;
            }
            else if(ylAxisLim.max == ylAxisLim.min && ylAxisLim.max > 0)
            {
                ylAxisLim.max = ylAxisLim.max*2;
                ylAxisLim.min = 0;
            }
            else if(ylAxisLim.max == ylAxisLim.min && ylAxisLim.max < 0)
            {
                ylAxisLim.max = 0;
                ylAxisLim.min = ylAxisLim.min*2;
            }
            if(yrAxisLim.max == 0 && yrAxisLim.min == 0)
            {
                yrAxisLim.max = 1;
                yrAxisLim.min = -1;
            }
            else if(yrAxisLim.max == yrAxisLim.min && yrAxisLim.max > 0)
            {
                yrAxisLim.max = yrAxisLim.max*2;
                yrAxisLim.min = 0;
            }
            else if(yrAxisLim.max == yrAxisLim.min && yrAxisLim.max < 0)
            {
                yrAxisLim.max = 0;
                yrAxisLim.min = yrAxisLim.min*2;
            }

            // Calculate the axis ranges (used for calculating margins at top and bottom
            double leftAxisRange = ylAxisLim.max-ylAxisLim.min;
            double rightAxisRange = yrAxisLim.max-yrAxisLim.min;

            //If plot has log scale, we use a different approach for calculating margins
            //(fixed margins would not make sense with a log scale)

            //! @todo In new qwt the type in the transform has been removed, Trying with dynamic cast instead
            if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlots[plotID]->axisScaleEngine(QwtPlot::yLeft)))
            {
                leftAxisRange = 0;
                ylAxisLim.max = ylAxisLim.max*2.0;
                ylAxisLim.min = ylAxisLim.min/2.0;
            }
            if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlots[plotID]->axisScaleEngine(QwtPlot::yRight)))
            {
                rightAxisRange = 0;
                yrAxisLim.max = yrAxisLim.max*2.0;
                yrAxisLim.min = yrAxisLim.min/2.0;
            }


            // Auto calculate legend hight buffer offsets (Space needed to avoid overshadowing the plots)
            //! @todo only works for linear scale right now, need to check for log scale also
            AxisLimitsT leftLegendBufferOffset, rightLegendBufferOffset; // min = bottom offset, max = top offset
            calculateLegendBufferOffsets(plotID, leftLegendBufferOffset, rightLegendBufferOffset);

            // Scale the axes autoamtically if that option is set
            if (mpXAutoCheckBox->isChecked())
            {
                mXAxisLimits[plotID].min = xAxisLim.min;
                mXAxisLimits[plotID].max = xAxisLim.max;
                mpQwtPlots[plotID]->setAxisScale(QwtPlot::xBottom, mXAxisLimits[plotID].min, mXAxisLimits[plotID].max);
                mpQwtPlots[plotID]->setAxisAutoScale(QwtPlot::xBottom,true);
            }
            if (mpYLAutoCheckBox->isChecked())
            {
                mYLAxisLimits[plotID].min = ylAxisLim.min-0.05*leftAxisRange;
                mYLAxisLimits[plotID].max =  ylAxisLim.max+0.05*leftAxisRange;
                rescaleAxistoIncludeLegendBufferOffset(plotID, QwtPlot::yLeft, leftLegendBufferOffset, mYLAxisLimits[plotID]);
            }
            if (mpYRAutoCheckBox->isChecked())
            {
                mYRAxisLimits[plotID].min = yrAxisLim.min-0.05*rightAxisRange;
                mYRAxisLimits[plotID].max = yrAxisLim.max+0.05*rightAxisRange;
                rescaleAxistoIncludeLegendBufferOffset(plotID, QwtPlot::yRight, rightLegendBufferOffset, mYRAxisLimits[plotID]);
            }
            //! @todo left only applies to left even if the right is overshadowed, problem is that if left, right are bottom and top calculated buffers will be different on each axis

            // Create the zoom base (origianl zoom) rectangle
            leftAxisRange = mYLAxisLimits[plotID].max - mYLAxisLimits[plotID].min;
            rightAxisRange = mYRAxisLimits[plotID].max - mYRAxisLimits[plotID].min;
            double xAxisRange = mXAxisLimits[plotID].max - mXAxisLimits[plotID].min;
            QRectF tempDoubleRect;
            tempDoubleRect.setX(mXAxisLimits[plotID].min);
            tempDoubleRect.setWidth(xAxisRange);
            tempDoubleRect.setY(mYLAxisLimits[plotID].min);
            tempDoubleRect.setHeight(leftAxisRange);
            mpZoomerLeft[plotID]->setZoomBase(tempDoubleRect);
            tempDoubleRect.setY(mYRAxisLimits[plotID].min);
            tempDoubleRect.setHeight(rightAxisRange);
            mpZoomerRight[plotID]->setZoomBase(tempDoubleRect);

            // Now call the actual refresh of the axes
            mpQwtPlots[plotID]->updateAxes();
        }
    }
}


//! @brief Removes a curve from the plot tab
//! @param curve Pointer to curve to remove
void PlotTab::removeCurve(PlotCurve *curve)
{
    int plotID = getPlotIDFromCurve(curve);

    for(int i=0; i<mMarkerPtrs[plotID].size(); ++i)
    {
        if(mMarkerPtrs[plotID].at(i)->getCurve() == curve)
        {
            mpQwtPlots[plotID]->canvas()->removeEventFilter(mMarkerPtrs[plotID].at(i));
            mMarkerPtrs[plotID].at(i)->detach();
            mMarkerPtrs[plotID].removeAt(i);
            --i;
        }
    }

    for(int i=0; i<mUsedColors.size(); ++i)
    {
        if(curve->pen().color() == mUsedColors.at(i))
        {
            mUsedColors.removeAt(i);
            break;
        }
    }

    curve->detach();
    for(int plotID=0; plotID<2; ++plotID)
    {
        mPlotCurvePtrs[plotID].removeAll(curve);
    }
    delete(curve);

    // Reset timevector incase we had special x-axis set previously
    if (mPlotCurvePtrs[plotID].isEmpty() && mHasCustomXData)
    {
        resetXTimeVector();
    }

    rescaleAxesToCurves();
    updateLabels();
    update();
}

void PlotTab::removeAllCurvesOnAxis(const int axis)
{
    QList<PlotCurve*> curvePtrs = getCurves();
    for(int c=0; c<curvePtrs.size(); ++c)
    {
        if(curvePtrs[c]->getAxisY() == axis)
        {
            removeCurve(curvePtrs.at(c));
        }
    }
}


//! @brief Changes the X vector of current plot tab to specified variable
//! @param xArray New data for X-axis
//! @param componentName Name of component from which new data origins
//! @param portName Name of port form which new data origins
//! @param dataName Data name (physical quantity) of new data
//! @param dataUnit Unit of new data
void PlotTab::setCustomXVectorForAll(QVector<double> xArray, const VariableDescription &rVarDesc, HopsanPlotIDEnumT plotID)
{
    LogVariableContainer *cont = new LogVariableContainer(rVarDesc,0);
    cont->addDataGeneration(0, SharedTimeVectorPtrT(), xArray);
    setCustomXVectorForAll(cont->getDataGeneration(-1),plotID);
}

void PlotTab::setCustomXVectorForAll(SharedLogVariableDataPtrT pData, HopsanPlotIDEnumT plotID)
{
    for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
    {
        if (!mPlotCurvePtrs[plotID].at(i)->hasCustomXData())
        {
            mPlotCurvePtrs[plotID].at(i)->setCustomXData(pData);
        }
        //mPlotCurvePtrs[plotID].at(i)->setSamples(mSpecialXVector, mPlotCurvePtrs[plotID].at(i)->getDataVector());
        //mPlotCurvePtrs[plotID].at(i)->setDataUnit(mPlotCurvePtrs[plotID].at(i)->getDataUnit());
    }
    rescaleAxesToCurves();

    setTabOnlyCustomXVector(pData,plotID);
}


//! @brief Updates labels on plot axes
void PlotTab::updateLabels()
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, QwtText());
        mpQwtPlots[plotID]->setAxisTitle(QwtPlot::yLeft, QwtText());
        mpQwtPlots[plotID]->setAxisTitle(QwtPlot::yRight, QwtText());

        if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == PortVariableType)
        {
            QStringList leftUnits, rightUnits;
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                QString newUnit = QString(mPlotCurvePtrs[plotID].at(i)->getDataName() + " [" + mPlotCurvePtrs[plotID].at(i)->getDataUnit() + "]");
                if( !(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yLeft && leftUnits.contains(newUnit)) && !(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yRight && rightUnits.contains(newUnit)) )
                {
                    if(!mpQwtPlots[plotID]->axisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY()).isEmpty())
                    {
                        mpQwtPlots[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), QwtText(QString(mpQwtPlots[plotID]->axisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY()).text().append(", "))));
                    }
                    mpQwtPlots[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), QwtText(QString(mpQwtPlots[plotID]->axisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY()).text().append(newUnit))));
                    if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yLeft)
                    {
                        leftUnits.append(newUnit);
                    }
                    if(mPlotCurvePtrs[plotID].at(i)->getAxisY() == QwtPlot::yRight)
                    {
                        rightUnits.append(newUnit);
                    }
                }
            }
            if (mHasCustomXData)
            {
                mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, QwtText(mCustomXDataLabel));
            }
            else
            {
                mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, QwtText("Time [s]"));
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == FrequencyAnalysisType)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpQwtPlots[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Relative Magnitude [-]");
                mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == NyquistType)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpQwtPlots[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Im");
                mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, "Re");
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == BodeGainType)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpQwtPlots[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Magnitude [dB]");
                mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, QwtText());      //No label, because there will be a phase plot bellow with same label
            }
        }
        else if(mPlotCurvePtrs[plotID].size()>0 && mPlotCurvePtrs[plotID][0]->getCurveType() == BodePhaseType)
        {
            for(int i=0; i<mPlotCurvePtrs[plotID].size(); ++i)
            {
                mpQwtPlots[plotID]->setAxisTitle(mPlotCurvePtrs[plotID].at(i)->getAxisY(), "Phase [deg]");
                mpQwtPlots[plotID]->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
            }
        }
    }
}

bool PlotTab::isGridVisible()
{
    return mpGrid[FirstPlot]->isVisible();
}


void PlotTab::resetXTimeVector()
{
    mHasCustomXData = false;
    mCustomXDataLabel = "";

    //! @todo what about second plot
    for(int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
    {
        mPlotCurvePtrs[FirstPlot].at(i)->setCustomXData(SharedLogVariableDataPtrT()); //Remove any custom x-data
        //mPlotCurvePtrs[FIRSTPLOT].at(i)->setSamples(mPlotCurvePtrs[FIRSTPLOT].at(i)->getTimeVector(), mPlotCurvePtrs[FIRSTPLOT].at(i)->getDataVector());
        //mPlotCurvePtrs[FIRSTPLOT].at(i)->setDataUnit(mPlotCurvePtrs[FIRSTPLOT].at(i)->getDataUnit());
    }

    updateLabels();
    update();

    rescaleAxesToCurves();
    mpQwtPlots[FirstPlot]->replot();
    mpQwtPlots[FirstPlot]->updateGeometry();

    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}


//! @brief Slot that opens a dialog from where user can export current plot tab to a XML file
void PlotTab::exportToXml()
{

    //Open a dialog where text and font can be selected
    mpExportXmlDialog = new QDialog(gpMainWindow);
    mpExportXmlDialog->setWindowTitle("Export Plot Tab To XML");

    QLabel *pXmlIndentationLabel = new QLabel("Indentation: ");

    mpXmlIndentationSpinBox = new QSpinBox(this);
    mpXmlIndentationSpinBox->setRange(0,100);
    mpXmlIndentationSpinBox->setValue(2);

    mpIncludeTimeCheckBox = new QCheckBox("Include date && time");
    mpIncludeTimeCheckBox->setChecked(true);

    mpIncludeDescriptionsCheckBox = new QCheckBox("Include variable descriptions");
    mpIncludeDescriptionsCheckBox->setChecked(true);

    QLabel *pOutputLabel = new QLabel("Output data:");

    mpXmlOutputTextBox = new QTextEdit();
    mpXmlOutputTextBox->toHtml();
    mpXmlOutputTextBox->setReadOnly(true);
    mpXmlOutputTextBox->setMinimumSize(700, 210);

    QPushButton *pDoneInDialogButton = new QPushButton("Export");
    QPushButton *pCancelInDialogButton = new QPushButton("Cancel");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout();
    pDialogLayout->addWidget(pXmlIndentationLabel,          0, 0);
    pDialogLayout->addWidget(mpXmlIndentationSpinBox,       0, 1);
    pDialogLayout->addWidget(mpIncludeTimeCheckBox,         2, 0, 1, 2);
    pDialogLayout->addWidget(mpIncludeDescriptionsCheckBox, 3, 0, 1, 2);
    pDialogLayout->addWidget(pOutputLabel,                  4, 0, 1, 2);
    pDialogLayout->addWidget(mpXmlOutputTextBox,            5, 0, 1, 4);
    pDialogLayout->addWidget(pButtonBox,                    6, 2, 1, 2);

    mpExportXmlDialog->setLayout(pDialogLayout);

    connect(mpXmlIndentationSpinBox,        SIGNAL(valueChanged(int)),  this,               SLOT(updateXmlOutputTextInDialog()));
    connect(mpIncludeTimeCheckBox,          SIGNAL(toggled(bool)),      this,               SLOT(updateXmlOutputTextInDialog()));
    connect(mpIncludeDescriptionsCheckBox,  SIGNAL(toggled(bool)),      this,               SLOT(updateXmlOutputTextInDialog()));
    connect(pDoneInDialogButton,            SIGNAL(clicked()),          this,               SLOT(saveToXml()));
    connect(pCancelInDialogButton,          SIGNAL(clicked()),          mpExportXmlDialog,  SLOT(close()));

    updateXmlOutputTextInDialog();
    mpExportXmlDialog->open();
}


//! @brief Slot that exports plot tab to a specified comma-separated value file (.csv)
void PlotTab::exportToCsv()
{
    //Open file dialog and initialize the file stream
    QString filePath;
    QFileInfo fileInfo;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To CSV File"),
                                            gConfig.getPlotDataDir(),
                                            tr("Comma-separated values (*.csv)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());

    exportToCsv(filePath);
}


//! @brief Exports plot tab to comma-separated value file with specified filename
//! @param fileName File name
void PlotTab::exportToCsv(QString fileName)
{
    QFile file;
    file.setFileName(fileName);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + fileName);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file


    //Cycle plot curves
    if(mHasCustomXData)
    {
        //! @todo how to handle this with multiple xvectors per curve
        //! @todo take into account wheter cached or not, Should have some smart auto function for this in the data object

        QVector<double> xvec = mpCustomXData->getDataVector(); //! @todo shoudl direct access if not in cache
        for(int i=0; i<xvec.size(); ++i)
        {
            fileStream << xvec[i];
            for(int j=0; j<mPlotCurvePtrs[FirstPlot].size(); ++j)
            {
                fileStream << ", " << mPlotCurvePtrs[FirstPlot][j]->getDataVector()[i];
            }
            fileStream << "\n";
        }
    }
    else
    {
        for(int i=0; i<mPlotCurvePtrs[FirstPlot][0]->getTimeVector().size(); ++i)
        {
            fileStream << mPlotCurvePtrs[FirstPlot][0]->getTimeVector()[i];
            for(int j=0; j<mPlotCurvePtrs[FirstPlot].size(); ++j)
            {
                fileStream << ", " << mPlotCurvePtrs[FirstPlot][j]->getDataVector()[i];
            }
            fileStream << "\n";
        }
    }


    //        //Cycle plot curves
    //    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
    //    {
    //        fileStream << "x" << i;                                         //Write time/X vector
    //        if(mHasSpecialXAxis)
    //        {
    //            for(int j=0; j<mVectorX.size(); ++j)
    //            {
    //                fileStream << "," << mVectorX[j];
    //            }
    //        }
    //        else
    //        {
    //            for(int j=0; j<mPlotCurvePtrs[FIRSTPLOT][i]->getTimeVector().size(); ++j)
    //            {
    //                fileStream << "," << mPlotCurvePtrs[FIRSTPLOT][i]->getTimeVector()[j];
    //            }
    //        }
    //        fileStream << "\n";

    //        fileStream << "y" << i;                                             //Write data vector
    //        for(int k=0; k<mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector().size(); ++k)
    //        {
    //            fileStream << "," << mPlotCurvePtrs[FIRSTPLOT][i]->getDataVector()[k];
    //        }
    //        fileStream << "\n";
    //    }

    file.close();
}

void PlotTab::exportToHvc(QString fileName)
{
    if (mPlotCurvePtrs[FirstPlot].size() < 1)
    {
        return;
    }

    QFileInfo fileInfo;
    if (fileName.isEmpty())
    {
        //Open file dialog and initialize the file stream

        QString filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To CSV File"),
                                                        gConfig.getPlotDataDir(),
                                                        tr("HopsanValidationCfg (*.hvc)"));
        if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
        fileInfo.setFile(filePath);
    }

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + fileName);
        return;
    }

    // Save the csv data
    QString csvFileName=fileInfo.baseName()+".csv";
    this->exportToCsv(fileInfo.absolutePath()+"/"+csvFileName);

    qDebug() << fileInfo.absoluteFilePath();
    qDebug() << fileInfo.absolutePath()+"/"+csvFileName;


    // Save HVC xml data
    QDomDocument doc;
    QDomElement hvcroot = doc.createElement("hopsanvalidationconfiguration");
    doc.appendChild(hvcroot);
    hvcroot.setAttribute("hvcversion", "0.1");

    QString modelPath = relativePath(mPlotCurvePtrs[FirstPlot][0]->getLogDataVariablePtr()->getLogDataHandler()->getParentContainerObject()->getModelFileInfo(), QDir(fileInfo.absolutePath()));
    QDomElement validation = appendDomElement(hvcroot, "validation");
    validation.setAttribute("date", QDateTime::currentDateTime().toString("yyyyMMdd"));
    validation.setAttribute("time", QDateTime::currentDateTime().toString("hhmmss"));
    validation.setAttribute("hopsanguiversion", HOPSANGUIVERSION);
    validation.setAttribute("hopsancoreversion", gHopsanCoreVersion);
    appendDomTextNode(validation, "modelfile", modelPath);
    appendDomTextNode(validation, "parameterset", "");

    //Cycle plot curves
    for (int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
    {
        PlotCurve *pPlotCurve = mPlotCurvePtrs[FirstPlot][i];

        QDomElement component = appendDomElement(validation, "component");
        component.setAttribute("name", pPlotCurve->getComponentName());

        QDomElement port = appendDomElement(component, "port");
        port.setAttribute("name", pPlotCurve->getPortName());

        QDomElement variable = appendDomElement(port, "variable");
        variable.setAttribute("name", pPlotCurve->getDataName());

        appendDomTextNode(variable, "csvfile", csvFileName);
        appendDomIntegerNode(variable, "column", i+1);

        appendDomValueNode(variable, "tolerance", 0.01);
    }

    QTextStream hvcFileStream(&file);
    appendRootXMLProcessingInstruction(doc); //The xml "comment" on the first line
    doc.save(hvcFileStream, 2);
    file.close();
}


//! @brief Slot that exports plot tab to a specified matlab script file (.m)
void PlotTab::exportToMatlab()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To MATLAB File"),
                                            gConfig.getPlotDataDir(),
                                            tr("MATLAB script file (*.m)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

    //Write initial comment
    fileStream << "% MATLAB File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";

    //Cycle plot curves
    for(int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
    {
        fileStream << "x" << i << "=[";                                         //Write time/X vector
        if(mHasCustomXData)
        {
            //! @todo need smart function to autoselect copy or direct access depending on cached or not (also in other places)
            QVector<double> xvec = mpCustomXData->getDataVector();
            for(int j=0; j<xvec.size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << xvec[j];
            }
        }
        else
        {
            for(int j=0; j<mPlotCurvePtrs[FirstPlot][i]->getTimeVector().size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mPlotCurvePtrs[FirstPlot][i]->getTimeVector()[j];
            }
        }
        fileStream << "];\n";

        fileStream << "y" << i << "=[";                                             //Write data vector
        for(int k=0; k<mPlotCurvePtrs[FirstPlot][i]->getDataVector().size(); ++k)
        {
            if(k>0) fileStream << ",";
            fileStream << mPlotCurvePtrs[FirstPlot][i]->getDataVector()[k];
        }
        fileStream << "];\n";
    }

    //Cycle plot curves
    for(int i=0; i<mPlotCurvePtrs[SecondPlot].size(); ++i)
    {
        fileStream << "x" << i+mPlotCurvePtrs[FirstPlot].size() << "=[";                                         //Write time/X vector
        if(mHasCustomXData)
        {
            //! @todo need smart function to autoselect copy or direct access depending on cached or not (also in other places)
            QVector<double> xvec = mpCustomXData->getDataVector();
            for(int j=0; j<xvec.size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << xvec[j];
            }
        }
        else
        {
            for(int j=0; j<mPlotCurvePtrs[SecondPlot][i]->getTimeVector().size(); ++j)
            {
                if(j>0) fileStream << ",";
                fileStream << mPlotCurvePtrs[SecondPlot][i]->getTimeVector()[j];
            }
        }
        fileStream << "];\n";

        fileStream << "y" << i+mPlotCurvePtrs[FirstPlot].size() << "=[";                                             //Write data vector
        for(int k=0; k<mPlotCurvePtrs[SecondPlot][i]->getDataVector().size(); ++k)
        {
            if(k>0) fileStream << ",";
            fileStream << mPlotCurvePtrs[SecondPlot][i]->getDataVector()[k];
        }
        fileStream << "];\n";
    }

    //Write plot functions
    QStringList matlabColors;
    matlabColors << "r" << "g" << "b" << "c" << "m" << "y";
    fileStream << "hold on\n";
    fileStream << "subplot(2,1,1)\n";
    for(int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
    {
        if((mPlotCurvePtrs[FirstPlot][i]->getAxisY() == QwtPlot::yLeft && mLeftAxisLogarithmic) || (mPlotCurvePtrs[FirstPlot][i]->getAxisY() == QwtPlot::yRight && mRightAxisLogarithmic))
        {
            if(mBottomAxisLogarithmic)
                fileStream << "loglog";
            else
                fileStream << "semilogy";
        }
        else
        {
            if(mBottomAxisLogarithmic)
                fileStream << "semilogx";
            else
                fileStream << "plot";
        }
        fileStream << "(x" << i << ",y" << i << ",'-" << matlabColors[i%6] << "','linewidth'," << mPlotCurvePtrs[FirstPlot][i]->pen().width() << ")\n";
    }
    if(mPlotCurvePtrs[SecondPlot].size() > 0)
    {
        fileStream << "subplot(2,1,2)\n";
        for(int i=0; i<mPlotCurvePtrs[SecondPlot].size(); ++i)
        {
            if((mPlotCurvePtrs[SecondPlot][i]->getAxisY() == QwtPlot::yLeft && mLeftAxisLogarithmic) || (mPlotCurvePtrs[SecondPlot][i]->getAxisY() == QwtPlot::yRight && mRightAxisLogarithmic))
            {
                if(mBottomAxisLogarithmic)
                    fileStream << "loglog";
                else
                    fileStream << "semilogy";
            }
            else
            {
                if(mBottomAxisLogarithmic)
                    fileStream << "semilogx";
                else
                    fileStream << "plot";
            }
            fileStream << "(x" << i+mPlotCurvePtrs[FirstPlot].size() << ",y" << i+mPlotCurvePtrs[FirstPlot].size() << ",'-" << matlabColors[i%6] << "','linewidth'," << mPlotCurvePtrs[SecondPlot][i]->pen().width() << ")\n";
        }
    }

    file.close();
}


//! @brief Slot that exports plot tab to specified gnuplot file  (.dat)
void PlotTab::exportToGnuplot()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To gnuplot File"),
                                            gConfig.getPlotDataDir(),
                                            tr("gnuplot file (*.dat)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();

    //Write initial comment
    fileStream << "# gnuplot File Exported From Hopsan " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";
    fileStream << "# T";
    for(int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
    {
        fileStream << "                  Y" << i;
    }
    fileStream << "\n";

    //Write time and data vectors
    QString dummy;
    for(int i=0; i<mPlotCurvePtrs[FirstPlot].first()->getTimeVector().size(); ++i)
    {
        dummy.setNum(mPlotCurvePtrs[FirstPlot].first()->getTimeVector()[i]);
        fileStream << dummy;
        for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }

        for(int k=0; k<mPlotCurvePtrs[FirstPlot].size(); ++k)
        {
            dummy.setNum(mPlotCurvePtrs[FirstPlot][k]->getDataVector()[i]);
            fileStream << dummy;
            for(int j=0; j<20-dummy.size(); ++j) { fileStream << " "; }
        }
        fileStream << "\n";
    }

    file.close();
}

void PlotTab::exportToGraphics()
{
    QDialog *pGraphicsSettingsDialog = new QDialog(mpParentPlotWindow);
    pGraphicsSettingsDialog->setWindowTitle("Graphic Export");
    pGraphicsSettingsDialog->setWindowModality(Qt::WindowModal);

    mpImageDimUnit = new QComboBox();
    mpImageDimUnit->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mpImageDimUnit->addItem("px");
    mPreviousImageUnit = mpImageDimUnit->currentText();
    mpImageDimUnit->addItem("mm");
    mpImageDimUnit->addItem("cm");
    mpImageDimUnit->addItem("in");
    connect(mpImageDimUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(changedGraphicsExportSettings()));

    mpImageSetWidth = new QDoubleSpinBox(this);
    mpImageSetWidth->setDecimals(0);
    mpImageSetWidth->setRange(1,10000);
    mpImageSetWidth->setSingleStep(1);
    mpImageSetWidth->setValue(mpQwtPlots[FirstPlot]->width());
    connect(mpImageSetWidth, SIGNAL(editingFinished()), this, SLOT(changedGraphicsExportSettings()));

    mpImageSetHeight = new QDoubleSpinBox(this);
    mpImageSetHeight->setDecimals(0);
    mpImageSetHeight->setRange(1,10000);
    mpImageSetHeight->setSingleStep(1);
    mpImageSetHeight->setValue(mpQwtPlots[FirstPlot]->height());
    connect(mpImageSetHeight, SIGNAL(editingFinished()), this, SLOT(changedGraphicsExportSettings()));

    mpPixelSizeLabel = new QLabel(QString("%1X%2").arg(mpQwtPlots[FirstPlot]->width()).arg(mpQwtPlots[FirstPlot]->height()));
    mImagePixelSize = QSize(mpQwtPlots[FirstPlot]->width(), mpQwtPlots[FirstPlot]->height());


    mpImageDPI = new QDoubleSpinBox(this);
    mpImageDPI->setDecimals(0);
    mpImageDPI->setRange(1,10000);
    mpImageDPI->setSingleStep(1);
    mpImageDPI->setValue(96);
    connect(mpImageDPI, SIGNAL(editingFinished()), this, SLOT(changedGraphicsExportSettings()));

    // Vector
    mpImageFormat = new QComboBox();
    mpImageFormat->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mpImageFormat->addItem("pdf");
    mpImageFormat->addItem("ps");
    mpImageFormat->addItem("svg");
    mpImageFormat->addItem("png");
    mpImageFormat->addItem("jpeg");

    mpKeepAspectRatio = new QCheckBox("Keep Aspect Ratio");
    mpKeepAspectRatio->setChecked(true);

    int r=0;
    QGridLayout *pLayout = new QGridLayout();
    pLayout->addWidget(new QLabel("Format:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageFormat,r,1);
    pLayout->addWidget(new QLabel("Px:"),r,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpPixelSizeLabel,r,3);
    ++r;
    pLayout->addWidget(new QLabel("Dimension Unit:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageDimUnit,r,1);
    pLayout->addWidget(new QLabel("Width:"),r,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageSetWidth,r,3);
    pLayout->addWidget(new QLabel("Height:"),r+1,2, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageSetHeight,r+1,3);
    ++r;
    pLayout->addWidget(new QLabel("DPI:"),r,0, 1,1,Qt::AlignRight);
    pLayout->addWidget(mpImageDPI,r,1);
    mpImageDPI->setDisabled(true);
    ++r;
    pLayout->addWidget(mpKeepAspectRatio,r,1);
    ++r;

    QPushButton *pExportButton = new QPushButton("Export");
    pExportButton->setAutoDefault(false);
    pLayout->addWidget(pExportButton,r,0);
    connect(pExportButton, SIGNAL(clicked()), this, SLOT(exportImage()));
    QPushButton *pCloseButton = new QPushButton("Close");
    pCloseButton->setAutoDefault(false);
    pLayout->addWidget(pCloseButton,r,5);
    connect(pCloseButton, SIGNAL(clicked()), pGraphicsSettingsDialog, SLOT(close()));

    pGraphicsSettingsDialog->setLayout(pLayout);
    pGraphicsSettingsDialog->exec();
}

//! @brief Slot that exports plot tab as vector graphics to specified .pdf file
void PlotTab::exportToPdf()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, "Export File Name", gConfig.getPlotGfxDir(), "Portable Document Format (*.pdf);;PostScript Format (*.ps)", &selectedFilter);
    if ( !fileName.isEmpty() )
    {
        QFileInfo file(fileName);
        gConfig.setPlotGfxDir(file.absolutePath());

        QwtPlotRenderer renderer;

        QPrinter *printer = new QPrinter(QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::Custom);
        printer->setPaperSize(mpQwtPlots[FirstPlot]->size(), QPrinter::Point);
        printer->setOrientation(QPrinter::Landscape);
        printer->setFullPage(false);
        if(selectedFilter == "Portable Document Format (*.pdf)")
            printer->setOutputFormat(QPrinter::PdfFormat);
        else
            printer->setOutputFormat(QPrinter::PostScriptFormat);
        printer->setOutputFileName(fileName);
        renderer.renderTo(mpQwtPlots[FirstPlot],*printer);
    }
}

void PlotTab::exportToOldHop()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To OldHopsan Format File"),
                                            gConfig.getPlotDataDir(),
                                            tr("Hopsan Classic file (*.PLO)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());

    QStringList variables;
    for(int c=0; c<mPlotCurvePtrs[FirstPlot].size(); ++c)
    {
        variables.append(mPlotCurvePtrs[FirstPlot][c]->getLogDataVariablePtr()->getFullVariableName());
    }

    mPlotCurvePtrs[FirstPlot].first()->getLogDataVariablePtr()->getLogDataHandler()->getParentContainerObject()->getLogDataHandler()->exportToPlo(filePath, variables);

    //    file.setFileName(fileInfo.filePath());   //Create a QFile object
    //    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    //    {
    //        gpMainWindow->mpHcomWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
    //        return;
    //    }

    //    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    //    QDateTime dateTime = QDateTime::currentDateTime();
    //    QString dateTimeString = dateTime.toString();
    //    QFileInfo fii(filePath);
    //    QString namez = fii.baseName();
    //    QStringList ScalingvaluesList;
    //    QStringList StartvaluesList;
    //    QVector<double> Scalings;
    //    QString ScaleVal;

    //    QString modelPathwayy = gpMainWindow->mpProjectTabs->getCurrentContainer()->getModelFileInfo().filePath();
    //    QFileInfo fiz(modelPathwayy);
    //    QString namemodel = fiz.baseName();

    //        //Write initial comment
    //    fileStream << "    'VERSION' " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";
    //    fileStream << "    1 " << "\n";
    //    fileStream << "    '"<<namez<<".PLO"<<"'"<<"\n";
    //    fileStream << "        " << mPlotCurvePtrs[FIRSTPLOT].size()<<"    "<< mPlotCurvePtrs[FIRSTPLOT].first()->getTimeVector().size()<<"\n";
    //    fileStream << "    'Time      '";
    //    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].size(); ++i)
    //    {
    //        fileStream << ",    'Y" << i<<"      '";
    //    }
    //    fileStream <<",    '"<< "\n";

    //        //Write time and data vectors
    //    QString dummy;


    ////    double iMax = Scalings[0]; //set min and max as the first element
    ////    double iMin = Scalings[0];
    ////    for (int xi=0; xi<Scalings.size(); xi++)
    ////    {
    ////        if (Scalings[xi] < iMin)
    ////            iMin = Scalings[xi];
    ////        if (Scalings[xi] > iMax)
    ////            iMax = Scalings[xi];

    ////        double Scale = (iMax-iMin)/(Scalings.size());


    ////    }




    //    for(int kk=0; kk<mPlotCurvePtrs[FIRSTPLOT].size()+1; ++kk)
    //    {

    //        ScalingvaluesList.append(dummy.setNum(1.0,'E',6));
    //        fileStream <<"  "<< dummy;
    //        for(int j=0; j<12-dummy.size(); ++j) { fileStream << " "; }


    //    }
    //    fileStream << "\n";


    //    for(int i=0; i<mPlotCurvePtrs[FIRSTPLOT].first()->getTimeVector().size(); ++i)
    //    {
    //        dummy.setNum(mPlotCurvePtrs[FIRSTPLOT].first()->getTimeVector()[i],'E',6);
    //        fileStream <<"  "<<dummy;
    //        for(int j=0; j<12-dummy.size(); ++j) { fileStream << " "; }

    //        for(int k=0; k<mPlotCurvePtrs[FIRSTPLOT].size(); ++k)
    //        {
    //            dummy.setNum(mPlotCurvePtrs[FIRSTPLOT][k]->getDataVector()[i],'E',6);
    //            Scalings = mPlotCurvePtrs[FIRSTPLOT][k]->getDataVector();
    //            if(i == 0)
    //            {
    //               StartvaluesList.append(dummy.setNum(mPlotCurvePtrs[FIRSTPLOT][k]->getDataVector()[i],'E',6));
    //            }

    //            fileStream <<"  "<< dummy;
    //            for(int j=0; j<12-dummy.size(); ++j) { fileStream << " "; }

    //        }
    //        fileStream << "\n";
    //    }
    //    fileStream << "  "+namez+".PLO.DAT_-1" <<"\n";
    //    fileStream << "  "+namemodel+".for" <<"\n";
    //    fileStream <<"   Variable     Startvalue     Scaling" <<"\n";
    //    fileStream <<"------------------------------------------------------" <<"\n";
    //    for(int ii=0; ii<mPlotCurvePtrs[FIRSTPLOT].size(); ++ii)
    //    {
    //        fileStream << "  Y" << ii << "     " << StartvaluesList[ii]<<"      "<<ScalingvaluesList[ii]<<"\n";
    //    }



    //    file.close();
}

//! @brief Slot that exports plot tab as bitmap to specified .png file
void PlotTab::exportToPng()
{
    QString fileName = QFileDialog::getSaveFileName(
                this, "Export File Name", gConfig.getPlotGfxDir(),
                "Portable Network Graphics (*.png)");

    if(!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        gConfig.setPlotGfxDir(file.absolutePath());

        if(mpBarPlot->isVisible())
        {
            QPixmap pixmap = QPixmap::grabWidget(this);
            pixmap.save(fileName);
        }
        else
        {
            QPixmap pixmap(mpQwtPlots[FirstPlot]->width(), mpQwtPlots[FirstPlot]->height());
            pixmap.fill();
            QwtPlotRenderer renderer;
            renderer.renderTo(mpQwtPlots[FirstPlot], pixmap);
            pixmap.save(fileName);
        }
    }
}

void PlotTab::shiftAllGenerationsDown()
{
    for (int i=0; i<mPlotCurvePtrs[0].size(); ++i)
    {
        mPlotCurvePtrs[0][i]->setPreviousGeneration();
    }

    for (int i=0; i<mPlotCurvePtrs[1].size(); ++i)
    {
        mPlotCurvePtrs[1][i]->setPreviousGeneration();
    }
}

void PlotTab::shiftAllGenerationsUp()
{
    for (int i=0; i<mPlotCurvePtrs[0].size(); ++i)
    {
        mPlotCurvePtrs[0][i]->setNextGeneration();
    }

    for (int i=0; i<mPlotCurvePtrs[1].size(); ++i)
    {
        mPlotCurvePtrs[1][i]->setNextGeneration();
    }
}


void PlotTab::enableZoom(bool value)
{
    if(mpParentPlotWindow->mpPanButton->isChecked() && value)
    {
        mpParentPlotWindow->mpPanButton->setChecked(false);
        mpPanner[FirstPlot]->setEnabled(false);
        mpPanner[SecondPlot]->setEnabled(false);
    }
    if(mpParentPlotWindow->mpArrowButton->isChecked() && value)
    {
        mpParentPlotWindow->mpArrowButton->setChecked(false);
    }
    mpZoomerLeft[FirstPlot]->setEnabled(value);
    if(value)   { mpZoomerLeft[FirstPlot]->setRubberBand(QwtPicker::RectRubberBand); }
    else        { mpZoomerLeft[FirstPlot]->setRubberBand(QwtPicker::NoRubberBand); }
    mpZoomerRight[FirstPlot]->setEnabled(value);
    mpZoomerLeft[SecondPlot]->setEnabled(value);
    if(value)   { mpZoomerLeft[SecondPlot]->setRubberBand(QwtPicker::RectRubberBand); }
    else        { mpZoomerLeft[SecondPlot]->setRubberBand(QwtPicker::NoRubberBand); }
    mpZoomerRight[SecondPlot]->setEnabled(value);
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(false);
}

void PlotTab::resetZoom()
{
    mpZoomerLeft[FirstPlot]->zoom(0);
    mpZoomerRight[FirstPlot]->zoom(0);
    rescaleAxesToCurves();
}

void PlotTab::enableArrow(bool value)
{
    if(mpParentPlotWindow->mpZoomButton->isChecked() && value)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        mpZoomerLeft[FirstPlot]->setEnabled(false);
        mpZoomerLeft[FirstPlot]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[FirstPlot]->setEnabled(false);
        mpZoomerLeft[SecondPlot]->setEnabled(false);
        mpZoomerLeft[SecondPlot]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[SecondPlot]->setEnabled(false);
    }
    if(mpParentPlotWindow->mpPanButton->isChecked() && value)
    {
        mpParentPlotWindow->mpPanButton->setChecked(false);
        mpPanner[FirstPlot]->setEnabled(false);
        mpPanner[SecondPlot]->setEnabled(false);
    }

}


void PlotTab::enablePan(bool value)
{
    if(mpParentPlotWindow->mpZoomButton->isChecked() && value)
    {
        mpParentPlotWindow->mpZoomButton->setChecked(false);
        //mpParentPlotWindow->mpArrowButton->setChecked(false);
        mpZoomerLeft[FirstPlot]->setEnabled(false);
        mpZoomerLeft[FirstPlot]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[FirstPlot]->setEnabled(false);
        mpZoomerLeft[SecondPlot]->setEnabled(false);
        mpZoomerLeft[SecondPlot]->setRubberBand(QwtPicker::NoRubberBand);
        mpZoomerRight[SecondPlot]->setEnabled(false);
    }
    if(mpParentPlotWindow->mpArrowButton->isChecked() && value)
    {
        mpParentPlotWindow->mpArrowButton->setChecked(false);
    }
    mpPanner[FirstPlot]->setEnabled(value);
    mpPanner[SecondPlot]->setEnabled(value);
}


void PlotTab::enableGrid(bool value)
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        mpGrid[plotID]->setVisible(value);
    }
}


void PlotTab::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(mpQwtPlots[FirstPlot]->canvasBackground().color(), this);
    if (color.isValid())
    {
        mpQwtPlots[FirstPlot]->setCanvasBackground(color);
        mpQwtPlots[FirstPlot]->replot();
        mpQwtPlots[FirstPlot]->updateGeometry();
        mpQwtPlots[SecondPlot]->setCanvasBackground(color);
        mpQwtPlots[SecondPlot]->replot();
        mpQwtPlots[SecondPlot]->updateGeometry();
    }
}


QList<PlotCurve *> PlotTab::getCurves(HopsanPlotIDEnumT plotID)
{
    return mPlotCurvePtrs[plotID];
}


void PlotTab::setActivePlotCurve(PlotCurve *pCurve)
{
    // Mark deactive all others
    //! @todo if only one can be active it should be enough to deactivate that one
    for(int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
    {
        if(mPlotCurvePtrs[FirstPlot].at(i) != pCurve)
        {
            mPlotCurvePtrs[FirstPlot].at(i)->markActive(false);
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


PlotCurve *PlotTab::getActivePlotCurve()
{
    return mpActivePlotCurve;
}


QwtPlot *PlotTab::getPlot(HopsanPlotIDEnumT plotID)
{
    return mpQwtPlots[plotID];
}


void PlotTab::showPlot(HopsanPlotIDEnumT plotID, bool visible)
{
    mpQwtPlots[plotID]->setVisible(visible);
}


int PlotTab::getNumberOfCurves(HopsanPlotIDEnumT plotID)
{
    return mPlotCurvePtrs[plotID].size();
}



void PlotTab::update()
{
    for(int plotID=0; plotID<1; ++plotID)
    {
        mpQwtPlots[plotID]->enableAxis(QwtPlot::yLeft, false);
        mpQwtPlots[plotID]->enableAxis(QwtPlot::yRight, false);
        QList<PlotCurve *>::iterator cit;
        for(cit=mPlotCurvePtrs[plotID].begin(); cit!=mPlotCurvePtrs[plotID].end(); ++cit)
        {
            if(!mpQwtPlots[plotID]->axisEnabled((*cit)->getAxisY())) { mpQwtPlots[plotID]->enableAxis((*cit)->getAxisY()); }
            (*cit)->attach(mpQwtPlots[plotID]);
        }

        // Update plotmarkers
        for(int i=0; i<mMarkerPtrs[plotID].size(); ++i)
        {
            QPointF posF = mMarkerPtrs[plotID].at(i)->value();
            double x = mpQwtPlots[plotID]->transform(QwtPlot::xBottom, posF.x());
            double y = mpQwtPlots[plotID]->transform(QwtPlot::yLeft, posF.y());
            QPoint pos = QPoint(x,y);
            PlotCurve *pCurve = mMarkerPtrs[plotID].at(i)->getCurve();
            mMarkerPtrs[plotID].at(i)->setXValue(pCurve->sample(pCurve->closestPoint(pos)).x());
            mMarkerPtrs[plotID].at(i)->setYValue(mpQwtPlots[plotID]->invTransform(QwtPlot::yLeft, mpQwtPlots[plotID]->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));
            //!< @todo label text will be wrong if curve data has changed
            //!< @todo label text will be wrong if plot sclae or offset change
        }
        mpQwtPlots[plotID]->replot();
        mpQwtPlots[plotID]->updateGeometry();
    }
}


void PlotTab::insertMarker(PlotCurve *pCurve, double x, double y, QString altLabel, bool movable)
{
    int plotID = getPlotIDFromCurve(pCurve);

    PlotMarker *pMarker = new PlotMarker(pCurve, this);
    mMarkerPtrs[plotID].append(pMarker);

    pMarker->attach(mpQwtPlots[plotID]);
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

    mpQwtPlots[plotID]->canvas()->installEventFilter(pMarker);
    mpQwtPlots[plotID]->canvas()->setMouseTracking(true);
    pMarker->setMovable(movable);
}


//! @brief Inserts a curve marker at the specified curve
//! @param curve is a pointer to the specified curve
void PlotTab::insertMarker(PlotCurve *pCurve, QPoint pos, bool movable)
{
    int plotID = getPlotIDFromCurve(pCurve);

    PlotMarker *pMarker = new PlotMarker(pCurve, this);
    mMarkerPtrs[plotID].append(pMarker);

    pMarker->attach(mpQwtPlots[plotID]);
    QCursor cursor;
    pMarker->setXValue(pCurve->sample(pCurve->closestPoint(pos)).x());
    pMarker->setYValue(mpQwtPlots[plotID]->invTransform(QwtPlot::yLeft, mpQwtPlots[plotID]->transform(pCurve->yAxis(), pCurve->sample(pCurve->closestPoint(pos)).y())));

    pMarker->refreshLabel(pCurve->sample(pCurve->closestPoint(pos)).x(),
                          pCurve->sample(pCurve->closestPoint(mpQwtPlots[plotID]->canvas()->mapFromGlobal(cursor.pos()))).y());

    mpQwtPlots[plotID]->canvas()->installEventFilter(pMarker);
    mpQwtPlots[plotID]->canvas()->setMouseTracking(true);
    pMarker->setMovable(movable);
}


//! @brief Saves the current tab to a DOM element (XML)
//! @param rDomElement Reference to the dom element to save to
//! @param dateTime Tells whether or not date and time should be included
//! @param descriptions Tells whether or not variable descriptions shall be included
void PlotTab::saveToDomElement(QDomElement &rDomElement, bool dateTime, bool descriptions)
{
    if(dateTime)
    {
        QDateTime datetime;
        rDomElement.setAttribute("datetime", datetime.currentDateTime().toString(Qt::ISODate));
    }

    if(mpBarPlot->isVisible())
    {
        QAbstractItemModel *model = mpBarPlot->model();

        for(int c=0; c<model->columnCount(); ++c)
        {
            double losses = model->data(model->index(0, c)).toInt() - model->data(model->index(1, c)).toInt();;

            QDomElement dataTag = appendDomElement(rDomElement, "data");
            QDomElement varTag = appendDomElement(dataTag, "losses");
            QString valueString;
            valueString.setNum(losses);
            QDomText value = varTag.ownerDocument().createTextNode(valueString);
            varTag.appendChild(value);

            if(descriptions)
            {
                varTag.setAttribute("component", model->headerData(c, Qt::Horizontal).toString());
            }
        }
    }
    else
    {

        //Cycle plot curves and write data tags
        QString dummy;
        for(int j=0; j<mPlotCurvePtrs[FirstPlot][0]->getTimeVector().size(); ++j)
        {
            QDomElement dataTag = appendDomElement(rDomElement, "data");

            if(mHasCustomXData)        //Special x-axis, replace time with x-data
            {
                setQrealAttribute(dataTag, mpCustomXData->getDataName(), mpCustomXData->peekData(j,dummy), 10, 'g');
            }
            else                        //X-axis = time
            {
                setQrealAttribute(dataTag, "time", mPlotCurvePtrs[FirstPlot][0]->getTimeVector()[j], 10, 'g');
            }

            //Write variable tags for each variable
            for(int i=0; i<mPlotCurvePtrs[FirstPlot].size(); ++i)
            {
                QString numTemp;
                numTemp.setNum(i);
                QDomElement varTag = appendDomElement(dataTag, mPlotCurvePtrs[FirstPlot][i]->getDataName()+numTemp);
                QString valueString;
                valueString.setNum(mPlotCurvePtrs[FirstPlot][i]->getDataVector()[j]);
                QDomText value = varTag.ownerDocument().createTextNode(valueString);
                varTag.appendChild(value);

                if(descriptions)
                {
                    varTag.setAttribute("component", mPlotCurvePtrs[FirstPlot][i]->getComponentName());
                    varTag.setAttribute("port", mPlotCurvePtrs[FirstPlot][i]->getPortName());
                    varTag.setAttribute("type", mPlotCurvePtrs[FirstPlot][i]->getDataName());
                    varTag.setAttribute("unit", mPlotCurvePtrs[FirstPlot][i]->getDataUnit());
                }
            }
        }
    }
}


bool PlotTab::isSpecialPlot()
{
    return mIsSpecialPlot;
}


void PlotTab::setBottomAxisLogarithmic(bool value)
{
    mBottomAxisLogarithmic = value;
    if(value)
    {
        getPlot(FirstPlot)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
        getPlot(SecondPlot)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
    }
    else
    {
        getPlot(FirstPlot)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
        getPlot(SecondPlot)->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    }

}


bool PlotTab::hasLogarithmicBottomAxis()
{
    return mBottomAxisLogarithmic;
}

bool PlotTab::isZoomed(const int plotId) const
{
//    uint l = mpZoomerLeft[plotId]->zoomRectIndex();
//    uint r = mpZoomerRight[plotId]->zoomRectIndex();
//    qDebug() << "l,r: " << l <<" "<< r;
//    return (l!=0) && (r!=0);
    return (mpZoomerLeft[plotId]->zoomRectIndex() != 0) && (mpZoomerRight[plotId]->zoomRectIndex() != 0);
}


//! @todo this only tunrs on internal legend automatically, maybe need an otehr version with two arguments
void PlotTab::setLegendsVisible(bool value)
{
    if (value)
    {
        //Only turn on internal automatically
        mpLegendsInternalEnabledCheckBox->setChecked(true);
    }
    else
    {
        mpLegendsInternalEnabledCheckBox->setChecked(false);
        mpLegendsExternalEnabledCheckBox->setChecked(false);
    }
    applyLegendSettings();
}


//! @brief Private slot that updates the xml preview field in the export to xml dialog
QString PlotTab::updateXmlOutputTextInDialog()
{
    QDomDocument domDocument;
    QDomElement element = domDocument.createElement("hopsanplotdata");
    domDocument.appendChild(element);
    this->saveToDomElement(element, mpIncludeTimeCheckBox->isChecked(), mpIncludeDescriptionsCheckBox->isChecked());
    QString output = domDocument.toString(mpXmlIndentationSpinBox->value());

    QStringList lines = output.split("\n");

    //We want the first 10 lines and the last 2 from the xml output
    QString display;
    for(int i=0; i<10 && i<lines.size(); ++i)
    {
        display.append(lines[i]);
        display.append("\n");
    }
    for(int k=0; k<mpXmlIndentationSpinBox->value(); ++k) display.append(" ");
    if(lines.size() > 9)
    {
        display.append("...\n");
        display.append(lines[lines.size()-2]);
        display.append(lines[lines.size()-1]);
    }


    display.replace(" ", "&nbsp;");
    display.replace(">", "!!!GT!!!");
    display.replace("<", "<font color=\"saddlebrown\">&lt;");
    display.replace("!!!GT!!!","</font><font color=\"saddlebrown\">&gt;</font>");
    display.replace("\n", "<br>\n");
    display.replace("&lt;?xml", "&lt;?xml</font>");
    display.replace("&lt;data", "&lt;data</font>");

    display.replace("0&nbsp;", "0</font>&nbsp;");
    display.replace("1&nbsp;", "1</font>&nbsp;");
    display.replace("2&nbsp;", "2</font>&nbsp;");
    display.replace("3&nbsp;", "3</font>&nbsp;");
    display.replace("4&nbsp;", "4</font>&nbsp;");
    display.replace("5&nbsp;", "5</font>&nbsp;");
    display.replace("6&nbsp;", "6</font>&nbsp;");
    display.replace("7&nbsp;", "7</font>&nbsp;");
    display.replace("8&nbsp;", "8</font>&nbsp;");
    display.replace("9&nbsp;", "9</font>&nbsp;");

    display.replace("&lt;hopsanplotdata", "&lt;hopsanplotdata</font>");
    display.replace("&lt;losses", "&lt;losses</font>");
    display.replace("&nbsp;version=", "&nbsp;version=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;encoding=", "&nbsp;encoding=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;component=", "&nbsp;component=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;port=", "&nbsp;port=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;type=", "&nbsp;type=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;unit=", "&nbsp;unit=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;time=", "&nbsp;time=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("&nbsp;datetime=", "&nbsp;datetime=<font face=\"Consolas\" color=\"darkred\">");
    display.replace("\"&nbsp;", "\"</font>&nbsp;");

    display.replace("&nbsp;", "<font face=\"Consolas\">&nbsp;</font>");
    display.replace("</font></font>", "</font>");

    mpXmlOutputTextBox->setText(display);

    return output;
}


//! @brief Private slot that opens a file dialog and saves the current tab to a specified XML file
//! @note Don't call this directly, call exportToXml() first and it will subsequently call this slot
void PlotTab::saveToXml()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(this, tr("Export Plot Tab To XML File"),
                                            gConfig.getPlotDataDir(),
                                            tr("Extensible Markup Language (*.xml)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    gConfig.setPlotDataDir(fileInfo.absolutePath());
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    QDomDocument domDocument;
    QDomElement element = domDocument.createElement("hopsanplotdata");
    domDocument.appendChild(element);
    this->saveToDomElement(element, mpIncludeTimeCheckBox->isChecked(), mpIncludeDescriptionsCheckBox->isChecked());
    appendRootXMLProcessingInstruction(domDocument);

    QTextStream fileStream(&file);
    domDocument.save(fileStream, mpXmlIndentationSpinBox->value());
    file.close();

    mpExportXmlDialog->close();
}

void PlotTab::exportImage()
{
    QString fileName, fileFilter;
    if (mpImageFormat->currentText() == "pdf")
    {
        fileFilter = "Portable Document Format (*.pdf)";
    }
    else if (mpImageFormat->currentText() == "ps")
    {
        fileFilter = "PostScript Format (*.ps)";
    }
    else if (mpImageFormat->currentText() == "svg")
    {
        fileFilter = "Scalable Vector Graphics (*.svg)";
    }
    else if (mpImageFormat->currentText() == "png")
    {
        fileFilter = "Portable Network Graphics (*.png)";
    }
    else if (mpImageFormat->currentText() == "jpeg")
    {
        fileFilter = "Joint Photographic Experts Group (*.jpg)";
    }

    fileName = QFileDialog::getSaveFileName(this, "Export File Name", gConfig.getPlotGfxDir(), fileFilter);

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground,true);
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame,true);
    renderer.renderDocument(mpQwtPlots[FirstPlot],fileName,calcMMSize(),mpImageDPI->value());
}

void PlotTab::changedGraphicsExportSettings()
{

    // Recalculate values for setSize boxes if unit changes
    if (mPreviousImageUnit != mpImageDimUnit->currentText())
    {
        updateGraphicsExportSizeEdits();
        mPreviousImageUnit = mpImageDimUnit->currentText();

        mImagePixelSize = calcPXSize(); // Set new pxSize
    }
    else if (mpKeepAspectRatio->isChecked())
    {
        // Calc new actual pixel resolution
         QSizeF pxSize = calcPXSize();

        // Adjust size according to AR
        double ar = mImagePixelSize.width() / mImagePixelSize.height();
        // See which one changed
        if ( fabs(pxSize.width() - mImagePixelSize.width()) > fabs(pxSize.height() - mImagePixelSize.height()) )
        {
            pxSize.rheight() = pxSize.width() * 1/ar;
        }
        else
        {
            pxSize.rwidth() = pxSize.height() * ar;
        }
        mImagePixelSize = pxSize; // Set new pxSize

        updateGraphicsExportSizeEdits();
    }
    else
    {
        mImagePixelSize = calcPXSize(); // Set new pxSize
    }

    mpPixelSizeLabel->setText(QString("%1X%2").arg(round(mImagePixelSize.width())).arg(round(mImagePixelSize.height())));
}



int PlotTab::getPlotIDFromCurve(PlotCurve *pCurve)
{
    for(int plotID=0; plotID<2; ++plotID)
    {
        if(mPlotCurvePtrs[plotID].contains(pCurve))
            return plotID;
    }
    qFatal("Plot curve has no plot ID (should never happen)");
    Q_ASSERT(false);      //Plot curve has no plot ID (should never happen)
    return -1;
}

//! @brief HelpFunction for constructor
void PlotTab::constructLegendSettingsDialog()
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

    mpLegendLeftOffset = new QDoubleSpinBox(this);
    mpLegendLeftOffset->setRange(-DoubleMax, DoubleMax);
    mpLegendLeftOffset->setDecimals(2);
    mpLegendLeftOffset->setSingleStep(0.1);
    mpLegendLeftOffset->setValue(0);

    mpLegendRightOffset = new QDoubleSpinBox(this);
    mpLegendRightOffset->setRange(-DoubleMax, DoubleMax);
    mpLegendRightOffset->setDecimals(2);
    mpLegendRightOffset->setSingleStep(0.1);
    mpLegendRightOffset->setValue(0);

    mpLegendsAutoOffsetCheckBox = new QCheckBox(this);
    mpLegendsAutoOffsetCheckBox->setCheckable(true);
    mpLegendsAutoOffsetCheckBox->setChecked(true);

//    mpLegendsOffYREnabledCheckBox = new QCheckBox(this);
//    mpLegendsOffYREnabledCheckBox->setCheckable(true);

    mpLegendsInternalEnabledCheckBox = new QCheckBox(this);
    mpLegendsInternalEnabledCheckBox->setCheckable(true);
    mpLegendsInternalEnabledCheckBox->setChecked(true); //Internal on by default

    mpLegendsExternalEnabledCheckBox = new QCheckBox(this);
    mpLegendsExternalEnabledCheckBox->setCheckable(true);

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

    QGroupBox *legendBox = new QGroupBox( "Legend" );
    QGridLayout *legendBoxLayout = new QGridLayout( legendBox );

    int row = 0;
    legendBoxLayout->addWidget( new QLabel( "Size" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendFontSize, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Columns" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendCols, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Left Legend Position" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendLPosition, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Right Legend Position" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendRPosition, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Background" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendBgType, row, 1 );

    QPushButton *pFinishedLegButton = new QPushButton("Close", mpLegendSettingsDialog);
    QDialogButtonBox *pFinishedLegButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pFinishedLegButtonBox->addButton(pFinishedLegButton, QDialogButtonBox::ActionRole);

    row++;
    legendBoxLayout->addWidget( new QLabel( "Internal Legends" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendsInternalEnabledCheckBox, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "External Legends" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendsExternalEnabledCheckBox, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Legend BG Color" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendBgColor, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Legend Symbol" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendSymbolType, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Auto Offset" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendsAutoOffsetCheckBox, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Left Legend Offset" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendLeftOffset, row, 1 );

    row++;
    legendBoxLayout->addWidget( new QLabel( "Right Legend Offset" ), row, 0 );
    legendBoxLayout->addWidget( mpLegendRightOffset, row, 1 );

    row++;
    legendBoxLayout->addWidget( pFinishedLegButton, row, 1 );

    mpLegendSettingsDialog->setLayout(legendBoxLayout);

    connect(mpLegendFontSize, SIGNAL(valueChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendCols, SIGNAL(valueChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendsInternalEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(mpLegendsExternalEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(mpLegendBgType, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendSymbolType, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendLPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendRPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendBgColor, SIGNAL(currentIndexChanged(int)), this, SLOT(applyLegendSettings()));
    connect(mpLegendLeftOffset, SIGNAL(valueChanged(double)), this, SLOT(applyLegendSettings()));
    connect(mpLegendRightOffset, SIGNAL(valueChanged(double)), this, SLOT(applyLegendSettings()));
    connect(mpLegendsAutoOffsetCheckBox, SIGNAL(toggled(bool)), this, SLOT(applyLegendSettings()));
    connect(pFinishedLegButton, SIGNAL(clicked()), mpLegendSettingsDialog, SLOT(close()));
}

//! @brief HelpFunction for constructor
void PlotTab::constructAxisSettingsDialog()
{
    mpSetAxisDialog = new QDialog(this);
    mpSetAxisDialog->setWindowTitle("Set Axis Limits");

    mpXAutoCheckBox = new QCheckBox("Auto limits");
    mpXAutoCheckBox->setCheckable(true);
    mpXAutoCheckBox->setChecked(true);
    mpYLAutoCheckBox = new QCheckBox("Auto limits");
    mpYLAutoCheckBox->setCheckable(true);
    mpYLAutoCheckBox->setChecked(true);
    mpYRAutoCheckBox = new QCheckBox("Auto limits");
    mpYRAutoCheckBox->setCheckable(true);
    mpYRAutoCheckBox->setChecked(true);

    mpXminSpinBox = new QDoubleSpinBox(mpSetAxisDialog);
    mpXminSpinBox->setRange(-DoubleMax, DoubleMax);
    mpXminSpinBox->setDecimals(10);
    mpXminSpinBox->setSingleStep(0.1);

    mpXmaxSpinBox = new QDoubleSpinBox(mpSetAxisDialog);
    mpXmaxSpinBox->setRange(-DoubleMax, DoubleMax);
    mpXmaxSpinBox->setDecimals(10);
    mpXmaxSpinBox->setSingleStep(0.1);

    mpYLminSpinBox = new QDoubleSpinBox(mpSetAxisDialog);
    mpYLminSpinBox->setRange(-DoubleMax, DoubleMax);
    mpYLminSpinBox->setDecimals(10);
    mpYLminSpinBox->setSingleStep(0.1);

    mpYLmaxSpinBox = new QDoubleSpinBox(mpSetAxisDialog);
    mpYLmaxSpinBox->setRange(-DoubleMax, DoubleMax);
    mpYLmaxSpinBox->setDecimals(10);
    mpYLmaxSpinBox->setSingleStep(0.1);

    mpYRminSpinBox = new QDoubleSpinBox(mpSetAxisDialog);
    mpYRminSpinBox->setRange(-DoubleMax, DoubleMax);
    mpYRminSpinBox->setDecimals(10);
    mpYRminSpinBox->setSingleStep(0.1);

    mpYRmaxSpinBox = new QDoubleSpinBox(mpSetAxisDialog);
    mpYRmaxSpinBox->setRange(-DoubleMax, DoubleMax);
    mpYRmaxSpinBox->setDecimals(10);
    mpYRmaxSpinBox->setSingleStep(0.1);

    QPushButton *pFinishedButton = new QPushButton("Done", mpSetAxisDialog);
    QDialogButtonBox *pFinishedButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pFinishedButtonBox->addButton(pFinishedButton, QDialogButtonBox::ActionRole);

    QGridLayout *pAxisLimitsDialogLayout = new QGridLayout(mpSetAxisDialog);

    int r=0,c=0;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("X Axis")),r,c,1,2, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("min")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpXminSpinBox, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("max")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpXmaxSpinBox, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpXAutoCheckBox, r, c, 1, 2, Qt::AlignCenter);
    pAxisLimitsDialogLayout->setColumnMinimumWidth(c+2, 20);

    r=0;c=3;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("Left Y Axis")),r,c,1,2, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("min")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYLminSpinBox, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("max")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYLmaxSpinBox, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpYLAutoCheckBox, r, c, 1, 2, Qt::AlignCenter);
    pAxisLimitsDialogLayout->setColumnMinimumWidth(c+2, 20);

    r=0;c=6;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("Right Y Axis")),r,c,1,2, Qt::AlignCenter);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("min")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYRminSpinBox, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(new QLabel(tr("max")), r, c);
    pAxisLimitsDialogLayout->addWidget(mpYRmaxSpinBox, r, c+1);
    ++r;
    pAxisLimitsDialogLayout->addWidget(mpYRAutoCheckBox, r, c, 1, 2, Qt::AlignCenter);

    r=4;c=7;
    pAxisLimitsDialogLayout->addWidget(pFinishedButtonBox, r, c, 1, 2);

    mpSetAxisDialog->setLayout(pAxisLimitsDialogLayout);

    connect(mpXminSpinBox,      SIGNAL(valueChanged(double)),   this,           SLOT(applyAxisSettings()));
    connect(mpXmaxSpinBox,      SIGNAL(valueChanged(double)),   this,           SLOT(applyAxisSettings()));
    connect(mpYLminSpinBox,     SIGNAL(valueChanged(double)),   this,           SLOT(applyAxisSettings()));
    connect(mpYLmaxSpinBox,     SIGNAL(valueChanged(double)),   this,           SLOT(applyAxisSettings()));
    connect(mpYRminSpinBox,     SIGNAL(valueChanged(double)),   this,           SLOT(applyAxisSettings()));
    connect(mpYRmaxSpinBox,     SIGNAL(valueChanged(double)),   this,           SLOT(applyAxisSettings()));
    connect(mpXAutoCheckBox,    SIGNAL(toggled(bool)),          this,           SLOT(applyAxisSettings()));
    connect(mpYLAutoCheckBox,   SIGNAL(toggled(bool)),          this,           SLOT(applyAxisSettings()));
    connect(mpYRAutoCheckBox,   SIGNAL(toggled(bool)),          this,           SLOT(applyAxisSettings()));
    connect(pFinishedButton,    SIGNAL(clicked()),              mpSetAxisDialog, SLOT(close()));

}

//! @brief Help function to set legend symbole style
//! @todo allways sets for all curves, maybe should only set for one
void PlotTab::setLegendSymbol(const QString symStyle)
{
    for(int j=0; j<mPlotCurvePtrs[FirstPlot].size(); ++j)
    {
        mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendNoAttribute, false);
        mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowLine, false);
        mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowSymbol, false);
        mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowBrush, false);

        if( symStyle == "Rectangle")
        {
            mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendNoAttribute, true);
        }
        else if( symStyle == "Line")
        {
            mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowLine, true);
        }
        else if( symStyle == "Symbol")
        {
            mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowSymbol, true);
         }
        else if ( symStyle == "Line&Symbol")
        {
            mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowLine, true);
            mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowSymbol, true);
        }
        else if( symStyle == "Brush")
        {
            mPlotCurvePtrs[FirstPlot].at(j)->setLegendAttribute( PlotCurve::LegendShowBrush, true);
        }

        // Fix legend size after possible change in style
        mPlotCurvePtrs[FirstPlot].at(j)->resetLegendSize();
    }
}

void PlotTab::setTabOnlyCustomXVector(SharedLogVariableDataPtrT pData, HopsanPlotIDEnumT /*plotID*/)
{
    mHasCustomXData = true;
    mpCustomXData = pData;
    mCustomXDataLabel = QString(mpCustomXData->getDataName() + " [" + mpCustomXData->getDataUnit() + "]");

    updateLabels();
    update();
    mpParentPlotWindow->mpResetXVectorButton->setEnabled(true);
}


QSizeF PlotTab::calcMMSize() const
{
    QSizeF pxSize = calcPXSize();
    const double pxToMM = 1.0/mpImageDPI->value()*in2mm ;
    return QSizeF(pxSize.width()*pxToMM,pxSize.height()*pxToMM);
}

QSizeF PlotTab::calcPXSize(QString unit) const
{
    if (unit.isEmpty())
    {
        unit = mpImageDimUnit->currentText();
    }

    QSizeF pxSize;
    if ( unit == "px")
    {
        pxSize = QSizeF(mpImageSetWidth->value(), mpImageSetHeight->value());
    }
    else if (unit == "mm")
    {
        const double mmToPx = 1.0/in2mm * mpImageDPI->value();
        pxSize = QSizeF(mpImageSetWidth->value()*mmToPx,mpImageSetHeight->value()*mmToPx);
    }
    else if (unit == "cm")
    {
        const double cmToPx = 10.0/in2mm * mpImageDPI->value();
        pxSize = QSizeF(mpImageSetWidth->value()*cmToPx,mpImageSetHeight->value()*cmToPx);
    }
    else if (unit == "in")
    {
        pxSize = QSizeF(mpImageSetWidth->value()*mpImageDPI->value(), mpImageSetHeight->value()*mpImageDPI->value());
    }

    //! @todo round to int, ceil or floor, handle truncation
    return pxSize;
}

void PlotTab::updateGraphicsExportSizeEdits()
{
    QSizeF newSize;
    mpImageDPI->setDisabled(false);
    if (mpImageDimUnit->currentText() == "px")
    {
        newSize.setWidth(round(mImagePixelSize.width()));
        newSize.setHeight(round(mImagePixelSize.height()));
        mpImageSetWidth->setDecimals(0);
        mpImageSetHeight->setDecimals(0);
        mpImageDPI->setDisabled(true);
    }
    else if (mpImageDimUnit->currentText() == "mm")
    {
        const double px2mm = 1.0/mpImageDPI->value()*in2mm;
        newSize = mImagePixelSize*px2mm;
        mpImageSetWidth->setDecimals(2);
        mpImageSetHeight->setDecimals(2);
    }
    else if (mpImageDimUnit->currentText() == "cm")
    {
        const double px2cm = 1.0/(10*mpImageDPI->value())*in2mm;
        newSize = mImagePixelSize*px2cm;
        mpImageSetWidth->setDecimals(3);
        mpImageSetHeight->setDecimals(3);
    }
    else if (mpImageDimUnit->currentText() == "in")
    {
        const double px2in = 1.0/(mpImageDPI->value());
        newSize = mImagePixelSize*px2in;
        mpImageSetWidth->setDecimals(3);
        mpImageSetHeight->setDecimals(3);
    }

    mpImageSetWidth->setValue(newSize.width());
    mpImageSetHeight->setValue(newSize.height());
}

void PlotTab::rescaleAxistoIncludeLegendBufferOffset(const int plotId, const QwtPlot::Axis axisId, const AxisLimitsT &rLegendBufferOffset, AxisLimitsT &rAxisLimits)
{
    //! @todo only works for top buffer right now
    if(dynamic_cast<QwtLogScaleEngine*>(mpQwtPlots[plotId]->axisScaleEngine(axisId)))
    {
        //! @todo what shoul happen here ?
        //                double leftlegendheigh = mpLeftPlotLegend->geometry(mpQwtPlots[plotID]->geometry()).height();
        //                double rightlegendheigh = mpRightPlotLegend->geometry(mpQwtPlots[plotID]->geometry()).height();
        //                bufferoffset = max(leftlegendheigh,rightlegendheigh);
        //                double rheight = mpQwtPlots[plotID]->axisWidget(QwtPlot::yRight)->size().height();
        //                double rinterval = mpQwtPlots[plotID]->axisInterval(QwtPlot::yRight).width();
        //                //                double rscale = rinterval/rheight; //change
        //                double lheight = mpQwtPlots[plotID]->axisWidget(QwtPlot::yLeft)->size().height();
        //                double linterval = mpQwtPlots[plotID]->axisInterval(QwtPlot::yLeft).width();
        //                double lscale = linterval/lheight;
        //                heightLeft = 0;
        //                yMaxLeft = yMaxLeft*2.0;
        //                yMinLeft = yMinLeft/2.0;
        //                heightRight = 0;
        //                yMaxRight = yMaxRight*2.0;
        //                yMinRight = yMinRight/2.0;
        //                double bufferoffsetL =  bufferoffset*lscale;//marginss1;
        //                //                double bufferoffsetR =  bufferoffset*rscale;//marginss2;
        //                mpQwtPlots[plotID]->setAxisScale(QwtPlot::yLeft, yMinLeft-0.05*heightLeft, yMaxLeft+0.05*heightLeft+bufferoffsetL);
        //                mAxisLimits[plotID].yLMin = yMinLeft-0.05*heightLeft;
        //                mAxisLimits[plotID].yLMax =  yMaxLeft+0.05*heightLeft+bufferoffsetL;
        //                //                mpQwtPlots[plotID]->setAxisScale(QwtPlot::yRight, yMinRight-0.05*heightRight, yMaxRight+0.05*heightRight+bufferoffsetR);
        //                //                mAxisLimits[plotID].yRMin = yMinRight-0.05*heightRight;
        //                //                mAxisLimits[plotID].yRMax = yMaxRight+0.05*heightRight+bufferoffsetR;
        //mpQwtPlots[plotId]->setAxisScale(axisId, rAxisLimits.min, rAxisLimits.max);
        mpQwtPlots[plotId]->setAxisAutoScale(axisId, true);
    }
    else
    {
        const double axis_height = mpQwtPlots[plotId]->axisWidget(axisId)->size().height();
        const double axis_interval = mpQwtPlots[plotId]->axisInterval(axisId).width();
        const double scale = axis_interval/axis_height;

        rAxisLimits.min += scale*rLegendBufferOffset.min;
        rAxisLimits.max += scale*rLegendBufferOffset.max;
        mpQwtPlots[plotId]->setAxisScale(axisId, rAxisLimits.min, rAxisLimits.max);
    }
}

void PlotTab::calculateLegendBufferOffsets(const int plotId, PlotTab::AxisLimitsT &rLeftLegendOffset, PlotTab::AxisLimitsT &rRightLegendOffset)
{
    //! @todo only works for linear scale right now, need to check for log scale also
    double leftLegendHeight = mpLeftPlotLegend->geometry(mpQwtPlots[plotId]->geometry()).height() + mpLeftPlotLegend->margin();
    double rightLegendHeight = mpRightPlotLegend->geometry(mpQwtPlots[plotId]->geometry()).height()  + mpRightPlotLegend->margin();

    if(mpLegendsAutoOffsetCheckBox->isChecked())
    {
        if ((mpLegendLPosition->currentText() == mpLegendRPosition->currentText()) && (mpLegendRPosition->currentText() == "Top"))
        {
            rLeftLegendOffset.max = rRightLegendOffset.max = qMax(leftLegendHeight,rightLegendHeight);
            rLeftLegendOffset.min = rRightLegendOffset.max = 0;
        }
        if ((mpLegendLPosition->currentText() == mpLegendRPosition->currentText()) && (mpLegendRPosition->currentText() == "Bottom"))
        {
            rLeftLegendOffset.max = rRightLegendOffset.max = 0;
            rLeftLegendOffset.min = rRightLegendOffset.min = qMax(leftLegendHeight,rightLegendHeight);
        }
        else if (mpLegendLPosition->currentText() == "Bottom")
        {
            rLeftLegendOffset.min = leftLegendHeight;
            rRightLegendOffset.max = rightLegendHeight;
            rLeftLegendOffset.max = rRightLegendOffset.min = 0;
        }
        else if (mpLegendRPosition->currentText() == "Bottom")
        {
            rRightLegendOffset.min = rightLegendHeight;
            rLeftLegendOffset.max = leftLegendHeight;
            rRightLegendOffset.max = rLeftLegendOffset.min = 0;
        }
    }
    else
    {
        if (mpLegendLPosition->currentText() == "Top")
        {
            rLeftLegendOffset.max = mpLegendLeftOffset->value()*leftLegendHeight;
            rLeftLegendOffset.min = 0;
        }
        else if (mpLegendLPosition->currentText() == "Bottom")
        {
            rLeftLegendOffset.max = 0;
            rLeftLegendOffset.min = mpLegendLeftOffset->value()*leftLegendHeight;
        }
        //! @todo Center? than what to do

        if (mpLegendRPosition->currentText() == "Top")
        {
            rRightLegendOffset.max = mpLegendRightOffset->value()*rightLegendHeight;
            rRightLegendOffset.min = 0;
        }
        else if (mpLegendRPosition->currentText() == "Bottom")
        {
            rRightLegendOffset.max = 0;
            rRightLegendOffset.min = mpLegendRightOffset->value()*rightLegendHeight;
        }
        //! @todo Center? than what to do
    }
}


//! @brief Defines what happens when used drags something into the plot window
void PlotTab::dragEnterEvent(QDragEnterEvent *event)
{
    // Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    if (event->mimeData()->hasText())
    {
        // Create the hover rectangle (size will be changed by dragMoveEvent)
        mpPainterWidget->createRect(0,0,this->width(), this->height());

        event->acceptProposedAction();
    }
}


//! @brief Defines what happens when user is dragging something in the plot window.
void PlotTab::dragMoveEvent(QDragMoveEvent *event)
{
    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    QCursor cursor;
    if(this->mapFromGlobal(cursor.pos()).y() > getPlot()->canvas()->height()*2.0/3.0+getPlot()->canvas()->y()+10 && getNumberOfCurves(FirstPlot) >= 1)
    {
        mpPainterWidget->createRect(getPlot()->canvas()->x(), getPlot()->canvas()->height()*2.0/3.0+getPlot()->canvas()->y(), getPlot()->canvas()->width(), getPlot()->canvas()->height()*1.0/3.0);
        mpParentPlotWindow->showHelpPopupMessage("Replace X-axis with selected variable.");
    }
    else if(this->mapFromGlobal(cursor.pos()).x() < getPlot()->canvas()->x()+9 + getPlot()->canvas()->width()/2)
    {
        mpPainterWidget->createRect(getPlot()->canvas()->x(), getPlot()->canvas()->y(), getPlot()->canvas()->width()/2, getPlot()->canvas()->height());
        mpParentPlotWindow->showHelpPopupMessage("Add selected variable to left Y-axis.");
    }
    else
    {
        mpPainterWidget->createRect(getPlot()->canvas()->x() + getPlot()->canvas()->width()/2, getPlot()->canvas()->y(), getPlot()->canvas()->width()/2, getPlot()->canvas()->height());
        mpParentPlotWindow->showHelpPopupMessage("Add selected variable to right Y-axis.");
    }
    QWidget::dragMoveEvent(event);
}


//! @brief Defines what happens when user drags something out from the plot window.
void PlotTab::dragLeaveEvent(QDragLeaveEvent *event)
{
    mpPainterWidget->clearRect();

    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    QWidget::dragLeaveEvent(event);
}


//! @brief Defines what happens when user drops something in the plot window
void PlotTab::dropEvent(QDropEvent *event)
{
    QWidget::dropEvent(event);

    mpPainterWidget->clearRect();

    //Don't accept drag events to FFT and Bode plots
    if(mPlotCurvePtrs[0].size() > 0 && mPlotCurvePtrs[0][0]->getCurveType() != PortVariableType) return;

    if (event->mimeData()->hasText())
    {
        QString mimeText = event->mimeData()->text();
        if(mimeText.startsWith("HOPSANPLOTDATA:"))
        {
            qDebug() << mimeText;
            mimeText.remove("HOPSANPLOTDATA:");

            QCursor cursor;
            if(this->mapFromGlobal(cursor.pos()).y() > getPlot()->canvas()->height()*2.0/3.0+getPlot()->canvas()->y()+10 && getNumberOfCurves(FirstPlot) >= 1)
            {
//                pNewDesc->mDataUnit = gConfig.getDefaultUnit(pNewDesc->mDataName);
//                setCustomXVector(gpMainWindow->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->getPlotDataValues(desc->getFullName(), -1), pNewDesc );
                setCustomXVectorForAll(gpMainWindow->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->getPlotData(mimeText, -1));
                //! @todo do we need to reset to default unit too ?
            }
            else if(this->mapFromGlobal(cursor.pos()).x() < getPlot()->canvas()->x()+9 + getPlot()->canvas()->width()/2)
            {
                mpParentPlotWindow->addPlotCurve(gpMainWindow->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->getPlotData(mimeText, -1), QwtPlot::yLeft);
            }
            else
            {
                mpParentPlotWindow->addPlotCurve(gpMainWindow->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->getPlotData(mimeText, -1), QwtPlot::yRight);
            }
        }
    }
}


//! @brief Handles the right-click menu in the plot tab
void PlotTab::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget::contextMenuEvent(event);

    //   return;
    if(this->mpZoomerLeft[FirstPlot]->isEnabled())
    {
        return;
    }

    QMenu menu;

    QMenu *yAxisRightMenu;
    QMenu *yAxisLeftMenu;
    QMenu *changeUnitsMenu;
    QMenu *insertMarkerMenu;

    QAction *setRightAxisLogarithmic = 0;
    QAction *setLeftAxisLogarithmic = 0;


    yAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    yAxisRightMenu = menu.addMenu(QString("Right Y Axis"));

    yAxisLeftMenu->setEnabled(mpQwtPlots[FirstPlot]->axisEnabled(QwtPlot::yLeft));
    yAxisRightMenu->setEnabled(mpQwtPlots[FirstPlot]->axisEnabled(QwtPlot::yRight));

    //Create menu and actions for changing units
    changeUnitsMenu = menu.addMenu(QString("Change Units"));
    QMap<QAction *, PlotCurve *> actionToCurveMap;
    QMap<QString, double> unitMap;
    QList<PlotCurve *>::iterator itc;
    QMap<QString, double>::iterator itu;
    for(itc=mPlotCurvePtrs[FirstPlot].begin(); itc!=mPlotCurvePtrs[FirstPlot].end(); ++itc)
    {
        QMenu *pTempMenu = changeUnitsMenu->addMenu(QString((*itc)->getComponentName() + ", " + (*itc)->getPortName() + ", " + (*itc)->getDataName()));
        unitMap = gConfig.getCustomUnits((*itc)->getDataName());
        for(itu=unitMap.begin(); itu!=unitMap.end(); ++itu)
        {
            QAction *pTempAction = pTempMenu->addAction(itu.key());
            actionToCurveMap.insert(pTempAction, (*itc));
        }
    }


    //Create actions for making axis logarithmic
    if(mpQwtPlots[FirstPlot]->axisEnabled(QwtPlot::yLeft))
    {
        setLeftAxisLogarithmic = yAxisLeftMenu->addAction("Logarithmic Scale");
        setLeftAxisLogarithmic->setCheckable(true);
        setLeftAxisLogarithmic->setChecked(mLeftAxisLogarithmic);
    }
    if(mpQwtPlots[FirstPlot]->axisEnabled(QwtPlot::yRight))
    {
        setRightAxisLogarithmic = yAxisRightMenu->addAction("Logarithmic Scale");
        setRightAxisLogarithmic->setCheckable(true);
        setRightAxisLogarithmic->setChecked(mRightAxisLogarithmic);
    }


    //Create menu for inserting curve markers
    insertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    for(int plotID=0; plotID<2; ++plotID)
    {
        for(itc=mPlotCurvePtrs[plotID].begin(); itc!=mPlotCurvePtrs[plotID].end(); ++itc)
        {
            QAction *pTempAction = insertMarkerMenu->addAction((*itc)->getCurveName());
            actionToCurveMap.insert(pTempAction, (*itc));
        }
    }



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
    if(selectedAction->parentWidget()->parentWidget() == changeUnitsMenu)
    {
        actionToCurveMap.find(selectedAction).value()->setCustomDataUnit(selectedAction->text());
    }


    //Make axis logarithmic
    if (selectedAction == setRightAxisLogarithmic)
    {
        mRightAxisLogarithmic = !mRightAxisLogarithmic;
        if(mRightAxisLogarithmic)
        {
            mpQwtPlots[FirstPlot]->setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine(10));
            rescaleAxesToCurves();
            mpQwtPlots[FirstPlot]->replot();
            mpQwtPlots[FirstPlot]->updateGeometry();
        }
        else
        {
            mpQwtPlots[FirstPlot]->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
            rescaleAxesToCurves();
            mpQwtPlots[FirstPlot]->replot();
            mpQwtPlots[FirstPlot]->updateGeometry();
        }
    }
    else if (selectedAction == setLeftAxisLogarithmic)
    {
        mLeftAxisLogarithmic = !mLeftAxisLogarithmic;
        if(mLeftAxisLogarithmic)
        {
            qDebug() << "Logarithmic!";
            mpQwtPlots[FirstPlot]->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
            rescaleAxesToCurves();
            mpQwtPlots[FirstPlot]->replot();
            mpQwtPlots[FirstPlot]->updateGeometry();
        }
        else
        {
            qDebug() << "Linear!";
            mpQwtPlots[FirstPlot]->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
            rescaleAxesToCurves();
            mpQwtPlots[FirstPlot]->replot();
            mpQwtPlots[FirstPlot]->updateGeometry();
        }
    }


    //Insert curve marker
    if(selectedAction->parentWidget() == insertMarkerMenu)
    {
        insertMarker(actionToCurveMap.find(selectedAction).value(), event->pos());
    }

}


//! @brief Constructor for painter widget, used for painting transparent rectangles when dragging things to plot tabs
PainterWidget::PainterWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}


//! @brief Creates a rectangle with specified dimensions
void PainterWidget::createRect(int x, int y, int w, int h)
{
    mX = x;
    mY = y;
    mWidth = w;
    mHeight = h;
    mErase=false;
    update();
}


//! @brief Removes any previously drawn rectangles
void PainterWidget::clearRect()
{
    mErase=true;
    update();
}


//! @brief Paint event, does the actual drawing
void PainterWidget::paintEvent(QPaintEvent *)
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


void HopQwtPlot::resizeEvent(QResizeEvent *e)
{
    QwtPlot::resizeEvent(e);
    emit resized();
}


HopQwtPlot::HopQwtPlot(QWidget *pParent) : QwtPlot(pParent)
{
    // Nothing special
}
