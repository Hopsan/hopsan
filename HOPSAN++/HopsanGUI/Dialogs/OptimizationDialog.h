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

class OptimizationDialog : public QDialog
{
    Q_OBJECT

public:
    OptimizationDialog(MainWindow *parent = 0);

private:
    void generateScriptFile();
    bool verifyNumberOfVariables(int n);
    QString getFunctionCode(int i);
    QStringList getFunctionDataList(int i);
    bool verifyFunctionVariables(int i);

public slots:
    virtual void open();

private slots:
    void updateChosenParameters(QTreeWidgetItem* item, int i);
    void updateChosenVariables(QTreeWidgetItem* item, int i);
    void addFunction();
    void removeFunction();
    void updateOutputBox();
    void run();

private:
    //Settings tab
    QWidget *mpSettingsWidget;
    QGridLayout *mpSettingsLayout;
    QLabel *mpIterationsLabel;
    QSpinBox *mpIterationsSpinBox;
    QLabel *mpSearchPointsLabel;
    QSpinBox *mpSearchPointsSpinBox;
    QLabel *mpAlphaLabel;
    QDoubleSpinBox *mpAlphaSpinBox;
    QLabel *mpBetaLabel;
    QDoubleSpinBox *mpBetaSpinBox;
    QLabel *mpGammaLabel;
    QDoubleSpinBox *mpGammaSpinBox;
    QCheckBox *mpPlottingCheckBox;

    //Parameters tab
    QTreeWidget *mpParametersList;
    QGridLayout *mpParametersLayout;
    QWidget *mpParametersWidget;

    //Objective function tab
    QComboBox *mpFunctionsComboBox;
    QTreeWidget *mpVariablesList;
    QPushButton *mpAddFunctionButton;
    QList<QDoubleSpinBox*> mWeightSpinBoxPtrs;
    QList<QDoubleSpinBox*> mNormSpinBoxPtrs;
    QList<QDoubleSpinBox*> mExpSpinBoxPtrs;
    QList<QLabel*> mFunctionLabelPtrs;
    QList<QWidget*> mDataWidgetPtrs;
    QList< QList<QDoubleSpinBox*> > mDataSpinBoxPtrs;
    QList<QToolButton*> mRemoveFunctionButtonPtrs;
    QGridLayout *mpObjectiveLayout;
    QWidget *mpObjectiveWidget;

    //Tab widget
    QTabWidget *mpTabWidget;

    //Output
    QTextEdit *mpOutputBox;

    //Buttons
    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpGenerateButton;
    QPushButton *mpRunButton;
    QDialogButtonBox *mpButtonBox;

    //Main layout
    QGridLayout *mpLayout;

    //Member variables
    QString mScript;
    QStringList mFunctions;
    QList<int> mSelectedFunctions;
    QList<QStringList> mFunctionComponents;
    QList<QStringList> mFunctionPorts;
    QList<QStringList> mFunctionVariables;
    QList<double> mFunctionData;
    QStringList mSelectedComponents;
    QStringList mSelectedParameters;
    QList<QLabel*> mpParameterLabels;
    QList<QDoubleSpinBox*> mpParameterMinSpinBoxes;
    QList<QDoubleSpinBox*> mpParameterMaxSpinBoxes;
    QList<QStringList> mSelectedVariables;
};

#endif // OPTIMIZATIONDIALOG_H
