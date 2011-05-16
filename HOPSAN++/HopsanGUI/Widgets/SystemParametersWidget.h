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


    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;
class MainWindow;
class GUISystem;
class GUIModelObject;


class SystemParameterTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    SystemParameterTableWidget(int rows, int columns, QWidget *parent=0);

    double getParameter(QString name);
    bool hasParameter(QString name);

public slots:
    void setParameter(QString name, QString valueTxt, bool doUpdate=true);
    void setParameter(QString name, double value, bool doUpdate=true);
    void setParameters();
    void changeParameter(QTableWidgetItem *item);

//private slots:
    void openComponentPropertiesDialog();
    void addParameter();
    void removeSelectedParameters();
    void update();

signals:
    void modifiedSystemParameter();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QLabel *mpNameLabel;
    QLineEdit *mpNameBox;
    QLabel *mpValueLabel;
    QLineEdit *mpValueBox;
    QPushButton *mpAddInDialogButton;
    QPushButton *mpDoneInDialogButton;
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
