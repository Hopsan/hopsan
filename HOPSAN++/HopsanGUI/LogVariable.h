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
//! @file   LogDataHandler.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2012-12-18
//!
//! @brief Contains the LogData classes
//!
//$Id$

#ifndef LOGVARIABLE_H
#define LOGVARIABLE_H

#include <QSharedPointer>
#include <QVector>
#include <QString>
#include <QColor>
#include <QObject>
#include <QPointer>

#include "CachableDataVector.h"

// Forward declaration
class LogVariableData;
class LogDataHandler;

typedef QSharedPointer<QVector<double> > SharedTimeVectorPtrT;
class UniqueSharedTimeVectorPtrHelper
{
public:
    SharedTimeVectorPtrT makeSureUnique(QVector<double> &rTimeVector);

private:
    QVector< SharedTimeVectorPtrT > mSharedTimeVecPointers;
};

QString makeConcatName(const QString componentName, const QString portName, const QString dataName);
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName);



//! @class VariableDescription
//! @brief Container class for strings describing a plot variable
class VariableDescription
{
public:
    enum VarTypeT {ModelVariableType, ImportedVariableType, ScriptVariableType, ScriptTempVariableType};
    QString mModelPath;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mDataDescription;
    QString mAliasName;
    VarTypeT mVarType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    QString getVarTypeString() const;

    bool operator==(const VariableDescription &other) const;
};

typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT;
typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;



class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedLogVariableDataPtrT> GenerationMapT;

    //! @todo also need qucik constructor for creating a container with one generation directly
    LogVariableContainer(const VariableDescription &rVarDesc, LogDataHandler *pParentLogDataHandler);
    ~LogVariableContainer();
    void addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData);
    void addDataGeneration(const int generation, const SharedTimeVectorPtrT time, const QVector<double> &rData);
    void removeDataGeneration(const int generation);
    void removeGenerationsOlderThen(const int gen);
    void removeAllGenerations();

    SharedLogVariableDataPtrT getDataGeneration(const int gen=-1);
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;

    SharedVariableDescriptionT getVariableDescription() const;
    QString getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getModelPath() const;
    QString getComponentName() const;
    QString getPortName() const;
    QString getDataName() const;
    QString getDataUnit() const;

    void preventAutoRemove(const int gen);
    void allowAutoRemove(const int gen);

    LogDataHandler *getLogDataHandler();

    void setAliasName(const QString alias);

signals:
    void nameChanged();

private:
    LogDataHandler *mpParentLogDataHandler;
    SharedVariableDescriptionT mVariableDescription;
    GenerationMapT mDataGenerations;
    QList<int> mKeepGenerations;
};


class LogVariableData : public QObject
{
    Q_OBJECT
    friend class LogVariableContainer;

public:
    ~LogVariableData();

    SharedTimeVectorPtrT mSharedTimeVectorPtr;

    const SharedVariableDescriptionT getVariableDescription() const;
    QString getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getModelPath() const;
    QString getComponentName() const;
    QString getPortName() const;
    QString getDataName() const;
    QString getDataUnit() const;
    int getGeneration() const;
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;

    double getOffset() const;
    //double getScale() const;
    QVector<double> getDataVector();
    int getDataSize() const;

    void addToData(const SharedLogVariableDataPtrT pOther);
    void addToData(const double other);
    void subFromData(const SharedLogVariableDataPtrT pOther);
    void subFromData(const double other);
    void multData(const SharedLogVariableDataPtrT pOther);
    void multData(const double other);
    void divData(const SharedLogVariableDataPtrT pOther);
    void divData(const double other);
    void absData();
    void diffBy(const SharedLogVariableDataPtrT pOther);
    void lowPassFilter(const SharedLogVariableDataPtrT pTime, const double w);
    void frequencySpectrum(const SharedLogVariableDataPtrT pTime, const bool doPowerSpectrum);
    void assignFrom(const SharedLogVariableDataPtrT pOther);
    void assignFrom(const QVector<double> &rSrc);
    void assignFrom(SharedTimeVectorPtrT time, const QVector<double> &rData);
    void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    double pokeData(const int index, const double value, QString &rErr);
    double peekData(const int index, QString &rErr) const;
    bool indexInRange(const int idx) const;
    double averageOfData() const;
    double minOfData() const;
    double maxOfData() const;
    double indexOfMinOfData() const;
    double indexOfMaxOfData() const;

    void preventAutoRemoval();
    void allowAutoRemoval();

    void cacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;

    LogDataHandler *getLogDataHandler();

public slots:
    void setValueOffset(double offset);
    void setTimeOffset(double offset);
    //setScale(double scale);


signals:
    void dataChanged();
    void nameChanged();

protected:
    LogVariableData(const int generation, SharedTimeVectorPtrT time, const QVector<double> &rData, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
    double peekData(const int idx) const;

private:
    typedef QVector<double> DataVectorT;
    CachableDataVector *mpCachedDataVector;
    QPointer<LogVariableContainer> mpParentVariableContainer;
    SharedVariableDescriptionT mpVariableDescription;

    double mAppliedValueOffset;
    double mAppliedTimeOffset;
    int mGeneration;
};

#endif // LOGVARIABLE_H
