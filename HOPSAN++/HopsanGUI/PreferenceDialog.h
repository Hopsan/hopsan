//$Id$

#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>

class MainWindow;

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    PreferenceDialog(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

public slots:
    void show();
    void updateValues();
    void browseUser();
    void browseIso();

private:
    bool mIsoBool;

    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLineEdit *mpNumberOfSamplesBox;

    QLabel *mpUserIconLabel;
    QLabel *mpIsoIconLabel;
    QLabel *mpNumberOfSamplesLabel;

    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;

    QPushButton *mpIsoIconBrowseButton;
    QPushButton *mpUserIconBrowseButton;
    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;

    QDialogButtonBox *mpButtonBox;
    QWidget *mpCentralwidget;
    QGridLayout *mpLayout;
};

#endif // PREFERENCEDIALOG_H
