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

#include "Configuration.h"
#include "LogVariable.h"
#include "LogDataHandler.h"
#include "MainWindow.h"
#include "Utilities/GUIUtilities.h"

#include <limits>
#include <QMessageBox>

//! @brief Creates a free unhandled time vector logvariable, it can not have generations or be cached
SharedLogVariableDataPtrT createFreeTimeVariabel(const QVector<double> &rTime)
{
    SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription());
    pVarDesc->mDataName = TIMEVARIABLENAME;
    pVarDesc->mDataUnit = "s";
    // Since there is no parent we can nog cahe this to disk or give it a generation, it is a free floating time vector (logvariable)
    // Note! the time vaariable does not have a time vector, the time is the data in this case
    return SharedLogVariableDataPtrT(new LogVariableData(0, SharedLogVariableDataPtrT(0), rTime, pVarDesc, SharedMultiDataVectorCacheT(0), 0));
}

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

void LogVariableData::setPlotOffset(double offset)
{
    mDataPlotOffset = offset;
    emit dataChanged();
}

void LogVariableData::setPlotScaleAndOffset(const double scale, const double offset)
{
    mDataPlotScale = scale;
    mDataPlotOffset = offset;
    emit dataChanged();
}

void LogVariableData::setTimePlotOffset(double offset)
{
    if (!mSharedTimeVectorPtr.isNull())
    {
        mSharedTimeVectorPtr->setPlotOffset(offset);
    }
}

void LogVariableData::setTimePlotScale(double scale)
{
    if (!mSharedTimeVectorPtr.isNull())
    {
        mSharedTimeVectorPtr->setPlotScale(scale);
    }
}

void LogVariableData::setTimePlotScaleAndOffset(const double scale, const double offset)
{
    if (!mSharedTimeVectorPtr.isNull())
    {
        mSharedTimeVectorPtr->setPlotScaleAndOffset(scale, offset);
    }
}

void LogVariableData::setPlotScale(double scale)
{
    mDataPlotScale = scale;
    emit dataChanged();
}

LogVariableData::LogVariableData(const int generation, SharedLogVariableDataPtrT time, const QVector<double> &rData, SharedVariableDescriptionT varDesc, SharedMultiDataVectorCacheT pGenerationMultiCache, LogVariableContainer *pParent)
{
    mpParentVariableContainer = pParent;
    mpVariableDescription = varDesc;
    mDataPlotOffset = 0.0;
    mDataPlotScale = 1.0;
    mGeneration = generation;
    mSharedTimeVectorPtr = time;
    connect(mSharedTimeVectorPtr.data(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()), Qt::UniqueConnection);
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

const QString &LogVariableData::getAliasName() const
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

QString LogVariableData::getSmartName() const
{
    if (mpVariableDescription->mAliasName.isEmpty())
    {
        return mpVariableDescription->getFullName();
    }
    else
    {
        return mpVariableDescription->mAliasName;
    }
}

const QString &LogVariableData::getModelPath() const
{
    return mpVariableDescription->mModelPath;
}

const QString &LogVariableData::getComponentName() const
{
    return mpVariableDescription->mComponentName;
}

const QString &LogVariableData::getPortName() const
{
    return mpVariableDescription->mPortName;
}

const QString &LogVariableData::getDataName() const
{
    return mpVariableDescription->mDataName;
}

const QString &LogVariableData::getDataUnit() const
{
    return mpVariableDescription->mDataUnit;
}

bool LogVariableData::hasAliasName() const
{
    return !mpVariableDescription->mAliasName.isEmpty();
}

int LogVariableData::getGeneration() const
{
    return mGeneration;
}

int LogVariableData::getLowestGeneration() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getLowestGeneration();
}

int LogVariableData::getHighestGeneration() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return mGeneration;
    }
    return mpParentVariableContainer->getHighestGeneration();
}

int LogVariableData::getNumGenerations() const
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 1;
    }
    return mpParentVariableContainer->getNumGenerations();
}

const SharedLogVariableDataPtrT LogVariableData::getSharedTimePointer() const
{
    return mSharedTimeVectorPtr;
}

