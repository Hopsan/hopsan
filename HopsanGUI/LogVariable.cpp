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
//! @file   LogDataHandler.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2012-12-18
//!
//! @brief Contains the LogData classes
//!
//$Id$

#include "global.h"
#include "Configuration.h"
#include "LogVariable.h"
#include "LogDataHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include <QMessageBox>

#include <limits>
#include <QMessageBox>

SharedVariableDescriptionT createTimeVariableDescription()
{
    SharedVariableDescriptionT pVarDesc(new VariableDescription());
    pVarDesc->mDataName = TIMEVARIABLENAME;
    pVarDesc->mDataUnit = "s";
    return pVarDesc;
}


SharedVariableDescriptionT createFrequencyVariableDescription()
{
    SharedVariableDescriptionT pVarDesc(new VariableDescription());
    pVarDesc->mDataName = FREQUENCYVARIABLENAME;
    pVarDesc->mDataUnit = "Hz";
    return pVarDesc;
}

SharedVectorVariableT createFreeVectorVariable(const QVector<double> &rData, SharedVariableDescriptionT pVarDesc)
{
    return SharedVectorVariableT(new VectorVariable(rData, 0, pVarDesc, SharedMultiDataVectorCacheT()));
}

//! @brief Creates a free unhandled time vector logvariable, it can not have generations or be cached
SharedVectorVariableT createFreeTimeVectorVariabel(const QVector<double> &rTime)
{
    // Since there is no parent we can not cache this to disk or give it a generation, it is a free floating time vector (logvariable)
    return SharedVectorVariableT(new VectorVariable(rTime, 0, createTimeVariableDescription(), SharedMultiDataVectorCacheT(0)));
}

//! @brief Creates a free unhandled frequency vector logvariable, it can not have generations or be cached
SharedVectorVariableT createFreeFrequencyVectorVariabel(const QVector<double> &rFrequency)
{
    // Since there is no parent we can not cache this to disk or give it a generation, it is a free floating time vector (logvariable)
    return SharedVectorVariableT(new VectorVariable(rFrequency, 0, createFrequencyVariableDescription(), SharedMultiDataVectorCacheT(0)));
}

//! @brief This is a variable factory, variables will be free and wont be connected to a data chace
SharedVectorVariableT createFreeVariable(VariableTypeT type, SharedVariableDescriptionT pVarDesc)
{
    switch(type)
    {
    case VectorType:
        return SharedVectorVariableT(new VectorVariable(QVector<double>(), 0, pVarDesc, SharedMultiDataVectorCacheT()));
    case TimeDomainType:
        return SharedVectorVariableT(new TimeDomainVariable(createFreeTimeVectorVariabel(QVector<double>()), QVector<double>(), 0, pVarDesc, SharedMultiDataVectorCacheT()));
    case FrequencyDomainType:
        return SharedVectorVariableT(new FrequencyDomainVariable(createFreeFrequencyVectorVariabel(QVector<double>()), QVector<double>(), 0, pVarDesc, SharedMultiDataVectorCacheT()));
    default:
        //! @todo support all types
        // Not yet supported by factory
        return SharedVectorVariableT();
    }
}

//! @todo this should not be here should be togheter with plotsvariable stuf in some other file later
QString makeConcatName(const QString componentName, const QString portName, const QString dataName)
{
    if (componentName.isEmpty() && portName.isEmpty())
    {

        return dataName;
    }
    else
    {
        //! @todo default separator should be DEFINED
        return componentName+"#"+portName+"#"+dataName;
    }
    return "ERRORinConcatName";
}

//! @todo this should not be here should be togheter with plotsvariable stuf in some other file later
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName)
{
    rCompName.clear();
    rPortName.clear();
    rVarName.clear();
    QStringList slist = fullName.split('#');
    if (slist.size() == 1)
    {
        rVarName = slist[0];
    }
    else if (slist.size() == 3)
    {
        rCompName = slist[0];
        rPortName = slist[1];
        rVarName = slist[2];
    }
    else
    {
        rVarName = "ERRORinsplitConcatName";
    }
}

QString VariableDescription::getFullName() const
{
    return makeConcatName(mComponentName,mPortName,mDataName);
}

QString VariableDescription::getFullNameWithSeparator(const QString sep) const
{
    if (mComponentName.isEmpty())
    {
        return mDataName;
    }
    else
    {
        return mComponentName+sep+mPortName+sep+mDataName;
    }
}

void VariableDescription::setFullName(const QString compName, const QString portName, const QString dataName)
{
    mComponentName = compName;
    mPortName = portName;
    mDataName = dataName;
}


void VectorVariable::setPlotOffset(double offset)
{
    mDataPlotOffset = offset;
    emit dataChanged();
}

void VectorVariable::setPlotScaleAndOffset(const double scale, const double offset)
{
    mCustomUnitScale.setOnlyScale(scale);
    mDataPlotOffset = offset;
    emit dataChanged();
}

void VectorVariable::setTimePlotOffset(double offset)
{
    Q_UNUSED(offset);
    // Do nothing by default
}

void VectorVariable::setTimePlotScale(double scale)
{
    Q_UNUSED(scale);
    // Do nothing by default
}

void VectorVariable::setTimePlotScaleAndOffset(const double scale, const double offset)
{
    Q_UNUSED(scale);
    Q_UNUSED(offset);
    // Do nothing by default
}

void VectorVariable::setPlotScale(double scale)
{
    mCustomUnitScale.setOnlyScale(scale);
    emit dataChanged();
}

VectorVariable::VectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache)
{
    mpVariableDescription = varDesc;
    mDataPlotOffset = 0.0;
    mGeneration = generation;
    mpCachedDataVector = new CachableDataVector(rData, pGenerationMultiCache, gpConfig->getCacheLogData());
}

VectorVariable::~VectorVariable()
{
    if (mpCachedDataVector != 0)
    {
        delete mpCachedDataVector;
    }
}

void VectorVariable::replaceSharedTFVector(SharedVectorVariableT pToFVector)
{
    // Disconnect the old one
    if (mpSharedTimeOrFrequencyVector)
    {
        mpSharedTimeOrFrequencyVector.data()->disconnect(this, 0);
    }

    // Assign
    mpSharedTimeOrFrequencyVector = pToFVector;

    // Connect to the new one
    if (mpSharedTimeOrFrequencyVector)
    {
        connect(mpSharedTimeOrFrequencyVector.data(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()), Qt::UniqueConnection);
    }
}

const SharedVariableDescriptionT VectorVariable::getVariableDescription() const
{
    return mpVariableDescription;
}

VariableSourceTypeT VectorVariable::getVariableSourceType() const
{
    return mpVariableDescription->mVariableSourceType;
}

VariableTypeT VectorVariable::getVariableType() const
{
    return VectorType;
}

const QString &VectorVariable::getAliasName() const
{
    return mpVariableDescription->mAliasName;
}

QString VectorVariable::getFullVariableName() const
{
    return mpVariableDescription->getFullName();
}

