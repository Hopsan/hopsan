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
//! @file   ModelObjectPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-03
//!
//! @brief Contains the base class for modelobject properties dialogs
//!
//$Id$

#ifndef MODELOBJECTPROPERTIESDIALOG_H
#define MODELOBJECTPROPERTIESDIALOG_H

#include <QtGui>

//Forward Declaration
class ModelObject;
class ParameterSettingsLayout;

class ModelObjectPropertiesDialog : public QDialog
{
    Q_OBJECT
public:
    ModelObjectPropertiesDialog(ModelObject *pParentObject, QWidget *pParentWidget);

protected:
    virtual bool setParameterValues(QVector<ParameterSettingsLayout*> &rParamLayouts);
    ModelObject *mpModelObject;
};

#endif // MODELOBJECTPROPERTIESDIALOG_H
