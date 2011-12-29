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

#include "Configuration.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/XMLUtilities.h"
#include "common.h"


PortSpecification::PortSpecification(QString porttype, QString nodetype, QString name, bool notrequired, QString defaultvalue)
{
    this->porttype = porttype;
    this->nodetype = nodetype;
    this->name = name;
    this->notrequired = notrequired;
    this->defaultvalue = defaultvalue;
}


ParameterSpecification::ParameterSpecification(QString name, QString displayName, QString description, QString unit, QString init)
{
    this->name = name;
    this->displayName = displayName;
    this->description = description;
    this->unit = unit;
    this->init = init;
}


//! @brief Constructor
ComponentGeneratorDialog::ComponentGeneratorDialog(MainWindow *parent)
    : QDialog(parent)
{
    //Set the name and size of the main window
    this->resize(640,480);
    this->setWindowTitle("Component Generator");
    this->setPalette(gConfig.getPalette());

    //Equation Text Field
    mpEquationsTextField = new QTextEdit(this);
    mpEquationsLayout = new QGridLayout(this);
    mpEquationsLayout->addWidget(mpEquationsTextField);
    mpEquationsGroupBox = new QGroupBox("Equations");
    mpEquationsGroupBox->setLayout(mpEquationsLayout);

    //Buttons
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpCancelButton->setAutoDefault(false);
    mpCompileButton = new QPushButton(tr("&Compile"), this);
    mpCompileButton->setDefault(true);
    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCompileButton, QDialogButtonBox::ActionRole);

    //General settings
    mpComponentNameLabel = new QLabel("Type Name: ");
    mpComponentNameEdit = new QLineEdit(this);
    mpComponentDisplayLabel = new QLabel("Display Name: ");
    mpComponentDisplayEdit = new QLineEdit(this);
    mpComponentTypeLabel = new QLabel("CQS Type: ");
    mpComponentTypeComboBox = new QComboBox(this);
    mpComponentTypeComboBox->addItems(QStringList() << "C" << "Q" << "S");

    //Group boxes and layouts
    mpParametersGroupBox = new QGroupBox("Parameters", this);
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

    mpPortsGroupBox = new QGroupBox("Ports", this);
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


    //Main layout
    mpLayout = new QGridLayout(this);
    setLayout(mpLayout);

    update();

    //Connections
    connect(mpCancelButton,    SIGNAL(clicked()), this, SLOT(reject()));
    connect(mpCompileButton,   SIGNAL(clicked()), this, SLOT(compile()));
}


//! @brief Reimplementation of open() slot, used to initialize the dialog
void ComponentGeneratorDialog::open()
{
    QDialog::open();
}


void ComponentGeneratorDialog::addPort()
{
    mPortList.append(PortSpecification());
    update();
}


void ComponentGeneratorDialog::addParameter()
{
    mParametersList.append(ParameterSpecification());
    update();
}


void ComponentGeneratorDialog::removePort()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mvRemovePortButtons.indexOf(button);

    mPortList.removeAt(i);
    qDebug() << "Removing port with index " << i;

    update();
}


void ComponentGeneratorDialog::removeParameter()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    int i = mvRemoveParameterButtons.indexOf(button);

    mParametersList.removeAt(i);

    update();
}


void ComponentGeneratorDialog::updateValues()
{
    //Assume sender is a line edit
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());

    if(mvPortNameEdits.contains(lineEdit))
    {
        int i = mvPortNameEdits.indexOf(lineEdit);
        mPortList[i].name = lineEdit->text();
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
    }
    else if(mvNodeTypeComboBoxes.contains(comboBox))
    {
        int i = mvNodeTypeComboBoxes.indexOf(comboBox);
        mPortList[i].nodetype = comboBox->currentText();
    }
}


