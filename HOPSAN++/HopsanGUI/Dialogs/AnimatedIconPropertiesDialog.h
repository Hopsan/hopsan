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

#include <QtGui>

#include "GUIObjects/AnimatedComponent.h"


class AnimatedIconPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    AnimatedIconPropertiesDialog(AnimatedComponent *pAnimatedComponent, int index, MainWindow *parent = 0);

private slots:
    void setValues();

private:
    //Member pointers
    AnimatedComponent *mpAnimatedComponent;
    int mIdx;
    ModelObjectAnimationData *mpData;

    //Multiplier
    QLabel *mpMultplierLabel;
    QLineEdit *mpMultiplierLineEdit;

    //Divisor
    QLabel *mpDivisorLabel;
    QLineEdit *mpDivisorLineEdit;

    //Speed
    QLabel *mpSpeedXLabel;
    QLabel *mpSpeedYLabel;
    QLabel *mpSpeedThetaLabel;
    QLineEdit *mpSpeedXLineEdit;
    QLineEdit *mpSpeedYLineEdit;
    QLineEdit *mpSpeedThetaLineEdit;

    //Buttons
    QDialogButtonBox *mpButtonBox;
    QPushButton *mpOkButton;
    QPushButton *mpCancelButton;

    //Layout
    QGridLayout *mpLayout;
};

#endif // ANIMATEDICONPROPERTIESDIALOG_H
