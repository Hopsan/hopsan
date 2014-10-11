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
//! @file   ComponentPropertiesDialog3.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

//Qt includes
#include <QDebug>
#include <QMessageBox>
#include <QScrollArea>
#include <QApplication>
#include <QDesktopWidget>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QMenu>


//Hopsan includes
#include "common.h"
#include "global.h"
#include "ComponentPropertiesDialog3.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/EditComponentDialog.h"
#include "Dialogs/MovePortsDialog.h"
#include "Dialogs/ParameterSettingsLayout.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/HighlightingUtilities.h"
#include "LibraryHandler.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"


//! @class ComponentPropertiesDialog3
//! @brief The ComponentPropertiesDialog3 class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!





//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog3::ComponentPropertiesDialog3(ModelObject *pModelObject, QWidget *pParent)
    : QDialog(pParent)
{
    mpModelObject = pModelObject;

    if(mpModelObject->getTypeName() == MODELICATYPENAME && false)    //! @todo Temporarily disabled for Modelica experiments, DO NOT MERGE
    {
        this->hide();

        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Modelica, gpMainWindowWidget);
        pEditDialog->exec();

        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess;
            QString typeName = pEditDialog->getCode().section("model ", 1, 1).section(" ",0,0);
            QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            QDir().mkpath(libPath);
            int solver = pEditDialog->getSolver();

            QFile moFile(libPath+typeName+".mo");
            moFile.open(QFile::WriteOnly | QFile::Truncate);
            moFile.write(pEditDialog->getCode().toUtf8());
            moFile.close();

            coreAccess.generateFromModelica(libPath+typeName+".mo", solver, true);
            gpLibraryHandler->loadLibrary(libPath);

            mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
        }
        delete(pEditDialog);

        this->close();
        return;
    }
    else if(mpModelObject->getTypeName() == "CppComponent")
    {
        this->hide();

        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Cpp, gpMainWindowWidget);
        pEditDialog->exec();

        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess;
            QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);

            QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            QDir().mkpath(libPath);

            QFile hppFile(libPath+typeName+".hpp");
            hppFile.open(QFile::WriteOnly | QFile::Truncate);
            hppFile.write(pEditDialog->getCode().toUtf8());
            hppFile.close();

            coreAccess.generateFromCpp(libPath+typeName+".hpp");
            coreAccess.generateLibrary(libPath, QStringList() << typeName+".hpp");
            coreAccess.compileComponentLibrary(libPath+typeName+"_lib.xml");
            gpLibraryHandler->loadLibrary(libPath+typeName+"_lib.xml");

            mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
        }
        delete(pEditDialog);

        this->close();
        return;
    }

    this->setPalette(gpConfig->getPalette());
    setWindowTitle(tr("Component Properties"));
    createEditStuff();

    connect(this, SIGNAL(lockModelWidget(bool)), mpModelObject->getParentContainerObject()->mpModelWidget, SLOT(lockTab(bool)));

    // Lock model for changes
    emit lockModelWidget(true);
}


//! @brief Verifies that a parameter value does not begin with a number but still contains illegal characters.
//! @note This is a temporary solution. It shall be removed when parsing equations as parameters works.
bool VariableTableWidget::cleanAndVerifyParameterValue(QString &rValue, const QString type)
{
    //! @todo I think CORE should handle all of this stuff
    QString tempVal = rValue;
    QStringList sysParamNames = mpModelObject->getParentContainerObject()->getParameterNames();
    QString error;

    if(verifyParameterValue(tempVal, type, sysParamNames, error))
    {
        // Set corrected text
        rValue = tempVal;
        return true;
    }
    else
    {
        QMessageBox::critical(gpMainWindowWidget, "Error", error.append(" Resetting parameter value!"));
        return false;
    }
}

bool VariableTableWidget::setAliasName(const int row)
{
    // Skip separator rows
    if (columnSpan(row,0)>2)
    {
        return true;
    }

    QString alias = item(row,VariableTableWidget::Alias)->text();
    //! @todo since alias=empty means unregister we can not skip it, but there should be some check if a variable has changed, so that we do not need to go thourgh set for all variables every freeking time
    QString name = item(row,VariableTableWidget::Name)->toolTip();
    QStringList parts = name.split("#");
    if (parts.size() == 2)
    {
        //! @todo look over this name stuff
        mpModelObject->getParentContainerObject()->setVariableAlias(mpModelObject->getName()+"#"+parts[0]+"#"+parts[1], alias);
    }
    return true;
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog3::okPressed()
{
    setName();
    setAliasNames();
    if (setVariableValues())
    {
        close();
    }
}


void ComponentPropertiesDialog3::editPortPos()
{
    //! @todo who owns the dialog, is it ever removed?
    MovePortsDialog *dialog = new MovePortsDialog(mpModelObject->getAppearanceData(), mpModelObject->getLibraryAppearanceData(), mpModelObject->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpModelObject, SLOT(refreshExternalPortsAppearanceAndPosition()), Qt::UniqueConnection);
}

void ComponentPropertiesDialog3::copyToNewComponent()
{
    QString sourceCode = mpSourceCodeTextEdit->toPlainText();

    QDateTime time = QDateTime();
    uint t = time.currentDateTime().toTime_t();     //Number of milliseconds since 1970
    double rd = rand() / (double)RAND_MAX;
    int r = int(rd*1000000.0);                      //Random number between 0 and 1000000
    QString randomName = mpModelObject->getTypeName()+QString::number(t)+QString::number(r);


    sourceCode.replace("#ifdef "+mpModelObject->getTypeName().toUpper()+"_HPP_INCLUDED", "");
    sourceCode.replace("#define "+mpModelObject->getTypeName().toUpper()+"_HPP_INCLUDED", "");
    sourceCode.replace("#endif //  "+mpModelObject->getTypeName().toUpper()+"_HPP_INCLUDED", "");
    sourceCode.replace("class "+mpModelObject->getTypeName()+" :", "class "+randomName+" :");
    sourceCode.replace("return new "+mpModelObject->getTypeName()+"()", "return new "+randomName+"()");

    EditComponentDialog *pEditDialog = new EditComponentDialog(sourceCode, EditComponentDialog::Cpp, gpMainWindowWidget);

    pEditDialog->exec();

    if(pEditDialog->result() == QDialog::Accepted)
    {
        CoreGeneratorAccess coreAccess;
        QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);
        QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
        QString libPath = dummy+typeName+"/";
        QDir().mkpath(libPath);

        QFile hppFile(libPath+typeName+".hpp");
        hppFile.open(QFile::WriteOnly | QFile::Truncate);
        hppFile.write(pEditDialog->getCode().toUtf8());
        hppFile.close();

        coreAccess.generateFromCpp(libPath+typeName+".hpp");
        coreAccess.generateLibrary(libPath, QStringList() << typeName+".hpp");
        coreAccess.compileComponentLibrary(libPath+typeName+"_lib.xml");
        gpLibraryHandler->loadLibrary(libPath+typeName+"_lib.xml");
        mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
    }
    delete(pEditDialog);
}

