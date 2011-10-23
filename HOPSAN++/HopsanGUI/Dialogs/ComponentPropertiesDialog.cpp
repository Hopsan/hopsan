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
#include "../MainWindow.h"
#include "../GUIPort.h"
#include "../Widgets/MessageWidget.h"
#include "../GUIObjects/GUIComponent.h"
#include "../GUIObjects/GUIContainerObject.h"
#include "../GUIObjects/GUISystem.h"
#include "../UndoStack.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Widgets/SystemParametersWidget.h"
#include "../Widgets/LibraryWidget.h"
#include "../Configuration.h"
#include "../Utilities/GUIUtilities.h"


//! @class ComponentPropertiesDialog
//! @brief The ComponentPropertiesDialog class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog::ComponentPropertiesDialog(GUIComponent *pGUIComponent, MainWindow *parent)
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

    QVector<QString> qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes;
    mpGUIComponent->getParameters(qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes);
    size_t nParam=0;
    size_t nStV=0;
    for(int i=0; i<qParameterNames.size(); ++i)
    {
        if(interpretedAsStartValue(qDescriptions[i]))
        {
//            QString portName = qDescriptions[i];
//            portName.remove("Port ");
//            QString portType = mpGUIComponent->getPort(portName)->getPortType();
//            qDebug() << "The port type is " << portType;
//            if((portType != "MULTIPORT") && (portType != "POWERMULTIPORT") && (portType != "READMULTIPORT"))
//            {
//                qDebug() << "Doing it!";
                QString unit = gConfig.getDefaultUnit(qParameterNames[i].section("::", 1, 1));
                unit.prepend("[");
                unit.append("]");
                mvStartValueLayout.push_back(new ParameterLayout(qParameterNames[i], qDescriptions[i],
                                                                 qParameterValues[i],
                                                                 unit,
                                                                 qTypes[i],
                                                                 mpGUIComponent));
                startValueLayout->addLayout(mvStartValueLayout.back(), nParam, 0);
                ++nParam;
//            }
        }
        else
        {
            mvParameterLayout.push_back(new ParameterLayout(qParameterNames[i], qDescriptions[i],
                                                            qParameterValues[i],
                                                            qUnits[i],
                                                            qTypes[i],
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
        descriptionSize = std::max(descriptionSize, mvParameterLayout.at(i)->mDescriptionNameLabel.width());
        nameSize = std::max(nameSize, mvParameterLayout.at(i)->mDataNameLabel.width());
    }
    //Start values
    for(int i=0; i<mvStartValueLayout.size(); ++i)
    {
        descriptionSize = std::max(descriptionSize, mvStartValueLayout.at(i)->mDescriptionNameLabel.width());
        nameSize = std::max(nameSize, mvStartValueLayout.at(i)->mDataNameLabel.width());
    }
    //Paramters
    for(int i=0; i<mvParameterLayout.size(); ++i)
    {
        mvParameterLayout.at(i)->mDescriptionNameLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvParameterLayout.at(i)->mDataNameLabel.setFixedWidth(nameSize+10);
    }
    //Start values
    for(int i=0; i<mvStartValueLayout.size(); ++i)
    {
        mvStartValueLayout.at(i)->mDescriptionNameLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
        mvStartValueLayout.at(i)->mDataNameLabel.setFixedWidth(nameSize+10);
    }

    //qDebug() << "after parnames";

    mpCancelButton = new QPushButton(tr("&Cancel"), this);
    mpOkButton = new QPushButton(tr("&Ok"), this);
    mpOkButton->setDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);

    connect(mpOkButton, SIGNAL(clicked()), SLOT(okPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), SLOT(close()));

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
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
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
        mainLayout->addWidget(pParameterLabel, lr, 0);
        ++lr;
        mainLayout->addLayout(parameterLayout, lr, 0);
        ++lr;
    }
    else
    {
        pParameterLabel->hide();
    }
    if(!(mvStartValueLayout.isEmpty()))
    {
        mainLayout->addWidget(pStartValueLabel,lr, 0);
        ++lr;
        mainLayout->addLayout(startValueLayout, lr, 0);
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

    pPrimaryWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
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
    pScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWindowTitle(tr("Parameters"));
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog::okPressed()
{
    mpGUIComponent->getParentContainerObject()->renameGUIModelObject(mpGUIComponent->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    setParametersAndStartValues();
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
    for (int i=0 ; i < vParLayout.size(); ++i )
    {
        QString valueTxt = vParLayout[i]->getDataValueTxt();

        //Get the old value to se if it changed
        QString oldValueTxt = mpGUIComponent->getParameterValue(vParLayout[i]->getDataName());

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


//ParameterLayout::ParameterLayout(QString dataName, QString descriptionName, double dataValue, QString unitName, GUIModelObject *pGUIModelObject, QWidget *parent)
//    : QGridLayout(parent)
//{
//    QString dataValueStr;
//    dataValueStr.setNum(dataValue);
//    commonConstructorCode(dataName, descriptionName, dataValueStr, unitName, pGUIModelObject);
//}


ParameterLayout::ParameterLayout(QString dataName, QString descriptionName, QString dataValue, QString unitName, QString typeName, GUIModelObject *pGUIModelObject, QWidget *parent)
    : QGridLayout(parent)
{
    commonConstructorCode(dataName, descriptionName, dataValue, unitName, typeName, pGUIModelObject);
}


void ParameterLayout::commonConstructorCode(QString dataName, QString descriptionName, QString dataValue, QString unitName, QString typeName, GUIModelObject *pGUIModelObject)
{
    mDataName = dataName;

    mpGUIModelObject = pGUIModelObject;

    mDescriptionNameLabel.setMinimumWidth(30);
    mDescriptionNameLabel.setMaximumWidth(200);
    mDataNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mDataNameLabel.setMinimumWidth(10);
    mDataNameLabel.setMaximumWidth(100);
    mDataValuesLineEdit.setMinimumWidth(100);
    mDataValuesLineEdit.setMaximumWidth(100);
    mUnitNameLabel.setMinimumWidth(50);
    mUnitNameLabel.setMaximumWidth(50);

    mResetDefaultToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    mResetDefaultToolButton.setToolTip("Reset Default Value");

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    mSystemParameterToolButton.setToolTip("Map To System Parameter");

    mDataNameLabel.setText(parseVariableDescription(dataName));
    mDataNameLabel.adjustSize();
    mDescriptionNameLabel.setText(descriptionName);
    mDescriptionNameLabel.adjustSize();
    mUnitNameLabel.setText(parseVariableUnit(unitName));
    mDataValuesLineEdit.setText(dataValue);

    addWidget(&mDescriptionNameLabel, 0, 0);
    addWidget(&mDataNameLabel, 0, 1);
    addWidget(&mDataValuesLineEdit, 0, 2);
    addWidget(&mUnitNameLabel, 0, 3);
    addWidget(&mResetDefaultToolButton, 0, 4);
    addWidget(&mSystemParameterToolButton, 0, 5);

    QPalette palette( mDataValuesLineEdit.palette() );
    palette.setColor( QPalette::Text, QColor("gray") );
    mDataValuesLineEdit.setPalette(palette);

    pickColor();

    connect(&mResetDefaultToolButton, SIGNAL(clicked()), this, SLOT(setDefaultValue()));
    connect(&mSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(showListOfSystemParameters()));
    connect(&mDataValuesLineEdit, SIGNAL(editingFinished()), this, SLOT(pickColor()));
}


QString ParameterLayout::getDescriptionName()
{
    return mDescriptionNameLabel.text();
}


QString ParameterLayout::getDataName()
{
    return mDataName;
}


double ParameterLayout::getDataValue()
{
    return mDataValuesLineEdit.text().toDouble();
}

QString ParameterLayout::getDataValueTxt()
{
    return mDataValuesLineEdit.text();
}


void ParameterLayout::setDataValueTxt(QString valueTxt)
{
    mDataValuesLineEdit.setText(valueTxt);
}


//! @brief Sets the value in the text field to the default parameter value
void ParameterLayout::setDefaultValue()
{
    QString defaultText = mpGUIModelObject->getDefaultParameter(mDataName);
    if(defaultText != QString())
        mDataValuesLineEdit.setText(defaultText);
    pickColor();
}


void ParameterLayout::showListOfSystemParameters()
{
    //mSystemParameterToolButton.setDown(false);

    QMenu menu;

    QMap<std::string, std::string> SystemMap = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getCoreSystemAccessPtr()->getSystemParametersMap();
    QMap<std::string, std::string>::iterator it;
    for(it=SystemMap.begin(); it!=SystemMap.end(); ++it)
    {
//        QString valueString;
//        valueString.setNum(it.value());
        QAction *tempAction = menu.addAction(QString(it.key().c_str()));
        tempAction->setIconVisibleInMenu(false);
    }

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());
    if(selectedAction != 0)
    {
        mDataValuesLineEdit.setText(selectedAction->text());
    }
}


void ParameterLayout::pickColor()
{
    if(mDataValuesLineEdit.text() == mpGUIModelObject->getDefaultParameter(mDataName))
    {
        QPalette palette( mDataValuesLineEdit.palette() );
        palette.setColor( QPalette::Text, QColor("gray") );
        mDataValuesLineEdit.setPalette(palette);
    }
    else
    {
        QPalette palette( mDataValuesLineEdit.palette() );
        palette.setColor( QPalette::Text, QColor("black") );
        mDataValuesLineEdit.setPalette(palette);
    }
}



//! @brief Verifies that a parameter value does not begin with a number but still contains illegal characters.
//! @note This is a temporary solution. It shall be removed when parsing equations as parameters works.
//! @param value String with parameter that shall be verified
void ComponentPropertiesDialog::verifyNewValue(QString value)
{
    if(mpGUIComponent->mpParentContainerObject->getCoreSystemAccessPtr()->getSystemParametersMap().contains(value.toStdString()))
    {
        return;
    }

    if(value[0].isNumber())
    {
        bool onlyNumbers=true;
        for(int i=1; i<value.size(); ++i)
        {
            if(!value[i].isDigit() && !(value[i] == 'e') && !(value[i] == '+') && !(value[i] == '-') && !(value[i] == '.') && !(value[i] == ','))
            {
                onlyNumbers=false;
            }
            else if(value.count("e") > 1 || value.count(".") > 1 || value.count(",") > 1)
            {
                onlyNumbers=false;
            }
            else if((value[i] == '+' || value[i] == '-') && !(value[i-1] == 'e'))
            {
                onlyNumbers=false;
            }
        }
        if(!onlyNumbers)
        {
           QMessageBox::warning(this, "Warning", "Parameter with value \"" + value + "\" will (probably) be truncated into a number.");
        }
    }
}
