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

        //Multipliers and Divisors
        QLabel *pMultipliersAndDivisorsLabel = new QLabel("Multipliers and Divisors");
        pMultipliersAndDivisorsLabel->setFont(boldFont);
        pScrollLayout->addWidget(pMultipliersAndDivisorsLabel, row, 0, 1, 2);
        ++row;

        //Multipliers
        QLabel *pMultipliersLabel = new QLabel("Multipliers: ", this);
        mpMultipliersLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pMultipliersLabel,             row, 0);
        pScrollLayout->addWidget(mpMultipliersLineEdits.last(), row, 1);
        ++row;

        //Divisors
        QLabel *pDivisorsLabel = new QLabel("Divisors: ", this);
        mpDivisorsLineEdits.append(new QLineEdit(this));
        pScrollLayout->addWidget(pDivisorsLabel,             row, 0);
        pScrollLayout->addWidget(mpDivisorsLineEdits.last(), row, 1);
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

        //Scale Data Index 1
        QLabel *pScaleData1Label = new QLabel("Scale Data Index 1: ", this);
        mpScaleDataIdx1LineEdits.append(new QLineEdit(this));
        mpScaleDataIdx1LineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pScaleData1Label,               row, 0);
        pScrollLayout->addWidget(mpScaleDataIdx1LineEdits.last(), row, 1);
        ++row;

        //Scale Data Index 2
        QLabel *pScaleData2Label = new QLabel("Scale Data Index 2: ", this);
        mpScaleDataIdx2LineEdits.append(new QLineEdit(this));
        mpScaleDataIdx2LineEdits.last()->setValidator(new QIntValidator(this));
        pScrollLayout->addWidget(pScaleData2Label,               row, 0);
        pScrollLayout->addWidget(mpScaleDataIdx2LineEdits.last(), row, 1);
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
        tempStr.clear();
        for(int m=0; m<mpData->movables[i].multipliers.size(); ++m)
        {
            tempStr.append(mpData->movables[i].multipliers[m]+",");
        }
        tempStr.chop(1);
        mpMultipliersLineEdits[i]->setText(tempStr);
        tempStr.clear();
        for(int m=0; m<mpData->movables[i].divisors.size(); ++m)
        {
            tempStr.append(mpData->movables[i].divisors[m]+",");
        }
        tempStr.chop(1);
        mpDivisorsLineEdits[i]->setText(tempStr);
        mpStartXLineEdits[i]->setText(QString::number(mpData->movables[i].startX));
        mpStartYLineEdits[i]->setText(QString::number(mpData->movables[i].startY));
        mpStartThetaLineEdits[i]->setText(QString::number(mpData->movables[i].startTheta));

        QStringList tempStr1,tempStr2,tempStr3,tempStr4;
        for(int r=0; r<mpData->movables[i].movementX.size(); ++r)
        {
            tempStr1.append(QString::number(mpData->movables[i].movementX[r]));
            tempStr2.append(QString::number(mpData->movables[i].movementY[r]));
            tempStr3.append(QString::number(mpData->movables[i].movementTheta[r]));
            tempStr4.append(QString::number(mpData->movables[i].movementDataIdx[r]));
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
        for(int r=0; r<mpData->movables[i].resizeX.size(); ++r)
        {
            tempStr1.append(QString::number(mpData->movables[i].resizeX[r]));
            tempStr2.append(QString::number(mpData->movables[i].resizeY[r]));
            tempStr3.append(QString::number(mpData->movables[i].scaleDataIdx1[r]));
            tempStr4.append(QString::number(mpData->movables[i].scaleDataIdx2[r]));
        }
        mpResizeXLineEdits[i]->setText(tempStr1.join(","));
        mpResizeYLineEdits[i]->setText(tempStr2.join(","));
        mpScaleDataIdx1LineEdits[i]->setText(tempStr3.join(","));
        mpScaleDataIdx2LineEdits[i]->setText(tempStr4.join(","));

        QString r = QString::number(mpData->movables[i].initColorR);
        QString g = QString::number(mpData->movables[i].initColorG);
        QString b = QString::number(mpData->movables[i].initColorB);
        QString a = QString::number(mpData->movables[i].initColorA);
        mpInitColorLineEdits[i]->setText(r+","+g+","+b+","+a);
        r = QString::number(mpData->movables[i].colorR);
        g = QString::number(mpData->movables[i].colorG);
        b = QString::number(mpData->movables[i].colorB);
        a = QString::number(mpData->movables[i].colorA);
        mpColorModifiersLineEdits[i]->setText(r+","+g+","+b+","+a);
        mpColorDataIdxLineEdits[i]->setText(QString::number(mpData->movables[i].colorDataIdx));
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

        m.multipliers = mpMultipliersLineEdits[i]->text().split(",");   //! @todo Check that variable exists
        m.divisors = mpDivisorsLineEdits[i]->text().split(",");   //! @todo Check that variable exists

        m.startX = mpStartXLineEdits[i]->text().toDouble();
        m.startY = mpStartYLineEdits[i]->text().toDouble();
        m.startTheta = mpStartThetaLineEdits[i]->text().toDouble();

        QStringList tempStr1 = mpMovementXLineEdits[i]->text().split(",");
        QStringList tempStr2 = mpMovementYLineEdits[i]->text().split(",");
        QStringList tempStr3 = mpMovementThetaLineEdits[i]->text().split(",");
        QStringList tempStr4 = mpMovementDataIdxLineEdits[i]->text().split(",");

        m.movementX.clear();
        m.movementY.clear();
        m.movementTheta.clear();
        m.movementDataIdx.clear();
        for(int r=0; r<tempStr1.size(); ++r)
        {
            m.movementX.append(tempStr1[r].toDouble());
            m.movementY.append(tempStr2[r].toDouble());
            m.movementTheta.append(tempStr3[r].toDouble());
            m.movementDataIdx.append(tempStr4[r].toInt());
        }

        m.initScaleX = mpInitScaleXLineEdits[i]->text().toDouble();
        m.initScaleY = mpInitScaleYLineEdits[i]->text().toDouble();

        tempStr1 = mpResizeXLineEdits[i]->text().split(",");
        tempStr2 = mpResizeYLineEdits[i]->text().split(",");
        tempStr3 = mpScaleDataIdx1LineEdits[i]->text().split(",");
        tempStr4 = mpScaleDataIdx2LineEdits[i]->text().split(",");

        m.resizeX.clear();
        m.resizeY.clear();
        m.scaleDataIdx1.clear();
        m.scaleDataIdx2.clear();
        for(int r=0; r<tempStr1.size(); ++r)
        {
            m.resizeX.append(tempStr1[r].toDouble());
            m.resizeY.append(tempStr2[r].toDouble());
            m.scaleDataIdx1.append(tempStr3[r].toInt());
            m.scaleDataIdx2.append(tempStr4[r].toInt());
        }

        QStringList splitInitColor = mpInitColorLineEdits[i]->text().split(",");
        if(splitInitColor.size() == 4)
        {
            m.initColorR = splitInitColor[0].toDouble();
            m.initColorG = splitInitColor[1].toDouble();
            m.initColorB = splitInitColor[2].toDouble();
            m.initColorA = splitInitColor[3].toDouble();
        }
        else if(splitInitColor.size() == 3)
        {
            m.initColorR = splitInitColor[0].toDouble();
            m.initColorG = splitInitColor[1].toDouble();
            m.initColorB = splitInitColor[2].toDouble();
            m.initColorA = 255;
        }
        else if(splitInitColor.size() > 0)
        {
            gpMessageHandler->addErrorMessage("Initial color field must contain 3 or 4 values.");
            gpMessageHandler->addInfoMessage("Ignoring initial colors.");
        }

        QStringList splitColors = mpColorModifiersLineEdits[i]->text().split(",");
        if(splitColors.size() == 4)
        {
            m.colorR = splitColors[0].toDouble();
            m.colorG = splitColors[1].toDouble();
            m.colorB = splitColors[2].toDouble();
            m.colorA = splitColors[3].toDouble();
        }
        else if(splitColors.size() == 3)
        {
            m.colorR = splitColors[0].toDouble();
            m.colorG = splitColors[1].toDouble();
            m.colorB = splitColors[2].toDouble();
            m.colorA = 255;
        }
        else if(splitColors.size() > 0)
        {
            gpMessageHandler->addErrorMessage("Color modifiers field must contain 3 or 4 values.");
            gpMessageHandler->addInfoMessage("Ignoring colors modifiers.");
        }

        m.colorDataIdx = mpColorDataIdxLineEdits[i]->text().toInt();

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
    QDomElement animationRoot = domDocument.createElement("animation");
    domDocument.appendChild(animationRoot);

    gpLibraryHandler->getModelObjectAppearancePtr(mpAnimatedComponent->mpModelObject->getTypeName())->getAnimationDataPtr()->saveToDomElement(animationRoot);
    QString baseIconPath = gpLibraryHandler->getModelObjectAppearancePtr(mpAnimatedComponent->mpModelObject->getTypeName())->getAnimationDataPtr()->baseIconPath;

    //Store icon paths (they are not included in saveToDomElement() )
    QStringList iconPaths;
    foreach(const ModelObjectAnimationMovableData &m, mpData->movables)
    {
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
