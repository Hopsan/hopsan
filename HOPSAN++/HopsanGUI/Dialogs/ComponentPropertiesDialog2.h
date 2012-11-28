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
//! @file   ComponentPropertiesDialog.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#ifndef COMPONENTPROPERTIESDIALOG2_H
#define COMPONENTPROPERTIESDIALOG2_H

#include <QtGui>
#include "Dialogs/ModelObjectPropertiesDialog.h"
#include "CoreAccess.h"

class Component;
class ParameterSettingsLayout;
class MainWindow;


class ParametersTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnIdT {Name,Unit,Type,Alias,Value,ResetValue,SetSysPar,NumColumns};
    ParametersTableModel(Component* pComponent, QObject *pParent=0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    //bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

    //void setContainer(ContainerObject *pContainerObject);
    //bool addOrSetParameter(CoreParameterData &rParameterData);
    //bool hasParameter(const QString name);

protected:
    //void removeParameter(const int row);
    bool cleanAndVerifyParameterValue(QString &rValue, const QString paramType);

    QVector<QString> mHeaders;
    Component *mpComponent;
    QVector<CoreParameterData> mParameters;
    //QVector<CoreParameterData> mParameterData;
};

class PortVariablesTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ColumnIdT {Name,Unit,Type,Alias,StartValue,ResetValue,SetSysPar,Scale,NumColumns};
    PortVariablesTableModel(Component* pComponent, QObject *pParent=0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    //bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

    //void setContainer(ContainerObject *pContainerObject);
    //bool addOrSetParameter(CoreParameterData &rParameterData);
    //bool hasParameter(const QString name);

protected:
    //void removeParameter(const int row);
    bool cleanAndVerifyParameterValue(QString &rValue, const QString paramType);

    QVector<QString> mHeaders;
    Component *mpComponent;
    QVector<CoreParameterData> mParameters;
    //QVector<CoreParameterData> mParameterData;
};


class SysParSelectButtonDelegate : public QStyledItemDelegate
{
private:
    ModelObject *mpModelObject;

public:
    SysParSelectButtonDelegate(ModelObject *pModelObject, QObject* pParent);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
//    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
//    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

class ResetValueButtonDelegate : public QStyledItemDelegate
{
private:

public:
    ResetValueButtonDelegate(QObject* pParent) : QStyledItemDelegate(pParent)
    {

    }
};


class ComponentPropertiesDialog2 : public ModelObjectPropertiesDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog2(Component *pComponent, MainWindow *pParent=0);

protected slots:
    void okPressed();
    void editPortPos();

protected:
    void setParametersAndStartValues();
    void setName();

private:
    Component *mpComponent;
    ParametersTableModel *mpParameterTableModel;
    PortVariablesTableModel *mpPortVariablesTableModel;
    QTableView *mpParameterTableView;
    QTableView *mpPortVariableTableView;

    void createEditStuff();
    void createHelpStuff();
    void createNameAndTypeStuff();
    bool interpretedAsStartValue(QString &parameterDescription);

    QGridLayout *mpMainLayout;


    QLineEdit *mpNameEdit;

//    QVector<ParameterSettingsLayout*> mvParameterLayout;
//    QVector<ParameterSettingsLayout*> mvStartValueLayout;


    QWidget *mpExtension;
};

#endif // COMPONENTPROPERTIESDIALOG_H
