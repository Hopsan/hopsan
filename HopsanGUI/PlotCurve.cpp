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
//! @file   PlutCurve.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010
//!
//! @brief Contains a class for plot curves
//!
//$Id$

//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "ModelHandler.h"
#include "PlotCurve.h"
#include "PlotTab.h"
#include "PlotArea.h"
#include "PlotWindow.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Utilities/GUIUtilities.h"
#include "LogDataHandler2.h"

//Other includes
#include <limits>
#include <qwt_plot_zoomer.h>
#include <QColorDialog>
#include <QDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>


const double DoubleMax = std::numeric_limits<double>::max();


class AlignmentSelectionStruct
{
public:
    AlignmentSelectionStruct(const Qt::Alignment alignment, const QString &label)
    {
        mAlignment = alignment;
        mLabel = label;
    }
    Qt::Alignment mAlignment;
    QString mLabel;
};

class SymbolSelectionStruct
{
public:
    SymbolSelectionStruct(const QwtSymbol::Style style, const QString &label)
    {
        mStyle = style;
        mLabel = label;
    }
    QwtSymbol::Style mStyle;
    QString mLabel;
};


//! @brief Constructor for plot curves.
//! @param pData A shared pointer to the data to plot
//! @param curveType The type of the curve (controls the name and some other special things)
//! @todo why is the axis in the curve constructor, it would make more sense if the axis is specified when adding a curve to a plot area /Peter
PlotCurve::PlotCurve(SharedVectorVariableT data, const QwtPlot::Axis axisY, const HopsanPlotCurveTypeEnumT curveType)
    : QObject(), QwtPlotCurve()
{
    mpParentPlotArea = 0;
    mHaveCustomData = false;
    mShowVsSamples = false;
    mData = data;

    mCurveExtraDataScale = 1.0;
    mCurveExtraDataOffset = 0.0;
    mCurveTFOffset = 0.0;

    mpCurveSymbol = 0;
    mCurveSymbolSize = 8;
    mIsActive = false;
    mIncludeGenInTitle = true;
    mIncludeSourceInTitle = false;
    mCurveType = curveType;

    mAxisY = axisY;

    // We do not want imported data to auto refresh, in case the data name is the same as something from the model (alias)
    if (data->isImported())
    {
        mAutoUpdate = false;
    }
    else
    {
        mAutoUpdate = true;
    }

    // Set QwtPlotCurve stuff
    //! @todo maybe this code should be run when we are adding a curve to a plottab
    refreshCurveTitle();
    updateCurve();
    this->setYAxis(axisY);
    this->setItemAttribute(QwtPlotItem::Legend, true);

    if(curveType != PortVariableType)
    {
        setAutoUpdate(false);
    }

    // Create relay connections
    connect(this, SIGNAL(curveDataUpdated()), this, SIGNAL(curveInfoUpdated()));
    // Create data connections
    connectDataSignals();

    if (mData->getLogDataHandler())
    {
        mData->getLogDataHandler()->incrementOpenPlotCurves();
    }
}

void PlotCurve::refreshCurveTitle()
{
    setTitle(getCurveName(mIncludeGenInTitle, mIncludeSourceInTitle));
}

//! @brief Destructor for plot curves
PlotCurve::~PlotCurve()
{
    // If the curve data had a data handler then decrement its open curves counter
    LogDataHandler2* pDataHandler = mData->getLogDataHandler();
    if (pDataHandler)
    {
        pDataHandler->decrementOpenPlotCurves();
    }

    // Delete custom data if any
    deleteCustomData();
}

void PlotCurve::setIncludeGenerationInTitle(bool doit)
{
    mIncludeGenInTitle=doit;
}

void PlotCurve::setIncludeSourceInTitle(bool doit)
{
    mIncludeSourceInTitle=doit;
}


//! @brief Returns the current generation a plot curve is representing
int PlotCurve::getGeneration() const
{
    return mData->getGeneration();
}

QString PlotCurve::getCurveName() const
{
    if (mData->hasCustomLabel())
    {
        return mData->getCustomLabel();
    }
    else if (mData->hasAliasName())
    {
        return mData->getAliasName();
    }
    else
    {
        return mData->getFullVariableNameWithSeparator(", ");
    }
}

QString PlotCurve::getCurveName(bool includeGeneration, bool includeSourceFile) const
{
    QString name = getCurveName();
    if (includeGeneration)
    {
        name.append(QString("  (%1)").arg(mData->getGeneration()+1));
    }
    if (includeSourceFile)
    {
        QString source;
        if (mData->isImported())
        {
            source = mData->getImportedFileName();
        }
        else
        {
            source = mData->getModelPath();
        }

        if (!source.isEmpty())
        {
            QFileInfo file(source);
            name.append(QString("    %1").arg(file.fileName()));
        }
    }
    return name;
}


//! @brief Returns the type of the curve
HopsanPlotCurveTypeEnumT PlotCurve::getCurveType()
{
    return mCurveType;
}


//! @brief Returns the name of the component a plot curve is created from
const QString &PlotCurve::getComponentName() const
{
    return mData->getComponentName();
}


//! @brief Returns the name of the port a plot curve is created from
const QString &PlotCurve::getPortName() const
{
    return mData->getPortName();
}


//! @brief Returns the data name (physical quantity) of a plot curve
const QString &PlotCurve::getDataName() const
{
    return mData->getDataName();
}


//! @brief Returns the original data unit of a plot curve
const QString &PlotCurve::getDataUnit() const
{
    return mData->getDataUnit();
}

const QString &PlotCurve::getDataQuantity() const
{
    return mData->getDataQuantity();
}


//! @brief Returns the current unit of a plot curve in the following priority (Local unit, Data unit or Original unit)
QString PlotCurve::getCurrentPlotUnit() const
{
    QString localScale = QString::number(mCurveExtraDataScale);
    UnitScale us = getCurveDataUnitScale();
    if (!us.isEmpty())
    {
        if (localScale != "1")
        {
            return QString("%1 * %2").arg(localScale).arg(us.mUnit);
        }
        else
        {
            return us.mUnit;
        }
    }
    return "";
}

QString PlotCurve::getCurrentXPlotUnit() const
{
    //QString localScale = QString::number(mCurveExtraDataScale);
    UnitScale us = getCurveXDataUnitScale();
    if (!us.isEmpty())
    {
//        if (localScale != "1")
//        {
//            return QString("%1 * %2").arg(localScale).arg(us.mUnit);
//        }
//        else
//        {
            return us.mUnit;
//        }
    }
    return "";
}

VariableSourceTypeT PlotCurve::getDataSource() const
{
    return mData->getVariableSourceType();
}

