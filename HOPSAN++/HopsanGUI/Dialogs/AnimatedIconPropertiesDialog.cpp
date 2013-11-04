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
//! @file   AnimatedIconPropertiesDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-05-02
//!
//! @brief Contains a dialog class for animation settings in animated components
//!
//$Id$

#include <QtGui>
//#include <cassert>
//#include <iostream>

#include "global.h"
#include "GUIObjects/AnimatedComponent.h"
#include "AnimatedIconPropertiesDialog.h"
#include "Configuration.h"


//! @class AnimatedIconPropertiesDialog
//! @brief The AnimatedIconPropertiesDialog class is a dialog used to change animation settings in animated component.


//! @brief Constructor for the parameter dialog for animated components
//! @param [in] pAnimatedComponent Pointer to the animated component
//! @param [in] index Index of movable icon that was double-clicked
//! @param [in] Pointer to the parent window
AnimatedIconPropertiesDialog::AnimatedIconPropertiesDialog(AnimatedComponent *pAnimatedComponent, int index, QWidget *parent)
    : QDialog(parent)
{
    this->setPalette(gpConfig->getPalette());
    this->setWindowTitle("Movable Properties");

    mpAnimatedComponent = pAnimatedComponent;
    mIdx = index;
    mpData = mpAnimatedComponent->getAnimationDataPtr();

    //Adjustable
    mpAdjustableCheckBox = new QCheckBox("Adjustable");
    mpAdjustableCheckBox->setChecked(mpData->isAdjustable[mIdx]);

    //Multiplier
    mpMultplierLabel = new QLabel("Parameter multiplier: ", this);
    mpMultiplierLineEdit = new QLineEdit(mpData->multipliers[mIdx], this);

    //Divisor
    mpDivisorLabel = new QLabel("Paramter divisor: ", this);
    mpDivisorLineEdit = new QLineEdit(mpData->divisors[mIdx], this);

    //Speed X
    mpSpeedXLabel = new QLabel("Horizontal Speed: ", this);
    mpSpeedXLineEdit = new QLineEdit(this);
    mpSpeedXLineEdit->setText(QString::number(mpData->speedX[mIdx]));
    mpSpeedXLineEdit->setValidator(new QDoubleValidator(this));

    //Speed Y
    mpSpeedYLabel = new QLabel("Vertical Speed: ", this);
    mpSpeedYLineEdit = new QLineEdit(this);
    mpSpeedYLineEdit->setText(QString::number(mpData->speedY[mIdx]));
    mpSpeedYLineEdit->setValidator(new QDoubleValidator(this));

    //Speed Theta
    mpSpeedThetaLabel = new QLabel("Rotational Speed: ", this);
    mpSpeedThetaLineEdit = new QLineEdit(this);
    mpSpeedThetaLineEdit->setText(QString::number(mpData->speedTheta[mIdx]));
    mpSpeedThetaLineEdit->setValidator(new QDoubleValidator(this));

    //Resize X
    mpResizeXLabel = new QLabel("Horizontal Resize Factor: ", this);
    mpResizeXLineEdit = new QLineEdit(this);
    mpResizeXLineEdit->setText(QString::number(mpData->resizeX[mIdx]));
    mpResizeXLineEdit->setValidator(new QDoubleValidator(this));

    //Resize Y
    mpResizeYLabel = new QLabel("Vertical Resize Factor: ", this);
    mpResizeYLineEdit = new QLineEdit(this);
    mpResizeYLineEdit->setText(QString::number(mpData->resizeY[mIdx]));
    mpResizeYLineEdit->setValidator(new QDoubleValidator(this));

    //Buttons
    mpOkButton = new QPushButton("Ok", this);
    mpOkButton->setDefault(true);
    mpCancelButton = new QPushButton("Cancel", this);
    mpButtonBox = new QDialogButtonBox(this);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::AcceptRole);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::RejectRole);

    //Layout
    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpAdjustableCheckBox,   0, 0, 1, 2);
    mpLayout->addWidget(mpMultplierLabel,       1, 0);
    mpLayout->addWidget(mpMultiplierLineEdit,   1, 1);
    mpLayout->addWidget(mpDivisorLabel,         2, 0);
    mpLayout->addWidget(mpDivisorLineEdit,      2, 1);
    mpLayout->addWidget(mpSpeedXLabel,          3, 0);
    mpLayout->addWidget(mpSpeedXLineEdit,       3, 1);
    mpLayout->addWidget(mpSpeedYLabel,          4, 0);
    mpLayout->addWidget(mpSpeedYLineEdit,       4, 1);
    mpLayout->addWidget(mpSpeedThetaLabel,      5, 0);
    mpLayout->addWidget(mpSpeedThetaLineEdit,   5, 1);
    mpLayout->addWidget(mpResizeXLabel,         6, 0);
    mpLayout->addWidget(mpResizeXLineEdit,      6, 1);
    mpLayout->addWidget(mpResizeYLabel,         7, 0);
    mpLayout->addWidget(mpResizeYLineEdit,      7, 1);
    mpLayout->addWidget(mpButtonBox,            8, 0, 1, 2);

    this->setLayout(mpLayout);

    connect(mpOkButton,     SIGNAL(pressed()), this, SLOT(setValues()));
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(reject()));
}


//! @brief Write back new values to the animated component
void AnimatedIconPropertiesDialog::setValues()
{
    //! todo Store new values in container object and save to HMF

    mpData->isAdjustable[mIdx] = mpAdjustableCheckBox->isChecked();
    mpData->multipliers[mIdx] = mpMultiplierLineEdit->text();
    mpData->divisors[mIdx] = mpDivisorLineEdit->text();
    mpData->speedX[mIdx] = mpSpeedXLineEdit->text().toDouble();
    mpData->speedY[mIdx] = mpSpeedYLineEdit->text().toDouble();
    mpData->speedTheta[mIdx] = mpSpeedThetaLineEdit->text().toDouble();
    mpData->resizeX[mIdx] = mpResizeXLineEdit->text().toDouble();
    mpData->resizeY[mIdx] = mpResizeYLineEdit->text().toDouble();

    this->accept();
}
