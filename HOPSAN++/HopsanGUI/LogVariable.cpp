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
    pVarDesc->mDataUnit = "rad/s";
    return pVarDesc;
}

SharedVariablePtrT createFreeVectorVariable(const QVector<double> &rData, SharedVariableDescriptionT pVarDesc)
{
    return SharedVariablePtrT(new VectorVariable(rData, 0, pVarDesc, SharedMultiDataVectorCacheT()));
}

//! @brief Creates a free unhandled time vector logvariable, it can not have generations or be cached
SharedVariablePtrT createFreeTimeVectorVariabel(const QVector<double> &rTime)
{
    // Since there is no parent we can not cache this to disk or give it a generation, it is a free floating time vector (logvariable)
    return SharedVariablePtrT(new VectorVariable(rTime, 0, createTimeVariableDescription(), SharedMultiDataVectorCacheT(0)));
}

//! @brief Creates a free unhandled frequency vector logvariable, it can not have generations or be cached
SharedVariablePtrT createFreeFrequencyVectorVariabel(const QVector<double> &rFrequency)
{
    // Since there is no parent we can not cache this to disk or give it a generation, it is a free floating time vector (logvariable)
    return SharedVariablePtrT(new VectorVariable(rFrequency, 0, createFrequencyVariableDescription(), SharedMultiDataVectorCacheT(0)));
}

//! @brief This is a variable factory, variables will be free and wont be connected to a data chace
SharedVariablePtrT createFreeVariable(VariableTypeT type, SharedVariableDescriptionT pVarDesc)
{
    switch(type)
    {
    case VectorType:
        return SharedVariablePtrT(new VectorVariable(QVector<double>(), 0, pVarDesc, SharedMultiDataVectorCacheT()));
    case TimeDomainType:
        return SharedVariablePtrT(new TimeDomainVariable(createFreeTimeVectorVariabel(QVector<double>()), QVector<double>(), 0, pVarDesc, SharedMultiDataVectorCacheT()));
    case FrequencyDomainType:
        return SharedVariablePtrT(new FrequencyDomainVariable(createFreeFrequencyVectorVariabel(QVector<double>()), QVector<double>(), 0, pVarDesc, SharedMultiDataVectorCacheT()));
    default:
        //! @todo support all types
        // Not yet supported by factory
        return SharedVariablePtrT();
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

bool VariableDescription::operator==(const VariableDescription &other) const
{
    return (mComponentName == other.mComponentName && mPortName == other.mPortName && mDataName == other.mDataName && mDataUnit == other.mDataUnit);
}

void VectorVariable::setPlotOffset(double offset)
{
    mDataPlotOffset = offset;
    emit dataChanged();
}

void VectorVariable::setPlotScaleAndOffset(const double scale, const double offset)
{
    mCustomUnitScale.setOnlyScale(scale);
    mDataPlotScale = scale;
    mDataPlotOffset = offset;
    emit dataChanged();
}

void VectorVariable::setTimePlotOffset(double offset)
{
    // Do nothing by default
}

void VectorVariable::setTimePlotScale(double scale)
{
    // Do nothing by default
}

void VectorVariable::setTimePlotScaleAndOffset(const double scale, const double offset)
{
    // Do nothing by default
}

void VectorVariable::setPlotScale(double scale)
{
    mCustomUnitScale.setOnlyScale(scale);
    mDataPlotScale = scale;
    emit dataChanged();
}

VectorVariable::VectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache)
{
    mpParentVariableContainer = 0;
    mpVariableDescription = varDesc;
    mDataPlotOffset = 0.0;
    mDataPlotScale = 1.0;
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

const QString &VectorVariable::getPlotScaleDataUnit() const
{
    return mCustomUnitScale.mUnit;
}

const QString &VectorVariable::getActualPlotDataUnit() const
{
    if (mCustomUnitScale.mUnit.isEmpty())
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

int VectorVariable::getGeneration() const
{
    return mGeneration;
}

int VectorVariable::getLowestGeneration() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getLowestGeneration();
}

int VectorVariable::getHighestGeneration() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getHighestGeneration();
}

int VectorVariable::getNumGenerations() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 1;
    }
    return mpParentVariableContainer->getNumGenerations();
}

