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
//! @file   HelpDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the Help dialog
//!
//$Id$

//Qt includes
#include <QUrl>
#include <QToolBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>

//Hopsan includes
#include "common.h"
#include "global.h"
#include "DesktopHandler.h"
#include "HelpDialog.h"

//! @class HelpDialog
//! @brief A class for displaying the "Help" dialog
//!
//! Shows the HTML user guide from Doxygen
//!

//! Constructor for the help dialog
//! @param parent Pointer to the main window
HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowIcon(gpMainWindowWidget->windowIcon());
    this->setObjectName("HelpDialog");
    this->setWindowTitle("Hopsan User Guide");
    this->setMinimumSize(640, 480);
    this->setWindowModality(Qt::NonModal);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    mpHelp = new WebViewWrapper(true, this);
    pLayout->addWidget(mpHelp);

    //! @todo Set size depending one screen size
    this->resize(1024,768);
}


void HelpDialog::open()
{
    mpHelp->loadHtmlFile(QUrl::fromLocalFile(gpDesktopHandler->getHelpPath() + "index.html"));

    //Using show instead of open for modaless window
    QDialog::show();
}


void HelpDialog::open(QString file)
{
    mpHelp->loadHtmlFile(QUrl::fromLocalFile(gpDesktopHandler->getHelpPath() + "/" + file));

    //Using show instead of open for modaless window
    QDialog::show();
}


void HelpDialog::centerOnScreen()
{
    int sx = qApp->desktop()->screenGeometry().center().x();
    int sy = qApp->desktop()->screenGeometry().center().y();
    int w = this->width();
    int h = this->height();

    this->move(sx-w/2, sy-h/2);
}

