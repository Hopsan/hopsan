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
class Component;

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    ParameterDialog(Component *coreComponent, QWidget *parent = 0);

protected slots:
    void setParameters();

private:
    Component *mpCoreComponent;

    QLabel *label;
    QLineEdit *lineEdit;

    std::vector<QLabel*> labelList;
    std::vector<QLineEdit*> lineEditList;
    QCheckBox *caseCheckBox;
    QCheckBox *fromStartCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QCheckBox *searchSelectionCheckBox;
    QCheckBox *backwardCheckBox;
    QDialogButtonBox *buttonBox;
    QPushButton *findButton;
    QPushButton *moreButton;
    QWidget *extension;
};

#endif // PARAMETERDIALOG_H