double LogVariableData::getPlotOffset() const
{
    return mDataPlotOffset;
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
    emit dataChanged();
}


void LogVariableData::absData()
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    for (int i=0; i<pData->size(); ++i)
    {
        (*pData)[i] = fabs((*pData)[i]);
    }
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::diffBy(const SharedLogVariableDataPtrT pOther)
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    QVector<double> diffX;
    if(pOther != 0)
    {
        diffX = pOther->getDataVectorCopy();
    }
    else
    {
        // If no diff vector supplied, use time
        if (mSharedTimeVectorPtr)
        {
            diffX = mSharedTimeVectorPtr->getDataVectorCopy();
        }
        else
        {
            //! @todo error message
            // Abort
            return;
        }
    }
    for(int i=0; i<pData->size()-1; ++i)
    {
        (*pData)[i] = ((*pData)[i+1]-(*pData)[i])/(diffX[i+1]-diffX[i]);
    }
    pData->resize(pData->size()-1);
    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::lowPassFilter(const SharedLogVariableDataPtrT pTime, const double w)
{
    DataVectorT* pData = mpCachedDataVector->beginFullVectorOperation();
    DataVectorT timeData;
    if(pTime == 0)
    {
        if (mSharedTimeVectorPtr)
        {
            timeData = mSharedTimeVectorPtr->getDataVectorCopy();
        }
        else
        {
            //! @todo error message
            // Abort
            return;
        }
    }
    else
    {
        timeData = pTime.data()->getDataVectorCopy();
    }

    double Al = 2.0/(2.0*3.14159265359*w);
    double temp1 = (*pData)[0];
    double temp = temp1;
    (*pData)[0] = temp;
    for(int i=1; i<pData->size(); ++i)
    {
        double T = timeData[i]-timeData[i-1];
        double ALF = Al/T;
        double G = 1.0+ALF;
        double A1 = (1.0-ALF)/G;
        double B1 = 1.0/G;
        temp = temp1;
        temp1 = (*pData)[i];
        (*pData)[i] = -A1*(*pData)[i-1] + B1*(temp1+temp);
    }

    mpCachedDataVector->endFullVectorOperation(pData);
    emit dataChanged();
}

void LogVariableData::frequencySpectrum(const SharedLogVariableDataPtrT pTime, const bool doPowerSpectrum)
{
    DataVectorT* pDataVec = mpCachedDataVector->beginFullVectorOperation();
    DataVectorT timeVec;
    if(pTime == 0)
    {
        if (mSharedTimeVectorPtr)
        {
            timeVec = mSharedTimeVectorPtr->getDataVectorCopy();
        }
        else
        {
            //! @todo error message
            // Abort
            return;
        }
    }
    else
    {
        timeVec = pTime.data()->getDataVectorCopy();
    }

    //Vector size has to be an even potential of 2.
    //Calculate largets potential that is smaller than or equal to the vector size.
    int n = pow(2, int(log2(pDataVec->size())));
    if(n != pDataVec->size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(pDataVec->size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindow, gpMainWindow->tr("Wrong Vector Size"),
                                 "Size of data vector must be an even power of 2. Number of log samples was reduced from " + oldString + " to " + newString + ".");
        reduceVectorSize((*pDataVec), n);
        reduceVectorSize(timeVec, n);
    }

    //Create a complex vector
    QVector< std::complex<double> > vComplex = realToComplex(*pDataVec);

    //Apply the fourier transform
    FFT(vComplex);

    //Scalar multiply complex vector with its conjugate, and divide it with its size
    pDataVec->clear();
    for(int i=1; i<n/2; ++i)        //FFT is symmetric, so only use first half
    {
        if(doPowerSpectrum)
        {
            pDataVec->append(real(vComplex[i]*conj(vComplex[i]))/n);
        }
        else
        {
            pDataVec->append(sqrt(vComplex[i].real()*vComplex[i].real() + vComplex[i].imag()*vComplex[i].imag()));
        }
    }

    //Create the x vector (frequency)
    double max = timeVec.last();
    timeVec.clear();
    for(int i=1; i<n/2; ++i)
    {
        timeVec.append(double(i)/max);
    }

    //! @todo need an easier way to create individual free data variables, hmm dont I already have one in Logdatahandler ?
    //! @todo maybe need a special function to create a free time vector log data variable
    VariableDescription varDesc;
    varDesc.mDataName = TIMEVARIABLENAME;
    varDesc.mDataUnit = "s";
    LogVariableContainer dummyTimeContainer(varDesc, 0);
    dummyTimeContainer.addDataGeneration(this->getGeneration(), SharedLogVariableDataPtrT(), timeVec);
    mSharedTimeVectorPtr = dummyTimeContainer.getDataGeneration(-1);

    mpCachedDataVector->endFullVectorOperation(pDataVec);
    emit dataChanged();
}


