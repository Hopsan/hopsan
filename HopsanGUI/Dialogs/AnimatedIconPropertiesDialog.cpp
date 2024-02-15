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
//! @file   AnimatedIconPropertiesDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-05-02
//!
//! @brief Contains a dialog class for animation settings in animated components
//!
//$Id$

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTabWidget>
#include <QScrollArea>

#include "global.h"
#include "MessageHandler.h"
#include "LibraryHandler.h"
#include "GUIObjects/AnimatedComponent.h"
#include "GUIObjects/GUIModelObject.h"
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
    this->setWindowTitle("Animated Component Properties");

    this->resize(800,600);

    mpAnimatedComponent = pAnimatedComponent;
    mIdx = index;
    mpData = mpAnimatedComponent->getAnimationDataPtr();

    QTabWidget *pTabWidget = new QTabWidget(this);

    for(int i=0; i<mpData->movables.size(); ++i)
    {
        QWidget *pMovableWidget = new QWidget(this);
        QScrollArea *pScrollArea = new QScrollArea(this);
        QWidget *pScrollWidget = new QWidget(this);
        QGridLayout *pMovableLayout = new QGridLayout(this);
        QGridLayout *pScrollLayout = new QGridLayout(this);
        pScrollWidget->setPalette(this->palette());

        int row = 0;

        QFont boldFont = gpConfig->getFont();
        boldFont.setBold(true);

        //Data Ports
        QLabel *pDataSourcesLabel = new QLabel("Data Sources");
        pDataSourcesLabel->setFont(boldFont);
        pScrollLayout->addWidget(pDataSourcesLabel, row, 0, 1, 2);
        ++row;

        //Data Ports
        QLabel *pDataPortsLabel = new QLabel("Data Ports: ", this);
        mpDataPortsLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pDataPortsLabel,             row, 0);
        pScrollLayout->addWidget(mpDataPortsLineEdits.last(), row, 1);
        ++row;

        //Data Names
        QLabel *pDataNamesLabel = new QLabel("Data Names: ", this);
        mpDataNamesLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pDataNamesLabel,             row, 0);
        pScrollLayout->addWidget(mpDataNamesLineEdits.last(), row, 1);
        ++row;

        //Movement
        QLabel *pMovementLabel = new QLabel("Movement");
        pMovementLabel->setFont(boldFont);
        pScrollLayout->addWidget(pMovementLabel, row, 0, 1, 2);
        ++row;

        //Start X
        QLabel *pStartXLabel = new QLabel("Initial Horizontal Position: ", this);
        mpStartXLineEdits.append(new QLineEdit(this));
        mpStartXLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pStartXLabel,             row, 0);
        pScrollLayout->addWidget(mpStartXLineEdits.last(), row, 1);
        ++row;

        //Start Y
        QLabel *pStartYLabel = new QLabel("Initial Vertical Position: ", this);
        mpStartYLineEdits.append(new QLineEdit(this));
        mpStartYLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pStartYLabel,             row, 0);
        pScrollLayout->addWidget(mpStartYLineEdits.last(), row, 1);
        ++row;

        //Start Theta
        QLabel *pStartThetaLabel = new QLabel("Initial Angle: ", this);
        mpStartThetaLineEdits.append(new QLineEdit(this));
        mpStartThetaLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pStartThetaLabel,             row, 0);
        pScrollLayout->addWidget(mpStartThetaLineEdits.last(), row, 1);
        ++row;

        //Movement X
        QLabel *pMovementXLabel = new QLabel("Horizontal Movement: ", this);
        mpMovementXLineEdits.append(new QLineEdit(this));
        mpMovementXLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pMovementXLabel,             row, 0);
        pScrollLayout->addWidget(mpMovementXLineEdits.last(), row, 1);
        ++row;

        //Movement Y
        QLabel *pMovementYLabel = new QLabel("Vertical Movement: ", this);
        mpMovementYLineEdits.append(new QLineEdit(this));
        mpMovementYLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pMovementYLabel,             row, 0);
        pScrollLayout->addWidget(mpMovementYLineEdits.last(), row, 1);
        ++row;

        //Movement Theta
        QLabel *pMovementThetaLabel = new QLabel("Rotational Movement: ", this);
        mpMovementThetaLineEdits.append(new QLineEdit(this));
        mpMovementThetaLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pMovementThetaLabel,             row, 0);
        pScrollLayout->addWidget(mpMovementThetaLineEdits.last(), row, 1);
        ++row;

        //Movement Data Index
        QLabel *pMovementDataIdxLabel = new QLabel("Movement Data Index: ", this);
        mpMovementDataIdxLineEdits.append(new QLineEdit(this));
        mpMovementDataIdxLineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pMovementDataIdxLabel,       row, 0);
        pScrollLayout->addWidget(mpMovementDataIdxLineEdits.last(), row, 1);
        ++row;

        //Movement Multipliers
        QLabel *pMovementMultipliersLabel = new QLabel("Movement Multipliers: ", this);
        mpMovementMultipliersLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pMovementMultipliersLabel,       row, 0);
        pScrollLayout->addWidget(mpMovementMultipliersLineEdits.last(), row, 1);
        ++row;

        //Movement Divisors
        QLabel *pMovementDivisorsLabel = new QLabel("Movement Divisors: ", this);
        mpMovementDivisorsLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pMovementDivisorsLabel,       row, 0);
        pScrollLayout->addWidget(mpMovementDivisorsLineEdits.last(), row, 1);
        ++row;

        //Stretch
        QLabel *pStretchLabel = new QLabel("Stretch");
        pStretchLabel->setFont(boldFont);
        pScrollLayout->addWidget(pStretchLabel, row, 0, 1, 2);
        ++row;

        //Init Scale X
        QLabel *pInitScaleXLabel = new QLabel("Initial Horizontal Stretch: ", this);
        mpInitScaleXLineEdits.append(new QLineEdit(this));
        mpInitScaleXLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pInitScaleXLabel,             row, 0);
        pScrollLayout->addWidget(mpInitScaleXLineEdits.last(), row, 1);
        ++row;

        //Init Scale Y
        QLabel *pInitScaleYLabel = new QLabel("Initial Vertical Stretch: ", this);
        mpInitScaleYLineEdits.append(new QLineEdit(this));
        mpInitScaleYLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pInitScaleYLabel,             row, 0);
        pScrollLayout->addWidget(mpInitScaleYLineEdits.last(), row, 1);
        ++row;

        //Resize X
        QLabel *pResizeXLabel = new QLabel("Horizontal Stretch: ", this);
        mpResizeXLineEdits.append(new QLineEdit(this));
        mpResizeXLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pResizeXLabel,             row, 0);
        pScrollLayout->addWidget(mpResizeXLineEdits.last(), row, 1);
        ++row;

        //Resize Y
        QLabel *pResizeYLabel = new QLabel("Vertical Stretch: ", this);
        mpResizeYLineEdits.append(new QLineEdit(this));
        mpResizeYLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pResizeYLabel,             row, 0);
        pScrollLayout->addWidget(mpResizeYLineEdits.last(), row, 1);
        ++row;

        //Resize Data Index 1
        QLabel *pScaleData1Label = new QLabel("Resize Data Index 1: ", this);
        mpResizeDataIdx1LineEdits.append(new QLineEdit(this));
        mpResizeDataIdx1LineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pScaleData1Label,               row, 0);
        pScrollLayout->addWidget(mpResizeDataIdx1LineEdits.last(), row, 1);
        ++row;

        //Resize Data Index 2
        QLabel *pScaleData2Label = new QLabel("Resize Data Index 2: ", this);
        mpResizeDataIdx2LineEdits.append(new QLineEdit(this));
        mpResizeDataIdx2LineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pScaleData2Label,               row, 0);
        pScrollLayout->addWidget(mpResizeDataIdx2LineEdits.last(), row, 1);
        ++row;

        //Movement Multipliers
        QLabel *pResizeMultipliersLabel = new QLabel("Resize Multipliers: ", this);
        mpResizeMultipliersLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pResizeMultipliersLabel,       row, 0);
        pScrollLayout->addWidget(mpResizeMultipliersLineEdits.last(), row, 1);
        ++row;

        //Movement Divisors
        QLabel *pResizeDivisorsLabel = new QLabel("Resize Divisors: ", this);
        mpResizeDivisorsLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pResizeDivisorsLabel,       row, 0);
        pScrollLayout->addWidget(mpResizeDivisorsLineEdits.last(), row, 1);
        ++row;

        //Colors
        QLabel *pColorsLabel = new QLabel("Colors");
        pColorsLabel->setFont(boldFont);
        pScrollLayout->addWidget(pColorsLabel, row, 0, 1, 2);
        ++row;

        //Init Color (R,G,B,A)
        QLabel *pInitColorLabel = new QLabel("Initial Color (R,G,B,A): ", this);
        mpInitColorLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pInitColorLabel,             row, 0);
        pScrollLayout->addWidget(mpInitColorLineEdits.last(), row, 1);
        ++row;

        //Color Modifiers (R,G,B,A)
        QLabel *pColorModifiersLabel = new QLabel("Color Modifiers (R,G,B,A): ", this);
        mpColorModifiersLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pColorModifiersLabel,             row, 0);
        pScrollLayout->addWidget(mpColorModifiersLineEdits.last(), row, 1);
        ++row;

        //Color Data Index
        QLabel *pColorDataIdxLabel = new QLabel("Color Data Index: ", this);
        mpColorDataIdxLineEdits.append(new QLineEdit(this));
        mpColorDataIdxLineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pColorDataIdxLabel,               row, 0);
        pScrollLayout->addWidget(mpColorDataIdxLineEdits.last(), row, 1);
        ++row;

        //Transform Origin
        QLabel *pTransformOriginLabel = new QLabel("Transform Origin");
        pTransformOriginLabel->setFont(boldFont);
        pScrollLayout->addWidget(pTransformOriginLabel, row, 0, 1, 2);
        ++row;

        //Transform Origin X
        QLabel *pTransformOriginXLabel = new QLabel("Transform Origin X: ", this);
        mpTransformOriginXLineEdits.append(new QLineEdit(this));
        mpTransformOriginXLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pTransformOriginXLabel,             row, 0);
        pScrollLayout->addWidget(mpTransformOriginXLineEdits.last(), row, 1);
        ++row;

        //Transform Origin Y
        QLabel *pTransformOriginYLabel = new QLabel("Transform Origin Y: ", this);
        mpTransformOriginYLineEdits.append(new QLineEdit(this));
        mpTransformOriginYLineEdits.last()->setValidator(new QDoubleValidator(this));
        pScrollLayout->addWidget(pTransformOriginYLabel,             row, 0);
        pScrollLayout->addWidget(mpTransformOriginYLineEdits.last(), row, 1);
        ++row;

        //Relative Movable
        QLabel *pRelativeMovableLabel = new QLabel("Relative Movable");
        pRelativeMovableLabel->setFont(boldFont);
        pScrollLayout->addWidget(pRelativeMovableLabel, row, 0, 1, 2);
        ++row;

        //Relative Movable Index
        QLabel *pRelativeMovableIdxLabel = new QLabel("Movable Index: ", this);
        mpMovableRelativeLineEdits.append(new QLineEdit(this));
        mpMovableRelativeLineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pRelativeMovableIdxLabel,          row, 0);
        pScrollLayout->addWidget(mpMovableRelativeLineEdits.last(), row, 1);
        ++row;

        //Movable Ports
        QLabel *pMovablePortsLabel = new QLabel("Movable Ports");
        pMovablePortsLabel->setFont(boldFont);
        pScrollLayout->addWidget(pMovablePortsLabel, row, 0, 1, 2);
        ++row;

        //Movable Port Names
        QLabel *pMovablePortNamesLabel = new QLabel("Port Names: ", this);
        mpMovablePortNamesLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pMovablePortNamesLabel,          row, 0);
        pScrollLayout->addWidget(mpMovablePortNamesLineEdits.last(), row, 1);
        ++row;

        //Movable Port Start X
        QLabel *pMovablePortStartXLabel = new QLabel("Start Position X: ", this);
        mpMovablePortStartXLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pMovablePortStartXLabel,          row, 0);
        pScrollLayout->addWidget(mpMovablePortStartXLineEdits.last(), row, 1);
        ++row;

        //Movable Port Start Y
        QLabel *pMovablePortStartYLabel = new QLabel("Start Position Y: ", this);
        mpMovablePortStartYLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pMovablePortStartYLabel,          row, 0);
        pScrollLayout->addWidget(mpMovablePortStartYLineEdits.last(), row, 1);
        ++row;

        pScrollWidget->setLayout(pScrollLayout);
        pScrollArea->setWidget(pScrollWidget);
        pMovableLayout->addWidget(pScrollArea);
        pMovableWidget->setLayout(pMovableLayout);
        pScrollArea->setWidgetResizable(true);

        pScrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        pTabWidget->addTab(pMovableWidget, "Movable "+QString::number(i));
    }
    pTabWidget->setCurrentIndex(index);

    //Buttons
    QPushButton *pOkButton = new QPushButton("Ok", this);
    pOkButton->setDefault(true);
    QPushButton *pResetButton = new QPushButton("Reset Default Values", this);
    QPushButton *pCancelButton = new QPushButton("Cancel", this);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(this);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::AcceptRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);
    pButtonBox->addButton(pResetButton, QDialogButtonBox::ResetRole);

    //Layout
    QVBoxLayout *pDialogLayout = new QVBoxLayout(this);
    pDialogLayout->addWidget(pTabWidget);
    pDialogLayout->addWidget(pButtonBox);

    this->setLayout(pDialogLayout);

    connect(pOkButton,      SIGNAL(pressed()), this, SLOT(setValues()));
    connect(pResetButton,   SIGNAL(pressed()), this, SLOT(resetValues()));
    connect(pCancelButton,  SIGNAL(pressed()), this, SLOT(reject()));

    updateValues();
}

