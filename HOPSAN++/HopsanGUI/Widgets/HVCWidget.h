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
//! @file   HVCWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012
//!
//! @brief Contains class for Hopsan validation widget
//!
//$Id: LibraryWidget.cpp 5499 2013-06-04 10:52:33Z robbr48 $

#ifndef HVCWIDGET_H
#define HVCWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QTreeWidget>

class FullNameVariableTreeWidget : public QTreeWidget
{
public:
    FullNameVariableTreeWidget(QWidget *pParent=0);
    void addFullNameVariable(const QString &rFullName);

protected:
//    void mousePressEvent(QMouseEvent *event);
//    void dropEvent(QDropEvent *event);
//    void dragEnterEvent(QDragEnterEvent *event);

private:
    void addFullNameVariable(const QString &rFullName, const QString &rRemaningName, QTreeWidgetItem *pParentItem);
    QString mTopLevelSystemName;

};

class HvcConfig
{
public:
    QString mFullVarName;
    QString mDataFile;
    int mDataColumn;
    double mTolerance;
};

class HVCWidget : public QDialog
{
    Q_OBJECT
public:
    explicit HVCWidget(QWidget *parent = 0);
    
signals:
    
public slots:
    void openHvcFile();
    void clearContents();
    void runHvcTest();

private:
    QString mModelFilePath;
    QList<HvcConfig> mDataConfigs;
    QLineEdit *mpHvcOpenPathEdit;
    FullNameVariableTreeWidget *mpAllVariablesTree;
    FullNameVariableTreeWidget *mpSelectedVariablesTree;

};

#endif // HVCWIDGET_H
