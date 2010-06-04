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
//! @file   simulationsetupwidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-08
//!
//! @brief Contains a class for setting simulation times
//!
//$Id$

#ifndef SIMULATIONSETUPWIDGET_H
#define SIMULATIONSETUPWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QString>
#include <QtGui>


class QHBoxLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class MainWindow;


class SimulationSetupWidget : public QGroupBox
{
    Q_OBJECT

public:
    SimulationSetupWidget(const QString &title, MainWindow *parent = 0);

    void setStartTimeLabel(double startTime);
    void setTimeStepLabel(double timeStep);
    void setFinishTimeLabel(double finishTime);

    double getStartTimeLabel();
    double getTimeStepLabel();
    double getFinishTimeLabel();

    MainWindow *mpParentMainWindow;
    QGroupBox *mpGroupBox;
    QHBoxLayout *mpSimulationLayout;
    QLineEdit *mpStartTimeLabel;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;
    QLineEdit *mpTimeStepLabel;
    QLineEdit *mpFinishTimeLabel;
    QPushButton *mpSimulateButton;

private:
    void setValue(QLineEdit *lineEdit, double value);
    double getValue(QLineEdit *lineEdit);
    void fixFinishTime();
    void fixTimeStep();

public slots:
    void fixLabelValues();

};

#endif // SIMULATIONSETUPWIDGET_H
