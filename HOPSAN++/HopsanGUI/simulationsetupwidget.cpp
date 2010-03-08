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
#include "simulationsetupwidget.h"


//! Constructor.
//! @param title is the title of the group box.
//! @param parent defines a parent to the new instanced object.
SimulationSetupWidget::SimulationSetupWidget(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    //mpGroupBox = new QGroupBox(title, this);

    mpSimulationLayout = new QHBoxLayout;
    mpStartTimeLabel = new QLineEdit;
    mpStartTimeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpStartTimeLabel->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpStartTimeLabel));
    mpTimeStepLabel = new QLineEdit;
    mpTimeStepLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeStepLabel->setValidator(new QDoubleValidator(0.0, 999.0, 6, mpStartTimeLabel));
    mpFinishTimeLabel = new QLineEdit;
    mpFinishTimeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpFinishTimeLabel->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpFinishTimeLabel));
    mpTimeLabelDeliminator1 = new QLabel(tr("::"));
    mpTimeLabelDeliminator2 = new QLabel(tr("::"));
    mpSimulateButton = new QPushButton(tr("Simulate!"));

    mpSimulationLayout->addStretch(1);
    mpSimulationLayout->addWidget(mpStartTimeLabel);
    mpSimulationLayout->addWidget(mpTimeLabelDeliminator1);
    mpSimulationLayout->addWidget(mpTimeStepLabel);
    mpSimulationLayout->addWidget(mpTimeLabelDeliminator2);
    mpSimulationLayout->addWidget(mpFinishTimeLabel);
    mpSimulationLayout->addWidget(mpSimulateButton);
    mpSimulationLayout->addStretch(1);

    this->setLayout(mpSimulationLayout);

    connect(mpFinishTimeLabel, SIGNAL(editingFinished()), SLOT(fixFinishTime()));
    connect(mpTimeStepLabel, SIGNAL(editingFinished()), SLOT(fixTimeStep()));

}

//! Make sure that the timestep is in the right range i.e. not larger than the simulation time.
void SimulationSetupWidget::fixTimeStep()
{
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
    double startTime = mpStartTimeLabel->text().toDouble();
    double timeStep = mpTimeStepLabel->text().toDouble();
    double finishTime = mpFinishTimeLabel->text().toDouble();
    if (timeStep > (finishTime - startTime))
    {
        QString valueTxt;
        valueTxt.setNum(finishTime - startTime, 'g', 6 );
        mpTimeStepLabel->setText(valueTxt);
    }

}


//! Make sure that the finishs time of the simulation is not smaller than start time.
void SimulationSetupWidget::fixFinishTime()
{
    double startTime = mpStartTimeLabel->text().toDouble();
    double finishTime = mpFinishTimeLabel->text().toDouble();
    if (finishTime < startTime)
        mpFinishTimeLabel->setText(mpStartTimeLabel->text());

}

