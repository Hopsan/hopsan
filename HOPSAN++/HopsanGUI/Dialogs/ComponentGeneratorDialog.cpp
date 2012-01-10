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

#include <QFont>

#include "Configuration.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Utilities/XMLUtilities.h"
#include "Widgets/PyDockWidget.h"
#include "common.h"


//! @brief Constructor
ComponentGeneratorDialog::ComponentGeneratorDialog(MainWindow *parent)
    : QDialog(parent)
{
    //Set the name and size of the main window
    this->resize(640,480);
    this->setWindowTitle("Component Generator");
    this->setPalette(gConfig.getPalette());

    //Equation Text Field
    mpGivenLabel = new QLabel("Given: ");
    mpSoughtLabel = new QLabel("Sought: ");

    QFont monoFont = QFont("Monospace", 10, 63);
    monoFont.setStyleHint(QFont::Monospace);

    mpInitTextField = new QTextEdit(this);                                      //Initialize code text field
    mpInitTextField->setFont(monoFont);
    mpInitLayout = new QGridLayout(this);
    mpInitLayout->addWidget(mpInitTextField, 0, 0);
    mpInitWidget = new QWidget(this);
    mpInitWidget->setLayout(mpInitLayout);

    mpSimulateTextField = new QTextEdit(this);                                  //SimulateOneTimeStep code text field
    mpSimulateTextField->setFont(monoFont);
    mpSimulateLayout = new QGridLayout(this);
    mpSimulateLayout->addWidget(mpSimulateTextField, 0, 0);
    mpSimulateWidget = new QWidget(this);
    mpSimulateWidget->setLayout(mpSimulateLayout);

    mpFinalizeTextField = new QTextEdit(this);                                  //Finalize code text field
    mpFinalizeTextField->setFont(monoFont);
    mpFinalizeLayout = new QGridLayout(this);
    mpFinalizeLayout->addWidget(mpFinalizeTextField, 0, 0);
    mpFinalizeWidget = new QWidget(this);
    mpFinalizeWidget->setLayout(mpFinalizeLayout);

    mpEquationsTextField = new QTextEdit(this);                                 //Equation text field
    mpEquationsTextField->setFont(monoFont);
    mpBoundaryEquationsLabel = new QLabel("Boundary Equations:", this);
    mpBoundaryEquationsTextField = new QTextEdit(this);
    mpBoundaryEquationsTextField->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mpBoundaryEquationsTextField->setFont(monoFont);
    mpEquationsLayout = new QGridLayout(this);
    mpEquationsLayout->addWidget(mpEquationsTextField, 0, 0);
    mpEquationsLayout->addWidget(mpBoundaryEquationsLabel, 1, 0);
    mpEquationsLayout->addWidget(mpBoundaryEquationsTextField, 2, 0);
    mpEquationsLayout->setRowStretch(0, 1);
    mpEquationsLayout->setRowStretch(1, 0);
    mpEquationsLayout->setRowStretch(2, 0);
    mpEquationsWidget = new QWidget(this);
    mpEquationsWidget->setLayout(mpEquationsLayout);

    mpCodeTabs = new QTabWidget(this);                                          //Tab layout
    mpCodeTabs->addTab(mpInitWidget, "Initialize");
    mpCodeTabs->addTab(mpSimulateWidget, "Simulate");
    mpCodeTabs->addTab(mpFinalizeWidget, "Finalize");
    mpCodeLayout = new QGridLayout(this);
    mpCodeLayout->addWidget(mpGivenLabel, 0, 0);
    mpCodeLayout->addWidget(mpSoughtLabel, 1, 0);
    mpCodeLayout->addWidget(mpCodeTabs, 2, 0);
    mpCodeLayout->addWidget(mpEquationsWidget, 3, 0);
    mpCodeGroupBox = new QGroupBox("Equations", this);
    mpCodeGroupBox->setLayout(mpCodeLayout);

    //Buttons
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpCompileButton = new QPushButton(tr("&Compile"), this);
    mpCompileButton->setDefault(true);
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCompileButton, QDialogButtonBox::ActionRole);

    //General settings
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

    mpSaveButton = new QToolButton(this);
    mpSaveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Save.png"));

    mpGenerateFromLabel = new QLabel("Generate From: ");
    mpGenerateFromComboBox = new QComboBox(this);
    mpGenerateFromComboBox->addItems(QStringList() << "Equations" << "C++ Code");
    mpGenerateFromComboBox->setCurrentIndex(0);
    connect(mpGenerateFromComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(update()));

    mpComponentNameLabel = new QLabel("Type Name: ");
    mpComponentNameEdit = new QLineEdit(this);

    mpComponentDisplayLabel = new QLabel("Display Name: ");
    mpComponentDisplayEdit = new QLineEdit(this);

    mpComponentTypeLabel = new QLabel("CQS Type: ");
    mpComponentTypeComboBox = new QComboBox(this);
    mpComponentTypeComboBox->addItems(QStringList() << "C" << "Q" << "S");
    connect(mpComponentTypeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(update()));
    connect(mpComponentTypeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateValues()));

    mpAddItemButton = new QToolButton(this);
    mpAddItemButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
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
    mpParametersGroupBox = new QGroupBox("Parameters", this);                   //Parameters layout
    mpParametersLayout = new QGridLayout();
    mpParametersGroupBox->setLayout(mpParametersLayout);
    mpPortNamesLabel = new QLabel("Name:", this);
    mpPortTypeLabel = new QLabel("Port Type:", this);
    mpNodeTypelabel = new QLabel("Node Type:", this);
    mpPortRequiredLabel = new QLabel("Required:", this);
    mpPortDefaultLabel = new QLabel("Default Value:", this);
    mpAddPortButton = new QToolButton(this);
    mpAddPortButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddPortButton->setToolTip("Add Port");
    connect(mpAddPortButton, SIGNAL(pressed()), this, SLOT(addPort()));

    mpPortsGroupBox = new QGroupBox("Ports", this);                             //Ports layout
    mpPortsLayout = new QGridLayout();
    mpPortsGroupBox->setLayout(mpPortsLayout);
    mpParametersNameLabel = new QLabel("Name:", this);
    mpParametersDisplayLabel = new QLabel("Display Name:", this);
    mpParametersDescriptionLabel = new QLabel("Description:", this);
    mpParametersUnitLabel = new QLabel("Unit:", this);
    mpParametersInitLabel = new QLabel("Default Value:", this);
    mpAddParameterButton = new QToolButton(this);
    mpAddParameterButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddParameterButton->setToolTip("Add Parameter");
    connect(mpAddParameterButton, SIGNAL(pressed()), this, SLOT(addParameter()));

    mpUtilitiesGroupBox = new QGroupBox("Utilities", this);                     //Utilities layout
    mpUtilitiesLayout = new QGridLayout();
    mpUtilitiesGroupBox->setLayout(mpUtilitiesLayout);
    mpUtilitiesLabel = new QLabel("Utility:", this);
    mpUtilityNamesLabel = new QLabel("Name:", this);
    mpAddUtilityButton = new QToolButton(this);
    mpAddUtilityButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddUtilityButton->setToolTip("Add Utility");
    connect(mpAddUtilityButton, SIGNAL(pressed()), this, SLOT(addUtility()));

    mpStaticVariablesGroupBox = new QGroupBox("Static Variables", this);        //Static variables layout
    mpStaticVariablesLayout = new QGridLayout();
    mpStaticVariablesGroupBox->setLayout(mpStaticVariablesLayout);
    mpStaticVariableNamesLabel = new QLabel("Name:", this);
    mpAddStaticVariableButton = new QToolButton(this);
    mpAddStaticVariableButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Add.png"));
    mpAddStaticVariableButton->setToolTip("Add Static Variable");
    connect(mpAddStaticVariableButton, SIGNAL(pressed()), this, SLOT(addStaticVariable()));



    //Main layout
    mpLayout = new QGridLayout(this);
    setLayout(mpLayout);

    update();
    updateValues();

    //Connections
    connect(mpCancelButton,    SIGNAL(clicked()), this, SLOT(reject()));
    connect(mpCompileButton,   SIGNAL(clicked()), this, SLOT(compile()));
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
    qDebug() << "Removing port with index " << i;
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
    qDebug() << "updateValues()";

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
        mpEquationsWidget->show();
        qDebug() << "CQSType = " << mpComponentTypeComboBox->currentText();
        mpBoundaryEquationsTextField->setVisible(!mpBoundaryEquationsTextField->toPlainText().isEmpty() && mpComponentTypeComboBox->currentText() == "Q");
        mpBoundaryEquationsLabel->setVisible(mpBoundaryEquationsTextField->isVisible());
    }
    else
    {
        mpCodeTabs->show();
        mpEquationsWidget->hide();
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

    if(!mPortList.isEmpty())
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
    mpPortNamesLabel->setVisible(!mPortList.isEmpty());
    mpPortTypeLabel->setVisible(!mPortList.isEmpty());
    mpNodeTypelabel->setVisible(!mPortList.isEmpty());
    mpPortRequiredLabel->setVisible(!mPortList.isEmpty());
    mpPortDefaultLabel->setVisible(!mPortList.isEmpty());
    mpAddPortButton->setVisible(!mPortList.isEmpty());
    mpPortsGroupBox->setVisible(!mPortList.isEmpty());


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

    if(!mParametersList.isEmpty())
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

    mpParametersNameLabel->setVisible(!mParametersList.isEmpty());
    mpParametersDisplayLabel->setVisible(!mParametersList.isEmpty());
    mpParametersDescriptionLabel->setVisible(!mParametersList.isEmpty());
    mpParametersUnitLabel->setVisible(!mParametersList.isEmpty());;
    mpParametersInitLabel->setVisible(!mParametersList.isEmpty());
    mpAddParameterButton->setVisible(!mParametersList.isEmpty());
    mpParametersGroupBox->setVisible(!mParametersList.isEmpty());

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

    if(!mUtilitiesList.isEmpty())
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

    mpUtilitiesLabel->setVisible(!mUtilitiesList.isEmpty());
    mpUtilityNamesLabel->setVisible(!mUtilitiesList.isEmpty());
    mpAddUtilityButton->setVisible(!mUtilitiesList.isEmpty());
    mpUtilitiesGroupBox->setVisible(!mUtilitiesList.isEmpty());


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

    if(!mStaticVariablesList.isEmpty())
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
    mpStaticVariableNamesLabel->setVisible(!mStaticVariablesList.isEmpty());
    mpAddStaticVariableButton->setVisible(!mStaticVariablesList.isEmpty());
    mpStaticVariablesGroupBox->setVisible(!mStaticVariablesList.isEmpty());

    while(!mpLayout->isEmpty())
    {
        delete(mpLayout->itemAt(0));
        mpLayout->removeItem(mpLayout->itemAt(0));
    }

    mpLayout->addWidget(mpLoadButton,               0, 0);
    mpLayout->addWidget(mpSaveButton,               0, 1);
    mpLayout->addWidget(mpGenerateFromLabel,        0, 2);
    mpLayout->addWidget(mpGenerateFromComboBox,     0, 3);
    mpLayout->addWidget(mpComponentNameLabel,       0, 4);
    mpLayout->addWidget(mpComponentNameEdit,        0, 5);
    mpLayout->addWidget(mpComponentDisplayLabel,    0, 6);
    mpLayout->addWidget(mpComponentDisplayEdit,     0, 7);
    mpLayout->addWidget(mpComponentTypeLabel,       0, 8);
    mpLayout->addWidget(mpComponentTypeComboBox,    0, 9);
    mpLayout->addWidget(mpAddItemButton,            0, 10);
    mpLayout->addWidget(mpPortsGroupBox,            1, 0, 1, 11);
    mpLayout->addWidget(mpParametersGroupBox,       2, 0, 1, 11);
    mpLayout->addWidget(mpUtilitiesGroupBox,        3, 0, 1, 11);
    mpLayout->addWidget(mpStaticVariablesGroupBox,  4, 0, 1, 11);
    mpLayout->addWidget(mpCodeGroupBox,             5, 0, 1, 11);
    mpLayout->addWidget(mpButtonBox,                6, 0, 1, 11);
    mpLayout->setRowStretch(5, 1);



    updateGivenSoughtText();


    updateBoundaryEquations();
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


//! @brief Generates XML and compiles the new component
void ComponentGeneratorDialog::compile()
{
    //Verify that everything is ok

    if(!verifyParameteres(mParametersList))
    {
        qDebug() << "Verification of parameters failed.";
        return;
    }

    if(!verifyPorts(mPortList))
    {
        qDebug() << "Verification of ports failed.";
        return;
    }

    if(!verifyUtilities(mUtilitiesList))
    {
        qDebug() << "Verification of utilities failed.";
        return;
    }

    if(!verifyStaticVariables(mStaticVariablesList))
    {
        qDebug() << "Verification of static variables failed.";
        return;
    }

    if(mpGenerateFromComboBox->currentIndex() == 0)         //Compile from equations
    {
        qDebug() << "Compiling equations";

        //Create list of equqtions
        QString plainEquations = mpEquationsTextField->toPlainText();
        QStringList equations = plainEquations.split("\n");
        QString plainBoundaryEquations = mpBoundaryEquationsTextField->toPlainText();
        QStringList boundaryEquations = plainBoundaryEquations.split("\n");
        equations.append(boundaryEquations);
        equations.removeAll("");

        //Identify derivatives, and replace them with "s"
        identifyDerivatives(equations);

        //Verify the equations
        if(!verifyEquations(equations))
        {
            qDebug() << "Verification failed.";
            return;
        }

        //Replace reserved words to avoid collision
        replaceReservedWords(equations);
        replaceReservedWords(mPortList);

        qDebug() << "Equations = " << equations;

        //Generate a list of state variables (= "output" variables)
        QStringList stateVars;
        for(int i=0; i<mPortList.size(); ++i)
        {
            QString num = QString().setNum(i+1);
            if(mPortList[i].porttype == "WritePort")
            {
                stateVars << mPortList[i].name;
            }
            else if(mPortList[i].porttype == "PowerPort" && mpComponentTypeComboBox->currentText() == "C")
            {
                QStringList cVars;
                cVars << getCVariables(mPortList[i].nodetype);
                for(int v=0; v<cVars.size(); ++v)
                {
                    stateVars << cVars[v]+num;
                }
            }
            else if(mPortList[i].porttype == "PowerPort" && mpComponentTypeComboBox->currentText() == "Q")
            {
                QStringList qVars;
                qVars << getQVariables(mPortList[i].nodetype);
                for(int v=0; v<qVars.size(); ++v)
                {
                    stateVars << qVars[v]+num;
                }
            }
        }

        //Identify used variables in each equation
        QList<QStringList> leftSymbols, rightSymbols;
        for(int i=0; i<equations.size(); ++i)
        {
            leftSymbols.append(QStringList());
            rightSymbols.append(QStringList());
            identifyVariables(equations[i], leftSymbols[i], rightSymbols[i]);
        }

        //Sum up all used variables to a single list
        QStringList allSymbols;
        for(int i=0; i<equations.size(); ++i)
        {
            allSymbols.append(leftSymbols.at(i));
            allSymbols.append(rightSymbols.at(i));
        }
        allSymbols.removeDuplicates();
        allSymbols.append("mTime");         //Simulation time
        allSymbols.append("mTimestep");     //Simulation time step
        allSymbols.append("qi00");          //Delay operator

        //Load sympy libraries
        gpMainWindow->mpPyDockWidget->runCommand("from sympy import *");

        //Create symbol objects for all variables
        for(int i=0; i<allSymbols.size(); ++i)
        {
            gpMainWindow->mpPyDockWidget->runCommand(allSymbols[i]+"=Symbol(\""+allSymbols[i]+"\")");
        }

//        //Create a vector with
//        QString command = "X=Matrix([[";
//        for(int i=0; i<allSymbols.size(); ++i)
//        {
//            command.append(allSymbols.at(i)+"],[");
//        }
//        command.chop(2);
//        command.append("])");
//        gpMainWindow->mpPyDockWidget->runCommand(command);

        //Define system equations
        for(int i=0; i<equations.size(); ++i)
        {
            QString iStr = QString().setNum(i);
            gpMainWindow->mpPyDockWidget->runCommand("left"+iStr+" = " + equations.at(i).section("=",0,0));
            gpMainWindow->mpPyDockWidget->runCommand("right"+iStr+" = " + equations.at(i).section("=",1,1));
            gpMainWindow->mpPyDockWidget->runCommand("f"+iStr+" = left"+iStr+"-right"+iStr);
            gpMainWindow->mpPyDockWidget->runCommand("f"+iStr+" = f"+iStr+".subs(s, 2/mTimestep*(1-qi00)/(1+qi00))");
            gpMainWindow->mpPyDockWidget->runCommand("f"+iStr+" = f"+iStr+".as_numer_denom()[0]");
            gpMainWindow->mpPyDockWidget->runCommand("f"+iStr+" = simplify(f"+iStr+")");
        }

        //Generate the Jacobian matrix
        QStringList jString;
        for(int i=0; i<equations.size(); ++i)
        {
            for(int j=0; j<stateVars.size(); ++j)
            {
                QString iStr = QString().setNum(i);
                QString jStr = QString().setNum(j);
                gpMainWindow->mpPyDockWidget->runCommand("j"+iStr+jStr+" = diff(f"+iStr+".subs(qi00, 0), "+stateVars.at(j)+")");
                gpMainWindow->mpPyDockWidget->runCommand("print(j"+iStr+jStr+")");
                jString.append(gpMainWindow->mpPyDockWidget->getLastOutput());      //Create C++ stringlist of Jacobian
            }
        }

        //Print each system quation and read output to create C++ strings from them
        QStringList sysEquations;
        for(int i=0; i<equations.size(); ++i)
        {
            gpMainWindow->mpPyDockWidget->runCommand("print(f"+QString().setNum(i)+")");
            sysEquations.append(gpMainWindow->mpPyDockWidget->getLastOutput());
        }

        qDebug() << "Jacobian = " << jString;
        qDebug() << "System Equations = " << sysEquations;
        qDebug() << "State Variables = " << stateVars;

        //Replace delay operators in equations with delay utilities
        QStringList delayTerms;
        QStringList delaySteps;
        translateDelaysFromPython(sysEquations, delayTerms, delaySteps);

        //General component data
        QString typeName = mpComponentNameEdit->text();
        QString displayName = mpComponentDisplayEdit->text();
        QString cqsType = mpComponentTypeComboBox->currentText();

        //Call utility to generate and compile the source code
        generateComponentSourceCode(typeName, displayName, cqsType, mPortList, mParametersList, sysEquations, stateVars, jString, delayTerms, delaySteps);
    }
    else if(mpGenerateFromComboBox->currentIndex() == 1)        //Compile from C++ code
    {
        //Generate a DOM document from member variables

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

        //Save to file
        const int IndentSize = 4;
        if(!QDir(DATAPATH).exists())
            QDir().mkdir(DATAPATH);
        QFile xmlsettings(gExecPath + "generated_component.xml");
        if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            qDebug() << "Failed to open file for writing: " << gExecPath << "generated_component.xml";
            return;
        }
        QTextStream out(&xmlsettings);
        domDocument.save(out, IndentSize);

        //Call utility to generate and compile source code
        generateComponentSourceCode("temp.hpp", componentRoot);
    }
}