QString VectorVariable::getFullVariableNameWithSeparator(const QString sep) const
{
    return mpVariableDescription->getFullNameWithSeparator(sep);
}

QString VectorVariable::getSmartName() const
{
    if (mpVariableDescription->mAliasName.isEmpty())
    {
        return mpVariableDescription->getFullName();
    }
    else
    {
        return mpVariableDescription->mAliasName;
    }
}

const QString &VectorVariable::getModelPath() const
{
    return mpVariableDescription->mModelPath;
}

const QString &VectorVariable::getComponentName() const
{
    return mpVariableDescription->mComponentName;
}

const QString &VectorVariable::getPortName() const
{
    return mpVariableDescription->mPortName;
}

const QString &VectorVariable::getDataName() const
{
    return mpVariableDescription->mDataName;
}

const QString &VectorVariable::getDataUnit() const
{
    return mpVariableDescription->mDataUnit;
}

const QString &VectorVariable::getCustomLabel() const
{
    return mpVariableDescription->mCustomLabel;
}

const QString &VectorVariable::getPlotScaleDataUnit() const
{
    return mCustomUnitScale.mUnit;
}

const QString &VectorVariable::getActualPlotDataUnit() const
{
    if (mCustomUnitScale.isEmpty())
    {
        return getDataUnit();
    }
    else
    {
        return mCustomUnitScale.mUnit;
    }
}

bool VectorVariable::hasAliasName() const
{
    return !mpVariableDescription->mAliasName.isEmpty();
}

bool VectorVariable::hasCustomLabel() const
{
    return !mpVariableDescription->mCustomLabel.isEmpty();
}

int VectorVariable::getGeneration() const
{
    return mGeneration;
}

bool VectorVariable::isImported() const
{
    return false;
}

QString VectorVariable::getImportedFileName() const
{
    return QString();
}


const SharedVectorVariableT VectorVariable::getSharedTimeOrFrequencyVector() const
{
    return mpSharedTimeOrFrequencyVector;
}

double VectorVariable::getPlotOffset() const
{
    return mDataPlotOffset;
}

void VectorVariable::addToData(const SharedVectorVariableT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
}
void VectorVariable::addToData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}
void VectorVariable::subFromData(const SharedVectorVariableT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += -pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}
void VectorVariable::subFromData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += -other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void VectorVariable::multData(const SharedVectorVariableT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] *= pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void VectorVariable::multData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] *= other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void VectorVariable::divData(const SharedVectorVariableT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] /= pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void VectorVariable::divData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] /= other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}


void VectorVariable::absData()
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
        (*pData)[i] = fabs((*pData)[i]);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void VectorVariable::diffBy(SharedVectorVariableT pOther)
{
    if(pOther)
    {
        // Get data vectors
        DataVectorT* pThisData = mpCachedDataVector->beginFullVectorOperation();
        DataVectorT* pOtherData = pOther->beginFullVectorOperation(); //!< @todo it would be nice to be able to check out read only

        // Check so that vectors have same size
        if (pThisData->size() != pOtherData->size())
        {
            // Abort
            //! @todo error message
            return;
        }

        // Performe diff operation
        for(int i=0; i<pThisData->size()-1; ++i)
        {
            (*pThisData)[i] = ((*pThisData)[i+1]-(*pThisData)[i])/((*pOtherData)[i+1]-(*pOtherData)[i]);
        }
        pThisData->resize(pThisData->size()-1);

        // Return data vectors
        pOther->endFullVectorOperation(pOtherData);
        mpCachedDataVector->endFullVectorOperation(pThisData);

        emit dataChanged();
    }
    else
    {
        //! @todo error message
        // Abort
        return;
    }
}

void VectorVariable::integrateBy(SharedVectorVariableT pOther)
{
    if(pOther)
    {
        // Get data pointers
        DataVectorT* pThisData = mpCachedDataVector->beginFullVectorOperation();
        DataVectorT* pOtherData = pOther->beginFullVectorOperation();

        // Check so that vectors have same size
        if (pThisData->size() != pOtherData->size())
        {
            // Abort
            //! @todo error message
            return;
        }

        // Run integration operation
        QVector<double> res;
        res.reserve(pThisData->size());
        res.append(0);
        for (int i=1; i<pThisData->size(); ++i)
        {
            res.append( res[i-1] + 0.5*( (*pOtherData)[i] - (*pOtherData)[i-1] )*( (*pThisData)[i-1] + (*pThisData)[i]) );
        }
        *pThisData = res;

        // Return data pointers
        pOther->endFullVectorOperation(pOtherData);
        mpCachedDataVector->endFullVectorOperation(pThisData);

        emit dataChanged();
    }
    else
    {
        //! @todo Error message
        // Abort
        return;
    }
}

void VectorVariable::lowPassFilter(SharedVectorVariableT pTime, const double w)
{
    if(pTime)
    {
        // Get data vector pointers
        DataVectorT* pThisData = mpCachedDataVector->beginFullVectorOperation();
        DataVectorT* pTimeData = pTime->beginFullVectorOperation();

        // Check so that vectors have same size
        if (pThisData->size() != pTimeData->size())
        {
            // Abort
            //! @todo error message
            return;
        }

        // Run the lp operation
        double Al = 2.0/(2.0*M_PI*w);
        double temp1 = (*pThisData)[0];
        double temp = temp1;
        (*pThisData)[0] = temp;
        for(int i=1; i<pThisData->size(); ++i)
        {
            double T = (*pTimeData)[i]-(*pTimeData)[i-1];
            double ALF = Al/T;
            double G = 1.0+ALF;
            double A1 = (1.0-ALF)/G;
            double B1 = 1.0/G;
            temp = temp1;
            temp1 = (*pThisData)[i];
            (*pThisData)[i] = -A1*(*pThisData)[i-1] + B1*(temp1+temp);
        }

        // Return data ptrs
        pTime->endFullVectorOperation(pTimeData);
        mpCachedDataVector->endFullVectorOperation(pThisData);

        emit dataChanged();
    }
    else
    {
        //! @todo error message
        // Abort
        return;
    }
}