bool PlotCurve::hasCurveDataUnitScale() const
{
    return !mCurveDataUnitScale.isEmpty();
}

const QString &PlotCurve::getDataModelPath() const
{
    return mData->getModelPath();
}

QString PlotCurve::getDataFullName() const
{
    return mData->getFullVariableName();
}

QString PlotCurve::getDataSmartName() const
{
    return mData->getSmartName();
}


const SharedVectorVariableT PlotCurve::getSharedVectorVariable() const
{
    return mData;
}


//! @brief Tells which Y-axis a plot curve is assigned to
int PlotCurve::getAxisY()
{
    return mAxisY;
}

PlotArea *PlotCurve::getParentPlotArea() const
{
    return mpParentPlotArea;
}


//! @brief Returns the (unscaled) data vector of a plot curve
QVector<double> PlotCurve::getVariableDataCopy() const
{
    //! @todo this is no longer a reference need to see where it was used to avoid REALLY slow code fetching data all the time /Peter
    return mData->getDataVectorCopy();
}

//! @brief Returns the minimum and maximum value of the curve (for values higher then 0)
//! @details values <= 0 are ignored
bool PlotCurve::minMaxPositiveNonZeroYValues(double &rMin, double &rMax)
{
    int imax, imin;
    return mData->positiveNonZeroMinMaxOfData(rMin, rMax, imin, imax);
}

bool PlotCurve::minMaxPositiveNonZeroXValues(double &rMin, double &rMax)
{
    int imax, imin;
    if (mCustomXdata)
    {
        return mCustomXdata->positiveNonZeroMinMaxOfData(rMin, rMax, imin, imax);
    }
    else if (mData->getSharedTimeOrFrequencyVector())
    {
        return mData->getSharedTimeOrFrequencyVector()->positiveNonZeroMinMaxOfData(rMin, rMax, imin, imax);
    }
    else
    {
        rMin = 0;
        rMax = mData->getDataSize()-1;
        return (rMax > -1);
    }
}


//! @brief Returns the shared time or frequency vector of the plot curve
//! This returns the TIME vector, NOT any special X-axes if they are used.
const SharedVectorVariableT PlotCurve::getSharedTimeOrFrequencyVariable() const
{
    return mData->getSharedTimeOrFrequencyVector();
}

bool PlotCurve::hasCustomXData() const
{
    return !mCustomXdata.isNull();
}

bool PlotCurve::getShowVsSamples() const
{
    return mShowVsSamples;
}

const SharedVectorVariableT PlotCurve::getSharedCustomXVariable() const
{
    return mCustomXdata;
}


//! @brief Sets the generation of a plot curve
//! Updates the data to specified generation, and updates plot info box.
//! @param generation Generation to use
bool PlotCurve::setGeneration(const int generation)
{
    if(mData)
    {
        SharedVectorVariableT pNewData = switchVariableGeneration(mData, generation);
        if (pNewData)
        {
            if (hasCustomXData())
            {
                SharedVectorVariableT pNewXData = switchVariableGeneration(mCustomXdata, generation);
                if (pNewXData)
                {
                    disconnectDataSignals();
                    mData = pNewData;
                    connectDataSignals();
                    setCustomXData(pNewXData);
                }
                // If we cant switch the custom X data then the swithc shold not be allowed
                else
                {
                    return false;
                }
            }
            else
            {
                disconnectDataSignals();
                mData = pNewData;
                connectDataSignals();
            }
        }
        else
        {
            return false;
        }

        updateCurve();
        refreshCurveTitle();

        //! @todo should this be done here
        mpParentPlotArea->resetZoom();

        return true;
    }
    return false;
}


//! @brief Sets the unit of a plot curve
//! @details The physical quantity will be checked, if it does not match the current unit, the new unit will be ignored
//! @param[in] rUnit Name of new unit
//! @note If unit is not registered for data then nothing will happen
void PlotCurve::setCurveDataUnitScale(const QString &rUnit)
{
    QString dataQuantity = getDataQuantity();

    // For varaibles without quantity, try to lookup, but only unique quantities
    if (dataQuantity.isEmpty())
    {
        // Only set the new unit if it represents the same physical quantity as the current unit
        QStringList pqs = gpConfig->getQuantitiesForUnit(rUnit);
        QStringList pqsOrg = gpConfig->getQuantitiesForUnit(getDataUnit());
        if ( !(pqs.isEmpty() || pqsOrg.isEmpty()) )
        {
            if (pqs.front() == pqsOrg.front())
            {
                UnitScale us;
                gpConfig->getUnitScale(pqs.first(), rUnit, us);
                setCurveDataUnitScale(us);
            }
        }
    }
    // If quantity known then make sure that unit is actually a valid unit for that quantity
    else
    {
        // Check so that this unit is relevant for this type of data (datname). Else it will be ignored
        if (gpConfig->hasUnitScale(dataQuantity,rUnit))
        {
            UnitScale us;
            gpConfig->getUnitScale(dataQuantity, rUnit, us);
            setCurveDataUnitScale(us);
        }
    }
}

void PlotCurve::setCurveDataUnitScale(const UnitScale &rUS)
{
    if (rUS.isEmpty())
    {
        resetCurveDataUnitScale();
    }
    else
    {
        mCurveDataUnitScale = rUS;

        // Clear the custom scale if it is one and we have a data unit
        if (!getDataUnit().isEmpty() && mCurveDataUnitScale.isOne())
        {
            resetCurveDataUnitScale();
        }
        else
        {
            updateCurve();
            //! @todo shouldn't these be triggered by signal in update curve?
            mpParentPlotArea->replot();
        }
    }
}

const UnitScale PlotCurve::getCurveDataUnitScale() const
{
    if (mCurveDataUnitScale.isEmpty())
    {
        // If data have an original unit then return that as a unit scale with scaling 1.0
        if (!mData->getDataUnit().isEmpty())
        {
            return UnitScale(mData->getDataName(), mData->getDataUnit(), "1.0");
        }
        // If not then return empty
        else
        {
            return UnitScale();
        }
    }
    // Return the custom unitscale
    else
    {
        return mCurveDataUnitScale;
    }
}

void PlotCurve::resetCurveDataUnitScale()
{
    mCurveDataUnitScale.clear();
    updateCurve();

    //! @todo shouldn't these be triggered by signal in update curve?
    mpParentPlotArea->replot();
}

bool PlotCurve::hasCurveXDataUnitScale() const
{
    return !mCurveXDataUnitScale.isEmpty();
}

