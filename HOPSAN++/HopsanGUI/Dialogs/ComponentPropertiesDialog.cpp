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
//! @file   ComponentPropertiesDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

#include <QtGui>
#include <cassert>
#include <iostream>

#include "ComponentPropertiesDialog.h"
#include "MainWindow.h"
#include "GUIPort.h"
#include "Widgets/MessageWidget.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "UndoStack.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Configuration.h"
#include "Utilities/GUIUtilities.h"
#include "Dialogs/MovePortsDialog.h"


//! @class ComponentPropertiesDialog
//! @brief The ComponentPropertiesDialog class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog::ComponentPropertiesDialog(Component *pGUIComponent, MainWindow *parent)
    : QDialog(parent)
{
    mpGUIComponent = pGUIComponent;
    this->setPalette(gConfig.getPalette());
    createEditStuff();
}


//! @brief Check if the parameter is a start value
//! @param [in,out] parameterDescription The description of the parameter/startvalue
//! @returns true if it is a startvalue, otherwise false
//!
//! This method is used to determine whether or not a parameter should be interpretted
//! as a start value by the GUI. In HOPSANcore there is no difference between parameters
//! and start values. The start values are registred and stored in the same container.
//! But, a start value is taged by "startvalue:" in the description.
bool ComponentPropertiesDialog::interpretedAsStartValue(QString &parameterDescription)
{    
    QString startValueString = "startvalue:";
    bool res=false;
    if(parameterDescription.contains(startValueString, Qt::CaseInsensitive))
    {
        parameterDescription.remove(startValueString, Qt::CaseInsensitive);
        res = true;
    }
    return res;
}