bool VectorVariable::isImported() const
{
    return false;
}

QString VectorVariable::getImportedFileName() const
{
    return QString();
}


const SharedVariablePtrT VectorVariable::getSharedTimeOrFrequencyVector() const
{
    return mpSharedTimeOrFrequencyVector;
}

double VectorVariable::getPlotOffset() const
{
    return mDataPlotOffset;
}

void VectorVariable::addToData(const SharedVariablePtrT pOther)
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
void VectorVariable::subFromData(const SharedVariablePtrT pOther)
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

void VectorVariable::multData(const SharedVariablePtrT pOther)
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

void VectorVariable::divData(const SharedVariablePtrT pOther)
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

void VectorVariable::diffBy(SharedVariablePtrT pOther)
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

void VectorVariable::integrateBy(SharedVariablePtrT pOther)
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

void VectorVariable::lowPassFilter(SharedVariablePtrT pTime, const double w)
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
        double Al = 2.0/(2.0*3.14159265359*w);
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

SharedVariablePtrT VectorVariable::toFrequencySpectrum(const SharedVariablePtrT pTime, const bool doPowerSpectrum)
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
            return SharedVariablePtrT();
        }

        // Vector size has to be an even potential of 2.
        // Calculate largets potential that is smaller than or equal to the vector size.
        const int n = pow(2, int(log2(data.size())));
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
        freq.reserve(n/2-1);
        mag.reserve(n/2-1);
        const double maxt = time.last();
        for(int i=1; i<n/2; ++i)        //FFT is symmetric, so only use first half
        {
            if(doPowerSpectrum)
            {
                mag.append(real(vComplex[i]*conj(vComplex[i]))/n);
            }
            else
            {
                mag.append(sqrt(vComplex[i].real()*vComplex[i].real() + vComplex[i].imag()*vComplex[i].imag()));
            }

            // Build freq vector
            freq.append(double(i)/maxt);
        }

        SharedVariableDescriptionT pDesc(new VariableDescription(*mpVariableDescription.data()));
        //! @todo we may need to change description information for this variable to avoid trouble
        return SharedVariablePtrT(new FrequencyDomainVariable(createFreeFrequencyVectorVariabel(freq), mag, this->getGeneration(), pDesc, SharedMultiDataVectorCacheT()));
    }
    else
    {
        //! @todo error message
        // Abort
        return SharedVariablePtrT();
    }
}


void VectorVariable::assignFrom(const SharedVariablePtrT pOther)
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

void VectorVariable::assignFrom(SharedVariablePtrT time, const QVector<double> &rData)
{
    // By default we do not have a time vector so lets just assign the data
    //! @todo maybe should give error or warnign
    assignFrom(rData);
}

void VectorVariable::assignFrom(QVector<double> &rTime, QVector<double> &rData)
{
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
    TicToc timer;
    double ret = 0;
    int i=0;
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(; i<pVector->size(); ++i)
    {
        ret += pVector->at(i);
    }
    ret /= i;
    mpCachedDataVector->endFullVectorOperation(pVector);
    //timer.tocDbg("Actual average calc");
    return ret;
}

double VectorVariable::minOfData(int &rIdx) const
{
    rIdx = -1;
    double ret = std::numeric_limits<double>::max();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) < ret)
        {
            ret = pVector->at(i);
            rIdx=i;
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double VectorVariable::minOfData() const
{
    int dummy;
    return minOfData(dummy);
}

//double LogVariableData::indexOfMinOfData() const
//{
//    double minVal = std::numeric_limits<double>::max();
//    double ret = 0;
//    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
//    for(int i=0; i<pVector->size(); ++i)
//    {
//        if(pVector->at(i) < minVal)
//        {
//            minVal = pVector->at(i);
//            ret=i;
//        }
//    }
//    mpCachedDataVector->endFullVectorOperation(pVector);
//    return ret;
//}

//double LogVariableData::indexOfMaxOfData() const
//{
//    double maxVal = std::numeric_limits<double>::min();
//    double ret = 0;
//    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
//    for(int i=0; i<pVector->size(); ++i)
//    {
//        if(pVector->at(i) > maxVal)
//        {
//            maxVal = pVector->at(i);
//            ret=i;
//        }
//    }
//    mpCachedDataVector->endFullVectorOperation(pVector);
//    return ret;
//}

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
}


