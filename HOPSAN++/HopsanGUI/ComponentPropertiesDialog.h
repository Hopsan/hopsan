//!
//! @file   ComponentPropertiesDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#ifndef ComponentPropertiesDialog_H
#define ComponentPropertiesDialog_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QVBoxLayout;

class GUIComponent;

class ComponentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog(GUIComponent *pGUIComponent, QWidget *parent = 0);

protected slots:
    void okPressed();
    void showListOfGlobalParameters();

protected:
    void setParameters();
    void setStartValues();

private:
    GUIComponent *mpGUIComponent;

    void createEditStuff();

    QLabel *label;
    QLineEdit *lineEdit;

    QLineEdit *mpNameEdit;
    std::vector<QLabel*> mParameterVarVector;
    std::vector<QLabel*> mParameterDescriptionVector;
    std::vector<QLabel*> mParameterUnitVector;
    std::vector<QLineEdit*> mParameterValueVector;
    std::vector<QToolButton*> mGlobalParameterVector;

    QVector<QToolButton *> mGlobalParameterButtons;
    QVector<QLineEdit *> mParameterValueLineEdits;

    QVector<QVector<QLabel*> > mStartDataNames;
    QVector<QVector<QLineEdit*> > mStartDataValues;
    QVector<QVector<QLabel*> > mStartDataUnits;

    QVBoxLayout *mpParameterGlobalLayout;

    QDialogButtonBox *buttonBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QWidget *extension;
};

#endif // ComponentPropertiesDialog_H
