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
//$Id: ComponentPropertiesDialog3.cpp 4807 2012-11-28 14:07:11Z petno25 $

//Qt includes
#include <QtGui>
#include <QDebug>

//Hopsan includes
#include "common.h"
#include "ComponentPropertiesDialog3.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "Dialogs/EditComponentDialog.h"
#include "Dialogs/MovePortsDialog.h"
#include "Dialogs/ParameterSettingsLayout.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "MainWindow.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/SystemParametersWidget.h"



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

    if(mpModelObject->getTypeName() == "ModelicaComponent")
    {
        this->hide();

        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Modelica);
        pEditDialog->exec();

        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess(gpMainWindow->mpLibrary);
            QString typeName = pEditDialog->getCode().section("model ", 1, 1).section(" ",0,0);
            QString dummy = gDesktopHandler.getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            coreAccess.generateFromModelica(pEditDialog->getCode(), libPath, typeName);
            gpMainWindow->mpLibrary->loadAndRememberExternalLibrary(libPath, "");
            mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
        }
        delete(pEditDialog);

        this->close();
        return;
    }
    else if(mpModelObject->getTypeName() == "CppComponent")
    {
        this->hide();

        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Cpp);
        pEditDialog->exec();

        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess(gpMainWindow->mpLibrary);
            QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);
            QString libPath = gDesktopHandler.getGeneratedComponentsPath()+typeName+"/";
            coreAccess.generateFromCpp(pEditDialog->getCode(), true, libPath);
            gpMainWindow->mpLibrary->loadAndRememberExternalLibrary(libPath, "");
            mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
        }
        delete(pEditDialog);

        this->close();
        return;
    }

    this->setPalette(gConfig.getPalette());
    setWindowTitle(tr("Component Properties"));
    //    if(mpModelObject->getTypeName().startsWith("CppComponent"))
    //    {
    //        createCppEditStuff();
    //    }
    //    else if(mpModelObject->getTypeName().startsWith("ModelicaComponent"))
    //    {
    //        createModelicaEditStuff();
    //    }
    //    else
    //    {
    createEditStuff();
    //    }

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
        QMessageBox::critical(this->parentWidget(), "Error", error.append(" Resetting parameter value!"));
        return false;
    }
}

