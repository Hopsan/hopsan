/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 rediibuting any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ComponentGeneratorDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the component generator dialog
//!
//$Id$

#include <cassert>

#include <QFont>

#include "Configuration.h"
#include "GUIPort.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "Dialogs/MovePortsDialog.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Utilities/SymHop.h"
#include "Utilities/XMLUtilities.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/PyDockWidget.h"
#include "common.h"

using namespace SymHop;


ModelicaHighlighter::ModelicaHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns  << "\\bVariableLimits\\b" << "\\bVariable2Limits\\b";       //These are special and will not be treated as normal functions later on
    QStringList functions = getSupportedFunctionsList();
    for(int i=0; i<functions.size(); ++i)
    {
        keywordPatterns << "\\b"+functions[i]+"\\b";
    }

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}


void ModelicaHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}



//! @brief Constructor
ComponentGeneratorDialog::ComponentGeneratorDialog(MainWindow *parent)
    : QDialog(parent)
{
    Expression dummy = Expression("(1.0/-Z+1.0)/(1.0/Z+1.0)");
    dummy.expandParentheses();
    dummy.expandParentheses();
    //qDebug() << "Little test: " << dummy.toString();

    dummy = Expression("2.0*x+5.0*y*x-3.0*y/z+pow(x*z,3.0)");
    dummy.factor(Expression("x"));
    //qDebug() << "Factoring: " << dummy.toString();
    dummy.expand();
    //qDebug() << "Expanding: " << dummy.toString();
    dummy.linearize();
    //qDebug() << "Linearized: " << dummy.toString();
    assert(dummy.toString() == "z*5.0*y*x+z*2.0*x-3.0*y+pow(z,4.0)*pow(x,3.0)");

    dummy = Expression("pow(x,3.0)*pow(y,2.0)/x+pow(z,5.0)/pow(z,3.0)");
    //qDebug() << "Expand powers test expression: " << dummy.toString();
    dummy.expandPowers();
    //qDebug() << "Expanded: " << dummy.toString();
    assert(dummy.toString() == "x*x*y*y+z*z");

    dummy = Expression("x*y*(x-5.0*(2.0-z))/(2.0*(1.0-x)-5.0)");
    //qDebug() << "Expand parenthesis test expression: " << dummy.toString();
    dummy.expandParentheses();
    //qDebug() << "Expanded: " << dummy.toString();

    dummy = Expression("a*b*c + a*b*d + b*c*d");
    //qDebug() << "Factored most common factor test expression: " << dummy.toString();
    dummy.factorMostCommonFactor();
    //qDebug() << "Factored most common factor: " << dummy.toString();
    assert(dummy.toString() == "b*(a*c+a*d+c*d)");

    dummy = Expression("M*4*pow(1/-Z+1.0,2.0)*x1/(T*(1/Z+1.0)*T*(1/Z+1))+B*2/T*(1/-Z+1)/(1/Z+1)*x1+k*x1-F2-F1");
    //qDebug() << "Test: " << dummy.toString();
    dummy.linearize();
    //qDebug() << "Test finished: " << dummy.toString();

    mpAppearance = 0;

    //Set the name and size of the main window
    this->resize(640,480);
    this->setWindowTitle("Component Generator (experimental)");
    this->setPalette(gConfig.getPalette());
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setMaximumHeight(480);

    //Equation Text Field
    mpGivenLabel = new QLabel("Given: ");
    mpGivenLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpSoughtLabel = new QLabel("Sought: ");
    mpSoughtLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QFont monoFont = QFont("Monospace", 10, 50);
    monoFont.setStyleHint(QFont::Monospace);

    mpInitTextField = new QTextEdit(this);                                      //Initialize code text field
    mpInitTextField->setFont(monoFont);
    mpInitLayout = new QGridLayout(this);
    mpInitLayout->addWidget(mpInitTextField, 0, 0);
    mpInitWidget = new QWidget(this);
    mpInitWidget->setLayout(mpInitLayout);
    mpInitWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpSimulateTextField = new QTextEdit(this);                                  //SimulateOneTimeStep code text field
    mpSimulateTextField->setFont(monoFont);
    mpSimulateLayout = new QGridLayout(this);
    mpSimulateLayout->addWidget(mpSimulateTextField, 0, 0);
    mpSimulateWidget = new QWidget(this);
    mpSimulateWidget->setLayout(mpSimulateLayout);
    mpSimulateWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpFinalizeTextField = new QTextEdit(this);                                  //Finalize code text field
    mpFinalizeTextField->setFont(monoFont);
    mpFinalizeLayout = new QGridLayout(this);
    mpFinalizeLayout->addWidget(mpFinalizeTextField, 0, 0);
    mpFinalizeWidget = new QWidget(this);
    mpFinalizeWidget->setLayout(mpFinalizeLayout);
    mpFinalizeWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpEquationsTextField = new QTextEdit(this);                                 //Equation text field
    mpEquationsTextField->setFont(monoFont);
    mpEquationsTextField->setMaximumHeight(300);
    mpEquationHighLighter = new ModelicaHighlighter(mpEquationsTextField->document());
    mpBoundaryEquationsLabel = new QLabel("Boundary Equations:", this);
    mpBoundaryEquationsTextField = new QTextEdit(this);
    mpBoundaryEquationsTextField->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mpBoundaryEquationsTextField->setFont(monoFont);
    mpBoundaryEquationsTextField->setMaximumHeight(200);
    mpBoundaryEquationHighLighter = new ModelicaHighlighter(mpBoundaryEquationsTextField->document());
    mpEquationsLayout = new QGridLayout(this);
    mpEquationsLayout->addWidget(mpEquationsTextField, 0, 0);
    mpEquationsLayout->addWidget(mpBoundaryEquationsLabel, 1, 0);
    mpEquationsLayout->addWidget(mpBoundaryEquationsTextField, 2, 0);
    mpEquationsLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mpEquationsWidget = new QWidget(this);
    mpEquationsWidget->setLayout(mpEquationsLayout);
    mpEquationsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpInitAlgorithmsTextField = new QTextEdit(this);
    mpInitAlgorithmsTextField->setFont(monoFont);
    mpInitAlgorithmsTextField->setMaximumHeight(300);
    mpInitAlgorithmsHighLighter = new ModelicaHighlighter(mpInitAlgorithmsTextField->document());
    mpInitAlgorithmsLayout = new QGridLayout(this);
    mpInitAlgorithmsLayout->addWidget(mpInitAlgorithmsTextField);
    mpInitAlgorithmsWidget = new QWidget(this);
    mpInitAlgorithmsWidget->setLayout(mpInitAlgorithmsLayout);
    mpInitAlgorithmsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpFinalAlgorithmsTextField = new QTextEdit(this);
    mpFinalAlgorithmsTextField->setFont(monoFont);
    mpFinalAlgorithmsTextField->setMaximumHeight(300);
    mpFinalAlgorithmsHighLighter = new ModelicaHighlighter(mpFinalAlgorithmsTextField->document());
    mpFinalAlgorithmsLayout = new QGridLayout(this);
    mpFinalAlgorithmsLayout->addWidget(mpFinalAlgorithmsTextField);
    mpFinalAlgorithmsWidget = new QWidget(this);
    mpFinalAlgorithmsWidget->setLayout(mpFinalAlgorithmsLayout);
    mpFinalAlgorithmsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpCodeTabs = new QTabWidget(this);                                          //Tab layout
    mpCodeTabs->addTab(mpInitWidget, "Initialize");
    mpCodeTabs->addTab(mpSimulateWidget, "Simulate");
    mpCodeTabs->addTab(mpFinalizeWidget, "Finalize");
    mpCodeTabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpEquationTabs = new QTabWidget(this);
    mpEquationTabs->addTab(mpInitAlgorithmsWidget, "Initial Algorithms");
    mpEquationTabs->addTab(mpEquationsWidget, "Equations");
    mpEquationTabs->addTab(mpFinalAlgorithmsWidget, "Final Algorithms");
    mpEquationTabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpCodeLayout = new QGridLayout(this);
    mpCodeLayout->addWidget(mpGivenLabel, 0, 0);
    mpCodeLayout->addWidget(mpSoughtLabel, 1, 0);
    mpCodeLayout->addWidget(mpCodeTabs, 2, 0);
    mpCodeLayout->addWidget(mpEquationTabs, 2, 0);
    mpCodeGroupBox = new QGroupBox("Equations", this);
    mpCodeGroupBox->setLayout(mpCodeLayout);
    mpCodeGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    //Buttons
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpAppearanceButton = new QPushButton(tr("&Appearance"), this);
    mpAppearanceButton->setAutoDefault(false);
    mpCompileButton = new QPushButton(tr("&Compile"), this);
    mpCompileButton->setDefault(true);
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::RejectRole);
    mpButtonBox->addButton(mpAppearanceButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCompileButton, QDialogButtonBox::ActionRole);

    //General settings
    mpRecentLabel = new QLabel("Recent Models:");
    mpRecentComboBox = new QComboBox(this);
    mpLoadRecentButton = new QPushButton("Load Recent");
    mpRemoveRecentButton = new QPushButton("Remove");
    connect(mpRemoveRecentButton,       SIGNAL(pressed()), this, SLOT(removeRecentComponent()));
    connect(mpLoadRecentButton,         SIGNAL(pressed()), this, SLOT(loadRecentComponent()));

    mpLoadButton = new QToolButton(this);
    mpLoadButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Open.png"));
    QAction *mpLoadFromModelicaAction = new QAction(tr("&Load Modelica File (.mo)"), this);
    QAction *mpLoadFromXmlAction = new QAction(tr("&Load XML File"), this);
    connect(mpLoadFromModelicaAction,   SIGNAL(triggered()), this, SLOT(loadFromModelica()));
    connect(mpLoadFromXmlAction,        SIGNAL(triggered()), this, SLOT(loadFromXml()));
    mpLoadMenu = new QMenu(this);
    mpLoadMenu->addAction(mpLoadFromModelicaAction);
    mpLoadMenu->addAction(mpLoadFromXmlAction);
    mpLoadButton->setMenu(mpLoadMenu);
    mpLoadButton->setPopupMode(QToolButton::InstantPopup);
    mpLoadButton->setToolTip("Add Item");
    mpLoadButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpSaveButton = new QToolButton(this);
    mpSaveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Save.png"));
    mpSaveButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpGenerateFromLabel = new QLabel("Generate From: ");
    mpGenerateFromComboBox = new QComboBox(this);
    mpGenerateFromComboBox->addItems(QStringList() << "Equations" << "C++ Code");
    mpGenerateFromComboBox->setCurrentIndex(0);
    mpGenerateFromComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    connect(mpGenerateFromComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(update()));

    mpComponentNameLabel = new QLabel("Type Name: ");
    mpComponentNameEdit = new QLineEdit(this);
    mpComponentNameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpComponentNameEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpComponentDisplayLabel = new QLabel("Display Name: ");
    mpComponentDisplayLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpComponentDisplayEdit = new QLineEdit(this);
    mpComponentDisplayEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mpComponentTypeLabel = new QLabel("CQS Type: ");
    mpComponentTypeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpComponentTypeComboBox = new QComboBox(this);
    mpComponentTypeComboBox->addItems(QStringList() << "C" << "Q" << "S");
    mpComponentTypeComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    connect(mpComponentTypeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(update()));
    connect(mpComponentTypeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateValues()));

    mpAddItemButton = new QToolButton(this);
    mpAddItemButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
    mpAddItemButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpAddItemMenu = new QMenu(this);
    QAction *mpAddPortAction = new QAction(tr("&Add Port"), this);
    QAction *mpAddParameterAction = new QAction(tr("&Add Parameter"), this);
    QAction *mpAddUtilityAction = new QAction(tr("&Add Utility Function"), this);
    QAction *mpAddStaticVariableAction = new QAction(tr("&Add Static Variable"), this);
    connect(mpAddPortAction, SIGNAL(triggered()), this, SLOT(addPort()));
    connect(mpAddParameterAction, SIGNAL(triggered()), this, SLOT(addParameter()));
    connect(mpAddUtilityAction, SIGNAL(triggered()), this, SLOT(addUtility()));
    connect(mpAddStaticVariableAction, SIGNAL(triggered()), this, SLOT(addStaticVariable()));
    mpAddItemMenu->addAction(mpAddPortAction);
    mpAddItemMenu->addAction(mpAddParameterAction);
    mpAddItemMenu->addAction(mpAddUtilityAction);
    mpAddItemMenu->addAction(mpAddStaticVariableAction);
    mpAddItemButton->setMenu(mpAddItemMenu);
    mpAddItemButton->setPopupMode(QToolButton::InstantPopup);
    mpAddItemButton->setToolTip("Add Item");

    //Group boxes and layouts
    mpPortsGroupBox = new QGroupBox("Ports", this);                             //Ports layout
    mpPortsGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpPortsLayout = new QGridLayout();
    mpPortsGroupBox->setLayout(mpPortsLayout);
    mpPortNamesLabel = new QLabel("Name:", this);
    mpPortTypeLabel = new QLabel("Port Type:", this);
    mpNodeTypelabel = new QLabel("Node Type:", this);
    mpPortRequiredLabel = new QLabel("Required:", this);
    mpPortDefaultLabel = new QLabel("Default Value:", this);
    mpAddPortButton = new QToolButton(this);
    mpAddPortButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddPortButton->setToolTip("Add Port");
    connect(mpAddPortButton, SIGNAL(pressed()), this, SLOT(addPort()));
    mpPortsMinMaxButton = new QToolButton(this);
    mpPortsMinMaxButton->setFixedSize(15, 15);
    mpPortsMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    connect(mpPortsMinMaxButton, SIGNAL(pressed()), this, SLOT(togglePortsBox()));
    mPortsBoxVisible = true;

    mpParametersGroupBox = new QGroupBox("Parameters", this);                   //Parameters layout
    mpParametersGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpParametersLayout = new QGridLayout();
    mpParametersGroupBox->setLayout(mpParametersLayout);
    mpParametersNameLabel = new QLabel("Name:", this);
    mpParametersDisplayLabel = new QLabel("Display Name:", this);
    mpParametersDescriptionLabel = new QLabel("Description:", this);
    mpParametersUnitLabel = new QLabel("Unit:", this);
    mpParametersInitLabel = new QLabel("Default Value:", this);
    mpAddParameterButton = new QToolButton(this);
    mpAddParameterButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddParameterButton->setToolTip("Add Parameter");
    connect(mpAddParameterButton, SIGNAL(pressed()), this, SLOT(addParameter()));
    mpParametersMinMaxButton = new QToolButton(this);
    mpParametersMinMaxButton->setFixedSize(15, 15);
    mpParametersMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    connect(mpParametersMinMaxButton, SIGNAL(pressed()), this, SLOT(toggleParametersBox()));
    mParametersBoxVisible = true;

    mpUtilitiesGroupBox = new QGroupBox("Utilities", this);                     //Utilities layout
    mpUtilitiesGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpUtilitiesLayout = new QGridLayout();
    mpUtilitiesGroupBox->setLayout(mpUtilitiesLayout);
    mpUtilitiesLabel = new QLabel("Utility:", this);
    mpUtilityNamesLabel = new QLabel("Name:", this);
    mpAddUtilityButton = new QToolButton(this);
    mpAddUtilityButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddUtilityButton->setToolTip("Add Utility");
    connect(mpAddUtilityButton, SIGNAL(pressed()), this, SLOT(addUtility()));
    mpUtilitiesMinMaxButton = new QToolButton(this);
    mpUtilitiesMinMaxButton->setFixedSize(15, 15);
    mpUtilitiesMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    connect(mpUtilitiesMinMaxButton, SIGNAL(pressed()), this, SLOT(toggleUtilitiesBox()));
    mUtilitiesBoxVisible = true;

    mpStaticVariablesGroupBox = new QGroupBox("Static Variables", this);        //Static variables layout
    mpStaticVariablesGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpStaticVariablesLayout = new QGridLayout();
    mpStaticVariablesGroupBox->setLayout(mpStaticVariablesLayout);
    mpStaticVariableNamesLabel = new QLabel("Name:", this);
    mpAddStaticVariableButton = new QToolButton(this);
    mpAddStaticVariableButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddStaticVariableButton->setToolTip("Add Static Variable");
    connect(mpAddStaticVariableButton, SIGNAL(pressed()), this, SLOT(addStaticVariable()));
    mpStaticVariablesMinMaxButton = new QToolButton(this);
    mpStaticVariablesMinMaxButton->setFixedSize(15, 15);
    mpStaticVariablesMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    connect(mpStaticVariablesMinMaxButton, SIGNAL(pressed()), this, SLOT(toggleStaticVariablesBox()));
    mStaticVariablesBoxVisible = true;


    mpSymPyWarning = new QLabel(this);
    mpSymPyWarning->setText("<font color='red'>When compiling from equations, you need to have Python 2.6 or 2.7 with the SymPy package installed.</font>");
    mpSymPyWarning->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    //Main layout (create widgets, but don't do anything before contents are updated)
    mpLayout = new QGridLayout(this);
    mpCentralWidget = new QWidget(this);
    mpScrollArea = new QScrollArea(this);
    mpCentralLayout = new QGridLayout(this);

    update();
    updateValues();

    mpCentralWidget->setPalette(gConfig.getPalette());
    mpCentralWidget->setLayout(mpLayout);
    mpScrollArea->setPalette(gConfig.getPalette());
    mpScrollArea->setWidget(mpCentralWidget);
    mpScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpScrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mpScrollArea->setMaximumHeight(480);
    mpCentralLayout->addWidget(mpScrollArea);
    setLayout(mpCentralLayout);

    autoResize();

    //Connections
    connect(mpCancelButton,     SIGNAL(clicked()), this, SLOT(reject()));
    connect(mpCompileButton,    SIGNAL(clicked()), this, SLOT(generateComponent()));
    connect(mpAppearanceButton, SIGNAL(clicked()), this, SLOT(openAppearanceDialog()));
}


