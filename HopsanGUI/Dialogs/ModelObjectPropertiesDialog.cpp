/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   ModelObjectPropertiesDialog.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-03
//!
//! @brief Contains the base class for modelobject properties dialogs
//!
//$Id$

#include <QMenu>
#include <QMessageBox>

#include "ModelObjectPropertiesDialog.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"

#include "UndoStack.h"
#include "Widgets/ModelWidget.h"

#include "global.h"
#include "Utilities/GUIUtilities.h"
#include "GUIPort.h"


ParameterSettingsLayout::ParameterSettingsLayout(const CoreParameterData &rParameterData, ModelObject *pModelObject, QWidget *pParent) : QGridLayout(pParent)
{
    mpModelObject = pModelObject;
    mParameterType = rParameterData.mType;

    // Set name label
    mName = rParameterData.mName;
    mNameLabel.setText(mName);
    //mNameLabel.setText(parseVariableDescription(mName));
    mNameLabel.setMinimumWidth(100);
    mNameLabel.setMaximumWidth(1000);
    mNameLabel.adjustSize();
    mNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    // Set description label
    mDescriptionLabel.setMinimumWidth(100);
    mDescriptionLabel.setMaximumWidth(1000);
    mDescriptionLabel.setText(rParameterData.mDescription);
    //mDescriptionNameLabel.setWordWrap(true);
    mDescriptionLabel.adjustSize();

    mQuantityLabel.setMinimumWidth(100);
    mQuantityLabel.setMaximumWidth(100);
    mQuantityLabel.setText(rParameterData.mQuantity);

    // Set unit label
    mUnitLabel.setMinimumWidth(50);
    mUnitLabel.setMaximumWidth(50);
    mUnitLabel.setText(parseVariableUnit(rParameterData.mUnit));

    // Set value line edit
    mValueLineEdit.setMinimumWidth(100);
    mValueLineEdit.setMaximumWidth(100);
    mValueLineEdit.setText(rParameterData.mValue);

    // Set tool buttons
    mResetDefaultToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    mResetDefaultToolButton.setToolTip("Reset Default Value");

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    mSystemParameterToolButton.setToolTip("Map To System Parameter");

    // Add labels, edits and buttons
    int i=1;
    addWidget(&mNameLabel, 0, i);
    ++i;
    addWidget(&mValueLineEdit, 0, i);
    ++i;
    addWidget(&mResetDefaultToolButton, 0, i);
    ++i;
    addWidget(&mSystemParameterToolButton, 0, i);
    ++i;
    addWidget(&mUnitLabel, 0, i);
    ++i;
    addWidget(&mQuantityLabel, 0, i);
    ++i;
    addWidget(&mDescriptionLabel, 0, i);
    ++i;

    // Determine value text color
    pickValueTextColor();

    // Connect signals to buttons
    connect(&mResetDefaultToolButton, SIGNAL(clicked()), this, SLOT(setDefaultValue()));
    connect(&mSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(showListOfSystemParameters()));
    connect(&mValueLineEdit, SIGNAL(textChanged(QString)), this, SLOT(pickValueTextColor()));
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

//void ParameterSettingsLayout::makePort(bool isPort)
//{
//    if (isPort)
//    {
//        Port * pPort = mpModelObject->createRefreshExternalPort(mName);
//        if (pPort)
//        {
//            // Make sure that our new port has the "correct" angle
//            pPort->setRotation(180);
//        }
//    }
//    else
//    {
//        mpModelObject->removeExternalPort(mName);
//    }
//}


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


ModelObjectPropertiesDialog::ModelObjectPropertiesDialog(ModelObject *pParentObject, QWidget *pParentWidget)
    : QDialog(pParentWidget)
{
    mpModelObject = pParentObject;
}

bool ModelObjectPropertiesDialog::setParameterValues(QVector<ParameterSettingsLayout*> &rParamLayouts)
{
    bool success = true;
    bool addedUndoPost = false;

    //Parameters
    for (int i=0; i<rParamLayouts.size(); ++i)
    {
        //Get the old value to se if it changed
        QString oldValueTxt = mpModelObject->getParameterValue(rParamLayouts[i]->getDataName());

        //Parameter has changed, add to undo stack and set the parameter
        bool isOk = rParamLayouts[i]->cleanAndVerifyParameterValue();

        if(isOk)
        {
            QString valueTxt = rParamLayouts[i]->getDataValueTxt();

            //This is done as a check as well
            if(!mpModelObject->setParameterValue(rParamLayouts[i]->getDataName(), valueTxt))
            {
                QMessageBox::critical(0, "Hopsan GUI",
                                      QString("'%1' is an invalid value for parameter '%2'.")
                                      .arg(valueTxt)
                                      .arg(rParamLayouts[i]->getDataName()));
                rParamLayouts[i]->setDataValueTxt(oldValueTxt);
                isOk = false;
            }
            if(oldValueTxt != valueTxt)
            {
                if(!addedUndoPost)
                {
                    mpModelObject->getParentContainerObject()->getUndoStackPtr()->newPost(UNDO_CHANGEDPARAMETERS);
                    addedUndoPost = true;
                }

                mpModelObject->getParentContainerObject()->getUndoStackPtr()->registerChangedParameter(mpModelObject->getName(),
                                                                                                       rParamLayouts[i]->getDataName(),
                                                                                                       oldValueTxt,
                                                                                                       valueTxt);
                mpModelObject->getParentContainerObject()->mpModelWidget->hasChanged();
            }
        }
        else
        {
            // Reset old value
            rParamLayouts[i]->setDataValueTxt(oldValueTxt);
        }
        success = success && isOk;
    }
    return success;
}
