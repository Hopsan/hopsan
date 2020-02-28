/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   SystemParametersWidget.cpp
//! @brief Contains a System parameter widget class
//!
//$Id$

#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

#include "SystemParametersWidget.h"
#include "common.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "FindWidget.h"
#include "Configuration.h"

namespace {

QStringList getAllAccesibleParentAndGrandparentSystemParameterNames(ContainerObject* pThisSystem) {
    QStringList output;
    if (!pThisSystem->isTopLevelContainer()) {
        output = getAllAccessibleSystemParameterNames(pThisSystem->getParentContainerObject());
    }
    return output;
}

}


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
    this->setContainer(pContainerObject);
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
    CoreParameterData data;
    bool isOk=false;
    if (index.isValid() && role == Qt::EditRole)
    {
        switch(index.column())
        {
        case 0:
            isOk = true;
            // If new name given try to rename
            if (value.toString() != mParameterData[index.row()].mName)
            {
                if (mpContainerObject)
                {
                    isOk = mpContainerObject->renameParameter(mParameterData[index.row()].mName, value.toString());
                    if (isOk)
                    {
                       mParameterData[index.row()].mName = value.toString();
                       emit dataChanged(index, index);
                    }
                    else
                    {
                        QMessageBox::critical(0, "Error",
                                              QString("'%1' is an invalid parameter name or name already exist")
                                              .arg(value.toString()));
                    }
                }
                else
                {
                    isOk = false;
                    QMessageBox::critical(0, "Error", "The container object has been removed, (this should not happen!)");
                }
            }
            break;
        case 1:
            data = mParameterData[index.row()];
            data.mValue = value.toString();
            isOk = addOrSetParameter(data);
            if (isOk)
            {
                mParameterData[index.row()] = data;
                emit dataChanged(index, index);
            }
            break;
        case 2:
            data = mParameterData[index.row()];
            data.mType = value.toString();
            isOk = addOrSetParameter(data);
            if (isOk)
            {
                mParameterData[index.row()] = data;
                emit dataChanged(index, index);
            }
            break;
        }
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
    Q_UNUSED(parent);
    // Prevent removal if model is locked
    if ( mpContainerObject && (mpContainerObject->getModelLockLevel()==NotLocked) && !mpContainerObject->isLocallyLocked() )
    {
        beginRemoveRows(QModelIndex(), row, row+count-1);

        for (int i=row; i<row+count; ++i)
        {
            removeParameter(row);
        }

        endRemoveRows();
        return true;
    }
    return false;
}

bool SysParamTableModel::insertRows(int row, int count, const QModelIndex &/*parent*/)
{
    beginInsertRows(QModelIndex(), row, row+count-1);
    // No need to do anything else for now
    endInsertRows();
    return true;
}

bool SysParamTableModel::addOrSetParameter(CoreParameterData &rParameterData)
{
    // Prevent setting or adding parameter if system is fully locked
    if (mpContainerObject && ((mpContainerObject->getModelLockLevel()>LimitedLock) || mpContainerObject->isLocallyLocked()) )
    {
        return false;
    }

    bool isOk;
    QString errorString;
    QStringList thisSystemParameterNames = mpContainerObject->getParameterNames();
    QStringList allAccessibleParentSystemParameterNames = getAllAccesibleParentAndGrandparentSystemParameterNames(mpContainerObject);

    isOk = verifyParameterValue(rParameterData.mValue, rParameterData.mType, thisSystemParameterNames, allAccessibleParentSystemParameterNames, errorString);
    if (!isOk)
    {
        QMessageBox::critical(0, "Error", errorString);
    }
    else if (mpContainerObject)
    {
        CoreParameterData oldParamData;
        mpContainerObject->getParameter(rParameterData.mName, oldParamData);

        isOk = mpContainerObject->setOrAddParameter(rParameterData);
        //Error check
        if(!isOk)
        {
            QMessageBox::critical(0, "Hopsan GUI",
                                  QString("The name '%1' is invalid or already used by something else, or the value '%2' is invalid.")
                                  .arg(rParameterData.mName, rParameterData.mValue));
            return false;
        }

        // We don't need to add new row or update because setOrAddParameter should have signaled for update already

        //! @todo check if other stuff than value or type has changed
        CoreParameterData newParameter;
        mpContainerObject->getParameter(rParameterData.mName, newParameter);
        if( oldParamData.mValue != newParameter.mValue ||  oldParamData.mType != newParameter.mType )
        {
            mpContainerObject->hasChanged();
        }
    }
    else
    {
        isOk = false;
    }

    //! @todo if OK then we should update or emit data changed or something
    return isOk;
}

void SysParamTableModel::getFullParameterData(const QModelIndex &index, CoreParameterData &rParameterData)
{
    if (index.isValid() && index.row() < mParameterData.size())
    {
       rParameterData = mParameterData[index.row()];
    }
}