//! @brief Auto resizes the dialog to fit contents and/or screen geometry
void ComponentGeneratorDialog::autoResize()
{
    //mpCentralWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    mpLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    //mpCentralLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    int maxHeight = qApp->desktop()->screenGeometry().height()*0.7;
    mpScrollArea->setFixedHeight(std::min(mpCentralWidget->height()+30, maxHeight));
    if(mpScrollArea->verticalScrollBar()->isVisible())
    {
        mpScrollArea->setMinimumWidth(mpCentralWidget->width()+25);
    }
    else
    {
        mpScrollArea->setMinimumWidth(mpCentralWidget->width());
    }
    mpScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mpScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->resize(mpScrollArea->size());

    this->move(qApp->desktop()->screenGeometry().center() - this->rect().center());

    qDebug() << "mpCentralWidget.height() = " << mpCentralWidget->height();
    qDebug() << "mpScrollArea.height() = " << mpScrollArea->height();
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void ComponentGeneratorDialog::open()
{
    QDialog::open();
}


//! @brief Adds a new blank port
void ComponentGeneratorDialog::addPort()
{
    mPortList.append(PortSpecification());
    update();
    updateValues();
}


//! @brief Adds a new blank parameter
void ComponentGeneratorDialog::addParameter()
{
    mParametersList.append(ParameterSpecification());
    update();
}


//! @brief Adds a new blank utility
void ComponentGeneratorDialog::addUtility()
{
    mUtilitiesList.append(UtilitySpecification());
    update();
}


//! @brief Adds a new blank static variable
void ComponentGeneratorDialog::addStaticVariable()
{
    mStaticVariablesList.append(StaticVariableSpecification());
    update();
}


//! @brief Removes a port
//! Removes the port whos remove button emited the signal
void ComponentGeneratorDialog::removePort()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mvRemovePortButtons.indexOf(button);
    mPortList.removeAt(i);
    //qDebug() << "Removing port with index " << i;
    updateValues();
    update();
}


//! @brief Removes a parameter
//! Removes the parameter whos remove button emited the signal
void ComponentGeneratorDialog::removeParameter()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mvRemoveParameterButtons.indexOf(button);
    mParametersList.removeAt(i);
    update();
}


//! @brief Removes a utility
//! Removes the utility whos remove button emited the signal
void ComponentGeneratorDialog::removeUtility()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mvRemoveUtilityButtons.indexOf(button);
    mUtilitiesList.removeAt(i);
    update();
}


//! @brief Removes a static variable
//! Removes the static variable whos remove button emited the signal
void ComponentGeneratorDialog::removeStaticVariable()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mvRemoveStaticVariableButtons.indexOf(button);
    mStaticVariablesList.removeAt(i);
    update();
}


//! @brief Slots that updates values in member variables from values in the boxes
//! Identifies the sender widget to avoid updating everything all the time
void ComponentGeneratorDialog::updateValues()
{
    //qDebug() << "updateValues()";

    //Assume sender is a line edit
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());

    if(mvPortNameEdits.contains(lineEdit))
    {
        int i = mvPortNameEdits.indexOf(lineEdit);
        mPortList[i].name = lineEdit->text();
        updateGivenSoughtText();
        updateBoundaryEquations();
        return;
    }
    else if(mvPortDefaultEdits.contains(lineEdit))
    {
        int i = mvPortDefaultEdits.indexOf(lineEdit);
        mPortList[i].defaultvalue = lineEdit->text();
    }
    else if(mvParameterNameEdits.contains(lineEdit))
    {
        int i = mvParameterNameEdits.indexOf(lineEdit);
        mParametersList[i].name = lineEdit->text();
        updateGivenSoughtText();
        return;
    }
    else if(mvParameterDisplayEdits.contains(lineEdit))
    {
        int i = mvParameterDisplayEdits.indexOf(lineEdit);
        mParametersList[i].displayName = lineEdit->text();
    }
    else if(mvParameterDescriptionEdits.contains(lineEdit))
    {
        int i = mvParameterDescriptionEdits.indexOf(lineEdit);
        mParametersList[i].description = lineEdit->text();
    }
    else if(mvParameterUnitEdits.contains(lineEdit))
    {
        int i = mvParameterUnitEdits.indexOf(lineEdit);
        mParametersList[i].unit = lineEdit->text();
    }
    else if(mvParameterInitEdits.contains(lineEdit))
    {
        int i = mvParameterInitEdits.indexOf(lineEdit);
        mParametersList[i].init = lineEdit->text();
    }
    else if(mvUtilityNameEdits.contains(lineEdit))
    {
        int i = mvUtilityNameEdits.indexOf(lineEdit);
        mUtilitiesList[i].name = lineEdit->text();
    }
    else if(mvStaticVariableNameEdits.contains(lineEdit))
    {
        int i = mvStaticVariableNameEdits.indexOf(lineEdit);
        mStaticVariablesList[i].name = lineEdit->text();
    }

    //Assume sender is a check box
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
    if(mvRequiredCheckBoxes.contains(checkBox))
    {
        int i = mvRequiredCheckBoxes.indexOf(checkBox);
        mPortList[i].notrequired = !checkBox->isChecked();
    }

    //Assume sender is a combo box
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if(mvPortTypeComboBoxes.contains(comboBox))
    {
        int i = mvPortTypeComboBoxes.indexOf(comboBox);
        mPortList[i].porttype = comboBox->currentText();
        updateGivenSoughtText();
        updateBoundaryEquations();
        return;
    }
    else if(mvNodeTypeComboBoxes.contains(comboBox))
    {
        int i = mvNodeTypeComboBoxes.indexOf(comboBox);
        mPortList[i].nodetype = comboBox->currentText();
        updateGivenSoughtText();
        updateBoundaryEquations();
        return;
    }
    else if(mvUtilitiesComboBoxes.contains(comboBox))
    {
        int i = mvUtilitiesComboBoxes.indexOf(comboBox);
        mUtilitiesList[i].utility = comboBox->currentText();
    }
}


