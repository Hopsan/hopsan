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

    //Functions
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

    //Model structure keywords
    keywordFormat.setForeground(Qt::darkRed);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordPatterns.clear();
    keywordPatterns  << "\\bmodel\\b" << "\\bequation\\b" << "\\balgorithm\\b" << "\\bend\\b" << "\\bannotation\\b";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Parameter and type keywords
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordPatterns.clear();
    keywordPatterns  << "\\bparameter\\b" << "\\bReal\\b";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Comments
    keywordFormat.setForeground(Qt::darkGreen);
    keywordFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    //Quotations
    keywordFormat.setForeground(Qt::darkGreen);
    keywordFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("\".*\"");
    rule.format = keywordFormat;
    highlightingRules.append(rule);
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
    : QMainWindow(parent)
{
    Expression dummy = Expression("(1.0/-Z+1.0)/(1.0/Z+1.0)");
    dummy.expandParentheses();
    dummy.expandParentheses();

    dummy = Expression("2.0*x+5.0*y*x-3.0*y/z+pow(x*z,3.0)");
    dummy.factor(Expression("x"));
    dummy.expand();
    dummy.linearize();
    assert(dummy.toString() == "z*5.0*y*x+z*2.0*x-3.0*y+pow(z,4.0)*pow(x,3.0)");

    dummy = Expression("pow(x,3.0)*pow(y,2.0)/x+pow(z,5.0)/pow(z,3.0)");
    dummy.expandPowers();
    assert(dummy.toString() == "x*x*y*y+z*z");

    dummy = Expression("x*y*(x-5.0*(2.0-z))/(2.0*(1.0-x)-5.0)");
    dummy.expandParentheses();

    dummy = Expression("a*b*c + a*b*d + b*c*d");
    dummy.factorMostCommonFactor();
    assert(dummy.toString() == "b*(a*c+a*d+c*d)");

    dummy = Expression("M*4*pow(1/-Z+1.0,2.0)*x1/(T*(1/Z+1.0)*T*(1/Z+1))+B*2/T*(1/-Z+1)/(1/Z+1)*x1+k*x1-F2-F1");
    dummy.linearize();

    //Set the name and size of the main window
    this->resize(640,480);
    this->setWindowTitle("Component Generator (experimental)");
    this->setPalette(gConfig.getPalette());

    //Set the size and position of the main window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    int w = this->size().width();
    int h = this->size().height();
    int x = (sw - w)/2;
    int y = (sh - h)/2;
    this->move(x, y);       //Move window to center of screen

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

    //Main layout (create widgets, but don't do anything before contents are updated)
    mpCentralLayout = new QVBoxLayout(this);

    mpEquationTabs = new QTabWidget(this);
    mpEquationTabs->setPalette(gConfig.getPalette());
    mpEquationTabs->setTabsClosable(true);
    mNumberOfUntitledTabs = 0;
   // mpEquationTabs->addTab(mpScrollArea, "Untitled");

    //Actions
    mpNewAction = new QAction("New model", this);
    mpNewAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-New.png"));
    mpNewAction->setToolTip("New model");

    mpLoadAction = new QAction("Load Modelica model (.mo)", this);
    mpLoadAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Open.png"));
    mpLoadAction->setToolTip("Load model (.mo)");

    mpSaveAction = new QAction("Save model", this);
    mpSaveAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Save.png"));
    mpSaveAction->setToolTip("Save model");

    mpWizardAction = new QAction("Launch template wizard", this);
    mpWizardAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Wizard.png"));
    mpWizardAction->setToolTip("Launch template wizard");

    //Tool bar
    mpToolBar = new QToolBar(this);
    mpToolBar->addAction(mpNewAction);
    mpToolBar->addAction(mpLoadAction);
    mpToolBar->addAction(mpSaveAction);
    mpToolBar->addAction(mpWizardAction);
    this->addToolBar(mpToolBar);

    //Menu bar
    mpFileMenu = new QMenu("File", this);
    mpFileMenu->addAction(mpNewAction);
    mpFileMenu->addAction(mpLoadAction);
    mpFileMenu->addAction(mpSaveAction);
    mpFileMenu->addAction(mpWizardAction);
    mpMenuBar = new QMenuBar(this);
    mpMenuBar->addMenu(mpFileMenu);
    this->setMenuBar(mpMenuBar);


    mpCentralLayout->addWidget(mpEquationTabs);
    mpCentralLayout->addWidget(mpButtonBox);
    mpCentralLayout->setStretch(0,0);
    mpCentralLayout->setStretch(1,0);
    mpCentralLayout->setStretch(2,0);
    mpCentralLayout->setStretch(3,1);

    QWidget *pCentralWidget = new QWidget(this);
    pCentralWidget->setLayout(mpCentralLayout);

    this->setCentralWidget(pCentralWidget);

    addNewTab();

    //Connections
    connect(mpCancelButton,     SIGNAL(clicked()),              this, SLOT(close()));
    connect(mpCompileButton,    SIGNAL(clicked()),              this, SLOT(generateComponent()));
    connect(mpAppearanceButton, SIGNAL(clicked()),              this, SLOT(openAppearanceDialog()));
    connect(mpNewAction,        SIGNAL(triggered()),            this, SLOT(addNewTab()));
    connect(mpLoadAction,       SIGNAL(triggered()),            this, SLOT(loadFromModelica()));
    connect(mpSaveAction,       SIGNAL(triggered()),            this, SLOT(saveToModelica()));
    connect(mpWizardAction,     SIGNAL(triggered()),            this, SLOT(openComponentGeneratorWizard()));
    connect(mpEquationTabs,     SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}


void ComponentGeneratorDialog::addNewTab()
{
    QFont monoFont = QFont("Monospace", 10, 50);
    monoFont.setStyleHint(QFont::Monospace);

    mEquationTextFieldPtrs.append(new QPlainTextEdit(this));
    mEquationTextFieldPtrs.last()->setFont(monoFont);
    mEquationTextFieldPtrs.last()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mEquationHighLighterPtrs.append(new ModelicaHighlighter(mEquationTextFieldPtrs.last()->document()));

    mEquationsLayoutPtrs.append(new QGridLayout(this));
    mEquationsLayoutPtrs.last()->addWidget(mEquationTextFieldPtrs.last(), 0, 0);

    mScrollAreaPtrs.append(new QScrollArea(this));
    mScrollAreaPtrs.last()->setPalette(gConfig.getPalette());
    mScrollAreaPtrs.last()->setLayout(mEquationsLayoutPtrs.last());
    mScrollAreaPtrs.last()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mScrollAreaPtrs.last()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString tabName = "Untitled";
    if(mNumberOfUntitledTabs > 0)
    {
        tabName.append(QString::number(mNumberOfUntitledTabs));
    }
    mpEquationTabs->addTab(mScrollAreaPtrs.last(), tabName);
    mpEquationTabs->setCurrentWidget(mScrollAreaPtrs.last());

    ++mNumberOfUntitledTabs;
    mModelFiles.append(QFileInfo());
    mHasChanged.append(false);

    connect(mEquationTextFieldPtrs.last(), SIGNAL(textChanged()), this, SLOT(tabChanged()));
}


void ComponentGeneratorDialog::addNewTab(QString code, QString tabName)
{
    addNewTab();
    mEquationTextFieldPtrs.last()->setPlainText(code);
    if(!tabName.isEmpty())
    {
        mpEquationTabs->setTabText(mpEquationTabs->count()-1, tabName);
    }
    mHasChanged.last() = false;

}


void ComponentGeneratorDialog::closeTab(int i)
{
    mpEquationTabs->removeTab(i);
    mEquationsLayoutPtrs.removeAt(i);
    mEquationTextFieldPtrs.removeAt(i);
    mEquationHighLighterPtrs.removeAt(i);
    mHasChanged.removeAt(i);
    mModelFiles.removeAt(i);
}


void ComponentGeneratorDialog::tabChanged()
{
    QPlainTextEdit *textEdit = qobject_cast<QPlainTextEdit *>(sender());

    int idx = mEquationTextFieldPtrs.indexOf(textEdit);

    if(!mHasChanged[idx] && !textEdit->toPlainText().isEmpty())
    {
        mHasChanged[idx] = true;
        mpEquationTabs->setTabText(idx, mpEquationTabs->tabText(idx)+"*");
    }
}


void ComponentGeneratorDialog::generateComponent()
{
    QString typeName, displayName, cqsType;
    QStringList initAlgorithms, equations, finalAlgorithms;
    QList<PortSpecification> portList;
    QList<ParameterSpecification> parametersList;

    parseModelicaModel(mEquationTextFieldPtrs.at(mpEquationTabs->currentIndex())->toPlainText(), typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

    //Generate component
    generateComponentObject(typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);
}


//! @brief Loads a model from a Modelica file
void ComponentGeneratorDialog::loadFromModelica()
{
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Modlica File"),
                                                         gConfig.getModelicaModelsDir(),
                                                         tr("Modelica Model (*.mo)"));
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

    addNewTab(code);
    mModelFiles.last() = fileInfo;
}


//! @brief Saves current model to a Modelica file
void ComponentGeneratorDialog::saveToModelica()
{
    int idx = mpEquationTabs->currentIndex();

    QFileInfo fileInfo;
    if(mModelFiles.at(idx).fileName().isEmpty())
    {
        QDir fileDialogSaveDir;
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                     gConfig.getModelicaModelsDir(),
                                                     tr("Modelica Model (.mo)"));

        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }

        fileInfo = QFileInfo(modelFilePath);
        gConfig.setModelicaModelsDir(fileInfo.absolutePath());
    }
    else
    {
        fileInfo = mModelFiles.at(idx);
    }



    QFile file(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file: "+fileInfo.filePath());
        return;
    }

    QTextStream t(&file);
    t << mEquationTextFieldPtrs.at(idx)->toPlainText();
    file.close();

    if(mHasChanged[idx])
    {
        mHasChanged[idx]=false;
        QString tabName = mpEquationTabs->tabText(idx);
        tabName.remove("*");
        mpEquationTabs->setTabText(idx, tabName);
    }
}