bool VariableTableWidget::setAliasName(const int row)
{
    // Skip separator rows
    if (columnSpan(row,0)>1)
    {
        return true;
    }

    QString alias = item(row,VariableTableWidget::Alias)->text();
    //! @todo since alias=empty means unregister we can not skip it, but there should be some check if a variable has changed, so that we do not need to go thourgh set for all variables every freeking time
    QString name = item(row,VariableTableWidget::Name)->text();
    QStringList parts = name.split("#");
    if (parts.size() == 2)
    {
        mpModelObject->getParentContainerObject()->setVariableAlias(mpModelObject->getName(), parts[0], parts[1], alias);
    }
    //! @todo check if OK
    return true;
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog3::okPressed()
{
    setName();

//    if(mpModelObject->getTypeName().startsWith("CppComponent"))
//    {
//        recompileCppFromDialog();
//    }
//    else
//    {
        setAliasNames();

        if (setVariableValues())
        {
            close();
        }
 //   }

}


void ComponentPropertiesDialog3::editPortPos()
{
    //! @todo who owns the dialog, is it ever removed?
    MovePortsDialog *dialog = new MovePortsDialog(mpModelObject->getAppearanceData(), mpModelObject->getLibraryAppearanceData(), mpModelObject->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpModelObject, SLOT(refreshExternalPortsAppearanceAndPosition()), Qt::UniqueConnection);
}

void ComponentPropertiesDialog3::recompile()
{
    QString basePath = this->mpModelObject->getAppearanceData()->getBasePath();
    QString libPath = this->mpModelObject->getAppearanceData()->getLibPath();
    QString fileName = this->mpModelObject->getAppearanceData()->getSourceCodeFile();

    bool modelica = mpModelObject->getAppearanceData()->getSourceCodeFile().endsWith(".mo");
    QString sourceCode = mpSourceCodeTextEdit->toPlainText();

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

    if(!gpMainWindow->mpLibrary->recompileComponent(basePath+libPath, modelica, sourceCode))
    {
        qDebug() << "Failure!";
        QFile sourceFile(basePath+fileName);
        if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
            return;
        sourceFile.write(oldSourceCode.toStdString().c_str());
        sourceFile.close();
    }

    this->close();
}

bool ComponentPropertiesDialog3::setAliasNames()
{
    return mpVariableTableWidget->setAliasNames();
//    for (int r=0; r<mpVariableTableWidget->rowCount(); ++r)
//    {
//        if (mpVariableTableWidget->columnSpan(r,0)>1)
//        {
//            // Skip separator rows
//            continue;
//        }

//        QString alias = mpVariableTableWidget->item(r,VariableTableWidget::Alias)->text();
////        if (!alias.isEmpty())
////        {
//        //! @todo since alias=empty means unregister we can not skip it, but there should be some check if a variable has changed, so that we do not need to go thourgh set for all variables every freeking time
//            QString name = mpVariableTableWidget->item(r,VariableTableWidget::Name)->text();
//            QStringList parts = name.split("::");
//            if (parts.size() == 2)
//            {
//                mpModelObject->getParentContainerObject()->setVariableAlias(mpModelObject->getName(), parts[0], parts[1], alias);
//            }
////            else
////            {
////                return false;
////            }
//            //! @todo check if OK
////        }
//    }
//    return true;
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
bool ComponentPropertiesDialog3::setVariableValues()
{
    return mpVariableTableWidget->setStartValues();
}

void ComponentPropertiesDialog3::setName()
{
    mpModelObject->getParentContainerObject()->renameModelObject(mpModelObject->getName(), mpNameEdit->text());
}

void ComponentPropertiesDialog3::recompileCppFromDialog()
{
    mpModelObject->setCppCode(mpTextEdit->toPlainText());
    mpModelObject->setCppInputs(mpInputPortsSpinBox->value());
    mpModelObject->setCppOutputs(mpOutputPortsSpinBox->value());

    mpModelObject->getParentContainerObject()->recompileCppComponents(mpModelObject);

    this->close();
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
    mpRecompileButton = new QPushButton(tr("&Recompile"), this);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pOkButton = new QPushButton(tr("&Ok"), this);
    pOkButton->setDefault(true);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpRecompileButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pEditPortPos, QDialogButtonBox::ActionRole);
    connect(pOkButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pEditPortPos, SIGNAL(clicked()), this, SLOT(editPortPos()));
    connect(mpRecompileButton, SIGNAL(clicked()), this, SLOT(recompile()));
    mpRecompileButton->setVisible(false);
    return pButtonBox;
}

QWidget *ComponentPropertiesDialog3::createHelpWidget()
{
    if(!mpModelObject->getHelpText().isNull() || !mpModelObject->getHelpPicture().isNull())
    {
        QScrollArea *pHelpScrollArea = new QScrollArea();
        QGroupBox *pHelpWidget = new QGroupBox();
        QVBoxLayout *pHelpLayout = new QVBoxLayout(pHelpWidget);

        QLabel *pHelpHeading = new QLabel(gpMainWindow->mpLibrary->getAppearanceData(mpModelObject->getTypeName())->getDisplayName());
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
    mpRecompileButton->setVisible(true);

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
    pPropertiesWidget->setPalette(gConfig.getPalette());

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
    columnHeaders.append("Unit");
    columnHeaders.append("Description");
    columnHeaders.append("Type");
    columnHeaders.append("Value");
    columnHeaders.append("PlotScale");
    columnHeaders.append("");
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
        createSeparatorRow(r,"Constants");
        ++r;
    }
    for (int i=0; i<constantsIds.size(); ++i)
    {
        CoreVariameterDescription variameter;
        variameter.mName = parameters[constantsIds[i]].mName;
        variameter.mDescription = parameters[constantsIds[i]].mDescription;
        variameter.mUnit = parameters[constantsIds[i]].mUnit;
        variameter.mDataType = parameters[constantsIds[i]].mType;
        createTableRow(r, variameter, Constant);
        ++r;
    }

    // Now fetch and write the variables
    QVector<CoreVariameterDescription> variameters;
    mpModelObject->getVariameterDescriptions(variameters);

    // Write inputVariables
    createSeparatorRow(r,"InputVariables");
    ++r;
    for (int i=0; i<variameters.size(); ++i)
    {
        if (variameters[i].mVariameterType == InputVaraiable)
        {
            createTableRow(r, variameters[i], InputVaraiable);
            ++r;
        }
    }

    // Write outputVariables
    createSeparatorRow(r,"OutputVariables");
    ++r;
    for (int i=0; i<variameters.size(); ++i)
    {
        if (variameters[i].mVariameterType == OutputVariable)
        {
            createTableRow(r, variameters[i], OutputVariable);
            ++r;
        }
    }

    // Write remaning port variables
    QString currPortName;
    for (int i=0; i<variameters.size(); ++i)
    {
        if (variameters[i].mVariameterType == OtherVariable)
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
                createSeparatorRow(r,"Port: "+portName);
                ++r;
            }

            createTableRow(r, variameters[i], OtherVariable);
            ++r;
        }
    }

    resizeColumnToContents(Name);
    resizeColumnToContents(Unit);
    resizeColumnToContents(Type);
    resizeColumnToContents(Buttons);

    horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->hide();

    connect(this, SIGNAL(cellChanged(int,int)), this, SLOT(cellChangedSlot(int,int)));
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
//        if (!setStartValue(r))
//        {
//            ok=falase;
//            break;
//        }

        // First check if row is separator, then skip it
        if (columnSpan(row,0)>1)
        {
            continue;
        }

        QString value = item(row,VariableTableWidget::Value)->text();
        // If startvalue is empty (disabled, then we should not atempt to change it)
        if (value.isEmpty())
        {
            continue;
        }

        // Extract name and value from row
        QString name = item(row,VariableTableWidget::Name)->text();


        // Get the old value to see if a changed has occured
        QString oldValue = mpModelObject->getParameterValue(name);
        if (oldValue != value)
        {
            // Parameter has changed, add to undo stack and set the parameter
            bool isOk = cleanAndVerifyParameterValue(value, item(row,VariableTableWidget::Type)->text());
            if(isOk)
            {
                // If we fail to set the parameter, then warning box and reset value
                if(!mpModelObject->setParameterValue(name, value))
                {
                    QMessageBox::critical(0, "Hopsan GUI", QString("'%1' is an invalid value for parameter '%2'.").arg(value).arg(name));
                    // Reset old value
                    item(row,VariableTableWidget::Value)->setText(oldValue);
                    isOk = false;
                }

                // Add an undo post (but only one for all values changed this time
                if(!addedUndoPost)
                {
                    mpModelObject->getParentContainerObject()->getUndoStackPtr()->newPost("changedparameters");
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
            else
            {
                // Reset old value
                item(row,VariableTableWidget::Value)->setText(oldValue);
                allok = false;
                break;
            }
        }
        //! @todo if we set som ok but som later fail pressing cancel will not restore the first that were set ok /Peter
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

void VariableTableWidget::resetDefaultValueAtRow(int row)
{
    QString name = item(row,Name)->text();
    if(mpModelObject)
    {
        QString defaultText = mpModelObject->getDefaultParameterValue(name);
        if(defaultText != QString())
        {
            item(row,Value)->setText(defaultText);
            item(row,Value)->setTextColor(Qt::gray);
        }
    }
}

void VariableTableWidget::selectSystemParameterAtRow(int row)
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
        gpMainWindow->mpSystemParametersWidget->openAddParameterDialog();
        return;
    }
    QString parNameString = actionParamMap.value(selectedAction);
    if(!parNameString.isEmpty())
    {
        item(row,Value)->setText(parNameString);
        selectValueTextColor(row);
    }
}

