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
#include "Utilities/GUIUtilities.h"

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

void LogVariableData::setValueOffset(double offset)
{
    mAppliedValueOffset += offset;
    for (int i=0; i<mDataVector.size(); ++i)
    {
        mDataVector[i] += offset;
    }

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

LogVariableData::LogVariableData(const int generation, const QVector<double> &rTime, const QVector<double> &rData, SharedVariableDescriptionT varDesc, LogVariableContainer *pParent)
{
    mpParentVariableContainer = pParent;
    mpVariableDescription = varDesc;
    mAppliedValueOffset = 0;
    mAppliedTimeOffset = 0;
    mGeneration = generation;
    mSharedTimeVectorPtr = SharedTimeVectorPtrT(new QVector<double>(rTime));
    mDataVector = rData;
}

LogVariableData::LogVariableData(const int generation, SharedTimeVectorPtrT time, const QVector<double> &rData, SharedVariableDescriptionT varDesc, LogVariableContainer *pParent)
{
    mpParentVariableContainer = pParent;
    mpVariableDescription = varDesc;
    mAppliedValueOffset = 0;
    mAppliedTimeOffset = 0;
    mGeneration = generation;
    mSharedTimeVectorPtr = time;
    mDataVector = rData;
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
    for (int i=0; i<mDataVector.size(); ++i)
    {
       mDataVector[i] += pOther->mDataVector[i];
    }
}
void LogVariableData::addToData(const double other)
{
    for (int i=0; i<mDataVector.size(); ++i)
    {
        mDataVector[i] += other;
    }
}
void LogVariableData::subFromData(const SharedLogVariableDataPtrT pOther)
{
    //! @todo DANGER will crash if other not as long (also in other places)
    for (int i=0; i<mDataVector.size(); ++i)
    {
        mDataVector[i] -= pOther->mDataVector[i];
    }
}
void LogVariableData::subFromData(const double other)
{
    for (int i=0; i<mDataVector.size(); ++i)
    {
        mDataVector[i] -= other;
    }
}

void LogVariableData::multData(const SharedLogVariableDataPtrT pOther)
{
    for (int i=0; i<mDataVector.size(); ++i)
    {
       mDataVector[i] *= pOther->mDataVector[i];
    }

}

void LogVariableData::multData(const double other)
{
    for (int i=0; i<mDataVector.size(); ++i)
    {
        mDataVector[i] *= other;
    }
}

void LogVariableData::divData(const SharedLogVariableDataPtrT pOther)
{
    for (int i=0; i<mDataVector.size(); ++i)
    {
       mDataVector[i] /= pOther->mDataVector[i];
    }
}

void LogVariableData::divData(const double other)
{
    for (int i=0; i<mDataVector.size(); ++i)
    {
        mDataVector[i] /= other;
    }
}
void LogVariableData::assignToData(const SharedLogVariableDataPtrT pOther)
{
    mDataVector = pOther->mDataVector;
    mSharedTimeVectorPtr = pOther->mSharedTimeVectorPtr;
}

bool LogVariableData::pokeData(const int index, const double value)
{

    if (index >= 0 && index < mDataVector.size())
    {
        mDataVector[index] = value;
        emit dataChanged();
        return true;
    }
    return false;
}

double LogVariableData::peekData(const int index)
{
    //! @todo check index range, figure out whay kind of error to return
    return mDataVector[index];
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

void LogVariableContainer::addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData)
{
    //! @todo what if a generation already exist, then we must properly delete the old data before we add new one
    SharedLogVariableDataPtrT pData = SharedLogVariableDataPtrT(new LogVariableData(generation, rTime, rData, mVariableDescription, this));
    connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
    mDataGenerations.insert(generation, pData);
}

void LogVariableContainer::addDataGeneration(const int generation, const SharedTimeVectorPtrT time, const QVector<double> &rData)
{
    //! @todo what if a generation already exist, then we must properly delete the old data before we add new one
    SharedLogVariableDataPtrT pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableDescription, this));
    connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
    mDataGenerations.insert(generation, pData);
}

//! @todo Need to remove this class if final generation is deleted
void LogVariableContainer::removeDataGeneration(const int generation)
{
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
