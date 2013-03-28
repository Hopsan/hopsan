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

#include "LogVariable.h"
#include "LogDataHandler.h"
#include "Utilities/GUIUtilities.h"
#include "Configuration.h"

#include <limits>

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

//! @todo is this needed now that we have addToData
void LogVariableData::setValueOffset(double offset)
{
    mAppliedValueOffset += offset;
    addToData(offset);
    emit dataChanged();
}

void LogVariableData::setTimeOffset(double offset)
{
    mAppliedTimeOffset += offset; //! @todo this does not make sense any more
//    for (int i=0; mTimeVector.size(); ++i)
//    {
//        mTimeVector[i] += offset;
//    }

    for (int i=0; mSharedTimeVectorPtr->size(); ++i)
    {
        //! @todo FIX
        //QVector<double>* ptr = mSharedTimeVectorPtr.data();
        (*mSharedTimeVectorPtr)[i] += offset;
        //(*ptr)[i] += offset;
    }

    emit dataChanged();
}

LogVariableData::LogVariableData(const int generation, const QVector<double> &rTime, const QVector<double> &rData, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent)
{
    mpParentVariableContainer = pParent;
    mpVariableDescription = varDesc;
    mAppliedValueOffset = 0;
    mAppliedTimeOffset = 0;
    mGeneration = generation;
    mSharedTimeVectorPtr = SharedTimeVectorPtrT(new QVector<double>(rTime));
    mpCachedDataVector = new CachableDataVector(rData, pGenerationMultiCache, gConfig.getCacheLogData());
}

LogVariableData::LogVariableData(const int generation, SharedTimeVectorPtrT time, const QVector<double> &rData, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent)
{
    mpParentVariableContainer = pParent;
    mpVariableDescription = varDesc;
    mAppliedValueOffset = 0;
    mAppliedTimeOffset = 0;
    mGeneration = generation;
    mSharedTimeVectorPtr = time;
    mpCachedDataVector = new CachableDataVector(rData, pGenerationMultiCache, gConfig.getCacheLogData());
}

LogVariableData::~LogVariableData()
{
    if (mpCachedDataVector != 0)
    {
        delete mpCachedDataVector;
    }
}

const SharedVariableDescriptionT LogVariableData::getVariableDescription() const
{
    return mpVariableDescription;
}

QString LogVariableData::getAliasName() const
{
    return mpVariableDescription->mAliasName;
}

QString LogVariableData::getFullVariableName() const
{
    return mpVariableDescription->getFullName();
}

QString LogVariableData::getFullVariableNameWithSeparator(const QString sep) const
{
    return mpVariableDescription->getFullNameWithSeparator(sep);
}

QString LogVariableData::getModelPath() const
{
    return mpVariableDescription->mModelPath;
}

QString LogVariableData::getComponentName() const
{
    return mpVariableDescription->mComponentName;
}

QString LogVariableData::getPortName() const
{
    return mpVariableDescription->mPortName;
}

QString LogVariableData::getDataName() const
{
    return mpVariableDescription->mDataName;
}

QString LogVariableData::getDataUnit() const
{
    return mpVariableDescription->mDataUnit;
}

int LogVariableData::getGeneration() const
{
    return mGeneration;
}

int LogVariableData::getLowestGeneration() const
{
    //! @todo will crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getLowestGeneration();
}

int LogVariableData::getHighestGeneration() const
{
    //! @todo will crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getHighestGeneration();
}

int LogVariableData::getNumGenerations() const
{
    //! @todo will crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 1;
    }
    return mpParentVariableContainer->getNumGenerations();
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
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//        mDataVector[i] += other;
//    }
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
////! @todo DANGER will crash if other not as long (also in other places)
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//        mDataVector[i] -= pOther->mDataVector[i];
//    }
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
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//        mDataVector[i] -= other;
//    }
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
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//       mDataVector[i] *= pOther->mDataVector[i];
//    }
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
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//        mDataVector[i] *= other;
//    }
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
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//       mDataVector[i] /= pOther->mDataVector[i];
//    }
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
//    for (int i=0; i<mDataVector.size(); ++i)
//    {
//        mDataVector[i] /= other;
//    }
    emit dataChanged();
}
void LogVariableData::assignFrom(const SharedLogVariableDataPtrT pOther)
{
    mpCachedDataVector->replaceData(pOther->getDataVector());
    mSharedTimeVectorPtr = pOther->mSharedTimeVectorPtr;
    emit dataChanged();
}

void LogVariableData::assignFrom(const QVector<double> &rSrc)
{
    mpCachedDataVector->replaceData(rSrc);
    mSharedTimeVectorPtr = SharedTimeVectorPtrT(); //No time vector assigned
    emit dataChanged();
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

double LogVariableData::minOfData() const
{
    double ret = std::numeric_limits<double>::max();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) < ret)
        {
            ret = pVector->at(i);
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double LogVariableData::maxOfData() const
{
    double ret = std::numeric_limits<double>::min();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) > ret)
        {
            ret = pVector->at(i);
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

bool LogVariableData::indexInRange(const int idx) const
{
    //! @todo Do we need to check timevector also ? (or should we assume thay are the same)
    return (idx>=0 && idx<mpCachedDataVector->size());
}

LogDataHandler *LogVariableData::getLogDataHandler()
{
    //! @todo will crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 0;
    }
    return mpParentVariableContainer->getLogDataHandler();
}

QString LogVariableContainer::getAliasName() const
{
    return mVariableDescription->mAliasName;
}

QString LogVariableContainer::getFullVariableName() const
{
    return mVariableDescription->getFullName();
}

QString LogVariableContainer::getFullVariableNameWithSeparator(const QString sep) const
{
    return mVariableDescription->getFullNameWithSeparator(sep);
}

