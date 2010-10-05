/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   OptionsWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id: OptionsWidget.cpp 1196 2010-04-01 09:55:04Z robbr48 $


#include <QtGui>
#include <QDebug>

#include "OptionsWidget.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GraphicsView.h"

class ProjectTabWidget;


//! Constructor for the options dialog
//! @param parent Pointer to the main window
OptionsWidget::OptionsWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;

        //Set the name and size of the main window
    this->setObjectName("OptionsWidget");
    this->resize(640,480);
    this->setWindowTitle("Options");

        //Interface Options
    mpBackgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
    mpBackgroundColorButton = new QToolButton();
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(mpParentMainWindow->mBackgroundColor.red());
    greenString.setNum(mpParentMainWindow->mBackgroundColor.green());
    blueString.setNum(mpParentMainWindow->mBackgroundColor.blue());
    mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mpBackgroundColorButton->setAutoRaise(true);

    mpAntiAliasingCheckBox = new QCheckBox(tr("Use Anti-Aliasing"));
    mpAntiAliasingCheckBox->setCheckable(true);
    mpAntiAliasingCheckBox->setChecked(mpParentMainWindow->mAntiAliasing);

    mpInvertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    mpInvertWheelCheckBox->setCheckable(true);
    mpInvertWheelCheckBox->setChecked(mpParentMainWindow->mInvertWheel);

    mpSnappingCheckBox = new QCheckBox(tr("Auto Snap Components"));
    mpSnappingCheckBox->setCheckable(true);
    mpSnappingCheckBox->setChecked(mpParentMainWindow->mSnapping);

    mpInterfaceGroupBox = new QGroupBox(tr("Interface"));
    mpInterfaceLayout = new QGridLayout;
    mpInterfaceLayout->addWidget(mpInvertWheelCheckBox, 0, 0);
    mpInterfaceLayout->addWidget(mpAntiAliasingCheckBox, 1, 0);
    mpInterfaceLayout->addWidget(mpSnappingCheckBox, 2, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorLabel, 3, 0);
    mpInterfaceLayout->addWidget(mpBackgroundColorButton, 3, 1);
    mpInterfaceGroupBox->setLayout(mpInterfaceLayout);

        //Simulation Options
    mpEnableProgressBarCheckBox = new QCheckBox(tr("Enable Simulation Progress Bar"));
    mpEnableProgressBarCheckBox->setCheckable(true);
    mpEnableProgressBarCheckBox->setChecked(mpParentMainWindow->mEnableProgressBar);

    mpProgressBarLabel = new QLabel(tr("Progress Bar Time Step [ms]"));
    mpProgressBarLabel->setEnabled(mpParentMainWindow->mEnableProgressBar);
    mpProgressBarSpinBox = new QSpinBox();
    mpProgressBarSpinBox->setMinimum(1);
    mpProgressBarSpinBox->setMaximum(5000);
    mpProgressBarSpinBox->setSingleStep(10);
    mpProgressBarSpinBox->setValue(mpParentMainWindow->mProgressBarStep);
    mpProgressBarSpinBox->setEnabled(mpParentMainWindow->mEnableProgressBar);

    mpUseMulticoreCheckBox = new QCheckBox(tr("Use Multi-Threaded Simulation"));
    mpUseMulticoreCheckBox->setCheckable(true);
    mpUseMulticoreCheckBox->setChecked(mpParentMainWindow->mUseMulticore);

    mpSimulationGroupBox = new QGroupBox(tr("Simulation"));
    mpSimulationLayout = new QGridLayout;
    mpSimulationLayout->addWidget(mpEnableProgressBarCheckBox, 0, 0);
    mpSimulationLayout->addWidget(mpProgressBarLabel, 1, 0);
    mpSimulationLayout->addWidget(mpProgressBarSpinBox, 1, 1);
    mpSimulationLayout->addWidget(mpUseMulticoreCheckBox, 2, 0, 1, 2);
    mpSimulationGroupBox->setLayout(mpSimulationLayout);

    mpCancelButton = new QPushButton(tr("&Cancel"));
    mpCancelButton->setAutoDefault(false);
    mpOkButton = new QPushButton(tr("&Done"));
    mpOkButton->setAutoDefault(true);

    mpButtonBox = new QDialogButtonBox(Qt::Horizontal);
    mpButtonBox->addButton(mpCancelButton, QDialogButtonBox::ActionRole);
    mpButtonBox->addButton(mpOkButton, QDialogButtonBox::ActionRole);

    connect(mpParentMainWindow->optionsAction,SIGNAL(triggered()),this,SLOT(show()));
    connect(mpEnableProgressBarCheckBox,SIGNAL(toggled(bool)), mpProgressBarLabel, SLOT(setEnabled(bool)));
    connect(mpEnableProgressBarCheckBox,SIGNAL(toggled(bool)), mpProgressBarSpinBox, SLOT(setEnabled(bool)));
    connect(mpBackgroundColorButton, SIGNAL(pressed()), this, SLOT(colorDialog()));
    connect(mpCancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(mpOkButton, SIGNAL(pressed()), this, SLOT(updateValues()));

    QGridLayout *pLayout = new QGridLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpInterfaceGroupBox);
    pLayout->addWidget(mpSimulationGroupBox);
    pLayout->addWidget(mpButtonBox, 4, 0);
    setLayout(pLayout);
}


