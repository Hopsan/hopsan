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

    void updateParameterOutputs(const std::vector<double> &objectives, const std::vector<std::vector<double> > &values, int bestId, int worstId);
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
    void setOutputDisabled(bool disabled);
    void run();

private slots:
    void setAlgorithm(int i);
    void updateChosenParameters(QTreeWidgetItem* item, int i);
    void removeParameter();
    void updateChosenVariables(QTreeWidgetItem* item, int i);
    void addFunction();
    void removeFunction();
    void update(int idx);
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

    void generateNelderMeadScript();
    void generateComplexRFScript(const QString &subAlgorithm);
    void generateParticleSwarmScript();
    void generateDifferentialEvolutionScript();
    void generateParameterSweepScript();

    void generateObjectiveFunctionCode(QString &templateCode);
    void generateParameterCode(QString &templateCode);
    void generateCommonOptions(QString &templateCode);
    QString generateFunctionCode(int i);
    bool verifyNumberOfVariables(int i, int nSelVar);
    bool loadObjectiveFunctions();

    void loadConfiguration();

    void addObjectiveFunction(int idx, double weight, double norm, double exp, QList<QStringList> selectedVariables, QStringList objData);

    //Original system
    SystemContainer *mpSystem;

    //Settings page
    QComboBox *mpAlgorithmBox;
    QSpinBox *mpIterationsSpinBox;
    QLabel *mpParticlesLabel;
    QSpinBox *mpParticlesSpinBox;
    QLineEdit *mpAlphaLineEdit;
    QLabel *mpAlphaLabel;
    QLabel *mpBetaLabel;
    QLabel *mpGammaLabel;
    QLabel *mpRhoLabel;
    QLabel *mpSigmaLabel;
    QLineEdit *mpBetaLineEdit;
    QLineEdit *mpGammaLineEdit;
    QLineEdit *mpRhoLineEdit;
    QLineEdit *mpSigmaLineEdit;
    QLabel *mpOmega1Label;
    QLineEdit *mpOmega1LineEdit;
    QLabel *mpOmega2Label;
    QLineEdit *mpOmega2LineEdit;
    QLabel *mpC1Label;
    QLineEdit *mpC1LineEdit;
    QLabel *mpC2Label;
    QLineEdit *mpC2LineEdit;
    QLabel *mpVmaxLabel;
    QLineEdit *mpVmaxLineEdit;
    QLabel *mpFLabel;
    QLineEdit *mpFLineEdit;
    QLabel *mpCRLabel;
    QLineEdit *mpCRLineEdit;
    QLabel *mpNumModelsLabel;
    QLineEdit *mpNumModelsLineEdit;
    QLabel *mpMethodLabel;
    QComboBox *mpMethodComboBox;
    QLabel *mpLengthLabel;
    QSpinBox *mpLengthSpinBox;
    QLabel *mpPercDiffLabel;
    QLineEdit *mpPercDiffLineEdit;
    QLabel *mpCountMaxLabel;
    QSpinBox *mpCountMaxSpinBox;
    QLineEdit *mpEpsilonXLineEdit;
    QCheckBox *mpPlotParticlesCheckBox;
    QCheckBox *mpPlotEntropyCheckBox;
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

    bool mOutputDisabled=false;
};

#endif // OPTIMIZATIONDIALOG_H
