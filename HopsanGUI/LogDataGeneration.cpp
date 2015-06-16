//!
//! @file   LogDataGeneration.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   20150504
//!
//! @brief Contains the LogDataGeneration class
//!

#include "LogDataGeneration.h"
#include "global.h"
#include "Configuration.h"

LogDataGeneration::LogDataGeneration(const QString &rImportfile)
{
    mImportedFromFile = rImportfile;
}

LogDataGeneration::~LogDataGeneration()
{
    clear(true);
}

int LogDataGeneration::getGenerationNumber() const
{
    SharedVectorVariableT first;
    // Ask first variable
#if QT_VERSION >= 0x050200
    first = mVariables.first();
    if (!first)
    {
        first = mAliasVariables.first();
    }
#else
    if (!mVariables.isEmpty() || !mAliasVariables.isEmpty())
    {
        if (!mVariables.isEmpty())
        {
            first = mVariables.begin().value();
        }
        else
        {
            first = mAliasVariables.begin().value();
        }
    }
#endif
    if (first)
    {
        return first->getGeneration();
    }
    else
    {
        return -1;
    }
}

int LogDataGeneration::getNumVariables() const
{
    return mVariables.count();
}

int LogDataGeneration::getNumKeepVariables() const
{
    return mNumKeepVariables;
}


bool LogDataGeneration::isEmpty()
{
    return mVariables.isEmpty();
}


bool LogDataGeneration::clear(bool force)
{
    // Loop through variables but only remove those that are not tagged as keep (unless forced removal)
    // We use a copy of values to avoid making the loop invalid
    QList<SharedVectorVariableT> avars = mAliasVariables.values();
    for (SharedVectorVariableT &avar : avars)
    {
        if (force || avar->isAutoremovalAllowed())
        {
            disconnect(avar.data(), SIGNAL(allowAutoRemovalChanged(bool)), this, SLOT(variableAutoRemovalChanged(bool)));
            mAliasVariables.remove(avar->getAliasName());
        }
    }
    QList<SharedVectorVariableT> vars =  mVariables.values();
    for (SharedVectorVariableT &var : vars)
    {
        if (force || var->isAutoremovalAllowed())
        {
            disconnect(var.data(), SIGNAL(allowAutoRemovalChanged(bool)), this, SLOT(variableAutoRemovalChanged(bool)));
            emit var->beingRemoved();
            mVariables.remove(var->getFullVariableName());
        }
    }

    // If no variables remain
    if (mAliasVariables.isEmpty() && mVariables.isEmpty())
    {
        mImportedFromFile.clear();
        mNumKeepVariables = 0;
        return true;
    }
    else
    {
        return false;
    }
}


bool LogDataGeneration::isImported() const
{
    return !mImportedFromFile.isEmpty();
}


QString LogDataGeneration::getImportFileName() const
{
    return mImportedFromFile;
}


QStringList LogDataGeneration::getVariableFullNames() const
{
    QStringList retval;
    for (auto dit = mVariables.begin(); dit!=mVariables.end(); ++dit)
    {
        retval.append(dit.key());
    }
    return retval;
}

bool LogDataGeneration::haveVariable(const QString &rFullName) const
{
    return mVariables.contains(rFullName);
}


void LogDataGeneration::addVariable(const QString &rFullName, SharedVectorVariableT variable, bool isAlias)
{
    if (variable)
    {
        if (isAlias)
        {
            mAliasVariables.insert(rFullName, variable);
        }
        else
        {
            mVariables.insert(rFullName, variable);
        }

        if (!variable->isAutoremovalAllowed())
        {
            // This will increment numKeepVariables one and emit signal to log data handler
            variableAutoRemovalChanged(true);
        }

        connect(variable.data(), SIGNAL(allowAutoRemovalChanged(bool)), this, SLOT(variableAutoRemovalChanged(bool)));
    }
}

bool LogDataGeneration::removeVariable(const QString &rFullName)
{
    bool didRemove = false;

    // First try alias
    auto alias_it = mAliasVariables.find(rFullName);
    if (alias_it != mAliasVariables.end())
    {
        disconnect(alias_it.value().data(), SIGNAL(allowAutoRemovalChanged(bool)), this, SLOT(variableAutoRemovalChanged(bool)));
        mAliasVariables.erase(alias_it);
        didRemove = true;
    }
    // If not alias then remove actual variable
    else
    {
        auto full_it = mVariables.find(rFullName);
        if (full_it != mVariables.end())
        {
            disconnect(full_it.value().data(), SIGNAL(allowAutoRemovalChanged(bool)), this, SLOT(variableAutoRemovalChanged(bool)));
            emit full_it.value()->beingRemoved();
            mVariables.erase(full_it);
            didRemove = true;
        }
    }
    return didRemove;
}

