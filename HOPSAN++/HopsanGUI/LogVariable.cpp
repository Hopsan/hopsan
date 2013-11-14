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

#include <limits>
#include <QMessageBox>

//! @brief Creates a free unhandled time vector logvariable, it can not have generations or be cached
SharedLogVariableDataPtrT createFreeTimeVariabel(const QVector<double> &rTime)
{
    SharedVariableCommonDescriptionT pVarDesc = SharedVariableCommonDescriptionT(new VariableCommonDescription());
    pVarDesc->mDataName = TIMEVARIABLENAME;
    pVarDesc->mDataUnit = "s";
    // Since there is no parent we can nog cahe this to disk or give it a generation, it is a free floating time vector (logvariable)
    // Note! the time vaariable does not have a time vector, the time is the data in this case
    return SharedLogVariableDataPtrT(new LogVariableData(0, SharedLogVariableDataPtrT(0), rTime, pVarDesc, SharedMultiDataVectorCacheT(0), 0));
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

QString VariableCommonDescription::getFullName() const
{
    return makeConcatName(mComponentName,mPortName,mDataName);
}

QString VariableCommonDescription::getFullNameWithSeparator(const QString sep) const
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

void VariableCommonDescription::setFullName(const QString compName, const QString portName, const QString dataName)
{
    mComponentName = compName;
    mPortName = portName;
    mDataName = dataName;
}

bool VariableCommonDescription::operator==(const VariableCommonDescription &other) const
{
    return (mComponentName == other.mComponentName && mPortName == other.mPortName && mDataName == other.mDataName && mDataUnit == other.mDataUnit);
}

void LogVariableData::setPlotOffset(double offset)
{
    mDataPlotOffset = offset;
    emit dataChanged();
}

void LogVariableData::setPlotScaleAndOffset(const double scale, const double offset)
{
    mCustomUnitScale.setOnlyScale(scale);
    mDataPlotScale = scale;
    mDataPlotOffset = offset;
    emit dataChanged();
}

void LogVariableData::setTimePlotOffset(double offset)
{
    if (!mSharedTimeVectorPtr.isNull())
    {
        mSharedTimeVectorPtr->setPlotOffset(offset);
    }
}

void LogVariableData::setTimePlotScale(double scale)
{
    if (!mSharedTimeVectorPtr.isNull())
    {
        mSharedTimeVectorPtr->setPlotScale(scale);
    }
}

void LogVariableData::setTimePlotScaleAndOffset(const double scale, const double offset)
{
    if (!mSharedTimeVectorPtr.isNull())
    {
        mSharedTimeVectorPtr->setPlotScaleAndOffset(scale, offset);
    }
}

void LogVariableData::setPlotScale(double scale)
{
    mCustomUnitScale.setOnlyScale(scale);
    mDataPlotScale = scale;
    emit dataChanged();
}

LogVariableData::LogVariableData(const int generation, SharedLogVariableDataPtrT time, const QVector<double> &rData, SharedVariableCommonDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent)
{
    mpParentVariableContainer = pParent;
    mpVariableCommonDescription = varDesc;
    mDataPlotOffset = 0.0;
    mDataPlotScale = 1.0;
    mGeneration = generation;
    mSharedTimeVectorPtr = time;
    if (!mSharedTimeVectorPtr.isNull())
    {
        connect(mSharedTimeVectorPtr.data(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()), Qt::UniqueConnection);
    }
    mpCachedDataVector = new CachableDataVector(rData, pGenerationMultiCache, gpConfig->getCacheLogData());
}

LogVariableData::~LogVariableData()
{
    if (mpCachedDataVector != 0)
    {
        delete mpCachedDataVector;
    }
}

const SharedVariableCommonDescriptionT LogVariableData::getVariableCommonDescription() const
{
    return mpVariableCommonDescription;
}

const SharedVariableUniqueDescriptionT LogVariableData::getVariableUniqueDescription() const
{
    return mpVariableUniqueDescription;
}

void LogVariableData::setVariableUniqueDescription(const VariableUniqueDescription &rDesc)
{
    // We need to make a copy here so that it is truly unique
    mpVariableUniqueDescription.clear();
    mpVariableUniqueDescription = SharedVariableUniqueDescriptionT(new VariableUniqueDescription(rDesc));
}

const SharedLogVariableDataPtrT LogVariableData::getTimeVector() const
{
    return mSharedTimeVectorPtr;
}

VariableSourceTypeT LogVariableData::getVariableSource() const
{
    // First check if we have unique override
    if (mpVariableUniqueDescription)
    {
        return mpVariableUniqueDescription->mVariableSourceType;
    }
    else
    {
        return mpVariableCommonDescription->mVariableSourceType;
    }
}

const QString &LogVariableData::getAliasName() const
{
    return mpVariableCommonDescription->mAliasName;
}

QString LogVariableData::getFullVariableName() const
{
    return mpVariableCommonDescription->getFullName();
}

QString LogVariableData::getFullVariableNameWithSeparator(const QString sep) const
{
    return mpVariableCommonDescription->getFullNameWithSeparator(sep);
}

QString LogVariableData::getSmartName() const
{
    if (mpVariableCommonDescription->mAliasName.isEmpty())
    {
        return mpVariableCommonDescription->getFullName();
    }
    else
    {
        return mpVariableCommonDescription->mAliasName;
    }
}

const QString &LogVariableData::getModelPath() const
{
    return mpVariableCommonDescription->mModelPath;
}

const QString &LogVariableData::getComponentName() const
{
    return mpVariableCommonDescription->mComponentName;
}

const QString &LogVariableData::getPortName() const
{
    return mpVariableCommonDescription->mPortName;
}

const QString &LogVariableData::getDataName() const
{
    return mpVariableCommonDescription->mDataName;
}

const QString &LogVariableData::getDataUnit() const
{
    return mpVariableCommonDescription->mDataUnit;
}

const QString &LogVariableData::getPlotScaleDataUnit() const
{
    return mCustomUnitScale.mUnit;
}

const QString &LogVariableData::getActualPlotDataUnit() const
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

bool LogVariableData::hasAliasName() const
{
    return !mpVariableCommonDescription->mAliasName.isEmpty();
}

int LogVariableData::getGeneration() const
{
    return mGeneration;
}

int LogVariableData::getLowestGeneration() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getLowestGeneration();
}

