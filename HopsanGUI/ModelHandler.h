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

#include <QObject>
#include <QDomElement>
#include <QAction>
#include <QFileSystemWatcher>

#include "GraphicsViewPort.h"

class ModelWidget;
class CentralTabWidget;
class SystemObject;
class SystemObject;
class LogDataHandler2;
class SimulationThreadHandler;
class DebuggerWidget;
class TextEditorWidget;

class ModelStateInfo
{
public:
    QString modelFile;
    QString backupFile;
    bool hasChanged;
    QString tabName;
    QSharedPointer<LogDataHandler2> logDataHandler;
    QDomDocument model;
    GraphicsViewPort viewPort;
};


class ModelHandler : public QObject
{
    Q_OBJECT
public:
    enum LoadOption {
        NoLoadOptions = 0x0,
        IgnoreAlreadyOpen = 0x1,
        Detatched = 0x2,
        DontAddToRecentModels = 0x4
    };
    Q_DECLARE_FLAGS(LoadOptions, LoadOption)

    ModelHandler(QObject *parent=0);

    void addModelWidget(ModelWidget *pModelWidget, const QString &name, ModelHandler::LoadOptions options=NoLoadOptions);

    void setCurrentModel(int idx);
    void setCurrentModel(ModelWidget *pWidget);

    ModelWidget *getModel(const QString &rModelFilePath);
    ModelWidget *getModel(int idx);
    ModelWidget *getCurrentModel();
    SystemObject *getTopLevelSystem(const QString &rModelFilePath);
    SystemObject *getTopLevelSystem(int idx);
    SystemObject *getCurrentTopLevelSystem();
    SystemObject *getViewContainerObject(int idx);
    SystemObject *getCurrentViewContainerObject();
    QSharedPointer<LogDataHandler2> getCurrentLogDataHandler();

    int count() const;

    ModelWidget *loadModel(QString modelFileName, ModelHandler::LoadOptions options=NoLoadOptions);
    TextEditorWidget *loadTextFile(QString scriptFileName);
    SimulationThreadHandler *mpSimulationThreadHandler;

public slots:
    ModelWidget *addNewModel(QString modelName="Untitled", ModelHandler::LoadOptions options=NoLoadOptions);
    void loadModel();
    void loadModel(QAction *action);
    void newTextFile();
    void loadTextFile();
    void loadModelParametersFromHpf();
    void loadModelParametersFromSsv();
    bool closeModelByTabIndex(int tabIdx, bool force=false);
    bool closeModel(int idx, bool force=false);
    bool closeScript(int idx, bool force=false);
    bool closeModel(ModelWidget *pModel, bool force=false);
    bool closeAllModels(bool force=false);
    void selectModelByTabIndex(int tabIdx);
    void saveState();
    void restoreState();

    void revertCurrentModel();

    void createLabviewWrapperFromCurrentModel();
    void exportCurrentModelToFMU1_32();
    void exportCurrentModelToFMU1_64();
    void exportCurrentModelToFMU2_32();
    void exportCurrentModelToFMU2_64();
    void exportCurrentModelToFMU3_32();
    void exportCurrentModelToFMU3_64();
    void exportCurrentModelToSimulink();
    void exportCurrentModelToExe_32();
    void exportCurrentModelToExe_64();

    void showLosses(bool show);
    void measureSimulationTime();
    void launchDebugger();
    void openAnimation();

    bool simulateAllOpenModels_nonblocking(bool modelsHaveNotChanged=false);
    bool simulateAllOpenModels_blocking(bool modelsHaveNotChanged=false);
    bool simulateMultipleModels_nonblocking(QVector<ModelWidget*> models, bool noChange=false);
    bool simulateMultipleModels_blocking(QVector<ModelWidget*> models, bool noChanges=false);

signals:
    void newModelWidgetAdded();
    void checkMessages();
    void modelChanged(ModelWidget *);

private:
    void refreshMainWindowConnections();
    void disconnectMainWindowConnections(ModelWidget* pModel);
    void disconnectMainWindowConnections(TextEditorWidget *pScriptEditor);
    void connectMainWindowConnections(ModelWidget *pModel);
    void connectMainWindowConnections(TextEditorWidget *pScriptEditor);
    void setToolBarSimulationTimeFromTab(ModelWidget *pModel);

    QList<ModelWidget*> mModelPtrs;
    QList<TextEditorWidget*> mTextEditors;
    int mCurrentIdx;
    int mNumberOfUntitledModels;
    int mNumberOfUntitledScripts;

    QList<ModelStateInfo> mStateInfoList;
    int mStateInfoIndex;

    DebuggerWidget *mpDebugger;

    QFileSystemWatcher *mpFileWatcher;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModelHandler::LoadOptions)

#endif // MODELHANDLER_H