void PlotCurve::setCurveXDataUnitScale(const QString &rUnit)
{
    if (mCustomXdata)
    {
        // For varaibles without quantity, try to lookup, but only allow unique quantities
        QString xDataQuantity = mCustomXdata->getDataQuantity();
        if (xDataQuantity.isEmpty())
        {
            // Only set the new unit if it represents the same physical quantity as the current unit
            QStringList pqs = gpConfig->getQuantitiesForUnit(rUnit);
            QStringList pqsOrg = gpConfig->getQuantitiesForUnit(mCustomXdata->getDataUnit());
            if ( !(pqs.isEmpty() || pqsOrg.isEmpty()) )
            {
                if (pqs.front() == pqsOrg.front())
                {
                    UnitScale us;
                    gpConfig->getUnitScale(pqs.first(), rUnit, us);
                    setCurveXDataUnitScale(us);
                }
            }
        }
        // If quantity known then make sure that unit is actually a valid unit for that quantity
        else
        {
            // Check so that this unit is relevant for this type of data (datname). Else it will be ignored
            if (gpConfig->hasUnitScale(xDataQuantity,rUnit))
            {
                UnitScale us;
                gpConfig->getUnitScale(xDataQuantity, rUnit, us);
                setCurveXDataUnitScale(us);
            }
        }
    }
}

void PlotCurve::setCurveXDataUnitScale(const UnitScale &rUS)
{
    if (rUS.isEmpty())
    {
        resetCurveXDataUnitScale();
    }
    else
    {
        mCurveXDataUnitScale = rUS;

        // Clear the custom scale if it is one and we have a data unit
        if (!mCustomXdata->getDataUnit().isEmpty() && mCurveXDataUnitScale.isOne())
        {
            resetCurveXDataUnitScale();
        }
        else
        {
            updateCurve();
            //! @todo shouldn't these be triggered by signal in update curve?
            mpParentPlotArea->replot();
        }
    }
}

const UnitScale PlotCurve::getCurveXDataUnitScale() const
{
    if (mCurveXDataUnitScale.isEmpty())
    {
        if (mCustomXdata)
        {
            // If data have an original unit then return that as a unit scale with scaling 1.0
            if (!mCustomXdata->getDataUnit().isEmpty())
            {
                return UnitScale(mData->getDataName(), mData->getDataUnit(), "1.0");
            }
            // If not then return empty below
        }
        return UnitScale();
    }
    // Return the custom unitscale
    else
    {
        return mCurveXDataUnitScale;
    }
}

void PlotCurve::resetCurveXDataUnitScale()
{
    mCurveXDataUnitScale.clear();
    updateCurve();

    //! @todo shouldn't these be triggered by signal in update curve?
    mpParentPlotArea->replot();
}

void PlotCurve::setCurveTFUnitScale(UnitScale us)
{
    mCurveTFUnitScale = us;
    updateCurve();
}

UnitScale PlotCurve::getCurveTFUnitScale() const
{
    if (mCurveTFUnitScale.isEmpty())
    {
        SharedVectorVariableT tfVar = mData->getSharedTimeOrFrequencyVector();
        if (tfVar)
        {
            return UnitScale(tfVar->getDataName(), tfVar->getDataUnit(), "1.0");
        }
        else
        {
            return UnitScale();
        }
    }
    else
    {
        return mCurveTFUnitScale;
    }
}

void PlotCurve::setCurveTFOffset(double offset)
{
    mCurveTFOffset = offset;
    updateCurve();
}

double PlotCurve::getCurveTFOffset() const
{
    return mCurveTFOffset;
}


//! @brief Sets the curve specific time or frequency vector scale and offset
void PlotCurve::setCurveTFUnitScaleAndOffset(const UnitScale &rUS, double offset)
{
    mCurveTFUnitScale = rUS;
    mCurveTFOffset = offset;
    updateCurve();
}

void PlotCurve::setCurveExtraDataScaleAndOffset(const double scale, const double offset)
{
    mCurveExtraDataScale = scale;
    mCurveExtraDataOffset = offset;
    updateCurve();
}


void PlotCurve::setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData)
{
    // First disconnect all signals from the old data
    disconnectDataSignals();

    // If we already have custom data, then delete it from memory as it is being replaced
    deleteCustomData();

    // Create new custom data
    //! @todo we are abusing timedomain variable here
    mData = SharedVectorVariableT(new TimeDomainVariable(createFreeTimeVectorVariabel(rvTime), rvData, 0,
                                                       SharedVariableDescriptionT(new VariableDescription(rVarDesc)), SharedMultiDataVectorCacheT()));
    mHaveCustomData = true;

    // Connect signals
    connectDataSignals();

    updateCurve();
}

void PlotCurve::setCustomXData(const VariableDescription &rVarDesc, const QVector<double> &rvXdata)
{
    setCustomXData(createFreeVectorVariable(rvXdata, SharedVariableDescriptionT(new VariableDescription(rVarDesc))));
}

void PlotCurve::setCustomXData(SharedVectorVariableT data)
{
    //! @todo maybe prevent reset if timevector is null, but then it will (currently) be impossible to reset x vector in curve.

    // Disconnect any signals first, in case we are changing x-data
    disconnectCustomXDataSignals();

    // Make sure generation is same
    data = switchVariableGeneration(data, mData->getGeneration());

    // Set new data and connect signals
    mCustomXdata = data;
    connectCustomXDataSignals();

    // Redraw curve
    updateCurve();

    emit customXDataChanged();
}

void PlotCurve::setCustomXData(const QString fullName)
{
    // If empty then reset time vector
    if (fullName.isEmpty())
    {
        setCustomXData(SharedVectorVariableT());
    }
    else
    {
        LogDataHandler2 *pHandler = mData->getLogDataHandler();
        if (pHandler)
        {
            // Fetch data, generation check is don in the other version of this function
            SharedVectorVariableT data = pHandler->getVectorVariable(fullName, -1);
            if (data)
            {
                setCustomXData(data);
            }
        }
    }
}

void PlotCurve::setShowVsSamples(bool tf)
{
    mShowVsSamples = tf;
    updateCurve();
}

bool PlotCurve::isAutoUpdating() const
{
    return mAutoUpdate;
}

QColor PlotCurve::getLineColor() const
{
    return mLineColor;
}


void PlotCurve::resetLegendSize()
{
    // For now hardcoded but maybe in the future be possible to select, (default 8x8 is to small to see difference between dashed and solid lines)
    setLegendIconSize(QSize(40,12));
}


//! @brief Changes a curve to the previous available model generation
void PlotCurve::gotoPreviousGeneration()
{
    // We do not want to switch generations automatically for curves representing imported data.
    // That would make it very difficult to compare imported data with a model variable of the same name
    // This was decided based on how AC is using the program
    if (mData && !mData->isImported() && mAutoUpdate)
    {
        auto pLDH = mData->getLogDataHandler();
        if (pLDH)
        {
            // Loop until we find next lower generation, abort if gen<0
            int gen = getGeneration()-1;
            while ((gen >= 0) && (gen >= pLDH->getLowestGenerationNumber())  && !setNonImportedGeneration(gen))
            {
                --gen;
            }
        }
    }
}