int LogVariableData::getHighestGeneration() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getHighestGeneration();
}

int LogVariableData::getNumGenerations() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 1;
    }
    return mpParentVariableContainer->getNumGenerations();
}

bool LogVariableData::isImported() const
{
    // first check unique override
    if (mpVariableUniqueDescription)
    {
        return (mpVariableUniqueDescription->mVariableSourceType == ImportedVariableType);
    }
    else
    {
        return (mpVariableCommonDescription->mVariableSourceType == ImportedVariableType);
    }
}

QString LogVariableData::getImportedFromFileName() const
{
    // Check unique override
    if (mpVariableUniqueDescription)
    {
        return mpVariableUniqueDescription->mImportFileName;
    }
    else
    {
        // This should never happen
        return QString();
    }
}

const SharedLogVariableDataPtrT LogVariableData::getSharedTimePointer() const
{
    return mSharedTimeVectorPtr;
}

double LogVariableData::getPlotOffset() const
{
    return mDataPlotOffset;
}

void LogVariableData::addToData(const SharedLogVariableDataPtrT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
}
void LogVariableData::addToData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}
void LogVariableData::subFromData(const SharedLogVariableDataPtrT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += -pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}
void LogVariableData::subFromData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] += -other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::multData(const SharedLogVariableDataPtrT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] *= pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::multData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] *= other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::divData(const SharedLogVariableDataPtrT pOther)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] /= pOther->peekData(i);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::divData(const double other)
{
    DataVectorT* pData =  mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
       (*pData)[i] /= other;
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}


