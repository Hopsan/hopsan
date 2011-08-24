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

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   SystemParametersWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-04
//!
//! @brief Contains a System parameter widget class
//!
//$Id$

#include <QtGui>
#include <QWidget>
#include <QDialog>

#include "../MainWindow.h"
#include "SystemParametersWidget.h"
#include "ProjectTabWidget.h"
#include "../GUIObjects/GUISystem.h"
#include "../common.h"


TypeComboBox::TypeComboBox(size_t row, size_t column, SystemParameterTableWidget *parent)
    : QComboBox(parent)
{
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mRow = row;
    mColumn = column;
    mParent = parent;
    addItem("double");
    addItem("integer");
    addItem("bool");
    addItem("string");
    connect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(typeHasChanged(QString)), Qt::UniqueConnection);
}


void TypeComboBox::typeHasChanged(QString /*newType*/)
{
    qDebug() << "aksJLKJAFLKJDFLkjsdlfkj   ";
    if(mRow > -1)
    {
        mParent->setCurrentCell(mRow, mColumn);
        mParent->changeParameter();
    }
}


//! Construtor for System Parameters widget, where the user can see and change the System parameters in the model.
//! @param parent Pointer to the main window
SystemParametersWidget::SystemParametersWidget(MainWindow *parent)
    : QWidget(parent)
{
    //mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("SystemParameterWidget");
    this->resize(400,500);
    this->setWindowTitle("System Parameters");

    mpSystemParametersTable = new SystemParameterTableWidget(0,1,this);

    mpAddButton = new QPushButton(tr("&Set"), this);
    mpAddButton->setFixedHeight(30);
    mpAddButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpAddButton->setAutoDefault(false);
    QFont tempFont = mpAddButton->font();
    tempFont.setBold(true);
    mpAddButton->setFont(tempFont);
    mpAddButton->setEnabled(false);

    mpRemoveButton = new QPushButton(tr("&Unset"), this);
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setFont(tempFont);
    mpRemoveButton->setEnabled(false);

    mpGridLayout = new QGridLayout(this);
    mpGridLayout->addWidget(mpSystemParametersTable, 0, 0);
    mpGridLayout->addWidget(mpAddButton, 1, 0);
    mpGridLayout->addWidget(mpRemoveButton, 2, 0);

    mpGridLayout->setContentsMargins(4,4,4,4);

    update();

    connect(mpAddButton, SIGNAL(clicked()), mpSystemParametersTable, SLOT(openComponentPropertiesDialog()));
    connect(mpRemoveButton, SIGNAL(clicked()), mpSystemParametersTable, SLOT(removeSelectedParameters()));
    connect(gpMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(update()));//StrÃ¶ssel!
    connect(gpMainWindow->mpProjectTabs, SIGNAL(newTabAdded()), this, SLOT(update()));//StrÃ¶ssel!
}


void SystemParametersWidget::update()
{
    if(gpMainWindow->mpProjectTabs->count()>0)
    {
        mpAddButton->setEnabled(true);
        mpRemoveButton->setEnabled(true);
    }
    else
    {
        mpAddButton->setEnabled(false);
        mpRemoveButton->setEnabled(false);
    }

    mpSystemParametersTable->update();
}


SystemParameterTableWidget::SystemParameterTableWidget(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setBaseSize(400, 500);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->hide();

    update();
}


void SystemParameterTableWidget::keyPressEvent(QKeyEvent *event)
{
    QTableWidget::keyPressEvent(event);
    if(event->key() == Qt::Key_Delete)
    {
        qDebug() << "Delete current System Parameter Widget Items";
        removeSelectedParameters();
    }
}