void ComponentPropertiesDialog3::recompile()
{
    QString basePath = this->mpModelObject->getAppearanceData()->getBasePath();
    QString fileName = this->mpModelObject->getAppearanceData()->getSourceCodeFile();

    QString sourceCode = mpSourceCodeTextEdit->toPlainText();
    int solver = mpSolverComboBox->currentIndex();

    //Read source code from file
    QFile oldSourceFile(basePath+fileName);
    if(!oldSourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString oldSourceCode;
    while(!oldSourceFile.atEnd())
    {
        oldSourceCode.append(oldSourceFile.readLine());
    }
    oldSourceFile.close();

    QFile sourceFile(basePath+fileName);
    if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        return;
    sourceFile.write(sourceCode.toStdString().c_str());
    sourceFile.close();

    //Recompile with new code
    gpLibraryHandler->recompileLibrary(*gpLibraryHandler->getEntry(mpModelObject->getTypeName()).pLibrary, true, solver);

    this->close();
}

bool ComponentPropertiesDialog3::setAliasNames()
{
    return mpVariableTableWidget->setAliasNames();
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
bool ComponentPropertiesDialog3::setVariableValues()
{
    bool isOK = mpVariableTableWidget->setStartValues();
    isOK *= mpVariableTableWidget->setCustomPlotScaleValues();
    return isOK;
}

void ComponentPropertiesDialog3::setName()
{
    mpModelObject->getParentContainerObject()->renameModelObject(mpModelObject->getName(), mpNameEdit->text());
}

void ComponentPropertiesDialog3::closeEvent(QCloseEvent* event)
{
    emit lockModelWidget(false);
    QWidget::closeEvent(event);
}

void ComponentPropertiesDialog3::reject()
{
    emit lockModelWidget(false);
    QDialog::reject();
    QDialog::close();
}

QGridLayout* ComponentPropertiesDialog3::createNameAndTypeEdit()
{
    QGridLayout *pNameTypeLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ");
    mpNameEdit = new QLineEdit(mpModelObject->getName());
    QLabel *pCQSTypeLabel = new QLabel("CQS Type: \""+mpModelObject->getTypeCQS()+"\"");
    QLabel *pTypeNameLabel = new QLabel("Typename: \""+mpModelObject->getTypeName()+"\"");
    pNameTypeLayout->addWidget(pNameLabel,0,0);
    pNameTypeLayout->addWidget(mpNameEdit,0,1);
    pNameTypeLayout->addWidget(pTypeNameLabel,1,0,1,2);
    if (!mpModelObject->getSubTypeName().isEmpty())
    {
        QLabel *pSubTypeNameLabel = new QLabel("SubTypename: \""+mpModelObject->getSubTypeName()+"\"");
        pNameTypeLayout->addWidget(pSubTypeNameLabel,2,0,1,2);
        pNameTypeLayout->addWidget(pCQSTypeLabel,3,0,1,2);
    }
    else
    {
        pNameTypeLayout->addWidget(pCQSTypeLabel,2,0,1,2);
    }

    return pNameTypeLayout;
}

QDialogButtonBox* ComponentPropertiesDialog3::createButtonBox()
{
    QPushButton *pEditPortPos = new QPushButton(tr("&Move ports"), this);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pOkButton = new QPushButton(tr("&Ok"), this);
    pOkButton->setDefault(true);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pEditPortPos, QDialogButtonBox::ActionRole);
    connect(pOkButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pEditPortPos, SIGNAL(clicked()), this, SLOT(editPortPos()));
    return pButtonBox;
}

QWidget *ComponentPropertiesDialog3::createHelpWidget()
{
    if(!mpModelObject->getHelpText().isNull() || !mpModelObject->getHelpPicture().isNull() || !mpModelObject->getHelpLink().isNull())
    {
        QScrollArea *pHelpScrollArea = new QScrollArea();
        QGroupBox *pHelpWidget = new QGroupBox();
        QVBoxLayout *pHelpLayout = new QVBoxLayout(pHelpWidget);

        QLabel *pHelpHeading = new QLabel(gpLibraryHandler->getModelObjectAppearancePtr(mpModelObject->getTypeName())->getDisplayName());
        pHelpHeading->setAlignment(Qt::AlignCenter);
        QFont tempFont = pHelpHeading->font();
        tempFont.setPixelSize(16);
        tempFont.setBold(true);
        pHelpHeading->setFont(tempFont);
        pHelpLayout->addWidget(pHelpHeading);

        if(!mpModelObject->getHelpPicture().isNull())
        {
            QLabel *pHelpPicture = new QLabel();
            QPixmap helpPixMap(mpModelObject->getAppearanceData()->getBasePath() + mpModelObject->getHelpPicture());
            pHelpPicture->setPixmap(helpPixMap);
            pHelpLayout->addWidget(pHelpPicture);
            pHelpPicture->setAlignment(Qt::AlignHCenter);
        }

        if(!mpModelObject->getHelpText().isNull())
        {
            QLabel *pHelpText = new QLabel(mpModelObject->getHelpText(), this);
            pHelpText->setWordWrap(true);
            pHelpLayout->addWidget(pHelpText);
            pHelpText->setAlignment(Qt::AlignHCenter);
        }

        if (!mpModelObject->getHelpLink().isNull())
        {
            QString link = QString("<a href=\"%1\">%2</a>").arg(mpModelObject->getAppearanceData()->getBasePath()+mpModelObject->getHelpLink()).arg(mpModelObject->getHelpLink());
            QLabel *pHelpLink = new QLabel(link,this);
            pHelpLink->setOpenExternalLinks(true);
            pHelpLayout->addWidget(pHelpLink);
            pHelpLink->setAlignment(Qt::AlignHCenter);
        }

        pHelpWidget->setStyleSheet(QString::fromUtf8("QGroupBox {background-color: white; border: 2px solid gray; border-radius: 5px;}"));

        pHelpWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        //pHelpLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        pHelpScrollArea->setWidget(pHelpWidget);
        pHelpScrollArea->setWidgetResizable(true);
        pHelpScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        return pHelpScrollArea;
    }
    return 0;
}

QWidget *ComponentPropertiesDialog3::createSourcodeBrowser(QString &rFilePath)
{
    rFilePath.prepend(mpModelObject->getAppearanceData()->getBasePath());
    QFile file(rFilePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString code;
    QTextStream t(&file);
    code = t.readAll();
    file.close();

    mpSourceCodeTextEdit = new QTextEdit();
    mpSourceCodeTextEdit->setReadOnly(false);
    mpSourceCodeTextEdit->setText(code);
    if(rFilePath.endsWith(".hpp"))
    {
        //! @todo who own and who deleats
        CppHighlighter *pHighLighter = new CppHighlighter(mpSourceCodeTextEdit->document());
        Q_UNUSED(pHighLighter);
    }
    else if(rFilePath.endsWith(".mo"))
    {
        ModelicaHighlighter *pHighLighter = new ModelicaHighlighter(mpSourceCodeTextEdit->document());
        Q_UNUSED(pHighLighter);
    }

    QWidget *pTempWidget = new QWidget(this);
    QVBoxLayout *pLayout = new QVBoxLayout(pTempWidget);
    pLayout->addWidget(mpSourceCodeTextEdit);

    QLabel *pSolverLabel = new QLabel("Solver: ", this);
    mpSolverComboBox = new QComboBox(this);
    mpSolverComboBox->addItem("Numerical Integration");
    mpSolverComboBox->addItem("Bilinear Transform");
    mpNewComponentButton = new QPushButton(tr("&Copy to new component"), this);
    connect(mpNewComponentButton, SIGNAL(clicked()), this, SLOT(copyToNewComponent()));
    mpRecompileButton = new QPushButton(tr("&Recompile"), this);
    mpRecompileButton->setEnabled(true);
    connect(mpRecompileButton, SIGNAL(clicked()), this, SLOT(recompile()));
    QHBoxLayout *pSolverLayout = new QHBoxLayout();
    pSolverLayout->addWidget(pSolverLabel);
    pSolverLayout->addWidget(mpSolverComboBox);
    pSolverLayout->addWidget(new QWidget(this));
    pSolverLayout->addWidget(mpNewComponentButton);
    pSolverLayout->addWidget(mpRecompileButton);
    pSolverLayout->setStretch(2,1);
    pLayout->addLayout(pSolverLayout);

    return pTempWidget;//return pSourceCodeTextEdit;
}

void ComponentPropertiesDialog3::createEditStuff()
{
    QGridLayout* pPropertiesLayout = new QGridLayout(this);
    int row=0;

    // Parents to new objects bellow should be set automatically when adding layout or widget to other layout or widget

    // Add help picture and text
    //------------------------------------------------------------------------------------------------------------------------------
    QWidget *pHelpBoxWidget = createHelpWidget();
    QWidget *pDummyWidget = new QWidget(this);
    QVBoxLayout *pHelpLayout = new QVBoxLayout();
    pHelpLayout->addWidget(pHelpBoxWidget);
    pHelpLayout->addWidget(pDummyWidget);
    pHelpLayout->setStretch(1,1);
    QWidget *pHelpWidget = new QWidget(this);
    pHelpWidget->setLayout(pHelpLayout);
    //------------------------------------------------------------------------------------------------------------------------------

    // Add name edit and type information
    //------------------------------------------------------------------------------------------------------------------------------
    QGridLayout *pNameTypeLayout = createNameAndTypeEdit();
    pPropertiesLayout->addLayout(pNameTypeLayout, row, 0, Qt::AlignLeft);
    pPropertiesLayout->setRowStretch(row,0);
    //------------------------------------------------------------------------------------------------------------------------------

    // Add button box with buttons
    //------------------------------------------------------------------------------------------------------------------------------
    QDialogButtonBox *pButtonBox = createButtonBox();
    pPropertiesLayout->addWidget(pButtonBox, row, 1);
    //------------------------------------------------------------------------------------------------------------------------------
    ++row;

    // Add Parameter settings table
    //------------------------------------------------------------------------------------------------------------------------------
    mpVariableTableWidget = new VariableTableWidget(mpModelObject,this);
    mpVariableTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pPropertiesLayout->addWidget(mpVariableTableWidget, row, 0,1,2);
    pPropertiesLayout->setRowStretch(row,1);
    qDebug() << "Table: " << mpVariableTableWidget->sizeHint() << "  " << mpVariableTableWidget->minimumWidth() << "  " << mpVariableTableWidget->minimumHeight();
    //------------------------------------------------------------------------------------------------------------------------------

    QWidget *pPropertiesWidget = new QWidget(this);
    pPropertiesWidget->setLayout(pPropertiesLayout);
    pPropertiesWidget->setPalette(gpConfig->getPalette());

    // Add Code edit stuff, A new tab in a new widget will be created
    //------------------------------------------------------------------------------------------------------------------------------
    QString filePath = mpModelObject->getAppearanceData()->getSourceCodeFile();

    QGridLayout *pMainLayout = new QGridLayout(this);

    QTabWidget *pTabWidget = new QTabWidget(this);
    pTabWidget->addTab(pPropertiesWidget, "Properties");
    pTabWidget->addTab(pHelpWidget, "Description");
    if(!filePath.isEmpty())
    {
        QWidget* pSourceBrowser = createSourcodeBrowser(filePath);
        pTabWidget->addTab(pSourceBrowser, "Source Code");
    }
    pMainLayout->addWidget(pTabWidget);

    //------------------------------------------------------------------------------------------------------------------------------

    setLayout(pMainLayout);

    // Avoid a dialog that is higher than the availible space
    //! @todo this prevents fullscreen mode ,maybe limit should be fullscrean height
    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
    this->setMaximumHeight(maxHeight);
}

VariableTableWidget::VariableTableWidget(ModelObject *pModelObject, QWidget *pParent) :
    TableWidgetTotalSize(pParent)
{
    mpModelObject = pModelObject;

    this->setColumnCount(NumCols);
    QStringList columnHeaders;
    columnHeaders.append("Name");
    columnHeaders.append("Alias");
    columnHeaders.append("Description");
    columnHeaders.append("Unit");
    columnHeaders.append("Value");
    columnHeaders.append("PlotScale");
    columnHeaders.append("Port");
    this->setHorizontalHeaderLabels(columnHeaders);

    // Decide which parameters should be shown as Constants
    QVector<CoreParameterData> parameters;
    mpModelObject->getParameters(parameters);
    QVector<int> constantsIds;
    for (int i=0; i<parameters.size(); ++i)
    {
        if (!parameters[i].mName.contains("#"))
        {
            constantsIds.push_back(i);
        }
    }

    // Write the constants first
    int r=0;
    if (!constantsIds.isEmpty())
    {
        createSeparatorRow(r,"Constants", Constant);
        ++r;
    }
    for (int i=0; i<constantsIds.size(); ++i)
    {
        CoreVariameterDescription variameter;
        variameter.mName = parameters[constantsIds[i]].mName;
        variameter.mDescription = parameters[constantsIds[i]].mDescription;
        variameter.mUnit = parameters[constantsIds[i]].mUnit;
        variameter.mDataType = parameters[constantsIds[i]].mType;
        variameter.mConditions = parameters[constantsIds[i]].mConditions;
        createTableRow(r, variameter, Constant);
        ++r;
    }

    // Now fetch and write the input variables
    QVector<CoreVariameterDescription> variameters;
    mpModelObject->getVariameterDescriptions(variameters);

    // Write inputVariables
    const int inputVarSeparatorRow = r;
    for (int i=0; i<variameters.size(); ++i)
    {
        if (variameters[i].mVariameterType == InputVaraiable)
        {
            createTableRow(r, variameters[i], InputVaraiable);
            ++r;
        }
    }
    // Insert the separator row if there were any input variables)
    if (r != inputVarSeparatorRow)
    {
        createSeparatorRow(inputVarSeparatorRow,"InputVariables", InputVaraiable);
        ++r;
    }

    // Write outputVariables
    const int outputVarSeparatorRow = r;
    for (int i=0; i<variameters.size(); ++i)
    {
        if (variameters[i].mVariameterType == OutputVariable)
        {
            createTableRow(r, variameters[i], OutputVariable);
            ++r;
        }
    }
    // Insert the separator row if there were any output variables)
    if (r != outputVarSeparatorRow)
    {
        createSeparatorRow(outputVarSeparatorRow,"OutputVariables", OutputVariable);
        ++r;
    }

    // Write remaning port variables
    QString currPortName;
    for (int i=0; i<variameters.size(); ++i)
    {
        if ( (variameters[i].mVariameterType == OtherVariable) &&
             ( gpConfig->getShowHiddenNodeDataVariables() || (variameters[i].mVariabelType != "Hidden") ) )
        {
            // Extract current port name to see if we should make a separator
            QString portName = variameters[i].mPortName;
            if (portName != currPortName)
            {
                currPortName = portName;
                Port* pPort = mpModelObject->getPort(portName);
                if (pPort)
                {
                    QString desc = pPort->getPortDescription();
                    if (!desc.isEmpty())
                    {
                        portName.append("     "+desc);
                    }
                }
                createSeparatorRow(r,"Port: "+portName, OtherVariable);
                ++r;
            }

            createTableRow(r, variameters[i], OtherVariable);
            ++r;
        }
    }

    resizeColumnToContents(Name);
    resizeColumnToContents(Unit);
    resizeColumnToContents(Value);
    resizeColumnToContents(Scale);
    resizeColumnToContents(ShowPort);
    setColumnWidth(Description, 2*columnWidth(Description));

#if QT_VERSION >= 0x050000
    horizontalHeader()->setSectionResizeMode(Name, QHeaderView::ResizeToContents);
//    horizontalHeader()->setSectionResizeMode(Alias, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(Description, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(Value, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(Unit, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(Scale, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(ShowPort, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionsClickable(false);
#else
    horizontalHeader()->setResizeMode(Name, QHeaderView::ResizeToContents);
//    horizontalHeader()->setResizeMode(Alias, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Description, QHeaderView::Stretch);
    horizontalHeader()->setResizeMode(Value, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Unit, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Scale, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(ShowPort, QHeaderView::ResizeToContents);
    horizontalHeader()->setClickable(false);
#endif

    verticalHeader()->hide();
}

bool VariableTableWidget::setStartValues()
{
    //Hack that will make sure all values currently being edited is set before using them
    setDisabled(true);
    setEnabled(true);

    bool addedUndoPost = false;
    bool allok=true;
    for (int row=0; row<rowCount(); ++row)
    {

        // First check if row is separator, then skip it
        if (columnSpan(row,0)>1)
        {
            continue;
        }

        // If startvalue is empty (disabled, then we should not atempt to change it)
        bool isDisabled = qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)))->isValueDisabled();
        if (isDisabled)
        {
            continue;
        }

        ParameterValueSelectionWidget *pValueWideget = qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)));
        // Extract name and value from row
        QString name = pValueWideget->getName();
        QString value = pValueWideget->getValueText();

        if(mpModelObject->getTypeName() == MODELICATYPENAME && name != "model")
        {
            continue;
        }

        // Check if we have new custom scaling
        UnitScale customUnitScale;
        UnitSelectionWidget *pUnitWidget = pValueWideget->getUnitSelectionWidget();
        if (pUnitWidget)
        {
            bool isDouble=false;
            value.toDouble(&isDouble);

            // Unregister unitscale if default unit has been reset or if the value is not a number (but a parameter name, (Actually a string))
            if (pUnitWidget->isDefaultSelected() || !isDouble)
            {
                mpModelObject->unregisterCustomParameterUnitScale(name);
            }
            // Else register (remember) the new unit scale
            else
            {
                pUnitWidget->getSelectedUnitScale(customUnitScale);
                mpModelObject->registerCustomParameterUnitScale(name, customUnitScale);
            }
        }

        // Get the old value to see if a changed has occured
        QString oldValue = mpModelObject->getParameterValue(name);
        if (oldValue != value)
        {
            // If we have a custom unit scale then undo the scale and set a value expressed in the default unit
            if (!customUnitScale.isEmpty())
            {
                value = customUnitScale.invRescale(value);
            }

            // Parameter has changed, add to undo stack and set the parameter
            if( cleanAndVerifyParameterValue(value, qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)))->getDataType()) )
            {
                // If we fail to set the parameter, then warning box and reset value
                if(!mpModelObject->setParameterValue(name, value))
                {
                    QMessageBox::critical(0, "Hopsan GUI", QString("'%1' is an invalid value for parameter '%2'. Resetting old value '%3'!").arg(value).arg(name).arg(oldValue));
                    // Reset old value
                    qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)))->setValueText(oldValue);
                    allok = false;
                    break;
                }
                else
                {
                    // Add an undo post (but only one for all values changed this time
                    if(!addedUndoPost)
                    {
                        mpModelObject->getParentContainerObject()->getUndoStackPtr()->newPost(UNDO_CHANGEDPARAMETERS);
                        addedUndoPost = true;
                    }
                    // Register the change in undo stack
                    mpModelObject->getParentContainerObject()->getUndoStackPtr()->registerChangedParameter(mpModelObject->getName(),
                                                                                                           name,
                                                                                                           oldValue,
                                                                                                           value);
                    // Mark project tab as changed
                    mpModelObject->getParentContainerObject()->hasChanged();
                }
            }
            else
            {
                // Reset old value
                qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)))->setValueText(oldValue);
                allok = false;
                break;
            }
        }
        //! @todo if we set som ok but som later fail pressing cancel will not restore the first that were set ok /Peter
    }
    return allok;
}

