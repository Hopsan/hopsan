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
#include "Utilities/XMLUtilities.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/PyDockWidget.h"
#include "common.h"
#include "CoreAccess.h"



CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkYellow);
    keywordFormat.setFontWeight(QFont::Normal);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\busing\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    preProcessorFormat.setForeground(Qt::darkBlue);
    preProcessorFormat.setFontWeight(QFont::Normal);
    keywordPatterns.clear();
    keywordPatterns << "^#?\\binclude\\b" << "^#?\\bifdef\\b" << "^#?\\bifndef\\b"
                    << "^#?\\belseif\\b" << "^#?\\belse\\b" << "^#?\\bendif\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = preProcessorFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("<.*>");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    tagFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void CppHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}





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
    //Set the name and size of the main window
    this->resize(1024,768);
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

    mpCloseAction = new QAction("Close", this);
    mpCloseAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Close.png"));
    mpCloseAction->setToolTip("Close");

    mpWizardAction = new QAction("Launch template wizard", this);
    mpWizardAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Wizard.png"));
    mpWizardAction->setToolTip("Launch template wizard");

    mpHelpAction = new QAction("Open Context Help", this);
    mpHelpAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    mpHelpAction->setToolTip("Open Context Help");

    mpModelicaHighlighterAction = new QAction("Modelica", this);
    mpModelicaHighlighterAction->setToolTip("Modelica Highlighter");
    mpModelicaHighlighterAction->setCheckable(true);
    mpModelicaHighlighterAction->setChecked(true);

    mpCppHighlighterAction = new QAction("C++", this);
    mpCppHighlighterAction->setToolTip("C++ Highlighter");
    mpCppHighlighterAction->setCheckable(true);
    mpCppHighlighterAction->setChecked(false);

    //Tool bar
    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    mpToolBar = new QToolBar(this);
    mpToolBar->addAction(mpNewAction);
    mpToolBar->addAction(mpLoadAction);
    mpToolBar->addAction(mpSaveAction);
    mpToolBar->addAction(mpWizardAction);
    mpToolBar->addWidget(spacerWidget);
    mpToolBar->addAction(mpHelpAction);
    this->addToolBar(mpToolBar);





    //Menu bar
    mpRecentMenu = new QMenu("Recent models", this);

    mpFileMenu = new QMenu("File", this);
    mpFileMenu->addAction(mpNewAction);
    mpFileMenu->addAction(mpLoadAction);
    mpFileMenu->addAction(mpSaveAction);
    mpFileMenu->addMenu(mpRecentMenu);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpWizardAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpCloseAction);

    mpHighlighterMenu = new QMenu("Highlighter", this);
    mpHighlighterMenu->addAction(mpModelicaHighlighterAction);
    mpHighlighterMenu->addAction(mpCppHighlighterAction);

    mpEditMenu = new QMenu("Edit", this);
    mpEditMenu->addMenu(mpHighlighterMenu);

    mpMenuBar = new QMenuBar(this);
    mpMenuBar->addMenu(mpFileMenu);
    mpMenuBar->addMenu(mpEditMenu);
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

    if(!gConfig.getRecentGeneratorModels().isEmpty())
    {
        QFile modelFile(gConfig.getRecentGeneratorModels().first());
        if(modelFile.exists())
            loadModel(gConfig.getRecentGeneratorModels().first());
        else
            addNewTab();
    }
    else
        addNewTab();

    updateRecentList();

    //Connections
    connect(mpCancelButton,     SIGNAL(clicked()),              this,           SLOT(close()));
    connect(mpCompileButton,    SIGNAL(clicked()),              this,           SLOT(generateComponent()));
    connect(mpAppearanceButton, SIGNAL(clicked()),              this,           SLOT(openAppearanceDialog()));
    connect(mpNewAction,        SIGNAL(triggered()),            this,           SLOT(addNewTab()));
    connect(mpLoadAction,       SIGNAL(triggered()),            this,           SLOT(loadModel()));
    connect(mpSaveAction,       SIGNAL(triggered()),            this,           SLOT(saveModel()));
    connect(mpCloseAction,      SIGNAL(triggered()),            this,           SLOT(close()));
    connect(mpWizardAction,     SIGNAL(triggered()),            this,           SLOT(openComponentGeneratorWizard()));
    connect(mpEquationTabs,     SIGNAL(tabCloseRequested(int)), this,           SLOT(closeTab(int)));
    connect(mpHelpAction,       SIGNAL(triggered()),            gpMainWindow,   SLOT(openContextHelp()));

    connect(mpModelicaHighlighterAction, SIGNAL(triggered(bool)), this, SLOT(setModelicaHighlighter()));
    connect(mpCppHighlighterAction, SIGNAL(triggered(bool)), this, SLOT(setCppHighlighter()));
}