//! @brief Updates the widgets from data in the member variables
void ComponentGeneratorDialog::update()
{
    if(mpGenerateFromComboBox->currentIndex() == 0)
    {
        mpCodeTabs->hide();
        mpEquationTabs->show();
        //qDebug() << "CQSType = " << mpComponentTypeComboBox->currentText();
        mpBoundaryEquationsTextField->setVisible(!mpBoundaryEquationsTextField->toPlainText().isEmpty() && mpComponentTypeComboBox->currentText() == "Q");
        mpBoundaryEquationsLabel->setVisible(mpBoundaryEquationsTextField->isVisible());
    }
    else
    {
        mpCodeTabs->show();
        mpEquationTabs->hide();
    }


    QLayoutItem *pChild;
    while((pChild = mpPortsLayout->takeAt(0)))
    {
        delete(pChild);
    }

    int row=0;

    mpPortsLayout->addWidget(mpPortNamesLabel, row, 0);
    mpPortsLayout->addWidget(mpPortTypeLabel, row, 1);
    mpPortsLayout->addWidget(mpNodeTypelabel, row, 2);
    mpPortsLayout->addWidget(mpPortRequiredLabel, row, 3);
    mpPortsLayout->addWidget(mpPortDefaultLabel, row, 4);
    mpPortsLayout->addWidget(mpAddPortButton, row, 5);

    ++row;

    for(int i=0; i<mvPortNameEdits.size(); ++i)
        delete(mvPortNameEdits.at(i));
    for(int i=0; i<mvPortTypeComboBoxes.size(); ++i)
        delete(mvPortTypeComboBoxes.at(i));
    for(int i=0; i<mvNodeTypeComboBoxes.size(); ++i)
        delete(mvNodeTypeComboBoxes.at(i));
    for(int i=0; i<mvRequiredCheckBoxes.size(); ++i)
        delete(mvRequiredCheckBoxes.at(i));
    for(int i=0; i<mvPortDefaultEdits.size(); ++i)
        delete(mvPortDefaultEdits.at(i));
    for(int i=0; i<mvRemovePortButtons.size(); ++i)
        delete(mvRemovePortButtons.at(i));
    mvPortNameEdits.clear();
    mvPortTypeComboBoxes.clear();
    mvNodeTypeComboBoxes.clear();
    mvRequiredCheckBoxes.clear();
    mvPortDefaultEdits.clear();
    mvRemovePortButtons.clear();

    if(!mPortList.isEmpty() && mPortsBoxVisible)
    {
        for(int i=0; i<mPortList.size(); ++i)
        {
            QLineEdit *pPortNameEdit = new QLineEdit(mPortList.at(i).name, this);
            QComboBox *pPortTypeComboBox = new QComboBox(this);
            QStringList portTypes = QStringList() << "ReadPort" << "WritePort" << "PowerPort";
            pPortTypeComboBox->addItems(portTypes);
            pPortTypeComboBox->setCurrentIndex(portTypes.indexOf(mPortList.at(i).porttype));
            QComboBox *pNodeTypeComboBox = new QComboBox(this);
            QStringList nodeTypes = QStringList() << "NodeSignal" << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic" << "NodePneumatic" << "NodeElectric";
            pNodeTypeComboBox->addItems(nodeTypes);
            pNodeTypeComboBox->setCurrentIndex(nodeTypes.indexOf(mPortList.at(i).nodetype));
            QCheckBox *pRequiredCheckBox = new QCheckBox(this);
            pRequiredCheckBox->setChecked(!mPortList.at(i).notrequired);
            QLineEdit *pDefaultEdit = new QLineEdit(mPortList.at(i).defaultvalue, this);
            QToolButton *pRemoveButton = new QToolButton(this);
            pRemoveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));

            connect(pPortNameEdit,      SIGNAL(textChanged(QString)),           this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pPortTypeComboBox,  SIGNAL(currentIndexChanged(QString)),   this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pNodeTypeComboBox,  SIGNAL(currentIndexChanged(QString)),   this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pRequiredCheckBox,  SIGNAL(toggled(bool)),                  this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pDefaultEdit,       SIGNAL(textChanged(QString)),           this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pRemoveButton,      SIGNAL(pressed()),                      this, SLOT(removePort()), Qt::UniqueConnection);

            mpPortsLayout->addWidget(pPortNameEdit, row, 0);
            mpPortsLayout->addWidget(pPortTypeComboBox, row, 1);
            mpPortsLayout->addWidget(pNodeTypeComboBox, row, 2);
            mpPortsLayout->addWidget(pRequiredCheckBox, row, 3);
            mpPortsLayout->addWidget(pDefaultEdit, row, 4);
            mpPortsLayout->addWidget(pRemoveButton, row, 5);
            mpPortsLayout->setAlignment(pRequiredCheckBox, Qt::AlignCenter);

            mvPortNameEdits.append(pPortNameEdit);
            mvPortTypeComboBoxes.append(pPortTypeComboBox);
            mvNodeTypeComboBoxes.append(pNodeTypeComboBox);
            mvRequiredCheckBoxes.append(pRequiredCheckBox);
            mvPortDefaultEdits.append(pDefaultEdit);
            mvRemovePortButtons.append(pRemoveButton);

            ++row;
        }
    }
    mpPortNamesLabel->setVisible(!mPortList.isEmpty() && mPortsBoxVisible);
    mpPortTypeLabel->setVisible(!mPortList.isEmpty() && mPortsBoxVisible);
    mpNodeTypelabel->setVisible(!mPortList.isEmpty() && mPortsBoxVisible);
    mpPortRequiredLabel->setVisible(!mPortList.isEmpty() && mPortsBoxVisible);
    mpPortDefaultLabel->setVisible(!mPortList.isEmpty() && mPortsBoxVisible);
    mpAddPortButton->setVisible(!mPortList.isEmpty() && mPortsBoxVisible);
    mpPortsGroupBox->setVisible(!mPortList.isEmpty());
    mpPortsMinMaxButton->setVisible(!mPortList.isEmpty());


    while(!mpParametersLayout->isEmpty())
    {
        delete(mpParametersLayout->itemAt(0));
        mpParametersLayout->removeItem(mpParametersLayout->itemAt(0));
    }

    row=0;

    mpParametersLayout->addWidget(mpParametersNameLabel, row, 0);
    mpParametersLayout->addWidget(mpParametersDisplayLabel, row, 1);
    mpParametersLayout->addWidget(mpParametersDescriptionLabel, row, 2);
    mpParametersLayout->addWidget(mpParametersUnitLabel, row, 3);
    mpParametersLayout->addWidget(mpParametersInitLabel, row, 4);
    mpParametersLayout->addWidget(mpAddParameterButton, row, 5);

    ++row;

    for(int i=0; i<mvParameterNameEdits.size(); ++i)
        delete(mvParameterNameEdits.at(i));
    for(int i=0; i<mvParameterDisplayEdits.size(); ++i)
        delete(mvParameterDisplayEdits.at(i));
    for(int i=0; i<mvParameterDescriptionEdits.size(); ++i)
        delete(mvParameterDescriptionEdits.at(i));
    for(int i=0; i<mvParameterUnitEdits.size(); ++i)
        delete(mvParameterUnitEdits.at(i));
    for(int i=0; i<mvParameterInitEdits.size(); ++i)
        delete(mvParameterInitEdits.at(i));
    for(int i=0; i<mvRemoveParameterButtons.size(); ++i)
        delete(mvRemoveParameterButtons.at(i));
    mvParameterNameEdits.clear();
    mvParameterDisplayEdits.clear();
    mvParameterDescriptionEdits.clear();
    mvParameterUnitEdits.clear();
    mvParameterInitEdits.clear();
    mvRemoveParameterButtons.clear();

    if(!mParametersList.isEmpty() &&  mParametersBoxVisible)
    {
        for(int i=0; i<mParametersList.size(); ++i)
        {
            QLineEdit *pParameterNameEdit = new QLineEdit(mParametersList.at(i).name, this);
            QLineEdit *pParameterDisplayEdit = new QLineEdit(mParametersList.at(i).displayName, this);
            QLineEdit *pParameterDescriptionEdit = new QLineEdit(mParametersList.at(i).description, this);
            QLineEdit *pParameterUnitEdit = new QLineEdit(mParametersList.at(i).unit, this);
            QLineEdit *pParameterInitEdit = new QLineEdit(mParametersList.at(i).init, this);
            QToolButton *pRemoveButton = new QToolButton(this);
            pRemoveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));

            connect(pParameterNameEdit,         SIGNAL(textChanged(QString)),  this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pParameterDisplayEdit,      SIGNAL(textChanged(QString)),  this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pParameterDescriptionEdit,  SIGNAL(textChanged(QString)),  this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pParameterUnitEdit,         SIGNAL(textChanged(QString)),  this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pParameterInitEdit,         SIGNAL(textChanged(QString)),  this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pRemoveButton,              SIGNAL(pressed()),          this, SLOT(removeParameter()), Qt::UniqueConnection);

            mpParametersLayout->addWidget(pParameterNameEdit, row, 0);
            mpParametersLayout->addWidget(pParameterDisplayEdit, row, 1);
            mpParametersLayout->addWidget(pParameterDescriptionEdit, row, 2);
            mpParametersLayout->addWidget(pParameterUnitEdit, row, 3);
            mpParametersLayout->addWidget(pParameterInitEdit, row, 4);
            mpParametersLayout->addWidget(pRemoveButton, row, 5);

            mvParameterNameEdits.append(pParameterNameEdit);
            mvParameterDisplayEdits.append(pParameterDisplayEdit);
            mvParameterDescriptionEdits.append(pParameterDescriptionEdit);
            mvParameterUnitEdits.append(pParameterUnitEdit);
            mvParameterInitEdits.append(pParameterInitEdit);
            mvRemoveParameterButtons.append(pRemoveButton);

            ++row;
        }
    }

    mpParametersNameLabel->setVisible(!mParametersList.isEmpty() && mParametersBoxVisible);
    mpParametersDisplayLabel->setVisible(!mParametersList.isEmpty() && mParametersBoxVisible);
    mpParametersDescriptionLabel->setVisible(!mParametersList.isEmpty() && mParametersBoxVisible);
    mpParametersUnitLabel->setVisible(!mParametersList.isEmpty() && mParametersBoxVisible);;
    mpParametersInitLabel->setVisible(!mParametersList.isEmpty() && mParametersBoxVisible);
    mpAddParameterButton->setVisible(!mParametersList.isEmpty() && mParametersBoxVisible);
    mpParametersGroupBox->setVisible(!mParametersList.isEmpty());
    mpParametersMinMaxButton->setVisible(!mParametersList.isEmpty());

    while(!mpUtilitiesLayout->isEmpty())
    {
        delete(mpUtilitiesLayout->itemAt(0));
        mpUtilitiesLayout->removeItem(mpUtilitiesLayout->itemAt(0));
    }

    row=0;

    mpUtilitiesLayout->addWidget(mpUtilitiesLabel, row, 0);
    mpUtilitiesLayout->addWidget(mpUtilityNamesLabel, row, 1);
    mpUtilitiesLayout->addWidget(mpAddUtilityButton, row, 2);

    ++row;

    for(int i=0; i<mvUtilitiesComboBoxes.size(); ++i)
        delete(mvUtilitiesComboBoxes.at(i));
    for(int i=0; i<mvUtilityNameEdits.size(); ++i)
        delete(mvUtilityNameEdits.at(i));
    for(int i=0; i<mvRemoveUtilityButtons.size(); ++i)
        delete(mvRemoveUtilityButtons.at(i));
    mvUtilitiesComboBoxes.clear();
    mvUtilityNameEdits.clear();
    mvRemoveUtilityButtons.clear();

    if(!mUtilitiesList.isEmpty() && mUtilitiesBoxVisible)
    {
        for(int i=0; i<mUtilitiesList.size(); ++i)
        {
            QComboBox *pUtilitiesComboBox = new QComboBox(this);
            QStringList utilities = QStringList() << "CSVParser" << "Delay" << "DoubleIntegratorWithDamping" << "DoubleIntegratorWithDampingAndCoulumbFriction" << "FirstOrderTransferFunction" << "Integrator" << "IntegratorLimited" << "Matrix" << "SecondOrderTransferFunction" << "TurbulentFlowFunction" << "ValveHysteresis" << "Vec" << "WhiteGaussianNoise";
            pUtilitiesComboBox->addItems(utilities);
            pUtilitiesComboBox->setCurrentIndex(utilities.indexOf(mUtilitiesList.at(i).utility));
            QLineEdit *pUtilityNameEdit = new QLineEdit(mUtilitiesList.at(i).name, this);
            QToolButton *pRemoveButton = new QToolButton(this);
            pRemoveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));

            connect(pUtilitiesComboBox,         SIGNAL(currentIndexChanged(QString)),   this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pUtilityNameEdit,           SIGNAL(textChanged(QString)),           this, SLOT(updateValues()), Qt::UniqueConnection);
            connect(pRemoveButton,              SIGNAL(pressed()),                      this, SLOT(removeUtility()), Qt::UniqueConnection);

            mpUtilitiesLayout->addWidget(pUtilitiesComboBox, row, 0);
            mpUtilitiesLayout->addWidget(pUtilityNameEdit, row, 1);
            mpUtilitiesLayout->addWidget(pRemoveButton, row, 2);

            mvUtilitiesComboBoxes.append(pUtilitiesComboBox);
            mvUtilityNameEdits.append(pUtilityNameEdit);
            mvRemoveUtilityButtons.append(pRemoveButton);

            ++row;
        }
    }

    mpUtilitiesLabel->setVisible(!mUtilitiesList.isEmpty() && mUtilitiesBoxVisible);
    mpUtilityNamesLabel->setVisible(!mUtilitiesList.isEmpty() && mUtilitiesBoxVisible);
    mpAddUtilityButton->setVisible(!mUtilitiesList.isEmpty() && mUtilitiesBoxVisible);
    mpUtilitiesGroupBox->setVisible(!mUtilitiesList.isEmpty());
    mpUtilitiesMinMaxButton->setVisible(!mUtilitiesList.isEmpty());


    while(!mpStaticVariablesLayout->isEmpty())
    {
        delete(mpStaticVariablesLayout->itemAt(0));
        mpStaticVariablesLayout->removeItem(mpStaticVariablesLayout->itemAt(0));
    }

    row=0;

    mpStaticVariablesLayout->addWidget(mpStaticVariableNamesLabel, row, 0);
    mpStaticVariablesLayout->addWidget(mpAddStaticVariableButton, row, 1);

    ++row;

    for(int i=0; i<mvStaticVariableNameEdits.size(); ++i)
        delete(mvStaticVariableNameEdits.at(i));
    for(int i=0; i<mvRemoveStaticVariableButtons.size(); ++i)
        delete(mvRemoveStaticVariableButtons.at(i));
    mvStaticVariableNameEdits.clear();
    mvRemoveStaticVariableButtons.clear();

    if(!mStaticVariablesList.isEmpty() && mStaticVariablesBoxVisible)
    {
        for(int i=0; i<mStaticVariablesList.size(); ++i)
        {
            QLineEdit *pStaticVariableNameEdit = new QLineEdit(mStaticVariablesList.at(i).name, this);
            QToolButton *pRemoveButton = new QToolButton(this);
            pRemoveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));

            connect(pStaticVariableNameEdit,    SIGNAL(textChanged(QString)),   this, SLOT(updateValues()),         Qt::UniqueConnection);
            connect(pRemoveButton,              SIGNAL(pressed()),              this, SLOT(removeStaticVariable()), Qt::UniqueConnection);

            mpStaticVariablesLayout->addWidget(pStaticVariableNameEdit, row, 0);
            mpStaticVariablesLayout->addWidget(pRemoveButton, row, 1);

            mvStaticVariableNameEdits.append(pStaticVariableNameEdit);
            mvRemoveStaticVariableButtons.append(pRemoveButton);

            ++row;
        }
    }
    mpStaticVariableNamesLabel->setVisible(!mStaticVariablesList.isEmpty() && mStaticVariablesBoxVisible);
    mpAddStaticVariableButton->setVisible(!mStaticVariablesList.isEmpty() && mStaticVariablesBoxVisible);
    mpStaticVariablesGroupBox->setVisible(!mStaticVariablesList.isEmpty());
    mpStaticVariablesMinMaxButton->setVisible(!mStaticVariablesList.isEmpty());

    //Warn about SymPy when compiling from equations
    mpSymPyWarning->setVisible(mpGenerateFromComboBox->currentIndex() == 0);

    while(!mpLayout->isEmpty())
    {
        delete(mpLayout->itemAt(0));
        mpLayout->removeItem(mpLayout->itemAt(0));
    }

    QHBoxLayout *pRecentLayout = new QHBoxLayout(this);
    pRecentLayout->addWidget(mpRecentLabel);
    pRecentLayout->addWidget(mpRecentComboBox);
    pRecentLayout->addWidget(mpLoadRecentButton);
    pRecentLayout->addWidget(mpRemoveRecentButton);
    pRecentLayout->setStretch(1, 1);
    pRecentLayout->setSizeConstraint(QLayout::SetMinimumSize);
    QWidget *pRecentWidget = new QWidget(this);
    pRecentWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    pRecentWidget->setLayout(pRecentLayout);

    mpLayout->addWidget(pRecentWidget,                      0, 0, 1, 11);
    mpLayout->addWidget(mpLoadButton,                       1, 0);
    mpLayout->addWidget(mpSaveButton,                       1, 1);
    mpLayout->addWidget(mpGenerateFromLabel,                1, 2);
    mpLayout->addWidget(mpGenerateFromComboBox,             1, 3);
    mpLayout->addWidget(mpComponentNameLabel,               1, 4);
    mpLayout->addWidget(mpComponentNameEdit,                1, 5);
    mpLayout->addWidget(mpComponentDisplayLabel,            1, 6);
    mpLayout->addWidget(mpComponentDisplayEdit,             1, 7);
    mpLayout->addWidget(mpComponentTypeLabel,               1, 8);
    mpLayout->addWidget(mpComponentTypeComboBox,            1, 9);
    mpLayout->addWidget(mpAddItemButton,                    1, 10);
    mpLayout->addWidget(mpPortsGroupBox,                    2, 0, 1, 11);
    mpLayout->addWidget(mpPortsMinMaxButton,                2, 0, 1, 1);
    mpLayout->setAlignment(mpPortsMinMaxButton, Qt::AlignTop);
    mpLayout->addWidget(mpParametersGroupBox,               3, 0, 1, 11);
    mpLayout->addWidget(mpParametersMinMaxButton,           3, 0, 1, 1);
    mpLayout->setAlignment(mpParametersMinMaxButton, Qt::AlignTop);
    mpLayout->addWidget(mpUtilitiesGroupBox,                4, 0, 1, 11);
    mpLayout->addWidget(mpUtilitiesMinMaxButton,            4, 0, 1, 1);
    mpLayout->setAlignment(mpUtilitiesMinMaxButton, Qt::AlignTop);
    mpLayout->addWidget(mpStaticVariablesGroupBox,          5, 0, 1, 11);
    mpLayout->addWidget(mpStaticVariablesMinMaxButton,      5, 0, 1, 1);
    mpLayout->setAlignment(mpStaticVariablesMinMaxButton, Qt::AlignTop);
    mpLayout->addWidget(mpSymPyWarning,                     6, 0, 1, 11);
    mpLayout->addWidget(mpCodeGroupBox,                     7, 0, 1, 11);
    mpLayout->addWidget(mpButtonBox,                        8, 0, 1, 11);
    mpLayout->setSizeConstraint(QLayout::SetMinimumSize);
    //mpLayout->setRowStretch(6, 1);

    updateGivenSoughtText();
    updateBoundaryEquations();
    updateRecentList();

    autoResize();
}


