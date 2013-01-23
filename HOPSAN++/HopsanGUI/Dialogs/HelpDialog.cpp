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
//! @file   HelpDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the Help dialog
//!
//$Id$

#include <QWebView>

#include "MainWindow.h"
#include "HelpDialog.h"
#include "common.h"

//! @class HelpDialog
//! @brief A class for displaying the "Help" dialog
//!
//! Shows the HTML user guide from Doxygen
//!

//! Constructor for the help dialog
//! @param parent Pointer to the main window
HelpDialog::HelpDialog(MainWindow *parent)
    : QDialog()
{
    this->setWindowIcon(gpMainWindow->windowIcon());
    this->setObjectName("HelpDialog");
    this->setWindowTitle("Hopsan User Guide");
    this->setMinimumSize(640, 480);
    this->setWindowModality(Qt::NonModal);

    mpHelp = new QWebView(this);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(mpHelp);
    this->setLayout(pLayout);

    //! @todo Set size depending one screen size
    this->resize(1024,768);
}


void HelpDialog::open()
{
    mpHelp->load(QUrl::fromLocalFile(QString(HELPPATH) + "index.html"));

    //Using show instead of open for modaless window
    QDialog::show();
}


void HelpDialog::open(QString file)
{
    mpHelp->load(QUrl::fromLocalFile(QString(HELPPATH) + file));

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