bool SysParamTableModel::hasParameter(const QString name)
{
    if (mpContainerObject)
    {
        return mpContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(name);
    }
    else
    {
        return false;
    }
}

void SysParamTableModel::sortByColumn(int c)
{
    this->sort(c);
}

void SysParamTableModel::setContainer(ContainerObject *pContainerObject)
{
    mpContainerObject = pContainerObject;
    if (mpContainerObject)
    {
        emit layoutAboutToBeChanged();
        mpContainerObject->getParameters(mParameterData);
        emit layoutChanged();
    }
    else
    {
        emit layoutAboutToBeChanged();
        mParameterData.clear();
        emit layoutChanged();
    }
}


void SysParamTableModel::removeParameter(const int row)
{
    // Prevent removal if model or system is locked
    if ( mpContainerObject && (mpContainerObject->getModelLockLevel()==NotLocked) && !mpContainerObject->isLocallyLocked() )
    {
        mpContainerObject->getCoreSystemAccessPtr()->removeSystemParameter(mParameterData[row].mName);
        mParameterData.remove(row);
        mpContainerObject->hasChanged();
    }
}


//! @brief Constructor for System Parameters widget, where the user can see and change the System parameters in the model.
//! @param parent Pointer to the main window
SystemParametersWidget::SystemParametersWidget(QWidget *pParent)
    : QWidget(pParent)
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

    mpEditButton = new QPushButton(tr("&Edit"));
    mpEditButton->setFixedHeight(30);
    mpEditButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpEditButton->setAutoDefault(false);
    mpEditButton->setEnabled(false);
    mpEditButton->setFont(buttonFont);

    mpRemoveButton = new QPushButton(tr("&Unset"));
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setEnabled(false);
    mpRemoveButton->setFont(buttonFont);

    mpSysParamTableView = new QTableView();
    mpSysParamTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpSysParamTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(mpSysParamTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(highlightComponents(QModelIndex)));

    ParamTypeComboBoxDelegate *pComboBoxDelegate = new ParamTypeComboBoxDelegate();
    mpSysParamTableView->setItemDelegateForColumn(2, pComboBoxDelegate);

    QGridLayout *pGridLayout = new QGridLayout(this);
    pGridLayout->addWidget(mpSysParamTableView, 0, 0);
    pGridLayout->addWidget(mpAddButton, 1, 0);
    pGridLayout->addWidget(mpEditButton, 2, 0);
    pGridLayout->addWidget(mpRemoveButton, 3, 0);

    pGridLayout->setContentsMargins(4,4,4,4);

    mpModel = new SysParamTableModel(mpContainerObject, this);

    mpProxyModel = new QSortFilterProxyModel(this);
    mpProxyModel->setSourceModel(mpModel);
    mpSysParamTableView->setModel(mpProxyModel);
    mpSysParamTableView->setSortingEnabled(true);

    update();

    connect(mpAddButton, SIGNAL(clicked()), this, SLOT(openAddParameterDialog()));
    connect(mpEditButton, SIGNAL(clicked()), this, SLOT(openEditParameterDialog()));
    connect(mpRemoveButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
}

QPointer<ContainerObject> SystemParametersWidget::getRepresentedContainerObject()
{
    return mpContainerObject;
}

void SystemParametersWidget::update(ContainerObject *pNewContainer)
{
    if (mpContainerObject != pNewContainer)
    {
        // Disconnect all signals from old container
        if (mpContainerObject)
        {
            disconnect(mpContainerObject, 0, this, 0);
        }

        // Assign new and Connect new signals if new container is not null ptr
        mpContainerObject = pNewContainer;

        if (mpContainerObject)
        {
            connect(mpContainerObject, SIGNAL(systemParametersChanged()), this, SLOT(update()), Qt::UniqueConnection);
        }
    }
    this->update();
}

void SystemParametersWidget::update()
{
    // Set container will refresh the internal parameter data and emit layoutChanged to the view
    mpModel->setContainer(mpContainerObject);

    if (mpContainerObject)
    {
        mpAddButton->setEnabled(true);
        mpEditButton->setEnabled(true);
        mpRemoveButton->setEnabled(true);

        // Make sure we show the widget, if it is hidden
        mpSysParamTableView->show();

//        qDebug() << "--------------List isEnabled: " << mpSysParamTableView->isEnabled();
//        qDebug() << "--------------List isHidden: " << mpSysParamTableView->isHidden();
//        qDebug() << "--------------List isVisible: " << mpSysParamTableView->isVisible();
//        qDebug() << "--------------SysParWidget isVisible: " << this->isVisible();
//        qDebug() << "--------------SysParWidget Parent isVisible: " << gpMainWindowWidget->isVisible();
    }
    else
    {
        mpAddButton->setEnabled(false);
        mpEditButton->setEnabled(false);
        mpRemoveButton->setEnabled(false);
    }
}