void ComponentGeneratorDialog::addNewTab()
{
    QFont monoFont = QFont("Monospace", 10, 50);
    monoFont.setStyleHint(QFont::Monospace);

    mEquationTextFieldPtrs.append(new QPlainTextEdit(this));
    mEquationTextFieldPtrs.last()->setFont(monoFont);
    mEquationTextFieldPtrs.last()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mEquationHighLighterPtrs.append(new CppHighlighter(mEquationTextFieldPtrs.last()->document()));

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

    connect(mEquationTextFieldPtrs.last()->document(), SIGNAL(modificationChanged(bool)), this, SLOT(tabChanged()));
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
    QTextDocument *doc = qobject_cast<QTextDocument *>(sender());
    QPlainTextEdit *textEdit;

    for(int t=0; t<mEquationTextFieldPtrs.size(); ++t)
    {
        if(mEquationTextFieldPtrs[t]->document() == doc)
        {
            textEdit = mEquationTextFieldPtrs[t];
        }
    }

    int idx = mEquationTextFieldPtrs.indexOf(textEdit);

    if(!mHasChanged[idx] && !textEdit->toPlainText().isEmpty())
    {
        mHasChanged[idx] = true;
        mpEquationTabs->setTabText(idx, mpEquationTabs->tabText(idx)+"*");
    }
}


void ComponentGeneratorDialog::generateComponent()
{
    //Check if it is a modelica model
    QString code = mEquationTextFieldPtrs.at(mpEquationTabs->currentIndex())->toPlainText();
    while(code.endsWith("\n"))
    {
        code.chop(1);   //Remove extra lines at end of code
    }

    if(code.startsWith("model "))
    {
        QString name = code.section(" ",1,1);
        if(code.endsWith("end "+name+";"))
        {
            //It is (probably) a Modelica model, try and compile it
            CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
            pCoreAccess->generateFromModelica(code);
            delete(pCoreAccess);


//            QString typeName, displayName, cqsType;
//            QStringList initAlgorithms, equations, finalAlgorithms;
//            QList<PortSpecification> portList;
//            QList<ParameterSpecification> parametersList;

//            //Parse Modelica code into SymHop objects
//            parseModelicaModel(code, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

//            //Generate component object and compile new component
//            generateComponentObject(typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

            return;
        }
    }

    //Check if it is a C++ model
    if(code.contains("void simulateOneTimestep()"))
    {
        //It is (probably) a C++ model, try and compile it

        qDebug() << "C++";

        CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
        pCoreAccess->generateFromCpp(code);
        delete(pCoreAccess);

//        comp.plainCode = code;

//        compileComponent("equation.hpp", comp, appearance);
    }

    //! @todo Error message
}


//! @brief Loads a model from a Modelica file
void ComponentGeneratorDialog::loadModel()
{
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Modlica File"),
                                                         gConfig.getModelicaModelsDir(),
                                                         tr("Modelica or C++ Models (*.mo *.hpp)"));
    if(modelFileName.isEmpty())
    {
        return;
    }

    loadModel(modelFileName);
}


void ComponentGeneratorDialog::loadModel(QString modelFileName)
{
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

    addNewTab(code, fileInfo.fileName());
    mModelFiles.last() = fileInfo;

    qDebug() << "Tab name: " << mpEquationTabs->tabText(mpEquationTabs->count()-1);

    if(modelFileName.endsWith(".mo"))
    {
        setModelicaHighlighter();
    }

    qDebug() << "Tab name: " << mpEquationTabs->tabText(mpEquationTabs->count()-1);

    gConfig.addRecentGeneratorModel(modelFileName);
    updateRecentList();

    qDebug() << "Tab name: " << mpEquationTabs->tabText(mpEquationTabs->count()-1);
}


