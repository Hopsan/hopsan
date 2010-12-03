//!
//! @file   ComponentPropertiesDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#ifndef COMPONENTPROPERTIESDIALOG_H
#define COMPONENTPROPERTIESDIALOG_H

#include <QtGui>

class GUIModelObject;
class GUIComponent;
class ParameterLayout;

class ComponentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog(GUIComponent *pGUIComponent, QWidget *parent = 0);

protected slots:
    void okPressed();

protected:
    void setParametersAndStartValues();

private:
    GUIComponent *mpGUIComponent;

    void createEditStuff();

    QLabel *label;
    QLineEdit *lineEdit;

    QLineEdit *mpNameEdit;

    QVector<ParameterLayout *> mvParameterLayout;
    QVector<QVector<ParameterLayout *> > mvStartValueLayout;

    QDialogButtonBox *buttonBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QWidget *extension;
};


class ParameterLayout : public QGridLayout
{
    Q_OBJECT

public:
    ParameterLayout(QString dataName="", QString descriptionName="", double dataValue=0, QString unitName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);
    ParameterLayout(QString dataName="", QString descriptionName="", QString dataValue="", QString unitName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);

    QString getDescriptionName();
    QString getDataName();
    double getDataValue();
    QString getDataValueTxt();
    void setDataValueTxt(QString valueTxt);

protected slots:
    void showListOfSystemParameters();

protected:
    GUIModelObject *mpGUIModelObject;
    QLabel mDataNameLabel;
    QLabel mDescriptionNameLabel;
    QLineEdit mDataValuesLineEdit;
    QLabel mUnitNameLabel;
    QToolButton mSystemParameterToolButton;

private:
    void commonConstructorCode(QString dataName="", QString descriptionName="", QString dataValue="", QString unitName="", GUIModelObject *pGUIModelObject=0);
};

#endif // COMPONENTPROPERTIESDIALOG_H