//! @brief Loads a model from a Modelica file
void ComponentGeneratorDialog::loadFromModelica()
{
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Modlica File"),
                                                         gExecPath+"/",
                                                         tr("Modelica File (*.mo)"));
    if(modelFileName.isEmpty())
    {
        return;
    }

    QFile file(modelFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"), "Unable to read Modelica file.");
        return;
    }
    QString code;
    QTextStream t(&file);
    code = t.readAll();
    file.close();

    //Parse the file
    QString typeName, displayName;
    QStringList equations;
    parseModelicaModel(code, typeName, displayName, equations, mPortList, mParametersList);

    //Update everything
    update();
    updateValues();
    mpComponentNameEdit->setText(typeName);
    mpComponentDisplayEdit->setText(displayName);
    for(int i=0; i<equations.size(); ++i)
    {
        mpEquationsTextField->append(equations.at(i));
    }
}


//! @brief Loads a model from XML code
//! @todo Finish implementation
void ComponentGeneratorDialog::loadFromXml()
{
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Modlica File"),
                                                         gExecPath+"/",
                                                         tr("Modelica File (*.mo)"));
    if(modelFileName.isEmpty())
    {
        return;
    }

    QFile file(QString(DATAPATH) + QString("hopsanconfig.xml"));
    qDebug() << "Reading config from " << file.fileName();
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

    //! @todo Define a better XML syntax and implement this