SharedVectorVariableT VectorVariable::toFrequencySpectrum(const SharedVectorVariableT pTime, const bool doPowerSpectrum)
{
    if(pTime)
    {
        // Fetch data
        DataVectorT time, data;
        mpCachedDataVector->copyDataTo(data);
        time = pTime->getDataVectorCopy();

        // Check so that vectors have same size
        if (data.size() != time.size())
        {
            // Abort
            //! @todo error message
            return SharedVectorVariableT();
        }

        // Vector size has to be an even potential of 2.
        // Calculate largets potential that is smaller than or equal to the vector size.
#ifndef __APPLE__
        const int n = pow(2, int(log2(data.size()))); // This is odd.... /magse
#else
        const int n = (int)round(ldexp(2.0, int(log2(data.size()))));
#endif
        if(n != data.size())     // Vector is not an exact potential, so reduce it
        {
            QString oldString, newString;
            oldString.setNum(data.size());
            newString.setNum(n);
            QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Wrong Vector Size"),
                                     gpMainWindowWidget->tr("Size of data vector must be an even power of 2. Number of log samples was reduced from ") + oldString + gpMainWindowWidget->tr(" to ") + newString + ".");
            reduceVectorSize(data, n);
            reduceVectorSize(time, n);
        }

        // Create a complex vector
        QVector< std::complex<double> > vComplex = realToComplex(data);

        // Apply the fourier transform
        FFT(vComplex);

        // Scalar multiply complex vector with its conjugate, and divide it with its size
        // Alos build frequency vector
        DataVectorT freq, mag;
        freq.reserve(n/2);
        mag.reserve(n/2);
        const double maxt = time.last();

        // FFT is symmetric, so only use first half
        // Also skip f=0, but include n/2 (nyquist)
        for(int i=1; i<=n/2; ++i)
        {
            if(doPowerSpectrum)
            {
                mag.append(real(vComplex[i]*conj(vComplex[i]))/double(n));
            }
            else
            {
                mag.append(std::abs(vComplex[i])/double(n));
            }

            // Build freq vector
            freq.append(double(i)/maxt);
        }

        SharedVariableDescriptionT pDesc(new VariableDescription());
        pDesc->mCustomLabel = mpVariableDescription->getFullNameWithSeparator(",");
        if (doPowerSpectrum)
        {
            pDesc->mDataName = "Power Spectrum";
        }
        else
        {
            pDesc->mDataName = "Relative Magnitude";
        }

        //! @todo we may need to change description information for this variable to avoid trouble
        return SharedVectorVariableT(new FrequencyDomainVariable(createFreeFrequencyVectorVariabel(freq), mag, this->getGeneration(), pDesc, SharedMultiDataVectorCacheT()));
    }
    else
    {
        //! @todo error message
        // Abort
        return SharedVectorVariableT();
    }
}


void VectorVariable::assignFrom(const SharedVectorVariableT pOther)
{
    mpCachedDataVector->replaceData(pOther->getDataVectorCopy());
    emit dataChanged();
}

void VectorVariable::assignFrom(const QVector<double> &rSrc)
{
    mpCachedDataVector->replaceData(rSrc);
    emit dataChanged();
}

void VectorVariable::assignFrom(const double src)
{
    assignFrom(QVector<double>() << src);
}

void VectorVariable::assignFrom(SharedVectorVariableT time, const QVector<double> &rData)
{
    Q_UNUSED(time);
    // By default we do not have a time vector so lets just assign the data
    //! @todo maybe should give error or warnign
    assignFrom(rData);
}

void VectorVariable::assignFrom(const QVector<double> &rTime, const QVector<double> &rData)
{
    Q_UNUSED(rTime);
    // By default we do not have a time vector so lets just assign the data
    //! @todo maybe should give error or warnign
    assignFrom(rData);
}

double VectorVariable::pokeData(const int index, const double value, QString &rErr)
{
    if (indexInRange(index))
    {
        if (mpCachedDataVector->poke(index, value))
        {
            emit dataChanged();
            double val;
            mpCachedDataVector->peek(index, val);
            return val;
        }
        rErr = mpCachedDataVector->getError();
        return -1;
    }
    rErr = "Index out of range";
    return -1;
}

double VectorVariable::peekData(const int index, QString &rErr) const
{
    double val = -1;
    if (indexInRange(index))
    {
        if (!mpCachedDataVector->peek(index, val))
        {
            rErr = mpCachedDataVector->getError();
        }
        return val;
    }
    rErr = "Index out of range";
    return val;
}

