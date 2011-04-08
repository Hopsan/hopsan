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
    QHBoxLayout *mpNSamplesLayout;
    QLabel *mpNSamplesLabel;
    QHBoxLayout *mpCQSLayout;
    QLabel *mpCQSLabel;
    QLabel *mpCQSTypeLabel;
    QVBoxLayout *mpMainLayout;
    QDialogButtonBox *mpButtonBox;
    QPushButton *mpCancelButton;
    QPushButton *mpDoneButton;

private slots:
    void setValues();
    void browseUser();
    void browseIso();
    void browseScript();
};

#endif // CONTAINERPROPERTIESDIALOG_H