void VariableTableWidget::makePortAtRow(int row, bool isPort)
{
//! @todo hmm it does not make sense to have startvalues as ports (or maybe it does, but startvalues are run before init and simulate), but then you could have startvalue to startvalue to startvalue ...
    const QString name = item(row,Name)->text().split("#").at(0);
    Port * pPort=0;
    if (isPort)
    {
        pPort = mpModelObject->getPort(name);
        if (pPort)
        {
            pPort->setEnable(true);
            mpModelObject->createRefreshExternalPort(name);
        }
        else
        {
            pPort = mpModelObject->createRefreshExternalPort(name);
            if (pPort)
            {
                // Make sure that our new port has the "correct" angle
                pPort->setRotation(180);
                pPort->setModified(true);
            }
        }
    }
    else
    {
        mpModelObject->hideExternalDynamicParameterPort(name);
    }
}

void VariableTableWidget::cellChangedSlot(const int row, const int col)
{
    if (col == Value)
    {
        selectValueTextColor(row);
    }
}

void VariableTableWidget::createTableRow(const int row, const CoreVariameterDescription &rData, const VariameterTypEnumT variametertype)
{
    QString fullName;
    QTableWidgetItem *pItem;
    insertRow(row);

    //! @todo maybe store the variamter data objects localy, and check for hiden info there, would also make it possible to check for changes without asking core all of the time /Peter
    if (rData.mPortName.isEmpty())
    {
        fullName = rData.mName;
    }
    else
    {
        fullName = rData.mPortName+"#"+rData.mName;
    }

    pItem = new QTableWidgetItem(fullName);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Name,pItem);

    pItem = new QTableWidgetItem(rData.mAlias);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags);
    if (variametertype != Constant)
    {
        pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    }
    setItem(row,Alias,pItem);

    //pItem = new QTableWidgetItem(parseVariableUnit(rData.mUnit));
    pItem = new QTableWidgetItem(rData.mUnit);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Unit,pItem);

    pItem = new QTableWidgetItem(rData.mDescription);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Description,pItem);

    pItem = new QTableWidgetItem(rData.mDataType);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Type,pItem);

    QString value = mpModelObject->getParameterValue(fullName);
    pItem = new QTableWidgetItem(value);
    if (value.isEmpty())
    {
        pItem->setFlags(Qt::NoItemFlags);
    }
    else
    {
        pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    }
    pItem->setTextAlignment(Qt::AlignCenter);
    setItem(row,Value,pItem);
    selectValueTextColor(row);

    // Create the custom plot unit display and selection button
    QWidget *pPlotScaleWidget = new PlotScaleSelectionWidget(row, rData, mpModelObject);
    this->setIndexWidget(model()->index(row,Scale), pPlotScaleWidget);


    // Set tool buttons
    QWidget* pTooButtons = new QWidget();
    QHBoxLayout* pToolButtonsLayout = new QHBoxLayout(pTooButtons);
    pToolButtonsLayout->setContentsMargins(0,0,0,0);

    RowAwareToolButton *pResetDefaultToolButton = new RowAwareToolButton(row);
    pResetDefaultToolButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    pResetDefaultToolButton->setToolTip("Reset Default Value");
    pResetDefaultToolButton->setFixedSize(24,24);
    connect(pResetDefaultToolButton, SIGNAL(triggeredAtRow(int)), this, SLOT(resetDefaultValueAtRow(int)));
    pToolButtonsLayout->addWidget(pResetDefaultToolButton);

    RowAwareToolButton *pSystemParameterToolButton = new RowAwareToolButton(row);
    pSystemParameterToolButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    pSystemParameterToolButton->setToolTip("Map To System Parameter");
    pSystemParameterToolButton->setFixedSize(24,24);
    connect(pSystemParameterToolButton, SIGNAL(triggeredAtRow(int)), this, SLOT(selectSystemParameterAtRow(int)));
    pToolButtonsLayout->addWidget(pSystemParameterToolButton);

    if ( (variametertype == InputVaraiable) || (variametertype == OutputVariable))
    {
        RowAwareCheckBox *pEnablePortCheckBox = new RowAwareCheckBox(row);
        pEnablePortCheckBox->setToolTip("Show/hide port");
        Port *pPort = mpModelObject->getPort(rData.mPortName);
        pEnablePortCheckBox->setChecked((pPort && pPort->getPortAppearance()->mEnabled));
        pEnablePortCheckBox->setFixedSize(24,24);
        connect(pEnablePortCheckBox, SIGNAL(checkedAtRow(int, bool)), this, SLOT(makePortAtRow(int,bool)));
        pToolButtonsLayout->addWidget(pEnablePortCheckBox);
    }
    pToolButtonsLayout->addStretch(2);
    this->setIndexWidget(model()->index(row,Buttons), pTooButtons);

    mVariableDescriptions.insert(row, rData);
}