double VectorVariable::maxOfData(int &rIdx) const
{
    rIdx = -1;
    double ret = -std::numeric_limits<double>::max();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) > ret)
        {
            ret = pVector->at(i);
            rIdx = i;
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double VectorVariable::maxOfData() const
{
    int dummy;
    return maxOfData(dummy);
}

void VectorVariable::preventAutoRemoval()
{
    if (!mpParentVariableContainer.isNull())
    {
        mpParentVariableContainer.data()->preventAutoRemove(mGeneration);
    }
}

void VectorVariable::allowAutoRemoval()
{
    if (!mpParentVariableContainer.isNull())
    {
        mpParentVariableContainer.data()->allowAutoRemove(mGeneration);
    }
}

void VectorVariable::setCacheDataToDisk(const bool toDisk)
{
    mpCachedDataVector->setCached(toDisk);
}

bool VectorVariable::isCachingDataToDisk() const
{
    return mpCachedDataVector->isCached();
}

LogVariableContainer *VectorVariable::getLogVariableContainer()
{
    return mpParentVariableContainer;
}

bool VectorVariable::indexInRange(const int idx) const
{
    //! @todo Do we need to check timevector also ? (or should we assume thay are the same)
    return (idx>=0 && idx<mpCachedDataVector->size());
}

LogDataHandler *VectorVariable::getLogDataHandler()
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 0;
    }
    return mpParentVariableContainer->getLogDataHandler();
}


const QString &LogVariableContainer::getName() const
{
    return mName;
}


void LogVariableContainer::preventAutoRemove(const int gen)
{
    //! @todo what happens if we tell it to keep a generation it does not have
    //! @todo maybe special -1 input for ALL
    if (!mKeepGenerations.contains(gen))
    {
        mKeepGenerations.prepend(gen);
    }
}

void LogVariableContainer::allowAutoRemove(const int gen)
{
    //! @todo maybe special -1 input for ALL
    mKeepGenerations.removeOne(gen);
}

LogDataHandler *LogVariableContainer::getLogDataHandler()
{
    return mpParentLogDataHandler;
}

void LogVariableContainer::actuallyRemoveDataGen(GenerationMapT::iterator git)
{
    // Remove from alias and imported registers if needed
    if (mAliasGenIndexes.contains(git.key()))
    {
        mAliasGenIndexes.removeValue(git.key());
    }
    if (mImportedGenIndexes.contains(git.key()))
    {
        mImportedGenIndexes.removeValue(git.key());
    }
    //! @todo should we really emit this if we are removing an alias
    emit logVariableBeingRemoved(git.value());
    mDataGenerations.erase(git);
}