void ComponentGeneratorDialog::openAppearanceDialog()
{
//    generateAppearance();

//    MovePortsDialog *mpMP = new MovePortsDialog(mpAppearance,USERGRAPHICS,this);
//    mpMP->open();
}


void ComponentGeneratorDialog::openComponentGeneratorWizard()
{
    ComponentGeneratorWizard *pWizard = new ComponentGeneratorWizard(this);
    pWizard->show();
}


ComponentGeneratorWizard::ComponentGeneratorWizard(ComponentGeneratorDialog *parent)
    : QWizard(parent)
{
    mpParent = parent;

    mpFirstPage = new QWizardPage(this);
    mpFirstPage->setTitle("General settings");
    mpFirstPageLayout = new QGridLayout(this);
    mpFirstPage->setLayout(mpFirstPageLayout);
    mpNumberOfPortsLabel = new QLabel("Number of ports:");
    mpNumberOfPortsSpinBox = new QSpinBox(this);
    mpNumberOfPortsSpinBox->setValue(2);
    mpNumberOfParametersLabel = new QLabel("Number of parameters:");
    mpNumberOfParametersSpinBox = new QSpinBox(this);
    mpNumberOfParametersSpinBox->setValue(0);
    mpTypeNameLabel = new QLabel("Type name:", this);
    mpTypeNameLineEdit = new QLineEdit(this);
    mpDisplayNameLabel = new QLabel("Display name: ", this);
    mpDisplayNameLineEdit = new QLineEdit(this);
    mpCqsTypeLabel = new QLabel("CQS type: ", this);
    mpCqsTypeComboBox = new QComboBox(this);
    mpCqsTypeComboBox->addItems(QStringList() << "C" << "Q" << "S");
    mpFirstPageLayout->addWidget(mpTypeNameLabel,0,0);
    mpFirstPageLayout->addWidget(mpTypeNameLineEdit,0,1);
    mpFirstPageLayout->addWidget(mpDisplayNameLabel,1,0);
    mpFirstPageLayout->addWidget(mpDisplayNameLineEdit,1,1);
    mpFirstPageLayout->addWidget(mpCqsTypeLabel,2,0);
    mpFirstPageLayout->addWidget(mpCqsTypeComboBox,2,1);
    mpFirstPageLayout->addWidget(mpNumberOfPortsLabel,3,0);
    mpFirstPageLayout->addWidget(mpNumberOfPortsSpinBox,3,1);
    mpFirstPageLayout->addWidget(mpNumberOfParametersLabel,4,0);
    mpFirstPageLayout->addWidget(mpNumberOfParametersSpinBox,4,1);

    mpSecondPage = new QWizardPage(this);
    mpSecondPage->setTitle("Port settings");
    mpSecondPageLayout = new QGridLayout(this);
    mpSecondPage->setLayout(mpSecondPageLayout);
    mpPortIdTitle = new QLabel("Id");
    mpPortNameTitle = new QLabel("Name");
    mpPortTypeTitle = new QLabel("Port type");
    mpNodeTypeTitle = new QLabel("Node type");
    mpDefaultValueTitle = new QLabel("Default value");
    mpSecondPageLayout->addWidget(mpPortIdTitle,        0,0);
    mpSecondPageLayout->addWidget(mpPortNameTitle,      0,1);
    mpSecondPageLayout->addWidget(mpPortTypeTitle,      0,2);
    mpSecondPageLayout->addWidget(mpDefaultValueTitle,  0,3);

    mpThirdPage = new QWizardPage(this);
    mpThirdPage->setTitle("Parameter settings!");
    mpThirdPage->setFinalPage(true);
    mpThirdPageLayout = new QGridLayout(this);
    mpThirdPage->setLayout(mpThirdPageLayout);
    mpParameterNameTitle = new QLabel("Name");
    mpParameterUnitTitle = new QLabel("Unit");
    mpParameterDescriptionTitle = new QLabel("Description");
    mpParameterValueTitle = new QLabel("Default value");
    mpThirdPageLayout->addWidget(mpParameterNameTitle,        0,0);
    mpThirdPageLayout->addWidget(mpParameterUnitTitle,      0,1);
    mpThirdPageLayout->addWidget(mpParameterDescriptionTitle,      0,2);
    mpThirdPageLayout->addWidget(mpParameterValueTitle,      0,3);


    this->addPage(mpFirstPage);
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(updatePage(int)));
    connect(this, SIGNAL(accepted()), this, SLOT(generate()));
    this->addPage(mpSecondPage);
    this->addPage(mpThirdPage);
}


