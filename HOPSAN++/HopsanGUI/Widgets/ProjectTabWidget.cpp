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
#include "Configuration.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "ProjectTabWidget.h"

//! @class CentralTabWidget
//! @brief The CentralTabWidget class is the central tab widget in the main window

//! Constructor.
//! @param parent defines a parent to the new instanced object.
CentralTabWidget::CentralTabWidget(MainWindow *pParentMainWindow)
        :   QTabWidget(pParentMainWindow)
{
    this->setPalette(gConfig.getPalette());

    connect(this, SIGNAL(currentChanged(int)),  pParentMainWindow,                      SLOT(updateToolBarsToNewTab()), Qt::UniqueConnection);
    connect(this, SIGNAL(currentChanged(int)),  pParentMainWindow,                      SLOT(refreshUndoWidgetList()), Qt::UniqueConnection);

    setTabsClosable(true);

    connect(this,   SIGNAL(currentChanged(int)),    gpModelHandler, SLOT(selectModelByTabIndex(int)), Qt::UniqueConnection);
    connect(this,   SIGNAL(tabCloseRequested(int)), gpModelHandler, SLOT(closeModelByTabIndex(int)), Qt::UniqueConnection);

    //this->hide();
}

void CentralTabWidget::setTabNotClosable(int index)
{
    tabBar()->tabButton(index, QTabBar::RightSide)->resize(0, 0);
}

void CentralTabWidget::tabInserted(int index)
{
    tabBar()->setVisible(this->count() > 1);
    QTabWidget::tabInserted(index);
}

void CentralTabWidget::tabRemoved(int index)
{
    tabBar()->setVisible(this->count() > 1);
    QTabWidget::tabRemoved(index);
}



//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.