//! @brief Changes a curve to the next available model generation
void PlotCurve::gotoNextGeneration()
{
    // We do not want to switch generations automatically for curves representing imported data.
    // That would make it very difficult to compare imported data with a model variable of the same name
    // This was decided based on how AC is using the program
    if (mData && !mData->isImported() && mAutoUpdate)
    {
        auto pLDH = mData->getLogDataHandler();
        if (pLDH)
        {
            // Loop until we find next higher generation, abort if we reach the highest
            int gen = getGeneration()+1;
            while ((gen <= pLDH->getHighestGenerationNumber()) && !setNonImportedGeneration(gen))
            {
                ++gen;
            }
        }
    }
}


//! @brief Sets the line width of a plot curve
//! @param lineWidth Line width to give curve
void PlotCurve::setLineWidth(int lineWidth)
{
    mLineWidth = lineWidth;
    QPen tempPen = pen();
    // Add one pt extra width for active curves
    if (mIsActive)
    {
        tempPen.setWidth(lineWidth+1);
    }
    else
    {
        tempPen.setWidth(lineWidth);
    }
    setPen(tempPen);
}


void PlotCurve::setLineStyle(QString lineStyle)
{
    mLineStyle = lineStyle;
    setStyle(PlotCurve::Lines); //Assume we want lines
    QPen tempPen = pen();
    if(lineStyle == "Solid Line")
    {
        tempPen.setStyle(Qt::SolidLine);
    }
    else if(lineStyle == "Dash Line")
    {
        tempPen.setStyle(Qt::DashLine);
    }
    else if(lineStyle == "Dot Line")
    {
        tempPen.setStyle(Qt::DotLine);
    }
    else if(lineStyle == "Dash Dot Line")
    {
        tempPen.setStyle(Qt::DashDotLine);
    }
    else if(lineStyle == "Dash Dot Dot Line")
    {
        tempPen.setStyle(Qt::DashDotDotLine);
    }
    else
    {
        // Deactivate line completely
        setStyle(PlotCurve::NoCurve);
    }
    setPen(tempPen);
    resetLegendSize();
}

void PlotCurve::setLineSymbol(QString lineSymbol)
{
    mLineSymbol = lineSymbol;
    mpCurveSymbol = new QwtSymbol();
    if(lineSymbol == "Cross")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Cross);
    }
    else if(lineSymbol == "XCross")
    {
        mpCurveSymbol->setStyle(QwtSymbol::XCross);
    }
    else if(lineSymbol == "Ellipse")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Ellipse);
    }
    else if(lineSymbol == "Star 1")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Star1);
    }
    else if(lineSymbol == "Star 2")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Star2);
    }
    else if(lineSymbol == "Hexagon")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Hexagon);
    }
    else if(lineSymbol == "Rectangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Rect);
    }
    else if(lineSymbol == "Horizontal Line")
    {
        mpCurveSymbol->setStyle(QwtSymbol::HLine);
    }
    else if(lineSymbol == "Vertical Line")
    {
        mpCurveSymbol->setStyle(QwtSymbol::VLine);
    }
    else if(lineSymbol == "Diamond")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Diamond);
    }
    else if(lineSymbol == "Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Triangle);
    }
    else if(lineSymbol == "Up Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::UTriangle);
    }
    else if(lineSymbol == "Down Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::DTriangle);
    }
    else if(lineSymbol == "Right Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::RTriangle);
    }
    else if(lineSymbol == "Left Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::LTriangle);
    }
    else
    {
        mpCurveSymbol->setStyle(QwtSymbol::NoSymbol);
    }

    QPen tempPen = pen();
    tempPen.setStyle(Qt::SolidLine);
    mpCurveSymbol->setPen(tempPen);
    mpCurveSymbol->setSize(mCurveSymbolSize,mCurveSymbolSize);
    setSymbol(mpCurveSymbol);

    //! @todo Add a color or size picker for the markers
    resetLegendSize();
}

//! @brief Sets the color of a line
//! @brief color Color to give the line.
void PlotCurve::setLineColor(QColor color)
{
    QPen tempPen;
    mLineColor = color;

    // Set line color
    tempPen = pen();
    tempPen.setColor(color);
    setPen(tempPen);

    // Set symbol color, (but only if we have one, else an empty symbols will be created)
    if (mpCurveSymbol)
    {
        // Need to recreate the symbol so that legend will update
        setLineSymbol(mLineSymbol);
    }

    emit colorChanged(color);
}


//! @brief Sets the color of a line
//! @param colorName Svg name of the color
//! @see setLineColor(QColor color)
void PlotCurve::setLineColor(QString colorName)
{
    QColor color;
    if(colorName.isEmpty())
    {
        color = QColorDialog::getColor(pen().color(), mpParentPlotArea);
        if (!color.isValid()) { return; }
    }
    else
    {
        color = QColor(colorName);
    }
    setLineColor(color);
}


