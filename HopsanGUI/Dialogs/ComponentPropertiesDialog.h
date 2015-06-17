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
//! @file   ComponentPropertiesDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with parameters
//!
//$Id$

#ifndef COMPONENTPROPERTIESDIALOG_H
#define COMPONENTPROPERTIESDIALOG_H

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialogButtonBox>

#include "Dialogs/ModelObjectPropertiesDialog.h"

class Component;
class ParameterSettingsLayout;

class ComponentPropertiesDialog : public ModelObjectPropertiesDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog(Component *pComponent, QWidget *pParent=0);

protected slots:
    void okPressed();
    void editPortPos();

protected:
    void setParametersAndStartValues();
    void recompileCppFromDialog();
    virtual void closeEvent(QCloseEvent *);
    virtual void reject();

private:
    Component *mpComponent;

    void createEditStuff();
    void createCppEditStuff();
    void createModelicaEditStuff();
    bool interpretedAsStartValue(QString &parameterDescription);

    QLabel *mpLabel;
    QLineEdit *mpLineEdit;
    QLineEdit *mpNameEdit;

    QVector<ParameterSettingsLayout*> mvParameterLayout;
    QVector<ParameterSettingsLayout*> mvStartValueLayout;

    QDialogButtonBox *mpButtonBox;
    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;
    QPushButton *mpEditPortPos;
    QWidget *mpExtension;

    QSpinBox *mpInputPortsSpinBox;
    QSpinBox *mpOutputPortsSpinBox;
    QTextEdit *mpTextEdit;
};

#endif // COMPONENTPROPERTIESDIALOG_H
