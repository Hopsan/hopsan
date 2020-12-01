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
//! @file   QuickNavigationWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-xx
//!
//! @brief Contains the quick navigation widget that is used to go back after entering into container objects
//!
//$Id$

#ifndef QUICKNAVIGATIONWIDGET_H
#define QUICKNAVIGATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>

//Forward Declarations
class SystemObject;

class QuickNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    QuickNavigationWidget(QWidget *parent = 0);
    void addOpenContainer(SystemObject* pContainer);
    int getCurrentId();

signals:

public slots:
    void gotoContainerAndCloseSubcontainers(int id);

private:
    void refreshCurrentLabel();
    void refreshVisible();

    QVector<SystemObject*> mContainerObjectPtrs;
    QVector<QPushButton*> mPushButtonPtrs;
    QLabel *mpCurrentSysNameLabel;
    QButtonGroup *mpButtonGroup;
};

#endif // QUICKNAVIGATIONWIDGET_H