//    QDomElement utilitiesElement = modelRoot.firstChildElement("utilities");
//    QDomElement utilityElement = utilitiesElement.firstChildElement("utility");
//    while(!utilityElement.isNull())
//    {
//        comp.utilities.append(utilityElement.attribute ("utility"));
//        comp.utilityNames.append(utilityElement.attribute("name"));
//        utilityElement=utilityElement.nextSiblingElement("utility");
//    }

//    QDomElement parametersElement = modelRoot.firstChildElement("parameters");
//    QDomElement parameterElement = parametersElement.firstChildElement("parameter");
//    while(!parameterElement.isNull())
//    {
//        comp.parNames.append(parameterElement.attribute("name"));
//        comp.parInits.append(parameterElement.attribute("init"));
//        comp.parDisplayNames.append(parameterElement.attribute("displayname"));
//        comp.parDescriptions.append(parameterElement.attribute("description"));
//        comp.parUnits.append(parameterElement.attribute("unit"));
//        parameterElement=parameterElement.nextSiblingElement("parameter");
//    }

//    QDomElement variablesElemenet = modelRoot.firstChildElement("staticvariables");
//    QDomElement variableElement = variablesElemenet.firstChildElement("staticvariable");
//    while(!variableElement.isNull())
//    {
//        comp.varNames.append(variableElement.attribute("name"));
//        comp.varTypes.append(variableElement.attribute("datatype"));
//        variableElement=variableElement.nextSiblingElement("staticvariable");
//    }

//    QDomElement portsElement = modelRoot.firstChildElement("ports");
//    QDomElement portElement = portsElement.firstChildElement("port");
//    while(!portElement.isNull())
//    {
//        comp.portNames.append(portElement.attribute("name"));
//        comp.portTypes.append(portElement.attribute("type"));
//        comp.portNodeTypes.append(portElement.attribute("nodetype"));
//        comp.portDefaults.append(portElement.attribute("default"));
//        comp.portNotReq.append(portElement.attribute("notrequired") == "True");
//        portElement=portElement.nextSiblingElement("port");
//    }

//    QDomElement initializeElement = modelRoot.firstChildElement("initialize");
//    QDomElement initEquationElement = initializeElement.firstChildElement("equation");
//    while(!initEquationElement.isNull())
//    {
//        comp.initEquations.append(initEquationElement.text());
//        initEquationElement=initEquationElement.nextSiblingElement("equation");
//    }

//    QDomElement simulateElement = modelRoot.firstChildElement("simulate");
//    QDomElement equationElement = simulateElement.firstChildElement("equation");
//    while(!equationElement.isNull())
//    {
//        comp.simEquations.append(equationElement.text());
//        equationElement=equationElement.nextSiblingElement("equation");
//    }
}
