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
QString variableSourceTypeAsString(const VariableSourceTypeT type);
QString variableSourceTypeAsShortString(const VariableSourceTypeT type);

//! @brief This enum describes the variable type
enum VariableTypeT {VectorType, TimeDomainType, FrequencyDomainType, RealFrequencyDomainType, ImaginaryFrequencyDomainType, AmplitudeFrequencyDomainType, PhaseFrequencyDomainType, ComplexType, UndefinedVariableType};
QString variableTypeAsString(const VariableTypeT type);

//! @class VariableCommonDescription
//! @brief Container class for strings describing a log variable
class VariableDescription
{
public:
    VariableDescription() : mVariableSourceType(UndefinedVariableSourceType) {}
    QString mAliasName;
    QString mComponentName;
    QString mPortName;
    QString mDataName;

    QString mDataUnit;
    QString mDataDescription;
    QString mCustomLabel;

    QString mModelPath;
    VariableSourceTypeT mVariableSourceType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);
};


typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT;
typedef QSharedPointer<VectorVariable> SharedVectorVariableT;

SharedVariableDescriptionT createTimeVariableDescription();
SharedVariableDescriptionT createFrequencyVariableDescription();
SharedVectorVariableT createFreeVectorVariable(const QVector<double> &rData, SharedVariableDescriptionT pVarDesc);
SharedVectorVariableT createFreeTimeVectorVariabel(const QVector<double> &rTime);
SharedVectorVariableT createFreeFrequencyVectorVariabel(const QVector<double> &rFrequency);
SharedVectorVariableT createFreeVariable(VariableTypeT type, SharedVariableDescriptionT pVarDesc);

class IndexIntervalCollection
{
public:
    class MinMaxT
    {
    public:
        MinMaxT(int min, int max);
        int mMin, mMax;
    };

    void addValue(const int val);
    void removeValue(const int val);
    int min() const;
    int max() const;
    bool isContinuos() const;
    bool isEmpty() const;
    bool contains(const int val) const;
    void clear();
    int getNumAddedValues() const;

    QList<MinMaxT> getList() const;
    QList<int> getCompleteList() const;

private:
    QList<MinMaxT> mIntervalList;
};

class VectorVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedVectorVariableT> GenerationMapT;

    VectorVariableContainer(const QString &rName, LogDataHandler *pParentLogDataHandler);
    ~VectorVariableContainer();

    const QString &getName() const;

    void addDataGeneration(const int generation, SharedVectorVariableT pData);
    bool removeDataGeneration(const int generation, const bool force=false);
    void removeAllGenerations();
    bool removeAllImportedGenerations();
    bool purgeOldGenerations(const int purgeEnd, const int nGensToKeep);

    SharedVectorVariableT getDataGeneration(const int gen=-1) const;
    SharedVectorVariableT getNonAliasDataGeneration(int gen=-1) const;
    QList<SharedVectorVariableT> getAllDataGenerations() const;
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    QList<int> getGenerations() const;

    bool isStoringAlias() const;
    bool isGenerationAlias(const int gen) const;
    bool isStoringImported() const;
    bool isGenerationImported(const int gen) const;

    LogDataHandler *getLogDataHandler();

public slots:
    void allowGenerationAutoRemoval(int gen, bool allow);

signals:
    void importedVariableBeingRemoved(SharedVectorVariableT);
    void generationAdded();

private:
    void actuallyRemoveDataGen(GenerationMapT::iterator git);
    QString mName;
    LogDataHandler *mpParentLogDataHandler;
    GenerationMapT mDataGenerations;
    IndexIntervalCollection mAliasGenIndexes;
    IndexIntervalCollection mImportedGenIndexes;
    QList<int> mKeepGenerations;
};

typedef QSharedPointer<VectorVariableContainer> SharedVectorVariableContainerT;


class VectorVariable : public QObject
{
    Q_OBJECT
    friend class VectorVariableContainer;
    friend class LogDataHandler;

public:
    VectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                   SharedMultiDataVectorCacheT pGenerationMultiCache);
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
    virtual const QString &getDataName() const;
    const QString &getDataUnit() const;
    const QString &getCustomLabel() const;
    bool hasAliasName() const;
    bool hasCustomLabel() const;
    int getGeneration() const;
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
    double peekData(const int idx) const;
    double averageOfData() const;
    double minOfData(int &rIdx) const;
    double minOfData() const;
    double maxOfData(int &rIdx) const;
    double maxOfData() const;
    void minMaxOfData(double &rMin, double &rMax, int &rMinIdx, int &rMaxIdx) const;
    bool positiveNonZeroMinMaxOfData(double &rMin, double &rMax, int &rMinIdx, int &rMaxIdx) const;
    void elementWiseGt(QVector<double> &rResult, const double threshold) const;
    void elementWiseLt(QVector<double> &rResult, const double threshold) const;
    bool compare(SharedVectorVariableT pOther, const double eps) const;

    // Check out and return pointers to data (move to ram if necessary)
    QVector<double> *beginFullVectorOperation();
    bool endFullVectorOperation(QVector<double> *&rpData);

    // Functions that only read data but that require reimplementation in derived classes
    virtual const SharedVectorVariableT getSharedTimeOrFrequencyVector() const;
    virtual SharedVectorVariableT toFrequencySpectrum(const SharedVectorVariableT pTime, const bool doPowerSpectrum);

    // Functions that modify the data
    void assignFrom(const QVector<double> &rSrc);
    void assignFrom(const double src);
    void addToData(const SharedVectorVariableT pOther);
    void addToData(const double other);
    void subFromData(const SharedVectorVariableT pOther);
    void subFromData(const double other);
    void multData(const SharedVectorVariableT pOther);
    void multData(const double other);
    void divData(const SharedVectorVariableT pOther);
    void divData(const double other);
    void absData();
    double pokeData(const int index, const double value, QString &rErr);
    void append(const double y);

    // Functions that modify data, but that may require reimplementation in derived classes
    virtual void assignFrom(const SharedVectorVariableT pOther);
    virtual void assignFrom(SharedVectorVariableT time, const QVector<double> &rData);
    virtual void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    virtual void append(const double t, const double y);
    virtual void diffBy(SharedVectorVariableT pOther);
    virtual void integrateBy(SharedVectorVariableT pOther);
    virtual void lowPassFilter(SharedVectorVariableT pTime, const double w);

    // Functions to toggle "keep" generation
    void preventAutoRemoval();
    void allowAutoRemoval();

    // Handle disk caching and data streaming
    void setCacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;
    void sendDataToStream(QTextStream &rStream, QString separator);

    // Access to parent object pointers
    QPointer<LogDataHandler> getLogDataHandler();

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
    void allowAutoRemove(int gen, bool allow);

