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

#include "MainWindow.h"
#include "GlobalParametersWidget.h"

#include <QWidget>
#include <QDialog>

#include "MainWindow.h"
#include "ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"

#include "common.h"

//! Construtor for Global Parameters widget, where the user can see and change the global parameters in the model.
//! @param parent Pointer to the main window
GlobalParametersWidget::GlobalParametersWidget(MainWindow *parent)
    : QWidget(parent)
{
    //mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("UndoWidget");
    this->resize(400,500);
    this->setWindowTitle("Undo History");

    mpGlobalParametersTable = new QTableWidget(0,1,this);
    mpGlobalParametersTable->setBaseSize(400, 500);
    mpGlobalParametersTable->horizontalHeader()->setStretchLastSection(true);
    mpGlobalParametersTable->horizontalHeader()->hide();

    update();

    mpAddButton = new QPushButton(tr("&Set"), this);
    mpAddButton->setFixedHeight(30);
    mpAddButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpAddButton->setAutoDefault(false);
    QFont tempFont = mpAddButton->font();
    tempFont.setBold(true);
    mpAddButton->setFont(tempFont);

    mpRemoveButton = new QPushButton(tr("&Unset"), this);
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setFont(tempFont);

    mpGridLayout = new QGridLayout(this);
    mpGridLayout->addWidget(mpGlobalParametersTable, 0, 0);
    mpGridLayout->addWidget(mpAddButton, 1, 0);
    mpGridLayout->addWidget(mpRemoveButton, 2, 0);

    connect(mpAddButton,SIGNAL(clicked()),this,SLOT(openComponentPropertiesDialog()));
    connect(mpRemoveButton,SIGNAL(clicked()),this,SLOT(removeSelectedParameters()));
}


double GlobalParametersWidget::getParameter(QString name)
{
    return gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getGlobalParameter(name);
}


bool GlobalParametersWidget::hasParameter(QString name)
{
    return gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->hasGlobalParameter(name);
}


//! Slot that adds a global parameter value
//! @param name Lookup name for the global parameter
//! @param value Value of the global parameter
void GlobalParametersWidget::setParameter(QString name, double value)
{
    gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->setGlobalParameter(name, value);
    update();

    emit modifiedGlobalParameter();
}


void GlobalParametersWidget::setParameters()
{
    if(gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getNumberOfGlobalParameters() > 0)
    {
        for(size_t i=0; i<mpGlobalParametersTable->rowCount(); ++i)
        {
            QString name = mpGlobalParametersTable->item(i, 0)->text();
            double value = mpGlobalParametersTable->item(i, 1)->text().toDouble();
            gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->setGlobalParameter(name, value);
        }
    }
}


//! Slot that removes all selected global parameters in parameter table
//! @todo This shall remove the actual global parameters when they have been implemented, wherever they are stored.
void GlobalParametersWidget::removeSelectedParameters()
{
    QList<QTableWidgetItem *> pSelectedItems = mpGlobalParametersTable->selectedItems();
    QStringList parametersToRemove;
    QString tempName;
    double tempValue;

    for(size_t i=0; i<pSelectedItems.size(); ++i)
    {
        tempName = mpGlobalParametersTable->item(pSelectedItems[i]->row(),0)->text();
        if(!parametersToRemove.contains(tempName))
        {
            parametersToRemove.append(tempName);
        }
    }

    for(size_t j=0; j<parametersToRemove.size(); ++j)
    {
        qDebug() << "Removing: " << parametersToRemove[j];
        gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->removeGlobalParameter(parametersToRemove.at(j));
    }

    update();
}


//! Slot that opens "Add Parameter" dialog, where the user can add new global parameters
void GlobalParametersWidget::openComponentPropertiesDialog()
{
    QDialog *pAddComponentPropertiesDialog = new QDialog(this);
    pAddComponentPropertiesDialog->setWindowTitle("Set Global Parameter");

    mpNameLabel = new QLabel("Name: ", this);
    mpNameBox = new QLineEdit(this);
    mpValueLabel = new QLabel("Value: ", this);
    mpValueBox = new QLineEdit(this);
    mpValueBox->setValidator(new QDoubleValidator(this));
    mpAddInDialogButton = new QPushButton("Set", this);
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
    pAddComponentPropertiesDialog->setLayout(pDialogLayout);
    pAddComponentPropertiesDialog->show();

    connect(mpDoneInDialogButton,SIGNAL(clicked()),pAddComponentPropertiesDialog,SLOT(close()));
    connect(mpAddInDialogButton,SIGNAL(clicked()),this,SLOT(addParameter()));
}


//! @Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
void GlobalParametersWidget::addParameter()
{
    bool ok;    
    setParameter(mpNameBox->text(), mpValueBox->text().toDouble(&ok));
}


//! Updates the parameter table from the contents list
void GlobalParametersWidget::update()
{
    mpGlobalParametersTable->clear();

    if(gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getNumberOfGlobalParameters() == 0)
    {
        mpGlobalParametersTable->setColumnCount(1);
        mpGlobalParametersTable->setRowCount(1);
        mpGlobalParametersTable->verticalHeader()->hide();

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText("No global parameters set.");
        item->setBackgroundColor(QColor("white"));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        mpGlobalParametersTable->setItem(0,0,item);
    }
    else
    {
        mpGlobalParametersTable->setRowCount(0);
        mpGlobalParametersTable->setColumnCount(2);
        mpGlobalParametersTable->setColumnWidth(0, 120);
        mpGlobalParametersTable->verticalHeader()->show();
    }
    QMap<std::string, double>::iterator it;
    QMap<std::string, double> tempMap = gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getGlobalParametersMap();
    for(it=tempMap.begin(); it!=tempMap.end(); ++it)
    {
        QString valueString;
        valueString.setNum(it.value());
        this->mpGlobalParametersTable->insertRow(mpGlobalParametersTable->rowCount());
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString(it.key().c_str()));
        QTableWidgetItem *valueItem = new QTableWidgetItem(valueString);
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        mpGlobalParametersTable->setItem(mpGlobalParametersTable->rowCount()-1, 0, nameItem);
        mpGlobalParametersTable->setItem(mpGlobalParametersTable->rowCount()-1, 1, valueItem);
    }
}