//! @brief This function converts a VariableSourceTypeT enum into a string
QString variableSourceTypeAsString(const VariableSourceTypeT type)
{
    switch (type)
    {
    case ScriptVariableType :
        return "ScriptVariableType";
        break;
    case TempVariableType :
        return "TempVariableType";
        break;
    case ModelVariableType :
        return "ModelVariableType";
        break;
    case ImportedVariableType :
        return "ImportedVariableType";
        break;
    default :
        return "UndefinedVariableSourceType";
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


int LogVariableContainer::getLowestGeneration() const
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

int LogVariableContainer::getHighestGeneration() const
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

int LogVariableContainer::getNumGenerations() const
{
    return mDataGenerations.size();
}

QList<int> LogVariableContainer::getGenerations() const
{
    return mDataGenerations.keys();
}

bool LogVariableContainer::isStoringAlias() const
{
    return !mAliasGenIndexes.isEmpty();
}

bool LogVariableContainer::isGenerationAlias(const int gen) const
{
    return mAliasGenIndexes.contains(gen);
}

bool LogVariableContainer::isStoringImported() const
{
    return !mImportedGenIndexes.isEmpty();
}

bool LogVariableContainer::isGenerationImported(const int gen) const
{
    return mImportedGenIndexes.contains(gen);
}

SharedVariablePtrT LogVariableContainer::getDataGeneration(const int gen) const
{
    // If generation not specified (<0), then take latest (if not empty),
    if ( (gen < 0) && !mDataGenerations.empty() )
    {
        return (--mDataGenerations.end()).value();
    }

    // Else try to find specified generation
    // Return 0 ptr if generation not found
    return mDataGenerations.value(gen, SharedVariablePtrT(0));
}

QList<SharedVariablePtrT> LogVariableContainer::getAllDataGenerations() const
{
    return mDataGenerations.values();
}

bool LogVariableContainer::hasDataGeneration(const int gen)
{
    return mDataGenerations.contains(gen);
}

//! @brief Adds or replaces a data generation
void LogVariableContainer::addDataGeneration(const int generation, SharedVariablePtrT pData)
{
    // Set some data that was set by LogvariableData constructor when creating a new variable, in this case we need to overwrite
    pData->mpParentVariableContainer = this;
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
}

//! @brief Removes a generation of the variable
//! @note If last generation the container itself will be deletet from parent log data handler, so DO NOT CALL this while itterating through the log data map
//! @todo this functions should not call delete in parent if empty, it causes difficult to debugg problems while calling it during itteration, need to come up with a smarter solution
//! @returns True if the generation was removed, otherwise false (if generation was not present or taged as keep (when not forcing)
bool LogVariableContainer::removeDataGeneration(const int generation, const bool force)
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
bool LogVariableContainer::purgeOldGenerations(const int purgeEnd, const int nGensToKeep)
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

void LogVariableContainer::removeAllGenerations()
{
    // It is assumed that the generation map is sorted by key which it should be since adding will allways append
    QList<int> gens = mDataGenerations.keys();
    for (int it=0; it<gens.size(); ++it)
    {
        removeDataGeneration(gens[it]);
    }
}

bool LogVariableContainer::removeAllImportedGenerations()
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

LogVariableContainer::LogVariableContainer(const QString &rName, LogDataHandler *pParentLogDataHandler) : QObject()
{
    mName = rName;
    mpParentLogDataHandler = pParentLogDataHandler;
}

LogVariableContainer::~LogVariableContainer()
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


double VectorVariable::getPlotScale() const
{
    return mDataPlotScale;
}

void VectorVariable::setCustomUnitScale(const UnitScale &rUnitScale)
{
    mCustomUnitScale = rUnitScale;
    mDataPlotScale = rUnitScale.toDouble();
    emit dataChanged();
}

const UnitScale &VectorVariable::getCustomUnitScale() const
{
    return mCustomUnitScale;
}

void VectorVariable::removeCustomUnitScale()
{
    mCustomUnitScale.clear();
    mDataPlotScale = 1.0;
    emit dataChanged();
}


TimeDomainVariable::TimeDomainVariable(SharedVariablePtrT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(rData, generation, varDesc, pGenerationMultiCache)
{
    mpSharedTimeOrFrequencyVector = time;
    if (!mpSharedTimeOrFrequencyVector.isNull())
    {
        connect(mpSharedTimeOrFrequencyVector.data(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()), Qt::UniqueConnection);
    }
}

VariableTypeT TimeDomainVariable::getVariableType() const
{
    return TimeDomainType;
}

void TimeDomainVariable::diffBy(SharedVariablePtrT pOther)
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

void TimeDomainVariable::integrateBy(SharedVariablePtrT pOther)
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

void TimeDomainVariable::lowPassFilter(SharedVariablePtrT pTime, const double w)
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

SharedVariablePtrT TimeDomainVariable::toFrequencySpectrum(const SharedVariablePtrT pTime, const bool doPowerSpectrum)
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
            return SharedVariablePtrT();
        }
    }
    else
    {
        return VectorVariable::toFrequencySpectrum(pTime, doPowerSpectrum);
    }
}

void TimeDomainVariable::assignFrom(const SharedVariablePtrT pOther)
{
    mpSharedTimeOrFrequencyVector = pOther->getSharedTimeOrFrequencyVector();
    VectorVariable::assignFrom(pOther);
}

