/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   GlobalParametersWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-04
//!
//! @brief Contains a global parameter widget class
//!
//$Id$

#include <QtGui>

#include "GlobalParametersWidget.h"

#include <QWidget.h>
#include <QDialog.h>

#include <MainWindow.h>

//! Construtor for Global Parameters widget, where the user can see and change the global parameters in the model.
//! @param parent Pointer to the main window
GlobalParametersWidget::GlobalParametersWidget(MainWindow *parent)
    : QWidget(parent)
{
    mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("UndoWidget");
    this->resize(400,500);
    this->setWindowTitle("Undo History");

    mContents.clear();

    mpGlobalParametersTable = new QTableWidget(0,1,this);
    mpGlobalParametersTable->setBaseSize(400, 500);
    mpGlobalParametersTable->horizontalHeader()->setStretchLastSection(true);
    mpGlobalParametersTable->horizontalHeader()->hide();

    update();

    mpAddButton = new QPushButton(tr("&Add"), this);
    mpAddButton->setFixedHeight(30);
    mpAddButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpAddButton->setAutoDefault(false);
    QFont tempFont = mpAddButton->font();
    tempFont.setBold(true);
    mpAddButton->setFont(tempFont);

    mpRemoveButton = new QPushButton(tr("&Remove"), this);
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setFont(tempFont);

    mpGridLayout = new QGridLayout(this);
    mpGridLayout->addWidget(mpGlobalParametersTable, 0, 0);
    mpGridLayout->addWidget(mpAddButton, 1, 0);
    mpGridLayout->addWidget(mpRemoveButton, 2, 0);

    connect(this->mpAddButton,SIGNAL(clicked()),this,SLOT(openParameterDialog()));
    //! @todo Make the "Remove" button do something
}

//! Slot that adds a global parameter value
//! @param name Lookup name for the global parameter
//! @param value Value of the global parameter
void GlobalParametersWidget::setParameter(QString name, double value)
{
    //! @todo Check if parameter label is already registered in system. Cannot be done yet because map in system is not implemented.

    if(!name.startsWith("<"))
    {
        name.insert(0,"<");
    }
    if(!name.endsWith(">"))
    {
        name.append(">");
    }

    mContents.append(QPair<QString,double>(name, value));
    update();
}


//! Slot that opens "Add Parameter" dialog, where the user can add new global parameters
void GlobalParametersWidget::openParameterDialog()
{
    QDialog *pAddParameterDialog = new QDialog(this);
    pAddParameterDialog->setWindowTitle("Add Global Parameter");

    mpNameLabel = new QLabel("Name: ", this);
    mpNameBox = new QLineEdit(this);
    mpValueLabel = new QLabel("Value: ", this);
    mpValueBox = new QLineEdit(this);
    mpValueBox->setValidator(new QDoubleValidator(this));
    mpAddInDialogButton = new QPushButton("Add", this);
    mpDoneInDialogButton = new QPushButton("Done", this);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpAddInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpDoneInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(this);
    pDialogLayout->addWidget(mpNameLabel,0,0);
    pDialogLayout->addWidget(mpNameBox,0,1);
    pDialogLayout->addWidget(mpValueLabel,1,0);
    pDialogLayout->addWidget(mpValueBox,1,1);
    pDialogLayout->addWidget(pButtonBox,2,0,1,2);
    pAddParameterDialog->setLayout(pDialogLayout);
    pAddParameterDialog->show();

    connect(mpDoneInDialogButton,SIGNAL(clicked()),pAddParameterDialog,SLOT(close()));
    connect(mpAddInDialogButton,SIGNAL(clicked()),this,SLOT(addParameter()));
}


//! @Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
void GlobalParametersWidget::addParameter()
{
    bool ok;    
    setParameter(mpNameBox->text(), mpValueBox->text().toDouble(&ok));
    qDebug() << "ok = " << ok;
}


//! Updates the parameter table from the contents list
void GlobalParametersWidget::update()
{
    if(mContents.empty())
    {
        mpGlobalParametersTable->setColumnCount(1);
        mpGlobalParametersTable->setRowCount(1);
        mpGlobalParametersTable->verticalHeader()->hide();

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText("No global parameters set.");
        item->setBackgroundColor(QColor("white"));
        item->setTextAlignment(Qt::AlignCenter);
        mpGlobalParametersTable->setItem(0,0,item);
    }
    else
    {
        mpGlobalParametersTable->setColumnCount(2);
        mpGlobalParametersTable->removeRow(0);
        mpGlobalParametersTable->setColumnWidth(0, 120);
        mpGlobalParametersTable->verticalHeader()->show();
    }
    for(int i=0; i<mContents.size(); ++i)
    {
        QString valueString;
        valueString.setNum(mContents[i].second);
        this->mpGlobalParametersTable->insertRow(mpGlobalParametersTable->rowCount());
        mpGlobalParametersTable->setItem(mpGlobalParametersTable->rowCount()-1, 0, new QTableWidgetItem(mContents[i].first));
        mpGlobalParametersTable->setItem(mpGlobalParametersTable->rowCount()-1, 1, new QTableWidgetItem(valueString));
    }
}