//! @brief Used for parameter changes done directly in the label
void SystemParameterTableWidget::changeParameter(QTableWidgetItem */*item*/)
{
    //Filter out value labels
//    if(item->column() == 1)
    {
        QTableWidgetItem *nameItem = this->item(currentRow(), 0);
        QTableWidgetItem *valueItem = this->item(currentRow(), 1);
        QString parName = nameItem->text();
        QString parValue = valueItem->text();

        QString typeName;
        if(QComboBox *typeBox = qobject_cast<QComboBox *>(this->cellWidget(currentRow(), 2)))
        {
            typeName = typeBox->currentText();
        }

//        QString apa = item->text();
//        double ko = getParameter(parName);
        if(parValue != getParameter(parName))
        {
            gpMainWindow->mpProjectTabs->getCurrentTab()->hasChanged();
        }

        //Do not do update, then crash due to the rebuild of the QTableWidgetItems
        setParameter(parName, parValue, "", "", typeName, false);
        update();
    }
}


QString SystemParameterTableWidget::getParameter(QString name)
{
    return gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getSystemParameter(name);
}


bool SystemParameterTableWidget::hasParameter(QString name)
{
    return gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->hasSystemParameter(name);
}


////! Slot that adds a System parameter value
////! @param name Lookup name for the System parameter
////! @param value Value of the System parameter
//void SystemParameterTableWidget::setParameter(QString name, QString value, bool doUpdate)
//{
//    //Error check
//    if(!(gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->setSystemParameter(name, value)))
//    {
//        QMessageBox::critical(0, "Hopsan GUI",
//                              QString("'%1' is an invalid name for a system parameter.")
//                              .arg(name));
//        return;
//    }
//    if(doUpdate)
//    {
//        update();
//    }
//    emit modifiedSystemParameter();
//}


//! Slot that adds a System parameter value
//! @param name Lookup name for the System parameter
//! @param value Value of the System parameter
void SystemParameterTableWidget::setParameter(QString name, QString valueTxt, QString descriptionTxt, QString unitTxt, QString typeTxt, bool doUpdate)
{
//    //Error check
//    bool isDbl;
//    double value = valueTxt.toDouble((&isDbl));
//    if(!(isDbl))
//    {
//        QMessageBox::critical(0, "Hopsan GUI",
//                              QString("'%1' is not a valid number.")
//                              .arg(valueTxt));
//        QString oldValue = gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getSystemParameter(name);
//        QList<QTableWidgetItem *> items = selectedItems();
//        //Error if size() > 1, but it should not be! :)
//        for(int i = 0; i<items.size(); ++i)
//        {
//            items[i]->setText(oldValue);
//        }
//    }
//    else
    {
//        setParameter(name, valueTxt, doUpdate);
        //Error check
        if(!(gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->setSystemParameter(name, valueTxt, descriptionTxt, unitTxt, typeTxt)))
        {
            QMessageBox::critical(0, "Hopsan GUI",
                                  QString("'%1' is an invalid name for a system parameter.")
                                  .arg(name));
            return;
        }
        if(doUpdate)
        {
            update();
        }
        emit modifiedSystemParameter();
    }
}


void SystemParameterTableWidget::setParameters()
{
    //    if(gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getNumberOfSystemParameters() > 0)
    //    {
    for(int i=0; i<rowCount(); ++i)
    {
        QString name = item(i, 0)->text();
        QString value = item(i, 1)->text();
        setParameter(name, value);
    }
    //    }
}


//! Slot that removes all selected System parameters in parameter table
//! @todo This shall remove the actual System parameters when they have been implemented, wherever they are stored.
void SystemParameterTableWidget::removeSelectedParameters()
{
    if(gpMainWindow->mpProjectTabs->count()>0)
    {
        QList<QTableWidgetItem *> pSelectedItems = selectedItems();
        QStringList parametersToRemove;
        QString tempName;

        for(int i=0; i<pSelectedItems.size(); ++i)
        {
            tempName = item(pSelectedItems[i]->row(),0)->text();
            if(!parametersToRemove.contains(tempName))
            {
                parametersToRemove.append(tempName);
                gpMainWindow->mpProjectTabs->getCurrentTab()->hasChanged();
            }
            removeCellWidget(pSelectedItems[i]->row(), pSelectedItems[i]->column());
            delete pSelectedItems[i];
        }

        for(int j=0; j<parametersToRemove.size(); ++j)
        {
            qDebug() << "Removing: " << parametersToRemove[j];
            gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->removeSystemParameter(parametersToRemove.at(j));
        }
    }
    update();
}


