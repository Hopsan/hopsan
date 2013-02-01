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
    return mpParentVariableContainer->getLowestGeneration();
}

int LogVariableData::getHighestGeneration() const
{
    //! @todo will crash if container removed before data
    return mpParentVariableContainer->getHighestGeneration();
}

int LogVariableData::getNumGenerations() const
{
    //! @todo will crash if container removed before data
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
void LogVariableData::assignToData(const SharedLogVariableDataPtrT pOther)
{
    mpCachedDataVector->replaceData(pOther->getDataVector());
    mSharedTimeVectorPtr = pOther->mSharedTimeVectorPtr;
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

bool LogVariableData::indexInRange(const int idx) const
{
    //! @todo Do we need to check timevector also ? (or should we assume thay are the same)
    return (idx>=0 && idx<mpCachedDataVector->size());
}

LogDataHandler *LogVariableData::getLogDataHandler()
{
    //! @todo will crash if container removed before data
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

LogVariableContainer::LogVariableContainer(const SharedVariableDescriptionT &rVarDesc, LogDataHandler *pParentLogDataHandler) : QObject()
{
    mVariableDescription = rVarDesc;
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
    mpCachedDataVector->copyData(vec);
    return vec;
}

int LogVariableData::getDataSize() const
{
    return mpCachedDataVector->size();
}


CachedSingleDataVector::CachedSingleDataVector(const QVector<double> &rDataVector, const QString fileName)
{
    mNumElements = 0;

    if (fileName.isEmpty())
    {
        mDataVector = rDataVector;
        mNumElements = mDataVector.size();
    }
    else
    {
        if (setCacheFile(fileName))
        {
            writeToCache(rDataVector);
        }
    }
}

CachedSingleDataVector::~CachedSingleDataVector()
{
    mCacheFile.remove();
}

bool CachedSingleDataVector::setCacheFile(const QString fileName)
{
    // Prevent changing filename if file has already been created
    if (!mCacheFile.exists())
    {
        mCacheFile.setFileName(fileName);
        return true;
    }
    return false;
}


bool CachedSingleDataVector::isCached() const
{
    return mDataVector.isEmpty();
}

int CachedSingleDataVector::size() const
{
    return mNumElements;
}

bool CachedSingleDataVector::isEmpty() const
{
    return (mNumElements==0);
}

bool CachedSingleDataVector::copyData(QVector<double> &rData)
{
    if (isCached())
    {
        return readToMem(rData);
    }
    else
    {
        rData = mDataVector;
        return true;
    }
}

bool CachedSingleDataVector::replaceData(const QVector<double> &rNewData)
{
    if (isCached())
    {
        return writeToCache(rNewData);
    }
    else
    {
        mDataVector = rNewData;
        mNumElements = mDataVector.size();
        return true;
    }
}

bool CachedSingleDataVector::peek(const int idx, double &rVal)
{
    if (isCached())
    {
        if (mCacheFile.open(QIODevice::ReadOnly))
        {
            bool rc = mCacheFile.seek(sizeof(double)*idx);
            if (rc)
            {
                qint64 n = mCacheFile.peek((char*)&rVal, sizeof(double));
                mCacheFile.close();
                return true;
            }
            else
            {
                mCacheFile.close();
                mError = mCacheFile.errorString();
            }
        }
        else
        {
            mError = mCacheFile.errorString();
        }
        return false;
    }
    else
    {
        rVal = mDataVector[idx];
        return true;
    }
}

bool CachedSingleDataVector::poke(const int idx, const double val)
{
    if (isCached())
    {
        if (mCacheFile.open(QIODevice::ReadWrite))
        {
            bool rc = mCacheFile.seek(sizeof(double)*idx);
            if (rc)
            {
                qint64 n = mCacheFile.write((const char*)&val, sizeof(double));
                //! @todo handle writing out of bounds
                mCacheFile.close();
                return true;
            }
            else
            {
                mCacheFile.close();
                mError = mCacheFile.errorString();
            }
        }
        else
        {
            mError = mCacheFile.errorString();
        }
        return false;
    }
    else
    {
        mDataVector[idx] = val;
        return true;
    }
}

QVector<double> *CachedSingleDataVector::beginFullVectorOperation()
{
    QVector<double> *pData;
    if(isCached())
    {
        pData = new QVector<double>();
        readToMem(*pData);
    }
    else
    {
        pData = &mDataVector;
    }
    return pData;
}

//! @todo for now it is dangerous (very bad) to make operations between begin end (that do not operate only on the pointer), should this be blocked or even allowed through a theird state
bool CachedSingleDataVector::endFullVectorOperation(QVector<double> *&rpData)
{
    bool rc = true;
    if(isCached())
    {
        rc = writeToCache(*rpData);
        delete rpData;
    }
    rpData = 0;
    return rc;
}

QString CachedSingleDataVector::getError() const
{
    return mError;
}

bool CachedSingleDataVector::moveToCache()
{
    bool rc = true;
    if (!isCached())
    {
        rc = writeToCache(mDataVector);
        mDataVector.clear();
    }
    return rc;
}

bool CachedSingleDataVector::readToMem(QVector<double> &rDataVector)
{
    if (mCacheFile.open(QIODevice::ReadOnly))
    {
        rDataVector.resize(mNumElements);
        qint64 n = mCacheFile.read((char*)rDataVector.data(), sizeof(double)*mNumElements);
        if (n != sizeof(double)*mNumElements)
        {
            mError = mCacheFile.errorString();//"Failed to read all expected data from file";
            mCacheFile.close();
            return false;
        }
        mCacheFile.close();
        return true;
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}

bool CachedSingleDataVector::moveToMem()
{
    bool rc = true;
    if (isCached())
    {
        rc = readToMem(mDataVector);
        if (!rc)
        {
            mDataVector.clear();
        }
        //! @todo should we remove file ? on succfull move
    }
    return rc;
}

bool CachedSingleDataVector::writeToCache(const QVector<double> &rDataVector)
{
    if (mCacheFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qint64 n = mCacheFile.write((const char*)rDataVector.data(), sizeof(double)*rDataVector.size());
        if (n != sizeof(double)*rDataVector.size())
        {
            mError = mCacheFile.errorString();//"Failed to write data to file";
            mCacheFile.close();
            return false;
        }
        mNumElements = rDataVector.size();
        mCacheFile.close();
        return true;
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}


bool CachedSingleDataVector::setCached(const bool cached)
{
    bool rc;
    if (cached)
    {
        rc = moveToCache();
    }
    else
    {
        rc = moveToMem();
    }
    return rc;
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


MultiDataVectorCache::MultiDataVectorCache(const QString fileName)
{
    mIsMultiAppending = false;
    mNumSubscribers = 0;
    mCacheFile.setFileName(fileName);
}

MultiDataVectorCache::~MultiDataVectorCache()
{
    removeCacheFile();
}

bool MultiDataVectorCache::addVector(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes)
{
    //! @todo maybe have register for how long added data at each start adress was, to prevent destorying data
    return appendToCache(rDataVector, rStartByte, rNumBytes);
}

bool MultiDataVectorCache::beginMultiAppend()
{
    mIsMultiAppending = mCacheFile.open(QIODevice::WriteOnly | QIODevice::Append);
    return mIsMultiAppending;
}

bool MultiDataVectorCache::endMultiAppend()
{
    mCacheFile.close();
    mIsMultiAppending = false;
}

bool MultiDataVectorCache::copyData(const quint64 startByte, const quint64 nBytes, QVector<double> &rData)
{
    return readToMem(startByte, nBytes, &rData);
}

bool MultiDataVectorCache::replaceData(const quint64 startByte, const QVector<double> &rNewData)
{
    //! @todo prevent destroying data if new data have new length
    return writeInCache(startByte, rNewData);
}


bool MultiDataVectorCache::writeInCache(const quint64 startByte, const QVector<double> &rDataVector)
{
    if (mCacheFile.open(QIODevice::ReadWrite))
    {
        if (mCacheFile.seek(startByte))
        {
            qint64 n = mCacheFile.write((const char*)rDataVector.data(), sizeof(double)*rDataVector.size());
            //! @todo handle writing out of bounds
            mCacheFile.close();
            return true;
        }
        else
        {
            mCacheFile.close();
            mError = mCacheFile.errorString();
        }
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}

bool MultiDataVectorCache::appendToCache(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes)
{
    rNumBytes = 0;

    // Abort if data vector empty
    if (rDataVector.isEmpty())
    {
        return false;
    }

    //! @todo cleanup horrible multiappend stuff
    bool rc = true;
    if (!mIsMultiAppending)
    {
        rc = mCacheFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    if (rc)
    {
        rStartByte = mCacheFile.pos();
        rNumBytes = mCacheFile.write((const char*)rDataVector.data(), sizeof(double)*rDataVector.size());
        if ( rNumBytes == sizeof(double)*rDataVector.size())
        {
            if (!mIsMultiAppending)
            {
                mCacheFile.close();
            }
            return true;
        }
        else
        {
            if (!mIsMultiAppending)
            {
                mCacheFile.close();
            }
            mError = mCacheFile.errorString();
        }
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}

bool MultiDataVectorCache::readToMem(const quint64 startByte, const quint64 nBytes, QVector<double> *pDataVector)
{
    if (mCacheFile.open(QIODevice::ReadOnly))
    {
        if (mCacheFile.seek(startByte))
        {
            pDataVector->resize(nBytes/sizeof(double)); //!< @todo should check that division is becomes a resonable value
            qint64 n = mCacheFile.read((char*)pDataVector->data(), nBytes);
            if (n == nBytes)
            {
                mCacheFile.close();
                return true;
            }
            mError = mCacheFile.errorString();
        }
        else
        {
            mError = mCacheFile.errorString();
        }
        mCacheFile.close();
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}

void MultiDataVectorCache::removeCacheFile()
{
    bool rc = mCacheFile.remove();
    qDebug() << "Removing file: " << mCacheFile.fileName() << " : " << rc;
}


bool MultiDataVectorCache::peek(const quint64 byte, double &rVal)
{
    if (mCacheFile.open(QIODevice::ReadOnly))
    {
        if (mCacheFile.seek(byte))
        {
            qint64 n = mCacheFile.peek((char*)&rVal, sizeof(double));
            mCacheFile.close();
            return true;
        }
        else
        {
            mCacheFile.close();
            mError = mCacheFile.errorString();
        }
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}

bool MultiDataVectorCache::poke(const quint64 byte, const double val)
{
    if (mCacheFile.open(QIODevice::ReadWrite))
    {
        if (mCacheFile.seek(byte))
        {
            qint64 n = mCacheFile.write((const char*)&val, sizeof(double));
            //! @todo handle writing out of bounds
            mCacheFile.close();
            return true;
        }
        else
        {
            mCacheFile.close();
            mError = mCacheFile.errorString();
        }
    }
    else
    {
        mError = mCacheFile.errorString();
    }
    return false;
}

bool MultiDataVectorCache::checkoutVector(const quint64 startByte, const quint64 nBytes, QVector<double> *&rpData)
{
    //! @todo how to prevent checkout of same data multiple times
    QVector<double> *pData = new QVector<double>();
    if (readToMem(startByte, nBytes, pData))
    {
        rpData = pData;
        mCheckoutMap.insert(pData, CheckoutInfoT(startByte,nBytes));
        return true;
    }
    delete pData;
    return false;
}

bool MultiDataVectorCache::returnVector(QVector<double> *&rpData)
{
    bool rc = false;
    if (mCheckoutMap.contains(rpData))
    {
        CheckoutInfoT info = mCheckoutMap.value(rpData, CheckoutInfoT(0,0));
        if (info.nBytes == rpData->size()*sizeof(double))
        {
            rc = writeInCache(info.startByte, *rpData);
            delete rpData;
            rpData = 0;
        }
        else
        {
            mError = "Trying to write new data that is to long or to short";
            //! @todo handle this instead, append maybe
        }
    }
    else
    {
        mError = "Pointer to data that has not been checked out";
    }
    return rc;
}


QString MultiDataVectorCache::getError() const
{
    return mError;
}

void MultiDataVectorCache::incrementSubscribers()
{
    ++mNumSubscribers;
}

//! @brief Decrement num subscribers, if no subscribers remain the cache file will be deleted (even if this object still remains).
void MultiDataVectorCache::decrementSubscribers()
{
    // If mNumSubs are 0or1 before this subtraction (it will be the last subscriber), 0 case should not happen unless someone has forgotten to increment
    if ( mNumSubscribers-- <=1)
    {
        removeCacheFile();
    }
}


CachableDataVector::CachableDataVector(const QVector<double> &rDataVector, SharedMultiDataVectorCacheT pMultiCache, const bool cached)
{
    //! @todo make it possible to add data with multicahce but default not cached, need some way of remembering whterer data has been cached, maybe change NumElements to ncachedBytes
    mCacheStartByte = 0;
    mCacheNumBytes = 0;

    if ( (pMultiCache == 0) || !cached )
    {
        mDataVector = rDataVector;
    }
    else
    {
        mpMultiCache = pMultiCache;
        if (mpMultiCache->addVector(rDataVector,mCacheStartByte, mCacheNumBytes))
        {
            mpMultiCache->incrementSubscribers();
        }
        else
        {
            mError = mpMultiCache->getError();
        }
    }
}

CachableDataVector::~CachableDataVector()
{
    if ( mpMultiCache && mCacheNumBytes > 0  )
    {
        mpMultiCache->decrementSubscribers();
    }
}

bool CachableDataVector::setCached(const bool cached)
{
    bool rc;
    if (cached)
    {
        rc = moveToCache();
    }
    else
    {
        rc = copyToMem();
    }
    return rc;
}

bool CachableDataVector::isCached() const
{
    return mDataVector.isEmpty();
}

int CachableDataVector::size() const
{
    if (isCached())
    {
        return mCacheNumBytes/sizeof(double);
    }
    else
    {
        return mDataVector.size();
    }
}

bool CachableDataVector::isEmpty() const
{
    return (mCacheNumBytes == 0) && mDataVector.isEmpty();
}

bool CachableDataVector::copyData(QVector<double> &rData)
{
    if (isCached())
    {
        if (!mpMultiCache->copyData(mCacheStartByte, mCacheNumBytes, rData))
        {
            mError = mpMultiCache->getError();
            return false;
        }
    }
    else
    {
        rData = mDataVector;
    }
    return true;
}

bool CachableDataVector::replaceData(const QVector<double> &rNewData)
{
    //! @todo do this
    qFatal("Not yet implemented");
}

bool CachableDataVector::peek(const int idx, double &rVal)
{
    if (isCached())
    {
        if (!mpMultiCache->peek(mCacheStartByte+idx*sizeof(double), rVal))
        {
            mError = mpMultiCache->getError();
            return false;
        }
    }
    else
    {
        //! @todo bounds check
        rVal = mDataVector[idx];
    }
    return true;
}

bool CachableDataVector::poke(const int idx, const double val)
{
    if (isCached())
    {
        if (!mpMultiCache->poke(mCacheStartByte+idx*sizeof(double),val))
        {
            mError = mpMultiCache->getError();
            return false;
        }
    }
    else
    {
        //! @todo bounds check
        mDataVector[idx] = val;
    }
    return true;
}

QVector<double> *CachableDataVector::beginFullVectorOperation()
{
    //! @todo what if trying to checkout vector that has not been created
    QVector<double> *pData = 0;
    if(isCached())
    {
        if (!mpMultiCache->checkoutVector(mCacheStartByte, mCacheNumBytes, pData))
        {
            mError = mpMultiCache->getError();
        }
    }
    else
    {
        pData = &mDataVector;
    }
    return pData;
}

bool CachableDataVector::endFullVectorOperation(QVector<double> *&rpData)
{
    bool rc = true;
    if(isCached())
    {
        rc = mpMultiCache->returnVector(rpData);
        if (!rc)
        {
            mError = mpMultiCache->getError();
        }
    }
    rpData = 0;
    return rc;
}

QString CachableDataVector::getError() const
{
    return mError;
}

//! @todo handle if new vector is longer or shorter then cahced one
bool CachableDataVector::moveToCache()
{
    if (mpMultiCache)
    {
        if (mCacheNumBytes == 0)
        {
            if (mpMultiCache->addVector(mDataVector, mCacheStartByte, mCacheNumBytes))
            {
                mpMultiCache->incrementSubscribers();
                mDataVector.clear();
                return true;
            }
        }
        else if (mpMultiCache->replaceData(mCacheStartByte, mDataVector))
        {
            mDataVector.clear();
            return true;
        }
        mError = mpMultiCache->getError();
        return false;
    }
    mError = "No MultiCache set";
    return false;
}

bool CachableDataVector::copyToMem()
{
    if (mpMultiCache && (mCacheNumBytes > 0))
    {
        if (mpMultiCache->copyData(mCacheStartByte, mCacheNumBytes, mDataVector))
        {
            return true;
        }
        mError = mpMultiCache->getError();
        return false;
    }
    mError = "No cached data available";
    return false;
}