//! @brief Opens the scaling dialog for a plot curve
void PlotCurve::openScaleDialog()
{
    QDialog *pScaleDialog = new QDialog(mpParentPlotArea);
    pScaleDialog->setWindowTitle("Change plot-scale and plot-offsets");

    QLabel *pOriginalDataUnit = new QLabel(pScaleDialog);
    QLabel *pCustomDataUnit = new QLabel(pScaleDialog);
    UnitScale us = getCurveDataUnitScale();
    if (mData && !us.isEmpty())
    {
        pOriginalDataUnit->setText(mData->getDataUnit());
        pCustomDataUnit->setText(QString("%1     (%2)").arg(us.mUnit).arg(us.mScale));
    }
    else
    {
        pOriginalDataUnit->setEnabled(false);
        pCustomDataUnit->setEnabled(false);
    }

    QLabel *pCurveUnitScale = new QLabel(pScaleDialog);
    QLabel *pCurveUnitScaleUnit = new QLabel(pScaleDialog);
    pCurveUnitScale->setText(mCurveDataUnitScale.mScale);
    pCurveUnitScaleUnit->setText(mCurveDataUnitScale.mUnit);

    mpCurveExtraDataScaleLineEdit = new QLineEdit(pScaleDialog);
    mpCurveExtraDataScaleLineEdit->setValidator(new QDoubleValidator(mpCurveExtraDataScaleLineEdit));
    mpCurveExtraDataScaleLineEdit->setText(QString("%1").arg(mCurveExtraDataScale));

    mpCurveExtraDataOffsetLineEdit = new QLineEdit(pScaleDialog);
    mpCurveExtraDataOffsetLineEdit->setValidator(new QDoubleValidator(mpCurveExtraDataOffsetLineEdit));
    mpCurveExtraDataOffsetLineEdit->setText(QString("%1").arg(mCurveExtraDataOffset));


    QPushButton *pDoneButton = new QPushButton("Done", pScaleDialog);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(pScaleDialog);
    QLabel *pName = new QLabel(this->getCurveName() + QString(",     Generation: %1").arg(this->getGeneration()+1), pScaleDialog);
    QFont font = pName->font();
    font.setBold(true);
    pName->setFont(font);
    int r=0;
    pDialogLayout->addWidget(pName, r,0,1,2,Qt::AlignLeft);
    ++r;
    //Space
    pDialogLayout->setRowMinimumHeight(r,12);
    ++r;
    pDialogLayout->addWidget(new QLabel("Original Unit: ", pScaleDialog),  r,0);
    pDialogLayout->addWidget(pOriginalDataUnit,                            r,1);
    ++r;
    pDialogLayout->addWidget(new QLabel("Unit Scale: ", pScaleDialog),     r,0);
    pDialogLayout->addWidget(pCustomDataUnit,                              r,1);
    ++r;
    //Space
    pDialogLayout->setRowMinimumHeight(r,12);
    ++r;
    pDialogLayout->addWidget(new QLabel("Plot curve scale and offset that affects only this plot curve (regardless of generation):", pScaleDialog),r,0,1,2,Qt::AlignLeft);
    ++r;
    pDialogLayout->addWidget(new QLabel("Plot curve extra scale: ", pScaleDialog),      r,0);
    pDialogLayout->addWidget(mpCurveExtraDataScaleLineEdit,                             r,1);
    ++r;
    pDialogLayout->addWidget(new QLabel("Plot curve extra offset: ", pScaleDialog),     r,0);
    pDialogLayout->addWidget(mpCurveExtraDataOffsetLineEdit,                            r,1);
    ++r;

    pDialogLayout->addWidget(pButtonBox,r,0,1,2);
    pScaleDialog->setLayout(pDialogLayout);

    connect(pDoneButton,                SIGNAL(clicked()),pScaleDialog, SLOT(close()));
    connect(mpCurveExtraDataScaleLineEdit,  SIGNAL(textChanged(QString)),   SLOT(updateCurveExtraDataScaleAndOffsetFromDialog()));
    connect(mpCurveExtraDataOffsetLineEdit, SIGNAL(textChanged(QString)),   SLOT(updateCurveExtraDataScaleAndOffsetFromDialog()));

    pScaleDialog->exec();

    // Disconnect again to avoid triggering value update the next time the dialog is built
    disconnect(mpCurveExtraDataScaleLineEdit, 0, 0, 0);
    disconnect(mpCurveExtraDataOffsetLineEdit, 0, 0, 0);

    pScaleDialog->deleteLater();
}


void PlotCurve::updateCurveExtraDataScaleAndOffsetFromDialog()
{
    setCurveExtraDataScaleAndOffset(mpCurveExtraDataScaleLineEdit->text().toDouble(), mpCurveExtraDataOffsetLineEdit->text().toDouble());
}



//! @brief Updates a plot curve to the most recent available generation of its data
void PlotCurve::updateToNewGeneration()
{
    // Only change the generation if auto update is on
    if(mAutoUpdate)
    {
        setNonImportedGeneration(-1);
    }
    // Update the plot info box regardless of auto update setting, to show number of available generations correctly
    emit curveInfoUpdated();
}

//! @brief Activates (highlights) the plot curve
void PlotCurve::markActive(bool value)
{
    if(value)
    {
        mIsActive = true;
        //! @todo setZ to show selected on top, changes the actual curve order and legend order which looks strange, need to solve that somehow
        //setZ(ActiveCurveZOrderType);
    }
    else
    {
        mIsActive = false;
        //setZ(CurveZOrderType);
    }

    setLineWidth(mLineWidth);
    emit markedActive(mIsActive);
}


//! @brief Redraws the curve
//! Updates a curve with regard to special X-axis, units and scaling.
//! @todo after updating from python, scale is not refreshed maybe this should be done in here
//! @todo add optional index if we only want to update particular value
void PlotCurve::updateCurve()
{
    // Handle complex variables in a special way
    if (mData->getVariableType() == ComplexType)
    {
        ComplexVectorVariable *pComplexVar = qobject_cast<ComplexVectorVariable*>(mData.data());
        if (pComplexVar)
        {
            setSamples(pComplexVar->getRealDataCopy(), pComplexVar->getImagDataCopy());
        }
    }
    else
    {
        QVector<double> tempX, tempY;
        // We copy here, it should be faster then peek (at least when data is cached on disc)
        //! @todo maybe be smart about doing this copy
        tempY = mData->getDataVectorCopy();
        const double yScale = mCurveExtraDataScale*mCurveDataUnitScale.toDouble(1.0);
        const double yOffset = mCurveExtraDataOffset;

        if (mCustomXdata && !mShowVsSamples)
        {
            // Use special X-data
            // We copy here, it should be faster then peek (at least when data is cached on disc)
            tempX = mCustomXdata->getDataVectorCopy();
            const double xScale = mCurveXDataUnitScale.toDouble(1.0);
            const double xOffset = 0.0;
            for(int i=0; i<tempX.size() && i<tempY.size(); ++i)
            {
                tempX[i] = tempX[i]*xScale + xOffset;
                tempY[i] = tempY[i]*yScale + yOffset;
            }
        }
        // No special X-data use time vector if it exist else we cant draw curve (yet, x-date might be set later)
        else if (mData->getSharedTimeOrFrequencyVector() && !mShowVsSamples)
        {
            tempX = mData->getSharedTimeOrFrequencyVector()->getDataVectorCopy();
            const double timeScale = mCurveTFUnitScale.toDouble(1.0);
            const double timeOffset = mCurveTFOffset;

            for(int i=0; i<tempX.size() && i<tempY.size(); ++i)
            {
                tempX[i] = tempX[i]*timeScale + timeOffset;
                tempY[i] = tempY[i]*yScale + yOffset;
            }
        }
        else
        {
            // No timevector or special x-vector, plot vs samples
            tempX.resize(tempY.size());
            for (int i=0; i< tempX.size(); ++i)
            {
                tempX[i] = i;
                tempY[i] = tempY[i]*yScale + yOffset;
            }
        }

        setSamples(tempX, tempY);
    }

    emit curveDataUpdated();
}

