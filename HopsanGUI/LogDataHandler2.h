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
#include "Utilities/IndexIntervalCollection.h"

// Forward Declaration
class PlotWindow;
class ContainerObject;
class SystemContainer;
class LogDataHandler2;
class ModelWidget;



class Generation
{
    friend class LogDataHandler2;
public:
    Generation(const QString &rImportfile="");

    int getNumVariables() const;
    bool isEmpty();
    void clear();

    bool isImported() const;
    QString getImportFileName() const;

    void addVariable(const QString &rFullName, SharedVectorVariableT variable, bool isAlias);
    bool removeVariable(const QString &rFullName);

    QStringList getVariableFullNames() const;
    bool haveVariable(const QString &rFullName) const;

    SharedVectorVariableT getVariable(const QString &rFullName) const;
    QList<SharedVectorVariableT> getMatchingVariables(const QRegExp &rNameExp);

    QList<SharedVectorVariableT> getAllNonAliasVariables() const;
    QList<SharedVectorVariableT> getAllVariables() const;

    bool registerAlias(const QString &rFullName, const QString &rAlias);
    bool unregisterAlias(const QString &rAlias);
    bool unregisterAliasForFullName(const QString &rFullName);
    QString getFullNameFromAlias(const QString &rAlias);

private:
    typedef QMap< QString, SharedVectorVariableT > VariableMapT;

    QList<SharedVectorVariableT>  getMatchingVariables(const QRegExp &rNameExp, VariableMapT &rMap);

    VariableMapT mVariables;
    VariableMapT mAliasVariables;
    QList<SharedVectorVariableT> mTimeVectors;

    QString mImportedFromFile;
    SharedMultiDataVectorCacheT mGenerationDataCache;

    IndexIntervalCollection mAliasGenIndexes;

};

class LogDataHandler2 : public QObject
{
    Q_OBJECT

    typedef QMap< int, Generation* > GenerationMapT;

public:
    LogDataHandler2(ModelWidget *pParentModel);
    ~LogDataHandler2();

    void setParentModel(ModelWidget *pParentModel);
    ModelWidget *getParentModel();

    void collectLogDataFromModel(bool overWriteLastGeneration=false);
    void collectLogDataFromRemoteModel(std::vector<std::string> &rNames, std::vector<double> &rData, bool overWriteLastGeneration=false);
    void importFromPlo(QString importFilePath=QString());
    void importFromCSV_AutoFormat(QString importFilePath=QString());
    void importHopsanRowCSV(QString importFilePath=QString());
    void importFromPlainColumnCsv(QString importFilePath=QString());
    void importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> datacolumns, QStringList datanames, QVector<int> timecolumns);

    void exportGenerationToPlo(const QString &rFilePath, int gen, const int version=-1) const;
    void exportToPlo(const QString &rFilePath, QList<SharedVectorVariableT> variables, int version=-1) const;
    void exportToCSV(const QString &rFilePath, const QList<SharedVectorVariableT> &rVariables) const;
    void exportGenerationToCSV(const QString &rFilePath, const int gen) const;

    SharedVectorVariableT insertNewHopsanVariable(const QString &rDesiredname, VariableTypeT type=VectorType, const int gen=-1);
    SharedVectorVariableT insertNewHopsanVariable(SharedVectorVariableT pVariable, const int gen=-1);
    SharedVectorVariableT defineNewVectorVariable(const QString &rDesiredname, VariableTypeT type=VectorType);
    SharedVectorVariableT createOrphanVariable(const QString &rName, VariableTypeT type=VectorType);

    bool removeVariable(const QString &rVarName, int generation=-1);

    //int getNumVariables() const;
    bool isEmpty();
    void clear();

    QStringList getVariableFullNames(int generation=-1) const;
    bool hasVariable(const QString &rFullName, const int generation=-1);

    const SharedVectorVariableT getTimeVectorVariable(int generation) const;
    QVector<double> copyTimeVector(const int generation) const;

    QVector<double> copyVariableDataVector(const QString &rName, const int generation);
    SharedVectorVariableT getVectorVariable(const QString &rName, int generation=-1) const;

    QList<SharedVectorVariableT> getMatchingVariablesAtGeneration(const QRegExp &rNameExp, int generation=-1) const;
    QList<SharedVectorVariableT> getMatchingVariablesFromAllGeneration(const QRegExp &rNameExp) const;
    QList<SharedVectorVariableT> getAllNonAliasVariablesAtGeneration(const int generation) const;
    QList<SharedVectorVariableT> getAllVariablesAtGeneration(const int gen);
    QList<SharedVectorVariableT> getAllVariablesAtCurrentGeneration();
    QList<SharedVectorVariableT> getAllVariablesAtRespectiveNewestGeneration();
    QList<SharedVectorVariableT> getMatchingVariablesAtRespectiveNewestGeneration(const QRegExp &rNameExp) const;

    QList<QString> getImportedGenerationFileNames() const;
    QList<SharedVectorVariableT> getImportedVariablesForFile(const QString &rFileName);
    QList<int> getImportFileGenerations(const QString &rFilePath) const;
    QMap<QString, QList<int> > getImportFilesAndGenerations() const;
    void removeImportedFileGenerations(const QString &rFileName);
    bool isGenerationImported(const int gen);

    //QString getFullNameFromAlias(const QString &rAlias, const int gen=-1) const;

    int getNumberOfGenerations() const;
    QList<int> getGenerations() const;
    int getLowestGenerationNumber() const;
    int getHighestGenerationNumber() const;
    int getCurrentGenerationNumber() const;
    const Generation* getCurrentGeneration() const;
    const Generation* getGeneration(const int gen) const;

    void getVariableGenerationInfo(const QString &rFullName, int &rLowest, int &rHighest) const;

    void limitPlotGenerations();
    void preventGenerationAutoRemoval(const int gen);
    void allowGenerationAutoRemoval(const int gen);
    bool removeGeneration(const int gen, const bool force);


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

    void takeOwnershipOfData(LogDataHandler2 *pOtherHandler, const int otherGeneration=-2);

public slots:
    void registerAlias(const QString &rFullName, const QString &rAlias, int gen=-1);
    void unregisterAlias(const QString &rAlias, int gen=-1);


signals:
    void dataAdded();
    void dataAddedFromModel(bool);
    void dataRemoved();
    void aliasChanged();
    void closePlotsWithOwnedData();

private slots:
    void forgetImportedVariable(SharedVectorVariableT pData);

private:
    typedef QMultiMap< QString, int > ImportedGenerationsMapT;
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

    bool collectLogDataFromSystem(SystemContainer *pCurrentSystem, const QStringList &rSystemHieararchy, QMap<std::vector<double> *, SharedVectorVariableT> &rGenTimeVectors);

    QString getNewCacheName();
    void rememberIfImported(SharedVectorVariableT data);
    void removeGenerationCacheIfEmpty(const int gen);
    void unregisterAliasForFullName(const QString &rFullName);

    ModelWidget *mpParentModel;


    ImportedGenerationsMapT mImportedGenerationsMap;
    GenerationCacheMapT mGenerationCacheMap;

    GenerationMapT mGenerationMap;
    QList<int> mKeepGenerations;

    int mNumPlotCurves;
    int mCurrentGenerationNumber;
    QList<QDir> mCacheDirs;
    quint64 mCacheSubDirCtr;
};


#endif // LOGDATAHANDLER2_H
