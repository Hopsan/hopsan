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


    //Forward Declarations
class WorkspaceObject;
class GraphicsView;
class Connector;
class MainWindow;
class SystemContainer;
class ModelObject;
class SystemParameterTableWidget;


class TypeComboBox : public QComboBox
{
    Q_OBJECT

public:
    TypeComboBox(size_t row, size_t column, SystemParameterTableWidget *parent);

public slots:
    void typeHasChanged(QString newType);

protected:
    int mRow, mColumn;
    SystemParameterTableWidget *mParent;
};


class SystemParameterTableWidget : public QTableWidget
{
    friend class SystemParametersWidget;
    Q_OBJECT
public:
    SystemParameterTableWidget(int rows, int columns, QWidget *parent=0);

    QString getParameter(QString name);
    bool hasParameter(QString name);

public slots:
    void setParameter(QString name, QString valueTxt, QString descriptionTxt="", QString unitTxt="", QString typeTxt="", bool doUpdate=true);
//    void setParameter(QString name, double value, bool doUpdate=true);
    void setParameters();
    void changeParameter(QTableWidgetItem *item=0);

private slots:
    void openAddParameterDialog();
    void addParameter();
    void addParameterAndCloseDialog();
    void removeSelectedParameters();
    void update();

signals:
    void modifiedSystemParameter();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
 //   QComboBox *createTypeComboBox();

    QDialog *mpAddParameterDialog;
    QLabel *mpNameLabel;
    QLineEdit *mpNameBox;
    QLabel *mpValueLabel;
    QLineEdit *mpValueBox;
    QLabel *mpTypeLabel;
    QComboBox *mpTypeBox;
    QPushButton *mpAddInDialogButton;
    QPushButton *mpCancelInDialogButton;
    QPushButton *mpAddAndCloseInDialogButton;
};


class SystemParametersWidget : public QWidget
{
    Q_OBJECT

public:
    SystemParametersWidget(MainWindow *parent = 0);

public slots:
    void update();

private:
    //QList< QPair<QString, double> > mContents;

    SystemParameterTableWidget *mpSystemParametersTable;
    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
    QPushButton *mpCloseButton;
    QGridLayout *mpGridLayout;
};

#endif // SYSTEMPARAMETERSWIDGET_H
