//$Id$

#ifndef PREFERENCEWIDGET_H
#define PREFERENCEWIDGET_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>

class MainWindow;

class PreferenceWidget : public QDialog
{
    Q_OBJECT

public:
    PreferenceWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

    bool mIsoBool;

    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLabel *mpUserIconLabel;
    QLabel *mpIsoIconLabel;
    QPushButton *mpIsoIconBrowseButton;
    QPushButton *mpUserIconBrowseButton;

    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;

    QLabel *mpNumberOfSamplesLabel;
    QLineEdit *mpNumberOfSamplesBox;

    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;
    QDialogButtonBox *mpButtonBox;

    QWidget *mpCentralwidget;
    QGridLayout *mpLayout;



public slots:
    void show();
    void updateValues();
    void browseUser();
    void browseIso();
};

#endif // PREFERENCEWIDGET_H