bool VariableTableWidget::setCustomPlotScaleValues()
{
    bool allok=true;
    for (int row=0; row<rowCount(); ++row)
    {

        // First check if row is separator, then skip it
        if (columnSpan(row,0)>1)
        {
            continue;
        }

        // Extract PlotScaleSelector from row
        PlotScaleSelectionWidget *pPlotScaleSelector = qobject_cast<PlotScaleSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Scale)));
        if (!pPlotScaleSelector)
        {
            continue;
        }

        // Only register if changed
        if (pPlotScaleSelector->hasChanged())
        {
            pPlotScaleSelector->registerCustomScale();
        }
    }
    return allok;
}

bool VariableTableWidget::setAliasNames()
{
    for (int r=0; r<rowCount(); ++r)
    {
        if (!setAliasName(r))
        {
            return false;
        }
    }
    return true;
}

bool VariableTableWidget::focusNextPrevChild(bool next)
{
    bool retval = QTableWidget::focusNextPrevChild(next);

    int col = currentColumn();
    int row = currentRow();

    //Skip non-editable columns and separator rows
    if(columnSpan(row,0)>2 && next && row != rowCount()-1)
    {
        ++row;
        setCurrentCell(row,col);
    }
    else if(columnSpan(row,0)>2 && !next && row != 0)
    {
        --row;
        setCurrentCell(row,col);
    }

    while(!columnSpan(row,0)>2 && (col == Description || col == Unit || col == NumCols))
    {
        retval = focusNextPrevChild(next);
        col = currentColumn();
        row = currentRow();
    }

    QModelIndex idx = currentIndex();
    QWidget *pIndexWidget = this->indexWidget(idx);

    if(row == 0)
    {
        return retval;
    }
    else if(currentColumn() == Value)
    {
        ParameterValueSelectionWidget *pParWidget = qobject_cast<ParameterValueSelectionWidget*>(pIndexWidget);
        if(pParWidget)
        {
            pParWidget->getValueEditPtr()->setFocus();
        }
    }
    else if(currentColumn() == Scale)
    {
        PlotScaleSelectionWidget *pScaleWidget = qobject_cast<PlotScaleSelectionWidget*>(pIndexWidget);
        pScaleWidget->getPlotScaleEditPtr()->setFocus();
    }
    else if(currentColumn() == ShowPort)
    {
        HideShowPortWidget *pPortWidget = qobject_cast<HideShowPortWidget*>(pIndexWidget);
        pPortWidget->getCheckBoxPtr()->setFocus();
    }
    else
    {
        this->setFocus();
    }

    return retval;
}

