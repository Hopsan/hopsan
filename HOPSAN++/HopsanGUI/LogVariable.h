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
#include <QTextStream>

#include "CachableDataVector.h"
#include "common.h"
#include "UnitScale.h"

#define TIMEVARIABLENAME "Time"
#define FREQUENCYVARIABLENAME "Frequency"

// Forward declaration
class VectorVariable;
class LogDataHandler;

QString makeConcatName(const QString componentName, const QString portName, const QString dataName);
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName);

//! @brief This enum describes where a variable come from, the order signifies importance (ModelVariables most important)
enum VariableSourceTypeT {ModelVariableType, ImportedVariableType, ScriptVariableType, TempVariableType, UndefinedVariableSourceType};
QString getVariableSourceTypeAsString(const VariableSourceTypeT type);

//! @brief This enum describes the variable type
enum VariableTypeT {VectorType, TimeDomainType, FrequencyDomainType, RealFrequencyDomainType, ImaginaryFrequencyDomainType, AmplitudeFrequencyDomainType, PhaseFrequencyDomainType, ComplexType, UndefinedVariableType};

//! @class VariableCommonDescription
//! @brief Container class for strings describing a log variable (common data for all generations)
class VariableDescription
{
public:
    VariableDescription() : mVariableSourceType(UndefinedVariableSourceType) {}
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

    bool operator==(const VariableDescription &other) const;
};


typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT;
typedef QSharedPointer<VectorVariable> SharedVariablePtrT;

SharedVariableDescriptionT createTimeVariableDescription();
SharedVariableDescriptionT createFrequencyVariableDescription();
SharedVariablePtrT createFreeVectorVariable(const QVector<double> &rData, SharedVariableDescriptionT pVarDesc);
SharedVariablePtrT createFreeTimeVectorVariabel(const QVector<double> &rTime);
SharedVariablePtrT createFreeFrequencyVectorVariabel(const QVector<double> &rFrequency);

class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedVariablePtrT> GenerationMapT;

    //! @todo also need qucik constructor for creating a container with one generation directly
    LogVariableContainer(LogDataHandler *pParentLogDataHandler);
    ~LogVariableContainer();

//    SharedVariablePtrT addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData, SharedVariableDescriptionT pDescription);
//    SharedVariablePtrT addDataGeneration(const int generation, const SharedVariablePtrT time, const QVector<double> &rData, SharedVariableDescriptionT pDescription);
    void addDataGeneration(const int generation, SharedVariablePtrT pData);
    bool removeDataGeneration(const int generation, const bool force=false);
    bool purgeOldGenerations(const int purgeEnd, const int nGensToKeep);
    void removeAllGenerations();
    bool removeAllImportedGenerations();

    SharedVariablePtrT getDataGeneration(const int gen=-1) const;
    QList<SharedVariablePtrT> getAllDataGenerations() const;
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    QList<int> getGenerations() const;

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
    void logVariableBeingRemoved(SharedVariablePtrT);

private:
    LogDataHandler *mpParentLogDataHandler;
    SharedVariableDescriptionT mVariableCommonDescription;
    GenerationMapT mDataGenerations;
    QList<int> mKeepGenerations;
};


class VectorVariable : public QObject
{
    Q_OBJECT
    friend class LogVariableContainer;
    friend class LogDataHandler;

public:
    VectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                    SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
    ~VectorVariable();

    // Access variable type enums
    virtual VariableSourceTypeT getVariableSourceType() const;
    virtual VariableTypeT getVariableType() const;

    // Functions that read the data metadata
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
    virtual bool isImported() const;
    virtual QString getImportedFileName() const;

    // Data plot scaling
    void setCustomUnitScale(const UnitScale &rUnitScale);
    void removeCustomUnitScale();
    const UnitScale &getCustomUnitScale() const;
    const QString &getPlotScaleDataUnit() const;
    const QString &getActualPlotDataUnit() const;
    double getPlotScale() const;
    double getPlotOffset() const;

    // Functions that only read data
    int getDataSize() const;
    QVector<double> getDataVectorCopy() const;
    double first() const;
    double last() const;
    bool indexInRange(const int idx) const;
    double peekData(const int index, QString &rErr) const;
    double averageOfData() const;
    double minOfData(int &rIdx) const;
    double minOfData() const;
    double maxOfData(int &rIdx) const;
    double maxOfData() const;
    void elementWiseGt(QVector<double> &rResult, const double threshold) const;
    void elementWiseLt(QVector<double> &rResult, const double threshold) const;

    // Check out and return pointers to data (move to ram if necessary)
    QVector<double> *beginFullVectorOperation();
    bool endFullVectorOperation(QVector<double> *&rpData);

