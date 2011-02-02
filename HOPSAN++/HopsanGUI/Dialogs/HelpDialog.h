//!
//! @file   HelpDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-02
//!
//! @brief Contains a class for the Help dialog
//!
//$Id: HelpDialog.h 2427 2010-12-30 21:14:01Z petno25 $

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