void VariableTableWidget::createTableRow(const int row, const CoreVariameterDescription &rData, const VariameterTypEnumT variametertype)
{
    QString fullName, name;
    QTableWidgetItem *pItem;
    insertRow(row);

    //! @todo maybe store the variamter data objects localy, and check for hiden info there, would also make it possible to check for changes without asking core all of the time /Peter

    if (variametertype == Constant)
    {
        fullName = rData.mName;
        name = rData.mName;
    }
    else if (variametertype == OtherVariable)
    {
        if (rData.mPortName.isEmpty())
        {
            fullName = rData.mName;
        }
        else
        {
            fullName = rData.mPortName+"#"+rData.mName;
        }
        name = rData.mName;
    }
    else //For input and output variables
    {
        fullName = rData.mPortName+"#"+rData.mName;
        name = rData.mPortName;
    }

    // Set the name field
    pItem = new QTableWidgetItem(name);
    pItem->setToolTip(fullName);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Name,pItem);

    // Set the alias name field
    pItem = new QTableWidgetItem(rData.mAlias);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags);
    if (variametertype != Constant)
    {
        pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    }
    setItem(row,Alias,pItem);

    // Set the description field
    pItem = new QTableWidgetItem(rData.mDescription);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Description,pItem);

    // Set the unit field
    pItem = new QTableWidgetItem(rData.mUnit);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Unit,pItem);

    // Set the value field
    ParameterValueSelectionWidget *pValueWidget = new ParameterValueSelectionWidget(rData, variametertype, mpModelObject, this);
    this->setIndexWidget(model()->index(row,Value), pValueWidget);
    // Trigger signal to unit selector if syspar entered to disable the unit scrol box
    pValueWidget->checkIfSysParEntered();

    // Create the custom plot unit display and selection button
    if (variametertype != Constant)
    {
        QWidget *pPlotScaleWidget = new PlotScaleSelectionWidget(rData, mpModelObject, this);
        this->setIndexWidget(model()->index(row,Scale), pPlotScaleWidget);
    }
    else
    {
        pItem = new QTableWidgetItem();
        pItem->setFlags(Qt::NoItemFlags);
        setItem(row,Scale,pItem);
    }

    // Set the port hide/show button
    if ( (variametertype == InputVaraiable) || (variametertype == OutputVariable))
    {
        HideShowPortWidget *pWidget = new HideShowPortWidget(rData, mpModelObject, this);
        connect(pWidget, SIGNAL(toggled(bool)), pValueWidget, SLOT(refreshValueTextStyle()));
        this->setIndexWidget(model()->index(row,ShowPort), pWidget);
    }
    else
    {
        pItem = new QTableWidgetItem();
        pItem->setFlags(Qt::NoItemFlags);
        setItem(row,ShowPort,pItem);
    }
}