    // Functions that only read data but that require reimplementation in derived classes
    virtual const SharedVariablePtrT getSharedTimeVectorPointer() const;
    virtual const SharedVariablePtrT getSharedFrequencyVectorPointer() const;
    virtual SharedVariablePtrT frequencySpectrum(const SharedVariablePtrT pTime, const bool doPowerSpectrum);

    // Functions that modify the data
    void assignFrom(const QVector<double> &rSrc);
    void assignFrom(const double src);
    void addToData(const SharedVariablePtrT pOther);
    void addToData(const double other);
    void subFromData(const SharedVariablePtrT pOther);
    void subFromData(const double other);
    void multData(const SharedVariablePtrT pOther);
    void multData(const double other);
    void divData(const SharedVariablePtrT pOther);
    void divData(const double other);
    void absData();
    double pokeData(const int index, const double value, QString &rErr);
    void append(const double y);

    // Functions that modify data, but that may require reimplementation in derived classes
    virtual void assignFrom(const SharedVariablePtrT pOther);
    virtual void assignFrom(SharedVariablePtrT time, const QVector<double> &rData);
    virtual void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    virtual void append(const double t, const double y);
    virtual void diffBy(SharedVariablePtrT pOther);
    virtual void integrateBy(SharedVariablePtrT pOther);
    virtual void lowPassFilter(SharedVariablePtrT pTime, const double w);


    // Functions to toggle "keep" generation
    void preventAutoRemoval();
    void allowAutoRemoval();

    // Handle disk caching and data streaming
    void setCacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;
    void sendDataToStream(QTextStream &rStream, QString separator);

    // Access to parent object pointers
    LogVariableContainer *getLogVariableContainer();
    LogDataHandler *getLogDataHandler();

public slots:
    void setPlotScale(double scale);
    void setPlotOffset(double offset);
    void setPlotScaleAndOffset(const double scale, const double offset);

    // Slots that require reimplementation in derived classes
    virtual void setTimePlotScaleAndOffset(const double scale, const double offset);
    virtual void setTimePlotScale(double scale);
    virtual void setTimePlotOffset(double offset);

signals:
    void dataChanged();
    void nameChanged();

protected:
    typedef QVector<double> DataVectorT;
    double peekData(const int idx) const;

    CachableDataVector *mpCachedDataVector;
    QPointer<LogVariableContainer> mpParentVariableContainer;
    SharedVariableDescriptionT mpVariableDescription;

    UnitScale mCustomUnitScale;
    double mDataPlotScale;
    double mDataPlotOffset;
    int mGeneration;

private:
    QVector<double> *pTempCheckoutData;
};

class ImportedVariableBase
{
public:
    virtual bool isImported() const;
    virtual QString getImportedFileName() const;
protected:
    QString mImportFileName;

};

class ImportedVectorVariable : public VectorVariable, public ImportedVariableBase
{
    Q_OBJECT
public:
    ImportedVectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile,
                           SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
};

class TimeDomainVariable : public VectorVariable
{
    Q_OBJECT
public:
    TimeDomainVariable(SharedVariablePtrT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                       SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);

    virtual VariableTypeT getVariableType() const;

    const SharedVariablePtrT getSharedTimeVectorPointer() const;

    void diffBy(SharedVariablePtrT pOther);
    void integrateBy(SharedVariablePtrT pOther);
    void lowPassFilter(SharedVariablePtrT pTime, const double w);
    SharedVariablePtrT frequencySpectrum(const SharedVariablePtrT pTime, const bool doPowerSpectrum);
    void assignFrom(const SharedVariablePtrT pOther);
    virtual void assignFrom(SharedVariablePtrT time, const QVector<double> &rData);
    virtual void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    virtual void append(const double t, const double y);

public slots:
    void setTimePlotScaleAndOffset(const double scale, const double offset);
    void setTimePlotScale(double scale);
    void setTimePlotOffset(double offset);

protected:
    SharedVariablePtrT mpSharedTimeVector;

};

class ImportedTimeDomainVariable : public TimeDomainVariable, public ImportedVariableBase
{
    Q_OBJECT
public:
    ImportedTimeDomainVariable(SharedVariablePtrT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                               const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
};

class FrequencyDomainVariable : public VectorVariable
{
    Q_OBJECT
public:
    FrequencyDomainVariable(SharedVariablePtrT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                            SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
    //! @todo add a bunch of reimplemented functions
protected:
    SharedVariablePtrT mpSharedFrequencyVector;
};


class ComplexVectorVariable : public VectorVariable
{
    Q_OBJECT
public:
    ComplexVectorVariable(const QVector<double> &rReal, const QVector<double> &rImaginary, const int generation, SharedVariableDescriptionT varDesc,
                          SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent);
    //! @todo add a bunch of reimplemented functions
};

#endif // LOGVARIABLE_H
