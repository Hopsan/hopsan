#ifndef MODELHANDLER_H
#define MODELHANDLER_H

#include <QtGui>
#include <QObject>
#include <QDomElement>
#include "Widgets/ProjectTabWidget.h"

class ModelWidget;
class CentralTabWidget;
class SystemContainer;
class ContainerObject;
class LogDataHandler;
class SimulationThreadHandler;

class ModelHandler : public QObject
{
    Q_OBJECT
public:
    ModelHandler(QObject *parent=0);

    void addModelWidget(ModelWidget *pModelWidget, QString name, bool hidden=false);

    void setCurrentModel(int idx);
    void setCurrentModel(ModelWidget *pWidget);

    ModelWidget *getModel(int idx);
    ModelWidget *getCurrentModel();
    SystemContainer *getTopLevelSystem(int idx);
    SystemContainer *getCurrentTopLevelSystem();
    ContainerObject *getContainer(int idx);
    ContainerObject *getCurrentContainer();

    int count();

    ModelWidget *loadModel(QString modelFileName, bool ignoreAlreadyOpen=false, bool hidden=false);

    void setCurrentTopLevelSimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime);

    SimulationThreadHandler *mpSimulationThreadHandler;

public slots:
    ModelWidget *addNewModel(QString modelName="Untitled", bool hidden=false);
    void loadModel();
    void loadModel(QAction *action);
    void loadModelParameters();
    bool closeModelByTabIndex(int tabIdx);
    bool closeModel(int idx);
    bool closeModel(ModelWidget *pModel);
    bool closeAllModels();
    void modelChanged();
    void saveState();
    void restoreState();

    void createLabviewWrapperFromCurrentModel();
    void exportCurrentModelToFMU();
    void exportCurrentModelToSimulink();
    void exportCurrentModelToSimulinkCoSim();

    void showLosses(bool show);
    void measureSimulationTime();
    void launchDebugger();
    void openAnimation();

    bool simulateAllOpenModels_nonblocking(bool modelsHaveNotChanged=false);
    bool simulateAllOpenModels_blocking(bool modelsHaveNotChanged=false);

signals:
    void newModelWidgetAdded();
    void checkMessages();

private:
    QList<ModelWidget*> mModelPtrs;
    int mCurrentIdx;
    int mNumberOfUntitledModels;

    QStringList mStateInfoHmfList;
    QStringList mStateInfoBackupList;
    QList<bool> mStateInfoHasChanged;
    QStringList mStateInfoTabNames;
    QList<LogDataHandler*> mStateInfoLogDataHandlersList;
    QList<QDomDocument> mStateInfoModels;
};

#endif // MODELHANDLER_H