//! @brief Help function that updates "given variables" and "sought variables" label
void ComponentGeneratorDialog::updateGivenSoughtText()
{
    QString cqs = mpComponentTypeComboBox->currentText();
    QString givenText = "Given: ";
    QString soughtText = "Sought: ";
    for(int i=0; i<mParametersList.size(); ++i)
    {
        givenText.append(mParametersList[i].name + ", ");
    }
    for(int i=0; i<mPortList.size(); ++i)
    {
        QString num = QString().setNum(i+1);
        if(mPortList[i].porttype == "ReadPort")
        {
            givenText.append(mPortList[i].name + ", ");
        }
        else if(mPortList[i].porttype == "WritePort")
        {
            soughtText.append(mPortList[i].name + ", ");
        }
        else if(mPortList[i].porttype == "PowerPort" && cqs == "C")
        {
            QStringList qVars = getQVariables(mPortList[i].nodetype);
            QStringList cVars = getCVariables(mPortList[i].nodetype);
            for(int v=0; v<qVars.size(); ++v)
            {
                givenText.append(qVars[v]+num+", ");
            }
            for(int v=0; v<cVars.size(); ++v)
            {
                soughtText.append(cVars[v]+num+", ");
            }
        }
        else if(mPortList[i].porttype == "PowerPort" && cqs == "Q")
        {
            QStringList qVars = getQVariables(mPortList[i].nodetype);
            QStringList cVars = getCVariables(mPortList[i].nodetype);
            for(int v=0; v<qVars.size(); ++v)
            {
                soughtText.append(qVars[v]+num+", ");
            }
            for(int v=0; v<cVars.size(); ++v)
            {
                givenText.append(cVars[v]+num+", ");
            }
        }
    }
    givenText.chop(2);
    soughtText.chop(2);
    mpGivenLabel->setText(givenText);
    mpSoughtLabel->setText(soughtText);
}


