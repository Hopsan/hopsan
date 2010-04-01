//!
//! @file   ParameterDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class GUIObject;
class GUIComponent;
class Component;

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    ParameterDialog(GUIComponent *guiComponent, QWidget *parent = 0);

protected slots:
    void setParameters();

private:
    GUIComponent *mpGuiComponent;
    Component *mpCoreComponent;

    QLabel *label;
    QLineEdit *lineEdit;

    QLineEdit *mpNameEdit;
    std::vector<QLabel*> mVarVector;
    std::vector<QLabel*> mDescriptionVector;
    std::vector<QLabel*> mUnitVector;
    std::vector<QLineEdit*> mValueVector;
    QDialogButtonBox *buttonBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QWidget *extension;
};

#endif // PARAMETERDIALOG_H
