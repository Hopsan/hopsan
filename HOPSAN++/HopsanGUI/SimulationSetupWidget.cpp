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
//! @file   simulationsetupwidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-08
//!
//! @brief Contains a class for setting simulation times
//!
//$Id$

#include <iostream>
#include <QtGui>
#include "SimulationSetupWidget.h"
#include "mainwindow.h"
#include "ProjectTabWidget.h"


//! @class SimulationSetupWidget
//! @brief The SimulationSetupWidget class is a Widget used to interact with simulation times.
//!
//! It contains textboxes which the user use to setup simulations with (start, stop and timestep).
//!


//! Constructor.
//! @param title is the title of the group box.
//! @param parent defines a parent to the new instanced object.
SimulationSetupWidget::SimulationSetupWidget(const QString &title, MainWindow *parent)
    : QGroupBox(title, parent)
{
    //mpGroupBox = new QGroupBox(title, this);

    mpParentMainWindow = parent;

    mpSimulationLayout = new QHBoxLayout;
    mpStartTimeLabel = new QLineEdit("0.0");
    mpStartTimeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpStartTimeLabel->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpStartTimeLabel));
    mpTimeStepLabel = new QLineEdit("0.001");
    mpTimeStepLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeStepLabel->setValidator(new QDoubleValidator(0.0, 999.0, 6, mpStartTimeLabel));
    mpFinishTimeLabel = new QLineEdit("10.0");
    mpFinishTimeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpFinishTimeLabel->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpFinishTimeLabel));
    mpTimeLabelDeliminator1 = new QLabel(tr("::"));
    mpTimeLabelDeliminator2 = new QLabel(tr("::"));
    mpSimulateButton = new QPushButton(tr("Simulate!"));

    //mpSimulationLayout->addStretch(1);
    mpSimulationLayout->addWidget(mpStartTimeLabel);
    mpSimulationLayout->addWidget(mpTimeLabelDeliminator1);
    mpSimulationLayout->addWidget(mpTimeStepLabel);
    mpSimulationLayout->addWidget(mpTimeLabelDeliminator2);
    mpSimulationLayout->addWidget(mpFinishTimeLabel);
    mpSimulationLayout->addWidget(mpSimulateButton);
    mpSimulationLayout->addStretch(1);

    this->setLayout(mpSimulationLayout);

    connect(mpStartTimeLabel, SIGNAL(editingFinished()), SLOT(fixLabelValues()));
    connect(mpTimeStepLabel, SIGNAL(editingFinished()), SLOT(fixLabelValues()));
    connect(mpFinishTimeLabel, SIGNAL(editingFinished()), SLOT(fixLabelValues()));
}


//! Make sure the values make sens.
//! @see fixTimeStep()
void SimulationSetupWidget::fixLabelValues()
{
    fixFinishTime();
    fixTimeStep();
}


//! Make sure that the timestep is in the right range i.e. not larger than the simulation time.
//! @see fixFinishTime()
//! @see fixLabelValues()
void SimulationSetupWidget::fixTimeStep()
{
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
    if (getTimeStepLabel() > (getFinishTimeLabel() - getStartTimeLabel()))
        setTimeStepLabel(getFinishTimeLabel() - getStartTimeLabel());

    if (mpParentMainWindow->mpProjectTabs->getCurrentTab()) //crashes if not if statement if no tabs are there...
    {
        mpParentMainWindow->mpProjectTabs->getCurrentTab()->mGUIRootSystem.setDesiredTimeStep(getTimeStepLabel());
    }
}


//! Make sure that the finishs time of the simulation is not smaller than start time.
//! @see fixTimeStep()
//! @see fixLabelValues()
void SimulationSetupWidget::fixFinishTime()
{
    if (getFinishTimeLabel() < getStartTimeLabel())
        setFinishTimeLabel(getStartTimeLabel());

}


//! Sets a new value to a label.
//! @param lineEdit is a pointer to the label which should change
//! @param value is the new value
void SimulationSetupWidget::setValue(QLineEdit *lineEdit, double value)
{
    QString valueTxt;
    valueTxt.setNum(value, 'g', 6 );
    lineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! Sets a new startvalue.
//! @param startTime is the new value
void SimulationSetupWidget::setStartTimeLabel(double startTime)
{
    setValue(mpStartTimeLabel, startTime);
}


//! Sets a new timestep.
//! @param timeStep is the new value
void SimulationSetupWidget::setTimeStepLabel(double timeStep)
{
    setValue(mpTimeStepLabel, timeStep);
}


//! Sets a new finish value.
//! @param finishTime is the new value
void SimulationSetupWidget::setFinishTimeLabel(double finishTime)
{
    setValue(mpFinishTimeLabel, finishTime);
}


//! Acess function to the value of a label.
//! @param lineEdit is the linedit to read
//! @returns the value of the lineedit
double SimulationSetupWidget::getValue(QLineEdit *lineEdit)
{
    return lineEdit->text().toDouble();
}


//! Acess function to the starttimelabel value.
//! @returns the starttime value
double SimulationSetupWidget::getStartTimeLabel()
{
    return getValue(mpStartTimeLabel);
}


//! Acess function to the timesteplabel value.
//! @returns the timestep value
double SimulationSetupWidget::getTimeStepLabel()
{
    return getValue(mpTimeStepLabel);
}


//! Acess function to the finishlabel value.
//! @returns the finish value
double SimulationSetupWidget::getFinishTimeLabel()
{
    return getValue(mpFinishTimeLabel);
}
