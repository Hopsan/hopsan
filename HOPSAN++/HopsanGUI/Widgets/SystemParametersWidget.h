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
#include "CoreAccess.h"

// Forward Declarations
class ContainerObject;

class ParameterTypeComboBox : public QComboBox
{
public:
    ParameterTypeComboBox(QWidget *pParentWidget=0) : QComboBox(pParentWidget)
    {
        addItem("double");
        addItem("integer");
        addItem("bool");
        addItem("string");
    }
};

class ParamTypeComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


class SysParamTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SysParamTableModel(ContainerObject *pContainerObject, QObject *pParent=0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

    void setContainer(ContainerObject *pContainerObject);
    bool addOrSetParameter(CoreParameterData &rParameterData);
    bool hasParameter(const QString name);

protected:
    void removeParameter(const int row);

    QPointer<ContainerObject> mpContainerObject;
    QVector<CoreParameterData> mParameterData;
};


class SystemParametersWidget : public QWidget
{
    Q_OBJECT
public:
    SystemParametersWidget(QWidget *pParent=0);

public slots:
    void update();
    void update(ContainerObject* pNewContainer);

protected slots:
    void openAddParameterDialog();
    bool addParameter();
    void addParameterAndCloseDialog();
    void removeSelected();

private:
    ContainerObject *mpContainerObject;
    QTableView *mpSysParamTableView;

    QDialog *mpAddParameterDialog;
    QLineEdit *mpNameBox;
    QLineEdit *mpValueBox;
    QComboBox *mpTypeBox;

    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
};

#endif // SYSTEMPARAMETERSWIDGET_H