protected:
    typedef QVector<double> DataVectorT;

    QPointer<LogDataHandler> mpParentLogDataHandler;

    CachableDataVector *mpCachedDataVector;
    SharedVariableDescriptionT mpVariableDescription;
    SharedVectorVariableT mpSharedTimeOrFrequencyVector;

    UnitScale mCustomUnitScale;
    double mDataPlotScale;
    double mDataPlotOffset;
    int mGeneration;
};

class ImportedVectorVariable : public VectorVariable
{
    Q_OBJECT
public:
    ImportedVectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile,
                           SharedMultiDataVectorCacheT pGenerationMultiCache);
    bool isImported() const;
    QString getImportedFileName() const;
private:
    QString mImportFileName;
};

class TimeDomainVariable : public VectorVariable
{
    Q_OBJECT
public:
    TimeDomainVariable(SharedVectorVariableT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                       SharedMultiDataVectorCacheT pGenerationMultiCache);

    virtual VariableTypeT getVariableType() const;

    void diffBy(SharedVectorVariableT pOther);
    void integrateBy(SharedVectorVariableT pOther);
    void lowPassFilter(SharedVectorVariableT pTime, const double w);
    SharedVectorVariableT toFrequencySpectrum(const SharedVectorVariableT pTime, const bool doPowerSpectrum);
    void assignFrom(const SharedVectorVariableT pOther);
    virtual void assignFrom(SharedVectorVariableT time, const QVector<double> &rData);
    virtual void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    virtual void append(const double t, const double y);

public slots:
    void setTimePlotScaleAndOffset(const double scale, const double offset);
    void setTimePlotScale(double scale);
    void setTimePlotOffset(double offset);
};

class ImportedTimeDomainVariable : public TimeDomainVariable
{
    Q_OBJECT
public:
    ImportedTimeDomainVariable(SharedVectorVariableT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                               const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache);
    bool isImported() const;
    QString getImportedFileName() const;

private:
    QString mImportFileName;
};

class FrequencyDomainVariable : public VectorVariable
{
    Q_OBJECT
public:
    FrequencyDomainVariable(SharedVectorVariableT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                            SharedMultiDataVectorCacheT pGenerationMultiCache);
    //! @todo add a bunch of reimplemented functions
};

//! @todo complex varibales is a bit strange right now, it abuses overloading of function from vector variable. Reason is that real xy plots are not supported /Peter
class ComplexVectorVariable : public VectorVariable
{
    Q_OBJECT
public:
    ComplexVectorVariable(const QVector<double> &rReal, const QVector<double> &rImaginary, const int generation, SharedVariableDescriptionT varDesc,
                          SharedMultiDataVectorCacheT pGenerationMultiCache);
    ComplexVectorVariable(SharedVectorVariableT pReal, SharedVectorVariableT pImaginary, const int generation, SharedVariableDescriptionT varDesc);
    virtual VariableTypeT getVariableType() const;

    const QString &getDataName() const;
    const SharedVectorVariableT getSharedTimeOrFrequencyVector() const;

    QVector<double> getRealDataCopy() const;
    QVector<double> getImagDataCopy() const;
    //! @todo add a bunch of reimplemented functions
protected:
    SharedVectorVariableT mpSharedReal, mpSharedImag;
};

void createBodeVariables(const SharedVectorVariableT pInput, const SharedVectorVariableT pOutput, int Fmax,
                         SharedVectorVariableT &rNyquistData, SharedVectorVariableT &rNyquistDataInv,
                         SharedVectorVariableT &rGainData, SharedVectorVariableT &rPhaseData);


class HopsanVariable
{
public:
    HopsanVariable();
    HopsanVariable(SharedVectorVariableT pData);
    HopsanVariable(SharedVectorVariableContainerT pContainer, SharedVectorVariableT pData);

    LogDataHandler *getLogDataHandler();

    bool hasContainer() const;
    bool isNull() const;
    operator bool() const;
    bool operator!() const;

    bool isVariableAlias() const;

    SharedVectorVariableContainerT mpContainer;
    SharedVectorVariableT mpVariable;
};


#endif // LOGVARIABLE_H
