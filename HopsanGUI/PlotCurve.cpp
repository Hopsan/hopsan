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
//! @file   PlotCurve.cpp
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

namespace {
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

UnitConverter lookupUnitConverter(const QString& dataQuantity, const QString& newUnit, const QString& currentUnit) {
    UnitConverter us;
    // For varaibles without quantity, try to lookup, but only unique quantities
    if (dataQuantity.isEmpty())
    {
        // Only set the new unit if it represents the same physical quantity as the current unit
        QStringList pqs = gpConfig->getQuantitiesForUnit(newUnit);
        QStringList pqsOrg = gpConfig->getQuantitiesForUnit(currentUnit);
        if ( !(pqs.isEmpty() || pqsOrg.isEmpty()) )
        {
            if (pqs.front() == pqsOrg.front())
            {
                gpConfig->getUnitScale(pqs.first(), newUnit, us);
            }
        }
    }
    // If quantity known then make sure that unit is actually a valid unit for that quantity
    else
    {
        // Check so that this unit is relevant for this type of data (datname). Else it will be ignored
        if (gpConfig->hasUnitScale(dataQuantity, newUnit))
        {
            gpConfig->getUnitScale(dataQuantity, newUnit, us);
        }
    }
    return us;
}

class DataUnitConverter {
public:
    DataUnitConverter(const UnitConverter& uc, double dataPlotOffsett, bool invert, double localScale, double localOffset) :
        mUc(uc), mDataPlotOffsett(dataPlotOffsett), mInvert(invert), mLocalScale(localScale), mLocalOffset(localOffset) { }

    void convertVector(QVector<double> &rDataVector) {
        double direction = mInvert ? -1.0 : 1.0;
        if (mUc.isExpression()) {
            for (double& rV : rDataVector) {
                rV =  direction * mUc.convertFromBase(rV+mDataPlotOffsett) + mLocalOffset;
            }
        }
        else {
            const double scaleFromBaseToDesiredUnit = mLocalScale*direction/mUc.scaleToDouble(1.0);
            const double offsetInBaseUnit = mDataPlotOffsett - mUc.offsetToDouble();
            for (double& rV : rDataVector) {
                rV =  (rV+offsetInBaseUnit)*scaleFromBaseToDesiredUnit + mLocalOffset;
            }
        }
    }

    UnitConverter mUc;
    double mDataPlotOffsett;
    bool mInvert;
    double mLocalScale;
    double mLocalOffset;
};

}

//! @brief Constructor for plot curves.
//! @param pData A shared pointer to the data to plot
//! @param curveType The type of the curve (controls the name and some other special things)
//! @todo why is the axis in the curve constructor, it would make more sense if the axis is specified when adding a curve to a plot area /Peter
PlotCurve::PlotCurve(SharedVectorVariableT data, const QwtPlot::Axis axisY, const HopsanPlotCurveTypeEnumT curveType)
    : QObject(), QwtPlotCurve()
{
    mpParentPlotArea = nullptr;
    mHaveCustomData = false;
    mShowVsSamples = false;
    mData = data;
    mSetGeneration = mData->getGeneration();
    mSetGenerationIsValid = true;

    mCurveExtraDataScale = 1.0;
    mCurveExtraDataOffset = 0.0;

    mpCurveSymbol = nullptr;
    mCurveSymbolSize = 8;
    mIsActive = false;
    mIncludeGenInTitle = true;
    mIncludeSourceInTitle = false;
    mCurveType = curveType;

    mAxisY = axisY;

    // We do not want imported data to auto refresh, in case the data name is the same as something from the model (alias)
    mAutoUpdate = !data->isImported();

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
int PlotCurve::getDataGeneration() const
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
    UnitConverter us = getCurveDataUnitScale();
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
    UnitConverter uc = getCurveCustomXDataUnitScale();
    if (!uc.isEmpty()) {
        return uc.mUnit;
    }
    return {};
}