void VariableTableWidget::createSeparatorRow(const int row, const QString &rName, const VariameterTypEnumT variametertype)
{
    insertRow(row);

    QTableWidgetItem *pItem, *pItem2=0;
    pItem = new QTableWidgetItem(rName);
    pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    pItem->setBackgroundColor(Qt::lightGray);

    if ( variametertype == InputVaraiable )
    {
        pItem2 = new QTableWidgetItem("Default Value");
    }
    else if ( variametertype == OutputVariable )
    {
        pItem2 = new QTableWidgetItem("Start Value");
    }
    else if ( variametertype == OtherVariable )
    {
        pItem2 = new QTableWidgetItem("Start Value");
    }

    if (pItem2)
    {
        pItem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        pItem2->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
        pItem2->setBackgroundColor(Qt::lightGray);

        setSpan(row,Name,1,Value);
        setItem(row,Name,pItem);
        setSpan(row,Value,1,NumCols-Value);
        setItem(row,Value,pItem2);
    }
    else
    {
        setSpan(row,Name,1,NumCols);
        setItem(row,Name,pItem);
    }
    resizeRowToContents(row);
}

TableWidgetTotalSize::TableWidgetTotalSize(QWidget *pParent) : QTableWidget(pParent)
{
    mMaxVisibleRows=20;
}

QSize TableWidgetTotalSize::sizeHint() const
{
    int w=0;
    if (verticalHeader()->isVisible())
    {
        w += verticalHeader()->sizeHint().width();
    }
    w +=  + frameWidth()*2 + verticalScrollBar()->sizeHint().width();
    //qDebug() << "w: " << w << " lw: " << lineWidth() << "  mLw: " << midLineWidth() << "  frameWidth: " << frameWidth();
    //qDebug() << verticalScrollBar()->sizeHint().width();

    for (int c=0; c<columnCount(); ++c)
    {
        w += columnWidth(c);
    }

    int h = horizontalHeader()->sizeHint().height() + frameWidth()*2;
    for (int r=0; r<min(mMaxVisibleRows,rowCount()); ++r)
    {
        h += rowHeight(r);
    }
    return QSize(w, h);
}

void TableWidgetTotalSize::setMaxVisibleRows(const int maxRows)
{
    mMaxVisibleRows = maxRows;
}


PlotScaleSelectionWidget::PlotScaleSelectionWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent) :
    QWidget(pParent)
{
    mVariableTypeName = rData.mName;
    mVariablePortDataName = rData.mPortName+"#"+rData.mName;
    mOriginalUnit = rData.mUnit;
    mpModelObject = pModelObject;

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);

    mpPlotScaleEdit = new QLineEdit();
    mpPlotScaleEdit->setAlignment(Qt::AlignCenter);
    mpPlotScaleEdit->setFrame(false);
    pLayout->addWidget(mpPlotScaleEdit);

    UnitScale currCustom;
    pModelObject->getCustomPlotUnitOrScale(mVariablePortDataName, currCustom);
    if (!currCustom.mScale.isEmpty()) // Check if data exists
    {
        // If minus one scale then show -1
        if (currCustom.isMinusOne())
        {
            mpPlotScaleEdit->setText(currCustom.mScale);
        }
        // If unit not given, display the scale value
        else if (currCustom.mUnit.isEmpty())
        {
            mpPlotScaleEdit->setText(currCustom.mScale);
        }
        // If description given use it (usually the custom unit)
        else
        {
            mpPlotScaleEdit->setText(currCustom.mUnit);
        }

        //! @todo what about showinf manually set custom units like "1 [m]"
    }

    QToolButton *pScaleSelectionButton =  new QToolButton(this);
    pScaleSelectionButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
    pScaleSelectionButton->setToolTip("Select Unit Scaling");
    pScaleSelectionButton->setFixedSize(24,24);
    connect(pScaleSelectionButton, SIGNAL(clicked()), this, SLOT(createPlotScaleSelectionMenu()));
    pLayout->addWidget(pScaleSelectionButton);
}