//! @brief Saves current model to a Modelica file
void ComponentGeneratorDialog::saveModel()
{
    int idx = mpEquationTabs->currentIndex();
    bool modelica=false;

    //Check if it is a modelica model
    QString code = mEquationTextFieldPtrs.at(mpEquationTabs->currentIndex())->toPlainText();
    while(code.endsWith("\n"))
    {
        code.chop(1);   //Remove extra lines at end of code
    }

    if(code.startsWith("model "))
    {
        QString name = code.section(" ",1,1);
        if(code.endsWith("end "+name+";"))
        {
            modelica = true;
        }
    }

    QFileInfo fileInfo;
    QString modelFilePath;
    if(mModelFiles.at(idx).fileName().isEmpty())
    {
        if(modelica)
        {
            modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                         gConfig.getModelicaModelsDir(),
                                                         tr("Modelica models (*.mo)"));
        }
        else
        {
            modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                         gConfig.getModelicaModelsDir(),
                                                         tr("C++ header files (*.hpp)"));
        }

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
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file: "+fileInfo.filePath());
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

    gConfig.addRecentGeneratorModel(fileInfo.filePath());
    updateRecentList();
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


void ComponentGeneratorDialog::setModelicaHighlighter()
{
    int i = mpEquationTabs->currentIndex();

    bool wasChanged = mpEquationTabs->tabText(i).endsWith("*");

    delete(mEquationHighLighterPtrs.at(i));
    mEquationHighLighterPtrs[i] = new ModelicaHighlighter(mEquationTextFieldPtrs[i]->document());
    mpCppHighlighterAction->setChecked(false);

    //Make sure no asterix is added if model was not changed before
    if(!wasChanged && mpEquationTabs->tabText(i).endsWith("*"))
    {
        QString newText = mpEquationTabs->tabText(i);
        newText.remove("*");
        mpEquationTabs->setTabText(i, newText);
        mHasChanged[i] = wasChanged;
    }
}


void ComponentGeneratorDialog::setCppHighlighter()
{
    int i = mpEquationTabs->currentIndex();

    bool wasChanged = mpEquationTabs->tabText(i).endsWith("*");

    delete(mEquationHighLighterPtrs.at(i));
    mEquationHighLighterPtrs[i] = new CppHighlighter(mEquationTextFieldPtrs[i]->document());
    mpModelicaHighlighterAction->setChecked(false);

    //Make sure no asterix is added if model was not changed before
    if(!wasChanged && mpEquationTabs->tabText(i).endsWith("*"))
    {
        QString newText = mpEquationTabs->tabText(i);
        newText.chop(1);
        mpEquationTabs->setTabText(i, newText);
        mHasChanged[i] = wasChanged;
    }
}



//! @brief Updates the "Recent Models" list
void ComponentGeneratorDialog::updateRecentList()
{
    mpRecentMenu->clear();

    mpRecentMenu->setEnabled(!gConfig.getRecentGeneratorModels().empty());
    if(!gConfig.getRecentGeneratorModels().empty())
    {
        for(int i=0; i<gConfig.getRecentGeneratorModels().size(); ++i)
        {
            if(gConfig.getRecentGeneratorModels().at(i) != "")
            {
                QAction *tempAction;
                tempAction = mpRecentMenu->addAction(gConfig.getRecentGeneratorModels().at(i));
                connect(tempAction, SIGNAL(triggered()), this, SLOT(openRecentModel()));
            }
        }
    }
}


