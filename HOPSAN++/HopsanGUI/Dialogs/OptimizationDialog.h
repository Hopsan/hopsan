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

#include "MainWindow.h"

class MainWindow;


class OptimizationDialog : public QWizard
{
    Q_OBJECT

public:
    OptimizationDialog(MainWindow *parent = 0);


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
    QWizardPage *mpSettingsWidget;
    QLabel *mpSettingsLabel;
    QGridLayout *mpSettingsLayout;
    QLabel *mpAlgorithmLabel;
    QComboBox *mpAlgorithmBox;
    QLabel *mpIterationsLabel;
    QSpinBox *mpIterationsSpinBox;
    QLabel *mpSearchPointsLabel;
    QSpinBox *mpSearchPointsSpinBox;
    QLabel *mpParticlesLabel;
    QSpinBox *mpParticlesSpinBox;
    QLabel *mpAlphaLabel;
    QLineEdit *mpAlphaLineEdit;
    QLabel *mpBetaLabel;
    QLineEdit *mpBetaLineEdit;
    QLabel *mpGammaLabel;
    QLineEdit *mpGammaLineEdit;
    QLabel *mpOmegaLabel;
    QLineEdit *mpOmegaLineEdit;
    QLabel *mpC1Label;
    QLineEdit *mpC1LineEdit;
    QLabel *mpC2Label;
    QLineEdit *mpC2LineEdit;
    QLabel *mpEpsilonFLabel;
    QLineEdit *mpEpsilonFLineEdit;
    QLabel *mpEpsilonXLabel;
    QLineEdit *mpEpsilonXLineEdit;
    QCheckBox *mpMultiThreadedCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QCheckBox *mpPlottingCheckBox;
    QCheckBox *mpExport2CSVBox;

    //Parameters page
    QWizardPage *mpParametersWidget;
    QLabel *mpParametersLabel;
    QCheckBox *mpParametersLogCheckBox;
    QTreeWidget *mpParametersList;
    QGridLayout *mpParametersLayout;
    QLabel *mpParameterMinLabel;
    QLabel *mpParameterNameLabel;
    QLabel *mpParameterMaxLabel;

    //Objective function page
    QWizardPage *mpObjectiveWidget;
    QComboBox *mpMinMaxComboBox;
    QComboBox *mpFunctionsComboBox;
    QLabel *mpObjectiveLabel;
    QTreeWidget *mpVariablesList;
    QPushButton *mpAddFunctionButton;
    QLabel *mpWeightLabel;
    QLabel *mpNormLabel;
    QLabel *mpExpLabel;
    QLabel *mpDescriptionLabel;
    QLabel *mpDataLabel;
    QList<QLineEdit*> mWeightLineEditPtrs;
    QList<QLineEdit*> mNormLineEditPtrs;
    QList<QLineEdit*> mExpLineEditPtrs;
    QList<QLabel*> mFunctionLabelPtrs;
    QList<QString> mFunctionName;
    QList<QWidget*> mDataWidgetPtrs;
    QList< QList<QLineEdit*> > mDataLineEditPtrs;
    QList<QToolButton*> mRemoveFunctionButtonPtrs;
    QGridLayout *mpObjectiveLayout;

    //Output page
    QWizardPage *mpOutputWidget;
    QGridLayout *mpOutputLayout;


    QStringList mObjectiveFunctionDescriptions;
    QStringList mObjectiveFunctionCalls;
    QList<int> mObjectiveFunctionNumberOfVariables;
    QList<bool> mObjectiveFunctionUsesTimeVector;
    QList<QStringList> mObjectiveFunctionDataLists;

    //Output
    QTextEdit *mpOutputBox;

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