//! @brief Creates the contents in the parameter dialog
void ComponentPropertiesDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(mpGUIComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

//    QFont fontH2;
//    fontH2.setBold(true);
//    fontH2.setItalic(true);

    QLabel *pHelpPicture = new QLabel();
    QPixmap helpPixMap;
    helpPixMap.load(mpGUIComponent->getAppearanceData()->getBasePath() + mpGUIComponent->getHelpPicture());
    pHelpPicture->setPixmap(helpPixMap);

    QLabel *pHelpHeading = new QLabel(gpMainWindow->mpLibrary->getAppearanceData(mpGUIComponent->getTypeName())->getName(), this);
    pHelpHeading->setAlignment(Qt::AlignCenter);
    QFont tempFont = pHelpHeading->font();
    tempFont.setPixelSize(16);
    tempFont.setBold(true);
    pHelpHeading->setFont(tempFont);
    QLabel *pHelpText = new QLabel(mpGUIComponent->getHelpText(), this);
    pHelpText->setWordWrap(true);
    QLabel *pParameterLabel = new QLabel("Parameters", this);
    pParameterLabel->setFont(fontH1);
    QLabel *pStartValueLabel = new QLabel("Start Values", this);
    pStartValueLabel->setFont(fontH1);

    QGridLayout *parameterLayout = new QGridLayout();
    QGridLayout *startValueLayout = new QGridLayout();

    //QVector<QString> qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes;
    //mpGUIComponent->getParameters(qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes);

    QVector<CoreParameterData> paramDataVector;
    mpGUIComponent->getParameters(paramDataVector);

    size_t nParam=0;
    size_t nStV=0;
    for(int i=0; i<paramDataVector.size(); ++i)
    {
        if(interpretedAsStartValue(paramDataVector[i].description))
        {
            //QString unit = gConfig.getDefaultUnit(qParameterNames[i].section("::", 1, 1));
            paramDataVector[i].unit.prepend("[").append("]");
            mvStartValueLayout.push_back(new ParameterLayout(paramDataVector[i],
                                                             mpGUIComponent));
            startValueLayout->addLayout(mvStartValueLayout.back(), nParam, 0);
            ++nParam;
        }
        else
        {
            mvParameterLayout.push_back(new ParameterLayout(paramDataVector[i],
                                                            mpGUIComponent));
            parameterLayout->addLayout(mvParameterLayout.back(), nStV, 0);
            ++nStV;
        }
    }

    //Adjust sizes of labels, to make sure that all text is visible and that the spacing is not too big between them
    int descriptionSize=30;
    int nameSize = 10;
    //Paramters
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        descriptionSize = std::max(descriptionSize, mvParameterLayout.at(i)->mDescriptionLabel.width());
        nameSize = std::max(nameSize, mvParameterLayout.at(i)->mNameLabel.width());
    }
    //Start values
    for(int i=0; i<mvStartValueLayout.size(); ++i)
    {
        descriptionSize = std::max(descriptionSize, mvStartValueLayout.at(i)->mDescriptionLabel.width());
        nameSize = std::max(nameSize, mvStartValueLayout.at(i)->mNameLabel.width());
    }
    //Paramters
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        mvParameterLayout.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvParameterLayout.at(i)->mNameLabel.setFixedWidth(nameSize+10);
    }
    //Start values
    for(int i=0; i<mvStartValueLayout.size(); ++i)
    {
        mvStartValueLayout.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvStartValueLayout.at(i)->mNameLabel.setFixedWidth(nameSize+10);
    }

    //qDebug() << "after parnames";
    mpEditPortPos = new QPushButton(tr("&Move ports"), this);
    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpOkButton = new QPushButton(tr("&Ok"), this);
    mpOkButton->setDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpEditPortPos, QDialogButtonBox::ActionRole);

    connect(mpOkButton, SIGNAL(clicked()), SLOT(okPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), SLOT(close()));
    connect(mpEditPortPos, SIGNAL(clicked()), SLOT(editPortPos()));

    QGroupBox *pHelpGroupBox = new QGroupBox();
    QVBoxLayout *pHelpLayout = new QVBoxLayout();
    pHelpPicture->setAlignment(Qt::AlignCenter);
    pHelpLayout->addWidget(pHelpHeading);
    if(!mpGUIComponent->getHelpPicture().isNull())
        pHelpLayout->addWidget(pHelpPicture);
    if(!mpGUIComponent->getHelpText().isNull())
        pHelpLayout->addWidget(pHelpText);
    pHelpGroupBox->setStyleSheet(QString::fromUtf8("QGroupBox {background-color: white; border: 2px solid gray; border-radius: 5px; margin-top: 1ex;}"));
    pHelpGroupBox->setLayout(pHelpLayout);

    QGridLayout *pNameLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    QLabel *pTypeNameLabel = new QLabel("Type Name: \"" + mpGUIComponent->getTypeName() + "\"", this);
    pNameLayout->addWidget(pNameLabel,0,0);
    pNameLayout->addWidget(mpNameEdit,0,1);
    pNameLayout->addWidget(pTypeNameLabel,1,0,1,2);

    QGridLayout *mainLayout = new QGridLayout();
    //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    if(!mpGUIComponent->getHelpText().isNull() || !mpGUIComponent->getHelpPicture().isNull())
    {
        mainLayout->addWidget(pHelpGroupBox, lr, 0, 1, 2);
    }

    ++lr;

    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(mpButtonBox, lr, 1);

    ++lr;

    if(!(mvParameterLayout.empty()))
    {
        mainLayout->addWidget(pParameterLabel, lr, 0, 1, 2);
        ++lr;
        mainLayout->addLayout(parameterLayout, lr, 0, 1, 2);
        ++lr;
    }
    else
    {
        pParameterLabel->hide();
    }
    if(!(mvStartValueLayout.isEmpty()))
    {
        mainLayout->addWidget(pStartValueLabel,lr, 0, 1, 2);
        ++lr;
        mainLayout->addLayout(startValueLayout, lr, 0, 1, 2);
    }
    else
    {
        pStartValueLabel->hide();
    }

    QWidget *pPrimaryWidget = new QWidget(this);
    pPrimaryWidget->setLayout(mainLayout);
    pPrimaryWidget->setPalette(gConfig.getPalette());

    QScrollArea *pScrollArea = new QScrollArea(this);
    pScrollArea->setWidget(pPrimaryWidget);
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *pPrimaryLayout = new QGridLayout(this);
    pPrimaryLayout->addWidget(pScrollArea);
    setLayout(pPrimaryLayout);

    pPrimaryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    pPrimaryLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
    pScrollArea->setFixedHeight(std::min(pPrimaryWidget->height()+3, maxHeight));
    if(pScrollArea->minimumHeight() == maxHeight)
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+19);
    }
    else
    {
        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+3);
    }
    pScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWindowTitle(tr("Component Properties"));
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog::okPressed()
{
    mpGUIComponent->getParentContainerObject()->renameModelObject(mpGUIComponent->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    setParametersAndStartValues();
}


void ComponentPropertiesDialog::editPortPos()
{
    MovePortsDialog *dialog = new MovePortsDialog(mpGUIComponent->getAppearanceData(), mpGUIComponent->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpGUIComponent, SLOT(refreshAppearance()), Qt::UniqueConnection);
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
//! @see setParametersAndStartValues(QVector<ParameterLayout *> vParLayout)
void ComponentPropertiesDialog::setParametersAndStartValues()
{
    if(setValuesToSystem(mvParameterLayout) && setValuesToSystem(mvStartValueLayout))
    {
        std::cout << "Parameters and start values updated." << std::endl;
        this->close();
    }
}


//! @brief A convinience function that writes the data from a Qt layout to HOPSANcore
//! @see setParametersAndStartValues()
bool ComponentPropertiesDialog::setValuesToSystem(QVector<ParameterLayout *> &vParLayout)
{
    bool success = true;

    //Parameters
    bool addedUndoPost = false;
    //! @todo move this stuff into the parameterlayout class instead, it is all about to set parameters, for example ContainerPropertyDialog
    for (int i=0 ; i < vParLayout.size(); ++i)
    {
        QString valueTxt = vParLayout[i]->getDataValueTxt();

        //Get the old value to se if it changed
        QString oldValueTxt = mpGUIComponent->getParameterValue(vParLayout[i]->getDataName());

        //Strip trailing and leading spaces
        stripLTSpaces(valueTxt);

        //Parameter has changed, add to undo stack and set the parameter
        verifyNewValue(valueTxt);

        if(!mpGUIComponent->setParameterValue(vParLayout[i]->getDataName(), valueTxt)) //This is done as a check as well.
        {
            QMessageBox::critical(0, "Hopsan GUI",
                                  QString("'%1' is an invalid value for parameter '%2'.")
                                  .arg(valueTxt)
                                  .arg(vParLayout[i]->getDataName()));
            vParLayout[i]->setDataValueTxt(oldValueTxt);
            success = false;
        }
        if(oldValueTxt != valueTxt)
        {
            if(!addedUndoPost)
            {
                this->mpGUIComponent->getParentContainerObject()->getUndoStackPtr()->newPost("changedparameters");
                addedUndoPost = true;
            }

            this->mpGUIComponent->getParentContainerObject()->getUndoStackPtr()->registerChangedParameter(mpGUIComponent->getName(),
                                                                                                   vParLayout[i]->getDataName(),
                                                                                                   oldValueTxt,
                                                                                                   valueTxt);
            mpGUIComponent->getParentContainerObject()->mpParentProjectTab->hasChanged();
        }
    }
    return success;
}



ParameterLayout::ParameterLayout(const CoreParameterData &rParameterData, ModelObject *pModelObject, QWidget *pParent)
{
    mpModelObject = pModelObject;

    // Set name label
    mName = rParameterData.name;
    mNameLabel.setText(parseVariableDescription(mName));
    mNameLabel.setMinimumWidth(10);
    mNameLabel.setMaximumWidth(100);
    mNameLabel.adjustSize();
    mNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    // Set description label
    mDescriptionLabel.setMinimumWidth(100);
    mDescriptionLabel.setMaximumWidth(1000);
    mDescriptionLabel.setText(rParameterData.description);
    //mDescriptionNameLabel.setWordWrap(true);
    mDescriptionLabel.adjustSize();

    // Set unit label
    mUnitLabel.setMinimumWidth(50);
    mUnitLabel.setMaximumWidth(50);
    mUnitLabel.setText(parseVariableUnit(rParameterData.unit));

    // Set value line editd
    mValueLineEdit.setMinimumWidth(100);
    mValueLineEdit.setMaximumWidth(100);
    mValueLineEdit.setText(rParameterData.value);

    // Set tool buttons
    mResetDefaultToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    mResetDefaultToolButton.setToolTip("Reset Default Value");

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    mSystemParameterToolButton.setToolTip("Map To System Parameter");

    // Add lables, edits and buttons
    addWidget(&mDescriptionLabel, 0, 0);
    addWidget(&mNameLabel, 0, 1);
    addWidget(&mValueLineEdit, 0, 2);
    addWidget(&mUnitLabel, 0, 3);
    addWidget(&mResetDefaultToolButton, 0, 4);
    addWidget(&mSystemParameterToolButton, 0, 5);

    // If dynamic parameter add switch button
    if (rParameterData.isDynamic)
    {
        mDynamicEnabledCheckBox.setText("Dynamic");
        mDynamicEnabledCheckBox.setToolTip("Make Dynamic (Experimental)");
        mDynamicEnabledCheckBox.setChecked(rParameterData.isEnabled);
        addWidget(&mDynamicEnabledCheckBox, 0, 6);
    }

    // Determine value text color
    pickValueTextColor();

    // Connect signals to buttons
    connect(&mResetDefaultToolButton, SIGNAL(clicked()), this, SLOT(setDefaultValue()));
    connect(&mSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(showListOfSystemParameters()));
    connect(&mValueLineEdit, SIGNAL(textChanged(QString)), this, SLOT(pickValueTextColor()));
    connect(&mDynamicEnabledCheckBox, SIGNAL(isChecked()), this, SLOT(makePort(bool)));
}


//! @brief Returns the actual parameter name, not the fancy display name
QString ParameterLayout::getDataName()
{
    return mName;
}


double ParameterLayout::getDataValue()
{
    return mValueLineEdit.text().toDouble();
}

QString ParameterLayout::getDataValueTxt()
{
    return mValueLineEdit.text();
}


void ParameterLayout::setDataValueTxt(QString valueTxt)
{
    mValueLineEdit.setText(valueTxt);
}


//! @brief Sets the value in the text field to the default parameter value
void ParameterLayout::setDefaultValue()
{
    if(mpModelObject)
    {
        QString defaultText = mpModelObject->getDefaultParameterValue(mName);
        if(defaultText != QString())
            mValueLineEdit.setText(defaultText);
        pickValueTextColor();
    }
}


void ParameterLayout::showListOfSystemParameters()
{
    QMenu menu;
    QMap<QAction*, QString> actionParamMap;

    QVector<CoreParameterData> paramDataVector;
    mpModelObject->getParentContainerObject()->getParameters(paramDataVector);

    for (int i=0; i<paramDataVector.size(); ++i)
    {
        QAction *tempAction = menu.addAction(paramDataVector[i].name+" = "+paramDataVector[i].value);
        tempAction->setIconVisibleInMenu(false);
        actionParamMap.insert(tempAction, paramDataVector[i].name);
    }

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());

    QString parNameString = actionParamMap.value(selectedAction);
    if(!parNameString.isEmpty())
    {
        mValueLineEdit.setText(parNameString);
    }
}

void ParameterLayout::makePort(bool isPort)
{
    if (isPort)
    {

    }
    else
    {

    }
}


void ParameterLayout::pickValueTextColor()
{
    if(mpModelObject)
    {
        if(mValueLineEdit.text() == mpModelObject->getDefaultParameterValue(mName))
        {
            QPalette palette( mValueLineEdit.palette() );
            palette.setColor( QPalette::Text, QColor("gray") );
            mValueLineEdit.setPalette(palette);
        }
        else
        {
            QPalette palette( mValueLineEdit.palette() );
            palette.setColor( QPalette::Text, QColor("black") );
            mValueLineEdit.setPalette(palette);
        }
    }
}



//! @brief Verifies that a parameter value does not begin with a number but still contains illegal characters.
//! @note This is a temporary solution. It shall be removed when parsing equations as parameters works.
//! @param value String with parameter that shall be verified
void ComponentPropertiesDialog::verifyNewValue(QString &value)
{
    QStringList sysParamNames = mpGUIComponent->getParameterNames();
    if(sysParamNames.contains(value))
    {
        return;
    }

    if(value[0].isNumber())
    {
        bool onlyNumbers=true;
        value.replace(",", ".");

        if( value.count("e") > 1 || value.count(".") > 1 || value.count("E") > 1 )
        {
            onlyNumbers=false;
        }
        else
        {
            for(int i=1; i<value.size(); ++i)
            {
                if(!value[i].isDigit() && (value[i] != 'e') && (value[i] != 'E') && (value[i] != '+') && (value[i] != '-') && (value[i] != '.'))
                {
                    onlyNumbers=false;
                    break;
                }
                else if( (value[i] == '+' || value[i] == '-') && !((value[i-1] == 'e') || (value[i-1] == 'E')) )
                {
                    onlyNumbers=false;
                    break;
                }
            }
        }

        if(!onlyNumbers)
        {
           QMessageBox::warning(this, "Warning", "Invalid Parameter string \"" + value + "\" will be truncated into the first nummeric sub string.");
        }
    }
}