void AnimatedIconPropertiesDialog::updateValues()
{
    for(int i=0; i<mpData->movables.size(); ++i)
    {
        QString tempStr;
        for(int p=0; p<mpData->movables[i].dataPorts.size(); ++p)
        {
            tempStr.append(mpData->movables[i].dataPorts[p]+",");
        }
        tempStr.chop(1);
        mpDataPortsLineEdits[i]->setText(tempStr);
        tempStr.clear();
        for(int p=0; p<mpData->movables[i].dataNames.size(); ++p)
        {
            tempStr.append(mpData->movables[i].dataNames[p]+",");
        }
        tempStr.chop(1);
        mpDataNamesLineEdits[i]->setText(tempStr);

        mpStartXLineEdits[i]->setText(QString::number(mpData->movables[i].startX));
        mpStartYLineEdits[i]->setText(QString::number(mpData->movables[i].startY));
        mpStartThetaLineEdits[i]->setText(QString::number(mpData->movables[i].startTheta));

        QStringList tempStr1,tempStr2,tempStr3,tempStr4;
        for(const ModelObjectAnimationMovementData &movement : mpData->movables[i].movementData) {
            tempStr1.append(QString::number(movement.x));
            tempStr2.append(QString::number(movement.y));
            tempStr3.append(QString::number(movement.theta));
            tempStr4.append(QString::number(movement.dataIdx));
            mpMovementMultipliersLineEdits[i]->setText(movement.multiplier);
            mpMovementDivisorsLineEdits[i]->setText(movement.divisor);
        }

        mpMovementXLineEdits[i]->setText(tempStr1.join(","));
        mpMovementYLineEdits[i]->setText(tempStr2.join(","));
        mpMovementThetaLineEdits[i]->setText(tempStr3.join(","));
        mpMovementDataIdxLineEdits[i]->setText(tempStr4.join(","));


        mpInitScaleXLineEdits[i]->setText(QString::number(mpData->movables[i].initScaleX));
        mpInitScaleYLineEdits[i]->setText(QString::number(mpData->movables[i].initScaleY));

        tempStr1.clear();
        tempStr2.clear();
        tempStr3.clear();
        tempStr4.clear();
        for(const ModelObjectAnimationResizeData &resize : mpData->movables[i].resizeData) {
            tempStr1.append(QString::number(resize.x));
            tempStr2.append(QString::number(resize.y));
            tempStr3.append(QString::number(resize.dataIdx1));
            tempStr4.append(QString::number(resize.dataIdx2));
            mpResizeMultipliersLineEdits[i]->setText(resize.multiplier);
            mpResizeDivisorsLineEdits[i]->setText(resize.divisor);
        }
        mpResizeXLineEdits[i]->setText(tempStr1.join(","));
        mpResizeYLineEdits[i]->setText(tempStr2.join(","));
        mpResizeDataIdx1LineEdits[i]->setText(tempStr3.join(","));
        mpResizeDataIdx2LineEdits[i]->setText(tempStr4.join(","));

        QString r = QString::number(mpData->movables[i].colorData.initR);
        QString g = QString::number(mpData->movables[i].colorData.initG);
        QString b = QString::number(mpData->movables[i].colorData.initB);
        QString a = QString::number(mpData->movables[i].colorData.initA);
        mpInitColorLineEdits[i]->setText(r+","+g+","+b+","+a);
        r = QString::number(mpData->movables[i].colorData.r);
        g = QString::number(mpData->movables[i].colorData.g);
        b = QString::number(mpData->movables[i].colorData.b);
        a = QString::number(mpData->movables[i].colorData.a);
        mpColorModifiersLineEdits[i]->setText(r+","+g+","+b+","+a);
        mpColorDataIdxLineEdits[i]->setText(QString::number(mpData->movables[i].colorData.dataIdx));
        mpTransformOriginXLineEdits[i]->setText(QString::number(mpData->movables[i].transformOriginX));
        mpTransformOriginYLineEdits[i]->setText(QString::number(mpData->movables[i].transformOriginY));
        mpMovableRelativeLineEdits[i]->setText(QString::number(mpData->movables[i].movableRelative));
        tempStr.clear();
        for(int m=0; m<mpData->movables[i].movablePortNames.size(); ++m)
        {
            tempStr.append(mpData->movables[i].movablePortNames[m]+",");
        }
        tempStr.chop(1);
        mpMovablePortNamesLineEdits[i]->setText(tempStr);
        tempStr.clear();
        for(int m=0; m<mpData->movables[i].movablePortStartX.size(); ++m)
        {
            tempStr.append(QString::number(mpData->movables[i].movablePortStartX[m])+",");
        }
        tempStr.chop(1);
        mpMovablePortStartXLineEdits[i]->setText(tempStr);
        tempStr.clear();
        for(int m=0; m<mpData->movables[i].movablePortStartY.size(); ++m)
        {
            tempStr.append(QString::number(mpData->movables[i].movablePortStartY[m])+",");
        }
        tempStr.chop(1);
        mpMovablePortStartYLineEdits[i]->setText(tempStr);
    }
}


