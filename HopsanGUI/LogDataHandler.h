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
#include <QDir>

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

    typedef QMap< QString, SharedVectorVariableContainerT > LogDataMapT;

    void setParentContainerObject(ContainerObject *pParent);

    void collectLogDataFromModel(bool overWriteLastGeneration=false);
    void importFromPlo(QString importFilePath=QString());
    void importFromCSV_AutoFormat(QString importFilePath=QString());
    void importHopsanRowCSV(QString importFilePath=QString());
    void importFromPlainColumnCsv(QString importFilePath=QString());
    void importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> columns, QStringList names, const int timeColumnId=0);

    void exportGenerationToPlo(const QString &rFilePath, const int gen, const int version=-1) const;
    void exportToPlo(const QString &rFilePath, QList<HopsanVariable> variables, int version=-1) const;
    void exportToCSV(const QString &rFilePath, const QList<HopsanVariable> &rVariables) const;
    void exportGenerationToCSV(const QString &rFilePath, const int gen) const;

    SharedVectorVariableT defineNewVariable(const QString &rDesiredname, VariableTypeT type=VectorType);
    SharedVectorVariableT defineTempVariable(const QString &rDesiredname);
    SharedVectorVariableT createOrphanVariable(const QString &rName, VariableTypeT type=VectorType);

    bool deleteVariable(const QString &rVarName);
    bool deleteImportedVariable(const QString &rVarName);

    int getNumVariables() const;
    bool isEmpty();
    void clear();

    SharedVectorVariableContainerT getVariableContainer(const QString &rVarName) const;
    const QList<SharedVectorVariableContainerT> getAllVariableContainers() const;
    const QList<SharedVectorVariableContainerT> getVariableContainersMatching(const QRegExp &rNameExp) const;
    const QList<SharedVectorVariableContainerT> getVariableContainersMatching(const QRegExp &rNameExp, const int generation) const;

    void getVariableNamesWithHighestGeneration(QStringList &rNames, QList<int> &rGens) const;
    QStringList getVariableFullNames(const QString &rSeparator, const int generation=-1) const;
    bool hasVariable(const QString &rFullName, const int generation=-1);

    const SharedVectorVariableT getTimeVectorVariable(int generation) const;
    QVector<double> copyTimeVector(const int generation) const;

//    QVector<double> copyVariableDataVector(int generation, QString componentName, QString portName, QString dataName); //!< @deprecated
    QVector<double> copyVariableDataVector(const QString &rName, const int generation);