void LogVariableData::absData()
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
        (*pData)[i] = fabs((*pData)[i]);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::diffBy(const SharedLogVariableDataPtrT pOther)
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    QVector<double> diffX;
    if(pOther != 0)
    {
        diffX = pOther->getDataVectorCopy();
    }
    else
    {
        // If no diff vector supplied, use time
        if (mSharedTimeVectorPtr)
        {
            diffX = mSharedTimeVectorPtr->getDataVectorCopy();
        }
        else
        {
            //! @todo error message
            // Abort
            return;
        }
    }
    for(int i=0; i<pData->size()-1; ++i)
    {
        (*pData)[i] = ((*pData)[i+1]-(*pData)[i])/(diffX[i+1]-diffX[i]);
    }
    pData->resize(pData->size()-1);
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::lowPassFilter(const SharedLogVariableDataPtrT pTime, const double w)
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    DataVectorT timeData;
    if(pTime == 0)
    {
        if (mSharedTimeVectorPtr)
        {
            timeData = mSharedTimeVectorPtr->getDataVectorCopy();
        }
        else
        {
            //! @todo error message
            // Abort
            return;
        }
    }
    else
    {
        timeData = pTime.data()->getDataVectorCopy();
    }

    double Al = 2.0/(2.0*3.14159265359*w);
    double temp1 = (*pData)[0];
    double temp = temp1;
    (*pData)[0] = temp;
    for(int i=1; i<pData->size(); ++i)
    {
        double T = timeData[i]-timeData[i-1];
        double ALF = Al/T;
        double G = 1.0+ALF;
        double A1 = (1.0-ALF)/G;
        double B1 = 1.0/G;
        temp = temp1;
        temp1 = (*pData)[i];
        (*pData)[i] = -A1*(*pData)[i-1] + B1*(temp1+temp);
    }

    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::frequencySpectrum(const SharedLogVariableDataPtrT pTime, const bool doPowerSpectrum)
{
    DataVectorT* pDataVec = mpCachedDataVector->beginFullVectorOperation();
    DataVectorT timeVec;
    if(pTime == 0)
    {
        if (mSharedTimeVectorPtr)
        {
            timeVec = mSharedTimeVectorPtr->getDataVectorCopy();
        }
        else
        {
            //! @todo error message
            // Abort
            return;
        }
    }
    else
    {
        timeVec = pTime.data()->getDataVectorCopy();
    }

    //Vector size has to be an even potential of 2.
    //Calculate largets potential that is smaller than or equal to the vector size.
    int n = pow(2, int(log2(pDataVec->size())));
    if(n != pDataVec->size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(pDataVec->size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Wrong Vector Size"),
                                 gpMainWindowWidget->tr("Size of data vector must be an even power of 2. Number of log samples was reduced from ") + oldString + gpMainWindowWidget->tr(" to ") + newString + ".");
        reduceVectorSize((*pDataVec), n);
        reduceVectorSize(timeVec, n);
    }

    //Create a complex vector
    QVector< std::complex<double> > vComplex = realToComplex(*pDataVec);

    //Apply the fourier transform
    FFT(vComplex);

    //Scalar multiply complex vector with its conjugate, and divide it with its size
    pDataVec->clear();
    for(int i=1; i<n/2; ++i)        //FFT is symmetric, so only use first half
    {
        if(doPowerSpectrum)
        {
            pDataVec->append(real(vComplex[i]*conj(vComplex[i]))/n);
        }
        else
        {
            pDataVec->append(sqrt(vComplex[i].real()*vComplex[i].real() + vComplex[i].imag()*vComplex[i].imag()));
        }
    }

    //Create the x vector (frequency)
    double max = timeVec.last();
    timeVec.clear();
    for(int i=1; i<n/2; ++i)
    {
        timeVec.append(double(i)/max);
    }

    //! @todo need an easier way to create individual free data variables, hmm dont I already have one in Logdatahandler ?
    //! @todo maybe need a special function to create a free time vector log data variable
    VariableCommonDescription varDesc;
    varDesc.mDataName = TIMEVARIABLENAME;
    varDesc.mDataUnit = "s";
    LogVariableContainer dummyTimeContainer(varDesc, 0);
    dummyTimeContainer.addDataGeneration(this->getGeneration(), SharedLogVariableDataPtrT(), timeVec);
    mSharedTimeVectorPtr = dummyTimeContainer.getDataGeneration(-1);

    mpCachedDataVector->endFullVectorOperation(pDataVec);
    emit dataChanged();
}


void LogVariableData::assignFrom(const SharedLogVariableDataPtrT pOther)
{
    mpCachedDataVector->replaceData(pOther->getDataVectorCopy());
    mSharedTimeVectorPtr = pOther->mSharedTimeVectorPtr;
    emit dataChanged();
}

void LogVariableData::assignFrom(const QVector<double> &rSrc)
{
    mpCachedDataVector->replaceData(rSrc);
    mSharedTimeVectorPtr = SharedLogVariableDataPtrT(); //No time vector assigned
    emit dataChanged();
}

void LogVariableData::assignFrom(const double src)
{
    assignFrom(QVector<double>() << src);
}

void LogVariableData::assignFrom(SharedLogVariableDataPtrT time, const QVector<double> &rData)
{
    mpCachedDataVector->replaceData(rData);
    mSharedTimeVectorPtr = time;
    emit dataChanged();
}

