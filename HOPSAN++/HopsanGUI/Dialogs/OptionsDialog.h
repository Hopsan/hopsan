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
//! @file   OptionsDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id$

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QToolButton>
#include <QComboBox>

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(QWidget *parent = 0);

public slots:
    void reset();
    void openXml();
    void updateValues();
    void colorDialog();
    void show();

private slots:
    void addPressureUnit();
    void addFlowUnit();
    void addForceUnit();
    void addPositionUnit();
    void addVelocityUnit();
    void addTorqueUnit();
    void addAngleUnit();
    void addAngularVelocityUnit();
    void addTimeUnit();
    void addCustomUnitDialog(QString physicalQuantity);
    void addCustomUnit();
    void updateCustomUnits();

signals:
    void paletteChanged();

private:
    QColor mPickedBackgroundColor;

    QCheckBox *mpNativeStyleSheetCheckBox;
    QCheckBox *mpAlwaysLoadLastSessionCheckBox;
    QCheckBox *mpShowPopupHelpCheckBox;
    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;

    QToolButton *mpBackgroundColorButton;
    QWidget *mpInterfaceWidget;
    QGridLayout *mpInterfaceLayout;

    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QLabel *mpThreadsWarningLabel;
    QLabel *mpProgressBarLabel;
    QSpinBox *mpProgressBarSpinBox;
    QWidget *mpSimulationWidget;
    QGridLayout *mpSimulationLayout;

    QSpinBox *mpGenerationLimitSpinBox;
    QCheckBox *mpAutoLimitGenerationsCheckBox;
    QCheckBox *mpCacheLogDataCeckBox;


    QComboBox *mpPressureUnitComboBox;
    QComboBox *mpFlowUnitComboBox;
    QComboBox *mpForceUnitComboBox;
    QComboBox *mpPositionUnitComboBox;
    QComboBox *mpVelocityUnitComboBox;
    QComboBox *mpTorqueUnitComboBox;
    QComboBox *mpAngleUnitComboBox;
    QComboBox *mpAngularVelocityUnitComboBox;
    QComboBox *mpTimeUnitComboBox;

    QWidget *mpPlottingWidget;
    QGridLayout *mpPlottingLayout;

    QDialog *mpAddUnitDialog;
    QLineEdit *mpUnitNameBox;
    QLineEdit *mpScaleBox;

    QString mPhysicalQuantityToModify;
};

#endif // OPTIONSDIALOG_H