//! @brief Write back new values to the animated component
void AnimatedIconPropertiesDialog::setValues()
{
    //! todo Store new values in container object and save to HMF

    for(int i=0; i<mpData->movables.size(); ++i)
    {
        ModelObjectAnimationMovableData &m = mpData->movables[i];

        QStringList portsSplit = mpDataPortsLineEdits[i]->text().split(",");
        QStringList namesSplit = mpDataNamesLineEdits[i]->text().split(",");
        if(portsSplit.size() != namesSplit.size())
        {
            gpMessageHandler->addErrorMessage("Number of port names does not match number of data names.");
            gpMessageHandler->addInfoMessage("Ignoring data source fields.");
        }
        else
        {
            m.dataPorts = portsSplit;
            m.dataNames = namesSplit;
        }

        m.startX = mpStartXLineEdits[i]->text().toDouble();
        m.startY = mpStartYLineEdits[i]->text().toDouble();
        m.startTheta = mpStartThetaLineEdits[i]->text().toDouble();

        QStringList tempStr1 = mpMovementXLineEdits[i]->text().split(",");
        QStringList tempStr2 = mpMovementYLineEdits[i]->text().split(",");
        QStringList tempStr3 = mpMovementThetaLineEdits[i]->text().split(",");
        QStringList tempStr4 = mpMovementDataIdxLineEdits[i]->text().split(",");

        m.movementData.clear();
        for(int r=0; r<tempStr1.size(); ++r)
        {
            ModelObjectAnimationMovementData movement(tempStr1[r].toDouble(), tempStr2[r].toDouble(), tempStr3[r].toDouble(), tempStr4[r].toInt(),
                                                      mpMovementDivisorsLineEdits[i]->text(), mpMovementMultipliersLineEdits[i]->text());
            m.movementData.append(movement);
        }

        m.initScaleX = mpInitScaleXLineEdits[i]->text().toDouble();
        m.initScaleY = mpInitScaleYLineEdits[i]->text().toDouble();

        tempStr1 = mpResizeXLineEdits[i]->text().split(",");
        tempStr2 = mpResizeYLineEdits[i]->text().split(",");
        tempStr3 = mpResizeDataIdx1LineEdits[i]->text().split(",");
        tempStr4 = mpResizeDataIdx2LineEdits[i]->text().split(",");

        m.resizeData.clear();
        for(int r=0; r<tempStr1.size(); ++r)
        {
            ModelObjectAnimationResizeData resize(tempStr1[r].toDouble(), tempStr2[r].toDouble(), tempStr3[r].toInt(), tempStr4[r].toInt(),
                                                  mpResizeDivisorsLineEdits[i]->text(), mpResizeMultipliersLineEdits[i]->text());
            m.resizeData.append(resize);
        }

        QStringList splitInitColor = mpInitColorLineEdits[i]->text().split(",");
        if(splitInitColor.size() == 4)
        {
            m.colorData.initR = splitInitColor[0].toDouble();
            m.colorData.initG = splitInitColor[1].toDouble();
            m.colorData.initB = splitInitColor[2].toDouble();
            m.colorData.initA = splitInitColor[3].toDouble();
        }
        else if(splitInitColor.size() == 3)
        {
            m.colorData.initR = splitInitColor[0].toDouble();
            m.colorData.initG = splitInitColor[1].toDouble();
            m.colorData.initB = splitInitColor[2].toDouble();
            m.colorData.initA = 255;
        }
        else if(splitInitColor.size() > 0)
        {
            gpMessageHandler->addErrorMessage("Initial color field must contain 3 or 4 values.");
            gpMessageHandler->addInfoMessage("Ignoring initial colors.");
        }

        QStringList splitColors = mpColorModifiersLineEdits[i]->text().split(",");
        if(splitColors.size() == 4)
        {
            m.colorData.r = splitColors[0].toDouble();
            m.colorData.g = splitColors[1].toDouble();
            m.colorData.b = splitColors[2].toDouble();
            m.colorData.a = splitColors[3].toDouble();
        }
        else if(splitColors.size() == 3)
        {
            m.colorData.r = splitColors[0].toDouble();
            m.colorData.g = splitColors[1].toDouble();
            m.colorData.b = splitColors[2].toDouble();
            m.colorData.a = 255;
        }
        else if(splitColors.size() > 0)
        {
            gpMessageHandler->addErrorMessage("Color modifiers field must contain 3 or 4 values.");
            gpMessageHandler->addInfoMessage("Ignoring colors modifiers.");
        }

        m.colorData.dataIdx = mpColorDataIdxLineEdits[i]->text().toInt();

        m.transformOriginX = mpTransformOriginXLineEdits[i]->text().toDouble();
        m.transformOriginY = mpTransformOriginYLineEdits[i]->text().toDouble();

        m.movableRelative = mpMovableRelativeLineEdits[i]->text().toInt();

        QStringList splitMovablePortNames = mpMovablePortNamesLineEdits[i]->text().split(",");
        QStringList splitMovablePortStartX = mpMovablePortStartXLineEdits[i]->text().split(",");
        QStringList splitMovablePortStartY = mpMovablePortStartYLineEdits[i]->text().split(",");
        if(splitMovablePortNames.size() == splitMovablePortStartX.size() && splitMovablePortNames.size() == splitMovablePortStartY.size())
        {
            m.movablePortNames.clear();
            m.movablePortStartX.clear();
            m.movablePortStartY.clear();
            for(int p=0; p<splitMovablePortNames.size(); ++p)
            {
                m.movablePortNames.append(splitMovablePortNames[p]);
                m.movablePortStartX.append(splitMovablePortStartX[p].toDouble());
                m.movablePortStartY.append(splitMovablePortStartY[p].toDouble());
            }
        }
        else
        {
            gpMessageHandler->addErrorMessage("Number of movable port names does not match number of initial positions.");
            gpMessageHandler->addInfoMessage("Ignoring movable port fields.");
        }
    }

    this->accept();
}

