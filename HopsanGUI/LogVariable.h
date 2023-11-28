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
class LogDataHandler2;

QString makeFullVariableName(const QStringList &rSystemHierarchy, const QString &rComponentName, const QString &rPortName, const QString &rDataName);
QString makeFullVariableNameRegexpSafe(const QStringList &rSystemHierarchy, const QString &rComponentName, const QString &rPortName, const QString &rDataName);
QString makeFullParameterName(const QStringList &rSystemHierarchy, const QString &rCompName, const QString &rParamName);
bool splitFullVariableName(const QString &rFullName, QStringList &rSystemHierarchy, QString &rCompName, QString &rPortName, QString &rVarName);
bool splitFullComponentName(const QString &rFullName, QStringList &rSystemHierarchy, QString &rCompName);
bool splitFullParameterName(const QString &rFullName, QStringList &rSystemHierarchy, QString &rCompName, QString &rParamName);

//! @brief Enum describing the type of variable names available
enum VariableNameTypeT {FullName, Alias, AliasAndFull};

//! @brief This enum describes where a variable come from, the order signifies importance (ModelVariables most important)
enum VariableSourceTypeT {ModelVariableType, ImportedVariableType, ScriptVariableType, UndefinedVariableSourceType};
QString variableSourceTypeAsString(const VariableSourceTypeT type);
QString variableSourceTypeAsShortString(const VariableSourceTypeT type);

//! @brief This enum describes the variable type
enum VariableTypeT {VectorType, TimeDomainType, FrequencyDomainType, RealFrequencyDomainType, ImaginaryFrequencyDomainType, AmplitudeFrequencyDomainType, PhaseFrequencyDomainType, ComplexType, UndefinedVariableType};
QString variableTypeAsString(const VariableTypeT type);

enum SteadyStateIdentificationMethodEnumT {RectangularWindowTest, VarianceRatioTest, MovingAverageVarianceRatioTest};

typedef QSharedPointer<QStringList> SharedSystemHierarchyT;

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

    SharedSystemHierarchyT mpSystemHierarchy;

    QString mDataUnit;
    QString mDataQuantity;
    QString mDataDescription;
    QString mCustomLabel;

    QString mModelPath;
    VariableSourceTypeT mVariableSourceType;

    bool mModelInvertPlot=false;
    bool mLocalInvertInvertPlot=false;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);
};


typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT; //!< @todo I dont think this one need/should be shared anymore /Peter
typedef QSharedPointer<VectorVariable> SharedVectorVariableT;


SharedVariableDescriptionT createTimeVariableDescription();
SharedVariableDescriptionT createFrequencyVariableDescription();
SharedVectorVariableT createFreeVectorVariable(const QVector<double> &rData, SharedVariableDescriptionT pVarDesc);
SharedVectorVariableT createFreeTimeVectorVariabel(const QVector<double> &rTime);
SharedVectorVariableT createFreeFrequencyVectorVariabel(const QVector<double> &rFrequency);
SharedVectorVariableT createFreeVariable(VariableTypeT type, SharedVariableDescriptionT pVarDesc);

class VectorVariable : public QObject
{
    Q_OBJECT
    friend class LogDataHandler2;
    friend class LogDataGeneration;

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
    QString getSmartName(const QString sep="") const;
    const SharedSystemHierarchyT getSystemHierarchy() const;
    const QString &getModelPath() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    virtual const QString &getDataName() const;
    const QString &getDataUnit() const;
    const QString &getDataQuantity() const;
    const QString &getCustomLabel() const;
    UnitConverter getUnitScale() const;
    bool hasAliasName() const;
    bool hasCustomLabel() const;
    void setCustomLabel(const QString &label);
    int getGeneration() const;
    virtual bool isImported() const;
    virtual QString getImportedFileName() const;

    // Data plot scaling
    bool isPlotInverted() const;
    void setPlotInverted(bool tf);
    void togglePlotInverted();
    double getGenerationPlotOffsetIfTime() const;

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
    double rmsOfData() const;
    void elementWiseGt(QVector<double> &rResult, const double threshold) const;
    void elementWiseGt(QVector<double> &rResult, const SharedVectorVariableT pOther) const;
    void elementWiseLt(QVector<double> &rResult, const double threshold) const;
    void elementWiseLt(QVector<double> &rResult, const SharedVectorVariableT pOther) const;
    void elementWiseEq(QVector<double> &rResult, const double value, const double eps) const;
    void elementWiseEq(QVector<double> &rResult, const SharedVectorVariableT pOther, const double eps) const;
    bool compare(SharedVectorVariableT pOther, const double eps) const;
    int lower_bound(const double value, const bool assumeSorted=false) const;

    // Check out and return pointers to data (move to ram if necessary)
    QVector<double> *beginFullVectorOperation();
    bool endFullVectorOperation(QVector<double> *&rpData);