void ComponentGeneratorWizard::updatePage(int i)
{
    if(i == 1)      //Ports page
    {
        while(mPortIdPtrs.size() > mpNumberOfPortsSpinBox->value())
        {
            mpSecondPageLayout->removeWidget(mPortIdPtrs.last());
            delete(mPortIdPtrs.last());
            mPortIdPtrs.removeLast();
            mpSecondPageLayout->removeWidget(mPortNameLineEditPtrs.last());
            delete(mPortNameLineEditPtrs.last());
            mPortNameLineEditPtrs.removeLast();
            mpSecondPageLayout->removeWidget(mPortTypeComboBoxPtrs.last());
            delete(mPortTypeComboBoxPtrs.last());
            mPortTypeComboBoxPtrs.removeLast();
            mpSecondPageLayout->removeWidget(mPortDefaultSpinBoxPtrs.last());
            delete(mPortDefaultSpinBoxPtrs.last());
            mPortDefaultSpinBoxPtrs.removeLast();
        }

        int portId = mPortIdPtrs.size()+1;
        while(mPortIdPtrs.size() < mpNumberOfPortsSpinBox->value())
        {
            mPortIdPtrs.append(new QLabel(QString::number(portId), this));
            mPortNameLineEditPtrs.append(new QLineEdit("P"+QString::number(portId), this));
            mPortTypeComboBoxPtrs.append(new QComboBox(this));
            mPortTypeComboBoxPtrs.last()->addItems(QStringList() << "Signal Input" << "Signal Output" << "Linear Mechanical" << "Rotational Mechanical" << "Electric" << "Hydraulic" << "Pneumatic");
            mPortDefaultSpinBoxPtrs.append(new QDoubleSpinBox(this));
            mPortDefaultSpinBoxPtrs.last()->setValue(0);

            mpSecondPageLayout->addWidget(mPortIdPtrs.last(),               portId,0);
            mpSecondPageLayout->addWidget(mPortNameLineEditPtrs.last(),     portId,1);
            mpSecondPageLayout->addWidget(mPortTypeComboBoxPtrs.last(),     portId,2);
            mpSecondPageLayout->addWidget(mPortDefaultSpinBoxPtrs.last(),   portId,3);
            ++portId;
        }
    }
    else if(i == 2)     //Parameteres page
    {
        while(mParameterNameLineEditPtrs.size() > mpNumberOfParametersSpinBox->value())
        {
            mpThirdPageLayout->removeWidget(mParameterNameLineEditPtrs.last());
            delete(mParameterNameLineEditPtrs.last());
            mParameterNameLineEditPtrs.removeLast();
            mpThirdPageLayout->removeWidget(mParameterUnitLineEditPtrs.last());
            delete(mParameterUnitLineEditPtrs.last());
            mParameterUnitLineEditPtrs.removeLast();
            mpThirdPageLayout->removeWidget(mParameterDescriptionLineEditPtrs.last());
            delete(mParameterDescriptionLineEditPtrs.last());
            mParameterDescriptionLineEditPtrs.removeLast();
            mpThirdPageLayout->removeWidget(mParameterValueLineEditPtrs.last());
            delete(mParameterValueLineEditPtrs.last());
            mParameterValueLineEditPtrs.removeLast();
        }

        int parameterId = mParameterNameLineEditPtrs.size()+1;
        while(mParameterNameLineEditPtrs.size() < mpNumberOfParametersSpinBox->value())
        {
            mParameterNameLineEditPtrs.append(new QLineEdit(this));
            mParameterUnitLineEditPtrs.append(new QLineEdit(this));
            mParameterDescriptionLineEditPtrs.append(new QLineEdit(this));
            mParameterValueLineEditPtrs.append(new QLineEdit("0", this));

            mpThirdPageLayout->addWidget(mParameterNameLineEditPtrs.last(),         parameterId,0);
            mpThirdPageLayout->addWidget(mParameterUnitLineEditPtrs.last(),         parameterId,1);
            mpThirdPageLayout->addWidget(mParameterDescriptionLineEditPtrs.last(),  parameterId,2);
            mpThirdPageLayout->addWidget(mParameterValueLineEditPtrs.last(),        parameterId,3);
            ++parameterId;
        }
    }
}