void LogVariableData::assignFrom(QVector<double> &rTime, QVector<double> &rData)
{
    // We create a new non managed free timevector from the supplied time data
    assignFrom(createFreeTimeVariabel(rTime), rData);
}

double LogVariableData::pokeData(const int index, const double value, QString &rErr)
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

double LogVariableData::peekData(const int index, QString &rErr) const
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

double LogVariableData::averageOfData() const
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

double LogVariableData::minOfData(int &rIdx) const
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

double LogVariableData::minOfData() const
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

void LogVariableData::elementWiseGt(QVector<double> &rResult, const double threshold) const
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

void LogVariableData::elementWiseLt(QVector<double> &rResult, const double threshold) const
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


//! @brief Appends one point to a curve, NEVER USE THIS UNLESS A CUSTOM (PRIVATE) X (TIME) VECTOR IS USED!
void LogVariableData::append(const double t, const double y)
{
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
    mSharedTimeVectorPtr.data()->append(t);
    //! @todo FIXA, it is bad to append x-data to shared time vector, there should be a custom private xvector Peter
}

//! @brief Appends one point to a curve, NEVER USE THIS WHEN A SHARED TIMEVECTOR EXIST
void LogVariableData::append(const double y)
{
    //! @todo smarter append regardless of cached or not
    //! @todo mayebe a reserve function to reserve memory if we know how much to expect
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
}


double LogVariableData::maxOfData(int &rIdx) const
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

double LogVariableData::maxOfData() const
{
    int dummy;
    return maxOfData(dummy);
}

void LogVariableData::preventAutoRemoval()
{
    if (!mpParentVariableContainer.isNull())
    {
        mpParentVariableContainer.data()->preventAutoRemove(mGeneration);
    }
}

void LogVariableData::allowAutoRemoval()
{
    if (!mpParentVariableContainer.isNull())
    {
        mpParentVariableContainer.data()->allowAutoRemove(mGeneration);
    }
}

void LogVariableData::setCacheDataToDisk(const bool toDisk)
{
    mpCachedDataVector->setCached(toDisk);
}

bool LogVariableData::isCachingDataToDisk() const
{
    return mpCachedDataVector->isCached();
}

LogVariableContainer *LogVariableData::getLogVariableContainer()
{
    return mpParentVariableContainer;
}

bool LogVariableData::indexInRange(const int idx) const
{
    //! @todo Do we need to check timevector also ? (or should we assume thay are the same)
    return (idx>=0 && idx<mpCachedDataVector->size());
}

LogDataHandler *LogVariableData::getLogDataHandler()
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 0;
    }
    return mpParentVariableContainer->getLogDataHandler();
}


const QString &LogVariableContainer::getAliasName() const
{
    return mVariableCommonDescription->mAliasName;
}

QString LogVariableContainer::getFullVariableName() const
{
    return mVariableCommonDescription->getFullName();
}

QString LogVariableContainer::getFullVariableNameWithSeparator(const QString sep) const
{
    return mVariableCommonDescription->getFullNameWithSeparator(sep);
}

QString LogVariableContainer::getSmartName() const
{
    if (mVariableCommonDescription->mAliasName.isEmpty())
    {
        return mVariableCommonDescription->getFullName();
    }
    else
    {
        return mVariableCommonDescription->mAliasName;
    }
}

const QString &LogVariableContainer::getModelPath() const
{
    return mVariableCommonDescription->mModelPath;
}

const QString &LogVariableContainer::getComponentName() const
{
    return mVariableCommonDescription->mComponentName;
}

const QString &LogVariableContainer::getPortName() const
{
    return mVariableCommonDescription->mPortName;
}

const QString &LogVariableContainer::getDataName() const
{
    return mVariableCommonDescription->mDataName;
}

