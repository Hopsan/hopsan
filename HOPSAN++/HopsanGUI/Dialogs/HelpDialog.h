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
//! @file   HelpDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-02
//!
//! @brief Contains a class for the Help dialog
//!
//$Id$

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QWebView>
#include <QGridLayout>

class MainWindow;

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    HelpDialog(MainWindow *parent = 0);

private:
    QWebView *mpHelp;
    QPushButton *mpOkButton;
    QGridLayout *mpLayout;
};

#endif // HELPDIALOG_H
