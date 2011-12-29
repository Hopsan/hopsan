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
//! @file   ComponentGeneratorDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the component generator dialog
//!
//$Id$

#ifndef COMPONENTGENERATORDIALOG_H_INCLUDED
#define COMPONENTGENERATORDIALOG_H_INCLUDED

#include <QDialog>

#include "MainWindow.h"

class MainWindow;


class PortSpecification
{
public:
    PortSpecification(QString porttype = "ReadPort", QString nodetype = "NodeSignal", QString name = QString(), bool notrequired=false, QString defaultvalue=QString());
    QString porttype;
    QString nodetype;
    QString name;
    bool notrequired;
    QString defaultvalue;
};

class ParameterSpecification
{
public:
    ParameterSpecification(QString name = QString(), QString displayName = QString(), QString description = QString(), QString unit = QString(), QString init = QString());
    QString name;
    QString displayName;
    QString description;
    QString unit;
    QString init;
};

class ComponentGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    ComponentGeneratorDialog(MainWindow *parent = 0);

public slots:
    virtual void open();

private slots:
    void update();
    void addPort();
    void addParameter();
    void removePort();
    void removeParameter();
    void updateValues();
    void compile();

private:
    //Equations text edit
    QGridLayout *mpEquationsLayout;
    QGroupBox *mpEquationsGroupBox;
    QTextEdit *mpEquationsTextField;

    //Port Group Box
    QGroupBox *mpPortsGroupBox;
    QGridLayout *mpPortsLayout;
    QLabel *mpPortNamesLabel;
    QLabel *mpPortTypeLabel;
    QLabel *mpNodeTypelabel;
    QLabel *mpPortRequiredLabel;
    QLabel *mpPortDefaultLabel;
    QToolButton *mpAddPortButton;
    QVector<QLineEdit*> mvPortNameEdits;
    QVector<QComboBox*> mvPortTypeComboBoxes;
    QVector<QComboBox*> mvNodeTypeComboBoxes;
    QVector<QCheckBox*> mvRequiredCheckBoxes;
    QVector<QLineEdit*> mvPortDefaultEdits;
    QVector<QToolButton*> mvRemovePortButtons;

    //General Settings
    QLabel *mpComponentNameLabel;
    QLineEdit *mpComponentNameEdit;
    QLabel *mpComponentDisplayLabel;
    QLineEdit *mpComponentDisplayEdit;
    QLabel *mpComponentTypeLabel;
    QComboBox *mpComponentTypeComboBox;

    //Parameter Group Box
    QGroupBox *mpParametersGroupBox;
    QGridLayout *mpParametersLayout;
    QLabel *mpParametersNameLabel;
    QLabel *mpParametersDisplayLabel;
    QLabel *mpParametersDescriptionLabel;
    QLabel *mpParametersUnitLabel;
    QLabel *mpParametersInitLabel;
    QToolButton *mpAddParameterButton;
    QVector<QLineEdit*> mvParameterNameEdits;
    QVector<QLineEdit*> mvParameterDisplayEdits;
    QVector<QLineEdit*> mvParameterDescriptionEdits;
    QVector<QLineEdit*> mvParameterUnitEdits;
    QVector<QLineEdit*> mvParameterInitEdits;
    QVector<QToolButton*> mvRemoveParameterButtons;

    //Buttons
    QPushButton *mpCancelButton;
    QPushButton *mpCompileButton;
    QDialogButtonBox *mpButtonBox;

    //Main layout
    QGridLayout *mpLayout;

    //Member variables
    QList<PortSpecification> mPortList;
    QList<ParameterSpecification> mParametersList;
};

#endif // COMPONENTGENERATORDIALOG_H_INCLUDED