void AnimatedIconPropertiesDialog::resetValues()
{
    QDomDocument domDocument;
    QDomElement animationRoot = domDocument.createElement(hmf::animation);
    domDocument.appendChild(animationRoot);

    QString subTypeName = mpAnimatedComponent->mpModelObject->getSubTypeName();
    QString typeName = mpAnimatedComponent->mpModelObject->getTypeName();
    SharedModelObjectAppearanceT pAppearanceData = gpLibraryHandler->getModelObjectAppearancePtr(typeName, subTypeName);
    pAppearanceData->getAnimationDataPtr()->saveToDomElement(animationRoot);
    QString baseIconPath = gpLibraryHandler->getModelObjectAppearancePtr(mpAnimatedComponent->mpModelObject->getTypeName())->getAnimationDataPtr()->baseIconPath;

    //Store icon paths (they are not included in saveToDomElement() )
    QStringList iconPaths;
    for(const ModelObjectAnimationMovableData &m : mpData->movables) {
        iconPaths << m.iconPath;
    }

    //! @todo Maybe more things are not included in saveToDomElement(), make sure they are added here...

    mpData->movables.clear();
    mpData->readFromDomElement(animationRoot,baseIconPath);

    //Restore icon paths
    for(int m=0; m<iconPaths.size(); ++m)
    {
        mpData->movables[m].iconPath = iconPaths[m];
    }

    updateValues();
}