//! @brief Slot that opens "Add Parameter" dialog, where the user can add new System parameters
void SystemParametersWidget::openAddParameterDialog()
{
    // Abort if a dialog is already open
    if (mpAddParameterDialog)
        return;

    mpAddParameterDialog = new QDialog(this);
    mpAddParameterDialog->setWindowTitle("Set System Parameter");

    mpNewParamNameEdit = new QLineEdit(mpAddParameterDialog);
    mpNewParamValueEdit = new QLineEdit(mpAddParameterDialog);
    mpNewParamDescriptionEdit = new QLineEdit(mpAddParameterDialog);
    mpNewParamUnitQuantityEdit = new QLineEdit(mpAddParameterDialog);
    mpNewParamTypeBox = new ParameterTypeComboBox();

    QDialogButtonBox *pButtonBox            = new QDialogButtonBox(Qt::Horizontal);
    QPushButton *pCancelInDialogButton      = new QPushButton("Cancel", mpAddParameterDialog);
    QPushButton *pAddInDialogButton         = new QPushButton(trUtf8("Add && Continue"), mpAddParameterDialog);
    QPushButton *pAddAndCloseInDialogButton = new QPushButton("Add && Close", mpAddParameterDialog);
    pButtonBox->addButton(pCancelInDialogButton,      QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pAddInDialogButton,         QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pAddAndCloseInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(mpAddParameterDialog);
    pDialogLayout->addWidget(new QLabel("Name: ", mpAddParameterDialog),0,0);
    pDialogLayout->addWidget(mpNewParamNameEdit,0,1);
    pDialogLayout->addWidget(new QLabel("Value: ", mpAddParameterDialog),1,0);
    pDialogLayout->addWidget(mpNewParamValueEdit,1,1);
    pDialogLayout->addWidget(new QLabel("Description: ", mpAddParameterDialog),2,0);
    pDialogLayout->addWidget(mpNewParamDescriptionEdit,2,1);
    pDialogLayout->addWidget(new QLabel("Quantity or Unit: ", mpAddParameterDialog),3,0);
    pDialogLayout->addWidget(mpNewParamUnitQuantityEdit,3,1);
    pDialogLayout->addWidget(new QLabel("Type: ", mpAddParameterDialog),4,0);
    pDialogLayout->addWidget(mpNewParamTypeBox,4,1);
    pDialogLayout->addWidget(pButtonBox,5,0,1,2);
    mpAddParameterDialog->setLayout(pDialogLayout);

    pAddInDialogButton->setDefault(true);
    mpAddParameterDialog->show();

    connect(mpAddParameterDialog,       SIGNAL(rejected()),     this,   SLOT(closeDialog()));
    connect(pCancelInDialogButton,      SIGNAL(clicked()),      this,   SLOT(closeDialog()));
    connect(pAddAndCloseInDialogButton, SIGNAL(clicked()),      this,   SLOT(addParameterAndCloseDialog()));
    connect(pAddInDialogButton,         SIGNAL(clicked()),      this,   SLOT(addParameter()));
}

//! @brief Slot that opens "Edit Parameter" dialog, where the user can edit existing system parameters
void SystemParametersWidget::openEditParameterDialog()
{
    // Abort if a dialog is already open
    if (mpAddParameterDialog)
        return;

    QModelIndexList idxList = mpSysParamTableView->selectionModel()->selectedRows();
    if (!idxList.isEmpty())
    {
        CoreParameterData data;
        QModelIndex sourceIndex = mpProxyModel->mapToSource(idxList.front());
        mpModel->getFullParameterData(sourceIndex, data);

        mpAddParameterDialog = new QDialog(this);
        mpAddParameterDialog->setWindowTitle("Edit System Parameter");

        mPreviousName = data.mName;
        mpNewParamNameEdit = new QLineEdit(data.mName, mpAddParameterDialog);
        mpNewParamValueEdit = new QLineEdit(data.mValue, mpAddParameterDialog);
        mpNewParamDescriptionEdit = new QLineEdit(data.mDescription, mpAddParameterDialog);
        QString qu = data.mQuantity;
        if (qu.isEmpty())
        {
            qu = data.mUnit;
        }
        mpNewParamUnitQuantityEdit = new QLineEdit(qu, mpAddParameterDialog);
        mpNewParamTypeBox = new ParameterTypeComboBox();
        int typeidx = mpNewParamTypeBox->findText(data.mType);
        if (typeidx >= 0)
        {
            mpNewParamTypeBox->setCurrentIndex(typeidx);
        }


        QDialogButtonBox *pButtonBox            = new QDialogButtonBox(Qt::Horizontal);
        QPushButton *pCancelInDialogButton      = new QPushButton(tr("Cancel"), mpAddParameterDialog);
        QPushButton *pOkInDialogButton          = new QPushButton(tr("Ok"),     mpAddParameterDialog);
        pButtonBox->addButton(pCancelInDialogButton,     QDialogButtonBox::ActionRole);
        pButtonBox->addButton(pOkInDialogButton,         QDialogButtonBox::ActionRole);

        QGridLayout *pDialogLayout = new QGridLayout(mpAddParameterDialog);
        pDialogLayout->addWidget(new QLabel("Name: ", mpAddParameterDialog),0,0);
        pDialogLayout->addWidget(mpNewParamNameEdit,0,1);
        pDialogLayout->addWidget(new QLabel("Value: ", mpAddParameterDialog),1,0);
        pDialogLayout->addWidget(mpNewParamValueEdit,1,1);
        pDialogLayout->addWidget(new QLabel("Description: ", mpAddParameterDialog),2,0);
        pDialogLayout->addWidget(mpNewParamDescriptionEdit,2,1);
        pDialogLayout->addWidget(new QLabel("Quantity or Unit: ", mpAddParameterDialog),3,0);
        pDialogLayout->addWidget(mpNewParamUnitQuantityEdit,3,1);
        pDialogLayout->addWidget(new QLabel("Type: ", mpAddParameterDialog),4,0);
        pDialogLayout->addWidget(mpNewParamTypeBox,4,1);
        pDialogLayout->addWidget(pButtonBox,5,0,1,2);
        mpAddParameterDialog->setLayout(pDialogLayout);

        pOkInDialogButton->setDefault(true);
        mpAddParameterDialog->show();

        connect(pCancelInDialogButton,      SIGNAL(clicked()), this,   SLOT(closeDialog()));
        connect(pOkInDialogButton,          SIGNAL(clicked()), this,   SLOT(editParameterAndCloseDialog()));
    }
}

void SystemParametersWidget::highlightComponents(QModelIndex index)
{
    if(index.column() != 0)
    {
        return;
    }
    QModelIndexList idxList = mpSysParamTableView->selectionModel()->selectedRows();
    QStringList names;
    for (auto &idx : idxList )
    {
        names.append(idx.model()->data(idx, Qt::EditRole).toString());
    }
    if (!names.isEmpty())
    {
        gpFindWidget->findSystemParameter(names, false);
    }
}


//! @brief Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
bool SystemParametersWidget::addParameter()
{
    return addOrEditParameter(false);
}

void SystemParametersWidget::editParameterAndCloseDialog()
{
    if (addOrEditParameter(true))
    {
        closeDialog();
    }
}


void SystemParametersWidget::addParameterAndCloseDialog()
{
    if(addParameter())
    {
        closeDialog();
    }
}

void SystemParametersWidget::removeSelected()
{
    QModelIndexList idxList = mpSysParamTableView->selectionModel()->selectedRows();
    while (idxList.size() > 0)
    {
        bool rc = mpSysParamTableView->model()->removeRows(idxList[0].row(), 1);
        if (!rc)
        {
            break;
        }
        idxList = mpSysParamTableView->selectionModel()->selectedRows();
    }
}

void SystemParametersWidget::closeDialog()
{
    disconnect(mpAddParameterDialog, SIGNAL(rejected()), this,SLOT(closeDialog()));
    mpAddParameterDialog->close();
    mpAddParameterDialog->deleteLater();
    mpAddParameterDialog = nullptr;
}

bool SystemParametersWidget::addOrEditParameter(bool editing)
{
    if (editing && (mPreviousName != mpNewParamNameEdit->text()) )
    {
        // Try to add new parameter settings
        bool rc = addOrEditParameter(false);
        // If added OK, then delete old parameter
        if (rc)
        {
            removeSelected();
            //! @todo need to find row
            //! @todo remove old
        }
        return rc;
    }
    else if (!editing && mpModel->hasParameter(mpNewParamNameEdit->text()))
    {
        //! @todo maybe we should warn about overwriting instead
        QMessageBox::critical(0, "Hopsan GUI",
                              QString("'%1' already exists, will not add!")
                              .arg(mpNewParamNameEdit->text()));
        return false;
    }
    else
    {
        // The unit field should be "" here, QuantityOrUnit in core will deal with deciding if quantity or unit should be used
        CoreParameterData data(mpNewParamNameEdit->text(), mpNewParamValueEdit->text(), mpNewParamTypeBox->currentText(), mpNewParamUnitQuantityEdit->text(), "", mpNewParamDescriptionEdit->text());
        return mpModel->addOrSetParameter(data);
    }
}
