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
    bool removeVariable(const QString &rAliasOrFullName, const VariableNameTypeT nametype=AliasAndFull);

    QStringList getVariableFullNames() const;
    bool haveVariable(const QString &rFullName) const;
    bool haveAlias(const QString &rAliasName) const;

    SharedVectorVariableT getVariable(const QString &rFullName) const;
    QList<SharedVectorVariableT> getMatchingVariables(const QRegExp &rNameExp, const VariableNameTypeT nametype=AliasAndFull);

    QList<SharedVectorVariableT> getAllAliasVariables() const;
    QList<SharedVectorVariableT> getAllNonAliasVariables() const;
    QList<SharedVectorVariableT> getAllVariables() const;

    bool registerQuantity(const QString &rFullName, const QString &rQuantity);

    bool registerAlias(const QString &rFullName, const QString &rAlias);
    bool unregisterAlias(const QString &rAlias);
    bool unregisterAliasForFullName(const QString &rFullName);
    QString getFullNameFromAlias(const QString &rAlias);

    void setTimeOffset(double timeOffset);
    double getTimeOffset() const;

    void switchGenerationDataCache(SharedMultiDataVectorCacheT pDataCache);

signals:
    void timeOffsetChanged();

private slots:
    void variableAutoRemovalChanged(bool allowRemoval);

private:
    typedef QMap< QString, SharedVectorVariableT > VariableMapT;

    QList<SharedVectorVariableT>  getMatchingVariables(const QRegExp &rNameExp, VariableMapT &rMap);
    bool removeVariablePrivate(const QString &rFullName);

    VariableMapT mVariables;
    VariableMapT mAliasVariables;
    int mNumKeepVariables = 0;
    double mTimeOffset = 0.0;

    QString mImportedFromFile;
};

#endif // LOGDATAGENERATION_H