void LogVariableData::assignFrom(const SharedLogVariableDataPtrT pOther)
{
    mpCachedDataVector->replaceData(pOther->getDataVectorCopy());
    mSharedTimeVectorPtr = pOther->mSharedTimeVectorPtr;
    emit dataChanged();
}

void LogVariableData::assignFrom(const QVector<double> &rSrc)
{
    mpCachedDataVector->replaceData(rSrc);
    mSharedTimeVectorPtr = SharedLogVariableDataPtrT(); //No time vector assigned
    emit dataChanged();
}

void LogVariableData::assignFrom(const double src)
{
    assignFrom(QVector<double>() << src);
}

void LogVariableData::assignFrom(SharedLogVariableDataPtrT time, const QVector<double> &rData)
{
    mpCachedDataVector->replaceData(rData);
    mSharedTimeVectorPtr = time;
    emit dataChanged();
}

void LogVariableData::assignFrom(QVector<double> &rTime, QVector<double> &rData)
{
    // We create a new non managed free timevector from the supplied time data
    assignFrom(createFreeTimeVariabel(rTime), rData);
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

double LogVariableData::averageOfData() const
{
    double ret = 0;
    int i=0;
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(; i<pVector->size(); ++i)
    {
        ret += pVector->at(i);
    }
    ret /= i;
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double LogVariableData::minOfData() const
{
    double ret = std::numeric_limits<double>::max();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) < ret)
        {
            ret = pVector->at(i);
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double LogVariableData::indexOfMinOfData() const
{
    double minVal = std::numeric_limits<double>::max();
    double ret = 0;
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) < minVal)
        {
            minVal = pVector->at(i);
            ret=i;
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

double LogVariableData::indexOfMaxOfData() const
{
    double maxVal = std::numeric_limits<double>::min();
    double ret = 0;
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) > maxVal)
        {
            maxVal = pVector->at(i);
            ret=i;
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}


//! @brief Appends one point to a curve, NEVER USE THIS UNLESS A CUSTOM (PRIVATE) X (TIME) VECTOR IS USED!
void LogVariableData::append(const double t, const double y)
{
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
    mSharedTimeVectorPtr.data()->append(t);
    //! @todo FIXA, it is bad to append x-data to shared time vector, there should be a custom private xvector Peter
}

//! @brief Appends one point to a curve, NEVER USE THIS WHEN A SHARED TIMEVECTOR EXIST
void LogVariableData::append(const double y)
{
    //! @todo smarter append regardless of cached or not
    //! @todo mayebe a reserve function to reserve memory if we know how much to expect
    DataVectorT *pData = mpCachedDataVector->beginFullVectorOperation();
    pData->append(y);
    mpCachedDataVector->endFullVectorOperation(pData);
}


double LogVariableData::maxOfData() const
{
    double ret = std::numeric_limits<double>::min();
    QVector<double> *pVector = mpCachedDataVector->beginFullVectorOperation();
    for(int i=0; i<pVector->size(); ++i)
    {
        if(pVector->at(i) > ret)
        {
            ret = pVector->at(i);
        }
    }
    mpCachedDataVector->endFullVectorOperation(pVector);
    return ret;
}

void LogVariableData::preventAutoRemoval()
{
    if (!mpParentVariableContainer.isNull())
    {
        mpParentVariableContainer.data()->preventAutoRemove(mGeneration);
    }
}

void LogVariableData::allowAutoRemoval()
{
    if (!mpParentVariableContainer.isNull())
    {
        mpParentVariableContainer.data()->allowAutoRemove(mGeneration);
    }
}

void LogVariableData::setCacheDataToDisk(const bool toDisk)
{
    mpCachedDataVector->setCached(toDisk);
}

bool LogVariableData::isCachingDataToDisk() const
{
    return mpCachedDataVector->isCached();
}

bool LogVariableData::indexInRange(const int idx) const
{
    //! @todo Do we need to check timevector also ? (or should we assume thay are the same)
    return (idx>=0 && idx<mpCachedDataVector->size());
}

LogDataHandler *LogVariableData::getLogDataHandler()
{
    // Using QPointer to avoid crash if container removed before data
    if (mpParentVariableContainer.isNull())
    {
        return 0;
    }
    return mpParentVariableContainer->getLogDataHandler();
}


const QString &LogVariableContainer::getAliasName() const
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

QString LogVariableContainer::getSmartName() const
{
    if (mVariableDescription->mAliasName.isEmpty())
    {
        return mVariableDescription->getFullName();
    }
    else
    {
        return mVariableDescription->mAliasName;
    }
}

const QString &LogVariableContainer::getModelPath() const
{
    return mVariableDescription->mModelPath;
}

const QString &LogVariableContainer::getComponentName() const
{
    return mVariableDescription->mComponentName;
}

const QString &LogVariableContainer::getPortName() const
{
    return mVariableDescription->mPortName;
}

const QString &LogVariableContainer::getDataName() const
{
    return mVariableDescription->mDataName;
}

const QString &LogVariableContainer::getDataUnit() const
{
    return mVariableDescription->mDataUnit;
}

void LogVariableContainer::preventAutoRemove(const int gen)
{
    //! @todo what happens if we tell it to keep a generation it does not have
    //! @todo maybe special -1 input for ALL
    if (!mKeepGenerations.contains(gen))
    {
        mKeepGenerations.prepend(gen);
    }
}

void LogVariableContainer::allowAutoRemove(const int gen)
{
    //! @todo maybe special -1 input for ALL
    mKeepGenerations.removeOne(gen);
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

QString VariableDescription::getVariableSourceTypeString() const
{
    switch (mVariableSourceType)
    {
    case ScriptVariableType :
        return "ScriptVariableType";
        break;
    case TempVariableType :
        return "TempVariableType";
        break;
    case ModelVariableType :
        return "ModelVariableType";
        break;
    case ImportedVariableType :
        return "ImportedVariableType";
        break;
    }
    return "UndefinedVariableSourceType";
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

SharedLogVariableDataPtrT LogVariableContainer::addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData)
{
    return addDataGeneration(generation, createFreeTimeVariabel(rTime), rData);
}

SharedLogVariableDataPtrT LogVariableContainer::addDataGeneration(const int generation, const SharedLogVariableDataPtrT time, const QVector<double> &rData)
{
    SharedLogVariableDataPtrT pData;
    if(mDataGenerations.contains(generation))
    {
        pData = mDataGenerations.find(generation).value();
        pData->assignFrom(time, rData);
    }
    else
    {

        if (mpParentLogDataHandler)
        {
            pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableDescription, mpParentLogDataHandler->getOrCreateGenerationMultiCache(generation), this));
        }
        else
        {
            pData = SharedLogVariableDataPtrT(new LogVariableData(generation, time, rData, mVariableDescription, SharedMultiDataVectorCacheT(), this));
        }

        connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
        mDataGenerations.insert(generation, pData);
    }
    return pData;
}