void PlotCurve::updateCurveName()
{
    refreshCurveTitle();
    emit curveInfoUpdated();
}

void PlotCurve::dataIsBeingRemoved()
{
    emit dataRemoved(this);
}

void PlotCurve::customXDataIsBeingRemoved()
{
    setCustomXData(SharedVectorVariableT());
}

bool PlotCurve::setNonImportedGeneration(const int gen)
{
    if (mData && mData->getLogDataHandler())
    {
        if (!mData->getLogDataHandler()->isGenerationImported(gen))
        {
            return setGeneration(gen);
        }
    }
    return false;
}

void PlotCurve::deleteCustomData()
{
    if (mHaveCustomData)
    {
        mData.clear();
        mHaveCustomData = false;
    }
}

void PlotCurve::connectDataSignals()
{
    connect(mData.data(), SIGNAL(dataChanged()), this, SLOT(updateCurve()), Qt::UniqueConnection);
    connect(mData.data(), SIGNAL(nameChanged()), this, SLOT(updateCurveName()), Qt::UniqueConnection);
    connect(mData.data(), SIGNAL(beingRemoved()), this, SLOT(dataIsBeingRemoved()), Qt::UniqueConnection);
}

void PlotCurve::connectCustomXDataSignals()
{
    if (mCustomXdata)
    {
        connect(mCustomXdata.data(), SIGNAL(dataChanged()), this, SLOT(updateCurve()), Qt::UniqueConnection);
        connect(mCustomXdata.data(), SIGNAL(beingRemoved()), this, SLOT(customXDataIsBeingRemoved()), Qt::UniqueConnection);
    }
}

void PlotCurve::disconnectDataSignals()
{
    if (mData)
    {
        // Disconnect all signals from the data variable to this
        mData.data()->disconnect(this);
    }
}

void PlotCurve::disconnectCustomXDataSignals()
{
    if (mCustomXdata)
    {
        // Disconnect all data signals from any custom x-data
        mCustomXdata.data()->disconnect(this);
    }
}


//! @brief Sets auto update flag for a plot curve
//! If this is activated, plot will automatically change to latest plot generation after next simulation.
void PlotCurve::setAutoUpdate(bool value)
{
    mAutoUpdate = value;
    emit curveInfoUpdated();
}


void PlotCurve::openFrequencyAnalysisDialog()
{
    mpParentPlotArea->mpParentPlotTab->openFrequencyAnalysisDialog(this);
}



//! @brief Constructor for plot markers
//! @param pCurve Pointer to curve the marker belongs to
//! @param pPlotTab Plot tab the marker is located in
//! @param markerSymbol The symbol the marker shall use
PlotMarker::PlotMarker(PlotCurve *pCurve, PlotArea *pPlotArea)
    : QObject(pPlotArea), QwtPlotMarker()
{
    mpCurve = pCurve;
    mpPlotArea = pPlotArea;
    mIsBeingMoved = false;
    mIsMovable = true;

    mIsHighlighted = false;
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setStyle(QwtSymbol::XCross);
    mpMarkerSymbol->setSize(12);
    setSymbol(mpMarkerSymbol);
    setColor("Black");
    setLabelAlignment(Qt::AlignTop);

    setRenderHint(QwtPlotItem::RenderAntialiased);
    setZ(CurveMarkerZOrderType);
}


