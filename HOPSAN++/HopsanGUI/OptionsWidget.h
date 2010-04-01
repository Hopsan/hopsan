//$Id: OptionsWidget.h 1195 2010-04-01 09:25:58Z robbr48 $

#ifndef OptionsWidget_H
#define OptionsWidget_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include "mainwindow.h"

class MainWindow;

class OptionsWidget : public QDialog
{
    Q_OBJECT

public:
    OptionsWidget(MainWindow *parent = 0);
    ~OptionsWidget();

    MainWindow *mpParentMainWindow;

    QPushButton *cancelButton;
    QPushButton *applyButton;
    QPushButton *okButton;
    QDialogButtonBox *buttonBox;

    QWidget *mpCentralwidget;


public slots:
    void updateValues();
};

#endif // OptionsWidget_H