SharedVectorVariableT LogDataGeneration::getVariable(const QString &rFullName) const
{
    SharedVectorVariableT tmp = mAliasVariables.value(rFullName, SharedVectorVariableT());
    if (!tmp)
    {
        tmp = mVariables.value(rFullName, SharedVectorVariableT());
    }
    return tmp;
}


//! @brief Returns multiple logdata variables based on regular expression search. Excluding temp variables but including aliases
//! @param [in] rNameExp The regular expression for the names to match
QList<SharedVectorVariableT> LogDataGeneration::getMatchingVariables(const QRegExp &rNameExp)
{
    QList<SharedVectorVariableT> results;
    results.append(getMatchingVariables(rNameExp, mAliasVariables));
    results.append(getMatchingVariables(rNameExp, mVariables));
    return results;
}

QList<SharedVectorVariableT> LogDataGeneration::getAllNonAliasVariables() const
{
    return mVariables.values();
}

QList<SharedVectorVariableT> LogDataGeneration::getAllVariables() const
{
    return mAliasVariables.values()+mVariables.values();
}

bool LogDataGeneration::registerQuantity(const QString &rFullName, const QString &rQuantity)
{
    SharedVectorVariableT var = getVariable(rFullName);
    QString bu = gpConfig->getBaseUnit(rQuantity);
    if (!bu.isEmpty())
    {
        var->mpVariableDescription->mDataQuantity = rQuantity;
        var->mpVariableDescription->mDataUnit = bu;
        emit var.data()->quantityChanged();
        return true;
    }
    return false;
}

bool LogDataGeneration::registerAlias(const QString &rFullName, const QString &rAlias)
{
    SharedVectorVariableT pFullVar = getVariable(rFullName);
    if (pFullVar)
    {
        // If alias is empty then we should unregister the alias
        if (rAlias.isEmpty())
        {
            return unregisterAliasForFullName(rFullName);
        }
        else
        {
            // If we get here then we should set a new alias or replace the previous one
            // First unregister the previous alias
            if (pFullVar->hasAliasName())
            {
                unregisterAliasForFullName(rFullName);
            }
            // Now insert the full data as new alias
            // existing data with alias name will be replaced (removed)
            pFullVar->mpVariableDescription->mAliasName = rAlias;
            addVariable(rAlias, pFullVar, true);
            //! @todo this will overwrite existing fullname if alias same as a full name /Peter
            return true;
        }
    }
    return false;
}

bool LogDataGeneration::unregisterAlias(const QString &rAlias)
{
    QString fullName = getFullNameFromAlias(rAlias);
    if (!fullName.isEmpty())
    {
        return unregisterAliasForFullName(fullName);
    }
    return false;
}

bool LogDataGeneration::unregisterAliasForFullName(const QString &rFullName)
{
    SharedVectorVariableT pFullVar = getVariable(rFullName);
    if (pFullVar && pFullVar->hasAliasName())
    {
        QString alias = pFullVar->getAliasName();
        pFullVar->mpVariableDescription->mAliasName = "";
        removeVariable(alias);
        return true;
    }
    return false;
}

//! @brief Returns plot variable for specified alias
//! @param[in] rAlias Alias of variable
QString LogDataGeneration::getFullNameFromAlias(const QString &rAlias)
{
    SharedVectorVariableT pAliasVar = mAliasVariables.value(rAlias);
    if (pAliasVar)
    {
        return pAliasVar->getFullVariableName();
    }
    return QString();
}

void LogDataGeneration::switchGenerationDataCache(SharedMultiDataVectorCacheT pDataCache)
{
    for (auto it=mVariables.begin(); it!=mVariables.end(); ++it)
    {
        SharedVectorVariableT &data = it.value();
        data->mpCachedDataVector->switchCacheFile(pDataCache);
        if (data->mpSharedTimeOrFrequencyVector)
        {
            data->mpSharedTimeOrFrequencyVector->mpCachedDataVector->switchCacheFile(pDataCache);
        }
    }
}

void LogDataGeneration::variableAutoRemovalChanged(bool allowRemoval)
{
    if (allowRemoval)
    {
        mNumKeepVariables--;
    }
    else
    {
        mNumKeepVariables++;
    }
}

QList<SharedVectorVariableT> LogDataGeneration::getMatchingVariables(const QRegExp &rNameExp, LogDataGeneration::VariableMapT &rMap)
{
    QList<SharedVectorVariableT> results;
    for (auto it = rMap.begin(); it!=rMap.end(); it++)
    {
        // Compare name with regexp
        QString name = it.key();
        if ( rNameExp.exactMatch(name) )
        {
            results.append(it.value());
        }
    }
    return results;
}