void TimeDomainVariable::assignFrom(SharedVariablePtrT time, const QVector<double> &rData)
{
    mpCachedDataVector->replaceData(rData);
    mpSharedTimeOrFrequencyVector = time;
    emit dataChanged();
}

void TimeDomainVariable::assignFrom(QVector<double> &rTime, QVector<double> &rData)
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


ImportedTimeDomainVariable::ImportedTimeDomainVariable(SharedVariablePtrT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    TimeDomainVariable(time, rData, generation, varDesc, pGenerationMultiCache)
{
    mImportFileName = rImportFile;
}

bool ImportedVariableBase::isImported() const
{
    return true;
}

QString ImportedVariableBase::getImportedFileName() const
{
    return mImportFileName;
}



ComplexVectorVariable::ComplexVectorVariable(const QVector<double> &rReal, const QVector<double> &rImaginary, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(QVector<double>(0), generation, varDesc, pGenerationMultiCache)
{
    mpCachedRealVector = new CachableDataVector(rReal, pGenerationMultiCache, gpConfig->getCacheLogData());
    mpCachedImagVector = new CachableDataVector(rImaginary, pGenerationMultiCache, gpConfig->getCacheLogData());
}

ComplexVectorVariable::ComplexVectorVariable(SharedVariablePtrT pReal, SharedVariablePtrT pImaginary, const int generation, SharedVariableDescriptionT varDesc)
    : VectorVariable(QVector<double>(0), generation, varDesc, SharedMultiDataVectorCacheT())
{
    mpCachedRealVector = 0;
    mpCachedImagVector = 0;
    mpSharedReal = pReal;
    mpSharedImag =pImaginary;
}

VariableTypeT ComplexVectorVariable::getVariableType() const
{
    return ComplexType;
}


FrequencyDomainVariable::FrequencyDomainVariable(SharedVariablePtrT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(rData, generation, varDesc, pGenerationMultiCache)
{
    mpSharedTimeOrFrequencyVector = frequency;
}

ImportedVectorVariable::ImportedVectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache) :
    VectorVariable(rData, generation, varDesc, pGenerationMultiCache)
{
    mImportFileName = rImportFile;
}


void createBode(const SharedVariablePtrT pInput, const SharedVariablePtrT pOutput, int Fmax, SharedVariablePtrT &rNyquistData, SharedVariablePtrT &rNyquistDataInv, SharedVariablePtrT &rGainData, SharedVariablePtrT &rPhaseData)
{
    // Create temporary real vectors
    //! @todo is X Y input output naming correct ?
    QVector<double> realYvector = pInput->getDataVectorCopy();
    QVector<double> realXvector = pOutput->getDataVectorCopy();

    // Abort and inform user if vectors are not of same size
    if(realXvector.size() != realYvector.size())
    {
        QMessageBox::warning(gpMainWindowWidget, QWidget::tr("Wrong Vector Size"), QWidget::tr("Input and output vector must be of same size."));
        return;
    }

    // Reduce vector size if they are not equal to an even potential of 2, and inform user
    int n = pow(2, int(log2(realXvector.size())));
    if(n != realXvector.size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(realXvector.size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindowWidget, QWidget::tr("Wrong Vector Size"), "Size of data vector must be an even power of 2. Number of log samples was reduced from " + oldString + " to " + newString + ".");
        reduceVectorSize(realXvector, n);
        reduceVectorSize(realYvector, n);
    }

    //Create complex vectors
    QVector< std::complex<double> > Y = realToComplex(realYvector);
    QVector< std::complex<double> > X = realToComplex(realXvector);

    //Apply the fourier transforms
    FFT(X);
    FFT(Y);

    //Divide the fourier transform elementwise and take their absolute value
    QVector< std::complex<double> > G;
    QVector<double> vRe;
    QVector<double> vIm;
    QVector<double> vImNeg;
    QVector<double> vBodeGain;
    QVector<double> vBodePhase;

    double phaseCorrection=0;
    QVector<double> vBodePhaseUncorrected;
    for(int i=0; i<Y.size()/2; ++i)
    {
        if(Y.at(i) == std::complex<double>(0,0))        //Check for division by zero
        {
            G.append(G[i-1]);    //! @todo This is not a good solution, and what if i=0?
        }
        else
        {
            G.append(X.at(i)/Y.at(i));                  //G(iw) = FFT(Y(iw))/FFT(X(iw))
        }
        if(i!=0)
        {
            vRe.append(G[i].real());
            vIm.append(G[i].imag());
            vImNeg.append(-G[i].imag());
            vBodeGain.append(20*log10(sqrt(G[i].real()*G[i].real() + G[i].imag()*G[i].imag())));  //Gain: abs(G) = sqrt(R^2 + X^2)
            vBodePhaseUncorrected.append(atan2(G[i].imag(), G[i].real())*180./3.14159265);          //Phase: arg(G) = arctan(X/R)

            // Correct the phase plot to make it continous (because atan2 is limited from -180 to +180)
            if(vBodePhaseUncorrected.size() > 1)
            {
                if(vBodePhaseUncorrected.last() > 170 && vBodePhaseUncorrected[vBodePhaseUncorrected.size()-2] < -170)
                    phaseCorrection -= 360;
                else if(vBodePhaseUncorrected.last() < -170 && vBodePhaseUncorrected[vBodePhaseUncorrected.size()-2] > 170)
                    phaseCorrection += 360;
            }
            vBodePhase.append(vBodePhaseUncorrected.last() + phaseCorrection);
        }
    }


    QVector<double> F;
    const double stoptime = pInput->getSharedTimeOrFrequencyVector()->last();
    F.reserve(G.size());
    for(int i=1; i<G.size(); ++i)
    {
        F.append((i+1)/stoptime);
        if(F.last() >= Fmax) break;
    }
    vBodeGain.resize(F.size());
    vBodePhase.resize(F.size());

    // Create the output variables
    rNyquistData = SharedVariablePtrT(new ComplexVectorVariable(vRe, vIm,pOutput->getGeneration(),
                                                                SharedVariableDescriptionT(new VariableDescription(*pOutput->getVariableDescription().data())),
                                                                SharedMultiDataVectorCacheT()));

    rNyquistDataInv = SharedVariablePtrT(new ComplexVectorVariable(vRe, vImNeg,pOutput->getGeneration(),
                                                                   SharedVariableDescriptionT(new VariableDescription(*pOutput->getVariableDescription().data())),
                                                                   SharedMultiDataVectorCacheT()));

    SharedVariablePtrT pFrequencyVar = createFreeFrequencyVectorVariabel(F);
    rGainData = SharedVariablePtrT(new FrequencyDomainVariable(pFrequencyVar, vBodeGain, pOutput->getGeneration(),
                                                               SharedVariableDescriptionT(new VariableDescription(*pOutput->getVariableDescription().data())),
                                                               SharedMultiDataVectorCacheT()));

    rPhaseData = SharedVariablePtrT(new FrequencyDomainVariable(pFrequencyVar, vBodePhase, pOutput->getGeneration(),
                                                                SharedVariableDescriptionT(new VariableDescription(*pOutput->getVariableDescription().data())),
                                                                SharedMultiDataVectorCacheT()));
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
            // First check if we should insert before or extend downwards
            if (val < mIntervalList[i].mMin)
            {
                // Extend
                if (val ==  mIntervalList[i].mMin-1)
                {
                    mIntervalList[i].mMin = val;
                }
                else
                // Add new
                {
                    mIntervalList.insert(i, MinMaxT(val,val));
                }
            }
            // Now check if we should insert or extend upwards
            else if (val > mIntervalList[i].mMax)
            {
                // Extend
                if (val ==  mIntervalList[i].mMax+1)
                {
                    mIntervalList[i].mMax = val;
                }
                else
                // Add new
                {
                    mIntervalList.insert(i+1, MinMaxT(val,val));
                }
            }

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
                mIntervalList[i].mMax -= 1;
                mIntervalList.insert(i+1,newInterv);
            }
        }
    }
}

int IndexIntervalCollection::min() const
{
    if (mIntervalList.isEmpty())
    {
        return 0;
    }
    return mIntervalList[0].mMin;
}

int IndexIntervalCollection::max() const
{
    if (mIntervalList.isEmpty())
    {
        return 0;
    }
    return mIntervalList[0].mMax;
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