//! Slot that opens "Add Parameter" dialog, where the user can add new System parameters
void SystemParameterTableWidget::openComponentPropertiesDialog()
{
    QDialog *pAddComponentPropertiesDialog = new QDialog(this);
    pAddComponentPropertiesDialog->setWindowTitle("Set System Parameter");

    mpNameLabel = new QLabel("Name: ", this);
    mpNameBox = new QLineEdit(this);
    mpValueLabel = new QLabel("Value: ", this);
    mpValueBox = new QLineEdit(this);
    //mpValueBox->setValidator(new QDoubleValidator(this));
    mpTypeLabel = new QLabel("Type: ", this);
    mpTypeBox = new TypeComboBox(-1, -1, this);
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
    pDialogLayout->addWidget(mpTypeLabel,2,0);
    pDialogLayout->addWidget(mpTypeBox,2,1);
    pDialogLayout->addWidget(pButtonBox,3,0,1,2);
    pAddComponentPropertiesDialog->setLayout(pDialogLayout);
    pAddComponentPropertiesDialog->show();

    connect(mpDoneInDialogButton,SIGNAL(clicked()),pAddComponentPropertiesDialog,SLOT(close()));
    connect(mpAddInDialogButton,SIGNAL(clicked()),this,SLOT(addParameter()));
}


//QComboBox *SystemParameterTableWidget::createTypeComboBox()
//{
//    QComboBox *box = new QComboBox(this);
//    box->addItem("double");
//    box->addItem("integer");
//    box->addItem("bool");
//    box->addItem("string");
//    connect(box, SIGNAL(currentIndexChanged(QString)), this, SLOT(typeHasChanged(QString)), Qt::UniqueConnection);
//    return box;
//}


//! @brief Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
void SystemParameterTableWidget::addParameter()
{
    qDebug() << mpTypeBox->currentText();
    setParameter(mpNameBox->text(), mpValueBox->text(), "", "", mpTypeBox->currentText());
    gpMainWindow->mpProjectTabs->getCurrentTab()->hasChanged();
}


//! Updates the parameter table from the contents list
void SystemParameterTableWidget::update()
{
    QVector<QString> qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes;

    clear();
    if(gpMainWindow->mpProjectTabs->count()>0)
    {
        gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getSystemParameters(qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes);
    }

    disconnect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(changeParameter(QTableWidgetItem*)));

    if(qParameterNames.isEmpty())
    {
        setColumnCount(1);
        setRowCount(1);
        verticalHeader()->hide();

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText("No System parameters set.");
        item->setBackgroundColor(QColor("white"));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        setItem(0,0,item);
    }
    else
    {
        setRowCount(0);
        setColumnCount(3);
        setColumnWidth(0, 120);
        verticalHeader()->show();

        for(int i=0; i<qParameterNames.size(); ++i)
        {
//            QString valueString;
//            valueString.setNum(it.value());
            insertRow(rowCount());
            QTableWidgetItem *nameItem = new QTableWidgetItem(qParameterNames[i]);
            QTableWidgetItem *valueItem = new QTableWidgetItem(qParameterValues[i]);
            nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            setItem(rowCount()-1, 0, nameItem);
            setItem(rowCount()-1, 1, valueItem);
            TypeComboBox *typeBox = new TypeComboBox(rowCount()-1, 2, this);

            for(int j=0; j<typeBox->count(); ++j)
            {
                if(qTypes[i] == typeBox->itemText(j))
                {
                    typeBox->setCurrentIndex(j);
                    break;
                }
            }

            setCellWidget(rowCount()-1, 2, typeBox);
        }
        connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(changeParameter(QTableWidgetItem*)), Qt::UniqueConnection);
    }
}
