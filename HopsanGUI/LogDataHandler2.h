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

//$Id$

#ifndef LOGDATAHANDLER2_H
#define LOGDATAHANDLER2_H


#include <QVector>
#include <QMap>
#include <QString>
#include <QColor>
#include <QObject>
#include <QDir>

#include "LogVariable.h"
#include "Widgets/ModelWidget.h"
#include "PlotCurveStyle.h"

// Forward Declaration
class PlotWindow;
class ModelWidget;
class LogDataGeneration;


class LogDataHandler2 : public QObject
{
    Q_OBJECT

public:
    typedef QMap<int, QString > ImportedGenerationsMapT;

    LogDataHandler2(ModelWidget *pParentModel);
    ~LogDataHandler2();

    void setParentModel(ModelWidget *pParentModel);
    ModelWidget *getParentModel();

    void createEmptyGeneration();
    void collectLogDataFromModel(bool overWriteLastGeneration=false);
    void collectLogDataFromRemoteModel(QVector<RemoteResultVariable> &rResultVariables, bool overWriteLastGeneration=false);
    void importFromPlo(QString importFilePath=QString());
    void importFromCSV_AutoFormat(QString importFilePath=QString());
    void importHopsanRowCSV(QString importFilePath=QString());
    void importFromPlainColumnCsv(QString importFilePath=QString(), const QChar separator=',', const int rowsToSkip=0, const int timecolumn=0);
    void importFromPlainRowCsv(QString importFilePath=QString(), const QChar separator=',', const int columnsToSkip=0, const int timeRow=0);
    void importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> datacolumns, QStringList datanames, QVector<int> timecolumns);

    void exportGenerationToPlo(const QString &rFilePath, int gen, const int version=-1) const;
    void exportToPlo(const QString &rFilePath, QList<SharedVectorVariableT> variables, int version=-1) const;
    void exportToCSV(const QString &rFilePath, const QList<SharedVectorVariableT> &rVariables) const;
    void exportGenerationToCSV(const QString &rFilePath, int gen) const;
    void exportToHDF5(const QString &rFilePath, const QList<SharedVectorVariableT> &rVariables) const;
    void exportGenerationToHDF5(const QString &rFilePath, int gen) const;

    SharedVectorVariableT insertNewVectorVariable(const QString &rDesiredname, VariableTypeT type=VectorType, const int gen=-1);
    SharedVectorVariableT insertNewVectorVariable(SharedVectorVariableT pVariable, const int gen=-1);
    SharedVectorVariableT defineNewVectorVariable(const QString &rDesiredname, VariableTypeT type=VectorType);
    SharedVectorVariableT createOrphanVariable(const QString &rName, VariableTypeT type=VectorType);

    bool removeVariable(const QString &rVarName, int generation=-1);

    bool isEmpty();
    void clear();

    QStringList getVariableFullNames(int generation=-1) const;
    bool hasVariable(const QString &rFullName, const int generation=-1);

    const SharedVectorVariableT getTimeVectorVariable(int generation=-1) const;
    QVector<double> copyTimeVector(const int generation=-1) const;

    QVector<double> copyVariableDataVector(const QString &rName, const int generation=-1);
    SharedVectorVariableT getVectorVariable(const QString &rName, int generation=-1) const;

    QList<SharedVectorVariableT> getMatchingVariablesAtGeneration(const QRegExp &rNameExp, int generation=-1, const VariableNameTypeT nametype=AliasAndFull) const;
    QList<SharedVectorVariableT> getMatchingVariablesFromAllGenerations(const QRegExp &rNameExp, const VariableNameTypeT nametype=AliasAndFull) const;
    QList<SharedVectorVariableT> getMatchingVariablesAtRespectiveNewestGeneration(const QRegExp &rNameExp, const VariableNameTypeT nametype=AliasAndFull) const;
    QList<SharedVectorVariableT> getAllNonAliasVariablesAtGeneration(const int generation) const;
    QList<SharedVectorVariableT> getAllVariablesAtGeneration(const int gen);
    QList<SharedVectorVariableT> getAllVariablesAtCurrentGeneration();
    QList<SharedVectorVariableT> getAllVariablesAtRespectiveNewestGeneration();


    QList<int> getImportedGenerations() const;
    ImportedGenerationsMapT getImportFilesAndGenerations() const;
    bool isGenerationImported(int generation);

    int getNumberOfGenerations() const;
    QList<int> getGenerations() const;
    int getLowestGenerationNumber() const;
    int getHighestGenerationNumber() const;
    int getCurrentGenerationNumber() const;
    int getHighestModelGeneration() const;
    const LogDataGeneration* getCurrentGeneration() const;
    const LogDataGeneration* getGeneration(const int gen) const;
    bool hasGeneration(const int gen) const;

    void getVariableGenerationInfo(const QString &rFullName, int &rLowest, int &rHighest) const;

    void limitPlotGenerations();
    bool removeGeneration(const int gen, const bool force);


    const QList<QDir> &getCacheDirs() const;
    SharedMultiDataVectorCacheT getGenerationMultiCache(const int gen);
    void pruneGenerationCache(const int generation);

    void incrementOpenPlotCurves();
    void decrementOpenPlotCurves();
    bool hasOpenPlotCurves();
    void closePlotsWithCurvesBasedOnOwnedData();

    PlotWindow *plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, PlotCurveStyle style=QColor());
    PlotWindow *plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, PlotCurveStyle style=QColor());
    PlotWindow *plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, PlotCurveStyle style=QColor());

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

    SharedVectorVariableT elementWiseGT(SharedVectorVariableT pData, const double thresh);
    SharedVectorVariableT elementWiseLT(SharedVectorVariableT pData, const double thresh);

    SharedVectorVariableT elementWisePower(SharedVectorVariableT a, const double x);

    void setGenerationTimePlotOffset(int generation, double offset);

    void takeOwnershipOfData(LogDataHandler2 *pOtherHandler, const int otherGeneration=-2);

