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
//! @file   OptimizationDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-10-24
//!
//! @brief Contains a class for the optimization dialog
//!
//$Id$

#ifndef OPTIMIZATIONDIALOG_H
#define OPTIMIZATIONDIALOG_H

#include <QMainWindow>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QToolButton>
#include <QComboBox>
#include <QWizard>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QProgressBar>
#include <QToolBar>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidget>

#include "common.h"

class TerminalWidget;
class SystemObject;
class GUIMessageHandler;
class OptimizationScriptWizard;


class OptimizationDialog : public QMainWindow
{
  Q_OBJECT

public:
  OptimizationDialog(QWidget *parent = 0);

  void updateParameterOutputs(const std::vector<double> &objectives, const std::vector<std::vector<double> > &values, const int bestId, const int worstId);
  void updateTotalProgressBar(double progress);
  void setOptimizationFinished();
  void setCode(const QString &code);

public slots:
    virtual void open();
    virtual void close();
    void setOutputDisabled(bool disabled);
    void run();

private slots:
    void generateScriptSkeleton();
    void openScriptWizard();
    void save();
    void saveAs(QString filePath = QString());
    void loadScriptFile(QString filePath = QString());
    void updateCoreProgressBars();
    void recreateCoreProgressBars();
    void recreateParameterOutputLineEdits();
    void applyParameters();
    void updateModificationStatus();

private:
    bool askForSavingScript();
    QToolBar *createToolBar();
    QWidget *createScriptWidget();
    QWidget *createRunWidget();

    //Original system
    SystemObject *mpSystem;

    //Main dialog
    QTabWidget *mpTabWidget;

    //Script tab
    QTextEdit *mpScriptBox;

    //Run tab
    QList<QProgressBar*> mCoreProgressBarPtrs;
    QProgressBar *mpTotalProgressBar;
    QList<QPushButton *> mParametersApplyButtonPtrs;
    QStandardItemModel *mpParametersModel;
    QStandardItemModel *mpCoreProgressBarsModel;
    QTableView *mpParametersOutputTableView;
    QTableWidget *mpCoreProgressBarsTableWidget;
    QGridLayout *mpCoreProgressBarsLayout;
    TerminalWidget *mpTerminal;
    QLabel *mpModelNameLabel;
    QLabel *mpScriptFileLabel;

    //Toolbar
    QToolButton *mpRunButton;

    //Utilities
    GUIMessageHandler *mpMessageHandler;
    QTimer *mpTimer;

    bool mCoreProgressBarsRecreated;
    bool mOutputDisabled=false;
    QVector<int> mParameterOutputIndexes;

    QFileInfo mScriptFileInfo;
    bool mScriptTextChanged = false;
    QString mSavedScriptText = QString();
};




#endif // OPTIMIZATIONDIALOG_H
