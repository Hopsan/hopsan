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
    enum AlgorithmT{ComplexRF, ComplexRFM, ComplexRFP, ParticleSwarm, ParameterSweep, Uninitialized};

    //Constructor
    OptimizationHandler(HcomHandler *pHandler);

    //Public access functions
    void startOptimization();
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
