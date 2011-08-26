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

//Forward Declaration
class GUIContainerObject;

class ContainerPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget);

private:
    GUIContainerObject *mpContainerObject;

    QLineEdit *mpNameEdit;
    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLineEdit *mpNSamplesEdit;
    QLineEdit *mpIsoIconScaleEdit;
    QLineEdit *mpUserIconScaleEdit;
    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;
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
    QLabel *mpTimeStepLabel;
    QLineEdit *mpTimeStepEdit;
    QHBoxLayout *mpNSamplesLayout;
    QLabel *mpNSamplesLabel;
    QHBoxLayout *mpCQSLayout;
    QLabel *mpCQSLabel;
    QLabel *mpCQSTypeLabel;
    QVBoxLayout *mpMainLayout;
    QDialogButtonBox *mpButtonBox;
    QPushButton *mpCancelButton;
    QPushButton *mpDoneButton;

    QGroupBox *mpSystemParametersGroupBox;
    QGridLayout *mpSystemParametersLayout;
    QList<QLabel *> mSystemParameterLabels;
    QList<QLineEdit *> mSystemParameterLineEdits;

private slots:
    void setValues();
    void browseUser();
    void browseIso();
    void browseScript();
};

#endif // CONTAINERPROPERTIESDIALOG_H