void ComponentGeneratorDialog::openRecentModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    qDebug() << "Trying to open " << action->text();
    if (action)
    {
        loadModel(action->text());
    }
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
            mPortTypeComboBoxPtrs.last()->addItems(QStringList() << "Signal Input" << "Signal Output");
            QStringList nodeTypes;
            NodeInfo::getNodeTypes(nodeTypes);
            Q_FOREACH(const QString &type, nodeTypes)
            {
                QString name = NodeInfo(type).niceName;
                name.replace(0, 1, name[0].toUpper());
                mPortTypeComboBoxPtrs.last()->addItem(name);
            }
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

    QString output;

    QString typeName = mpTypeNameLineEdit->text();
    QString displayName = mpDisplayNameLineEdit->text();
    QString cqsType = mpCqsTypeComboBox->currentText();
    QString cqsTypeLong = mpCqsTypeComboBox->currentText();
    if(cqsType == "S")
    {
        cqsTypeLong.append("ignal");
    }

    //Ports
    QMap<QString, QStringList> nodeToPortMap;
    for(int p=0; p<mPortIdPtrs.size(); ++p)
    {
        QString portType = mPortTypeComboBoxPtrs[p]->currentText();
        if(portType      == "Signal Input")             { portType = "NodeSignalIn"; }
        else if(portType == "Signal Output")            { portType = "NodeSignalOut"; }
        QStringList nodeTypes;
        NodeInfo::getNodeTypes(nodeTypes);
        Q_FOREACH(const QString &type, nodeTypes)
        {
            if(portType.toLower() == NodeInfo(type).niceName)
                portType = type;
        }

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


    bool modelica = false;
    if(modelica)
    {        //Initial declaration

        output.append("model "+typeName+" \""+displayName+"\"\n");
        output.append("  annotation(hopsanCqsType = \""+cqsType+"\");\n\n");

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
    }
    else
    {
        output.append("#include <math.h>\n");
        output.append("#include \"ComponentEssentials.h\"\n");
        output.append("#include \"ComponentUtilities.h\"\n\n");
        output.append("using namespace std;\n\n");
        output.append("namespace hopsan {\n\n");


        output.append("class "+typeName+" : public Component"+cqsTypeLong+"\n{\n");
        output.append("private:\n");

        //Port local variables
        output.append("    double ");
        int np = 1;
        QMap<QString, QStringList>::iterator pit;
        for(pit=nodeToPortMap.begin(); pit!=nodeToPortMap.end(); ++pit)
        {
            QStringList varNames;
            QString numStr;
            varNames << NodeInfo(pit.key()).qVariables << NodeInfo(pit.key()).cVariables;
            if(pit.key() == "NodeSignalIn" || pit.key() == "NodeSignalOut")
            {
                varNames << mPortNameLineEditPtrs[np-1]->text();
            }
            for(int i=0; i<pit.value().size(); ++i)
            {
                numStr = QString::number(np);
                if(pit.key() == "NodeSignalIn" || pit.key() == "NodeSignalOut")
                {
                    numStr = "";
                }
                for(int v=0; v<varNames.size(); ++v)
                {
                    output.append(varNames[v]+numStr+", ");
                }
                ++np;
            }
        }
        output.chop(2);
        output.append(";\n");

        //Parameter variables
        if(!mParameterNameLineEditPtrs.isEmpty())
        {
            output.append("    double ");
            for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
            {
                output.append(mParameterNameLineEditPtrs[p]->text()+", ");
            }
            output.chop(2);
            output.append(";\n");
        }

        //Node data pointers
        output.append("    double ");
        np = 1;
        for(pit=nodeToPortMap.begin(); pit!=nodeToPortMap.end(); ++pit)
        {
            QStringList varNames;
            varNames << NodeInfo(pit.key()).qVariables << NodeInfo(pit.key()).cVariables;
            QString numStr;

            for(int i=0; i<pit.value().size(); ++i)
            {
                if(pit.key() == "NodeSignalIn" || pit.key() == "NodeSignalOut")
                {
                    varNames.clear();
                    varNames << mPortNameLineEditPtrs[np-1]->text();
                }
                if(pit.key() != "NodeSignalIn" && pit.key() != "NodeSignalOut")
                {
                    numStr = QString::number(np);
                }
                for(int v=0; v<varNames.size(); ++v)
                {
                    output.append("*mpND_"+varNames[v]+numStr+", ");
                }
                ++np;
            }
        }
        output.chop(2);
        output.append(";\n");

        //Port pointers
        output.append("    Port ");
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            output.append("*mp"+mPortNameLineEditPtrs[p]->text()+", ");
        }
        output.chop(2);
        output.append(";\n\n");

        //Creator function
        output.append("public:\n");
        output.append("    static Component *Creator()\n");
        output.append("    {\n");
        output.append("        return new "+typeName+"();\n");
        output.append("    }\n\n");

        //Contructor
        output.append("    "+typeName+"() : Component"+cqsTypeLong+"()\n");
        output.append("    {\n");

        //Initiate parameters
        for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
        {
            QString name = mParameterNameLineEditPtrs[p]->text();
            QString value = mParameterValueLineEditPtrs[p]->text();
            output.append("        "+name+" = "+value+";\n");
        }
        if(!mParameterNameLineEditPtrs.isEmpty()) { output.append("\n"); }

        //Register parameters
        for(int p=0; p<mParameterNameLineEditPtrs.size(); ++p)
        {
            QString name = mParameterNameLineEditPtrs[p]->text();
            QString desc = mParameterDescriptionLineEditPtrs[p]->text();
            QString unit = mParameterUnitLineEditPtrs[p]->text();
            output.append("        registerParameter(\""+name+"\", \""+desc+"\", \""+unit+"\", "+name+");\n");
        }
        if(!mParameterNameLineEditPtrs.isEmpty()) { output.append("\n"); }

        QStringList portNames, portTypes, nodeTypes;

        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            portNames << mPortNameLineEditPtrs[p]->text();
            QString type = mPortTypeComboBoxPtrs[p]->currentText();

            if(type      == "Signal Input")             { portTypes << "ReadPort"; }
            else if(type == "Signal Output")            { portTypes << "WritePort"; }
            else                                        { portTypes << "PowerPort"; }

            if(type      == "Signal Input")             { nodeTypes << "NodeSignal"; }
            else if(type == "Signal Output")            { nodeTypes << "NodeSignal"; }
            QStringList allNodeTypes;
            NodeInfo::getNodeTypes(allNodeTypes);
            Q_FOREACH(const QString &t, allNodeTypes)
            {
                if(type.toLower() == NodeInfo(t).niceName)
                    nodeTypes << t;
            }

            output.append("        mp"+portNames[p]+" = add"+portTypes[p]+"(\""+portNames[p]+"\", \""+nodeTypes[p]+"\");\n");
        }
        output.append("    }\n\n");

        //Initialize
        output.append("    void initialize()\n");
        output.append("    {\n");

        //Assign node data pointers
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(nodeTypes[p] != "NodeSignal")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables << NodeInfo(nodeTypes[p]).cVariables;
            }
            else
            {
                if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
                if(portTypes[p] == "WritePort") { varNames << portNames[p]; }
            }

            QStringList varLabels = NodeInfo(nodeTypes[p]).variableLabels;
            QString numStr, defaultValue;
            if(portTypes[p] != "ReadPort" && portTypes[p] != "WritePort")
            {
                numStr = QString::number(p+1);
            }
            else
            {
                defaultValue = ", "+mPortDefaultSpinBoxPtrs[p]->text();
            }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        mpND_"+varNames[v]+numStr+" = getSafeNodeDataPtr(mp"+portNames[p]+", "+nodeTypes[p]+"::"+varLabels[v]+defaultValue+");\n");
            }
        }
        output.append("\n");

        //Create local port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            varNames << NodeInfo(nodeTypes[p]).qVariables << NodeInfo(nodeTypes[p]).cVariables;
            if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "ReadPort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        "+varNames[v]+numStr+" = (*mpND_"+varNames[v]+numStr+");\n");
            }
        }
        output.append("\n");

        output.append("        //WRITE INITIALIZATION HERE\n");
        output.append("    }\n\n");


        //SimulateOneTimestep
        output.append("    void simulateOneTimestep()\n");
        output.append("    {\n");

        //Create local input port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(cqsType == "Q")
            {
                varNames << NodeInfo(nodeTypes[p]).cVariables;
            }
            else if(cqsType == "C")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables;
            }
            if(portTypes[p] == "ReadPort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "ReadPort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        "+varNames[v]+numStr+" = (*mpND_"+varNames[v]+numStr+");\n");
            }
        }
        output.append("\n");

        output.append("        //WRITE SIMULATION CODE HERE\n\n");

        //Create local output port variables
        for(int p=0; p<mPortNameLineEditPtrs.size(); ++p)
        {
            QStringList varNames;
            if(cqsType == "C")
            {
                varNames << NodeInfo(nodeTypes[p]).cVariables;
            }
            else if(cqsType == "Q")
            {
                varNames << NodeInfo(nodeTypes[p]).qVariables;
            }
            if(portTypes[p] == "WritePort") { varNames << portNames[p]; }
            QString numStr;
            if(portTypes[p] != "WritePort") { numStr = QString::number(p+1); }

            for(int v=0; v<varNames.size(); ++v)
            {
                output.append("        (*mpND_"+varNames[v]+numStr+") = "+varNames[v]+numStr+";\n");
            }
        }
        output.append("\n");

        output.append("    }\n\n");

        //Finalize
        output.append("    void finalize()\n");
        output.append("    {\n");
        output.append("        //WRITE FINALIZE CODE HERE\n");
        output.append("    }\n");

        output.append("};\n");
        output.append("}\n");
    }

    mpParent->addNewTab(output);

    this->deleteLater();
}