void PlotScaleSelectionWidget::createPlotScaleSelectionMenu()
{
    QMap<QString, double> unitScales;
    QMenu menu;
    if (mVariableTypeName == "Value")
    {
        QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit(mOriginalUnit);
        if (pqs.size() > 0)
        {
            unitScales = gpConfig->getUnitScales(pqs.first());
        }
    }
    else
    {
        unitScales = gpConfig->getUnitScales(mVariableTypeName);
    }
    if (!unitScales.isEmpty())
    {
        QList<QString> keys = unitScales.keys();
        QMap<QAction*, int> actionScaleMap;

        for (int i=0; i<keys.size(); ++i)
        {
            QAction *tempAction = menu.addAction(keys[i]);
            actionScaleMap.insert(tempAction, i);
            tempAction->setIconVisibleInMenu(false);
        }

        //! @todo maybe add this
        //    if(!menu.isEmpty())
        //    {
        //        menu.addSeparator();
        //    }
        //    QAction *pAddAction = menu.addAction("Add global unit scale");
        QAction *pAddAction = 0;


        QCursor cursor;
        QAction *selectedAction = menu.exec(cursor.pos());
        if(selectedAction == pAddAction)
        {
            //! @todo maybe add this
            return;
        }

        int idx = actionScaleMap.value(selectedAction,-1);
        if (idx >= 0)
        {
            QString key =  keys.at(idx);
            if(!key.isEmpty())
            {
                // Set the selected unit scale
                //mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, key, QString("%1").arg(unitScales.value(key)));
                mpPlotScaleEdit->setText(key);
            }
        }
    }
}

void PlotScaleSelectionWidget::registerCustomScale()
{
    QMap<QString, double> unitScales;


    QString val = mpPlotScaleEdit->text();

    if (val=="-1" || val=="-1.0")
    {
        val = QString("-1 [%1]").arg(mOriginalUnit);
    }
    else
    {
        if (mVariableTypeName == "Value")
        {
            QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit(mOriginalUnit);
            if (pqs.size() > 0)
            {
                unitScales = gpConfig->getUnitScales(pqs.first());
            }
        }
        else
        {
            unitScales = gpConfig->getUnitScales(mVariableTypeName);
        }
    }

    if (unitScales.contains(val))
    {
        mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, val, QString("%1").arg(unitScales.value(val)));
    }
    else if (val.contains('['))
    {
        // Ok the user want to add a custom specific scale
        QStringList fields = val.split(' ');
        if (fields.size() == 2)
        {
            //! @todo need to check if text is valid number
            mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, fields.last().remove('[').remove(']'), fields.first());
        }
        else
        {
            //! @todo report error
        }
    }
    else
    {
        //! @todo need to check if text is valid number
        mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, "", val);
    }
}

bool PlotScaleSelectionWidget::hasChanged() const
{
    UnitScale us;
    mpModelObject->getCustomPlotUnitOrScale(mVariablePortDataName, us);
    if (us.mUnit.isEmpty())
    {
        return us.mScale != mpPlotScaleEdit->text();
    }
    else
    {
        return us.mUnit != mpPlotScaleEdit->text();
    }
}

QLineEdit *PlotScaleSelectionWidget::getPlotScaleEditPtr() const
{
    return mpPlotScaleEdit;
}


ParameterValueSelectionWidget::ParameterValueSelectionWidget(const CoreVariameterDescription &rData, VariableTableWidget::VariameterTypEnumT type, ModelObject *pModelObject, QWidget *pParent) :
    QWidget(pParent)
{
    mpValueEdit = 0;
    mpConditionalValueComboBox = 0;
    mpUnitSelectionWidget = 0;
    mDefaultUnitScale.setOnlyScale(1);
    mpModelObject = pModelObject;
    mVariameterType = type;
    mVariablePortName = rData.mPortName;

    //! @todo maybe store the variamter data objects localy, and check for hiden info there, would also make it possible to check for changes without asking core all of the time /Peter
    if (rData.mPortName.isEmpty())
    {
        mVariablePortDataName = rData.mName;
    }
    else
    {
        mVariablePortDataName = rData.mPortName+"#"+rData.mName;
    }
    mVariableDataType = rData.mDataType;

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);


    if (!rData.mDataType.isEmpty())
    {
        QLabel *pTypeLetter = new QLabel(rData.mDataType[0], this);
        pTypeLetter->setToolTip("DataType: "+rData.mDataType);
        pLayout->addWidget(pTypeLetter);
    }

    // Only set the rest if a value exist (it does not for disabled startvalues)
    if (mpModelObject->hasParameter(mVariablePortDataName))
    {
        QString value = mpModelObject->getParameterValue(mVariablePortDataName);

        mpValueEdit = new QLineEdit(this);
        mpValueEdit->setAlignment(Qt::AlignCenter);
        mpValueEdit->setFrame(false);

        if(mVariableDataType == "conditional")
        {
            mpConditionalValueComboBox = new QComboBox(this);
            for(int i=0; i<rData.mConditions.size(); ++i)
            {
                mpConditionalValueComboBox->addItem(rData.mConditions[i]);
            }
            mpConditionalValueComboBox->setCurrentIndex(value.toInt());
            mpValueEdit->setText(value);
            pLayout->addWidget(mpConditionalValueComboBox);
            connect(mpConditionalValueComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setConditionalValue(int)));
            mpValueEdit->hide();
        }
        else
        {
            pLayout->addWidget(mpValueEdit);
            UnitScale currentCustomUS;
            if (mpModelObject->getCustomParameterUnitScale(mVariablePortDataName, currentCustomUS))
            {
                mCustomScale = currentCustomUS;
                mpValueEdit->setText(mCustomScale.rescale(value));
            }
            else
            {
                mpValueEdit->setText(value);
            }
            connect(mpValueEdit, SIGNAL(editingFinished()), this, SLOT(setValue()));
            connect(mpValueEdit, SIGNAL(textChanged(QString)), this, SLOT(checkIfSysParEntered()));
            refreshValueTextStyle();

            // Set the unit field
            if (!rData.mUnit.isEmpty())
            {

                mDefaultUnitScale.setOnlyScale(1);
                if (!gpConfig->isRegisteredSIUnit(rData.mUnit))
                {
                    QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit(rData.mUnit);
                    if (pqs.size() > 1)
                    {
                        gpMessageHandler->addWarningMessage(QString("Multiple matches for custom unit scales for original unit: %1, wrong set may be shown!").arg(rData.mUnit), "WrongUSSet");
                    }

                    if (pqs.size() > 0)
                    {
                        UnitScale us;
                        gpConfig->getUnitScale(pqs.front(), rData.mUnit, us);
                        if (!us.isEmpty())
                        {
                            mDefaultUnitScale = us;
                        }
                    }
                }

                mpUnitSelectionWidget = new UnitSelectionWidget(rData.mUnit, this);
                mpUnitSelectionWidget->setUnitScaling(currentCustomUS);
                connect(mpUnitSelectionWidget, SIGNAL(unitChanged(UnitScale)), this, SLOT(rescaleByUnitScale(UnitScale)));
                pLayout->addWidget(mpUnitSelectionWidget);
            }
        }

        QToolButton *pResetButton =  new QToolButton(this);
        pResetButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
        pResetButton->setToolTip("Reset Default Value");
        pResetButton->setFixedSize(24,24);
        connect(pResetButton, SIGNAL(clicked()), this, SLOT(resetDefault()));
        pLayout->addWidget(pResetButton);

        if(mVariableDataType != "conditional")
        {
            QToolButton *pSystemParameterToolButton = new QToolButton(this);
            pSystemParameterToolButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
            pSystemParameterToolButton->setToolTip("Map To System Parameter");
            pSystemParameterToolButton->setFixedSize(24,24);
            connect(pSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(createSysParameterSelectionMenu()));
            pLayout->addWidget(pSystemParameterToolButton);
        }
    }
    else
    {
        QLabel *pLabel = new QLabel("Disabled",this);
        pLabel->setStyleSheet("color: Gray;");
        pLayout->addWidget(pLabel, 1, Qt::AlignCenter);
    }
}

