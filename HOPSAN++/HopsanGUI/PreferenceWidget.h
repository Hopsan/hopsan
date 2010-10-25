//$Id$

#ifndef PREFERENCEWIDGET_H
#define PREFERENCEWIDGET_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>

class MainWindow;

class PreferenceWidget : public QDialog
{
    Q_OBJECT

public:
    PreferenceWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

    bool mIsoBool;
    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;
    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;
    QPushButton *mpIsoIconBrowseButton;
    QPushButton *mpUserIconBrowseButton;
    QDialogButtonBox *mpButtonBox;
    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLabel *mpUserIconLabel;
    QLabel *mpIsoIconLabel;
    QWidget *mpCentralwidget;
    QGridLayout *mpLayout;

public slots:
    void show();
    void updateValues();
    void browseUser();
    void browseIso();
};

#endif // PREFERENCEWIDGET_H
