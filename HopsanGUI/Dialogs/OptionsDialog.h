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
    void resetConfigDefaults();
    void openConfigFile();
    void setValues();
    void colorDialog();
    void show();

signals:
    void paletteChanged();

private:
    QWidget *mpInterfaceWidget;
    QToolButton *mpBackgroundColorButton;
    QColor mPickedBackgroundColor;
    QCheckBox *mpNativeStyleSheetCheckBox;
    QCheckBox *mpShowPopupHelpCheckBox;
    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;

    QWidget *mpSimulationWidget;
    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QSpinBox *mpProgressBarSpinBox;

    QWidget *mpUnitScaleWidget;

    QWidget *mpPlottingWidget;
    QSpinBox *mpGenerationLimitSpinBox;
    QCheckBox *mpAutoLimitGenerationsCheckBox;
    QCheckBox *mpCacheLogDataCeckBox;
    QCheckBox *mpShowHiddenNodeDataVarCheckBox;
};

#endif // OPTIONSDIALOG_H
