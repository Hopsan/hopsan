#ifndef MODELHANDLER_H
#define MODELHANDLER_H

#include <QtGui>
#include <QObject>

class ProjectTab;
class ProjectTabWidget;
class SystemContainer;
class ContainerObject;

class ModelHandler : public QObject
{
    Q_OBJECT
public:
    ModelHandler(QObject *parent=0);

    void addModelWidget(ProjectTab *pModelWidget, QString name);
    void addNewProjectTab(QString modelName);

    ProjectTab *getModel(int idx);
    ProjectTab *getCurrentModel();
    SystemContainer *getTopLevelSystem(int idx);
    SystemContainer *getCurrentTopLevelSystem();
    ContainerObject *getContainer(int idx);
    ContainerObject *getCurrentContainer();

    void loadModel(QString modelFileName, bool ignoreAlreadyOpen=false);

    void setCurrentTopLevelSimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime);


public slots:
    void loadModel();
    void loadModel(QAction *action);
    //void loadModelParameters();
    bool closeModel(int idx);
    bool closeAllModels();
    void modelChanged();
    //void saveState();
    //void restoreState();

    //void createLabviewWrapperFromCurrentModel();
    //void exportCurrentModelToFMU();
    //void exportCurrentModelToSimulink();
    //void exportCurrentModelToSimulinkCoSim();

    //void showLosses(bool show);
    //void measureSimulationTime();
    void launchDebugger();
    //void openAnimation();

    //bool simulateAllOpenModels_nonblocking(bool modelsHaveNotChanged=false);
    //bool simulateAllOpenModels_blocking(bool modelsHaveNotChanged=false);

signals:
    void newModelWidgetAdded();

private:
    QList<ProjectTab*> mModelPtrs;
    int mCurrentIdx;
    int mNumberOfUntitledModels;
    ProjectTabWidget *mpProjectTabWidget;
};

#endif // MODELHANDLER_H