public slots:
    void registerAlias(const QString &rFullName, const QString &rAlias, int gen=-1);
    void unregisterAlias(const QString &rAlias, int gen=-1);
    bool registerQuantity(const QString &rFullName, const QString &rQuantity, int gen);
    bool registerQuantity(const QString &rFullName, const QString &rQuantity);


signals:
    void dataAdded();
    void dataAddedFromModel(bool);
    void dataRemoved();
    void aliasChanged();
    void quantityChanged();
    void closePlotsWithOwnedData();

private:
    typedef QMap< int, LogDataGeneration* > GenerationMapT;
    typedef QMap<int, SharedMultiDataVectorCacheT> GenerationCacheMapT;

    SharedVectorVariableT insertCustomVectorVariable(const QVector<double> &rVector, SharedVariableDescriptionT pVarDesc);
    SharedVectorVariableT insertCustomVectorVariable(const QVector<double> &rVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName);
    SharedVectorVariableT insertTimeVectorVariable(const QVector<double> &rTimeVector, SharedSystemHierarchyT pSysHierarchy);
    SharedVectorVariableT insertTimeVectorVariable(const QVector<double> &rTimeVector, const QString &rImportFileName);
    SharedVectorVariableT insertFrequencyVectorVariable(const QVector<double> &rFrequencyVector, SharedSystemHierarchyT pSysHierarchy);
    SharedVectorVariableT insertFrequencyVectorVariable(const QVector<double> &rFrequencyVector, const QString &rImportFileName);
    SharedVectorVariableT insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc);
    SharedVectorVariableT insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName);
    SharedVectorVariableT insertFrequencyDomainVariable(SharedVectorVariableT pFrequencyVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc);
    SharedVectorVariableT insertFrequencyDomainVariable(SharedVectorVariableT pFrequencyVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName);
    SharedVectorVariableT insertVariable(SharedVectorVariableT pVariable, QString keyName=QString(), int gen=-1);

    bool collectLogDataFromSystem(SystemObject *pCurrentSystem, const QStringList &rSystemHieararchy, QMap<std::vector<double> *, SharedVectorVariableT> &rGenTimeVectors);

    QString getNewCacheName(const QString &rDesiredName=QString());
    void removeGenerationCacheIfEmpty(const int gen);
    void pruneGenerationCache(const int generation, LogDataGeneration *pGeneration);

    ModelWidget *mpParentModel = nullptr;
    int mNumPlotCurves = 0;
    int mCurrentGenerationNumber = -1;

    ImportedGenerationsMapT mImportedGenerationsMap;
    GenerationCacheMapT mGenerationCacheMap;
    GenerationMapT mGenerationMap;

    QList<QDir> mCacheDirs;
    quint64 mCacheSubDirCtr = 0;
};


#endif // LOGDATAHANDLER2_H