void VariableTableWidget::createSeparatorRow(const int row, const QString name)
{
    QTableWidgetItem *pItem;
    pItem = new QTableWidgetItem(name);
    pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    pItem->setBackgroundColor(Qt::lightGray);
    insertRow(row);
    setSpan(row,Name,1,NumCols);
    setItem(row,Name,pItem);
    resizeRowToContents(row);
}

void VariableTableWidget::selectValueTextColor(const int row)
{
    if(mpModelObject)
    {
        const QString name = item(row,Name)->text();
        const QString value = item(row,Value)->text();
        if( value == mpModelObject->getDefaultParameterValue(name))
        {
            item(row,Value)->setTextColor(Qt::gray);
        }
        else
        {
            item(row,Value)->setTextColor(Qt::black);
        }
    }
}

//void VariableTableWidget::setStartValue(const int row)
//{

//}


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
    qDebug() << "w: " << w << " lw: " << lineWidth() << "  mLw: " << midLineWidth() << "  frameWidth: " << frameWidth();
    qDebug() << verticalScrollBar()->sizeHint().width();

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


RowAwareToolButton::RowAwareToolButton(const int row) : QToolButton()
{
    setRow(row);
    connect(this, SIGNAL(clicked(bool)), this, SLOT(clickedSlot()));
}

