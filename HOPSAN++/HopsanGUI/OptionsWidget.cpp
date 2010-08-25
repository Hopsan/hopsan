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

//$Id: OptionsWidget.cpp 1196 2010-04-01 09:55:04Z robbr48 $


#include <QtGui>
#include <QDebug>

#include "OptionsWidget.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GraphicsView.h"

class ProjectTabWidget;

OptionsWidget::OptionsWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;

        //Set the name and size of the main window

    this->setObjectName("OptionsWidget");
    this->resize(640,480);
    this->setWindowTitle("Options");

        //Interface Options


    backgroundColorLabel = new QLabel(tr("Work Area Background Color:"));
    backgroundColorButton = new QToolButton();
    QString redString;
    QString greenString;
    QString blueString;
    redString.setNum(mpParentMainWindow->mBackgroundColor.red());
    greenString.setNum(mpParentMainWindow->mBackgroundColor.green());
    blueString.setNum(mpParentMainWindow->mBackgroundColor.blue());
    backgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
    backgroundColorButton->setAutoRaise(true);

    antiAliasingCheckBox = new QCheckBox(tr("Use Anti-Aliasing"));
    antiAliasingCheckBox->setCheckable(true);
    antiAliasingCheckBox->setChecked(mpParentMainWindow->mAntiAliasing);

    invertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    invertWheelCheckBox->setCheckable(true);
    invertWheelCheckBox->setChecked(mpParentMainWindow->mInvertWheel);

    interfaceGroupBox = new QGroupBox(tr("Interface"));
    interfaceLayout = new QGridLayout;
    interfaceLayout->addWidget(invertWheelCheckBox, 0, 0);
    interfaceLayout->addWidget(antiAliasingCheckBox, 1, 0);
    interfaceLayout->addWidget(backgroundColorLabel, 2, 0);
    interfaceLayout->addWidget(backgroundColorButton, 2, 1);
    interfaceGroupBox->setLayout(interfaceLayout);

        //Simulation Options

    progressBarLabel = new QLabel(tr("Progress Bar Time Step [ms]"));
    progressBarSpinBox = new QSpinBox();
    progressBarSpinBox->setMinimum(1);
    progressBarSpinBox->setMaximum(5000);
    progressBarSpinBox->setSingleStep(10);
    progressBarSpinBox->setValue(mpParentMainWindow->mProgressBarStep);

    useMulticoreCheckBox = new QCheckBox(tr("Use Multi-Threaded Simulation"));
    useMulticoreCheckBox->setCheckable(true);
    useMulticoreCheckBox->setChecked(mpParentMainWindow->mUseMulticore);

    simulationGroupBox = new QGroupBox(tr("Simulation"));
    simulationLayout = new QGridLayout;
    simulationLayout->addWidget(progressBarLabel, 0, 0);
    simulationLayout->addWidget(progressBarSpinBox, 0, 1);
    simulationLayout->addWidget(useMulticoreCheckBox, 2, 0, 1, 2);
    simulationGroupBox->setLayout(simulationLayout);

    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setAutoDefault(false);
    okButton = new QPushButton(tr("&Done"));
    okButton->setAutoDefault(true);

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    connect(backgroundColorButton, SIGNAL(pressed()), this, SLOT(colorDialog()));
    connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(okButton, SIGNAL(pressed()), this, SLOT(updateValues()));


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    //mainLayout->addWidget(invertWheelCheckBox, 0, 0);
    //mainLayout->addWidget(progressBarLabel, 1, 0);
    //mainLayout->addWidget(progressBarSpinBox, 2, 0);
    //mainLayout->addWidget(useMulticoreCheckBox, 3, 0);
    mainLayout->addWidget(interfaceGroupBox);
    mainLayout->addWidget(simulationGroupBox);
    mainLayout->addWidget(buttonBox, 4, 0);
    setLayout(mainLayout);
}


void OptionsWidget::updateValues()
{
    mpParentMainWindow->mInvertWheel = invertWheelCheckBox->isChecked();
    mpParentMainWindow->mAntiAliasing = antiAliasingCheckBox->isChecked();
    for(size_t i=0; i<mpParentMainWindow->mpProjectTabs->count(); ++i)
    {
        mpParentMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->setRenderHint(QPainter::Antialiasing, mpParentMainWindow->mAntiAliasing);
    }
    mpParentMainWindow->mProgressBarStep = progressBarSpinBox->value();
    mpParentMainWindow->mUseMulticore = useMulticoreCheckBox->isChecked();
    mpParentMainWindow->saveSettings();
    this->accept();
}


void OptionsWidget::colorDialog()
{
    QColor color = QColorDialog::getColor(mpParentMainWindow->mBackgroundColor, this);
    if (color.isValid())
    {
        mpParentMainWindow->mBackgroundColor = color;
        for(size_t i=0; i<mpParentMainWindow->mpProjectTabs->count(); ++i)
        {
            mpParentMainWindow->mpProjectTabs->getTab(i)->mpGraphicsView->resetBackgroundBrush();
        }
        QString redString;
        QString greenString;
        QString blueString;
        redString.setNum(mpParentMainWindow->mBackgroundColor.red());
        greenString.setNum(mpParentMainWindow->mBackgroundColor.green());
        blueString.setNum(mpParentMainWindow->mBackgroundColor.blue());
        backgroundColorButton->setStyleSheet(QString("* { background-color: rgb(" + redString + "," + greenString + "," + blueString + ") }"));
        backgroundColorButton->setDown(false);
    }
}
