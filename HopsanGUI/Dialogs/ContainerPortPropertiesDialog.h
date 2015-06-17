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
//! @file   ContainerPortPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a class for manipulation of Container properties
//!
//$Id$

#ifndef CONTAINERPROPERTIESDIALOG_H
#define CONTAINERPROPERTIESDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>

#include "Dialogs/ModelObjectPropertiesDialog.h"

//Forward Declaration
class ContainerPort;

//! @todo We have three different properties dialog with basically the same "style", maybe we could have a class hierarchy, no big deal right now though
class ContainerPortPropertiesDialog : public ModelObjectPropertiesDialog
{
    Q_OBJECT

public:
    ContainerPortPropertiesDialog(ContainerPort *pContainerPort, QWidget *pParentWidget=0);

private:
    ContainerPort *mpContainerPort;
    QLineEdit *mpNameEdit;

private slots:
    void setValues();
};

#endif // CONTAINERPROPERTIESDIALOG_H