void RowAwareToolButton::setRow(const int row)
{
    mRow = row;
}


void RowAwareToolButton::clickedSlot()
{
    emit triggeredAtRow(mRow);
}

RowAwareCheckBox::RowAwareCheckBox(const int row) : QCheckBox()
{
    setRow(row);
    connect(this, SIGNAL(clicked(bool)), this, SLOT(checkedSlot(bool)));
}

void RowAwareCheckBox::setRow(const int row)
{
    mRow = row;
}


void RowAwareCheckBox::checkedSlot(const bool state)
{
    emit checkedAtRow(mRow, state);
}


PlotScaleSelectionWidget::PlotScaleSelectionWidget(const int row, const CoreVariameterDescription &rData, ModelObject *pModelObject)
{
    mVariableTypeName = rData.mName;
    mVariablePortDataName = rData.mPortName+"#"+rData.mName;
    mpModelObject = pModelObject;

    QHBoxLayout* pPlotScaleWidgetLayout = new QHBoxLayout(this);
    pPlotScaleWidgetLayout->setContentsMargins(0,0,0,0);
    pPlotScaleWidgetLayout->setSpacing(0);
    mpPlotScaleEdit = new QLineEdit();
    mpPlotScaleEdit->setAlignment(Qt::AlignCenter);
    mpPlotScaleEdit->setFrame(false);
    pPlotScaleWidgetLayout->addWidget(mpPlotScaleEdit);
    connect(mpPlotScaleEdit, SIGNAL(editingFinished()), this, SLOT(registerCustomScale()));

    UnitScale currCustom;
    pModelObject->getCustomPlotUnitOrScale(mVariablePortDataName, currCustom);
    if (!currCustom.mScale.isEmpty()) // Check if data exists
    {
        // If unit not given, display the scale value
        if (currCustom.mUnit.isEmpty())
        {
            mpPlotScaleEdit->setText(currCustom.mScale);
        }
        // If description given use it (usually the custom unit)
        else
        {
            mpPlotScaleEdit->setText(currCustom.mUnit);
        }
    }

    RowAwareToolButton *pScaleSelectionButton =  new RowAwareToolButton(row);
    pScaleSelectionButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
    pScaleSelectionButton->setToolTip("Select Unit Scaling");
    pScaleSelectionButton->setFixedSize(24,24);
    connect(pScaleSelectionButton, SIGNAL(triggeredAtRow(int)), this, SLOT(createPlotScaleSelectionMenu()));
    pPlotScaleWidgetLayout->addWidget(pScaleSelectionButton);
}

void PlotScaleSelectionWidget::createPlotScaleSelectionMenu()
{
    QMenu menu;
    QMap<QString, double> unitScales = gConfig.getCustomUnits(mVariableTypeName);
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
                mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, key, QString("%1").arg(unitScales.value(key)));
                mpPlotScaleEdit->setText(key);
            }
        }
    }
}

void PlotScaleSelectionWidget::registerCustomScale()
{
    //! @todo need to check if text is valid number
    mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, "", mpPlotScaleEdit->text());
}