//! @brief Event filter for plot markers
//! This will interrupt events from plot canvas, to enable using mouse and key events for modifying markers.
//! @returns True if event was interrupted, false if its propagation shall continue
//! @param object Pointer to the object the event belongs to (in this case the plot canvas)
//! @param event Event to be interrupted
bool PlotMarker::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    // Calculate midpoint of marker in plot coordinates
    QPointF midPoint;
    midPoint.setX(plot()->transform(QwtPlot::xBottom, xValue()));
    midPoint.setY(plot()->transform(QwtPlot::yLeft, yValue()));

    // Determine if mouse cursor is close to the midpoint (should this marker be "selected", else we can skip some event filtering for this marker)
    const bool cursorIsClose = (plot()->canvas()->mapToGlobal(midPoint.toPoint()) - QCursor::pos()).manhattanLength() < 35;
    if (cursorIsClose)
    {
        if (event->type() == QEvent::ContextMenu)
        {
            QMenu *pContextMenu = new QMenu();

            // Line style selection menu
            QMenu *pLineStyleMenu = new QMenu("Line Style");
            pContextMenu->addMenu(pLineStyleMenu);
            QAction *pNoLinesAction = new QAction("No Lines", pContextMenu);
            QAction *pVerticalLineAction = new QAction("Vertical Line", pContextMenu);
            QAction *pHorizontalLineAction = new QAction("Horizontal Line", pContextMenu);
            QAction *pCrossAction = new QAction("Cross", pContextMenu);
            pLineStyleMenu->addAction(pNoLinesAction);
            pLineStyleMenu->addAction(pVerticalLineAction);
            pLineStyleMenu->addAction(pHorizontalLineAction);
            pLineStyleMenu->addAction(pCrossAction);

            // Label alignment selection menu
            QMenu *pAlignmentMenu = new QMenu("Label Alignment");
            pContextMenu->addMenu(pAlignmentMenu);
            QList<AlignmentSelectionStruct> alignments;
            alignments.append(AlignmentSelectionStruct(Qt::AlignTop|Qt::AlignLeft, "Top Left"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignTop, "Top"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignTop|Qt::AlignRight, "Top Right"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignRight, "Right"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignBottom|Qt::AlignRight, "Bottom Right"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignBottom, "Bottom"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignBottom|Qt::AlignLeft, "Bottom Left"));
            alignments.append(AlignmentSelectionStruct(Qt::AlignLeft, "Left"));
            QAction *pAction;
            QMap<QAction*, AlignmentSelectionStruct*> alignSelectionMap;
            for (int i=0; i<alignments.size(); ++i)
            {
                pAction = new QAction(alignments[i].mLabel,pContextMenu);
                pAlignmentMenu->addAction(pAction);
                alignSelectionMap.insert(pAction, &alignments[i]);
            }

            // Marker symbol selection menu
            QMenu *pSymbolMenu = new QMenu("Marker Symbol");
            pContextMenu->addMenu(pSymbolMenu);
            QList<SymbolSelectionStruct> symbols;
            symbols.append(SymbolSelectionStruct(QwtSymbol::Ellipse, "Ellipse"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Rect, "Rect"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Diamond, "Diamond"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Triangle, "Triangle"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::DTriangle, "DTriangle"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::UTriangle, "UTriangle"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::LTriangle, "LTriangle"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::RTriangle, "RTriangle"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Cross, "Cross"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::XCross, "XCross"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Star1, "Star1"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Star2, "Star2"));
            symbols.append(SymbolSelectionStruct(QwtSymbol::Hexagon, "Hexagon"));
            QMap<QAction*, SymbolSelectionStruct*> symbolSelectionMap;
            for (int i=0; i<symbols.size(); ++i)
            {
                pAction = new QAction(symbols[i].mLabel,pContextMenu);
                pSymbolMenu->addAction(pAction);
                symbolSelectionMap.insert(pAction, &symbols[i]);
            }

            // Marker color selection menu
            QMenu *pColorMenu = new QMenu("Marker Color");
            pContextMenu->addMenu(pColorMenu);
            QAction *pCurveColorAction = new QAction("Curve Color",pContextMenu);
            QAction *pBlackColorAction = new QAction("Black ",pContextMenu);
            pColorMenu->addAction(pCurveColorAction);
            pColorMenu->addAction(pBlackColorAction);

            // Delete marker action
            QAction *pDeleteAction = pContextMenu->addAction("Remove Marker");


            // Execute selected action
            pAction = pContextMenu->exec(QCursor::pos());
            if (pAction == pDeleteAction)
            {
                mpPlotArea->removePlotMarker(this);
            }
            else if(pAction == pNoLinesAction)
            {
                this->setLineStyle(NoLine);
            }
            else if(pAction == pVerticalLineAction)
            {
                this->setLineStyle(VLine);
            }
            else if(pAction == pHorizontalLineAction)
            {
                this->setLineStyle(HLine);
            }
            else if(pAction == pCrossAction)
            {
                this->setLineStyle(Cross);
            }
            else if (pAction == pCurveColorAction)
            {
                connect(mpCurve, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)));
                setColor(mpCurve->pen().color());
            }
            else if (pAction == pBlackColorAction)
            {
                disconnect(mpCurve, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)));
                setColor("Black");
            }
            else if(alignSelectionMap.contains(pAction))
            {
                setLabelAlignment(alignSelectionMap.value(pAction)->mAlignment);
            }
            else if(symbolSelectionMap.contains(pAction))
            {
                mpMarkerSymbol->setStyle(symbolSelectionMap.value(pAction)->mStyle);
            }

            pContextMenu->deleteLater();
            return true;

        }

        // Mouse press events, used to initiate moving of a marker if mouse cursor is close enough
        else if (event->type() == QEvent::MouseButtonPress)
        {
            if(!mIsMovable)
                return false;

            if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            {
                //! @todo it is not very nice that this marker must go and check stuff in the parent plot area
                if(!mpPlotArea->mpQwtZoomerLeft->isEnabled() && !mpPlotArea->mpQwtPanner->isEnabled())
                {
                    mIsBeingMoved = true;
                    return true;
                }
            }
        }

        // When mouse moves over the marker (hover, or drag move) we highlight it
        else if ((event->type() == QEvent::MouseMove) && !mIsHighlighted)
        {
            mIsHighlighted = true;
            highlight(mIsHighlighted);
        }


        // Keypress event, will delete marker if delete key is pressed
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Delete)
            {
                mpPlotArea->removePlotMarker(this);
                return true;
            }
            return false;
        }
    }

    // We must check the following always, since a quick mouse move will take us outside the "cursorIsClose range" before a move event is generated

    // If we are moving the mouse, then update the position and label
    if ((event->type() == QEvent::MouseMove) && mIsBeingMoved)
    {
        int idx = mpCurve->closestPoint(plot()->canvas()->mapFromGlobal(QCursor::pos()));
        double x = mpCurve->sample(idx).x();
        double y = mpCurve->sample(idx).y();
        setXValue(x);
        setYValue(plot()->invTransform(QwtPlot::yLeft, plot()->transform(mpCurve->yAxis(), y)));
        refreshLabel(x, y);

        emit idxChanged(idx);

        return true;
    }
    // Mouse release event, will stop moving marker
    else if (event->type() == QEvent::MouseButtonRelease && (mIsBeingMoved == true))
    {
        mIsBeingMoved = false;
        return false;
    }

    // When mouse leaves hover we do not want it to be highlighted
    if (mIsHighlighted && !mIsBeingMoved && !cursorIsClose)
    {
        mIsHighlighted = false;
        highlight(mIsHighlighted);
    }

    return false;
}

void PlotMarker::setMovable(bool movable)
{
    mIsMovable = movable;
}

void PlotMarker::refreshLabel(const double x, const double y)
{
    refreshLabel(QString("( %1,  %2 )").arg(x).arg(y));
}

void PlotMarker::refreshLabel(const QString &label)
{
    QwtText qwtlabel(label);
    qwtlabel.setColor(Qt::black);
    qwtlabel.setBackgroundBrush(QColor(255,255,255,240));
    qwtlabel.setFont(QFont("Calibri", 14, QFont::Normal));

    Qt::Alignment alignment = labelAlignment();
    setLabel(qwtlabel);
    setLabelAlignment(alignment);
}


void PlotMarker::setColor(QColor color)
{
    mpMarkerSymbol->setPen(color,3);
    setLinePen(color,2, Qt::DotLine);
    highlight(mIsHighlighted);
}

void PlotMarker::highlight(bool tf)
{
    QColor color = mpMarkerSymbol->pen().color();
    if (tf)
    {
        color.setAlpha(150);
        mpMarkerSymbol->setPen(color, 5);
        setLinePen(color, 4, Qt::DotLine);
    }
    else
    {
        color.setAlpha(255);
        mpMarkerSymbol->setPen(color, 3);
        setLinePen(color, 2, Qt::DotLine);
    }
    emit highlighted(tf);
}


//! @brief Returns a pointer to the curve a plot marker belongs to
PlotCurve *PlotMarker::getCurve()
{
    return mpCurve;
}


QList<QwtLegendData> PlotCurve::legendData() const
{
    QList<QwtLegendData> list = QwtPlotCurve::legendData();
    for (int i=0; i<list.size(); ++i)
    {
        list[i].setValue( AxisIdRole, this->yAxis() );
    }
    return list;
}

//! @brief This function overload is required to avoid auto-scale problems when the data contains inf
//! @note This is related to issue #1151
QRectF PlotCurve::boundingRect() const
{
    QRectF rect = QwtPlotCurve::boundingRect();
    if (std::isinf(rect.width()) || std::isinf(rect.height()))
    {
        qDebug() << "---------------- Bounding rect        : " << rect;
        const double lim = DoubleMax*0.9;

        if (std::isinf(rect.top()))
        {
            rect.setTop(-lim);
        }

        if (std::isinf(rect.right()))
        {
            rect.setRight(lim);
        }

        if (std::isinf(rect.bottom()))
        {
            rect.setBottom(lim);
        }

        if (std::isinf(rect.left()))
        {
            rect.setLeft(-lim);
        }
        qDebug() << "---------------- Bounding rect (fixed): " << rect;
    }
    return rect;
}

