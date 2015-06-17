/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

private slots:
    void setCompiler32Path();
    void setCompiler64Path();
    void setCompiler32Path(QString path);
    void setCompiler64Path(QString path);


signals:
    void paletteChanged();

private:
    void setCompilerPath(QString path, bool x64);

    QWidget *mpInterfaceWidget;
    QToolButton *mpBackgroundColorButton;
    QColor mPickedBackgroundColor;
    QCheckBox *mpNativeStyleSheetCheckBox;
    QCheckBox *mpShowPopupHelpCheckBox;
    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;
    QCheckBox *mpAutoSetPwdToMwdCheckBox;

    QWidget *mpSimulationWidget;
    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QSpinBox *mpProgressBarSpinBox;

    QWidget *mpUnitScaleWidget;

    QLineEdit *mpCompiler32LineEdit;
    QLineEdit *mpCompiler64LineEdit;
    QLabel *mpCompiler32WarningLabel;
    QLabel *mpCompiler64WarningLabel;

    QLineEdit *mpRemoteHopsanAddress;
    QLineEdit *mpRemoteHopsanAddressServerAddress;
    QCheckBox *mpUseRemoteHopsanAddressServer;
    QCheckBox *mpUseRemoteOptimization;

    QWidget *mpPlottingWidget;
    QSpinBox *mpGenerationLimitSpinBox;
    QCheckBox *mpAutoLimitGenerationsCheckBox;
    QCheckBox *mpCacheLogDataCeckBox;
    QCheckBox *mpShowHiddenNodeDataVarCheckBox;
    QCheckBox *mpPlotWindowsOnTop;
};

#endif // OPTIONSDIALOG_H
