/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

#include <QDialog>
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

class TerminalWidget;
class SystemContainer;
class GUIMessageHandler;

class OptimizationDialog : public QWizard
{
    Q_OBJECT

    friend class OptimizationHandler;

public:
    OptimizationDialog(QWidget *parent = 0);

    void updateParameterOutputs(const QVector<double> &objectives, const QVector<QVector<double> > &values, int bestId, int worstId);
    void updateTotalProgressBar(double progress);
    void setOptimizationFinished();
    void setCode(const QString &code);
protected:
    QTreeWidgetItem* findParameterTreeItem(QString componentName, QString parameterName);

public slots:
    virtual void open();
    virtual void accept();
    virtual void reject();
    virtual void okPressed();

private slots:
    void setAlgorithm(int i);
    void updateChosenParameters(QTreeWidgetItem* item, int i);
    void removeParameter();
    void updateChosenVariables(QTreeWidgetItem* item, int i);
    void addFunction();
    void removeFunction();
    void update(int idx);
    void run();
    void saveScriptFile();
    void saveScriptFile(const QString &filePath);
    void loadScriptFile();
    void updateCoreProgressBars();
    void recreateCoreProgressBars();
    void recreateParameterOutputLineEdits();
    void applyParameters();
    void saveConfiguration();
    void regenerateScript();

private:
    void generateScriptFile();
    void generateComplexScript(const QString &subAlgorithm);
    void generateComplexScriptOld();
    void generateParticleSwarmScript();
    void generateParameterSweepScript();
    void generateParticleSwarmScriptOld();
    bool verifyNumberOfVariables(int i, int nSelVar);
    bool loadObjectiveFunctions();
    QString generateFunctionCode(int i);

    void loadConfiguration();

    void addObjectiveFunction(int idx, double weight, double norm, double exp, QList<QStringList> selectedVariables, QStringList objData);

    //Original system
    SystemContainer *mpSystem;

    //Settings page
    QComboBox *mpAlgorithmBox;
    QSpinBox *mpIterationsSpinBox;
    QLabel *mpSearchPointsLabel;
    QSpinBox *mpSearchPointsSpinBox;
    QLabel *mpParticlesLabel;
    QSpinBox *mpParticlesSpinBox;
    QLineEdit *mpAlphaLineEdit;
    QLabel *mpAlphaLabel;
    QLabel *mpBetaLabel;
    QLabel *mpGammaLabel;
    QLineEdit *mpBetaLineEdit;
    QLineEdit *mpGammaLineEdit;
    QLabel *mpOmegaLabel;
    QLineEdit *mpOmegaLineEdit;
    QLabel *mpC1Label;
    QLineEdit *mpC1LineEdit;
    QLabel *mpC2Label;
    QLineEdit *mpC2LineEdit;
    QLabel *mpLengthLabel;
    QSpinBox *mpLengthSpinBox;
    QLabel *mpPercDiffLabel;
    QLineEdit *mpPercDiffLineEdit;
    QLabel *mpCountMaxLabel;
    QSpinBox *mpCountMaxSpinBox;
    QLineEdit *mpEpsilonFLineEdit;
    QLineEdit *mpEpsilonXLineEdit;
    QCheckBox *mpPlotParticlesCheckBox;
    QCheckBox *mpPlotEntropyCheckBox;
    QCheckBox *mpPlottingCheckBox;
    QCheckBox *mpPlotBestWorstCheckBox;
    QCheckBox *mpExport2CSVBox;
    QCheckBox *mpFinalEvalCheckBox;

    //Parameters page
    QCheckBox *mpParametersLogCheckBox;
    QTreeWidget *mpParametersList;
    QGridLayout *mpParametersLayout;

    //Objective function page
    QComboBox *mpMinMaxComboBox;
    QComboBox *mpFunctionsComboBox;
    QTreeWidget *mpVariablesList;
    QPushButton *mpAddFunctionButton;
    QList<QLineEdit*> mWeightLineEditPtrs;
    QList<QLineEdit*> mNormLineEditPtrs;
    QList<QLineEdit*> mExpLineEditPtrs;
    QList<QLabel*> mFunctionLabelPtrs;
    QList<QString> mFunctionName;
    QList<QWidget*> mDataWidgetPtrs;
    QList< QList<QLineEdit*> > mDataLineEditPtrs;
    QList<QToolButton*> mRemoveFunctionButtonPtrs;
    QGridLayout *mpObjectiveLayout;

    QStringList mObjectiveFunctionDescriptions;
    QStringList mObjectiveFunctionCalls;
    QList<int> mObjectiveFunctionNumberOfVariables;
    QList<bool> mObjectiveFunctionUsesTimeVector;
    QList<QStringList> mObjectiveFunctionDataLists;

    //Output
    QTextEdit *mpOutputBox;
    TerminalWidget *mpTerminal;
    GUIMessageHandler *mpMessageHandler;

    //Run page
    QGridLayout *mpParametersOutputTextEditsLayout;
    QList<QLineEdit *> mParametersOutputLineEditPtrs;
    QList<QPushButton *> mParametersApplyButtonPtrs;
    QGridLayout *mpCoreProgressBarsLayout;
    QList<QProgressBar*> mCoreProgressBarPtrs;
    QProgressBar *mpTotalProgressBar;
    QPushButton *mpStartButton;
    QLabel *mpModelNameLabel;
    QLabel *mpScriptFileLabel;

    //Member variables
    QTimer *mpTimer;
    QString mScript;
    QStringList mFunctions;
    QStringList mSelectedFunctionsMinMax;
    QList<int> mSelectedFunctions;
    QList<QStringList> mFunctionComponents;
    QList<QStringList> mFunctionPorts;
    QList<QStringList> mFunctionVariables;
    QList<double> mFunctionData;
    QStringList mSelectedComponents;
    QStringList mSelectedParameters;
    QList<QLabel*> mpParameterLabels;
    QList<QLineEdit*> mpParameterMinLineEdits;
    QList<QLineEdit*> mpParameterMaxLineEdits;
    QList<QToolButton*> mpParameterRemoveButtons;
    QList<QStringList> mSelectedVariables;
    bool mCoreProgressBarsRecreated;
    QVector<int> mParameterOutputIndexes;
};

#endif // OPTIMIZATIONDIALOG_H
