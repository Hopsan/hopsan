//!
//! @file   LogDataGeneration.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   20150504
//!
//! @brief Contains the LogDataGeneration class
//!
//$Id$

#ifndef LOGDATAGENERATION_H
#define LOGDATAGENERATION_H

#include <QList>
#include <QString>
#include <QObject>
#include <QMap>

#include "LogVariable.h"

// Forward Declaration
class LogDataHandler2;


class LogDataGeneration : public QObject
{
    Q_OBJECT

    friend class LogDataHandler2;
public:
    LogDataGeneration(const QString &rImportfile="");
    ~LogDataGeneration();

    int getGenerationNumber() const;

    int getNumVariables() const;
    int getNumKeepVariables() const;
    bool isEmpty();
    bool clear(bool force);

    bool isImported() const;
    QString getImportFileName() const;

    void addVariable(const QString &rFullName, SharedVectorVariableT variable, bool isAlias);
    bool removeVariable(const QString &rFullName);

    QStringList getVariableFullNames() const;
    bool haveVariable(const QString &rFullName) const;

    SharedVectorVariableT getVariable(const QString &rFullName) const;
    QList<SharedVectorVariableT> getMatchingVariables(const QRegExp &rNameExp);

    QList<SharedVectorVariableT> getAllNonAliasVariables() const;
    QList<SharedVectorVariableT> getAllVariables() const;

    bool registerQuantity(const QString &rFullName, const QString &rQuantity);

    bool registerAlias(const QString &rFullName, const QString &rAlias);
    bool unregisterAlias(const QString &rAlias);
    bool unregisterAliasForFullName(const QString &rFullName);
    QString getFullNameFromAlias(const QString &rAlias);

    void switchGenerationDataCache(SharedMultiDataVectorCacheT pDataCache);

private slots:
    void variableAutoRemovalChanged(bool allowRemoval);

private:
    typedef QMap< QString, SharedVectorVariableT > VariableMapT;

    QList<SharedVectorVariableT>  getMatchingVariables(const QRegExp &rNameExp, VariableMapT &rMap);

    VariableMapT mVariables;
    VariableMapT mAliasVariables;
    QList<SharedVectorVariableT> mTimeVectors; //!< @todo use this
    int mNumKeepVariables = 0;

    QString mImportedFromFile;
};

#endif // LOGDATAGENERATION_H