//! Slot that updates and saves the settings based on the choices made in the dialog box
void OptionsWidget::updateValues()
{
    mpParentMainWindow->mInvertWheel = mpInvertWheelCheckBox->isChecked();
    mpParentMainWindow->mAntiAliasing = mpAntiAliasingCheckBox->isChecked();
    mpParentMainWindow->mSnapping = mpSnappingCheckBox->isChecked();
    for(size_t i=0; i<mpParentMainWindow->mpProjectTabs->count(); ++i)
    {
        mpParentMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->setRenderHint(QPainter::Antialiasing, mpParentMainWindow->mAntiAliasing);
    }
    mpParentMainWindow->mBackgroundColor = mPickedBackgroundColor;
    for(size_t i=0; i<mpParentMainWindow->mpProjectTabs->count(); ++i)
    {
        mpParentMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->updateViewPort();
    }
    mpParentMainWindow->mEnableProgressBar = mpEnableProgressBarCheckBox->isChecked();
    mpParentMainWindow->mProgressBarStep = mpProgressBarSpinBox->value();
    mpParentMainWindow->mUseMulticore = mpUseMulticoreCheckBox->isChecked();
    mpParentMainWindow->saveSettings();
    this->accept();
}


//! Slot that opens a color dialog where user can select a background color
void OptionsWidget::colorDialog()
{
    mPickedBackgroundColor = QColorDialog::getColor(mpParentMainWindow->mBackgroundColor, this);
    if (mPickedBackgroundColor.isValid())
    {
        QString redString;
        QString greenString;
        QString blueString;
        redString.setNum(mPickedBackgroundColor.red());
        greenString.setNum(mPickedBackgroundColor.green());
        blueString.setNum(mPickedBackgroundColor.blue());
        mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
        mpBackgroundColorButton->setDown(false);
    }
    else
    {
        mPickedBackgroundColor = mpParentMainWindow->mBackgroundColor;
    }
}


//! Reimplementation of show() slot. This is used to make sure that the background color button resets its color if the cancel button was pressed last time options were opened.
void OptionsWidget::show()
{
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(mpParentMainWindow->mBackgroundColor.red());
    greenString.setNum(mpParentMainWindow->mBackgroundColor.green());
    blueString.setNum(mpParentMainWindow->mBackgroundColor.blue());
    mpBackgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    mPickedBackgroundColor = mpParentMainWindow->mBackgroundColor;

    QDialog::show();
}
