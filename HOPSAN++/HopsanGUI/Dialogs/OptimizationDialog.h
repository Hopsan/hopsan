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

class OptimizationDialog : public QWizard
{
    Q_OBJECT

    friend class OptimizationHandler;

public:
    OptimizationDialog(QWidget *parent = 0);
    TerminalWidget *mpTerminal;

private:
    void generateScriptFile();
    void generateComplexScript();
    void generateComplexScriptOld();
    void generateParticleSwarmScript();
    void generateParticleSwarmScriptOld();
    bool verifyNumberOfVariables(int i, int nSelVar);
    bool loadObjectiveFunctions();
    QString generateFunctionCode(int i);

    void loadConfiguration();
    void saveConfiguration();

    void addObjectiveFunction(int idx, double weight, double norm, double exp, QList<QStringList> selectedVariables, QStringList objData);

protected:
    QTreeWidgetItem* findParameterTreeItem(QString componentName, QString parameterName);

public slots:
    virtual void open();
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

private:
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
    QLineEdit *mpEpsilonFLineEdit;
    QLineEdit *mpEpsilonXLineEdit;
    QCheckBox *mpPlotParticlesCheckBox;
    QCheckBox *mpPlottingCheckBox;
    QCheckBox *mpPlotBestWorstCheckBox;
    QCheckBox *mpExport2CSVBox;

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

    //Run page
    QPushButton *mpStartButton;
    QTextEdit *mpParametersOutputTextEdit;
    QProgressBar *mpTotalProgressBar;

    //Toolbar
    QToolButton *mpHelpButton;

    //Member variables
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
};

#endif // OPTIMIZATIONDIALOG_H
