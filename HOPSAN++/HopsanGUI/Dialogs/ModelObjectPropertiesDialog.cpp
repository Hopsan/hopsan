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
//! @file   ModelObjectPropertiesDialog.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-03
//!
//! @brief Contains the base class for modelobject properties dialogs
//!
//$Id$

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
                    mpModelObject->getParentContainerObject()->getUndoStackPtr()->newPost("changedparameters");
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
