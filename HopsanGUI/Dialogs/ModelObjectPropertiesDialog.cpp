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

#include <QMessageBox>

#include "ModelObjectPropertiesDialog.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"

#include "Dialogs/ParameterSettingsLayout.h"

#include "UndoStack.h"
#include "Widgets/ModelWidget.h"


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
