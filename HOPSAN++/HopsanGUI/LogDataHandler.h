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

#include <QVector>
#include <QMap>
#include <QString>
#include <QColor>
#include <QObject>

#include "LogVariable.h"

// Forward Declaration
class PlotWindow;
class ContainerObject;

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

    SharedLogVariableDataPtrT defineNewVariable(const QString desiredname);

    bool deleteVariable(SharedLogVariableDataPtrT a);
    bool deleteVariable(const QString &a);

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

    SharedLogVariableDataPtrT assignVariable(SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b);
    QString assignVariable(const QString &a, const QString &b);

    bool pokeVariable(SharedLogVariableDataPtrT a, const int index, const double value);
    bool pokeVariable(const QString &a, const int index, const double value);

    double peekVariable(SharedLogVariableDataPtrT a, const int b);
    double peekVariable(const QString &a, const int index);

    SharedLogVariableDataPtrT saveVariable(SharedLogVariableDataPtrT a);
    QString saveVariable(const QString &currName, const QString &newName);

    SharedLogVariableDataPtrT defineTempVariable(QString desiredname);

signals:
    void newDataAvailable();
    void closePlotsWithOwnedData();


private:
    ContainerObject *mpParentContainerObject;

    LogDataMapT mLogDataMap;
    //AliasMapT mPlotAliasMap;
    QList<SharedTimeVectorPtrT> mTimeVectorPtrs;

    FavoriteListT mFavoriteVariables;

    int mnPlotCurves;
    int mGenerationNumber;
    unsigned long int mTempVarCtr;
};

#endif // LOGDATAHANDLER_H
