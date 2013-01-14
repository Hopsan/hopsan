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
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mAliasName;
    VarTypeT mVarType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    QString getTypeVarString() const;

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
    LogVariableContainer(const SharedVariableDescriptionT &rVarDesc);
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
    QString getComponentName() const;
    QString getPortName() const;
    QString getDataName() const;
    QString getDataUnit() const;


    void setAliasName(const QString alias);

signals:
    void nameChanged();

private:
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
    //~LogVariableData();

    double mAppliedValueOffset;
    double mAppliedTimeOffset;
    int mGeneration;
    QVector<double> mDataVector;
    SharedTimeVectorPtrT mSharedTimeVectorPtr;

    const SharedVariableDescriptionT getVariableDescription() const;
    QString getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getComponentName() const;
    QString getPortName() const;
    QString getDataName() const;
    QString getDataUnit() const;
    int getGeneration() const;
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    double getOffset() const;
    void addtoData(const SharedLogVariableDataPtrT pOther);
    void addtoData(const double other);
    void subtoData(const SharedLogVariableDataPtrT pOther);
    void subtoData(const double other);
    void multtoData(const SharedLogVariableDataPtrT pOther);
    void multtoData(const double other);
    void divtoData(const SharedLogVariableDataPtrT pOther);
    void divtoData(const double other);
    void assigntoData(const SharedLogVariableDataPtrT pOther);
    bool poketoData(const int index, const double value);
    double peekFromData(const int index);
    //double getScale() const;


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
    QVector<double> getPlotDataValues(int generation, QString componentName, QString portName, QString dataName);
    QVector<double> getPlotDataValues(const QString fullName, int generation);
    SharedLogVariableDataPtrT getPlotData(int generation, QString componentName, QString portName, QString dataName);
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

//    bool componentHasPlotGeneration(int generation, QString fullName);
    void limitPlotGenerations();

//    void updateObjectName(QString oldName, QString newName);


    void incrementOpenPlotCurves();
    void decrementOpenPlotCurves();
    bool hasOpenPlotCurves();

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
    SharedLogVariableDataPtrT assignVariables(SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    bool pokeVariables(SharedLogVariableDataPtrT a, const int index, const double value);
    SharedLogVariableDataPtrT saveVariables(SharedLogVariableDataPtrT a);
    void delVariables(SharedLogVariableDataPtrT a);
    QString addVariables(const QString &a, const QString &b);
    QString subVariables(const QString &a, const QString &b);
    QString multVariables(const QString &a, const QString &b);
    QString divVariables(const QString &a, const QString &b);
    QString assignVariables(const QString &a, const QString &b);
    bool pokeVariables(const QString &a, const int index, const double value);
    double peekVariables(const QString &a, const int index);
    double peekVariables(SharedLogVariableDataPtrT a, const int b);
    QString delVariables(const QString &a);
    QString saveVariables(const QString &currName, const QString &newName);
    SharedLogVariableDataPtrT defineTempVariable(const QString desiredname);
    SharedLogVariableDataPtrT defineNewVariable(const QString desiredname);
    void removeTempVariable(const QString fullName);

signals:
    void newDataAvailable();


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
