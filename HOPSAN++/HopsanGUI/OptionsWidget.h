//$Id: OptionsWidget.h 1195 2010-04-01 09:25:58Z robbr48 $

#ifndef OptionsWidget_H
#define OptionsWidget_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QToolButton>

class MainWindow;

class OptionsWidget : public QDialog
{
    Q_OBJECT

public:
    OptionsWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

    //! @todo Add "m" at the beginning of the members

    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;
    QLabel *mpBackgroundColorLabel;
    QToolButton *mpBackgroundColorButton;
    QGridLayout *mpInterfaceLayout;
    QGroupBox *mpInterfaceGroupBox;

    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpProgressBarLabel;
    QSpinBox *mpProgressBarSpinBox;
    QGroupBox *mpSimulationGroupBox;
    QGridLayout *mpSimulationLayout;

    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;
    QDialogButtonBox *mpButtonBox;

    QWidget *mpCentralwidget;


public slots:
    void updateValues();
    void colorDialog();
    void show();

private:
    QColor mPickedBackgroundColor;
};

#endif // OptionsWidget_H