//! @note Make sure that pData is not some other variable, that will screw things up badly
void LogVariableContainer::addDataGeneration(const int generation, SharedLogVariableDataPtrT pData)
{
    if(mDataGenerations.contains(generation))
    {
        mDataGenerations.find(generation).value().data()->assignFrom(pData);
    }
    else
    {
        // Set some data that was set by LogvariableData constructor when creating a new variable, in this case we need to overwrite
        pData->mpParentVariableContainer = this;
        pData->mGeneration = generation;
        // Connect the namechanged signal
        connect(this, SIGNAL(nameChanged()), pData.data(), SIGNAL(nameChanged()));
        // Insert into generation storage
        mDataGenerations.insert(generation, pData);
    }
}

void LogVariableContainer::removeDataGeneration(const int generation, const bool force)
{
    // Skip removal of generations that should be kept
    if (mKeepGenerations.contains(generation))
    {
        if (force)
        {
            mDataGenerations.remove(generation);
            mKeepGenerations.removeOne(generation);
        }
    }
    else
    {
        //! @todo cache data will still be in the cachegenreationmap, need to clear whenevevr generation is removed (from anywere), mabe should restore inc dec Subscribers
        mDataGenerations.remove(generation);
    }

    // If last data generation removed tehn ask my parent to delete me
    if (mDataGenerations.isEmpty())
    {
        mpParentLogDataHandler->deleteVariable(this->getFullVariableName());
    }
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
    // It is assumed that the generation map is sorted by key which it should be since adding will allways append
    QList<int> gens = mDataGenerations.keys();
    for (int it=0; it<gens.size(); ++it)
    {
        removeDataGeneration(gens[it]);
    }

    mpParentLogDataHandler->deleteVariable(this->getFullVariableName());
}