QString LogVariableContainer::getModelPath() const
{
    return mVariableDescription->mModelPath;
}

QString LogVariableContainer::getComponentName() const
{
    return mVariableDescription->mComponentName;
}

QString LogVariableContainer::getPortName() const
{
    return mVariableDescription->mPortName;
}

QString LogVariableContainer::getDataName() const
{
    return mVariableDescription->mDataName;
}

QString LogVariableContainer::getDataUnit() const
{
    return mVariableDescription->mDataUnit;
}

LogDataHandler *LogVariableContainer::getLogDataHandler()
{
    return mpParentLogDataHandler;
}

void LogVariableContainer::setAliasName(const QString alias)
{
    mVariableDescription->mAliasName = alias;
    emit nameChanged();
}

QString VariableDescription::getVarTypeString() const
{
    switch (mVarType)
    {
    case S :
        return "Script";
        break;
    case ST :
        return "Script_Temp";
        break;
    case M :
        return "Model";
        break;
    case I :
        return "Import";
        break;
    default :
        return "UNDEFINEDTYPE";
    }
    return "";           //Needed for VC compilations
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

SharedVariableDescriptionT LogVariableContainer::getVariableDescription() const
{
    return mVariableDescription;
}

SharedLogVariableDataPtrT LogVariableContainer::getDataGeneration(const int gen)
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

bool LogVariableContainer::hasDataGeneration(const int gen)
{
    return mDataGenerations.contains(gen);
}

//! @todo not two functions with almost same implementation
void LogVariableContainer::addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData)
{
    SharedLogVariableDataPtrT pData;
    if (mpParentLogDataHandler)
    {
        pData = SharedLogVariableDataPtrT(new LogVariableData(generation, rTime, rData, mVariableDescription, mpParentLogDataHandler->getGenerationMultiCache(generation), this));
    }
    else
    {
        pData = SharedLogVariableDataPtrT(new LogVariableData(generation, rTime, rData, mVariableDescription, SharedMultiDataVectorCacheT(), this));
    }
    //! @todo what if a generation already exist, then we must properly delete the old data before we add new one

    connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
    mDataGenerations.insert(generation, pData);
}

void LogVariableContainer::addDataGeneration(const int generation, const SharedTimeVectorPtrT time, const QVector<double> &rData)
{
    SharedLogVariableDataPtrT pData;
    if (mpParentLogDataHandler)
    {
        pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableDescription, mpParentLogDataHandler->getGenerationMultiCache(generation), this));
    }
    else
    {
        pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableDescription, SharedMultiDataVectorCacheT(), this));
    }
    //! @todo what if a generation already exist, then we must properly delete the old data before we add new one

    connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
    mDataGenerations.insert(generation, pData);
}

//! @todo Need to remove this class if final generation is deleted
void LogVariableContainer::removeDataGeneration(const int generation)
{
    //! @todo cache data will still be in the cachegenreationmap, need to clear whenevevr generation is removed (from anywere), mabe should restore inc dec Subscribers
    mDataGenerations.remove(generation);
}

void LogVariableContainer::removeGenerationsOlderThen(const int gen)
{
    // It is assumed that the generation map is sorted by key which it should be since adding will allways append
    QList<int> gens = mDataGenerations.keys();
    for (int it=0; it<gens.size(); ++it)
    {
        if ( gens[it] < gen )
        {
            removeDataGeneration(gens[it]);
        }
        else
        {
            // There is no reason to continue the loop if we have found  gens[it] = gen
            break;
        }
    }
}

void LogVariableContainer::removeAllGenerations()
{
    //! @todo cache data will still be in the cachegenreationmap, need to clear whenevevr generation is removed (from anywere), mabe should restore inc dec Subscribers
    mDataGenerations.clear();
}

LogVariableContainer::LogVariableContainer(const VariableDescription &rVarDesc, LogDataHandler *pParentLogDataHandler) : QObject()
{
    mVariableDescription = SharedVariableDescriptionT(new VariableDescription(rVarDesc)); //Copy original data and create a new shared variable description
    mpParentLogDataHandler = pParentLogDataHandler;
}

LogVariableContainer::~LogVariableContainer()
{
    // Clear all data
    mDataGenerations.clear();
}

SharedTimeVectorPtrT UniqueSharedTimeVectorPtrHelper::makeSureUnique(QVector<double> &rTimeVector)
{
    const int nElements = rTimeVector.size();
    if (nElements > 0)
    {
        const double newFirst = rTimeVector[0];
        const double newLast = rTimeVector[nElements-1];

        for (int idx=0; idx<mSharedTimeVecPointers.size(); ++idx)
        {
            const int oldElements = mSharedTimeVecPointers[idx]->size();
            const double oldFirst = mSharedTimeVecPointers[idx]->at(0);
            const double oldLast = mSharedTimeVecPointers[idx]->at(oldElements-1);
            if ( (oldElements == nElements) &&
                 fuzzyEqual(newFirst, oldFirst, 1e-10) &&
                 fuzzyEqual(newLast, oldLast, 1e-10) )
            {
                return mSharedTimeVecPointers[idx];
            }
        }

        // If we did not already return then add this pointer
        QVector<double>* pNewVector = new QVector<double>(rTimeVector);
        mSharedTimeVecPointers.append(SharedTimeVectorPtrT(pNewVector));
        return mSharedTimeVecPointers.last();
    }
    return SharedTimeVectorPtrT();
}


QVector<double> LogVariableData::getDataVector()
{
    QVector<double> vec;
    mpCachedDataVector->copyDataTo(vec);
    return vec;
}

int LogVariableData::getDataSize() const
{
    return mpCachedDataVector->size();
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



