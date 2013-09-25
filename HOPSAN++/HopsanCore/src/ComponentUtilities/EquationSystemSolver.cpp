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
//! @file   EquationSystemSolver.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-02-04
//!
//! @brief Contains an equation system solver utility
//!
//$Id$

#include "ComponentUtilities/EquationSystemSolver.h"
#include "ComponentUtilities/ludcmp.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Component.h"
#include "ComponentUtilities/matrix.h"

#include <cstring>
#include <stdlib.h>

using namespace hopsan;

//! @class hopsan::EquationSystemSolver
//! @ingroup ComponentUtilityClasses
//! @brief A numerical solver utility for equation systems using LU-decomposition
//!
//! Solves \f$J*b = x\f$ by first transforming
//! it to \f$L*U*b = x\f$ and subsequently solving
//! \f$U*b = y\f$ and \f$L*y = x\f$
//!

//! @brief Constructor for equation system solver utility
//! @param pParentComponent Pointer to parent component
EquationSystemSolver::EquationSystemSolver(Component *pParentComponent, int n)
{
    mpParentComponent = pParentComponent;

    // Weights for equations, used when running several iterations
    mSystemEquationWeight[0]=1;
    mSystemEquationWeight[1]=0.67;
    mSystemEquationWeight[2]=0.5;
    mSystemEquationWeight[3]=0.5;

    mnVars = n;
    mpOrder = new int[n];                   //Used to keep track of the order of the equations
    mpDeltaStateVar = new Vec(n);           //Difference between nwe state variables and the previous ones
    mSingular = false;                      //Tells whether or not the Jaciabian is singular
}


EquationSystemSolver::EquationSystemSolver(Component *pParentComponent, int n, Matrix *pJacobian, Vec *pEquations, Vec *pVariables)
{
    mpParentComponent = pParentComponent;

    // Weights for equations, used when running several iterations
    mSystemEquationWeight[0]=1;
    mSystemEquationWeight[1]=0.67;
    mSystemEquationWeight[2]=0.5;
    mSystemEquationWeight[3]=0.5;

    mnVars = n;
    mpOrder = new int[n];                   //Used to keep track of the order of the equations
    mpDeltaStateVar = new Vec(n);           //Difference between nwe state variables and the previous ones
    mSingular = false;                      //Tells whether or not the Jaciabian is singular

    mpJacobian = pJacobian;
    mpEquations = pEquations;
    mpVariables = pVariables;
}


//! @brief Solves a system of equations
//! @param jacobian Jacobian matrix
//! @param equations Vector of system equations
//! @param variables Vector of state variables
//! @param iteration How many times the solver has been executed before in the same time step
void EquationSystemSolver::solve(Matrix &jacobian, Vec &equations, Vec &variables, int iteration)
{
    //Stop simulation if LU decomposition failed due to singularity
    if(!ludcmp(jacobian, mpOrder))
    {
        mpParentComponent->addErrorMessage("Unable to perform LU-decomposition: Jacobian matrix is probably singular.");
        mpParentComponent->stopSimulation();
    }

    //Solve system using L and U matrices
    solvlu(jacobian,equations,*mpDeltaStateVar,mpOrder);

    //Calculate new system variables
    for(int i=0; i<mnVars; ++i)
    {
        variables[i] = variables[i] - mSystemEquationWeight[iteration - 1] * (*mpDeltaStateVar)[i];
    }
}



//! @brief Solves a system of equations with just one iteration (slightly faster)
//! @param jacobian Jacobian matrix
//! @param equations Vector of system equations
//! @param variables Vector of state variables
void EquationSystemSolver::solve(Matrix &jacobian, Vec &equations, Vec &variables)
{
    //Stop simulation if LU decomposition failed due to singularity
    if(!ludcmp(jacobian, mpOrder))
    {
        mpParentComponent->addErrorMessage("Unable to perform LU-decomposition: Jacobian matrix is probably singular.");
        mpParentComponent->stopSimulation();
    }

    //Solve system using L and U matrices
    solvlu(jacobian,equations,*mpDeltaStateVar,mpOrder);

    //Calculate new system variables
    for(int i=0; i<mnVars; ++i)
    {
        variables[i] = variables[i] - (*mpDeltaStateVar)[i];
    }
}




//! @brief Solves a system of equations
//! @param jacobian Jacobian matrix
//! @param equations Vector of system equations
//! @param variables Vector of state variables
//! @param iteration How many times the solver has been executed before in the same time step
void EquationSystemSolver::solve()
{
    //Stop simulation if LU decomposition failed due to singularity
    if(!ludcmp(*mpJacobian, mpOrder))
    {
        mpParentComponent->addErrorMessage("Unable to perform LU-decomposition: Jacobian matrix is probably singular.");
        mpParentComponent->stopSimulation();
    }

    //Solve system using L and U matrices
    solvlu(*mpJacobian,*mpEquations,*mpDeltaStateVar,mpOrder);

    //Calculate new system variables
    for(int i=0; i<mnVars; ++i)
    {
        (*mpVariables)[i] = (*mpVariables)[i] - (*mpDeltaStateVar)[i];
    }
}




