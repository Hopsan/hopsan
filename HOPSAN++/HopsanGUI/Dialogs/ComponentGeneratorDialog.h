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
//! @file   ComponentGeneratorDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the component generator dialog
//!
//$Id$

#ifndef COMPONENTGENERATORDIALOG_H_INCLUDED
#define COMPONENTGENERATORDIALOG_H_INCLUDED

#include <QDialog>

#include "MainWindow.h"
#include "Utilities/ComponentGeneratorUtilities.h"

class MainWindow;
class ModelObjectAppearance;
class QTextDocument;


class ModelicaHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ModelicaHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
};




class ComponentGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    ComponentGeneratorDialog(MainWindow *parent = 0);

public slots:
    virtual void open();

private slots:
    void autoResize();
    void update();
    void addPort();
    void addParameter();
    void addUtility();
    void addStaticVariable();
    void removePort();
    void removeParameter();
    void removeUtility();
    void removeStaticVariable();
    void updateValues();
    void updateGivenSoughtText();
    void updateBoundaryEquations();
    void updateRecentList();
    void removeRecentComponent();
    void loadRecentComponent();
    void compile();
    void loadFromModelica();
    void loadFromXml();
    void loadFromXml(QString fileName);
    void saveDialogToXml();
    void generateAppearance();
    void openAppearanceDialog();
    void togglePortsBox();
    void toggleParametersBox();
    void toggleUtilitiesBox();
    void toggleStaticVariablesBox();