//! @brief Help function that updates boundary equations box
void ComponentGeneratorDialog::updateBoundaryEquations()
{
    mpBoundaryEquationsTextField->clear();
    int i=1;
    for(int j=0; j<mPortList.size(); ++j)
    {
        QStringList qVars = getQVariables(mPortList[j].nodetype);
        QStringList cVars = getCVariables(mPortList[j].nodetype);

        if(!qVars.isEmpty() && !cVars.isEmpty())
        {
            QString iStr = QString().setNum(i);
            mpBoundaryEquationsTextField->append(qVars.first()+iStr+" = "+cVars.first()+iStr+" + "+cVars.last()+iStr+"*"+qVars.last()+iStr);
        }
        ++i;
    }
    mpBoundaryEquationsTextField->setVisible(!mpBoundaryEquationsTextField->toPlainText().isEmpty() && mpComponentTypeComboBox->currentText() == "Q");
    mpBoundaryEquationsLabel->setVisible(mpBoundaryEquationsTextField->isVisible());
    mpBoundaryEquationsTextField->setFixedHeight(15*i);
}


void ComponentGeneratorDialog::updateRecentList()
{
    mpRecentComboBox->clear();

    QString dirString = QString(DATAPATH)+"compgen/";
    QDir dir(dirString);

    QStringList filters;
    filters << "*.xml";                     //Create the name filter
    dir.setFilter(QDir::NoFilter); //Clear all filters
    dir.setNameFilters(filters);   //Set the name filter

    QStringList xmlList  = dir.entryList();    //Create a list with all name of the files in dir libDir
    for (int i=0; i<xmlList.size(); ++i)        //Iterate over the file names
    {
        QString filename = dirString + "/" + xmlList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file or not a text file: " + filename);
            continue;
        }

        QDomDocument domDocument;        //Read appearance from file, First check if xml
        QString errorStr;
        int errorLine, errorColumn;
        if (domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            QDomElement xmlRoot = domDocument.documentElement();
            if (xmlRoot.tagName() != "hopsancomponent") //! @todo Hard coded xml tag
            {
                gpMainWindow->mpMessageWidget->printGUIDebugMessage(file.fileName() + ": The file is not an Hopsan Component Data file. Incorrect root tag name: " + xmlRoot.tagName() + " != hopsancomponent");
                continue;
            }
            else
            {
                mpRecentComboBox->addItem(xmlRoot.attribute("displayname"));
                mRecentComponentFileNames.append(filename);
            }
        }
        file.close();
    }
}


void ComponentGeneratorDialog::removeRecentComponent()
{
    int index = mpRecentComboBox->currentIndex();
    QString fileName = mRecentComponentFileNames.at(index);
    QFile().remove(fileName);
    mRecentComponentFileNames.removeAt(index);
    updateRecentList();
    mpRecentComboBox->setCurrentIndex(std::min(index, mpRecentComboBox->count()-1));
}


void ComponentGeneratorDialog::loadRecentComponent()
{
    QString fileName = mRecentComponentFileNames.at(mpRecentComboBox->currentIndex());

    if(!fileName.isEmpty())
    {
        //qDebug() << "Trying to open: " << fileName;
        loadFromXml(fileName);
    }
}


//! @brief Generates XML and compiles the new component
void ComponentGeneratorDialog::generateComponent()
{
    QProgressDialog *pProgressBar = new QProgressDialog(tr("Verifying"), QString(), 0, 0, gpMainWindow);
    pProgressBar->setWindowFlags(Qt::Window);
    pProgressBar->setWindowModality(Qt::ApplicationModal);
    pProgressBar->setWindowTitle(tr("Generating Hopsan Component"));
    pProgressBar->setMinimum(0);
    pProgressBar->setMaximum(100);
    pProgressBar->setValue(0);
    pProgressBar->setMinimumDuration(0);
    pProgressBar->show();

    //Verify parameters
    if(!verifyParameteres(mParametersList))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Verification of parameters failed.");
        delete(pProgressBar);
        return;
    }

                pProgressBar->setValue(1);

    //Verify ports
    if(!verifyPorts(mPortList))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Verification of ports failed.");
        delete(pProgressBar);
        return;
    }

    //Verify utilities
    if(!verifyUtilities(mUtilitiesList))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Verification of utilities failed.");
        delete(pProgressBar);
        return;
    }

                pProgressBar->setValue(2);

    //Verify static variables
    if(!verifyStaticVariables(mStaticVariablesList))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Verification of static variables failed.");
        delete(pProgressBar);
        return;
    }

    if(mpGenerateFromComboBox->currentIndex() == 0)         //Compile from equations
    {
        //Save dialog to XML file
        saveDialogToXml();

                pProgressBar->setLabelText("Collecting equations");
                pProgressBar->setValue(5);

        //Create list of initial algorithms
        QString plainInitAlgorithms = mpInitAlgorithmsTextField->toPlainText();
        QStringList initAlgorithms = plainInitAlgorithms.split("\n");
        initAlgorithms.removeAll("");

        QList<Expression> initAlgorithmExpressions;
        for(int i=0; i<initAlgorithms.size(); ++i)
        {
            initAlgorithmExpressions.append(Expression(initAlgorithms.at(i)));
        }

                pProgressBar->setValue(3);

        //Create list of equqtions
        QString plainEquations = mpEquationsTextField->toPlainText();
        QStringList equations = plainEquations.split("\n");
        if(mpComponentTypeComboBox->currentText() == "Q")
        {
            QString plainBoundaryEquations = mpBoundaryEquationsTextField->toPlainText();
            QStringList boundaryEquations = plainBoundaryEquations.split("\n");
            equations.append(boundaryEquations);
        }
        equations.removeAll("");

        QList<Expression> equationExpressions;
        for(int i=0; i<equations.size(); ++i)
        {
            equationExpressions.append(Expression(equations.at(i)));
        }

        //Create list of final algorithms
        QString plainFinalAlgorithms = mpFinalAlgorithmsTextField->toPlainText();
        QStringList finalAlgorithms = plainFinalAlgorithms.split("\n");
        finalAlgorithms.removeAll("");

        QList<Expression> finalAlgorithmExpressions;
        for(int i=0; i<finalAlgorithms.size(); ++i)
        {
            finalAlgorithmExpressions.append(Expression(finalAlgorithms.at(i)));
        }

                pProgressBar->setValue(4);

        //Identify variable limitations, and remove them from the equations list
        QList<Expression> limitedVariables;
        QList<Expression> limitedDerivatives;
        QList<Expression> limitMinValues;
        QList<Expression> limitMaxValues;
        QList<int> limitedVariableEquations;
        QList<int> limitedDerivativeEquations;
        for(int i=0; i<equationExpressions.size(); ++i)
        {
            if(equationExpressions[i].getFunctionName() == "VariableLimits")
            {
                if(i<1)
                {
                    //! @todo Use sorting instead?
                    gpMainWindow->mpMessageWidget->printGUIErrorMessage("VariableLimits not preceeded by equations defining variable.");
                    return;
                }

                limitedVariables << equationExpressions[i].getArgument(0);
                limitedDerivatives << Expression();
                limitMinValues << equationExpressions[i].getArgument(1);
                limitMaxValues << equationExpressions[i].getArgument(2);
                limitedVariableEquations << i-1;
                limitedDerivativeEquations << -1;

                equationExpressions.removeAt(i);
                --i;
            }
            else if(equationExpressions[i].getFunctionName()== "Variable2Limits")
            {
                if(i<2)
                {
                    gpMainWindow->mpMessageWidget->printGUIErrorMessage("Variable2Limits not preeded by equations defining variable and derivative.");
                    return;
                }

                limitedVariables << equationExpressions[i].getArgument(0);
                limitedDerivatives << equationExpressions[i].getArgument(1);
                limitMinValues << equationExpressions[i].getArgument(2);
                limitMaxValues << equationExpressions[i].getArgument(3);
                limitedVariableEquations << i-2;
                limitedDerivativeEquations << i-1;

                //qDebug() << "Found limitation of variable " << limitedVariables.last() << " with derivative " << limitedDerivatives.last();

                equationExpressions.removeAt(i);
                --i;
            }
            pProgressBar->setValue(4+4*double(i+1)/double(equationExpressions.size()));
        }

                pProgressBar->setValue(8);

        //Verify each equation
        for(int i=0; i<equationExpressions.size(); ++i)
        {
            if(!equationExpressions[i].verifyExpression())
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Component generation failed: Verification of variables failed.");
                return;
            }
        }

        QList<QList<Expression> > leftSymbols2, rightSymbols2;
        for(int i=0; i<equationExpressions.size(); ++i)
        {
            leftSymbols2.append(equationExpressions[i].getChild(0).getSymbols());
            rightSymbols2.append(equationExpressions[i].getChild(1).getSymbols());
        }
        for(int i=0; i<leftSymbols2.size(); ++i)
        {
            for(int j=0; j<leftSymbols2[i].size(); ++j)
                qDebug() << "Left symbol ("+QString::number(i)+"): " << leftSymbols2[i][j].toString();
        }
        for(int i=0; i<rightSymbols2.size(); ++i)
        {
            for(int j=0; j<rightSymbols2[i].size(); ++j)
                qDebug() << "Right symbol ("+QString::number(i)+"): " << rightSymbols2[i][j].toString();
        }

                pProgressBar->setValue(9);

        //Sum up all used variables to a single list
        QList<Expression> allSymbols;
        for(int i=0; i<equationExpressions.size(); ++i)
        {
            allSymbols.append(leftSymbols2.at(i));
            allSymbols.append(rightSymbols2.at(i));
        }

        QList<Expression> initSymbols2;
        for(int i=0; i<initAlgorithmExpressions.size(); ++i)
        {
            if(!initAlgorithmExpressions[i].isAssignment())
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Component generation failed: Initial algorithms section contains non-algorithms.");
                return;
            }
            initSymbols2.append(initAlgorithmExpressions[i].getChild(0));
        }
        for(int i=0; i<initSymbols2.size(); ++i)
        {
            qDebug() << "Init symbol: " << initSymbols2[i].toString();
        }

                pProgressBar->setValue(10);

        QList<Expression> finalSymbols2;
        for(int i=0; i<finalAlgorithmExpressions.size(); ++i)
        {
            //! @todo We must check that all algorithms are actually algorithms before doing this!
            if(!finalAlgorithmExpressions[i].isAssignment())
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Component generation failed: Final algorithms section contains non-algorithms.");
                return;
            }
            finalSymbols2.append(finalAlgorithmExpressions[i].getChild(0));
        }
        for(int i=0; i<finalSymbols2.size(); ++i)
        {
            qDebug() << "Final symbol: " << finalSymbols2[i].toString();
        }

        for(int i=0; i<mParametersList.size(); ++i)
        {
            allSymbols.append(Expression(mParametersList[i].name));
        }
        allSymbols.append(initSymbols2);
        allSymbols.append(finalSymbols2);
        removeDuplicates(allSymbols);

                pProgressBar->setValue(11);

        //Generate a list of state variables (= "output" variables & local variables)
        QList<Expression> nonStateVars;

        for(int i=0; i<mPortList.size(); ++i)
        {
            QString num = QString().setNum(i+1);
            if(mPortList[i].porttype == "ReadPort")
            {
                nonStateVars.append(Expression(mPortList[i].name));
            }
            else if(mPortList[i].porttype == "PowerPort" && mpComponentTypeComboBox->currentText() == "C")
            {
                QStringList qVars;
                qVars << getQVariables(mPortList[i].nodetype);
                for(int v=0; v<qVars.size(); ++v)
                {
                    nonStateVars.append(Expression(qVars[v]+num));
                }
            }
            else if(mPortList[i].porttype == "PowerPort" && mpComponentTypeComboBox->currentText() == "Q")
            {
                QStringList cVars;
                cVars << getCVariables(mPortList[i].nodetype);
                for(int v=0; v<cVars.size(); ++v)
                {
                    nonStateVars.append(Expression(cVars[v]+num));
                }
            }
        }

        for(int i=0; i<mParametersList.size(); ++i)
        {
            nonStateVars.append(Expression(mParametersList[i].name));
        }
        for(int i=0; i<initSymbols2.size(); ++i)
        {
            nonStateVars.append(initSymbols2[i]);
        }
        for(int i=0; i<finalSymbols2.size(); ++i)
        {
            nonStateVars.append(finalSymbols2[i]);
        }

        QList<Expression> stateVars = allSymbols;
        for(int i=0; i<nonStateVars.size(); ++i)
        {
            stateVars.removeAll(nonStateVars[i]);
        }

                pProgressBar->setValue(12);

        //Verify equation system
        if(!verifyEquationSystem(equationExpressions, stateVars))
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Verification of equation system failed.");
            delete(pProgressBar);
            return;
        }

        //Sort equation system so that each equation contains its corresponding state variable
        QList<QList<Expression> > symbols = leftSymbols2;
        for(int i=0; i<leftSymbols2.size(); ++i)
        {
            symbols[i].append(rightSymbols2[i]);
        }

                pProgressBar->setValue(13);

        if(!sortEquationSystem(equationExpressions, symbols, stateVars, limitedVariableEquations, limitedDerivativeEquations))
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not sort equations. System is probably under-determined.");
            delete(pProgressBar);
            return;
        }

        //Generate list of local variables (variables that are neither input nor output)
        QList<Expression> nonLocals;

        for(int i=0; i<mPortList.size(); ++i)
        {
            QString num = QString().setNum(i+1);
            if(mPortList[i].porttype == "ReadPort" || mPortList[i].porttype == "WritePort")
            {
                nonLocals.append(Expression(mPortList[i].name));     //Remove all readport/writeport varibles
            }
            else if(mPortList[i].porttype == "PowerPort")
            {
                QStringList qVars;
                QStringList cVars;
                qVars << getQVariables(mPortList[i].nodetype);
                cVars << getCVariables(mPortList[i].nodetype);
                for(int v=0; v<qVars.size(); ++v)
                {
                    nonLocals.append(Expression(qVars[v]+num));      //Remove all Q-type variables
                }
                for(int v=0; v<cVars.size(); ++v)
                {
                    nonLocals.append(Expression(cVars[v]+num));      //Remove all C-type variables
                }
            }
        }
        for(int i=0; i<mParametersList.size(); ++i)
        {
            nonLocals.append(Expression(mParametersList[i].name));   //Remove all parameters
        }

                pProgressBar->setValue(14);

        QList<Expression> localVars = allSymbols;
        for(int i=0; i<nonLocals.size(); ++i)
        {
            localVars.removeAll(nonLocals[i]);
        }
        for(int i=0; i<localVars.size(); ++i)
        {
            allSymbols.removeAll(localVars[i]);
        }

        for(int i=0; i<equationExpressions.size(); ++i)
        {
            equationExpressions[i].toLeftSided();
            equationExpressions[i].replaceBy(equationExpressions[i].getChild(0));
            qDebug() << "Left sided:  " << equationExpressions[i].toString();
        }

                pProgressBar->setValue(15);

        for(int i=0; i<equationExpressions.size(); ++i)
        {
            equationExpressions[i] = equationExpressions[i].bilinearTransform();
            qDebug() << "Transformed:  " << equationExpressions[i].toString();
        }

        for(int i=0; i<limitedVariableEquations.size(); ++i)
        {
            equationExpressions[limitedVariableEquations[i]].factor(limitedVariables[i]);

            QString num = equationExpressions[limitedVariableEquations[i]].getChild(1).toString();
            QString den = equationExpressions[limitedVariableEquations[i]].getChild(0).getChild(1).toString();
            equationExpressions[limitedVariableEquations[i]] = Expression(QStringList() << limitedVariables[i].toString() << "-" << "limit((-"+num+")/("+den+"))");

            qDebug() << "Limited: " << equationExpressions[limitedVariableEquations[i]].toString();


            if(!limitedDerivatives[i].toString().isEmpty())      //Variable2Limits (has a derivative)
            {
                equationExpressions[limitedDerivativeEquations[i]].factor(limitedDerivatives[i]);

                QString num = equationExpressions[limitedDerivativeEquations[i]].getChild(1).toString();
                QString den = equationExpressions[limitedDerivativeEquations[i]].getChild(0).getChild(1).toString();
                equationExpressions[limitedDerivativeEquations[i]] = Expression(QStringList() << limitedDerivatives[i].toString() << "-" << "dxLimit((-"+num+")/("+den+"))");

                qDebug() << "Limited: " << equationExpressions[limitedDerivativeEquations[i]].toString();
            }

            pProgressBar->setValue(16+5*double(i+1)/double(limitedVariableEquations.size()));
        }

        for(int e=0; e<equationExpressions.size(); ++e)
        {
            equationExpressions[e].linearize();
            equationExpressions[e].expandPowers();

            qDebug() << "Linearized: " << equationExpressions[e].toString();
        }

               pProgressBar->setValue(21);

        QList<QList<Expression> > jacobian;
        for(int e=0; e<equationExpressions.size(); ++e)
        {
            //Remove all delay operators, since they shall not be in the Jacobian anyway
            Expression tempExpr = equationExpressions[e];

            qDebug() << "Before replace: " << tempExpr.toString();

            tempExpr.replace(Expression("Z", Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));
            tempExpr.replace(Expression("-Z",Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));

            tempExpr._simplify(Expression::SimplifyWithoutMakingPowers, Expression::Recursive);

            qDebug() << "After replace: " << tempExpr.toString();

            //First differentiate the diagonal element and divide the equation with it, to get only ones on the diagonal later
            bool ok = true;
            Expression div = tempExpr.derivative(stateVars[e], ok);
            if(!(div == Expression("0.0",Expression::NoSimplifications)))
            {
                QList<Expression> terms = equationExpressions[e].getTerms();
                for(int t=0; t<terms.size(); ++t)
                {
                    terms[t] = Expression(terms[t], "/", div, Expression::SimplifyWithoutMakingPowers);
                }
                equationExpressions[e] = Expression(terms, "+", Expression::SimplifyWithoutMakingPowers);

                terms = tempExpr.getTerms();
                for(int t=0; t<terms.size(); ++t)
                {
                    terms[t] = Expression(terms[t], "/", div, Expression::SimplifyWithoutMakingPowers);
                }
                tempExpr = Expression(terms, "+", Expression::SimplifyWithoutMakingPowers);
            }

            //Now differentiate all jacobian elements
            jacobian.append(QList<Expression>());
            for(int j=0; j<stateVars.size(); ++j)
            {
                ok = true;
                tempExpr.replace(Expression(stateVars[j].negative()), Expression(stateVars[j], "*", Expression("-1.0", Expression::NoSimplifications), Expression::NoSimplifications));
                jacobian[e].append(tempExpr.derivative(stateVars[j], ok));
                qDebug() << "\nDifferentiating:\n" << tempExpr.toString() << "\n" << stateVars[j].toString() << "\n=\n" << jacobian[e].last().toString() << "\n";
                if(!ok)
                {
                    gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to differentiate expression: " + equationExpressions[e].toString() + " for variable " + stateVars[j].toString());
                    return;
                }
            }
                    pProgressBar->setValue(22+double(e)/double(equationExpressions.size())*34);
        }


        QList<Expression> delayTerms;
        QStringList delaySteps;
        for(int e=0; e<equationExpressions.size(); ++e)
        {
            equationExpressions[e].expand();
            qDebug() << "Before delay transform: " << equationExpressions[e].toString();
            equationExpressions[e].toDelayForm(delayTerms, delaySteps);
            qDebug() << "After delay transform: " << equationExpressions[e].toString();
        }
        for(int d=0; d<delayTerms.size(); ++d)
        {
            qDebug() << "DELAY TERM: " << delayTerms[d].toString();
            qDebug() << "DELAY STEP: " << delaySteps[d];
        }

                pProgressBar->setValue(56);

        //Fetch general component data
        QString typeName = mpComponentNameEdit->text();
        QString displayName = mpComponentDisplayEdit->text();
        QString cqsType = mpComponentTypeComboBox->currentText();

                pProgressBar->setLabelText("Compiling component");

        //Generate appearance object
        generateAppearance();

                pProgressBar->setValue(57);

        //Display equation system dialog
        showOutputDialog(jacobian, equationExpressions, stateVars);

        //Call utility to generate and compile the source code
        generateComponentObject(typeName, displayName, cqsType, mPortList, mParametersList, equationExpressions, stateVars, jacobian, delayTerms, delaySteps, localVars, initAlgorithmExpressions, finalAlgorithmExpressions, mpAppearance, pProgressBar);

