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

#ifndef GUIROOTSYSTEM_H
#define GUIROOTSYSTEM_H

#include "HopsanCore.h"
#include <QString>
#include <qdebug.h>

using namespace hopsan;

class GUIRootSystem
{
public:
    GUIRootSystem();

    bool connect(QString compname1, QString portname1, QString compname2, QString portname2);
    bool disconnect(QString compname1, QString portname1, QString compname2, QString portname2);

    void setDesiredTimeStep(double timestep);
    double getDesiredTimeStep();

    void setRootTypeCQS(const QString cqs_type, bool doOnlyLocalSet=false);
    void setSystemTypeCQS(QString systemName, const std::string cqs_type, bool doOnlyLocalSet=false);
    QString getTypeCQS(QString componentName);
    QString getSystemTypeCQS(QString systemName); //!< @todo dont think that we need this one the component specifik one should do

    void setRootSystemName(QString name, bool doOnlyLocalRename=false);
    QString getName(); //!< Shouldnt this one be named getRootSystemName like set name above
    QString setSystemName(QString systemname, QString name, bool doOnlyLocalRename=false); //!< @todo This might not be necessary, should be able to use the component base class specifik one
    QString rename(QString componentName, QString name, bool doOnlyLocalRename=false);

    QString getPortType(QString componentName, QString portName);
    QString getNodeType(QString componentName, QString portName);

    QVector<QString> getParameterNames(QString componentName);
    QString getParameterUnit(QString componentName, QString parameterName);
    QString getParameterDescription(QString componentName, QString parameterName);
    double getParameterValue(QString componentName, QString parameterName);
    void setParameter(QString componentName, QString parameterName, double value); //!< @todo maybe call this set parameter value

    QString createComponent(QString type);
    QString createSubSystem();
    void removeSubComponent(QString componentName, bool doDelete);

    bool isSimulationOk();
    void initialize(double mStartTime, double mFinishTime);
    void simulate(double mStartTime, double mFinishTime);
    void finalize(double mStartTime, double mFinishTime);
    double getCurrentTime();
    void stop();

    void deleteSystemPort(QString portname);
    QString addSystemPort(QString portname);
    QString renameSystemPort(QString oldname, QString newname);

    std::vector<double> getTimeVector(QString componentName, QString portName);
    void getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits);
    //void getPlotDataUnit(const QString compname, const QString portname, const string dataname, QString &rUnit);
    //QVector<QString> getPlotDataUnits();
    void getPlotData(const QString compname, const QString portname, const QString dataname, QVector<double> &rData);

    bool isPortConnected(QString componentName, QString portName);

private:
    //*****Core Interaction*****
    ComponentSystem *mpCoreComponentSystem;
    //**************************
};

#endif // GUIROOTSYSTEM_H
