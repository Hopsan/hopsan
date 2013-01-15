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

#ifndef LOGDATAHANDLER_H
#define LOGDATAHANDLER_H

#include <QSharedPointer>
#include <QVector>
#include <QMap>
#include <QString>
#include <QColor>
#include <QObject>

// Forward Declaration
class PlotWindow;
class ContainerObject;
class LogVariableData;
class LogVariableContainer;
class LogDataHandler;

typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;
typedef QSharedPointer<LogVariableContainer> SharedLogVariableContainerPtrT;
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
    enum VarTypeT {M, I, S, ST};
    QString mModelPath;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mAliasName;
    VarTypeT mVarType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    QString getVarTypeString() const;

    bool operator==(const VariableDescription &other) const;
};
typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT;


//! @class PlotData
//! @brief Object containg all plot data and plot data function associated with a container object


class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedLogVariableDataPtrT> GenerationMapT;

    //! @todo also need qucik constructor for creating a container with one generation directly
    LogVariableContainer(const SharedVariableDescriptionT &rVarDesc, LogDataHandler *pParentLogDataHandler);
    ~LogVariableContainer();
    void addDataGeneration(const int generation, const QVector<double> &rTime, const QVector<double> &rData);
    void addDataGeneration(const int generation, const SharedTimeVectorPtrT time, const QVector<double> &rData);
    void removeDataGeneration(const int generation);
    void removeGenerationsOlderThen(const int gen);

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

    LogDataHandler *getLogDataHandler();

    void setAliasName(const QString alias);

signals:
    void nameChanged();

private:
    LogDataHandler *mpParentLogDataHandler;
    SharedVariableDescriptionT mVariableDescription;
    GenerationMapT mDataGenerations;
};

class LogVariableData : public QObject
{
    Q_OBJECT

public:
    //! @todo maybe have protected constructor, to avoid creating these objects manually (need to be friend with container)
    LogVariableData(const int generation, const QVector<double> &rTime, const QVector<double> &rData, SharedVariableDescriptionT varDesc, LogVariableContainer *pParent);
    LogVariableData(const int generation, SharedTimeVectorPtrT time, const QVector<double> &rData, SharedVariableDescriptionT varDesc, LogVariableContainer *pParent);

    double mAppliedValueOffset;
    double mAppliedTimeOffset;
    int mGeneration;
    QVector<double> mDataVector;
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

    void addToData(const SharedLogVariableDataPtrT pOther);
    void addToData(const double other);
    void subFromData(const SharedLogVariableDataPtrT pOther);
    void subFromData(const double other);
    void multData(const SharedLogVariableDataPtrT pOther);
    void multData(const double other);
    void divData(const SharedLogVariableDataPtrT pOther);
    void divData(const double other);
    void assignToData(const SharedLogVariableDataPtrT pOther);
    bool pokeData(const int index, const double value);
    double peekData(const int index);

    LogDataHandler *getLogDataHandler();

public slots:
    void setValueOffset(double offset);
    void setTimeOffset(double offset);
    //setScale(double scale);


signals:
    void dataChanged();
    void nameChanged();

private:
    LogVariableContainer *mpParentVariableContainer;
    SharedVariableDescriptionT mpVariableDescription;
};



class LogDataHandler : public QObject
{
    Q_OBJECT

public:
    LogDataHandler(ContainerObject *pParent);
    ~LogDataHandler();

    typedef QMap<QString, SharedLogVariableContainerPtrT> LogDataMapT;
    typedef QMap<QString, LogVariableContainer*> AliasMapT;
    typedef QList<QVector<double> > TimeListT;
    typedef QList<VariableDescription> FavoriteListT;

    void collectPlotDataFromModel();
    void exportToPlo(QString filePath, QStringList variables);
    void importFromPlo();

    bool isEmpty();