//    SharedVectorVariableT getVectorVariable(int generation, QString componentName, QString portName, QString dataName); //!< @deprecated
    SharedVectorVariableT getVectorVariable(const QString &rName, const int generation) const;
    HopsanVariable getHopsanVariable(const QString &rName, const int generation) const;

    QVector<SharedVectorVariableT> getMatchingVariablesAtGeneration(const QRegExp &rNameExp, const int generation=-1) const;
    QList<HopsanVariable> getAllNonAliasVariablesAtRespectiveNewestGeneration();
    QList<HopsanVariable> getAllNonAliasVariablesAtGeneration(const int generation) const;
    QList<HopsanVariable> getAllVariablesAtRespectiveNewestGeneration();
    QList<HopsanVariable> getAllVariablesAtGeneration(const int gen);

    QList<QString> getImportedVariablesFileNames() const;
    QList<HopsanVariable> getImportedVariablesForFile(const QString &rFileName);
    QList<int> getImportFileGenerations(const QString &rFilePath) const;
    QMap<QString, QList<int> > getImportFilesAndGenerations() const;
    void removeImportedFileGenerations(const QString &rFileName);

    void defineAlias(const QString &rFullName);
    bool defineAlias(const QString &rAlias, const QString &rFullName);
    void undefinePlotAlias(const QString &rAlias);
    QString getFullNameFromAlias(const QString &rAlias);

    int getNumberOfGenerations() const;
    QList<int> getGenerations() const;
    int getLowestGenerationNumber() const;
    int getHighestGenerationNumber() const;
    void getLowestAndHighestGenerationNumber(int &rLowest, int &rHighest) const;
    int getCurrentGeneration() const;

    void limitPlotGenerations();
    void preventGenerationAutoRemoval(const int gen);
    void allowGenerationAutoRemoval(const int gen);
    void removeGeneration(const int gen, const bool force);

    ContainerObject *getParentContainerObject();
    const QList<QDir> &getCacheDirs() const;
    SharedMultiDataVectorCacheT getGenerationMultiCache(const int gen);

    void incrementOpenPlotCurves();
    void decrementOpenPlotCurves();
    bool hasOpenPlotCurves();
    void closePlotsWithCurvesBasedOnOwnedData();

    PlotWindow *plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color=QColor());
    PlotWindow *plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color=QColor());
    PlotWindow *plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color=QColor());

    SharedVectorVariableT addVariableWithScalar(const SharedVectorVariableT a, const double x);
    SharedVectorVariableT subVariableWithScalar(const SharedVectorVariableT a, const double x);
    SharedVectorVariableT mulVariableWithScalar(const SharedVectorVariableT a, const double x);
    SharedVectorVariableT divVariableWithScalar(const SharedVectorVariableT a, const double x);

    SharedVectorVariableT addVariables(const SharedVectorVariableT a, const SharedVectorVariableT b);
    SharedVectorVariableT subVariables(const SharedVectorVariableT a, const SharedVectorVariableT b);
    SharedVectorVariableT multVariables(const SharedVectorVariableT a, const SharedVectorVariableT b);
    SharedVectorVariableT divVariables(const SharedVectorVariableT a, const SharedVectorVariableT b);

    SharedVectorVariableT diffVariables(const SharedVectorVariableT a, const SharedVectorVariableT b);
    SharedVectorVariableT integrateVariables(const SharedVectorVariableT a, const SharedVectorVariableT b);
    SharedVectorVariableT lowPassFilterVariable(const SharedVectorVariableT a, const SharedVectorVariableT b, const double freq);

    double pokeVariable(SharedVectorVariableT a, const int index, const double value);
    double peekVariable(SharedVectorVariableT a, const int b);

    SharedVectorVariableT elementWiseGT(SharedVectorVariableT pData, const double thresh);
    SharedVectorVariableT elementWiseLT(SharedVectorVariableT pData, const double thresh);

    SharedVectorVariableT saveVariable(SharedVectorVariableT a);

    void takeOwnershipOfData(LogDataHandler *pOtherHandler, const int otherGeneration=-2);

public slots:
    void registerAlias(const QString &rFullName, const QString &rAlias);
    void unregisterAlias(const QString &rAlias);


signals:
    void dataAdded();
    void dataRemoved();
    void aliasChanged();
    void closePlotsWithOwnedData();

private slots:
    void forgetImportedVariable(SharedVectorVariableT pData);

private:
    typedef QMap< QString, QMultiMap<QString, HopsanVariable> > ImportedLogDataMapT;
    typedef QMap<int, SharedMultiDataVectorCacheT> GenerationCacheMapT;
    SharedVectorVariableT insertTimeVectorVariable(const QVector<double> &rTimeVector);
    SharedVectorVariableT insertTimeVectorVariable(const QVector<double> &rTimeVector, const QString &rImportFileName);
    SharedVectorVariableT insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc);
    SharedVectorVariableT insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName);
    void insertVariable(SharedVectorVariableT pVariable, QString keyName=QString(), int gen=-1);

    QString getNewCacheName();
    void rememberIfImported(HopsanVariable data);
    void removeGenerationCacheIfEmpty(const int gen);
    SharedVectorVariableT defineNewVectorVariable_NoNameCheck(const QString &rName, VariableTypeT type=VectorType);

    ContainerObject *mpParentContainerObject;

    LogDataMapT mLogDataMap;
    ImportedLogDataMapT mImportedLogDataMap;
    GenerationCacheMapT mGenerationCacheMap;

    int mnPlotCurves;
    int mGenerationNumber;
    quint64 mTempVarCtr;
    QList<QDir> mCacheDirs;
    quint64 mCacheSubDirCtr;
};

#endif // LOGDATAHANDLER_H
