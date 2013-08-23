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
//#include "Utilities/ComponentGeneratorUtilities.h"  //Needed because we define lists with classes declared here

class MainWindow;
class ModelObjectAppearance;
class QTextDocument;


class ComponentGeneratorDialog : public QMainWindow
{
    Q_OBJECT
    friend class ComponentGeneratorWizard;

public:
    ComponentGeneratorDialog(MainWindow *parent = 0);

private slots:
    void addNewTab();
    void closeTab(int i);
    void tabChanged();
    void saveModel();
    void generateComponent();
    void openAppearanceDialog();
    void openComponentGeneratorWizard();
    void loadModel();
    void setModelicaHighlighter();
    void setCppHighlighter();
    void updateRecentList();
    void openRecentModel();

  private:
    void loadModel(QString modelFileName);
    void addNewTab(QString code, QString tabName="");

    //Initialization & equations text edits
    QTabWidget *mpEquationTabs;
    int mNumberOfUntitledTabs;
    QList<QFileInfo> mModelFiles;
    QList<bool> mHasChanged;
    QList<QGridLayout *> mEquationsLayoutPtrs;
    QList<QPlainTextEdit *> mEquationTextFieldPtrs;
    QList<QSyntaxHighlighter *> mEquationHighLighterPtrs;
    QList<QScrollArea *> mScrollAreaPtrs;

    QPushButton *mpGenerateTemplateButton;

    //Utilities Group Box
    QGroupBox *mpUtilitiesGroupBox;
    QGridLayout *mpUtilitiesLayout;
    QLabel *mpUtilitiesLabel;
    QLabel *mpUtilityNamesLabel;
    QToolButton *mpAddUtilityButton;
    QVector<QComboBox*> mvUtilitiesComboBoxes;
    QVector<QLineEdit*> mvUtilityNameEdits;
    QVector<QToolButton*> mvRemoveUtilityButtons;

    //Buttons
    QPushButton *mpCancelButton;
    QPushButton *mpAppearanceButton;
    QPushButton *mpCompileButton;
    QDialogButtonBox *mpButtonBox;

    //Main layout
    QGridLayout *mpLayout;
    QWidget *mpCentralWidget;

    QVBoxLayout *mpCentralLayout;

    //Actions
    QAction *mpNewAction;
    QAction *mpLoadAction;
    QAction *mpSaveAction;
    QAction *mpCloseAction;
    QAction *mpWizardAction;
    QAction *mpModelicaHighlighterAction;
    QAction *mpCppHighlighterAction;
    QAction *mpHelpAction;

    //Tool bar
    QToolBar *mpToolBar;

    //Menu bar
    QMenuBar *mpMenuBar;
    QMenu *mpFileMenu;
    QMenu *mpEditMenu;
    QMenu *mpHighlighterMenu;
    QMenu *mpRecentMenu;

    bool mPortsBoxVisible;
    bool mParametersBoxVisible;
    bool mUtilitiesBoxVisible;
    bool mStaticVariablesBoxVisible;
};


class ComponentGeneratorWizard : public QWizard
{
    Q_OBJECT

public:
    ComponentGeneratorWizard(ComponentGeneratorDialog *parent = 0);

private slots:
    void updatePage(int i);
    void generate();

private:
    ComponentGeneratorDialog *mpParent;

    QWizardPage *mpFirstPage;
    QGridLayout *mpFirstPageLayout;
    QLabel *mpTypeNameLabel;
    QLineEdit *mpTypeNameLineEdit;
    QLabel *mpDisplayNameLabel;
    QLineEdit *mpDisplayNameLineEdit;
    QLabel *mpCqsTypeLabel;
    QComboBox *mpCqsTypeComboBox;
    QLabel *mpNumberOfPortsLabel;
    QSpinBox *mpNumberOfPortsSpinBox;
    QLabel *mpNumberOfParametersLabel;
    QSpinBox *mpNumberOfParametersSpinBox;

    QWizardPage *mpSecondPage;
    QGridLayout *mpSecondPageLayout;
    QLabel *mpPortIdTitle;
    QLabel *mpPortNameTitle;
    QLabel *mpPortTypeTitle;
    QLabel *mpNodeTypeTitle;
    QLabel *mpDefaultValueTitle;
    QList<QLabel*> mPortIdPtrs;
    QList<QLineEdit*> mPortNameLineEditPtrs;
    QList<QComboBox*> mPortTypeComboBoxPtrs;
    QList<QDoubleSpinBox*> mPortDefaultSpinBoxPtrs;

    QWizardPage *mpThirdPage;
    QGridLayout *mpThirdPageLayout;
    QLabel *mpParameterNameTitle;
    QLabel *mpParameterUnitTitle;
    QLabel *mpParameterDescriptionTitle;
    QLabel *mpParameterValueTitle;
    QList<QLineEdit*> mParameterNameLineEditPtrs;
    QList<QLineEdit*> mParameterUnitLineEditPtrs;
    QList<QLineEdit*> mParameterDescriptionLineEditPtrs;
    QList<QLineEdit*> mParameterValueLineEditPtrs;

};

#endif // COMPONENTGENERATORDIALOG_H_INCLUDED
