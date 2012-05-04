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
//! @file   SystemParametersWidget.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-11-20
//!
//! @brief Contains a System parameter widget class
//!
//$Id$

#ifndef SYSTEMPARAMETERSWIDGET_H
#define SYSTEMPARAMETERSWIDGET_H

#include <QtGui>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <QComboBox>

#include "CoreAccess.h"
#include "GUIObjects/GUIContainerObject.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"


    //Forward Declarations
class WorkspaceObject;
class GraphicsView;
class Connector;
class MainWindow;
class SystemContainer;
class ModelObject;
class SystemParameterListWidget;

class ComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate(QObject *parent=0) : QItemDelegate(parent){}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("double");
        editor->addItem("integer");
        editor->addItem("bool");
        editor->addItem("string");
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(comboBox->findText(value));
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }
};


class SysParamListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SysParamListModel(ContainerObject *pContainerObject, QObject *pParent=0)
        : QAbstractTableModel(pParent)
    {
        mpContainerObject = pContainerObject;
        mpContainerObject->getParameters(mParameterData);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return mParameterData.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant data(const QModelIndex &index, int role) const
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
                return mParameterData.at(index.row()).name;
                break;
            case 1:
                return mParameterData.at(index.row()).value;
                break;
            case 2:
                return mParameterData.at(index.row()).type;
                break;
            }
        }
        else
        {
            return QVariant();
        }
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            switch(index.column())
            {
            case 0:
                //mParameterData[index.row()].name = value.toString();
                break;
            case 1:
                mParameterData[index.row()].value = value.toString();
                break;
            case 2:
                mParameterData[index.row()].type = value.toString();
                break;
            }

            if (index.column() > 0)
            {
                this->addOrSetParameter(index.row());
            }

            emit dataChanged(index, index);
            return true;
        }
        return false;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
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
    }

protected:
    bool renameParameter(){}
    bool addOrSetParameter(const int idx)
    {
        bool ok;
        ok = mpContainerObject->getCoreSystemAccessPtr()->setSystemParameter(mParameterData[idx].name,
                                                                             mParameterData[idx].value,
                                                                             mParameterData[idx].description,
                                                                             mParameterData[idx].unit,
                                                                             mParameterData[idx].type);
        return ok;
    }

    ContainerObject *mpContainerObject;
    QVector<CoreParameterData> mParameterData;

};


class SystemParametersWidget : public QWidget
{
    Q_OBJECT

public:
    SystemParametersWidget(MainWindow *parent=0);

public slots:
    void update();
    void update(ContainerObject* pNewContainer);

protected slots:
    void openAddParameterDialog();
    void addParameter();
    void addParameterAndCloseDialog();
    void setParameter(QString name, QString valueTxt, QString descriptionTxt="", QString unitTxt="", QString typeTxt="");
    bool hasParameter(QString name);

private:
    ContainerObject *mpContainerObject;
    QTableView *mpSysParamListView;
    //SysParamListModel *mpParameterListModel;

    QDialog *mpAddParameterDialog;
    QLineEdit *mpNameBox;
    QLineEdit *mpValueBox;
    QComboBox *mpTypeBox;

    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
};



//class TypeComboBox : public QComboBox
//{
//    Q_OBJECT

//public:
//    TypeComboBox(const int row, const int column, SystemParameterTableWidget *parent);

//public slots:
//    void typeHasChanged(QString newType);

//protected:
//    int mRow, mColumn;
//    SystemParameterTableWidget *mParent;
//};

//class SysParamListView : public QListView
//{
//    Q_OBJECT

//public:
//    SysParamListView(QWidget *pParentWidget=0) : QListView(pParentWidget)
//    {
//        //mpParameterListModel = 0;

//    }

//    //SysParamListModel *mpParameterListModel;


//};


//class SystemParameterListWidget : public QListWidget
//{
//    friend class SystemParametersWidget;
//    Q_OBJECT
//public:
//    SystemParameterListWidget(QWidget *pParentWidget=0);

//    QString getParameterValue(QString name);
//    bool hasParameter(QString name);

//public slots:
//    void setParameter(QString name, QString valueTxt, QString descriptionTxt="", QString unitTxt="", QString typeTxt="", bool doUpdate=true);
//    //void setAllParameters();
//    void changeParameterName(const QString oldName);
//    void changeParameter();

//    void openAddParameterDialog();
//    void addParameter();
//    void addParameterAndCloseDialog();
//    void removeSelectedParameters();
//    void refreshTable();

//signals:
//    void modifiedSystemParameter();

//protected:
//    void removeParameter(const QString name);
//    void keyPressEvent(QKeyEvent *event);

//private:
//    QVector<QString> mParameterNames;
//    QDialog *mpAddParameterDialog;
//    QLabel *mpNameLabel;
//    QLineEdit *mpNameBox;
//    QLabel *mpValueLabel;
//    QLineEdit *mpValueBox;
//    QLabel *mpTypeLabel;
//    QComboBox *mpTypeBox;
//    QPushButton *mpAddInDialogButton;
//    QPushButton *mpCancelInDialogButton;
//    QPushButton *mpAddAndCloseInDialogButton;
//};

//class SystemParameterTableItem : public QListWidgetItem
//{
//    Q_OBJECT
//public:
//    SystemParameterTableItem(QString name, QString value, QString type, SystemParameterListWidget *pParentTable)
//    {
//        //connect(name, SIGNAL(editingFinished), this, SIGNAL())
//        mType.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
//        mType.addItem("double");
//        mType.addItem("integer");
//        mType.addItem("bool");
//        mType.addItem("string");

//        mName.setText(name);
//        mValue.setText(value);

//        mHBoxLayout.addWidget(&mName, 0, 0);
//        mHBoxLayout.addWidget(&mValue, 0, 0);
//        mHBoxLayout.addWidget(&mType, 0, 0);

//        // Select wich parameter type to display
//        for(int j=0; j< mType.count(); ++j)
//        {
//            if(type == mType.itemText(j))
//            {
//                mType.setCurrentIndex(j);
//                break;
//            }
//        }

//        setLayout(&mHBoxLayout);
//        //mHBoxLayout.setSpacing(0);

//        connect(&mName, SIGNAL(textChanged(QString)), pParentTable, SLOT(changeParameterName(QString)), Qt::UniqueConnection);
//        connect(&mValue, SIGNAL(editingFinished()), pParentTable, SLOT(changeParameter()), Qt::UniqueConnection);
//        connect(&mType, SIGNAL(currentIndexChanged(int)), pParentTable, SLOT(changeParameter()), Qt::UniqueConnection);
//    }


//    QHBoxLayout mHBoxLayout;
//    QLineEdit mName;
//    QLineEdit mValue;
//    QComboBox mType;
//};




#endif // SYSTEMPARAMETERSWIDGET_H