void ParameterValueSelectionWidget::setValueText(const QString &rText)
{
    if (mpValueEdit)
    {
        mpValueEdit->setText(rText);
        if (mpConditionalValueComboBox)
        {
            mpConditionalValueComboBox->setCurrentIndex(rText.toInt());
        }
    }
}

QString ParameterValueSelectionWidget::getValueText() const
{
    if (mpValueEdit)
    {
        return mpValueEdit->text();
    }
    else
    {
        return QString();
    }
}

const QString &ParameterValueSelectionWidget::getDataType() const
{
    return mVariableDataType;
}

const QString &ParameterValueSelectionWidget::getName() const
{
    return mVariablePortDataName;
}

UnitSelectionWidget *ParameterValueSelectionWidget::getUnitSelectionWidget()
{
    return mpUnitSelectionWidget;
}

bool ParameterValueSelectionWidget::isValueDisabled() const
{
    return (mpValueEdit == 0);
}

QLineEdit *ParameterValueSelectionWidget::getValueEditPtr() const
{
    return mpValueEdit;
}

void ParameterValueSelectionWidget::setValue()
{
    //! @todo maybe do something here, would be nice if we could check if value is OK, or maybe we should even set the value here
    refreshValueTextStyle();
}

void ParameterValueSelectionWidget::setConditionalValue(const int idx)
{
    if (mpValueEdit)
    {
        mpValueEdit->setText(QString("%1").arg(idx));
    }
}

void ParameterValueSelectionWidget::resetDefault()
{
    // It is important to reset the unit before resetting the value
    // it will make any custom unit scale to be reset first
    if (mpUnitSelectionWidget)
    {
        mpUnitSelectionWidget->resetDefault();
    }

    if(mpModelObject && mpValueEdit)
    {
        QString defaultText = mpModelObject->getDefaultParameterValue(mVariablePortDataName);
        if(!defaultText.isEmpty())
        {
            mpValueEdit->setText(defaultText);
            setDefaultValueTextStyle();

            if (mpConditionalValueComboBox)
            {
                mpConditionalValueComboBox->setCurrentIndex(defaultText.toInt());
            }
        }
    }
}

void ParameterValueSelectionWidget::createSysParameterSelectionMenu()
{
    QMenu menu;
    QMap<QAction*, QString> actionParamMap;

    QVector<CoreParameterData> paramDataVector;
    mpModelObject->getParentContainerObject()->getParameters(paramDataVector);

    for (int i=0; i<paramDataVector.size(); ++i)
    {
        QAction *tempAction = menu.addAction(paramDataVector[i].mName+" = "+paramDataVector[i].mValue);
        tempAction->setIconVisibleInMenu(false);
        actionParamMap.insert(tempAction, paramDataVector[i].mName);
    }

    if(!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *pAddAction = menu.addAction("Add System Parameter");

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());
    if(selectedAction == pAddAction)
    {
        //! @todo maybe the gpSystemParametersWidget should not be global, should be one for each system
        gpSystemParametersWidget->openAddParameterDialog();
        return;
    }
    QString parNameString = actionParamMap.value(selectedAction);
    if(!parNameString.isEmpty())
    {
        mpValueEdit->setText(parNameString);
        refreshValueTextStyle();
    }
}

void ParameterValueSelectionWidget::refreshValueTextStyle()
{
    if(mpModelObject && mpValueEdit)
    {
        if( mpValueEdit->text() == mpModelObject->getDefaultParameterValue(mVariablePortDataName) )
        {
            setDefaultValueTextStyle();
        }
        else
        {
            QString style("color: black; font: bold;");
            decideBackgroundColor(style);
            mpValueEdit->setStyleSheet(style);
        }

    }
}

void ParameterValueSelectionWidget::rescaleByUnitScale(const UnitScale &rUnitScale)
{
    if (mpValueEdit)
    {
        bool isOK=false;
        mpValueEdit->text().toDouble(&isOK);
        QString valS = mpValueEdit->text();
        if (isOK)
        {
            // If we alreday have a custom scale then unconvert first
            if (!mCustomScale.isEmpty())
            {
                //val = val / mCustomScale.toDouble();
                valS = mDefaultUnitScale.rescale(mCustomScale.invRescale(valS));
            }
            // Now convert based on new scale values
            valS = rUnitScale.rescale(mDefaultUnitScale.invRescale(valS));

            // Set new value and remember new custom scale
            mpValueEdit->setText(QString("%1").arg(valS));
            mCustomScale = rUnitScale;
            refreshValueTextStyle();
        }
    }
}

bool ParameterValueSelectionWidget::checkIfSysParEntered()
{
    bool syspar = false;
    if (mpValueEdit)
    {
        syspar = mpModelObject->getParentContainerObject()->hasParameter(mpValueEdit->text());
    }

    if (mpUnitSelectionWidget)
    {
        if (syspar)
        {
            mpUnitSelectionWidget->resetDefault();
        }
        mpUnitSelectionWidget->setDisabled(syspar);
    }
    return syspar;
}

void ParameterValueSelectionWidget::setDefaultValueTextStyle()
{
    if (mpValueEdit)
    {
        QString style("color: black; font: normal;");
        decideBackgroundColor(style);
        mpValueEdit->setStyleSheet(style);
    }
}