    // Functions that only read data but that require reimplementation in derived classes
    virtual const SharedVectorVariableT getSharedTimeOrFrequencyVector() const;
    virtual SharedVectorVariableT toFrequencySpectrum(const SharedVectorVariableT pTime, const FrequencySpectrumEnumT type, const WindowingFunctionEnumT windowingFunction=RectangularWindow, double minTime=-std::numeric_limits<double>::max(), double maxTime=std::numeric_limits<double>::max());
    virtual SharedVectorVariableT identifySteadyState(const SteadyStateIdentificationMethodEnumT method, double tol, double win=0, double stdev=0, double l1=0, double l2=0, double l3=0);

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
    void powerData(const double other);
    double pokeData(const int index, const double value, QString &rErr);
    void append(const double y);
    void chopAtBeginning();

    // Functions that modify data, but that may require reimplementation in derived classes
    virtual void assignFrom(const SharedVectorVariableT pOther);
    virtual void assignFrom(SharedVectorVariableT time, const QVector<double> &rData);
    virtual void assignFrom(const QVector<double> &rTime, const QVector<double> &rData);
    virtual void append(const double t, const double y);
    virtual void diffBy(SharedVectorVariableT pOther);
    virtual void integrateBy(SharedVectorVariableT pOther);
    virtual void lowPassFilter(SharedVectorVariableT pTime, const double w);

    // Functions to toggle "keep" data
    bool isAutoremovalAllowed() const;
    void preventAutoRemoval();
    void allowAutoRemoval();

    // Handle disk caching and data streaming
    void setCacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;
    void sendDataToStream(QTextStream &rStream, QString separator);

    // Access to parent object pointers
    QPointer<LogDataHandler2> getLogDataHandler();
    const QPointer<LogDataHandler2> getLogDataHandler() const;

    template<typename Function>
    QVector<double> invokeMathFunctionOnData(Function func)
    {
        QVector<double> *pData = mpCachedDataVector->beginFullVectorOperation();
        QVector<double> retdata(pData->size());
        for (int i=0; i<pData->size(); ++i)
        {
            retdata[i] = func((*pData)[i]);
        }
        mpCachedDataVector->endFullVectorOperation(pData);
        return retdata;
    }

signals:
    void beingRemoved();
    void dataChanged();
    void nameChanged();
    void quantityChanged();
    void allowAutoRemovalChanged(bool);

protected:
    void replaceSharedTFVector(SharedVectorVariableT pToFVector);
    typedef QVector<double> DataVectorT;

    QPointer<LogDataHandler2> mpParentLogDataHandler;

    CachableDataVector *mpCachedDataVector;
    SharedVariableDescriptionT mpVariableDescription;
    SharedVectorVariableT mpSharedTimeOrFrequencyVector;

    int mGeneration;
    bool mAllowAutoRemove = true;
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
    SharedVectorVariableT toFrequencySpectrum(const SharedVectorVariableT pTime, const FrequencySpectrumEnumT type, const WindowingFunctionEnumT windowingFunction, double minTime, double maxTime);
    void assignFrom(const SharedVectorVariableT pOther);
    virtual void assignFrom(SharedVectorVariableT time, const QVector<double> &rData);
    virtual void assignFrom(const QVector<double> &rTime, const QVector<double> &rData);
    virtual void append(const double t, const double y);
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
    virtual VariableTypeT getVariableType() const;

    virtual void assignFrom(const SharedVectorVariableT pOther);
    virtual void assignFrom(SharedVectorVariableT freq, const QVector<double> &rData);
    virtual void assignFrom(const QVector<double> &rFreq, const QVector<double> &rData);
    //! @todo add a bunch of reimplemented functions
};

class ImportedFrequencyDomainVariable : public FrequencyDomainVariable
{
    Q_OBJECT
public:
    ImportedFrequencyDomainVariable(SharedVectorVariableT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                                    const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache);
    bool isImported() const;
    QString getImportedFileName() const;

private:
    QString mImportFileName;
};

//! @todo complex variables is a bit strange right now, it abuses overloading of function from vector variable. Reason is that real xy plots are not supported /Peter
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


// Convenient functions
void createBodeVariables(const SharedVectorVariableT pInput, const SharedVectorVariableT pOutput, int Fmax,
                         SharedVectorVariableT &rNyquistData, SharedVectorVariableT &rNyquistDataInv,
                         SharedVectorVariableT &rGainData, SharedVectorVariableT &rPhaseData, WindowingFunctionEnumT windowFunction=RectangularWindow, double minTime=-std::numeric_limits<double>::max(), double maxTime=std::numeric_limits<double>::max());

SharedVectorVariableT switchVariableGeneration(SharedVectorVariableT pVar, int generation);

double pokeVariable(SharedVectorVariableT a, const int index, const double value);
double peekVariable(SharedVectorVariableT a, const int b);

#endif // LOGVARIABLE_H