    QVector<double> getTimeVector(int generation);
    QVector<double> getPlotDataValues(int generation, QString componentName, QString portName, QString dataName); //!< @deprecated
    QVector<double> getPlotDataValues(const QString fullName, int generation);
    SharedLogVariableDataPtrT getPlotData(int generation, QString componentName, QString portName, QString dataName); //!< @deprecated
    SharedLogVariableDataPtrT getPlotData(const QString fullName, const int generation);
    SharedLogVariableDataPtrT getPlotDataByAlias(const QString alias, const int generation);
    QVector<SharedLogVariableDataPtrT> getAllVariablesAtNewestGeneration();
    QVector<SharedLogVariableDataPtrT> getOnlyVariablesAtGeneration(const int generation);
    int getLatestGeneration() const;
    QStringList getPlotDataNames();

    void definePlotAlias(QString fullName);
    bool definePlotAlias(const QString alias, const QString fullName);
    void undefinePlotAlias(QString alias);
    AliasMapT getPlotAliasMap();
    QString getFullNameFromAlias(QString alias);
    QString getAliasFromFullName(QString fullName);

    void limitPlotGenerations();

    ContainerObject *getParentContainerObject();

    void incrementOpenPlotCurves();
    void decrementOpenPlotCurves();
    bool hasOpenPlotCurves();
    void closePlotsWithCurvesBasedOnOwnedData();

    FavoriteListT getFavoriteVariableList();
    void setFavoriteVariable(QString componentName, QString portName, QString dataName, QString dataUnit);
    void removeFavoriteVariableByComponentName(QString componentName);

    QString plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color=QColor());
    PlotWindow *plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color=QColor());

    SharedLogVariableDataPtrT addVariableWithScalar(const SharedLogVariableDataPtrT a, const double x);
    SharedLogVariableDataPtrT subVariableWithScalar(const SharedLogVariableDataPtrT a, const double x);
    SharedLogVariableDataPtrT mulVariableWithScalar(const SharedLogVariableDataPtrT a, const double x);
    SharedLogVariableDataPtrT divVariableWithScalar(const SharedLogVariableDataPtrT a, const double x);
    QString addVariableWithScalar(const QString &a, const double x);
    QString subVariableWithScalar(const QString &a, const double x);
    QString mulVariableWithScalar(const QString &a, const double x);
    QString divVariableWithScalar(const QString &a, const double x);

    SharedLogVariableDataPtrT addVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    SharedLogVariableDataPtrT subVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    SharedLogVariableDataPtrT multVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    SharedLogVariableDataPtrT divVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    QString addVariables(const QString &a, const QString &b);
    QString subVariables(const QString &a, const QString &b);
    QString multVariables(const QString &a, const QString &b);
    QString divVariables(const QString &a, const QString &b);

    SharedLogVariableDataPtrT assignVariables(SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    QString assignVariables(const QString &a, const QString &b);

    bool pokeVariables(SharedLogVariableDataPtrT a, const int index, const double value);
    bool pokeVariables(const QString &a, const int index, const double value);

    double peekVariables(SharedLogVariableDataPtrT a, const int b);
    double peekVariables(const QString &a, const int index);

    SharedLogVariableDataPtrT saveVariables(SharedLogVariableDataPtrT a);
    QString saveVariables(const QString &currName, const QString &newName);

    void delVariables(SharedLogVariableDataPtrT a);
    QString delVariables(const QString &a);

    SharedLogVariableDataPtrT defineNewVariable(const QString desiredname);
    SharedLogVariableDataPtrT defineTempVariable(const QString desiredname);
    void removeTempVariable(const QString fullName);

signals:
    void newDataAvailable();
    void closePlotsWithOwnedData();


private:
    ContainerObject *mpParentContainerObject;

    LogDataMapT mLogDataMap;
    AliasMapT mPlotAliasMap;
    QList<SharedTimeVectorPtrT> mTimeVectorPtrs;

    FavoriteListT mFavoriteVariables;

    int mnPlotCurves;
    int mGenerationNumber;
    unsigned long int mTempVarCtr;
};

#endif // LOGDATAHANDLER_H
