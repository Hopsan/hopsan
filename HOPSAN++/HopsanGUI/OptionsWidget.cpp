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

class ProjectTabWidget;

OptionsWidget::OptionsWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("OptionsWidget");
    this->resize(640,480);
    this->setWindowTitle("Options");

    invertWheelCheckBox = new QCheckBox(tr("Invert Mouse Wheel"));
    invertWheelCheckBox->setCheckable(true);
    invertWheelCheckBox->setChecked(mpParentMainWindow->mInvertWheel);

    progressBarLabel = new QLabel(tr("Simulation Progress Bar Time Step [ms]"));
    progressBarSpinBox = new QSpinBox();
    progressBarSpinBox->setMinimum(1);
    progressBarSpinBox->setMaximum(5000);
    progressBarSpinBox->setSingleStep(10);
    progressBarSpinBox->setValue(mpParentMainWindow->mProgressBarStep);

    useMulticoreCheckBox = new QCheckBox(tr("Use Multi-Threaded Simulation"));
    useMulticoreCheckBox->setCheckable(true);
    useMulticoreCheckBox->setChecked(mpParentMainWindow->mUseMulticore);

    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setAutoDefault(false);
    okButton = new QPushButton(tr("&Done"));
    okButton->setAutoDefault(true);

    buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    connect(cancelButton, SIGNAL(pressed()), this, SLOT(reject()));
    connect(okButton, SIGNAL(pressed()), this, SLOT(updateValues()));


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(invertWheelCheckBox, 0, 0);
    mainLayout->addWidget(progressBarLabel, 1, 0);
    mainLayout->addWidget(progressBarSpinBox, 2, 0);
    mainLayout->addWidget(useMulticoreCheckBox, 3, 0);
    mainLayout->addWidget(buttonBox, 4, 0);
    setLayout(mainLayout);
}


void OptionsWidget::updateValues()
{
    mpParentMainWindow->mInvertWheel = invertWheelCheckBox->isChecked();
    mpParentMainWindow->mProgressBarStep = progressBarSpinBox->value();
    mpParentMainWindow->mUseMulticore = useMulticoreCheckBox->isChecked();
    mpParentMainWindow->saveSettings();
    this->accept();
}
