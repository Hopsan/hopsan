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
#include "GUIPort.h"

ParameterSettingsLayout::ParameterSettingsLayout(const CoreParameterData &rParameterData, ModelObject *pModelObject, QWidget *pParent) : QGridLayout(pParent)
{
    mpModelObject = pModelObject;
    mParameterType = rParameterData.mType;

    // Set name label
    mName = rParameterData.mName;
    mNameLabel.setText(parseVariableDescription(mName));
    mNameLabel.setMinimumWidth(10);
    mNameLabel.setMaximumWidth(100);
    mNameLabel.adjustSize();
    mNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    // Set description label
    mDescriptionLabel.setMinimumWidth(100);
    mDescriptionLabel.setMaximumWidth(1000);
    mDescriptionLabel.setText(rParameterData.mDescription);
    //mDescriptionNameLabel.setWordWrap(true);
    mDescriptionLabel.adjustSize();

    // Set unit label
    mUnitLabel.setMinimumWidth(50);
    mUnitLabel.setMaximumWidth(50);
    mUnitLabel.setText(parseVariableUnit(rParameterData.mUnit));

    // Set value line editd
    mValueLineEdit.setMinimumWidth(100);
    mValueLineEdit.setMaximumWidth(100);
    mValueLineEdit.setText(rParameterData.mValue);

    // Set tool buttons
    mResetDefaultToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    mResetDefaultToolButton.setToolTip("Reset Default Value");

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    mSystemParameterToolButton.setToolTip("Map To System Parameter");

    // If dynamic parameter add switch button
    //! @todo dynamic parameters are deprecated
    if (rParameterData.mIsDynamic)
    {
        bool checked=false;
        //mDynamicEnabledCheckBox.setText("Dynamic");
        mDynamicEnabledCheckBox.setToolTip("Make Port (Experimental)");
        Port *pPort = mpModelObject->getPort(rParameterData.mName);
        if ( pPort != 0)
        {
            checked=true;

            // If the port exist and is connected then show in value editor that value will not be used
            // NOTE! We cant us "isEnabled from core since it will not be reset until a new simulation is run"
            //! @todo maybe dissable is not best way, you may want to change value without having to dissconnect first.
            if (pPort->isConnected())
            {
                mValueLineEdit.setEnabled(false);
            }
        }
        mDynamicEnabledCheckBox.setChecked(checked);
        addWidget(&mDynamicEnabledCheckBox, 0, 0);
    }

    // Add lables, edits and buttons
    int i=1;
    if(!mDescriptionLabel.text().isEmpty())
    {
        addWidget(&mDescriptionLabel, 0, i);
        ++i;
    }
    addWidget(&mNameLabel, 0, i);
    ++i;
    addWidget(&mValueLineEdit, 0, i);
    ++i;
    if(!mUnitLabel.text().isEmpty())
    {
        addWidget(&mUnitLabel, 0, i);
        ++i;
    }
    addWidget(&mResetDefaultToolButton, 0, i);
    ++i;
    addWidget(&mSystemParameterToolButton, 0, i);
    ++i;

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
        QAction *tempAction = menu.addAction(paramDataVector[i].mName+" = "+paramDataVector[i].mValue);
        tempAction->setIconVisibleInMenu(false);
        actionParamMap.insert(tempAction, paramDataVector[i].mName);
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
        Port * pPort = mpModelObject->createRefreshExternalPort(mName);
        if (pPort)
        {
            // Make sure that our new port has the "correct" angle
            pPort->setRotation(180);
        }
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
bool ParameterSettingsLayout::cleanAndVerifyParameterValue()
{
    QString value=mValueLineEdit.text();
    QStringList sysParamNames = mpModelObject->getParentContainerObject()->getParameterNames();
    QString error;

    bool isok = verifyParameterValue(value, mParameterType, sysParamNames, error);

    if(isok)
    {
        // Set corrected text
        mValueLineEdit.setText(value);
    }
    else
    {
        QMessageBox::critical(gpMainWindowWidget, "Error", error.append(" Resetting parameter value!"));
    }

    return isok;
}