//        //Delete the progress bar to avoid memory leaks
        delete(pProgressBar);
    }
    else if(mpGenerateFromComboBox->currentIndex() == 1)        //Compile from C++ code
    {
        //Generate a DOM document from member variables

        //qDebug() << "Compiling C++";

        saveDialogToXml();

        QDomDocument domDocument;
        QDomElement componentRoot = domDocument.createElement("hopsancomponent");
        componentRoot.setAttribute("typename", mpComponentNameEdit->text());
        componentRoot.setAttribute("displayname", mpComponentDisplayEdit->text());
        componentRoot.setAttribute("cqstype", mpComponentTypeComboBox->currentText());
        domDocument.appendChild(componentRoot);

        QDomElement portsElement = appendDomElement(componentRoot,"ports");
        for(int i=0; i<mPortList.size(); ++i)
        {
            QDomElement portElement = appendDomElement(portsElement,"port");
            portElement.setAttribute("name", mPortList.at(i).name);
            portElement.setAttribute("type", mPortList.at(i).porttype);
            portElement.setAttribute("nodetype", mPortList.at(i).nodetype);
            if(mPortList.at(i).notrequired)
                portElement.setAttribute("notrequired", "True");
            else
                portElement.setAttribute("notrequired", "False");
            portElement.setAttribute("default", mPortList.at(i).defaultvalue);
        }

        QDomElement parametersElement = appendDomElement(componentRoot,"parameters");
        for(int i=0; i<mParametersList.size(); ++i)
        {
            QDomElement parElement = appendDomElement(parametersElement,"parameter");
            parElement.setAttribute("name", mParametersList.at(i).name);
            parElement.setAttribute("displayname", mParametersList.at(i).displayName);
            parElement.setAttribute("description", mParametersList.at(i).description);
            parElement.setAttribute("unit", mParametersList.at(i).unit);
            parElement.setAttribute("init", mParametersList.at(i).init);
        }

        QDomElement utilitiesElement = appendDomElement(componentRoot,"utilities");
        for(int i=0; i<mUtilitiesList.size(); ++i)
        {
            QDomElement utilityElement = appendDomElement(utilitiesElement,"utility");
            utilityElement.setAttribute("utility", mUtilitiesList[i].utility);
            utilityElement.setAttribute("name", mUtilitiesList[i].name);
        }

        QDomElement variablesElement = appendDomElement(componentRoot,"staticvariables");
        for(int i=0; i<mStaticVariablesList.size(); ++i)
        {
            QDomElement variableElement = appendDomElement(variablesElement,"staticvariable");
            variableElement.setAttribute("name", mStaticVariablesList[i].name);
            variableElement.setAttribute("datatype", "double");
        }

        QDomElement initializeElement = appendDomElement(componentRoot,"initialize");
        QString plainInitEquations = mpInitTextField->toPlainText();
        QStringList initEquations = plainInitEquations.split("\n");
        for(int i=0; i<initEquations.size(); ++i)
        {
            appendDomTextNode(initializeElement,"equation",initEquations.at(i));
        }

        QDomElement simulateElement = appendDomElement(componentRoot,"simulate");
        QString plainEquations = mpSimulateTextField->toPlainText();
        QStringList equations = plainEquations.split("\n");
        for(int i=0; i<equations.size(); ++i)
        {
            appendDomTextNode(simulateElement,"equation",equations.at(i));
        }

        QDomElement finalizeElement = appendDomElement(componentRoot,"finalize");
        QString plainFinalEquations = mpFinalizeTextField->toPlainText();
        QStringList finalEquations = plainFinalEquations.split("\n");
        for(int i=0; i<finalEquations.size(); ++i)
        {
            appendDomTextNode(finalizeElement,"equation",finalEquations.at(i));
        }

        appendRootXMLProcessingInstruction(domDocument);

        //Make sure appearance is updated
        generateAppearance();

        //Call utility to generate and compile source code
        generateComponentObject("temp.hpp", componentRoot, mpAppearance);
    }
}


