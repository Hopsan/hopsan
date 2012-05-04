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
//! @file   ParameterSettingsLayout.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a Parameter Settings dialog class for changing parameter properties in components and systems
//!
//$Id$

#include "ParameterSettingsLayout.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"

ParameterSettingsLayout::ParameterSettingsLayout(const CoreParameterData &rParameterData, ModelObject *pModelObject, QWidget *pParent) : QGridLayout(pParent)
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

    // If dynamic parameter add switch button
    if (rParameterData.isDynamic)
    {
        bool checked=false;
        //mDynamicEnabledCheckBox.setText("Dynamic");
        mDynamicEnabledCheckBox.setToolTip("Make Port (Experimental)");
        if (mpModelObject->getPort(rParameterData.name) != 0)
        {
            checked=true;
        }
        mDynamicEnabledCheckBox.setChecked(checked);
        addWidget(&mDynamicEnabledCheckBox, 0, 0);

        //! @todo if parmeter dissabled (connected from outside) make entire ParameterLayout gray or locked (except for the checkbox)
    }

    // Add lables, edits and buttons
    addWidget(&mDescriptionLabel, 0, 1);
    addWidget(&mNameLabel, 0, 2);
    addWidget(&mValueLineEdit, 0, 3);
    addWidget(&mUnitLabel, 0, 4);
    addWidget(&mResetDefaultToolButton, 0, 5);
    addWidget(&mSystemParameterToolButton, 0, 6);



    // Determine value text color
    pickValueTextColor();

    // Connect signals to buttons
    connect(&mResetDefaultToolButton, SIGNAL(clicked()), this, SLOT(setDefaultValue()));
    connect(&mSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(showListOfSystemParameters()));
    connect(&mValueLineEdit, SIGNAL(textChanged(QString)), this, SLOT(pickValueTextColor()));
    connect(&mDynamicEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(makePort(bool)));
}


//! @brief Returns the actual parameter name, not the fancy display name
QString ParameterSettingsLayout::getDataName()
{
    return mName;
}


double ParameterSettingsLayout::getDataValue()
{
    return mValueLineEdit.text().toDouble();
}

QString ParameterSettingsLayout::getDataValueTxt()
{
    return mValueLineEdit.text();
}


void ParameterSettingsLayout::setDataValueTxt(QString valueTxt)
{
    mValueLineEdit.setText(valueTxt);
}


//! @brief Sets the value in the text field to the default parameter value
void ParameterSettingsLayout::setDefaultValue()
{
    if(mpModelObject)
    {
        QString defaultText = mpModelObject->getDefaultParameterValue(mName);
        if(defaultText != QString())
            mValueLineEdit.setText(defaultText);
        pickValueTextColor();
    }
}


void ParameterSettingsLayout::showListOfSystemParameters()
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

void ParameterSettingsLayout::makePort(bool isPort)
{
    if (isPort)
    {
        mpModelObject->createRefreshExternalPort(mName);
    }
    else
    {
        mpModelObject->removeExternalPort(mName);
    }
}


void ParameterSettingsLayout::pickValueTextColor()
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
bool ParameterSettingsLayout::cleanAndVerifyParameterValue()
{
    QString value=mValueLineEdit.text();

    QStringList sysParamNames = mpModelObject->getParentContainerObject()->getParameterNames();
    if(sysParamNames.contains(value))
    {
        return true;
    }

    //Strip trailing and leading spaces
    stripLTSpaces(value);

    bool onlyNumbers=true;
    if ( value[0].isNumber() || (value[0] == '-') )
    {
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
    }
    else
    {
        onlyNumbers = false;
    }

    // Set corrected text
    mValueLineEdit.setText(value);

    if(!onlyNumbers)
    {
       QMessageBox::critical(this->parentWidget(), "Error",
                             "Invalid Parameter string \""+ value +
                             "\". Only numbers are alowed. Nummeric strings like 1[eE][+-]5 will work. Resetting parameter value!");
    }

    return onlyNumbers;
}
