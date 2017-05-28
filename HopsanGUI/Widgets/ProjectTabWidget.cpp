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
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

//Qt includes
#include <QTabBar>
#include <QTabWidget>

//Hopsan includes
#include "ProjectTabWidget.h"


//! @class CentralTabWidget
//! @brief The CentralTabWidget class is the central tab widget in the main window

//! Constructor.
//! @param parent defines a parent to the new instanced object.
CentralTabWidget::CentralTabWidget(QWidget *parent)
        :   QTabWidget(parent)
{
    setPalette(parent->palette());
    setTabsClosable(true);
}

void CentralTabWidget::setTabNotClosable(int index)
{
    QWidget *pCloseButton = tabBar()->tabButton(index, QTabBar::RightSide);
    if (!pCloseButton)
    {
        pCloseButton = tabBar()->tabButton(index, QTabBar::LeftSide);
    }

    if (pCloseButton)
    {
        pCloseButton->setDisabled(true);
        pCloseButton->resize(0, 0);
    }
}

void CentralTabWidget::tabInserted(int index)
{
    tabBar()->setVisible(count() > 1);
    QTabWidget::tabInserted(index);
}

void CentralTabWidget::tabRemoved(int index)
{
    tabBar()->setVisible(count() > 1);
    QTabWidget::tabRemoved(index);
}

