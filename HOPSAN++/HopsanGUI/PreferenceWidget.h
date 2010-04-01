//$Id$

#ifndef PREFERENCEWIDGET_H
#define PREFERENCEWIDGET_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include "mainwindow.h"

class MainWindow;

class PreferenceWidget : public QDialog
{
    Q_OBJECT

public:
    PreferenceWidget(MainWindow *parent = 0);
    ~PreferenceWidget();

    MainWindow *mpParentMainWindow;

    bool isoBool;
    QCheckBox *isoCheckBox;
    QPushButton *cancelButton;
    QPushButton *applyButton;
    QPushButton *okButton;
    QDialogButtonBox *buttonBox;

    QWidget *mpCentralwidget;


public slots:
    void updateValues();
};

#endif // PREFERENCEWIDGET_H