void ParameterValueSelectionWidget::decideBackgroundColor(QString &rStyleString)
{
    if (mpValueEdit)
    {
        if (mVariameterType == VariableTableWidget::InputVaraiable)
        {
            Port *pPort = mpModelObject->getPort(mVariablePortName);
            if (pPort && pPort->isConnected())
            {
                rStyleString.append(" color: gray; background: LightGray; font: italic;");
                return;
            }
        }
        rStyleString.append(" background: white;");
    }
}


HideShowPortWidget::HideShowPortWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent) :
    QWidget(pParent)
{
    mpModelObject = pModelObject;
    mPortName = rData.mPortName;

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);
    mpCheckBox = new QCheckBox(this);
    pLayout->addWidget(mpCheckBox,Qt::AlignRight);

    mpCheckBox->setToolTip("Show/hide port");
    Port *pPort = mpModelObject->getPort(mPortName);
    mpCheckBox->setChecked((pPort && pPort->getPortAppearance()->mEnabled));
    connect(mpCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideShowPort(bool)));
    connect(mpCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
}

QCheckBox *HideShowPortWidget::getCheckBoxPtr() const
{
    return mpCheckBox;
}

void HideShowPortWidget::hideShowPort(const bool doShow)
{
    if (doShow)
    {
        Port *pPort = mpModelObject->getPort(mPortName);
        if (pPort)
        {
            pPort->setEnable(true);
            mpModelObject->createRefreshExternalPort(mPortName);
        }
        else
        {
            pPort = mpModelObject->createRefreshExternalPort(mPortName);
            if (pPort)
            {
                // Make sure that our new port has the "correct" angle
                //! @todo this is incorrect, can not have hardcoded 180
                pPort->setRotation(180);
                pPort->setModified(true);
            }
        }
    }
    else
    {
        mpModelObject->hideExternalDynamicParameterPort(mPortName);
    }
}


UnitSelectionWidget::UnitSelectionWidget(const QString &rDefaultUnit, QWidget *pParent) :
    QWidget(pParent)
{
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->setContentsMargins(0,0,0,0);
    mpUnitComboBox = 0;
    mDefaultUnit = rDefaultUnit;
    mDefaultIndex = -1;

    QStringList pqs = gpConfig->getPhysicalQuantitiesForUnit(mDefaultUnit);
    //! @todo if more then one is returned both will be shown, ther could be mixups, dont know how to handle that

    //! @todo this may show the wrong set of units
    if (pqs.size() > 1)
    {
        gpMessageHandler->addWarningMessage(QString("Multiple matches for custom unit scales for original unit: %1, There may be mixup!").arg(mDefaultUnit), "WrongUSSet");
        foreach(QString pq, pqs)
        {
            QMap<QString, double> map = gpConfig->getUnitScales(pq);
            QMap<QString, double>::iterator it;
            for (it=map.begin(); it!=map.end(); ++it)
            {
                mUnitScales.insert(QString("<%1> %2").arg(pq,it.key()), it.value());
            }
        }
    }
    else if (pqs.size() == 1)
    {
        mPhysicalQuantity = pqs.front();
        mUnitScales = gpConfig->getUnitScales(mPhysicalQuantity);
    }

    if (!mUnitScales.isEmpty())
    {
        mpUnitComboBox = new QComboBox(this);
        mpUnitComboBox->installEventFilter(new MouseWheelEventEater(this));
        mpUnitComboBox->setMinimumWidth(60);

        foreach(QString unit, mUnitScales.keys())
        {
            mpUnitComboBox->addItem(unit);
            if (mDefaultIndex < 0)
            {
                // Special handling if n pq's > 1
                if (pqs.size() > 1)
                {
                    double s = mUnitScales.value(unit);

                    unit.remove(0, unit.indexOf(">")+1);
                    unit = unit.trimmed();

                    // check if this is the default unit
                    if ( (mDefaultUnit == unit) && fuzzyEqual(s, 1.0, 1e-10) )
                    {
                        mDefaultIndex = mpUnitComboBox->count()-1;
                    }
                    //! @todo this may still fail as I do not really know the physical quantity
                }
                else
                {
                    // check if this is the default unit
                    if ( mDefaultUnit == unit )
                    {
                        mDefaultIndex = mpUnitComboBox->count()-1;
                    }
                }
            }
        }
        mpUnitComboBox->setCurrentIndex(mDefaultIndex);
        pLayout->addWidget(mpUnitComboBox);
        connect(mpUnitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionChanged(int)));
    }
}

void UnitSelectionWidget::setUnitScaling(const UnitScale &rUs)
{
    if (mpUnitComboBox && !rUs.isEmpty())
    {
        int i = mpUnitComboBox->findText(rUs.mUnit);
        // If we did not find anything then add <PhysicalQuantity> in front and try that
        if (i<0 && !rUs.mPhysicalQuantity.isEmpty())
        {
            i = mpUnitComboBox->findText(QString("<%1> %2").arg(rUs.mPhysicalQuantity, rUs.mUnit));
        }

        if (i >= 0)
        {
            mpUnitComboBox->setCurrentIndex(i);
        }
        else
        {
            //Add to list
            mUnitScales.insert(rUs.mUnit, rUs.toDouble());
            mpUnitComboBox->addItem(rUs.mUnit);
            mpUnitComboBox->setCurrentIndex(mpUnitComboBox->count()-1);
        }
    }
}

QString UnitSelectionWidget::getSelectedUnit() const
{
    if (mpUnitComboBox)
    {
        return mpUnitComboBox->currentText();
    }
    else
    {
        return mDefaultUnit;
    }
}

double UnitSelectionWidget::getSelectedUnitScale() const
{
    return mUnitScales.value(getSelectedUnit(),1);
}

void UnitSelectionWidget::getSelectedUnitScale(UnitScale &rUnitScale) const
{
    QString pq = mPhysicalQuantity;
    rUnitScale.mUnit = getSelectedUnit();
    // Clean up the extra info on units in case of multiple quantities
    if (rUnitScale.mUnit.startsWith("<"))
    {
        pq = rUnitScale.mUnit.mid(1, rUnitScale.mUnit.indexOf(">")-1);
        rUnitScale.mUnit.remove(0,rUnitScale.mUnit.indexOf(">")+1);
        rUnitScale.mUnit = rUnitScale.mUnit.trimmed();
    }
    //! @todo converting to and from text could damage the scale value due to truncation
    rUnitScale.mScale = QString("%1").arg(getSelectedUnitScale());
    rUnitScale.mPhysicalQuantity = pq;
}

bool UnitSelectionWidget::isDefaultSelected() const
{
    if (mpUnitComboBox)
    {
        return mpUnitComboBox->currentIndex() == mDefaultIndex;
    }
    else
    {
        return true;
    }
}

void UnitSelectionWidget::resetDefault()
{
    if (mpUnitComboBox)
    {
        mpUnitComboBox->setCurrentIndex(mDefaultIndex);
    }
}

void UnitSelectionWidget::selectionChanged(int idx)
{
    Q_UNUSED(idx)
    UnitScale us;
    getSelectedUnitScale(us);
    emit unitChanged(us);
}
