//$Id$

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class MainWindow;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(MainWindow *parent = 0);
};

#endif // ABOUTDIALOG_H
