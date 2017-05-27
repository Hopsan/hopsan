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
//! @file   AnimatedIconPropertiesDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-05-02
//!
//! @brief Contains a dialog class for animation settings in animated components
//!
//$Id$

#ifndef ANIMATEDICONPROPERTIESDIALOG_H
#define ANIMATEDICONPROPERTIESDIALOG_H

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QDoubleSpinbox;
class QLabel;
class QLineEdit;
class QGridLayout;

class AnimatedComponent;
class AnimatedIcon;
class ModelObjectAnimationData;
class MainWindow;


class AnimatedIconPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    AnimatedIconPropertiesDialog(AnimatedComponent *pAnimatedComponent, int index, QWidget *parent = 0);

private slots:
    void updateValues();
    void setValues();
    void resetValues();

private:
    //Member pointers
    AnimatedComponent *mpAnimatedComponent;
    int mIdx;
    ModelObjectAnimationData *mpData;

    //Data ports
    QList<QLineEdit*> mpDataPortsLineEdits;
    QList<QLineEdit*> mpDataNamesLineEdits;

    //Multipliers and divisors
    QList<QLineEdit*> mpMultipliersLineEdits;
    QList<QLineEdit*> mpDivisorsLineEdits;

    //Movement
    QList<QLineEdit*> mpStartXLineEdits;
    QList<QLineEdit*> mpStartYLineEdits;
    QList<QLineEdit*> mpStartThetaLineEdits;
    QList<QLineEdit*> mpMovementXLineEdits;
    QList<QLineEdit*> mpMovementYLineEdits;
    QList<QLineEdit*> mpMovementThetaLineEdits;
    QList<QLineEdit*> mpMovementDataIdxLineEdits;
    QList<QLineEdit*> mpMovementMultipliersLineEdits;
    QList<QLineEdit*> mpMovementDivisorsLineEdits;

    //Resize
    QList<QLineEdit*> mpInitScaleXLineEdits;
    QList<QLineEdit*> mpInitScaleYLineEdits;
    QList<QLineEdit*> mpResizeXLineEdits;
    QList<QLineEdit*> mpResizeYLineEdits;
    QList<QLineEdit*> mpResizeDataIdx1LineEdits;
    QList<QLineEdit*> mpResizeDataIdx2LineEdits;
    QList<QLineEdit*> mpResizeMultipliersLineEdits;
    QList<QLineEdit*> mpResizeDivisorsLineEdits;

    //Color
    QList<QLineEdit*> mpInitColorLineEdits;
    QList<QLineEdit*> mpColorModifiersLineEdits;
    QList<QLineEdit*> mpColorDataIdxLineEdits;

    //Transform origin
    QList<QLineEdit*> mpTransformOriginXLineEdits;
    QList<QLineEdit*> mpTransformOriginYLineEdits;

    //Relative movable
    QList<QLineEdit*> mpMovableRelativeLineEdits;

    //Movable ports
    QList<QLineEdit*> mpMovablePortNamesLineEdits;
    QList<QLineEdit*> mpMovablePortStartXLineEdits;
    QList<QLineEdit*> mpMovablePortStartYLineEdits;

//    //Adjustable
//    bool isAdjustable;
//    double adjustableMinX;
//    double adjustableMaxX;
//    double adjustableMinY;
//    double adjustableMaxY;
//    QString adjustablePort;
//    QString adjustableDataName;
//    double adjustableGainX;
//    double adjustableGainY;

//    //Switchable
//    bool isSwitchable;
//    double switchableOffValue;
//    double switchableOnValue;
//    QString switchablePort;
//    QString switchableDataName;

//    //Indicator
//    bool isIndicator;
//    QString indicatorPort;
//    QString indicatorDataName;
};

#endif // ANIMATEDICONPROPERTIESDIALOG_H
