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

#include "SystemParametersWidget.h"
#include "common.h"
#include "GUIObjects/GUIContainerObject.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "Utilities/GUIUtilities.h"


QWidget *ParamTypeComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    ParameterTypeComboBox *editor = new ParameterTypeComboBox(parent);
    return editor;
}

void ParamTypeComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    ParameterTypeComboBox *comboBox = static_cast<ParameterTypeComboBox*>(editor);
    comboBox->setCurrentIndex(comboBox->findText(value));
}

void ParamTypeComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    ParameterTypeComboBox *comboBox = static_cast<ParameterTypeComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void ParamTypeComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

SysParamTableModel::SysParamTableModel(ContainerObject *pContainerObject, QObject *pParent)
    : QAbstractTableModel(pParent)
{
    mpContainerObject = pContainerObject;
    mpContainerObject->getParameters(mParameterData);
}

int SysParamTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mParameterData.size();
}

int SysParamTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant SysParamTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= mParameterData.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case 0:
            return mParameterData.at(index.row()).mName;
            break;
        case 1:
            return mParameterData.at(index.row()).mValue;
            break;
        case 2:
            return mParameterData.at(index.row()).mType;
            break;
        }
    }

    return QVariant();
}

Qt::ItemFlags SysParamTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool SysParamTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QString oldValue;
    bool isOk=false;
    if (index.isValid() && role == Qt::EditRole)
    {
        switch(index.column())
        {
        case 0:
            isOk = renameParameter(index.row(), value.toString());
            break;
        case 1:
            //Need to remember old value in case set fails so that we can reset
            //! @todo or maybe we should update from core instead
            //! @todo maybe resetting mParametData should be handled in addORSetPArameter, but right now we have already overwritten the value (maybe should use memberTemp variable)
            oldValue = mParameterData[index.row()].mValue;
            mParameterData[index.row()].mValue = value.toString();
            isOk = this->addOrSetParameter(index.row());
            if (!isOk)
            {
                mParameterData[index.row()].mValue = oldValue;
            }
            break;
        case 2:
            oldValue = mParameterData[index.row()].mType;
            mParameterData[index.row()].mType = value.toString();
            isOk = this->addOrSetParameter(index.row());
            if (!isOk)
            {
                mParameterData[index.row()].mType = oldValue;
            }
            break;
        }

        emit dataChanged(index, index);
    }
    return isOk;
}

QVariant SysParamTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return QString("Name");
        case 1:
            return QString("Value");
        case 2:
            return QString("Type");
        }
    }
    else
    {
        return QString("%1").arg(section);
    }

    return QVariant();
}

bool SysParamTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), row, row+count-1);

    for (int i=row; i<row+count; ++i)
    {
        removeParameter(row);
    }

    endRemoveRows();
    return true;
}

bool SysParamTableModel::addOrSetParameter(CoreParameterData &rParameterData)
{
    bool ok;
    QString errorString;
    QStringList stringList;

    ok = verifyParameterValue(rParameterData.mValue, rParameterData.mType, stringList, errorString);
    if (!ok)
    {
        QMessageBox::critical(0, "Error", errorString);
    }
    else
    {
        CoreParameterData oldParamData;
        mpContainerObject->getParameter(rParameterData.mName, oldParamData);

        ok = mpContainerObject->getCoreSystemAccessPtr()->setSystemParameter(rParameterData.mName,
                                                                             rParameterData.mValue,
                                                                             rParameterData.mDescription,
                                                                             rParameterData.mUnit,
                                                                             rParameterData.mType);
        //Error check
        if(!ok)
        {
            QMessageBox::critical(0, "Hopsan GUI",
                                  QString("'%1' is an invalid name for a system parameter or '%2' is an invalid value.")
                                  .arg(rParameterData.mName, rParameterData.mValue));
            return false;
        }

        //! @todo check if other stuff then value has changed, at least type
        //! @todo dont go through main window to tag a tab as changed, should go through container
        //! @todo need to set has changed in more places after set rename add and remove maybe
        if(oldParamData.mValue != mpContainerObject->getParameterValue(rParameterData.mName))
        {
            gpMainWindow->mpProjectTabs->getCurrentTab()->hasChanged();
        }
    }



    //! @todo if Ok the we should update or emit data changed or something
    return ok;

}

bool SysParamTableModel::hasParameter(const QString name)
{
    return mpContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(name);
}

void SysParamTableModel::removeParameter(const int row)
{
    mpContainerObject->getCoreSystemAccessPtr()->removeSystemParameter(mParameterData[row].mName);
    mParameterData.remove(row);
}

bool SysParamTableModel::renameParameter(const int idx, const QString newName)
{
    CoreParameterData oldData = mParameterData[idx];
    removeParameter(idx);
    oldData.mName = newName;
    return addOrSetParameter(oldData);
}

bool SysParamTableModel::addOrSetParameter(const int idx)
{
    return addOrSetParameter(mParameterData[idx]);
}


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
        SysParamTableModel *pModel = new SysParamTableModel(mpContainerObject, this);
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
    SysParamTableModel* pModel = qobject_cast<SysParamTableModel*>(mpSysParamListView->model());
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
