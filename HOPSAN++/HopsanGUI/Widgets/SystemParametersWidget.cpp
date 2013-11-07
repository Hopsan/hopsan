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

#include "SystemParametersWidget.h"
#include "common.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
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

bool SysParamTableModel::removeRows(int row, int count, const QModelIndex &/*parent*/)
{
    beginRemoveRows(QModelIndex(), row, row+count-1);

    for (int i=row; i<row+count; ++i)
    {
        removeParameter(row);
    }

    endRemoveRows();
    return true;
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
    bool isOk;
    QString errorString;
    QStringList stringList;

    isOk = verifyParameterValue(rParameterData.mValue, rParameterData.mType, stringList, errorString);
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
                                  QString("'%1' is an invalid name for a system parameter or '%2' is an invalid value.")
                                  .arg(rParameterData.mName, rParameterData.mValue));
            return false;
        }

        // We dont need to add new row or update because setOrAddParameter should have signald for update already

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

    //! @todo if Ok then we should update or emit data changed or something
    return isOk;
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
    if (mpContainerObject)
    {
        mpContainerObject->getCoreSystemAccessPtr()->removeSystemParameter(mParameterData[row].mName);
        mParameterData.remove(row);
        mpContainerObject->hasChanged();
    }
}


//! @brief Construtor for System Parameters widget, where the user can see and change the System parameters in the model.
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

    mpRemoveButton = new QPushButton(tr("&Unset"));
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setEnabled(false);
    mpRemoveButton->setFont(buttonFont);

    mpSysParamTableView = new QTableView();
    mpSysParamTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mpSysParamTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ParamTypeComboBoxDelegate *pComboBoxDelegate = new ParamTypeComboBoxDelegate();
    mpSysParamTableView->setItemDelegateForColumn(2, pComboBoxDelegate);

    QGridLayout *pGridLayout = new QGridLayout(this);
    pGridLayout->addWidget(mpSysParamTableView, 0, 0);
    pGridLayout->addWidget(mpAddButton, 1, 0);
    pGridLayout->addWidget(mpRemoveButton, 2, 0);

    pGridLayout->setContentsMargins(4,4,4,4);

    mpModel = new SysParamTableModel(mpContainerObject, this);

    mpProxyModel = new QSortFilterProxyModel(this);
    mpProxyModel->setSourceModel(mpModel);
    mpSysParamTableView->setModel(mpProxyModel);
    mpSysParamTableView->setSortingEnabled(true);

    update();

    connect(mpAddButton, SIGNAL(clicked()), this, SLOT(openAddParameterDialog()));
    connect(mpRemoveButton, SIGNAL(clicked()), this, SLOT(removeSelected()));
    //connect(mpSysParamTableView->horizontalHeader(), SIGNAL(sectionClicked(int)), mpProxyModel, SLOT(sortByColumn(int)));
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
        mpRemoveButton->setEnabled(true);

        // Make sure we show the widget, if it is hidden
        mpSysParamTableView->show();

        qDebug() << "--------------List isEnabled: " << mpSysParamTableView->isEnabled();
        qDebug() << "--------------List isHidden: " << mpSysParamTableView->isHidden();
        qDebug() << "--------------List isVisible: " << mpSysParamTableView->isVisible();
        qDebug() << "--------------SysParWidget isVisible: " << this->isVisible();
        qDebug() << "--------------SysParWidget Parent isVisible: " << gpMainWindowWidget->isVisible();
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

    pAddInDialogButton->setDefault(true);
    mpAddParameterDialog->show();

    connect(pCancelInDialogButton,      SIGNAL(clicked()), mpAddParameterDialog, SLOT(close()));
    connect(pAddAndCloseInDialogButton, SIGNAL(clicked()), this,                SLOT(addParameterAndCloseDialog()));
    connect(pAddInDialogButton,         SIGNAL(clicked()), this,                SLOT(addParameter()));
}


//! @brief Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
bool SystemParametersWidget::addParameter()
{
    if (mpModel->hasParameter(mpNameBox->text()))
    {
        //! @todo maybe we should warn about overwriting instead
        QMessageBox::critical(0, "Hopsan GUI",
                              QString("'%1' already exists, will not add!")
                              .arg(mpNameBox->text()));
    }
    else
    {
        CoreParameterData data(mpNameBox->text(), mpValueBox->text(), mpTypeBox->currentText());
        if (mpModel->addOrSetParameter(data))
        {
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
    QModelIndexList idxList = mpSysParamTableView->selectionModel()->selectedRows();
    while (idxList.size() > 0)
    {
        mpSysParamTableView->model()->removeRows(idxList[0].row(), 1);
        idxList = mpSysParamTableView->selectionModel()->selectedRows();
    }
}
