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
//! @file   ContainerPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-24
//!
//! @brief Contains a class for manipulation of Container properties
//!
//$Id$

#ifndef CONTAINERPROPERTIESDIALOG_H
#define CONTAINERPROPERTIESDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QTextEdit>

#include "Dialogs/ModelObjectPropertiesDialog.h"

//Forward Declaration
class ContainerObject;
class ParameterSettingsLayout;

class ContainerPropertiesDialog : public ModelObjectPropertiesDialog
{
    Q_OBJECT

public:
    ContainerPropertiesDialog(ContainerObject *pContainerObject, QWidget *pParentWidget);

private:
    ContainerObject *mpContainerObject;

    QLineEdit *mpNameEdit;
    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLineEdit *mpNumLogSamplesEdit;
    QLineEdit *mpLogStartTimeEdit;
    QLineEdit *mpIsoIconScaleEdit;
    QLineEdit *mpUserIconScaleEdit;
    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;
    QCheckBox *mpSaveUndoCheckBox;
    QHBoxLayout *mpNameLayout;
    QLabel *mpNameLabel;
    QLabel *mpUserIconLabel;
    QLabel *mpIsoIconLabel;
    QPushButton *mpIsoIconBrowseButton;
    QPushButton *mpUserIconBrowseButton;
    QLabel *mpPyScriptLabel;
    QLineEdit *mpPyScriptPath;
    QPushButton *mpPyScriptBrowseButton;
    QGroupBox *mpAppearanceGroupBox;
    QGridLayout *mpAppearanceLayout;
    QGroupBox *mpSettingsGroupBox;
    QGridLayout *mpSettingsLayout;
    QGridLayout *mpTimeStepLayout;
    QCheckBox *mpTimeStepCheckBox;
    QCheckBox *mpUseStartValues;
    QLabel *mpTimeStepLabel;
    QLineEdit *mpTimeStepEdit;
    QHBoxLayout *mpNSamplesLayout;
    QHBoxLayout *mpCQSLayout;
    QLabel *mpCQSLabel;
    QLabel *mpCQSTypeLabel;
    QVBoxLayout *mpSettingsScrollLayout;
    QWidget *mpSettingsWidget;
    QLabel *mpAuthorLabel;
    QLabel *mpEmailLabel;
    QLabel *mpAffiliationLabel;
    QLabel *mpDescriptionLabel;
    QLineEdit *mpAuthorEdit;
    QLineEdit *mpEmailEdit;
    QLineEdit *mpAffiliationEdit;
    QTextEdit *mpDescriptionEdit;
    QVBoxLayout *mpScrollLayout;
    QWidget *mpPrimaryWidget;
    QVBoxLayout *mpMainLayout;
    QDialogButtonBox *mpButtonBox;
    QPushButton *mpCancelButton;
    QPushButton *mpDoneButton;
    QPushButton *mpEditPortPos;

    QGroupBox *mpSystemParametersGroupBox;
    QVector<ParameterSettingsLayout*> mvParameterLayoutPtrs;

protected slots:
    void editPortPos();

private slots:
    void fixTimeStepInheritance(bool value);
    void setValues();
    void browseUser();
    void browseIso();
    void browseScript();
    void clearLogData();
};

#endif // CONTAINERPROPERTIESDIALOG_H
