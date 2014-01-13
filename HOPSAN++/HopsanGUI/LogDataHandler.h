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

class LogDataStructT
{
public:
    LogDataStructT() : mpDataContainer(0), mIsAlias(false) {}
    LogDataStructT(LogVariableContainer* pDataContainer, bool isAlias);
    QPointer<LogVariableContainer> mpDataContainer;
    bool mIsAlias;
};

class LogDataHandler : public QObject
{
    Q_OBJECT

public:
    LogDataHandler(ContainerObject *pParent);
    ~LogDataHandler();

    typedef QMap< QString, LogDataStructT > LogDataMapT;
    typedef QList<QVector<double> > TimeListT;
    typedef QList<VariableDescription> FavoriteListT;

    void setParentContainerObject(ContainerObject *pParent);

    void collectLogDataFromModel(bool overWriteLastGeneration=false);
    void importFromPlo(QString importFilePath=QString());
    void importFromCSV_AutoFormat(QString importFilePath=QString());
    void importHopsanRowCSV(QString importFilePath=QString());
    void importFromPlainColumnCsv(QString importFilePath=QString());
    void importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> columns, QStringList names, const int timeColumnId=0);

    void exportGenerationToPlo(const QString &rFilePath, const int gen, const int version=-1) const;
    void exportToPlo(const QString &rFilePath, const QStringList &rVariables, const int version=-1) const;
    void exportToPlo(const QString &rFilePath, const QVector<SharedVariablePtrT> &rVariables, int version=-1) const;
    void exportToCSV(const QString &rFilePath, const QVector<SharedVariablePtrT> &rVariables) const;
    void exportGenerationToCSV(const QString &rFilePath, const int gen) const;

    SharedVariablePtrT defineNewVariable(const QString &rDesiredname);
    SharedVariablePtrT defineNewVariable(const QString &rDesiredname, const QString &rUnit, const QString &rDescription);
    //SharedLogVariableDataPtrT defineNewVariable(const QString desiredname, QVector<double> x, QVector<double> y);
    SharedVariablePtrT defineTempVariable(const QString &rDesiredname);
    SharedVariablePtrT createOrphanVariable(const QString &rName, VariableTypeT type=VectorType);

    bool deleteVariable(SharedVariablePtrT a);
    bool deleteVariable(const QString &a);
    bool deleteImportedVariable(const QString &rVarName);

    int getNumVariables() const;
    bool isEmpty();
    void clear();

    LogVariableContainer* getLogVariableContainer(const QString &rFullName) const;

    const LogDataStructT getCompleteLogVariableData(const QString &rName) const;
    const QList<LogDataStructT> getAllCompleteLogVariableData() const;
    const QList<LogDataStructT> getMultipleCompleteLogVariableData(const QRegExp &rNameExp) const;
    const QList<LogDataStructT> getMultipleCompleteLogVariableData(const QRegExp &rNameExp, const int generation) const;

    QStringList getLogDataVariableNames(const QString &rSeparator, const int generation=-1) const;
    void getLogDataVariableNamesWithHighestGeneration(const QString &rSeparator, QStringList &rNames, QList<int> &rGens) const;
    QStringList getLogDataVariableFullNames(const QString &rSeparator, const int generation=-1) const;
    bool hasLogVariableData(const QString &rFullName, const int generation=-1);

    const SharedVariablePtrT getTimeVectorPtr(int generation) const;
    QVector<double> copyTimeVector(const int generation) const;

    QVector<double> copyLogDataVariableValues(int generation, QString componentName, QString portName, QString dataName); //!< @deprecated
    QVector<double> copyLogDataVariableValues(const QString &rName, const int generation);
    SharedVariablePtrT getLogVariableDataPtr(int generation, QString componentName, QString portName, QString dataName); //!< @deprecated
    SharedVariablePtrT getLogVariableDataPtr(const QString &rName, const int generation) const;

    QVector<SharedVariablePtrT> getMultipleLogVariableDataPtrs(const QRegExp &rNameExp, const int generation=-1) const;
    QVector<SharedVariablePtrT> getAllVariablesAtNewestGeneration();
    QVector<SharedVariablePtrT> getAllVariablesAtGeneration(const int generation) const;

    QList<QString> getImportedVariablesFileNames() const;
    QList<SharedVariablePtrT> getImportedVariablesForFile(const QString &rFileName);
    QList<int> getImportFileGenerations(const QString &rFilePath) const;
    QMap<QString, QList<int> > getImportFilesAndGenerations() const;
    void removeImportedFileGenerations(const QString &rFileName);

    void defineAlias(const QString &rFullName);
    bool defineAlias(const QString &rAlias, const QString &rFullName);
    void undefinePlotAlias(const QString &rAlias);

    QString getFullNameFromAlias(const QString &rAlias);
    QString getAliasFromFullName(const QString &rFullName);

    QList<int> getGenerations() const;
    int getLowestGenerationNumber() const;
    int getHighestGenerationNumber() const;
    void getLowestAndHighestGenerationNumber(int &rLowest, int &rHighest) const;
    int getLatestGeneration() const; //!< @todo Name is a bit unclear, espicially compared to getHighestGenerationNumber()
    int getNumberOfGenerations() const;
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

    FavoriteListT getFavoriteVariableList();
    void setFavoriteVariable(QString componentName, QString portName, QString dataName, QString dataUnit);
    void removeFavoriteVariableByComponentName(QString componentName);

    QString plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color=QColor());
    QString plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color=QColor());
    PlotWindow *plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color=QColor());

    SharedVariablePtrT addVariableWithScalar(const SharedVariablePtrT a, const double x);
    SharedVariablePtrT subVariableWithScalar(const SharedVariablePtrT a, const double x);
    SharedVariablePtrT mulVariableWithScalar(const SharedVariablePtrT a, const double x);
    SharedVariablePtrT divVariableWithScalar(const SharedVariablePtrT a, const double x);
    QString addVariableWithScalar(const QString &a, const double x); //!< @deprecated
    QString subVariableWithScalar(const QString &a, const double x); //!< @deprecated
    QString mulVariableWithScalar(const QString &a, const double x); //!< @deprecated
    QString divVariableWithScalar(const QString &a, const double x); //!< @deprecated

    SharedVariablePtrT addVariables(const SharedVariablePtrT a, const SharedVariablePtrT b);
    SharedVariablePtrT subVariables(const SharedVariablePtrT a, const SharedVariablePtrT b);
    SharedVariablePtrT multVariables(const SharedVariablePtrT a, const SharedVariablePtrT b);
    SharedVariablePtrT divVariables(const SharedVariablePtrT a, const SharedVariablePtrT b);
    QString addVariables(const QString &a, const QString &b); //!< @deprecated
    QString subVariables(const QString &a, const QString &b); //!< @deprecated
    QString multVariables(const QString &a, const QString &b); //!< @deprecated
    QString divVariables(const QString &a, const QString &b); //!< @deprecated

    SharedVariablePtrT diffVariables(const SharedVariablePtrT a, const SharedVariablePtrT b);
    QString diffVariables(const QString &a, const QString &b); //!< @deprecated

    SharedVariablePtrT integrateVariables(const SharedVariablePtrT a, const SharedVariablePtrT b);
    QString integrateVariables(const QString &a, const QString &b); //!< @deprecated

    SharedVariablePtrT lowPassFilterVariable(const SharedVariablePtrT a, const SharedVariablePtrT b, const double freq);
    QString lowPassFilterVariable(const QString &a, const QString &b, const double freq); //!< @deprecated

    SharedVariablePtrT fftVariable(const SharedVariablePtrT a, const SharedVariablePtrT b, const bool doPowerSpectrum);
    QString fftVariable(const QString &a, const QString &b, const bool doPowerSpectrum); //!< @deprecated

    QString assignVariable(const QString &dst, const QString &src); //!< @deprecated
    QString assignVariable(const QString &dst, const QVector<double> &src); //!< @deprecated

    double pokeVariable(SharedVariablePtrT a, const int index, const double value);
    double pokeVariable(const QString &a, const int index, const double value); //!< @deprecated

    double peekVariable(SharedVariablePtrT a, const int b);
    double peekVariable(const QString &a, const int index); //!< @deprecated

    SharedVariablePtrT elementWiseGT(SharedVariablePtrT pData, const double thresh);
    SharedVariablePtrT elementWiseLT(SharedVariablePtrT pData, const double thresh);

    SharedVariablePtrT saveVariable(SharedVariablePtrT a);
    QString saveVariable(const QString &currName, const QString &newName); //!< @deprecated

    void appendVariable(const QString &a, const double x, const double y); //!< @deprecated

    void takeOwnershipOfData(LogDataHandler *pOtherHandler, const int otherGeneration=-2);