LogVariableContainer::LogVariableContainer(const VariableDescription &rVarDesc, LogDataHandler *pParentLogDataHandler) : QObject()
{
    mVariableDescription = SharedVariableDescriptionT(new VariableDescription(rVarDesc)); //Copy original data and create a new shared variable description
    mpParentLogDataHandler = pParentLogDataHandler;
}

LogVariableContainer::~LogVariableContainer()
{
    // Clear all data
    mDataGenerations.clear();
}

//SharedTimeVectorPtrT UniqueSharedTimeVectorPtrHelper::makeSureUnique(const QVector<double> &rTimeVector)
//{
//    const int nElements = rTimeVector.size();
//    if (nElements > 0)
//    {
//        const double newFirst = rTimeVector[0];
//        const double newLast = rTimeVector[nElements-1];

//        for (int idx=0; idx<mSharedTimeVecPointers.size(); ++idx)
//        {
//            const int oldElements = mSharedTimeVecPointers[idx]->size();
//            const double oldFirst = mSharedTimeVecPointers[idx]->at(0);
//            const double oldLast = mSharedTimeVecPointers[idx]->at(oldElements-1);
//            if ( (oldElements == nElements) &&
//                 fuzzyEqual(newFirst, oldFirst, 1e-10) &&
//                 fuzzyEqual(newLast, oldLast, 1e-10) )
//            {
//                return mSharedTimeVecPointers[idx];
//            }
//        }

//        // If we did not already return then add this pointer
//        mSharedTimeVecPointers.append(SharedTimeVectorPtrT(new LogVariableTime(rTimeVector)));
//        return mSharedTimeVecPointers.last();
//    }
//    return SharedTimeVectorPtrT();
//}


QVector<double> LogVariableData::getDataVectorCopy()
{
    QVector<double> vec;
    mpCachedDataVector->copyDataTo(vec);
    return vec;
}

int LogVariableData::getDataSize() const
{
    return mpCachedDataVector->size();
}

double LogVariableData::first() const
{
    return peekData(0);
}

double LogVariableData::last() const
{
    return peekData(mpCachedDataVector->size()-1);
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


//LogVariableTime::LogVariableTime(const QVector<double> &rVector)
//{
//    mScale = 1.0;
//    mTimeData = rVector;
//    emit dataChanged();
//}

//void LogVariableTime::setScale(const double scale)
//{
//    mScale *= scale;
//    emit dataChanged();
//}

//LogVariableTime::LogVariableTime()
//{
//    mScale = 1.0;
//}


double LogVariableData::getPlotScale() const
{
    return mDataPlotScale;
}
