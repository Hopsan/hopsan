/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
    void chooseCustomTempPath();

signals:
    void paletteChanged();

private:
    void setCompilerPath(QString path, bool x64);

    QWidget *mpInterfaceWidget;
    QToolButton *mpBackgroundColorButton;
    QColor mPickedBackgroundColor;
    QCheckBox *mpCheckDevelopmentUpdatesCheckBox;
    QCheckBox *mpNativeStyleSheetCheckBox;
    QCheckBox *mpShowPopupHelpCheckBox;
    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;
    QCheckBox *mpAutoSetPwdToMwdCheckBox;
    QDoubleSpinBox *mpZoomStepSpinBox;

    QWidget *mpSimulationWidget;
    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QSpinBox *mpProgressBarSpinBox;
    QCheckBox *mpLogDuringSimulationCheckBox;
    QLabel *mpLogStepsLabel;
    QSpinBox *mpLogStepsSpinBox;

    QWidget *mpUnitScaleWidget;

    QLineEdit *mpCompiler32LineEdit;
    QLineEdit *mpCompiler64LineEdit;
    QLabel *mpCompiler32WarningLabel;
    QLabel *mpCompiler64WarningLabel;
    QCheckBox *mpPrefereIncludedCompiler;
    QLabel *mpIncludedCompilerLabel;

    QLineEdit *mpRemoteHopsanAddress;
    QLineEdit *mpRemoteHopsanAddressServerAddress;
    QLineEdit *mpRemoteHopsanUserId;
    QSpinBox *mpRemoteShortTOSpinbox;
    QSpinBox *mpRemoteLongTOSpinbox;
    QCheckBox *mpUseRemoteHopsanAddressServer;
    QCheckBox *mpUseRemoteOptimization;

    QWidget *mpPlottingWidget;
    QSpinBox *mpGenerationLimitSpinBox;
    QCheckBox *mpAutoLimitGenerationsCheckBox;
    QCheckBox *mpCacheLogDataCeckBox;
    QCheckBox *mpShowHiddenNodeDataVarCheckBox;
    QCheckBox *mpPlotWindowsOnTop;
    QSpinBox *mpDefaultPloExportVersion;
    QLineEdit *mpCustomTempPathLineEdit;
};

#endif // OPTIONSDIALOG_H