public slots:
    void registerAlias(const QString &rFullName, const QString &rAlias);
    void unregisterAlias(const QString &rAlias);


signals:
    void newDataAvailable();
    void dataRemoved();
    void closePlotsWithOwnedData();

private slots:
    void forgetImportedLogDataVariable(SharedVariablePtrT pData);

private:
    typedef QMap< QString, QMultiMap<QString, SharedVariablePtrT> > ImportedLogDataMapT;
    typedef QMap<int, SharedMultiDataVectorCacheT> GenerationCacheMapT;
    SharedVariablePtrT insertTimeVariable(const QVector<double> &rTimeVector);
    SharedVariablePtrT insertTimeVariable(const QVector<double> &rTimeVector, const QString &rImportFileName);
    SharedVariablePtrT insertTimeDomainVariable(SharedVariablePtrT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc);
    SharedVariablePtrT insertTimeDomainVariable(SharedVariablePtrT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName);
    void insertVariable(SharedVariablePtrT pVariable);

    QString getNewCacheName();
    void rememberIfImported(SharedVariablePtrT pData);
    void removeGenerationCacheIfEmpty(const int gen);
    SharedVariablePtrT defineNewVectorVariable_NoNameCheck(const QString &rName);

    ContainerObject *mpParentContainerObject;

    LogDataMapT mLogDataMap;
    ImportedLogDataMapT mImportedLogDataMap;

    FavoriteListT mFavoriteVariables;
    GenerationCacheMapT mGenerationCacheMap;

    int mnPlotCurves;
    int mGenerationNumber;
    quint64 mTempVarCtr;
    QList<QDir> mCacheDirs;
    quint64 mCacheSubDirCtr;
};

#endif // LOGDATAHANDLER_H
