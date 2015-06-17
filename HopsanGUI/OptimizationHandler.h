/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   OptimizationHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-02
//! @version $Id$
//!
//! @brief Contains a handler for optimizations
//!
//$Id$

#ifndef OPTIMIZATIONHANDLER_H
#define OPTIMIZATIONHANDLER_H

//Qt includes
#include <QVector>
#include <QStringList>

//Forward declarations
class ModelWidget;
class TerminalWidget;
class HcomHandler;
class Configuration;
class OptimizationWorker;
class GUIMessageHandler;

class OptimizationHandler : public QObject
{
    Q_OBJECT

    friend class HcomHandler;
    friend class OptimizationDialog;
public:
    //Enums
    enum DataT{Integer, Double};
    enum AlgorithmT{Simplex, ComplexRF, ComplexRFM, ComplexRFP, ParticleSwarm, ParameterSweep, Uninitialized};

    //Constructor
    OptimizationHandler(HcomHandler *pHandler);

    //Public access functions
    void startOptimization(ModelWidget *pModel, QString &modelPath);
    void setOptimizationObjectiveValue(int idx, double value);
    void setParMin(int idx, double value);
    void setParMax(int idx, double value);
    double getOptimizationObjectiveValue(int idx);
    double getOptVar(const QString &var);
    double getOptVar(const QString &var, bool &ok) const;
    void setOptVar(const QString &var, const QString &value, bool &ok);
    double getParameter(const int pointIdx, const int parIdx) const;
    void setIsRunning(bool value);
    bool isRunning();
    QStringList *getOptParNamesPtr();


    const QVector<ModelWidget *> *getModelPtrs() const;
    void clearModels();
    void addModel(ModelWidget *pModel);

    int getAlgorithm() const;
    GUIMessageHandler *getMessageHandler();

    Configuration *mpConfig;
    HcomHandler *mpHcomHandler;

    DataT mParameterType; //! @todo Should be public

private:
    GUIMessageHandler *mpMessageHandler;
    OptimizationWorker *mpWorker;
    AlgorithmT mAlgorithm;
    bool mIsRunning;
};

#endif // OPTIMIZATIONHANDLER_H
