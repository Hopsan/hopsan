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

#include "MainWindow.h"
#include "SystemParametersWidget.h"
#include "ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"
#include "common.h"


//! @brief Construtor for System Parameters widget, where the user can see and change the System parameters in the model.
//! @param parent Pointer to the main window
SystemParametersWidget::SystemParametersWidget(MainWindow *parent)
    : QWidget(parent)
{
    mpContainerObject=0;
    //Set the name and size of the main window
    this->setObjectName("SystemParameterWidget");
    this->setWindowTitle("System Parameters");

    mpAddButton = new QPushButton(tr("&Add"));
    mpAddButton->setFixedHeight(30);
    mpAddButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpAddButton->setAutoDefault(false);
    mpAddButton->setEnabled(false);
    QFont buttonFont = mpAddButton->font();
    buttonFont.setBold(true);
    mpAddButton->setFont(buttonFont);

    mpRemoveButton = new QPushButton(tr("&Unset"));
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setEnabled(false);
    mpRemoveButton->setFont(buttonFont);

    mpSysParamListView = new QTableView();
    mpSysParamListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpSysParamListView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ParamTypeComboBoxDelegate *pComboBoxDelegate = new ParamTypeComboBoxDelegate();
    mpSysParamListView->setItemDelegateForColumn(2, pComboBoxDelegate);

    QGridLayout *pGridLayout = new QGridLayout(this);
    pGridLayout->addWidget(mpSysParamListView, 0, 0);
    pGridLayout->addWidget(mpAddButton, 1, 0);
    pGridLayout->addWidget(mpRemoveButton, 2, 0);

    pGridLayout->setContentsMargins(4,4,4,4);

    update();

    connect(mpAddButton, SIGNAL(clicked()), this, SLOT(openAddParameterDialog()));
    connect(mpRemoveButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
}

void SystemParametersWidget::update(ContainerObject *pNewContainer)
{
    if (mpContainerObject != pNewContainer)
    {
        mpContainerObject = pNewContainer;
        this->update();
    }
}

void SystemParametersWidget::update()
{
    if ( (mpContainerObject!=0) && (gpMainWindow->mpProjectTabs->count()>0) )
    {
        mpAddButton->setEnabled(true);
        mpRemoveButton->setEnabled(true);

        QAbstractItemModel *pOldModel = mpSysParamListView->model();
        SysParamListModel *pModel = new SysParamListModel(mpContainerObject, this);
        mpSysParamListView->setModel(pModel);
        delete pOldModel;

        mpSysParamListView->show();

        qDebug() << "--------------List isEnabled: " << mpSysParamListView->isEnabled();
        qDebug() << "--------------List isHidden: " << mpSysParamListView->isHidden();
        qDebug() << "--------------List isVisible: " << mpSysParamListView->isVisible();
        qDebug() << "--------------SysParWidget isVisible: " << this->isVisible();
        qDebug() << "--------------SysParWidget Parent isVisible: " << this->parentWidget()->isVisible();
    }
    else
    {
        mpAddButton->setEnabled(false);
        mpRemoveButton->setEnabled(false);
    }
}


//SystemParameterListWidget::SystemParameterListWidget(QWidget *pParentWidget)
//    : QListWidget(pParentWidget)
//{
//    //setFocusPolicy(Qt::StrongFocus);
//    //setSelectionMode(QAbstractItemView::SingleSelection);

//    //setBaseSize(400, 500);
//    //horizontalHeader()->setStretchLastSection(true);
//    //horizontalHeader()->hide();

//    refreshTable();
//}


//void SystemParameterListWidget::keyPressEvent(QKeyEvent *event)
//{
//    QListWidget::keyPressEvent(event);
//    if(event->key() == Qt::Key_Delete)
//    {
//        qDebug() << "Delete current System Parameter Widget Items";
//        removeSelectedParameters();
//    }
//}


//void SystemParameterListWidget::changeParameterName(const QString oldName)
//{
////    const int row = currentRow();
////    QString newParName = qobject_cast<SystemParameterTableItem*>(this->cellWidget(row,0))->mName.text();
////    if (newParName != oldName)
////    {
////        //Change name by removing system parameter before adding a new one
////        removeParameter(oldName);
////        changeParameter();
////    }
//}
////! @brief Used to update parameters from changes done directly in the labels
//void SystemParameterListWidget::changeParameter()
//{
//    const int row = currentRow();

//    //QString parName = qobject_cast<SystemParameterTableItem*>(this->cellWidget(row,0))->mName.text();
//   // QString newParValue = qobject_cast<SystemParameterTableItem*>(this->cellWidget(row,0))->mValue.text();
//    //QString newParType = qobject_cast<SystemParameterTableItem*>(this->cellWidget(row,0))->mType.currentText();
////    if(QComboBox *typeBox = qobject_cast<QComboBox*>(this->cellWidget(pItem->row(), 2)))
////    {
////        newParType = typeBox->currentText();
////    }

////    // Check if name was changed
////    if (pItem->column() == 0)
////    {
////        //Change name by removing system parameter before adding a new one
////        qDebug() << "Change name";
////        removeParameter(mParameterNames.at(pItem->row()));
////        mParameterNames.remove(pItem->row());
////    }

//    //Do not do update, then crash due to the rebuild of the QTableWidgetItems
//    //setParameter(parName, newParValue, "", "", newParType, false);

//    refreshTable();
//}


//QString SystemParameterListWidget::getParameterValue(QString name)
//{
//    return gpMainWindow->mpProjectTabs->getCurrentContainer()->getParameterValue(name);
//}


//bool SystemParametersWidget::hasParameter(QString name)
//{
//    return mpContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(name);
//}


////! @brief Slot that adds a System parameter value
////! @param name Lookup name for the System parameter
////! @param value Value of the System parameter
//void SystemParametersWidget::setParameter(QString name, QString valueTxt, QString descriptionTxt, QString unitTxt, QString typeTxt)
//{
//    CoreParameterData oldParamData;
//    mpContainerObject->getParameter(name, oldParamData);

//    //Error check
//    if(!(mpContainerObject->getCoreSystemAccessPtr()->setSystemParameter(name, valueTxt, descriptionTxt, unitTxt, typeTxt)))
//    {
//        QMessageBox::critical(0, "Hopsan GUI",
//                              QString("'%1' is an invalid name for a system parameter or '%2' is an invalid value.")
//                              .arg(name, valueTxt));
//        return;
//    }

//    //! @todo check if other stuff then value has changed, at least type
//    //! @todo dont go through main window to tag a tab as changed, should go through container
//    if(oldParamData.mValue != mpContainerObject->getParameterValue(name))
//    {
//        gpMainWindow->mpProjectTabs->getCurrentTab()->hasChanged();
//    }
//}


//void SystemParameterTableWidget::setAllParameters()
//{
//    for(int i=0; i<rowCount(); ++i)
//    {
//        QString name = item(i, 0)->text();
//        QString value = item(i, 1)->text();
//        setParameter(name, value);
//    }
//}


////! @brief Slot that removes all selected System parameters in parameter table
////! @todo should access container objects directly not go through the project tab
//void SystemParameterListWidget::removeSelectedParameters()
//{
//    if(gpMainWindow->mpProjectTabs->count()>0)
//    {
//        //QList<QTableWidgetItem*> pSelectedItems = selectedItems();
//        //QStringList parametersToRemove;
//        //QString tempName;

//        //for(int i=0; i<pSelectedItems.size(); ++i)
//        {
//            //removeParameter(item(pSelectedItems[i]->row(),0)->text());

////            if(!parametersToRemove.contains(tempName))
////            {
////                parametersToRemove.append(tempName);
////                //gpMainWindow->mpProjectTabs->getCurrentTab()->hasChanged();
////            }
//            //removeCellWidget(pSelectedItems[i]->row(), pSelectedItems[i]->column());
//            //delete pSelectedItems[i];
//        }

////        for(int j=0; j<parametersToRemove.size(); ++j)
////        {
////            removeParameter(parametersToRemove.at(j));
////        }
//    }

//    // Clear and refresh the table
//    refreshTable();
//}

//void SystemParameterListWidget::removeParameter(const QString name)
//{
//    qDebug() << "Removing: " << name;
//    gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->removeSystemParameter(name);
//}


//! Slot that opens "Add Parameter" dialog, where the user can add new System parameters
void SystemParametersWidget::openAddParameterDialog()
{

    QLabel *pNameLabel;
    QLabel *pValueLabel;
    QLabel *pTypeLabel;

    QPushButton *pAddInDialogButton;
    QPushButton *pCancelInDialogButton;
    QPushButton *pAddAndCloseInDialogButton;

    mpAddParameterDialog = new QDialog(this);
    mpAddParameterDialog->setWindowTitle("Set System Parameter");

    pNameLabel = new QLabel("Name: ", this);
    mpNameBox = new QLineEdit(this);
    pValueLabel = new QLabel("Value: ", this);
    mpValueBox = new QLineEdit(this);
    pTypeLabel = new QLabel("Type: ", this);

    mpTypeBox = new ParameterTypeComboBox();

    pCancelInDialogButton = new QPushButton("Cancel", this);
    pAddInDialogButton = new QPushButton(trUtf8("Add && Continue"), this);
    pAddAndCloseInDialogButton = new QPushButton("Add && Close", this);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pCancelInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pAddInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pAddAndCloseInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(this);
    pDialogLayout->addWidget(pNameLabel,0,0);
    pDialogLayout->addWidget(mpNameBox,0,1);
    pDialogLayout->addWidget(pValueLabel,1,0);
    pDialogLayout->addWidget(mpValueBox,1,1);
    pDialogLayout->addWidget(pTypeLabel,2,0);
    pDialogLayout->addWidget(mpTypeBox,2,1);
    pDialogLayout->addWidget(pButtonBox,3,0,1,2);
    mpAddParameterDialog->setLayout(pDialogLayout);
    mpAddParameterDialog->show();

    connect(pCancelInDialogButton,      SIGNAL(clicked()), mpAddParameterDialog, SLOT(close()));
    connect(pAddAndCloseInDialogButton, SIGNAL(clicked()), this,                SLOT(addParameterAndCloseDialog()));
    connect(pAddInDialogButton,         SIGNAL(clicked()), this,                SLOT(addParameter()));
}


//! @brief Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
bool SystemParametersWidget::addParameter()
{
    SysParamListModel* pModel = qobject_cast<SysParamListModel*>(mpSysParamListView->model());
    if (pModel->hasParameter(mpNameBox->text()))
    {
        QMessageBox::critical(0, "Hopsan GUI",
                              QString("'%1' already exists, will not add!")
                              .arg(mpNameBox->text()));
    }
    else
    {
        CoreParameterData data(mpNameBox->text(), mpValueBox->text(), mpTypeBox->currentText());
        if (pModel->addOrSetParameter(data))
        {
            update();
            return true;
        }
    }
    return false;
}


void SystemParametersWidget::addParameterAndCloseDialog()
{
    if(addParameter())
    {
        mpAddParameterDialog->close();
        delete(mpAddParameterDialog);
    }
}

void SystemParametersWidget::removeSelected()
{
    QModelIndexList idxList = mpSysParamListView->selectionModel()->selectedRows();
    for (int i=0; i<idxList.count(); ++i)
    {
        mpSysParamListView->model()->removeRows(idxList[i].row(), 1);
    }
}


////! @brief Updates the parameter table from the contents list
//void SystemParameterListWidget::refreshTable()
//{
////    // First clear all data
////    clear();


////    // Now fetch parameter data
////    QVector<CoreParameterData> paramDataVector;
////    if(gpMainWindow->mpProjectTabs->count()>0)
////    {
////        gpMainWindow->mpProjectTabs->getCurrentContainer()->getParameters(paramDataVector);
////    }

////    //disconnect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(changeParameter(QTableWidgetItem*)));

////    if(paramDataVector.isEmpty())
////    {
////        //setColumnCount(1);
////        //setRowCount(1);
////        //verticalHeader()->hide();

////        QLabel *item = new QLabel("No System parameters set.");
////        item->setBackgroundColor(QColor("white"));
////        item->setTextAlignment(Qt::AlignCenter);
////        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
////        this->addWidget(item);
////    }
////    else
////    {
////        //setRowCount(0);
////        //setColumnCount(1);
////        //verticalHeader()->show();

////        for(int i=0; i<paramDataVector.size(); ++i)
////        {
//////            insertRow(rowCount());
//////            const int rowIdx = rowCount()-1;

////            SystemParameterTableItem* pNewItem = new SystemParameterTableItem(paramDataVector[i].name, paramDataVector[i].value, paramDataVector[i].type, this);
////            //setCellWidget(rowIdx,0, pNewItem);
////            this->addWidget(pNewItem);

//////            QTableWidgetItem *nameItem = new QTableWidgetItem(paramDataVector[i].name);
//////            QTableWidgetItem *valueItem = new QTableWidgetItem(paramDataVector[i].value);
//////            nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
//////            valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
//////            setItem(rowIdx, 0, nameItem);
//////            setItem(rowIdx, 1, valueItem);
//////            mParameterNames.append(paramDataVector[i].name); //Store the parameter names, to facilitate rename function

//////            TypeComboBox *typeBox = new TypeComboBox(rowIdx, 2, this);
//////            disconnect(typeBox, SIGNAL(currentIndexChanged(QString)), typeBox, SLOT(typeHasChanged(QString)));

//////            // Select wich parameter type to display
//////            for(int j=0; j<typeBox->count(); ++j)
//////            {
//////                if(paramDataVector[i].type == typeBox->itemText(j))
//////                {
//////                    typeBox->setCurrentIndex(j);
//////                    break;
//////                }
//////            }

//////            // Add widget to cell
//////            setCellWidget(rowIdx, 2, typeBox);
//////            connect(typeBox, SIGNAL(currentIndexChanged(QString)), typeBox, SLOT(typeHasChanged(QString)), Qt::UniqueConnection);
////        }
////        //connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(changeParameter(QTableWidgetItem*)), Qt::UniqueConnection);

////        setColumnWidth(0, 100);
////        setColumnWidth(1, 100);
////        setColumnWidth(2, 80);
////    }
//}
