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
#include <QComboBox>

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
    QGroupBox *mpInterfaceGroupBox;
    QGridLayout *mpInterfaceLayout;

    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpProgressBarLabel;
    QSpinBox *mpProgressBarSpinBox;
    QGroupBox *mpSimulationGroupBox;
    QGridLayout *mpSimulationLayout;

    QLabel *mpPressureUnitLabel;
    QComboBox *mpPressureUnitComboBox;
    QPushButton *mpAddPressureUnitButton;
    QLabel *mpFlowUnitLabel;
    QComboBox *mpFlowUnitComboBox;
    QPushButton *mpAddFlowUnitButton;
    QLabel *mpForceUnitLabel;
    QComboBox *mpForceUnitComboBox;
    QPushButton *mpAddForceUnitButton;
    QLabel *mpPositionUnitLabel;
    QComboBox *mpPositionUnitComboBox;
    QPushButton *mpAddPositionUnitButton;
    QLabel *mpVelocityUnitLabel;
    QComboBox *mpVelocityUnitComboBox;
    QPushButton *mpAddVelocityUnitButton;
    QGroupBox *mpPlottingGroupBox;
    QGridLayout *mpPlottingLayout;

    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;
    QDialogButtonBox *mpButtonBox;

    QWidget *mpCentralwidget;


    QDialog *mpAddUnitDialog;
    QLabel *mpNameLabel;
    QLineEdit *mpUnitNameBox;
    QLabel *mpScaleLabel;
    QLineEdit *mpScaleBox;
    QPushButton *mpDoneInUnitDialogButton;
    QPushButton *mpCancelInUnitDialogButton;
    QString mPhysicalQuantityToModify;



public slots:
    void updateValues();
    void colorDialog();
    void show();

private slots:
    void addPressureUnit();
    void addFlowUnit();
    void addForceUnit();
    void addPositionUnit();
    void addVelocityUnit();
    void addAlternativeUnitDialog(QString physicalQuantity);
    void addAlternativeUnit();
    void updateAlternativeUnits();

private:
    QColor mPickedBackgroundColor;
};

#endif // OptionsWidget_H
