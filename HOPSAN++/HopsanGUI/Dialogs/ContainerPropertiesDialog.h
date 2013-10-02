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
//! @file   ContainerPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-24
//!
//! @brief Contains a class for manimulation of Container properties
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
