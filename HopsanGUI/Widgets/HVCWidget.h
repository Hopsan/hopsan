/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HVCWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012
//!
//! @brief Contains class for Hopsan validation widget
//!
//$Id$

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
    int mTimeColumn;
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