void ComponentGeneratorDialog::showOutputDialog(QList<QList<Expression> > jacobian, QList<Expression> equations, QList<Expression> variables)
{
    QDialog *pDialog = new QDialog(this);

    //Description label
    QLabel *pDescription = new QLabel("Resulting equations:", this);

    //Jacobian matrix
    QLabel *pJacobianLabel = new QLabel("J", this);
    pJacobianLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *pJacobianLayout = new QGridLayout(this);
    for(int i=0; i<equations.size(); ++i)
    {
        for(int j=0; j<equations.size(); ++j)
        {
            QString element = jacobian[i][j].toString();
            QString shortElement = element;
            shortElement.truncate(17);
            if(shortElement<element)
            {
                shortElement.append("...");
            }
            QLabel *pElement = new QLabel(shortElement, this);
            pElement->setToolTip(element);
            pElement->setAlignment(Qt::AlignCenter);
            pJacobianLayout->addWidget(pElement, j, i+1);
        }
    }
    QGroupBox *pJacobianBox = new QGroupBox(this);
    pJacobianBox->setLayout(pJacobianLayout);

    //Variables vector
    QLabel *pVariablesLabel = new QLabel("x", this);
    pVariablesLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *pVariablesLayout = new QGridLayout(this);
    for(int i=0; i<variables.size(); ++i)
    {
        QLabel *pElement = new QLabel(variables[i].toString(), this);
        pElement->setAlignment(Qt::AlignCenter);
        pVariablesLayout->addWidget(pElement, i, 0);
    }
    QGroupBox *pVariablesBox = new QGroupBox(this);
    pVariablesBox->setLayout(pVariablesLayout);

    //Equality sign
    QLabel *pEqualityLabel = new QLabel("=", this);
    pEqualityLabel->setAlignment(Qt::AlignCenter);

    //Equations vector
    QLabel *pEquationsLabel = new QLabel("b", this);
    pEquationsLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *pEquationsLayout = new QGridLayout(this);
    for(int i=0; i<equations.size(); ++i)
    {
        QString element = equations[i].toString();
        QString shortElement = element;
        shortElement.truncate(17);
        if(shortElement<element)
        {
            shortElement.append("...");
        }
        QLabel *pElement = new QLabel(shortElement, this);
        pElement->setToolTip(element);
        pElement->setAlignment(Qt::AlignCenter);
        pEquationsLayout->addWidget(pElement, i, 0);
    }
    QGroupBox *pEquationsBox = new QGroupBox(this);
    pEquationsBox->setLayout(pEquationsLayout);

    QPushButton *pOkButton = new QPushButton("Okay", this);
    QDialogButtonBox *pButtonGroup = new QDialogButtonBox(pDialog);
    pButtonGroup->addButton(pOkButton, QDialogButtonBox::AcceptRole);
    connect(pOkButton, SIGNAL(pressed()), pDialog, SLOT(close()));

    QGridLayout *pDialogLayout = new QGridLayout(this);
    pDialogLayout->addWidget(pDescription,      0, 0);
    pDialogLayout->addWidget(pJacobianLabel,    1, 0);
    pDialogLayout->addWidget(pJacobianBox,      2, 0);
    pDialogLayout->addWidget(pVariablesLabel,   1, 1);
    pDialogLayout->addWidget(pVariablesBox,     2, 1);
    pDialogLayout->addWidget(pEqualityLabel,    2, 2);
    pDialogLayout->addWidget(pEquationsLabel,   1, 3);
    pDialogLayout->addWidget(pEquationsBox,     2, 3);
    pDialogLayout->addWidget(pButtonGroup,      3, 0, 1, 4);

    pDialog->setLayout(pDialogLayout);

    pDialog->show();
}


//! @brief Loads a model from a Modelica file
void ComponentGeneratorDialog::loadFromModelica()
{
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Modlica File"),
                                                         gConfig.getModelicaModelsDir(),
                                                         tr("Modelica File (*.mo)"));
    if(modelFileName.isEmpty())
    {
        return;
    }

    QFile file(modelFileName);
    QFileInfo fileInfo(file);
    gConfig.setModelicaModelsDir(fileInfo.absolutePath());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"), "Unable to read Modelica file.");
        return;
    }

    QString code;
    QTextStream t(&file);
    code = t.readAll();
    file.close();

    mPortList.clear();              //! @todo Make a clear() member function that wraps this
    mParametersList.clear();
    mStaticVariablesList.clear();
    mUtilitiesList.clear();

    //Parse the file
    QString typeName, displayName, cqsType="Q";
    QStringList initAlgorithms;
    QStringList equations;
    QStringList finalAlgorithms;
    parseModelicaModel(code, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, mPortList, mParametersList);

    //Update everything
    update();
    updateValues();
    mpComponentNameEdit->setText(typeName);
    mpComponentDisplayEdit->setText(displayName);
    mpInitAlgorithmsTextField->clear();
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        mpInitAlgorithmsTextField->append(initAlgorithms.at(i));
    }
    mpEquationsTextField->clear();
    for(int i=0; i<equations.size(); ++i)
    {
        mpEquationsTextField->append(equations.at(i));
    }
    mpFinalAlgorithmsTextField->clear();
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        mpFinalAlgorithmsTextField->append(finalAlgorithms.at(i));
    }

    QStringList dummyCqs;               //Set correct cqs type
    dummyCqs << "C" << "Q" << "S";
    mpComponentTypeComboBox->setCurrentIndex(dummyCqs.indexOf(cqsType));
}


//! @brief Loads a model from XML code
//! @todo Finish implementation
void ComponentGeneratorDialog::loadFromXml()
{
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose XML File"),
                                                         gExecPath+"/",
                                                         tr("XML File (*.xml)"));
    if(modelFileName.isEmpty())
    {
        return;
    }

    loadFromXml(modelFileName);
}


void ComponentGeneratorDialog::loadFromXml(QString fileName)
{
    QFile file(fileName);
    //qDebug() << "Reading config from " << file.fileName();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"), "Unable to read XML file.");
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"),
                                 file.fileName()+gpMainWindow->tr(": Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return;
    }

    QDomElement modelRoot = domDocument.documentElement();

    mpComponentNameEdit->setText(modelRoot.attribute("typename"));
    mpComponentDisplayEdit->setText(modelRoot.attribute("displayname"));
    QStringList dummyCqs;
    dummyCqs << "C" << "Q" << "S";
    mpComponentTypeComboBox->setCurrentIndex(dummyCqs.indexOf(modelRoot.attribute("cqstype")));

    mUtilitiesList.clear();
    QDomElement utilitiesElement = modelRoot.firstChildElement("utilities");
    QDomElement utilityElement = utilitiesElement.firstChildElement("utility");
    while(!utilityElement.isNull())
    {
        UtilitySpecification u;
        u.utility = utilityElement.attribute ("utility");
        u.name = utilityElement.attribute("name");
        mUtilitiesList << u;
        utilityElement=utilityElement.nextSiblingElement("utility");
    }

    mParametersList.clear();
    QDomElement parametersElement = modelRoot.firstChildElement("parameters");
    QDomElement parameterElement = parametersElement.firstChildElement("parameter");
    while(!parameterElement.isNull())
    {
        ParameterSpecification p;
        p.name = parameterElement.attribute("name");
        p.init = parameterElement.attribute("init");
        p.displayName = parameterElement.attribute("displayname");
        p.description = parameterElement.attribute("description");
        p.unit = parameterElement.attribute("unit");
        mParametersList << p;
        //qDebug() << "Appended a parameter!";
        parameterElement=parameterElement.nextSiblingElement("parameter");
    }

    mStaticVariablesList.clear();
    QDomElement variablesElemenet = modelRoot.firstChildElement("staticvariables");
    QDomElement variableElement = variablesElemenet.firstChildElement("staticvariable");
    while(!variableElement.isNull())
    {
        StaticVariableSpecification s;
        s.name = variableElement.attribute("name");
        s.datatype = variableElement.attribute("datatype");
        mStaticVariablesList << s;
        variableElement=variableElement.nextSiblingElement("staticvariable");
    }

    mPortList.clear();
    QDomElement portsElement = modelRoot.firstChildElement("ports");
    QDomElement portElement = portsElement.firstChildElement("port");
    while(!portElement.isNull())
    {
        PortSpecification p;
        p.name = portElement.attribute("name");
        p.porttype = portElement.attribute("type");
        p.nodetype = portElement.attribute("nodetype");
        p.defaultvalue = portElement.attribute("default");
        p.notrequired = (portElement.attribute("notrequired") == "True");
        mPortList << p;
        portElement=portElement.nextSiblingElement("port");
    }

    mpInitTextField->clear();
    QDomElement initializeElement = modelRoot.firstChildElement("initialize");
    QDomElement initEquationElement = initializeElement.firstChildElement("equation");
    while(!initEquationElement.isNull())
    {
        mpInitTextField->append(initEquationElement.text());
        initEquationElement=initEquationElement.nextSiblingElement("equation");
    }

    mpSimulateTextField->clear();
    QDomElement simulateElement = modelRoot.firstChildElement("simulate");
    QDomElement simEquationElement = simulateElement.firstChildElement("equation");
    while(!simEquationElement.isNull())
    {
        mpSimulateTextField->append(simEquationElement.text());
        simEquationElement=simEquationElement.nextSiblingElement("equation");
    }

    mpFinalizeTextField->clear();
    QDomElement finalizeElement = modelRoot.firstChildElement("finalize");
    QDomElement finalEquationElement = finalizeElement.firstChildElement("equation");
    while(!finalEquationElement.isNull())
    {
        mpFinalizeTextField->append(finalEquationElement.text());
        finalEquationElement=finalEquationElement.nextSiblingElement("equation");
    }

    bool useEquations=false;
    mpEquationsTextField->clear();
    QDomElement equationsElement = modelRoot.firstChildElement("equations");
    QDomElement equationElement = equationsElement.firstChildElement("equation");
    while(!equationElement.isNull())
    {
        useEquations=true;
        mpEquationsTextField->append(equationElement.text());
        equationElement = equationElement.nextSiblingElement("equation");
    }
    if(useEquations)
    {
        mpGenerateFromComboBox->setCurrentIndex(0);
    }
    else
    {
        mpGenerateFromComboBox->setCurrentIndex(1);
    }

    mpInitAlgorithmsTextField->clear();
    QDomElement initAlgorithmEquations = modelRoot.firstChildElement("initalgorithms");
    QDomElement initAlgorithmEquation = initAlgorithmEquations.firstChildElement("equation");
    while(!initAlgorithmEquation.isNull())
    {
        mpInitAlgorithmsTextField->append(initAlgorithmEquation.text());
        initAlgorithmEquation = initAlgorithmEquation.nextSiblingElement("equation");
    }

    mpFinalAlgorithmsTextField->clear();
    QDomElement finalAlgorithmEquations = modelRoot.firstChildElement("finalalgorithms");
    QDomElement finalAlgorithmEquation = finalAlgorithmEquations.firstChildElement("equation");
    while(!finalAlgorithmEquation.isNull())
    {
        mpFinalAlgorithmsTextField->append(finalAlgorithmEquation.text());
        finalAlgorithmEquation = finalAlgorithmEquation.nextSiblingElement("equation");
    }

    mpBoundaryEquationsTextField->clear();
    QDomElement boundaryEquations = modelRoot.firstChildElement("boundaryequations");
    QDomElement boundaryEquation = boundaryEquations.firstChildElement("equation");
    while(!boundaryEquation.isNull())
    {
        mpBoundaryEquationsTextField->append(boundaryEquation.text());
        boundaryEquation = boundaryEquation.nextSiblingElement("equation");
    }

    update();
}



