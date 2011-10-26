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
//! @file   OptimizationDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-10-24
//!
//! @brief Contains a class for the optimization dialog
//!
//$Id$

#ifndef OPTIMIZATIONDIALOG_H
#define OPTIMIZATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QToolButton>
#include <QComboBox>

#include "MainWindow.h"

class MainWindow;

class OptimizationDialog : public QDialog
{
    Q_OBJECT

public:
    OptimizationDialog(MainWindow *parent = 0);

private:
    void generateScriptFile();

public slots:
    virtual void open();

private slots:
    void updateOutputBox();
    void run();

private:
    QTreeWidget *mpParametersList;

    QTextEdit *mpOutputBox;

    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpGenerateButton;
    QPushButton *mpRunButton;
    QDialogButtonBox *mpButtonBox;

    QGridLayout *mpLayout;

    QString mScript;
};

#endif // OPTIMIZATIONDIALOG_H