QString PlotCurve::getCurrentTFPlotUnit() const
{
    UnitConverter uc = getCurveTFUnitScale();
    if (!uc.isEmpty()) {
        return uc.mUnit;
    }
    return {};
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

QString PlotCurve::getAliasName() const
{
    return mData->getAliasName();
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

//! @brief Returns the minimum (values higher then 0) and maximum value of the curve y-values (with unit scale applied)
//! @details values <= 0 are ignored
bool PlotCurve::minMaxPositiveNonZeroYValues(double &rMin, double &rMax)
{
    int imax, imin;
    bool rv = mData->positiveNonZeroMinMaxOfData(rMin, rMax, imin, imax);
    if (!mCurveDataUnitScale.isEmpty()) {
        rMin = mCurveDataUnitScale.convertFromBase(rMin);
        rMax = mCurveDataUnitScale.convertFromBase(rMax);
    }
    return rv;
}

//! @brief Returns the minimum (values higher then 0) and maximum value of the curve x-values (with unit scale applied)
//! @details values <= 0 are ignored
bool PlotCurve::minMaxPositiveNonZeroXValues(double &rMin, double &rMax)
{
    int imax, imin;
    if (mCustomXdata)
    {
        bool rv =  mCustomXdata->positiveNonZeroMinMaxOfData(rMin, rMax, imin, imax);
        if (!mCurveCustomXDataUnitScale.isEmpty()) {
            rMin = mCurveCustomXDataUnitScale.convertFromBase(rMin);
            rMax = mCurveCustomXDataUnitScale.convertFromBase(rMax);
        }
        return rv;
    }
    else if (mData->getSharedTimeOrFrequencyVector())
    {
        bool rv = mData->getSharedTimeOrFrequencyVector()->positiveNonZeroMinMaxOfData(rMin, rMax, imin, imax);
        if (!mCurveTFUnitScale.isEmpty()) {
            rMin = mCurveTFUnitScale.convertFromBase(rMin);
            rMax = mCurveTFUnitScale.convertFromBase(rMax);
        }
        return rv;
    }
    else
    {
        rMin = 0;
        rMax = mData->getDataSize()-1;
        return (rMax > -1);
    }
}

bool PlotCurve::isCurveGenerationValid() const
{
    return mSetGenerationIsValid;
}

int PlotCurve::getCurveGeneration() const
{
    return mSetGeneration;
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
    mSetGeneration = generation;
    const bool requestedGenerationIsLatestAvailable = (generation == -1);
    if(mData)
    {
        SharedVectorVariableT pPreviousData = mData;
        SharedVectorVariableT pNewData = switchVariableGeneration(mData, generation);
        if (pNewData)
        {
            mSetGeneration = pNewData->getGeneration(); //Use this to replace gen=-1 (current) with actual generation number
            if (hasCustomXData())
            {
                SharedVectorVariableT pNewXData = switchVariableGeneration(mCustomXdata, pNewData->getGeneration());
                if (pNewXData)
                {
                    disconnectDataSignals();
                    mData = pNewData;
                    connectDataSignals();
                    setCustomXData(pNewXData);
                    mSetGenerationIsValid = true;
                }
                // If we cant switch the custom X data then the switch should not be allowed
                else
                {
                    mSetGenerationIsValid = false;
                }
            }
            else
            {
                disconnectDataSignals();
                mData = pNewData;
                connectDataSignals();
                mSetGenerationIsValid = true;
            }
        }
        else
        {
            mSetGenerationIsValid = false;
        }

        // If the curve is updated to latest available generation, and there is no gap between the previous generation then consider using the same inverted
        // plot toggle state from the previous generation.
        if (requestedGenerationIsLatestAvailable && mSetGenerationIsValid && (mSetGeneration > 0) &&
            pPreviousData && (pPreviousData->getGeneration() == (mSetGeneration-1))) {

            const auto& previousVarDesc = pPreviousData->getVariableDescription();
            const auto& newVarDesc = mData->getVariableDescription();
            const bool modelInvertedStateChanged = (previousVarDesc->mModelInvertPlot != newVarDesc->mModelInvertPlot);
            // If model invert state has changed let it override local choice, otherwise use same invert state as previous generation
            const bool invertNew = modelInvertedStateChanged ? newVarDesc->mModelInvertPlot : pPreviousData->isPlotInverted();
            mData->setPlotInverted(invertNew);
        }

        updateCurve();
        refreshCurveTitle();

        // Abort if curve generation is not valid, return false to indicate that the curve is currently not in a valid state
        if (!mSetGenerationIsValid)
        {
            return false;
        }

        //! @todo should this be done here
        mpParentPlotArea->resetZoom();

        return true;
    }
    mSetGenerationIsValid = false;
    return false;
}

bool PlotCurve::setNonImportedGeneration(const int generation)
{
    if (mData)
    {
        LogDataHandler2 *pLDH = mData->getLogDataHandler();
        if (pLDH)
        {
            if(!pLDH->isGenerationImported(generation))
            {
                return setGeneration(generation);
            }
        }
    }
    // Note! We return true here even if no switch was made, returning false only if we did switch and data was not available
    return true;
}

bool PlotCurve::autoDecrementModelSourceGeneration()
{
    // We do not want to switch generations automatically for curves representing imported data.
    // That would make it very difficult to compare imported data with a model variable of the same name
    // This was decided based on how AC is using the program
    if (mAutoUpdate && mData && !mData->isImported())
    {
        LogDataHandler2 *pLDH = mData->getLogDataHandler();
        if (pLDH)
        {
            // Loop until we find next non-imported higher generation, abort if we reach the highest
            int gen = mSetGeneration-1;
            while ( gen >= pLDH->getLowestGenerationNumber() )
            {
                if(!pLDH->isGenerationImported(gen))
                {
                    return setGeneration(gen);
                }
                --gen;
            }
        }
    }
    // Note! We return true here even if no switch was made, returning false only if we did switch and data was not available
    return true;
}

bool PlotCurve::autoIncrementModelSourceGeneration()
{
    // We do not want to switch generations automatically for curves representing imported data.
    // That would make it very difficult to compare imported data with a model variable of the same name
    // This was decided based on how AC is using the program
    if (mAutoUpdate && mData && !mData->isImported())
    {
        LogDataHandler2 *pLDH = mData->getLogDataHandler();
        if (pLDH)
        {
            // Loop until we find next non-imported higher generation, abort if we reach the highest
            int gen = mSetGeneration+1;
            while ( gen <= pLDH->getHighestGenerationNumber() )
            {
                if(!pLDH->isGenerationImported(gen))
                {
                    return setGeneration(gen);
                }
                ++gen;
            }
        }
    }
    // Note! We return true here even if no switch was made, returning false only if we did switch and data was not available
    return true;
}


//! @brief Sets the unit of a plot curve
//! @details The physical quantity will be checked, if it does not match the current unit, the new unit will be ignored
//! @param[in] rUnit Name of new unit
//! @note If unit is not registered for data then nothing will happen
void PlotCurve::setCurveDataUnitScale(const QString &rUnit)
{
    QString dataQuantity = getDataQuantity();
    UnitConverter us = lookupUnitConverter(dataQuantity, rUnit, getDataUnit());
    if (!us.isEmpty()) {
        setCurveDataUnitScale(us);
    }
}

void PlotCurve::setCurveDataUnitScale(const UnitConverter &rUS)
{
    if (rUS.isEmpty())
    {
        resetCurveDataUnitScale();
    }
    else
    {
        mCurveDataUnitScale = rUS;

        // Clear the custom scale if it is (a custom scale) 1 (and not an actual quantity/unit scaled as 1) and we have a data unit
        if (!getDataUnit().isEmpty() && (rUS.isScaleOne() && rUS.mUnit.isEmpty()))
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

const UnitConverter PlotCurve::getCurveDataUnitScale() const
{
    if (mCurveDataUnitScale.isEmpty())
    {
        // If data have an original unit then return that as a unit scale with scaling 1.0
        if (!mData->getDataUnit().isEmpty())
        {
            return UnitConverter(mData->getDataName(), mData->getDataUnit(), "1.0", "0");
        }
        // If not then return empty
        else
        {
            return UnitConverter();
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

bool PlotCurve::hasCurveCustomXDataUnitScale() const
{
    return !mCurveCustomXDataUnitScale.isEmpty();
}

void PlotCurve::setCurveCustomXDataUnitScale(const QString &rUnit)
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
                    setCurveCustomXDataUnitScale(gpConfig->getUnitScaleUC(pqs.first(), rUnit));
                }
            }
        }
        // If quantity known then make sure that unit is actually a valid unit for that quantity
        else
        {
            // Check so that this unit is relevant for this type of data (datname). Else it will be ignored
            if (gpConfig->hasUnitScale(xDataQuantity,rUnit)) {
                setCurveCustomXDataUnitScale(gpConfig->getUnitScaleUC(xDataQuantity, rUnit));
            }
        }
    }
}

void PlotCurve::setCurveCustomXDataUnitScale(const UnitConverter &rUS)
{
    // Clear the custom scale if it is empty or one and data have an actual unit
    if (rUS.isEmpty() || (!mCustomXdata->getDataUnit().isEmpty() && mCurveCustomXDataUnitScale.isScaleOne())) {
        resetCurveCustomXDataUnitScale();
    }
    else {
        mCurveCustomXDataUnitScale = rUS;

        updateCurve();
        //! @todo shouldn't these be triggered by signal in update curve?
        mpParentPlotArea->replot();
    }
}

UnitConverter PlotCurve::getCurveCustomXDataUnitScale() const
{
    // If we do not have a custom unit scale
    if (mCurveCustomXDataUnitScale.isEmpty())
    {
        // If custom x-data has an original unit then return that as a unit scale with scaling 1.0
        if ( mCustomXdata && !mCustomXdata->getDataUnit().isEmpty() )
        {
            return UnitConverter(mCustomXdata->getDataQuantity(), mCustomXdata->getDataUnit(), "1.0", "");
        }
        // If not then return empty unit scale
        return UnitConverter();
    }
    // Return the custom unit scale
    else
    {
        return mCurveCustomXDataUnitScale;
    }
}

void PlotCurve::resetCurveCustomXDataUnitScale()
{
    mCurveCustomXDataUnitScale.clear();
    updateCurve();

    //! @todo shouldn't these be triggered by signal in update curve?
    mpParentPlotArea->replot();
}

void PlotCurve::setCurveTFUnitScale(const UnitConverter &us)
{
    if (us.isEmpty()) {
        resetCurveTFUnitScale();
    }
    else {
        mCurveTFUnitScale = us;
        updateCurve();
        //! @todo shouldn't these be triggered by signal in update curve?
        mpParentPlotArea->replot();
    }
}

void PlotCurve::setCurveTFUnitScale(const QString &unit)
{
    auto pTof = getSharedTimeOrFrequencyVariable();
    if (pTof) {
        QString dataQuantity = pTof->getDataQuantity();
        UnitConverter us = lookupUnitConverter(dataQuantity, unit, pTof->getDataUnit());
        if (!us.isEmpty()) {
            setCurveTFUnitScale(us);
        }
    }
}

void PlotCurve::resetCurveTFUnitScale()
{
    mCurveTFUnitScale.clear();
    updateCurve();

    //! @todo shouldn't these be triggered by signal in update curve?
    mpParentPlotArea->replot();
}

UnitConverter PlotCurve::getCurveTFUnitScale() const
{
    if (mCurveTFUnitScale.isEmpty())
    {
        SharedVectorVariableT tfVar = mData->getSharedTimeOrFrequencyVector();
        if (tfVar)
        {
            return UnitConverter(tfVar->getDataName(), tfVar->getDataUnit(), "1.0", "");
        }
        else
        {
            return UnitConverter();
        }
    }
    else
    {
        return mCurveTFUnitScale;
    }
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

    // Reset custom scale if new data have different quantity
    QString currentQuantity = mCurveCustomXDataUnitScale.isEmpty() ? QString() : mCurveCustomXDataUnitScale.mQuantity;
    if(data && (data->getDataQuantity() != currentQuantity)) {
        mCurveCustomXDataUnitScale.clear();
    }

    // Set new data and connect signals
    mCustomXdata = data;
    connectCustomXDataSignals();

    // Redraw curve
    updateCurve();

    emit customXDataChanged(this);
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

bool PlotCurve::isInverted() const
{
    if (mData) {
        return mData->isPlotInverted();
    }
    return false;
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
    mpCurveSymbol->setStyle(PlotCurveStyle::toSymbolEnum(lineSymbol));

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
    UnitConverter us = getCurveDataUnitScale();
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
    QLabel *pName = new QLabel(this->getCurveName() + QString(",     Generation: %1").arg(this->getDataGeneration()+1), pScaleDialog);
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
    // Only change the generation if auto update is on and if this is not an imported variable
    if(mAutoUpdate && mData && !mData->isImported())
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
        //! @todo maybe be smart about doing this copy, only copy if a disk cached variable
        tempY = mData->getDataVectorCopy();

        const bool invertYData = mData->isPlotInverted();
        DataUnitConverter yConverter(mCurveDataUnitScale, mData->getGenerationPlotOffsetIfTime(), invertYData, mCurveExtraDataScale, mCurveExtraDataOffset);
        yConverter.convertVector(tempY);

        if (mCustomXdata && !mShowVsSamples)
        {
            // Use special X-data, Copy here, it should be faster then peek (at least when data is cached on disc)
            tempX = mCustomXdata->getDataVectorCopy();
            const bool xInvertData = mCustomXdata->isPlotInverted();
            constexpr double localCurveXScale = 1.0;
            constexpr double localCurveXOffset = 0.0;
            DataUnitConverter xConverter(mCurveCustomXDataUnitScale, mCustomXdata->getGenerationPlotOffsetIfTime(), xInvertData, localCurveXScale, localCurveXOffset);
            xConverter.convertVector(tempX);
        }
        // No special X-data use time vector if it exist else we cant draw curve (yet, x-date might be set later)
        else if (mData->getSharedTimeOrFrequencyVector() && !mShowVsSamples)
        {
            tempX = mData->getSharedTimeOrFrequencyVector()->getDataVectorCopy();
            const double timeDataOffset = mData->getSharedTimeOrFrequencyVector()->getGenerationPlotOffsetIfTime();
            constexpr bool notInverted = false;
            constexpr double localCurveTFScale = 1.0;
            constexpr double localCurveTFOffset = 0.0;
            DataUnitConverter xConverter(mCurveTFUnitScale, timeDataOffset, notInverted, localCurveTFScale, localCurveTFOffset);
            xConverter.convertVector(tempX);
        }
        else
        {
            // No time vector or special x-vector, plot vs samples
            tempX.resize(tempY.size());
            for (int i=0; i< tempX.size(); ++i) {
                tempX[i] = i;
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
    connect(mData.data(), SIGNAL(quantityChanged()), this, SIGNAL(curveInfoUpdated()), Qt::UniqueConnection);
}

void PlotCurve::connectCustomXDataSignals()
{
    if (mCustomXdata)
    {
        connect(mCustomXdata.data(), SIGNAL(dataChanged()), this, SLOT(updateCurve()), Qt::UniqueConnection);
        connect(mCustomXdata.data(), SIGNAL(nameChanged()), this, SLOT(updateCurveName()), Qt::UniqueConnection);
        connect(mCustomXdata.data(), SIGNAL(beingRemoved()), this, SLOT(customXDataIsBeingRemoved()), Qt::UniqueConnection);
        connect(mCustomXdata.data(), SIGNAL(quantityChanged()), this, SIGNAL(curveInfoUpdated()), Qt::UniqueConnection);
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

void PlotCurve::setInvertPlot(bool tf)
{
    if (mData) {
        mData->setPlotInverted(tf);
    }
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
            for (size_t i=1; i<PlotCurveStyle::symbol_enums.size(); ++i ) {
                symbols.append(SymbolSelectionStruct(PlotCurveStyle::symbol_enums[i], PlotCurveStyle::symbol_names[i]));
            }

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
    refreshLabel(QString("( %1,  %2 )").arg(x,0,'g',10).arg(y,0,'g',10));
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

void PlotMarker::updatePosition()
{
    const QPointF plotPos = this->value();

    const QwtPlot *plot = mpCurve->plot();
    const QwtScaleMap &xMap = plot->canvasMap(mpCurve->xAxis());
    const QwtScaleMap &yMap = plot->canvasMap(mpCurve->yAxis());

    const int px = qRound(xMap.transform(plotPos.x()));
    const int py = qRound(yMap.transform(plotPos.y()));
    const QPoint canvasPos(px, py);


    double dist = 0.0;
    const int index = mpCurve->closestPoint(canvasPos, &dist);

    if (index >= 0)
    {
        const QPointF closestPoint = mpCurve->sample(index);
        this->setXValue(closestPoint.x());
        this->setYValue(closestPoint.y());
    }
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
        connect(curves[i], SIGNAL(curveDataUpdated()), this, SLOT(updatePosition()));
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


void MultiPlotMarker::updatePosition()
{
    const QPointF plotPos = mPlotMarkerPtrs[0]->value();

    const auto curve = mPlotMarkerPtrs[0]->getCurve();
    const QwtPlot *plot = curve->plot();
    const QwtScaleMap &xMap = plot->canvasMap(curve->xAxis());
    const QwtScaleMap &yMap = plot->canvasMap(curve->yAxis());

    const int px = qRound(xMap.transform(plotPos.x()));
    const int py = qRound(yMap.transform(plotPos.y()));
    const QPoint canvasPos(px, py);

    double dist = 0.0;
    const int index = curve->closestPoint(canvasPos, &dist);

    if(index > 0) {
        moveAll(index);
    }
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
