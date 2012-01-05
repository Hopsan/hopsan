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

#include "MainWindow.h"

class GUIModelObject;
class GUIComponent;
class ParameterLayout;

class ComponentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog(GUIComponent *pGUIComponent, MainWindow *parent = 0);

protected slots:
    void okPressed();
    void editPortPos();

protected:
    bool setValuesToSystem(QVector<ParameterLayout *> &vParLayout);
    void setParametersAndStartValues();

private:
    GUIComponent *mpGUIComponent;

    void createEditStuff();
    bool interpretedAsStartValue(QString &parameterDescription);
    void verifyNewValue(QString &value);

    QLabel *mpLabel;
    QLineEdit *mpLineEdit;

    QLineEdit *mpNameEdit;

    QVector<ParameterLayout *> mvParameterLayout;
    QVector<ParameterLayout *> mvStartValueLayout;

    QDialogButtonBox *mpButtonBox;
    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;
    QPushButton *mpEditPortPos;
    QWidget *mpExtension;
};


//! @todo Move this class to better place, it is used by other than just GUIComponent
class ParameterLayout : public QGridLayout
{
    Q_OBJECT
    friend class ComponentPropertiesDialog;
public:
 //   ParameterLayout(QString dataName="", QString descriptionName="", double dataValue=0, QString unitName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);
    ParameterLayout(QString dataName="", QString descriptionName="", QString dataValue="", QString unitName="", QString typeName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);

    QString getDescriptionName();
    QString getDataName();
    double getDataValue();
    QString getDataValueTxt();
    void setDataValueTxt(QString valueTxt);

protected slots:
    void setDefaultValue();
    void showListOfSystemParameters();
    void pickColor();

protected:
    GUIModelObject *mpGUIModelObject;
    QLabel mDataNameLabel;
    QLabel mDescriptionNameLabel;
    QLineEdit mDataValuesLineEdit;
    QLabel mUnitNameLabel;
    QToolButton mResetDefaultToolButton;
    QToolButton mSystemParameterToolButton;

    QString mDataName;

private:
    void commonConstructorCode(QString dataName="", QString descriptionName="", QString dataValue="", QString unitName="",  QString typeName="", GUIModelObject *pGUIModelObject=0);
};

#endif // COMPONENTPROPERTIESDIALOG_H