PlotLegend::PlotLegend(QwtPlot::Axis axisId) :
    QwtPlotLegendItem()
{
    setMaxColumns(1);
    setRenderHint(QwtPlotItem::RenderAntialiased);
    setBackgroundMode(LegendBackground);
    setBackgroundBrush(QColor(Qt::white));
    setBorderRadius(8);
    setMargin(4);
    setSpacing(2);
    setItemMargin(0);
    QFont font = this->font();
    font.setPointSize(11);
    setFont(font);

    mAxis = axisId;
}

void PlotLegend::updateLegend(const QwtPlotItem *plotItem, const QList<QwtLegendData> &data)
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


//! @brief Constructor
//! @param pos Position where user clicked
//! @param pPlotArea Pointer to parent plot area
MultiPlotMarker::MultiPlotMarker(QPoint pos, PlotArea *pPlotArea)
{
    QList<PlotCurve*> curves = pPlotArea->getCurves();

    if(curves.isEmpty()) return;    //No curves, then nothing can be done

    int idx = curves[0]->closestPoint(pos);     //Index where to insert multi-marker

    //Create one marker per curve
    for(int i=0; i<curves.size(); ++i)
    {
        double x = curves[i]->sample(idx).x();
        double y = curves[i]->sample(idx).y();

        PlotMarker *pMarker = new PlotMarker(curves[i], pPlotArea);
        mPlotMarkerPtrs.append(pMarker);

        pMarker->attach(pPlotArea->getQwtPlot());
        pMarker->setXValue(x);
        pMarker->setYValue(y);
        pMarker->refreshLabel(x, y);
        pMarker->setLabelAlignment(Qt::AlignLeft);

        pPlotArea->getQwtPlot()->canvas()->installEventFilter(pMarker);
        pPlotArea->getQwtPlot()->canvas()->setMouseTracking(true);
        pMarker->setMovable(true);

        connect(pMarker, SIGNAL(idxChanged(int)), this, SLOT(moveAll(int)));
        connect(pMarker, SIGNAL(highlighted(bool)), this, SLOT(highlight(bool)));
    }

    //Create the dummy plot marker, it is never visible itself but is used to display the vertical line
    mpDummyMarker = new QwtPlotMarker();
    mpDummyMarker->attach(pPlotArea->getQwtPlot());
    mpDummyMarker->setXValue(curves[0]->sample(idx).x());
    mpDummyMarker->setLineStyle(QwtPlotMarker::VLine);
    mpDummyMarker->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol));
    mpDummyMarker->setLinePen(QColor("black"),2, Qt::DotLine);
}


//! @brief Adds a marker to the specified curve, used when adding curves to plot
//! @param pCurve Curve to insert marker at
void MultiPlotMarker::addMarker(PlotCurve *pCurve)
{
    //Calculate index where to insert marker
    QwtPlot *pPlot = mPlotMarkerPtrs.first()->getCurve()->plot();
    PlotCurve *pFirstCurve = mPlotMarkerPtrs.first()->getCurve();
    double x_pos = pPlot->transform(mPlotMarkerPtrs.first()->xAxis(), mPlotMarkerPtrs.first()->xValue());
    double y_pos = pPlot->transform(mPlotMarkerPtrs.first()->yAxis(), mPlotMarkerPtrs.first()->yValue());
    int idx = pFirstCurve->closestPoint(QPoint(x_pos,y_pos));

    //Calculate position on line
    double x = pCurve->sample(idx).x();
    double y = pCurve->sample(idx).y();

    //Create the marker
    PlotMarker *pMarker = new PlotMarker(pCurve, pCurve->getParentPlotArea());
    mPlotMarkerPtrs.append(pMarker);
    pMarker->attach(pCurve->plot());
    pMarker->setXValue(x);
    pMarker->setYValue(pCurve->plot()->invTransform(QwtPlot::yLeft, pCurve->plot()->transform(pCurve->yAxis(), y)));
    pMarker->refreshLabel(x, y);
    pMarker->setMovable(true);
    pMarker->setLineStyle(QwtPlotMarker::VLine);

    pCurve->getParentPlotArea()->getQwtPlot()->canvas()->installEventFilter(pMarker);
    pCurve->getParentPlotArea()->getQwtPlot()->canvas()->setMouseTracking(true);

    connect(pMarker, SIGNAL(idxChanged(int)), this, SLOT(moveAll(int)));
    connect(pCurve, SIGNAL(destroyed()), this, SLOT(update()));

    //Update position of all points (just in case)
    moveAll(idx);
}


//! @brief Removes marker from specified curve (used when removing curves from plot)
//! @param pCurve Pointer to curve from where the marker shall be removed
void MultiPlotMarker::removeMarker(PlotCurve *pCurve)
{
    //Loop through all markers
    for(int i=0; i<mPlotMarkerPtrs.size(); ++i)
    {
        //Find marker with specified curve
        if(mPlotMarkerPtrs[i]->getCurve() == pCurve)
        {
            //Delete and remove it
            delete(mPlotMarkerPtrs[i]);
            mPlotMarkerPtrs.removeAt(i);
            --i;
        }
    }
}


//! @brief Slot that highlights the vertical line
//! @param tf Tells whether to highlight or not
void MultiPlotMarker::highlight(bool tf)
{
    if(tf)
        mpDummyMarker->setLinePen(QColor("black"), 4, Qt::DotLine);
    else
        mpDummyMarker->setLinePen(QColor("black"), 2, Qt::DotLine);
}


//! @brief Moves all markers to specified index position
//! @param idx Index to move marker sto
void MultiPlotMarker::moveAll(int idx)
{
    //Move each marker
    for(int i=0; i<mPlotMarkerPtrs.size(); ++i)
    {
        double x = mPlotMarkerPtrs[i]->getCurve()->sample(idx).x();
        double y = mPlotMarkerPtrs[i]->getCurve()->sample(idx).y();

        PlotCurve *pCurve = mPlotMarkerPtrs[i]->getCurve();
        mPlotMarkerPtrs[i]->setXValue(x);
        mPlotMarkerPtrs[i]->setYValue(pCurve->plot()->invTransform(QwtPlot::yLeft, pCurve->plot()->transform(pCurve->yAxis(), y)));
        mPlotMarkerPtrs[i]->refreshLabel(x, y);
    }

    //Move the vertical line
    mpDummyMarker->setXValue(mPlotMarkerPtrs[0]->getCurve()->sample(idx).x());
}
