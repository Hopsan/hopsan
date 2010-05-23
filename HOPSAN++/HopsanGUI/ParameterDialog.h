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
class GUISubsystem;
class Component;
class ComponentSystem;

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    ParameterDialog(GUIComponent *pGUIComponent, QWidget *parent = 0);
    ParameterDialog(GUISubsystem *pGUISubsystem, QWidget *parent = 0);

protected slots:
    void setParameters();

private:
    GUIObject    *mpGUIObject;
    bool isGUISubsystem;

    void createEditStuff();

    QLabel *label;
    QLineEdit *lineEdit;

    QLineEdit *mpNameEdit;
    QLineEdit *mpCQSEdit;
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
