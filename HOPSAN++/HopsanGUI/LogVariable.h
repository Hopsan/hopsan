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

#define TIMEVARIABLENAME "Time"

// Forward declaration
class LogVariableData;
class LogDataHandler;

//class LogVariableTime : public QObject
//{
//    Q_OBJECT
//private:
//    double mScale;
//    QVector<double> mTimeData;

//public:
//    LogVariableTime();
//    LogVariableTime(const QVector<double> &rVector);
//    void setScale(const double scale);
//    inline double getScale() const { return mScale; }

//    inline const QVector<double> &data() const { return mTimeData; }
//    inline int size() const { return mTimeData.size(); }
//    inline const double &at(int idx) const { return mTimeData[idx]; }

//signals:
//    void dataChanged();
//};


//class UniqueSharedTimeVectorPtrHelper
//{
//public:
//    SharedTimeVectorPtrT makeSureUnique(const QVector<double> &rTimeVector);

//private:
//    QVector< SharedTimeVectorPtrT > mSharedTimeVecPointers;
//};

QString makeConcatName(const QString componentName, const QString portName, const QString dataName);
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName);



//! @class VariableDescription
//! @brief Container class for strings describing a plot variable
class VariableDescription
{
public:
    enum VariableSourceTypeT {ModelVariableType, ImportedVariableType, ScriptVariableType, TempVariableType};
    QString mModelPath;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mDataDescription;
    QString mAliasName;
    VariableSourceTypeT mVariableSourceType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    QString getVariableSourceTypeString() const;

    bool operator==(const VariableDescription &other) const;
};

typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT;
typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;

SharedLogVariableDataPtrT createFreeTimeVariabel(const QVector<double> &rTime);

class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedLogVariableDataPtrT> GenerationMapT;

    //! @todo also need qucik constructor for creating a container with one generation directly
    LogVariableContainer(const VariableDescription &rVarDesc, LogDataHandler *pParentLogDataHandler);
    ~LogVariableContainer();
    SharedLogVariableDataPtrT addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData);
    SharedLogVariableDataPtrT addDataGeneration(const int generation, const SharedLogVariableDataPtrT time, const QVector<double> &rData);
    void addDataGeneration(const int generation, SharedLogVariableDataPtrT pData);
    void removeDataGeneration(const int generation, const bool force=false);
    void removeGenerationsOlderThen(const int gen);
    void removeAllGenerations();

    SharedLogVariableDataPtrT getDataGeneration(const int gen=-1);
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;

    SharedVariableDescriptionT getVariableDescription() const;
    const QString &getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getSmartName() const;
    const QString &getModelPath() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;

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
    friend class LogDataHandler;

public:
    LogVariableData(const int generation, SharedLogVariableDataPtrT time, const QVector<double> &rData, SharedVariableDescriptionT varDesc,
                    SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
    ~LogVariableData();

    const SharedVariableDescriptionT getVariableDescription() const;
    const QString &getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getSmartName() const;
    const QString &getModelPath() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;
    bool hasAliasName() const;
    int getGeneration() const;
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;

    const SharedLogVariableDataPtrT getSharedTimePointer() const;
    double getPlotOffset() const;
    double getPlotScale() const;
    QVector<double> getDataVectorCopy();
    int getDataSize() const;
    double first() const;
    double last() const;

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
    void assignFrom(const double src);
    void assignFrom(SharedLogVariableDataPtrT time, const QVector<double> &rData);
    void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    double pokeData(const int index, const double value, QString &rErr);
    double peekData(const int index, QString &rErr) const;
    bool indexInRange(const int idx) const;
    double averageOfData() const;
    double minOfData() const;
    double maxOfData() const;
    double indexOfMinOfData() const;
    double indexOfMaxOfData() const;
    void append(const double t, const double y);
    void append(const double y);

    void preventAutoRemoval();
    void allowAutoRemoval();

    void setCacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;

    LogVariableContainer *getLogVariableContainer();
    LogDataHandler *getLogDataHandler();

public slots:
    void setTimePlotScaleAndOffset(const double scale, const double offset);
    void setTimePlotScale(double scale);
    void setTimePlotOffset(double offset);
    void setPlotScale(double scale);
    void setPlotOffset(double offset);
    void setPlotScaleAndOffset(const double scale, const double offset);



signals:
    void dataChanged();
    void nameChanged();

protected:
    double peekData(const int idx) const;

private:
    typedef QVector<double> DataVectorT;
    SharedLogVariableDataPtrT mSharedTimeVectorPtr;
    CachableDataVector *mpCachedDataVector;
    QPointer<LogVariableContainer> mpParentVariableContainer;
    SharedVariableDescriptionT mpVariableDescription;

    double mDataPlotScale;
    double mDataPlotOffset;
    int mGeneration;
};

#endif // LOGVARIABLE_H
