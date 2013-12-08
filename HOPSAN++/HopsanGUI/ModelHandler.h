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
//! @file   ModelHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-05-16
//!
//! @brief Contains a model handler object
//!
//$Id$

#ifndef MODELHANDLER_H
#define MODELHANDLER_H

#include <QtGui>
#include <QObject>
#include <QDomElement>

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

    void addModelWidget(ModelWidget *pModelWidget, const QString &name, bool detatched=false);

    void setCurrentModel(int idx);
    void setCurrentModel(ModelWidget *pWidget);

    ModelWidget *getModel(int idx);
    ModelWidget *getCurrentModel();
    SystemContainer *getTopLevelSystem(int idx);
    SystemContainer *getCurrentTopLevelSystem();
    ContainerObject *getViewContainerObject(int idx);
    ContainerObject *getCurrentViewContainerObject();

    int count() const;

    ModelWidget *loadModel(QString modelFileName, bool ignoreAlreadyOpen=false, bool detatched=false);

    void setCurrentTopLevelSimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime);

    SimulationThreadHandler *mpSimulationThreadHandler;

public slots:
    ModelWidget *addNewModel(QString modelName="Untitled", bool hidden=false);
    void loadModel();
    void loadModel(QAction *action);
    void loadModelParameters();
    bool closeModelByTabIndex(int tabIdx);
    bool closeModel(int idx, bool force=false);
    bool closeModel(ModelWidget *pModel, bool force=false);
    bool closeAllModels();
    void selectModelByTabIndex(int tabIdx);
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
    bool simulateMultipleModels_nonblocking(QVector<ModelWidget*> models);
    bool simulateMultipleModels_blocking(QVector<ModelWidget*> models);

signals:
    void newModelWidgetAdded();
    void checkMessages();
    void modelChanged(ModelWidget *);

private:
    void refreshMainWindowConnections();
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
