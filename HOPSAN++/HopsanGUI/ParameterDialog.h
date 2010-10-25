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
//class GUISubsystem;
class GUISystem;

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    ParameterDialog(GUIComponent *pGUIComponent, QWidget *parent = 0);
    //ParameterDialog(GUISubsystem *pGUISubsystem, QWidget *parent = 0);
    ParameterDialog(GUISystem *pGUISubsystem, QWidget *parent = 0);

protected slots:
    void okPressed();

protected:
    void setParameters();
    void setStartValues();

private:
    GUIObject    *mpGUIObject;
    bool isGUISubsystem;

    void createEditStuff();

    QLabel *label;
    QLineEdit *lineEdit;

    QLineEdit *mpNameEdit;
    QLineEdit *mpCQSEdit;
    std::vector<QLabel*> mParameterVarVector;
    std::vector<QLabel*> mParameterDescriptionVector;
    std::vector<QLabel*> mParameterUnitVector;
    std::vector<QLineEdit*> mParameterValueVector;

    QVector<QVector<QLabel*> > mStartDataNames;
    QVector<QVector<QLineEdit*> > mStartDataValues;
    QVector<QVector<QLabel*> > mStartDataUnits;


    QDialogButtonBox *buttonBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QWidget *extension;
};

#endif // PARAMETERDIALOG_H