private:
    void showOutputDialog(QStringList jacobian, QStringList equations, QStringList variables);

    //Initialization & equations text edits
    QLabel *mpGivenLabel;
    QLabel *mpSoughtLabel;
    QTabWidget *mpEquationTabs;
    QTabWidget *mpCodeTabs;
    QGridLayout *mpInitLayout;
    QWidget *mpInitWidget;
    QTextEdit *mpInitTextField;
    QGridLayout *mpSimulateLayout;
    QWidget *mpSimulateWidget;
    QTextEdit *mpSimulateTextField;
    QGridLayout *mpFinalizeLayout;
    QWidget *mpFinalizeWidget;
    QTextEdit *mpFinalizeTextField;
    QGridLayout *mpEquationsLayout;
    QTextEdit *mpInitAlgorithmsTextField;
    ModelicaHighlighter *mpInitAlgorithmsHighLighter;
    QWidget *mpInitAlgorithmsWidget;
    QGridLayout *mpInitAlgorithmsLayout;
    QTextEdit *mpFinalAlgorithmsTextField;
    ModelicaHighlighter *mpFinalAlgorithmsHighLighter;
    QWidget *mpFinalAlgorithmsWidget;
    QGridLayout *mpFinalAlgorithmsLayout;
    QWidget *mpEquationsWidget;
    QTextEdit *mpEquationsTextField;
    ModelicaHighlighter *mpEquationHighLighter;
    QLabel *mpBoundaryEquationsLabel;
    QTextEdit *mpBoundaryEquationsTextField;
    ModelicaHighlighter *mpBoundaryEquationHighLighter;
    QGridLayout *mpCodeLayout;
    QGroupBox *mpCodeGroupBox;


    //General Settings
    QLabel *mpRecentLabel;
    QComboBox *mpRecentComboBox;
    QPushButton *mpLoadRecentButton;
    QPushButton *mpRemoveRecentButton;
    QToolButton *mpLoadButton;
    QMenu *mpLoadMenu;
    QAction *mpLoadFromModelicaAction;
    QAction *mpLoadFromXmlAction;
    QToolButton *mpSaveButton;
    QLabel *mpGenerateFromLabel;
    QComboBox *mpGenerateFromComboBox;
    QLabel *mpComponentNameLabel;
    QLineEdit *mpComponentNameEdit;
    QLabel *mpComponentDisplayLabel;
    QLineEdit *mpComponentDisplayEdit;
    QLabel *mpComponentTypeLabel;
    QComboBox *mpComponentTypeComboBox;
    QToolButton *mpAddItemButton;
    QMenu *mpAddItemMenu;

    //Port Group Box
    QGroupBox *mpPortsGroupBox;
    QGridLayout *mpPortsLayout;
    QLabel *mpPortNamesLabel;
    QLabel *mpPortTypeLabel;
    QLabel *mpNodeTypelabel;
    QLabel *mpPortRequiredLabel;
    QLabel *mpPortDefaultLabel;
    QToolButton *mpAddPortButton;
    QVector<QLineEdit*> mvPortNameEdits;
    QVector<QComboBox*> mvPortTypeComboBoxes;
    QVector<QComboBox*> mvNodeTypeComboBoxes;
    QVector<QCheckBox*> mvRequiredCheckBoxes;
    QVector<QLineEdit*> mvPortDefaultEdits;
    QVector<QToolButton*> mvRemovePortButtons;

    //Parameter Group Box
    QGroupBox *mpParametersGroupBox;
    QGridLayout *mpParametersLayout;
    QLabel *mpParametersNameLabel;
    QLabel *mpParametersDisplayLabel;
    QLabel *mpParametersDescriptionLabel;
    QLabel *mpParametersUnitLabel;
    QLabel *mpParametersInitLabel;
    QToolButton *mpAddParameterButton;
    QVector<QLineEdit*> mvParameterNameEdits;
    QVector<QLineEdit*> mvParameterDisplayEdits;
    QVector<QLineEdit*> mvParameterDescriptionEdits;
    QVector<QLineEdit*> mvParameterUnitEdits;
    QVector<QLineEdit*> mvParameterInitEdits;
    QVector<QToolButton*> mvRemoveParameterButtons;
    QToolButton *mpPortsMinMaxButton;
    QToolButton *mpParametersMinMaxButton;
    QToolButton *mpUtilitiesMinMaxButton;
    QToolButton *mpStaticVariablesMinMaxButton;

    //Utilities Group Box
    QGroupBox *mpUtilitiesGroupBox;
    QGridLayout *mpUtilitiesLayout;
    QLabel *mpUtilitiesLabel;
    QLabel *mpUtilityNamesLabel;
    QToolButton *mpAddUtilityButton;
    QVector<QComboBox*> mvUtilitiesComboBoxes;
    QVector<QLineEdit*> mvUtilityNameEdits;
    QVector<QToolButton*> mvRemoveUtilityButtons;

    //Static Variables Group Box
    QGroupBox *mpStaticVariablesGroupBox;
    QGridLayout *mpStaticVariablesLayout;
    QLabel *mpStaticVariableNamesLabel;
    QToolButton *mpAddStaticVariableButton;
    QVector<QLineEdit*> mvStaticVariableNameEdits;
    QVector<QToolButton*> mvRemoveStaticVariableButtons;

    //SymPy Warning
    QLabel *mpSymPyWarning;

    //Buttons
    QPushButton *mpCancelButton;
    QPushButton *mpAppearanceButton;
    QPushButton *mpCompileButton;
    QDialogButtonBox *mpButtonBox;

    //Main layout
    QGridLayout *mpLayout;
    QWidget *mpCentralWidget;
    QScrollArea *mpScrollArea;
    QGridLayout *mpCentralLayout;

    //Member variables
    QList<PortSpecification> mPortList;
    QList<ParameterSpecification> mParametersList;
    QList<UtilitySpecification> mUtilitiesList;
    QList<StaticVariableSpecification> mStaticVariablesList;

    QStringList mRecentComponentFileNames;

    ModelObjectAppearance *mpAppearance;

    bool mPortsBoxVisible;
    bool mParametersBoxVisible;
    bool mUtilitiesBoxVisible;
    bool mStaticVariablesBoxVisible;
};

#endif // COMPONENTGENERATORDIALOG_H_INCLUDED