const QString &LogVariableContainer::getDataUnit() const
{
    return mVariableCommonDescription->mDataUnit;
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

void LogVariableContainer::setAliasName(const QString alias)
{
    mVariableCommonDescription->mAliasName = alias;
    emit nameChanged();
}

QString getVariableSourceTypeString(const VariableSourceTypeT type)
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

void LogVariableContainer::setVariableCommonDescription(const VariableCommonDescription &rNewDescription)
{
    // Copy new data
    *(mVariableCommonDescription.data()) = rNewDescription;
}

SharedVariableCommonDescriptionT LogVariableContainer::getVariableCommonDescription() const
{
    return mVariableCommonDescription;
}

SharedLogVariableDataPtrT LogVariableContainer::getDataGeneration(const int gen) const
{
    // If generation not specified (<0), then take latest (if not empty),
    if ( (gen < 0) && !mDataGenerations.empty() )
    {
        return (--mDataGenerations.end()).value();
    }

    // Else try to find specified generation
    // Return 0 ptr if generation not found
    return mDataGenerations.value(gen, SharedLogVariableDataPtrT(0));
}

QList<SharedLogVariableDataPtrT> LogVariableContainer::getAllDataGenerations() const
{
    return mDataGenerations.values();
}

bool LogVariableContainer::hasDataGeneration(const int gen)
{
    return mDataGenerations.contains(gen);
}

SharedLogVariableDataPtrT LogVariableContainer::addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData)
{
    return addDataGeneration(generation, createFreeTimeVariabel(rTime), rData);
}

SharedLogVariableDataPtrT LogVariableContainer::addDataGeneration(const int generation, const SharedLogVariableDataPtrT time, const QVector<double> &rData)
{
    SharedLogVariableDataPtrT pData;
    if(mDataGenerations.contains(generation))
    {
        pData = mDataGenerations.find(generation).value();
        pData->assignFrom(time, rData);
    }
    else
    {

        if (mpParentLogDataHandler)
        {
            pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableCommonDescription, mpParentLogDataHandler->getOrCreateGenerationMultiCache(generation), this));
        }
        else
        {
            pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableCommonDescription, SharedMultiDataVectorCacheT(), this));
        }

        connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
        mDataGenerations.insert(generation, pData);
    }
    return pData;
}

//! @note Make sure that pData is not some other variable, that will screw things up badly
void LogVariableContainer::addDataGeneration(const int generation, SharedLogVariableDataPtrT pData)
{
    if(mDataGenerations.contains(generation))
    {
        mDataGenerations.find(generation).value().data()->assignFrom(pData);
    }
    else
    {
        // Set some data that was set by LogvariableData constructor when creating a new variable, in this case we need to overwrite
        pData->mpParentVariableContainer = this;
        pData->mGeneration = generation;
        // Connect the namechanged signal
        connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
        // Insert into generation storage
        mDataGenerations.insert(generation, pData);
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
                emit logVariableBeingRemoved(git.value());
                mDataGenerations.erase(git);
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
            emit logVariableBeingRemoved(git.value());
            mDataGenerations.erase(git);
            didRemove=true;
        }
    }

    // If last data generation removed then ask my parent to delete me
    if (mDataGenerations.isEmpty())
    {
        mpParentLogDataHandler->deleteVariable(this->getFullVariableName());
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

LogVariableContainer::LogVariableContainer(const VariableCommonDescription &rVarDesc, LogDataHandler *pParentLogDataHandler) : QObject()
{
    mVariableCommonDescription = SharedVariableCommonDescriptionT(new VariableCommonDescription(rVarDesc)); //Copy original data and create a new shared variable description
    mpParentLogDataHandler = pParentLogDataHandler;
}

LogVariableContainer::~LogVariableContainer()
{
    // Clear all data
    mDataGenerations.clear();
}

QVector<double> LogVariableData::getDataVectorCopy()
{
    QVector<double> vec;
    mpCachedDataVector->copyDataTo(vec);
    return vec;
}

void LogVariableData::sendDataToStream(QTextStream &rStream, QString separator)
{
    mpCachedDataVector->streamDataTo(rStream, separator);
}

int LogVariableData::getDataSize() const
{
    return mpCachedDataVector->size();
}

double LogVariableData::first() const
{
    return peekData(0);
}

double LogVariableData::last() const
{
    return peekData(mpCachedDataVector->size()-1);
}



double LogVariableData::peekData(const int idx) const
{
    double val = -1;
    if (indexInRange(idx))
    {
        mpCachedDataVector->peek(idx, val);
    }
    return val;
}


double LogVariableData::getPlotScale() const
{
    return mDataPlotScale;
}

void LogVariableData::setCustomUnitScale(const UnitScale &rUnitScale)
{
    mCustomUnitScale = rUnitScale;
    mDataPlotScale = rUnitScale.toDouble();
    emit dataChanged();
}

const UnitScale &LogVariableData::getCustomUnitScale() const
{
    return mCustomUnitScale;
}

void LogVariableData::removeCustomUnitScale()
{
    mCustomUnitScale.clear();
    mDataPlotScale = 1.0;
    emit dataChanged();
}