void ComponentGeneratorDialog::saveDialogToXml()
{
    QDomDocument domDocument;
    QDomElement componentRoot = domDocument.createElement("hopsancomponent");
    componentRoot.setAttribute("typename", mpComponentNameEdit->text());
    componentRoot.setAttribute("displayname", mpComponentDisplayEdit->text());
    componentRoot.setAttribute("cqstype", mpComponentTypeComboBox->currentText());
    domDocument.appendChild(componentRoot);

    QDomElement portsElement = appendDomElement(componentRoot,"ports");
    for(int i=0; i<mPortList.size(); ++i)
    {
        QDomElement portElement = appendDomElement(portsElement,"port");
        portElement.setAttribute("name", mPortList.at(i).name);
        portElement.setAttribute("type", mPortList.at(i).porttype);
        portElement.setAttribute("nodetype", mPortList.at(i).nodetype);
        if(mPortList.at(i).notrequired)
            portElement.setAttribute("notrequired", "True");
        else
            portElement.setAttribute("notrequired", "False");
        portElement.setAttribute("default", mPortList.at(i).defaultvalue);
    }

    QDomElement parametersElement = appendDomElement(componentRoot,"parameters");
    for(int i=0; i<mParametersList.size(); ++i)
    {
        QDomElement parElement = appendDomElement(parametersElement,"parameter");
        parElement.setAttribute("name", mParametersList.at(i).name);
        parElement.setAttribute("displayname", mParametersList.at(i).displayName);
        parElement.setAttribute("description", mParametersList.at(i).description);
        parElement.setAttribute("unit", mParametersList.at(i).unit);
        parElement.setAttribute("init", mParametersList.at(i).init);
    }

    QDomElement utilitiesElement = appendDomElement(componentRoot,"utilities");
    for(int i=0; i<mUtilitiesList.size(); ++i)
    {
        QDomElement utilityElement = appendDomElement(utilitiesElement,"utility");
        utilityElement.setAttribute("utility", mUtilitiesList[i].utility);
        utilityElement.setAttribute("name", mUtilitiesList[i].name);
    }

    QDomElement variablesElement = appendDomElement(componentRoot,"staticvariables");
    for(int i=0; i<mStaticVariablesList.size(); ++i)
    {
        QDomElement variableElement = appendDomElement(variablesElement,"staticvariable");
        variableElement.setAttribute("name", mStaticVariablesList[i].name);
        variableElement.setAttribute("datatype", "double");
    }

    QDomElement initializeElement = appendDomElement(componentRoot,"initialize");
    QString plainInitEquations = mpInitTextField->toPlainText();
    QStringList initEquations = plainInitEquations.split("\n");
    for(int i=0; i<initEquations.size(); ++i)
    {
        appendDomTextNode(initializeElement,"equation",initEquations.at(i));
    }

    QDomElement simulateElement = appendDomElement(componentRoot,"simulate");
    QString plainSimEquations = mpSimulateTextField->toPlainText();
    QStringList simEquations = plainSimEquations.split("\n");
    for(int i=0; i<simEquations.size(); ++i)
    {
        appendDomTextNode(simulateElement,"equation",simEquations.at(i));
    }

    QDomElement finalizeElement = appendDomElement(componentRoot,"finalize");
    QString plainFinalEquations = mpFinalizeTextField->toPlainText();
    QStringList finalEquations = plainFinalEquations.split("\n");
    for(int i=0; i<finalEquations.size(); ++i)
    {
        appendDomTextNode(finalizeElement,"equation",finalEquations.at(i));
    }

    QDomElement initAlgorithmsElement = appendDomElement(componentRoot, "initalgorithms");
    QString plainInitAlgorithms = mpInitAlgorithmsTextField->toPlainText();
    QStringList initAlgorithms = plainInitAlgorithms.split("\n");
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        appendDomTextNode(initAlgorithmsElement,"equation",initAlgorithms.at(i));
    }

    QDomElement finalAlgorithmsElement = appendDomElement(componentRoot, "finalalgorithms");
    QString plainfinalAlgorithms = mpFinalAlgorithmsTextField->toPlainText();
    QStringList finalAlgorithms = plainfinalAlgorithms.split("\n");
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        appendDomTextNode(finalAlgorithmsElement,"equation",finalAlgorithms.at(i));
    }

    QDomElement equationsElement = appendDomElement(componentRoot,"equations");
    QString plainEquations = mpEquationsTextField->toPlainText();
    QStringList equations = plainEquations.split("\n");
    for(int i=0; i<equations.size(); ++i)
    {
        appendDomTextNode(equationsElement,"equation",equations.at(i));
    }

    QDomElement boundaryEquationsElement = appendDomElement(componentRoot,"boundaryequations");
    QString plainBoundaryEquations = mpBoundaryEquationsTextField->toPlainText();
    QStringList boundaryEquations = plainBoundaryEquations.split("\n");
    for(int i=0; i<boundaryEquations.size(); ++i)
    {
        appendDomTextNode(boundaryEquationsElement,"equation",boundaryEquations.at(i));
    }

    appendRootXMLProcessingInstruction(domDocument);

    //Save to file
    const int IndentSize = 4;
    if(!QDir(DATAPATH).exists())
        QDir().mkdir(DATAPATH);
    QFile xmlFile(QString(DATAPATH) + "generated_component.xml");
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        //qDebug() << "Failed to open file for writing: " << gExecPath << "generated_component.xml";
        return;
    }
    QTextStream out(&xmlFile);
    domDocument.save(out, IndentSize);

    if(!QDir(DATAPATH).exists())
    {
        QDir().mkpath(DATAPATH);
    }

    QDir().mkpath(QString(DATAPATH)+"compgen/");
    QFile().remove(QString(DATAPATH)+"compgen/component_"+mpComponentNameEdit->text()+".xml");
    xmlFile.copy(QString(DATAPATH)+"compgen/component_"+mpComponentNameEdit->text()+".xml");

    update();
}


void ComponentGeneratorDialog::generateAppearance()
{
    if(mpAppearance == 0)
    {
        mpAppearance = new ModelObjectAppearance();
    }

    mpAppearance->setIconPath(QString(OBJECTICONPATH)+"generatedcomponenticon.svg", USERGRAPHICS, AbsoluteRelativeT(0));

    QStringList leftPortNames, rightPortNames, topPortNames;
    QList<PortAppearance> leftPorts, rightPorts, topPorts;

    for(int p=0; p<mPortList.size(); ++p)
    {
        PortAppearanceMapT portMap = mpAppearance->getPortAppearanceMap();
        if(!portMap.contains(mPortList.at(p).name))
        {
            PortAppearance PortApp;
            PortApp.selectPortIcon(mpComponentTypeComboBox->currentText(), mPortList.at(p).porttype.toUpper(), mPortList.at(p).nodetype);

            if(mPortList.at(p).nodetype == "NodeSignal" && mPortList.at(p).porttype == "ReadPort")
            {
                leftPorts << PortApp;
                leftPortNames << mPortList.at(p).name;
            }
            else if(mPortList.at(p).nodetype == "NodeSignal" && mPortList.at(p).porttype == "WritePort")
            {
                rightPorts << PortApp;
                rightPortNames << mPortList.at(p).name;
            }
            else
            {
                topPorts << PortApp;
                topPortNames << mPortList.at(p).name;
            }
        }
    }

    PortAppearanceMapT portMap = mpAppearance->getPortAppearanceMap();
    PortAppearanceMapT::iterator it;
    QStringList keysToRemove;
    for(it=portMap.begin(); it!=portMap.end(); ++it)
    {
        bool exists=false;
        for(int j=0; j<mPortList.size(); ++j)
        {
            if(mPortList.at(j).name == it.key())
                exists=true;
        }
        if(!exists)
            keysToRemove << it.key();
    }
    for(int i=0; i<keysToRemove.size(); ++i)
    {
        mpAppearance->getPortAppearanceMap().remove(keysToRemove.at(i));
    }

    for(int i=0; i<leftPorts.size(); ++i)
    {
        leftPorts[i].x = 0.0;
        leftPorts[i].y = (double(i)+1)/(double(leftPorts.size())+1.0);
        leftPorts[i].rot = 180;
        mpAppearance->addPortAppearance(leftPortNames[i], &leftPorts[i]);
    }
    for(int i=0; i<rightPorts.size(); ++i)
    {
        rightPorts[i].x = 1.0;
        rightPorts[i].y = (double(i)+1)/(double(rightPorts.size())+1.0);
        rightPorts[i].rot = 0;
        mpAppearance->addPortAppearance(rightPortNames[i], &rightPorts[i]);
    }
    for(int i=0; i<topPorts.size(); ++i)
    {
        topPorts[i].x = (double(i)+1)/(double(topPorts.size())+1.0);
        topPorts[i].y = 0.0;
        topPorts[i].rot = 270;
        mpAppearance->addPortAppearance(topPortNames[i], &topPorts[i]);
    }
}


void ComponentGeneratorDialog::openAppearanceDialog()
{
    generateAppearance();

    MovePortsDialog *mpMP = new MovePortsDialog(mpAppearance,USERGRAPHICS,this);
    mpMP->open();
}


void ComponentGeneratorDialog::togglePortsBox()
{
    mPortsBoxVisible = !mPortsBoxVisible;
    if(mPortsBoxVisible)
    {
        mpPortsMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    }
    else
    {
        mpPortsMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
    }
    update();
    updateValues();
}

void ComponentGeneratorDialog::toggleParametersBox()
{
    mParametersBoxVisible = !mParametersBoxVisible;
    if(mParametersBoxVisible)
    {
        mpParametersMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    }
    else
    {
        mpParametersMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
    }
    update();
    updateValues();
}

void ComponentGeneratorDialog::toggleUtilitiesBox()
{
    mUtilitiesBoxVisible = !mUtilitiesBoxVisible;
    if(mUtilitiesBoxVisible)
    {
        mpUtilitiesMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    }
    else
    {
        mpUtilitiesMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
    }
    update();
    updateValues();
}

void ComponentGeneratorDialog::toggleStaticVariablesBox()
{
    mStaticVariablesBoxVisible = !mStaticVariablesBoxVisible;
    if(mStaticVariablesBoxVisible)
    {
        mpStaticVariablesMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Sub.png"));
    }
    else
    {
        mpStaticVariablesMinMaxButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
    }
    update();
    updateValues();
}