double VectorVariable::averageOfData() const
{
    double ret = 0;
    int i=0;
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(; i<pVector->size(); ++i)
    {
        ret += pVector->at(i);
    }
    ret /= i;
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double VectorVariable::minOfData(int &rIdx) const
{
    rIdx = -1;
    double ret = std::numeric_limits<double>::max();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    if (pVector)
    {
        for(int i=0; i<pVector->size(); ++i)
        {
            const double &v = (*pVector)[i];
            if(v < ret)
            {
                ret = v;
                rIdx=i;
            }
        }
        mpCachedDataVector->endFullVectorOperation(pVector);
    }
    return ret;
}

double VectorVariable::minOfData() const
{
    int dummy;
    return minOfData(dummy);
}


void VectorVariable::elementWiseGt(QVector<double> &rResult, const double threshold) const
{
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    rResult.resize(pVector->size());
    for(int i=0; i<pVector->size(); ++i)
    {
        if ((*pVector)[i] > threshold)
        {
            rResult[i] = 1;
        }
        else
        {
            rResult[i] = 0;
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
}

void VectorVariable::elementWiseLt(QVector<double> &rResult, const double threshold) const
{
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    rResult.resize(pVector->size());
    for(int i=0; i<pVector->size(); ++i)
    {
        if ((*pVector)[i] < threshold)
        {
            rResult[i] = 1;
        }
        else
        {
            rResult[i] = 0;
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
}

bool VectorVariable::compare(SharedVectorVariableT pOther, const double eps) const
{
    //! @todo right now we compare the data vectors we do not care about the type of variable (timedomain frequency domain and so on) maybe we should? /Peter
    bool isOK=false;
    if (this->getDataSize() == pOther->getDataSize())
    {
        QVector<double> *pThisData = mpCachedDataVector->beginFullVectorOperation();
        QVector<double> *pOtherData = pOther->beginFullVectorOperation();
        if (pThisData && pOtherData)
        {
            isOK=true;
            for (int i=0; i<pThisData->size(); ++i)
            {
                if (!fuzzyEqual((*pThisData)[i], (*pOtherData)[i], eps))
                {
                    isOK = false;
                    break;
                }
            }
        }
        pOther->endFullVectorOperation(pOtherData);
        mpCachedDataVector->endFullVectorOperation(pThisData);
    }
    return isOK;
}

QVector<double> *VectorVariable::beginFullVectorOperation()
{
    return mpCachedDataVector->beginFullVectorOperation();
}

bool VectorVariable::endFullVectorOperation(QVector<double> *&rpData)
{
    return mpCachedDataVector->endFullVectorOperation(rpData);
}


//! @brief Appends one point to a curve, NEVER USE THIS UNLESS A CUSTOM (PRIVATE) X (TIME) VECTOR IS USED!
void VectorVariable::append(const double t, const double y)
{
    Q_UNUSED(t);
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
}

//! @brief Appends one point to a curve, NEVER USE THIS WHEN A SHARED TIMEVECTOR EXIST
void VectorVariable::append(const double y)
{
    //! @todo smarter append regardless of cached or not
    //! @todo mayebe a reserve function to reserve memory if we know how much to expect
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}


double VectorVariable::maxOfData(int &rIdx) const
{
    rIdx = -1;
    double ret = -std::numeric_limits<double>::max();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    if (pVector)
    {
        for(int i=0; i<pVector->size(); ++i)
        {
            const double &v = (*pVector)[i];
            if(v > ret)
            {
                ret = v;
                rIdx = i;
            }
        }
        mpCachedDataVector->endFullVectorOperation(pVector);
    }
    return ret;
}

double VectorVariable::maxOfData() const
{
    int dummy;
    return maxOfData(dummy);
}

void VectorVariable::minMaxOfData(double &rMin, double &rMax, int &rMinIdx, int &rMaxIdx) const
{
    rMinIdx = -1;
    rMaxIdx = -1;
    rMin = std::numeric_limits<double>::max();
    rMax = -rMin;

    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    if (pVector)
    {
        for(int i=0; i<pVector->size(); ++i)
        {
            const double &v = (*pVector)[i];
            if(v < rMin)
            {
                rMin = v;
                rMinIdx = i;
            }
            if(v > rMax)
            {
                rMax = v;
                rMaxIdx = i;
            }
        }
        mpCachedDataVector->endFullVectorOperation(pVector);
    }
}

bool VectorVariable::positiveNonZeroMinMaxOfData(double &rMin, double &rMax, int &rMinIdx, int &rMaxIdx) const
{
    rMinIdx = -1;
    rMaxIdx = -1;
    rMin = std::numeric_limits<double>::max();
    rMax = std::numeric_limits<double>::epsilon();

    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    if (pVector)
    {
        for(int i=0; i<pVector->size(); ++i)
        {
            const double &v = (*pVector)[i];
            if( (v < rMin) && (v > std::numeric_limits<double>::epsilon()) )
            {
                rMin = v;
                rMinIdx = i;
            }
            if( v > rMax)
            {
                rMax = v;
                rMaxIdx = i;
            }
        }
        mpCachedDataVector->endFullVectorOperation(pVector);
    }
    return ((rMinIdx > -1) && (rMaxIdx>-1));
}

void VectorVariable::preventAutoRemoval()
{
    emit allowAutoRemove(mGeneration, false);
}

void VectorVariable::allowAutoRemoval()
{
    emit allowAutoRemove(mGeneration, true);
}

void VectorVariable::setCacheDataToDisk(const bool toDisk)
{
    mpCachedDataVector->setCached(toDisk);
}

bool VectorVariable::isCachingDataToDisk() const
{
    return mpCachedDataVector->isCached();
}

bool VectorVariable::indexInRange(const int idx) const
{
    //! @todo Do we need to check timevector also ? (or should we assume thay are the same)
    return (idx>=0 && idx<mpCachedDataVector->size());
}

QPointer<LogDataHandler> VectorVariable::getLogDataHandler()
{
    return mpParentLogDataHandler;
}


const QString &VectorVariableContainer::getName() const
{
    return mName;
}


void VectorVariableContainer::allowGenerationAutoRemoval(int gen, bool allow)
{
    if (allow)
    {
        //! @todo maybe special -1 input for ALL
        mKeepGenerations.removeOne(gen);
    }
    else
    {
        //! @todo what happens if we tell it to keep a generation it does not have
        //! @todo maybe special -1 input for ALL
        if (!mKeepGenerations.contains(gen))
        {
            mKeepGenerations.prepend(gen);
        }
    }
}


LogDataHandler *VectorVariableContainer::getLogDataHandler()
{
    return mpParentLogDataHandler;
}

void VectorVariableContainer::actuallyRemoveDataGen(GenerationMapT::iterator git)
{
    bool isAlias=false;
    // Disconnect signals from data to this container
    git.value().data()->disconnect(this,0);
    // Remove from alias and imported registers if needed
    if (mAliasGenIndexes.contains(git.key()))
    {
        mAliasGenIndexes.removeValue(git.key());
        isAlias = true;
    }
    if (mImportedGenIndexes.contains(git.key()))
    {
        mImportedGenIndexes.removeValue(git.key());
        if (!isAlias)
        {
            emit importedVariableBeingRemoved(git.value());
        }
    }
    if (mNonImportedGenIndexes.contains(git.key()))
    {
        mNonImportedGenIndexes.removeValue(git.key());
    }
    mDataGenerations.erase(git);
}

//! @brief This function converts a VariableSourceTypeT enum into a human readable string
QString variableSourceTypeAsString(const VariableSourceTypeT type)
{
    switch (type)
    {
    case ScriptVariableType :
        return "ScriptVariable";
        break;
    case ModelVariableType :
        return "ModelVariable";
        break;
    case ImportedVariableType :
        return "ImportedVariable";
        break;
    default :
        return "UndefinedVariableSource";
    }
}

//! @brief This function converts a VariableSourceTypeT enum into a human readable short string
QString variableSourceTypeAsShortString(const VariableSourceTypeT type)
{
    switch (type)
    {
    case ScriptVariableType :
        return "S";
        break;
    case ModelVariableType :
        return "M";
        break;
    case ImportedVariableType :
        return "I";
        break;
    default :
        return "U";
    }
}


QString variableTypeAsString(const VariableTypeT type)
{
    //! @todo add all of them
    //RealFrequencyDomainType, ImaginaryFrequencyDomainType, AmplitudeFrequencyDomainType, PhaseFrequencyDomainType,
    switch (type)
    {
    case VectorType :
        return "VectorType";
        break;
    case TimeDomainType :
        return "TimeDomainType";
        break;
    case FrequencyDomainType :
        return "FrequencyDomainType";
        break;
    case ComplexType :
        return "ComplexType";
        break;
    default :
        return "UndefinedVariableType";
    }
}



int VectorVariableContainer::getLowestGeneration() const
{
    if (mDataGenerations.empty())
    {
        return -1;
    }
    else
    {
        return mDataGenerations.begin().key();
    }
}

int VectorVariableContainer::getHighestGeneration() const
{
    if (mDataGenerations.empty())
    {
        return -1;
    }
    else
    {
        return (--mDataGenerations.end()).key();
    }
}

int VectorVariableContainer::getNumGenerations() const
{
    return mDataGenerations.size();
}

QList<int> VectorVariableContainer::getGenerations() const
{
    return mDataGenerations.keys();
}

int VectorVariableContainer::getNewestImportedGeneration() const
{
    return mImportedGenIndexes.max();
}

int VectorVariableContainer::getNewestNonImportedGeneration() const
{
    return mNonImportedGenIndexes.max();
}

int VectorVariableContainer::getNewestAliasGeneration() const
{
    return mAliasGenIndexes.max();
}

bool VectorVariableContainer::isStoringAlias() const
{
    return !mAliasGenIndexes.isEmpty();
}

bool VectorVariableContainer::isGenerationAlias(const int gen) const
{
    if (gen<0)
    {
        return mAliasGenIndexes.contains(getHighestGeneration());
    }
    else
    {
        return mAliasGenIndexes.contains(gen);
    }
}

bool VectorVariableContainer::isStoringImported() const
{
    return !mImportedGenIndexes.isEmpty();
}

bool VectorVariableContainer::isGenerationImported(const int gen) const
{
    if (gen<0)
    {
        return mImportedGenIndexes.contains(getHighestGeneration());
    }
    else
    {
        return mImportedGenIndexes.contains(gen);
    }
}

bool VectorVariableContainer::isStoringNonImported() const
{
    return !mNonImportedGenIndexes.isEmpty();
}

bool VectorVariableContainer::isGenerationNonImported(const int gen) const
{
    return mNonImportedGenIndexes.contains(gen);
}

SharedVectorVariableT VectorVariableContainer::getDataGeneration(const int gen) const
{
    // If generation not specified (<0), then take latest (if not empty),
    if ( (gen < 0) && !mDataGenerations.empty() )
    {
        return (--mDataGenerations.end()).value();
    }

    // Else try to find specified generation
    // Return 0 ptr if generation not found
    return mDataGenerations.value(gen, SharedVectorVariableT(0));
}

SharedVectorVariableT VectorVariableContainer::getNonAliasDataGeneration(int gen) const
{
    // We need to know the generation to check
    if (gen<0)
    {
        gen = getHighestGeneration();
    }

    if (!isGenerationAlias(gen))
    {
        return getDataGeneration(gen);
    }

    // If we can not find one, then return 0 ptr
    return SharedVectorVariableT();

}

QList<SharedVectorVariableT> VectorVariableContainer::getAllDataGenerations() const
{
    return mDataGenerations.values();
}

bool VectorVariableContainer::hasDataGeneration(const int gen)
{
    return mDataGenerations.contains(gen);
}

//! @brief Adds or replaces a data generation
void VectorVariableContainer::addDataGeneration(const int generation, SharedVectorVariableT pData)
{
    // Set generation that was set by LogvariableData constructor when creating a new variable, in this case we need to overwrite
    pData->mGeneration = generation;
    // Insert into generation storage
    mDataGenerations.insert(generation, pData);
    // Remember if alias
    if (pData->hasAliasName() && pData->getAliasName() == mName)
    {
        mAliasGenIndexes.addValue(generation);
    }
    // Remember if imported
    if (pData->isImported())
    {
        mImportedGenIndexes.addValue(generation);
    }
    else
    // Remember non-imported variables
    {
        mNonImportedGenIndexes.addValue(generation);
    }


    // Connect signals from data
    connect(pData.data(), SIGNAL(allowAutoRemove(int,bool)), this, SLOT(allowGenerationAutoRemoval(int,bool)), Qt::UniqueConnection);

    // Emit notification of new generation
    emit generationAdded();
}

//! @brief Removes a generation of the variable
//! @note If last generation the container itself will be deletet from parent log data handler, so DO NOT CALL this while itterating through the log data map
//! @todo this functions should not call delete in parent if empty, it causes difficult to debugg problems while calling it during itteration, need to come up with a smarter solution
//! @returns True if the generation was removed, otherwise false (if generation was not present or taged as keep (when not forcing)
bool VectorVariableContainer::removeDataGeneration(const int generation, const bool force)
{
    bool didRemove=false;
    // Skip removal of generations that should be kept
    if (mKeepGenerations.contains(generation))
    {
        if (force)
        {
            // We use find to search only once, (and reuse iterator)
            GenerationMapT::iterator git = mDataGenerations.find(generation);
            if (git != mDataGenerations.end())
            {
                actuallyRemoveDataGen(git);
                didRemove=true;
            }
            mKeepGenerations.removeOne(generation);
        }
    }
    else
    {
        // We use find to search only once, (and reuse iterator)
        GenerationMapT::iterator git = mDataGenerations.find(generation);
        if (git != mDataGenerations.end())
        {
            actuallyRemoveDataGen(git);
            didRemove=true;
        }
    }

    // If last data generation removed then ask my parent to delete me
    // NOTE! The parent must use deleteLater, else this will crash
    if (mDataGenerations.isEmpty())
    {
        mpParentLogDataHandler->deleteVariable(mName);
    }
    return didRemove;
}

//! @brief Limit the number of generations within the given interval
//! @returns True if something was removed else false
bool VectorVariableContainer::purgeOldGenerations(const int purgeEnd, const int nGensToKeep)
{
    bool didRemove = false;
    // Only do the purge if mingen is under upper limit
    int minGen = getLowestGeneration();
    if (minGen <= purgeEnd)
    {
        // loop through keys
        const int nTaggedKeep = mKeepGenerations.size();
        QList<int> keys = mDataGenerations.keys();
        for (int k=0; k<keys.size(); ++k)
        {
            // Only break loop when we have deleted all below purge limit or when total number of generations is less then the desired (+ those we want to keep)
            if ((keys[k] > purgeEnd) || (mDataGenerations.size() < (nGensToKeep+nTaggedKeep)) )
            {
                break;
            }
            else
            {
                // Try to remove each generation
                didRemove += removeDataGeneration(keys[k], false);
            }
        }
    }
    return didRemove;
}

void VectorVariableContainer::removeAllGenerations()
{
    // It is assumed that the generation map is sorted by key which it should be since adding will allways append
    QList<int> gens = mDataGenerations.keys();
    for (int it=0; it<gens.size(); ++it)
    {
        removeDataGeneration(gens[it]);
    }
}

bool VectorVariableContainer::removeAllImportedGenerations()
{
    bool didRemove=false;
    QList<int> gens = mDataGenerations.keys();
    for (int it=0; it<gens.size(); ++it)
    {
        if (mDataGenerations[gens[it]]->isImported())
        {
            didRemove += removeDataGeneration(gens[it], true);
        }
    }
    return didRemove;
}

VectorVariableContainer::VectorVariableContainer(const QString &rName, LogDataHandler *pParentLogDataHandler) : QObject()
{
    mName = rName;
    mpParentLogDataHandler = pParentLogDataHandler;
}

VectorVariableContainer::~VectorVariableContainer()
{
    // Clear all data
    mDataGenerations.clear();
}

QVector<double> VectorVariable::getDataVectorCopy() const
{
    QVector<double> vec;
    mpCachedDataVector->copyDataTo(vec);
    return vec;
}

void VectorVariable::sendDataToStream(QTextStream &rStream, QString separator)
{
    mpCachedDataVector->streamDataTo(rStream, separator);
}

int VectorVariable::getDataSize() const
{
    return mpCachedDataVector->size();
}

double VectorVariable::first() const
{
    return peekData(0);
}

double VectorVariable::last() const
{
    return peekData(mpCachedDataVector->size()-1);
}



double VectorVariable::peekData(const int idx) const
{
    double val = -1;
    if (indexInRange(idx))
    {
        mpCachedDataVector->peek(idx, val);
    }
    return val;
}

//! @brief Returns the custom plot scale or 1 if not plotscale set.
double VectorVariable::getPlotScale() const
{
    return mCustomUnitScale.toDouble(1.0);
}

void VectorVariable::setCustomUnitScale(const UnitScale &rUnitScale)
{
    mCustomUnitScale = rUnitScale;
    emit dataChanged();
}

const UnitScale &VectorVariable::getCustomUnitScale() const
{
    return mCustomUnitScale;
}

void VectorVariable::removeCustomUnitScale()
{
    mCustomUnitScale.clear();
    emit dataChanged();
}


TimeDomainVariable::TimeDomainVariable(SharedVectorVariableT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(rData, generation, varDesc, pGenerationMultiCache)
{
    replaceSharedTFVector(time);
}

VariableTypeT TimeDomainVariable::getVariableType() const
{
    return TimeDomainType;
}

void TimeDomainVariable::diffBy(SharedVectorVariableT pOther)
{
    // Choose other data or own time vector
    if(pOther.isNull())
    {
        // If no diff vector supplied, use own time
        if (mpSharedTimeOrFrequencyVector)
        {
            VectorVariable::diffBy(mpSharedTimeOrFrequencyVector);
            //! @todo if successfull we need to make our time vector shorter by one
        }
        else
        {
            // Abort
            //! @todo error message
            return;
        }
    }
    else
    {
        VectorVariable::diffBy(pOther);
    }
}

void TimeDomainVariable::integrateBy(SharedVectorVariableT pOther)
{
    // Choose other data or own time vector
    if(pOther.isNull())
    {
        // If no diff vector supplied, use own time
        if (mpSharedTimeOrFrequencyVector)
        {
            VectorVariable::integrateBy(mpSharedTimeOrFrequencyVector);
        }
        else
        {
            // Abort
            //! @todo error message
            return;
        }
    }
    else
    {
        VectorVariable::integrateBy(pOther);
    }
}

void TimeDomainVariable::lowPassFilter(SharedVectorVariableT pTime, const double w)
{
    // Choose other data or own time vector
    if(pTime.isNull())
    {
        // If no diff vector supplied, use own time
        if (mpSharedTimeOrFrequencyVector)
        {
            VectorVariable::lowPassFilter(mpSharedTimeOrFrequencyVector, w);
        }
        else
        {
            // Abort
            //! @todo error message
            return;
        }
    }
    else
    {
        VectorVariable::lowPassFilter(pTime, w);
    }
}

SharedVectorVariableT TimeDomainVariable::toFrequencySpectrum(const SharedVectorVariableT pTime, const bool doPowerSpectrum)
{
    // Choose other data or own time vector
    if(pTime.isNull())
    {
        // If no diff vector supplied, use own time
        if (mpSharedTimeOrFrequencyVector)
        {
            return VectorVariable::toFrequencySpectrum(mpSharedTimeOrFrequencyVector, doPowerSpectrum);
        }
        else
        {
            // Abort
            //! @todo error message
            return SharedVectorVariableT();
        }
    }
    else
    {
        return VectorVariable::toFrequencySpectrum(pTime, doPowerSpectrum);
    }
}

void TimeDomainVariable::assignFrom(const SharedVectorVariableT pOther)
{
    replaceSharedTFVector(pOther->getSharedTimeOrFrequencyVector());
    VectorVariable::assignFrom(pOther);
}

void TimeDomainVariable::assignFrom(SharedVectorVariableT time, const QVector<double> &rData)
{
    mpCachedDataVector->replaceData(rData);
    replaceSharedTFVector(time);
    emit dataChanged();
}

void TimeDomainVariable::assignFrom(const QVector<double> &rTime, const QVector<double> &rData)
{
    // We create a new non managed free timevector from the supplied time data
    assignFrom(createFreeTimeVectorVariabel(rTime), rData);
}

//! @brief Appends one point to a curve, NEVER USE THIS UNLESS A CUSTOM (PRIVATE) X (TIME) VECTOR IS USED!
//! @todo we need som kind of differnt variable typ for this
void TimeDomainVariable::append(const double t, const double y)
{
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
    mpSharedTimeOrFrequencyVector->append(t);
    //! @todo FIXA, it is bad to append x-data to shared time vector, there should be a custom private xvector Peter
}

void TimeDomainVariable::setTimePlotScaleAndOffset(const double scale, const double offset)
{
    if (mpSharedTimeOrFrequencyVector)
    {
        mpSharedTimeOrFrequencyVector->setPlotScaleAndOffset(scale, offset);
    }
}

void TimeDomainVariable::setTimePlotScale(double scale)
{
    if (mpSharedTimeOrFrequencyVector)
    {
        mpSharedTimeOrFrequencyVector->setPlotScale(scale);
    }
}

void TimeDomainVariable::setTimePlotOffset(double offset)
{
    if (mpSharedTimeOrFrequencyVector)
    {
        mpSharedTimeOrFrequencyVector->setPlotOffset(offset);
    }
}


ImportedTimeDomainVariable::ImportedTimeDomainVariable(SharedVectorVariableT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    TimeDomainVariable(time, rData, generation, varDesc, pGenerationMultiCache)
{
    mImportFileName = rImportFile;
}

bool ImportedTimeDomainVariable::isImported() const
{
    return true;
}

QString ImportedTimeDomainVariable::getImportedFileName() const
{
    return mImportFileName;
}

bool ImportedVectorVariable::isImported() const
{
    return true;
}

QString ImportedVectorVariable::getImportedFileName() const
{
    return mImportFileName;
}



ComplexVectorVariable::ComplexVectorVariable(const QVector<double> &rReal, const QVector<double> &rImaginary, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(QVector<double>(0), generation, varDesc, pGenerationMultiCache)
{
    SharedVariableDescriptionT pRealDesc(new VariableDescription());
    pRealDesc->mDataName = "Real";
    mpSharedReal = SharedVectorVariableT(new VectorVariable(rReal, generation, pRealDesc, pGenerationMultiCache));

    SharedVariableDescriptionT pImagDesc(new VariableDescription());
    pImagDesc->mDataName = "Imaginary";
    mpSharedImag = SharedVectorVariableT(new VectorVariable(rImaginary, generation, pImagDesc, pGenerationMultiCache));
}

ComplexVectorVariable::ComplexVectorVariable(SharedVectorVariableT pReal, SharedVectorVariableT pImaginary, const int generation, SharedVariableDescriptionT varDesc)
    : VectorVariable(QVector<double>(0), generation, varDesc, SharedMultiDataVectorCacheT())
{
    mpSharedReal = pReal;
    mpSharedImag = pImaginary;
}

VariableTypeT ComplexVectorVariable::getVariableType() const
{
    return ComplexType;
}

const QString &ComplexVectorVariable::getDataName() const
{
    return mpSharedImag->getDataName();
}

const SharedVectorVariableT ComplexVectorVariable::getSharedTimeOrFrequencyVector() const
{
    return mpSharedReal;
}

QVector<double> ComplexVectorVariable::getRealDataCopy() const
{
    return mpSharedReal->getDataVectorCopy();
}

QVector<double> ComplexVectorVariable::getImagDataCopy() const
{
    return mpSharedImag->getDataVectorCopy();
}


FrequencyDomainVariable::FrequencyDomainVariable(SharedVectorVariableT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(rData, generation, varDesc, pGenerationMultiCache)
{
    replaceSharedTFVector(frequency);
}

VariableTypeT FrequencyDomainVariable::getVariableType() const
{
    return FrequencyDomainType;
}

void FrequencyDomainVariable::assignFrom(const SharedVectorVariableT pOther)
{
    replaceSharedTFVector(pOther->getSharedTimeOrFrequencyVector());
    VectorVariable::assignFrom(pOther);
}

void FrequencyDomainVariable::assignFrom(SharedVectorVariableT freq, const QVector<double> &rData)
{
    mpCachedDataVector->replaceData(rData);
    replaceSharedTFVector(freq);
    emit dataChanged();
}

void FrequencyDomainVariable::assignFrom(const QVector<double> &rFreq, const QVector<double> &rData)
{
    // We create a new non managed free frequency vector from the supplied frequency data
    assignFrom(createFreeFrequencyVectorVariabel(rFreq), rData);
}

ImportedVectorVariable::ImportedVectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(rData, generation, varDesc, pGenerationMultiCache)
{
    mImportFileName = rImportFile;
}


void createBodeVariables(const SharedVectorVariableT pInput, const SharedVectorVariableT pOutput, int Fmax, SharedVectorVariableT &rNyquistData, SharedVectorVariableT &rNyquistDataInv, SharedVectorVariableT &rGainData, SharedVectorVariableT &rPhaseData)
{
    // Create temporary real vectors
    QVector<double> vRealIn = pInput->getDataVectorCopy();
    QVector<double> vRealOut = pOutput->getDataVectorCopy();

    // Abort and inform user if vectors are not of same size
    if(vRealOut.size() != vRealIn.size())
    {
        QMessageBox::warning(gpMainWindowWidget, QWidget::tr("Wrong Vector Size"), QWidget::tr("Input and output vector must be of same size."));
        return;
    }

    // Reduce vector size if they are not equal to an even potential of 2, and inform user
#ifndef __APPLE__
        int n = pow(2, int(log2(vRealOut.size()))); // Odd.... /magse
#else
        int n = (int)round(ldexp(2.0, int(log2(vRealOut.size()))));
#endif
    if(n != vRealOut.size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(vRealOut.size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindowWidget, QWidget::tr("Wrong Vector Size"), "Size of data vector must be an even power of 2. Number of log samples was reduced from " + oldString + " to " + newString + ".");
        reduceVectorSize(vRealOut, n);
        reduceVectorSize(vRealIn, n);
    }

    //Create complex vectors
    QVector< std::complex<double> > vCompIn = realToComplex(vRealIn);
    QVector< std::complex<double> > vCompOut = realToComplex(vRealOut);

    //Apply the fourier transforms
    FFT(vCompOut);
    FFT(vCompIn);

    // Calculate the trasfere function G and then the bode vectors
    QVector< std::complex<double> > G;
    QVector<double> vRe, vIm, vImNeg, vBodeGain, vBodePhase, vBodePhaseUncorrected, freq;
    // Reserve memory
    G.reserve(vCompIn.size()/2);
    vRe.reserve(vCompIn.size()/2);
    vIm.reserve(vCompIn.size()/2);
    vImNeg.reserve(vCompIn.size()/2);
    vBodeGain.reserve(vCompIn.size()/2);
    vBodePhase.reserve(vCompIn.size()/2);
    vBodePhaseUncorrected.reserve(vCompIn.size()/2);
    freq.reserve(vCompIn.size()/2);

    double phaseCorrection=0;
    for(int i=0; i<vCompIn.size()/2; ++i)
    {
        if(vCompIn[i] == std::complex<double>(0,0))        //Check for division by zero
        {
            G.append(G[i-1]);    //!< @warning This is not a good solution, and what if i=0?
        }
        else
        {
            G.append(vCompOut[i]/vCompIn[i]);                  //G(iw) = FFT(Y(iw))/FFT(X(iw))
        }

        if(i>0)
        {
            vRe.append(G[i].real());
            vIm.append(G[i].imag());
            vImNeg.append(-G[i].imag());
            vBodeGain.append( 20.*log10( std::abs(G[i]) ) );            // Gain: abs(G) = sqrt(R^2 + X^2) in dB
            vBodePhaseUncorrected.append( rad2deg(std::arg(G[i])) );    // Phase: arg(G) = arctan(X/R) in deg

            // Correct the phase plot to make it continous (because atan2 is limited from -180 to +180)
            if(vBodePhaseUncorrected.size() > 1)
            {
                //! @todo there is a risk here that the skip from +-180 to -+180 is missed if the value lies below (abs) 170
                if( (vBodePhaseUncorrected.last() > 170) && (vBodePhaseUncorrected[vBodePhaseUncorrected.size()-2] < -170) )
                {
                    phaseCorrection -= 360;
                }
                else if( (vBodePhaseUncorrected.last() < -170) && (vBodePhaseUncorrected[vBodePhaseUncorrected.size()-2] > 170) )
                {
                    phaseCorrection += 360;
                }
            }
            vBodePhase.append(vBodePhaseUncorrected.last() + phaseCorrection);
        }
    }

    // Build the frequency vector
    const double stoptime = pInput->getSharedTimeOrFrequencyVector()->last();
    // We skip f=0
    for(int i=1; i<G.size(); ++i)
    {
        freq.append(double(i)/stoptime);
        // Abort if we reach the desired max frequency
        if(freq.last() >= Fmax)
        {
            // Truncate the output vectors if we aborted due to Fmax
            vBodeGain.resize(freq.size());
            vBodePhase.resize(freq.size());
            break;
        }
    }


    // Create the output variables for nyquist plots
    //! @todo add description to description, (what was the data based on)
    SharedVariableDescriptionT pNyquistDesc(new VariableDescription()); pNyquistDesc->mDataName = "Nyquist";
    rNyquistData = SharedVectorVariableT(new ComplexVectorVariable(vRe, vIm, pOutput->getGeneration(), pNyquistDesc, SharedMultiDataVectorCacheT()));

    SharedVariableDescriptionT pNyquistInvDesc(new VariableDescription()); pNyquistInvDesc->mDataName = "Nyquist Inverted";
    rNyquistDataInv = SharedVectorVariableT(new ComplexVectorVariable(vRe, vImNeg, pOutput->getGeneration(), pNyquistInvDesc, SharedMultiDataVectorCacheT()));

    // Create the output variables for bode diagram
    SharedVectorVariableT pFrequencyVar = createFreeFrequencyVectorVariabel(freq);

    SharedVariableDescriptionT pGainDesc(new VariableDescription());
    pGainDesc->mDataName = "Magnitude";
    pGainDesc->mDataUnit = "dB";
    rGainData = SharedVectorVariableT(new FrequencyDomainVariable(pFrequencyVar, vBodeGain, pOutput->getGeneration(), pGainDesc, SharedMultiDataVectorCacheT()));

    SharedVariableDescriptionT pPhaseDesc(new VariableDescription());
    pPhaseDesc->mDataName = "Phase";
    pPhaseDesc->mDataUnit = "deg";
    rPhaseData = SharedVectorVariableT(new FrequencyDomainVariable(pFrequencyVar, vBodePhase, pOutput->getGeneration(), pPhaseDesc, SharedMultiDataVectorCacheT()));
}

void IndexIntervalCollection::addValue(const int val)
{
    if (mIntervalList.isEmpty())
    {
        mIntervalList.append(MinMaxT(val,val));
    }
    else
    {
        for(int i=0; i<mIntervalList.size(); ++i)
        {
            if (val < mIntervalList[i].mMin)
            {
                // Check if we should extend downwards
                if (val ==  mIntervalList[i].mMin-1)
                {
                    mIntervalList[i].mMin = val;
                }
                // If not then we should add a new one
                // We do not need to check if we should merge here as that should have been
                // handled while processing the previous intervals
                else
                {
                    mIntervalList.insert(i, MinMaxT(val,val));
                }
                break;
            }
            else if (val > mIntervalList[i].mMax)
            {
                // Check if we should extend upwards
                if (val ==  mIntervalList[i].mMax+1)
                {
                    mIntervalList[i].mMax = val;
                    //! @todo check merge FIXA /Peter
                    // If we have an other interval after ours, then lets check if we should merge them
                    if (i+1<mIntervalList.size())
                    {
                        // Check if we should merge
                        if (mIntervalList[i+1].mMin == val+1)
                        {
                            mergeIntervals(i,i+1);
                        }
                    }
                    break;
                }
                // Add new, if this is the last one
                // else this is handled by the less then check above
                else if (i+1 >= mIntervalList.size())
                {
                    mIntervalList.insert(i+1, MinMaxT(val,val));
                    break;
                }

                // Else keep checking the next iic
            }
            //! @todo need to check and merge intervals after adding
            // If non of the above were triggered then the value was within an already existing interval
        }
    }
}

void IndexIntervalCollection::removeValue(const int val)
{
    for(int i=0; i<mIntervalList.size(); ++i)
    {
        if ((val >= mIntervalList[i].mMin) && (val <= mIntervalList[i].mMax))
        {
            // If interval is singular then remove it
            if (mIntervalList[i].mMin == mIntervalList[i].mMax)
            {
                mIntervalList.removeAt(i);
            }
            // If val = min then shrink it
            else if (mIntervalList[i].mMin == val)
            {
                mIntervalList[i].mMin += 1;
            }
            // If val = max than shrink it
            else if (mIntervalList[i].mMax == val)
            {
                mIntervalList[i].mMax -= 1;
            }
            // Else split the interval
            else
            {
                MinMaxT newInterv(val+1, mIntervalList[i].mMax);
                mIntervalList[i].mMax = val-1;
                mIntervalList.insert(i+1,newInterv);
            }
            // Since we took care of the value we can break the loop
            break;
        }
    }
}

int IndexIntervalCollection::min() const
{
    if (mIntervalList.isEmpty())
    {
        return -1;
    }
    return mIntervalList.first().mMin;
}

int IndexIntervalCollection::max() const
{
    if (mIntervalList.isEmpty())
    {
        return -1;
    }
    return mIntervalList.last().mMax;
}

QList<IndexIntervalCollection::MinMaxT> IndexIntervalCollection::getList() const
{
    return mIntervalList;
}

QList<int> IndexIntervalCollection::getCompleteList() const
{
    QList<int> complete;
    for (int i=0; i<mIntervalList.size(); ++i)
    {
        for (int j=mIntervalList[i].mMin; j<=mIntervalList[i].mMax; ++j)
        {
            complete.append(j);
        }
    }
    return complete;
}

int IndexIntervalCollection::getNumIIC() const
{
    return mIntervalList.size();
}

int IndexIntervalCollection::getNumI() const
{
    int ctr=0;
    for (int ic=0; ic<mIntervalList.size(); ++ic)
    {
        ctr += (mIntervalList[ic].mMax - mIntervalList[ic].mMin) + 1;
    }
    return ctr;
}

void IndexIntervalCollection::testMe()
{
    addValue(1); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", " << getCompleteList();
    addValue(3); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(0); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(-2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(4); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(6); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(10); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(15); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(12); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    removeValue(4); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    removeValue(2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(4); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

}

void IndexIntervalCollection::mergeIntervals(int first, int second)
{
    // We assume that the intervals to merge are actually continuos neighbors
    mIntervalList[first].mMax = mIntervalList[second].mMax;
    mIntervalList.removeAt(second);
}

bool IndexIntervalCollection::isContinuos() const
{
    return (mIntervalList.size() == 1);
}

bool IndexIntervalCollection::isEmpty() const
{
    return mIntervalList.isEmpty();
}

bool IndexIntervalCollection::contains(const int val) const
{
    for(int i=0; i<mIntervalList.size(); ++i)
    {
        if ((val >= mIntervalList[i].mMin) && (val <= mIntervalList[i].mMax))
        {
            return true;
        }
    }
    return false;
}

void IndexIntervalCollection::clear()
{
    mIntervalList.clear();
}


IndexIntervalCollection::MinMaxT::MinMaxT(int min, int max)
{
    mMin = min;
    mMax = max;
}






HopsanVariable::HopsanVariable()
    : mpContainer(0), mpVariable(0)
{
    //Nothing
}

HopsanVariable::HopsanVariable(SharedVectorVariableT pData)
    : mpContainer(0), mpVariable(0)
{
    mpVariable = pData;
}

HopsanVariable::HopsanVariable(SharedVectorVariableContainerT pContainer, SharedVectorVariableT pData)
    : mpContainer(0), mpVariable(0)
{
    mpContainer = pContainer;
    mpVariable = pData;
}

void HopsanVariable::switchToGeneration(const int gen)
{
    if (mpContainer)
    {
        mpVariable = mpContainer->getDataGeneration(gen);
    }
}

LogDataHandler *HopsanVariable::getLogDataHandler()
{
    if (mpVariable)
    {
        return mpVariable->getLogDataHandler();
    }
    return 0;
}

bool HopsanVariable::hasContainer() const
{
    return !mpContainer.isNull();
}

bool HopsanVariable::isNull() const
{
    return mpVariable.isNull();
}

bool HopsanVariable::operator!() const
{
    return mpVariable.isNull();
}

bool HopsanVariable::isVariableAlias() const
{
    if (mpContainer && mpVariable)
    {
        return mpContainer->isGenerationAlias(mpVariable->getGeneration());
    }
    return false;
}

HopsanVariable::operator bool() const
{
    return !mpVariable.isNull();
}


ImportedFrequencyDomainVariable::ImportedFrequencyDomainVariable(SharedVectorVariableT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache)
    : FrequencyDomainVariable(frequency, rData, generation, varDesc, pGenerationMultiCache)
{
    mImportFileName = rImportFile;
}

bool ImportedFrequencyDomainVariable::isImported() const
{
    return true;
}

QString ImportedFrequencyDomainVariable::getImportedFileName() const
{
    return mImportFileName;
}
