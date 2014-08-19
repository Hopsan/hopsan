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
    void setValues();

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

    //Resize
    QList<QLineEdit*> mpInitScaleXLineEdits;
    QList<QLineEdit*> mpInitScaleYLineEdits;
    QList<QLineEdit*> mpResizeXLineEdits;
    QList<QLineEdit*> mpResizeYLineEdits;
    QList<QLineEdit*> mpScaleDataIdx1LineEdits;
    QList<QLineEdit*> mpScaleDataIdx2LineEdits;

    //Color
    QList<QLineEdit*> mpInitColorLineEdits;
    QList<QLineEdit*> mpColorModifiersLineEdits;
    QList<QLineEdit*> mpColorDataIdxLineEdits;

    //Transform origin
    QList<QLineEdit*> mpTransformOriginXLineEdits;
    QList<QLineEdit*> mpTransformOriginYLineEdits;

    //Relative movable
    QList<QLineEdit*> mpMovableRelativeLineEdits;

//    //Movable ports
//    QStringList movablePortNames;
//    QList<double> movablePortStartX;
//    QList<double> movablePortStartY;

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
