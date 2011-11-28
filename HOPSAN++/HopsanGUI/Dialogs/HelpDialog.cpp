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
#include <QGridLayout>
#include <QPushButton>

#include "MainWindow.h"
#include "HelpDialog.h"
#include "common.h"
#include "version_gui.h"


//! @class HelpDialog
//! @brief A class for displaying the "Help" dialog
//!
//! Shows the HTML user guide from Doxygen
//!

//! Constructor for the help dialog
//! @param parent Pointer to the main window
HelpDialog::HelpDialog(MainWindow *parent)
    : QDialog(parent)
{
    this->setObjectName("HelpDialog");
    this->setWindowTitle("Hopsan User Guide");
    this->setSizeGripEnabled(true);
    this->setMinimumSize(640, 480);

    mpHelp = new QWebView(this);

    mpLayout = new QGridLayout;
    mpLayout->addWidget(mpHelp, 0, 0);
    setLayout(mpLayout);

    this->resize(1024,768);
}


void HelpDialog::open()
{
    qDebug() << gExecPath + QString(HELPPATH) + "index.html";
    mpHelp->load(QUrl::fromLocalFile(gExecPath + QString(HELPPATH) + "index.html"));

    QDialog::open();
}