void ComponentGeneratorWizard::generate()
{
    //! @todo Add verifications!

    //Initial declaration
    QString output;
    output.append("model "+mpTypeNameLineEdit->text()+" \""+mpDisplayNameLineEdit->text()+"\"\n");
    output.append("  annotation(hopsanCqstype = \""+mpCqsTypeComboBox->currentText()+"\");\n\n");

    //Ports
    QMap<QString, QStringList> nodeToPortMap;
    for(int p=0; p<mPortIdPtrs.size(); ++p)
    {
        QString portType = mPortTypeComboBoxPtrs[p]->currentText();
        if(portType      == "Signal Input")             { portType = "NodeSignalIn"; }
        else if(portType == "Signal Output")            { portType = "NodeSignalOut"; }
        else if(portType == "Linear Mechanical")        { portType = "NodeMechanic"; }
        else if(portType == "Rotational Mechanical")    { portType = "NodeMechanicRotational"; }
        else if(portType == "Electric")                 { portType = "NodeElectric"; }
        else if(portType == "Hydraulic")                { portType = "NodeHydraulic"; }
        else if(portType == "Pneumatic")                { portType = "NodePneumatic"; }

        if(nodeToPortMap.contains(portType))
        {
            QStringList list = nodeToPortMap.find(portType).value();
            list.append(mPortNameLineEditPtrs[p]->text());
            nodeToPortMap.insert(portType, list);
        }
        else
        {
            nodeToPortMap.insert(portType, QStringList() << mPortNameLineEditPtrs[p]->text());
        }
    }
    QMap<QString, QStringList>::iterator pit;
    for(pit=nodeToPortMap.begin(); pit!=nodeToPortMap.end(); ++pit)
    {
        output.append("  "+pit.key()+" ");
        for(int p=0; p<pit.value().size(); ++p)
        {
            output.append(pit.value()[p]+", ");
        }
        output.chop(2);
        output.append(";\n");
    }
    output.append("\n");

    //Parameters
    for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
    {
        QString name = mParameterNameLineEditPtrs[p]->text();
        QString unit = mParameterUnitLineEditPtrs[p]->text();
        QString desc = mParameterDescriptionLineEditPtrs[p]->text();
        QString value = mParameterValueLineEditPtrs[p]->text();

        output.append("  parameter Real "+name+"(unit=\""+unit+"\")="+value+" \""+desc+"\";\n");
    }
    if(mParameterNameLineEditPtrs.size() > 0)
    {
        output.append("\n");
    }

    //The rest
    output.append("algorithm\n  //Place initial algorithms here\n\n");
    output.append("equation\n  //Place equations here\n\n");
    output.append("algorithm\n  //Place final algorithms here\n\n");
    output.append("end "+mpTypeNameLineEdit->text()+";\n");

    mpParent->addNewTab(output);

    this->deleteLater();
}