void ComponentGeneratorDialog::update()
{
    QLayoutItem *pChild;
    while(pChild = mpPortsLayout->takeAt(0))
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

        connect(pPortNameEdit,      SIGNAL(textChanged(QString)),           this, SLOT(updateValues()));
        connect(pPortTypeComboBox,  SIGNAL(currentIndexChanged(QString)),   this, SLOT(updateValues()));
        connect(pNodeTypeComboBox,  SIGNAL(currentIndexChanged(QString)),   this, SLOT(updateValues()));
        connect(pRequiredCheckBox,  SIGNAL(toggled(bool)),                  this, SLOT(updateValues()));
        connect(pDefaultEdit,       SIGNAL(textChanged(QString)),           this, SLOT(updateValues()));
        connect(pRemoveButton,      SIGNAL(pressed()),                      this, SLOT(removePort()));

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

    for(int i=0; i<mParametersList.size(); ++i)
    {
        QLineEdit *pParameterNameEdit = new QLineEdit(mParametersList.at(i).name, this);
        QLineEdit *pParameterDisplayEdit = new QLineEdit(mParametersList.at(i).displayName, this);
        QLineEdit *pParameterDescriptionEdit = new QLineEdit(mParametersList.at(i).description, this);
        QLineEdit *pParameterUnitEdit = new QLineEdit(mParametersList.at(i).unit, this);
        QLineEdit *pParameterInitEdit = new QLineEdit(mParametersList.at(i).init, this);
        QToolButton *pRemoveButton = new QToolButton(this);
        pRemoveButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));

        connect(pParameterNameEdit,         SIGNAL(textChanged(QString)),  this, SLOT(updateValues()));
        connect(pParameterDisplayEdit,      SIGNAL(textChanged(QString)),  this, SLOT(updateValues()));
        connect(pParameterDescriptionEdit,  SIGNAL(textChanged(QString)),  this, SLOT(updateValues()));
        connect(pParameterUnitEdit,         SIGNAL(textChanged(QString)),  this, SLOT(updateValues()));
        connect(pParameterInitEdit,         SIGNAL(textChanged(QString)),  this, SLOT(updateValues()));
        connect(pRemoveButton,              SIGNAL(pressed()),          this, SLOT(removeParameter()));

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

    while(!mpLayout->isEmpty())
    {
        delete(mpLayout->itemAt(0));
        mpLayout->removeItem(mpLayout->itemAt(0));
    }

    mpLayout->addWidget(mpComponentNameLabel,       0, 0);
    mpLayout->addWidget(mpComponentNameEdit,        0, 1);
    mpLayout->addWidget(mpComponentDisplayLabel,    0, 2);
    mpLayout->addWidget(mpComponentDisplayEdit,     0, 3);
    mpLayout->addWidget(mpComponentTypeLabel,       0, 4);
    mpLayout->addWidget(mpComponentTypeComboBox,    0, 5);
    mpLayout->addWidget(mpPortsGroupBox,            1, 0, 1, 6);
    mpLayout->addWidget(mpParametersGroupBox,       2, 0, 1, 6);
    mpLayout->addWidget(mpEquationsGroupBox,        3, 0, 1, 6);
    mpLayout->addWidget(mpButtonBox,                4, 0, 1, 6);
    mpLayout->setRowStretch(3, 1);
}


//! @brief Generates XML and compiles the new component
//! @todo Verify that everything is ok
void ComponentGeneratorDialog::compile()
{
    QDomDocument domDocument;
    QDomElement componentRoot = domDocument.createElement("hopsancomponent");
    componentRoot.setAttribute("typename", mpComponentNameEdit->text());
    componentRoot.setAttribute("displayname", mpComponentDisplayEdit->text());
    componentRoot.setAttribute("cqstype", mpComponentTypeComboBox->currentText());
    domDocument.appendChild(componentRoot);

    for(int i=0; i<mPortList.size(); ++i)
    {
        QDomElement portElement = appendDomElement(componentRoot,"port");
        portElement.setAttribute("name", mPortList.at(i).name);
        portElement.setAttribute("type", mPortList.at(i).porttype);
        portElement.setAttribute("nodetype", mPortList.at(i).nodetype);
        if(mPortList.at(i).notrequired)
            portElement.setAttribute("notrequired", "True");
        else
            portElement.setAttribute("notrequired", "False");
        portElement.setAttribute("default", mPortList.at(i).defaultvalue);
    }

    for(int i=0; i<mParametersList.size(); ++i)
    {
        QDomElement parElement = appendDomElement(componentRoot,"parameter");
        parElement.setAttribute("name", mParametersList.at(i).name);
        parElement.setAttribute("displayname", mParametersList.at(i).displayName);
        parElement.setAttribute("description", mParametersList.at(i).description);
        parElement.setAttribute("unit", mParametersList.at(i).unit);
        parElement.setAttribute("init", mParametersList.at(i).init);
    }

    QString plainEquations = mpEquationsTextField->toPlainText();
    QStringList equations = plainEquations.split("\n");
    for(int i=0; i<equations.size(); ++i)
    {
        appendDomTextNode(componentRoot,"equation",equations.at(i));
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


    generateComponentSourceCode("temp.hpp", componentRoot);



    //QDialog::close();
}