NumericalIntegrationSolver::NumericalIntegrationSolver(Component *pParentComponent, std::vector<double> *pStateVars)
{
    mpParentComponent = pParentComponent;
    mTimeStep = mpParentComponent->getTimestep();
    mpStateVars = pStateVars;
    mnStateVars = pStateVars->size();
}


const std::vector<HString> NumericalIntegrationSolver::getAvailableSolverTypes()
{
    std::vector<HString> availableSolvers;
    availableSolvers.push_back(HString("Forward Euler"));
    availableSolvers.push_back(HString("Midpoint Method"));
    availableSolvers.push_back(HString("Runge-Kutta"));
    availableSolvers.push_back(HString("Dormand-Prince"));
    return availableSolvers;
}


void NumericalIntegrationSolver::solve(const int solverType)
{
    switch(solverType)
    {
    case 0:
        solveForwardEuler();
        break;
    case 1:
        solveMidpointMethod();
        break;
    case 2:
        solveRungeKutta();
        break;
    case 3:
        solveDormandPrince();
        break;
    default:
        mpParentComponent->addErrorMessage("Unknown solver type!");
        mpParentComponent->stopSimulation();
    }
}


void NumericalIntegrationSolver::solveForwardEuler()
{
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*mpParentComponent->getStateVariableDerivative(i);
    }
}


void NumericalIntegrationSolver::solveMidpointMethod()
{
    std::vector<double> k1, k2;
    k1.resize(mnStateVars);
    k2.resize(mnStateVars);

    std::vector<double> orgStateVars;
    orgStateVars.resize(mnStateVars);

    orgStateVars= *mpStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        k1[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/2.0*k1[i];
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k2[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*k2[i];
    }
}


void NumericalIntegrationSolver::solveBackwardEuler()
{
    std::vector<double> orgStateVars;
    orgStateVars= *mpStateVars;

    double tol = 1e-5;
    bool stop=false;
    while(!stop)
    {
        stop=true;
        for(int i=0; i<mnStateVars; ++i)
        {
            (*mpStateVars)[i] = (*mpStateVars)[i] - ((*mpStateVars)[i] - orgStateVars[i] - mTimeStep*mpParentComponent->getStateVariableDerivative(i))/(1-mTimeStep*mpParentComponent->getStateVariableSecondDerivative(i));
        }
        for(int i=0; i<mnStateVars; ++i)
        {
            double error = (*mpStateVars)[i] - orgStateVars[i] - mTimeStep*mpParentComponent->getStateVariableDerivative(i);
            if(error > tol)
            {
                stop=false;
            }
        }
    }
}


void NumericalIntegrationSolver::solveRungeKutta()
{
    std::vector<double> k1, k2, k3, k4;
    k1.resize(mnStateVars);
    k2.resize(mnStateVars);
    k3.resize(mnStateVars);
    k4.resize(mnStateVars);

    std::vector<double> orgStateVars;
    orgStateVars.resize(mnStateVars);

    orgStateVars= *mpStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        k1[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/2.0*k1[i];
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k2[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/2.0*k2[i];
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k3[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*k3[i];
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k4[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/6.0*(k1[i]+2.0*k2[i]+2.0*k3[i]+k4[i]);
    }
}


void NumericalIntegrationSolver::solveDormandPrince()
{
    std::vector<double> k1, k2, k3, k4, k5, k6;
    k1.resize(mnStateVars);
    k2.resize(mnStateVars);
    k3.resize(mnStateVars);
    k4.resize(mnStateVars);
    k5.resize(mnStateVars);
    k6.resize(mnStateVars);

    std::vector<double> orgStateVars;
    orgStateVars.resize(mnStateVars);

    orgStateVars= *mpStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        k1[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/5.0*k1[i];
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k2[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/40.0*(3*k1[i] + 9*k2[i]);
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k3[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(44.0/45.0*k1[i] - 56.0/15.0*k2[i] + 32.0/9.0*k3[i]);
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k4[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(19372.0/6561.0*k1[i] - 25360.0/2187.0*k2[i] + 64448.0/6561.0*k3[i] - 212.0/729.0*k4[i]);
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k5[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {

        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(9017.0/3168.0*k1[i] -  355.0/33.0*k2[i] + 46732.0/5247.0*k3[i] + 49.0/176.0*k4[i] - 5103.0/18656.0*k5[i]);
    }
    for(int i=0; i<mnStateVars; ++i)
    {
        k6[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(35.0/384.0*k1[i] + 500.0/1113.0*k3[i] + 125.0/192.0*k4[i] - 2187.0/6784.0*k5[i] + 11.0/84.0*k6[i]);
    }
}

