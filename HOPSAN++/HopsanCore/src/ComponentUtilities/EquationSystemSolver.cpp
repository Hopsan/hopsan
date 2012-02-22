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


//! @brief Solves a system of equations
//! @param jacobian Jacobian matrix
//! @param equations Vector of system equations
//! @param variables Vector of state variables
//! @param iteration How many times the solver has been executed before in the same time step
void EquationSystemSolver::solve(Matrix &jacobian, Vec &equations, Vec &variables, int iteration)
{
    //LU decomposition
    ludcmp(jacobian, mpOrder, mSingular);

    //Stop simulation if LU decomposition failed due to singularity
    if(mSingular)
    {
        mpParentComponent->addErrorMessage("Unable to perform LU-decomposition: Jacobian matrix is singular.");
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
    //LU decomposition
    ludcmp(jacobian, mpOrder, mSingular);

    //Stop simulation if LU decomposition failed due to singularity
    if(mSingular)
    {
        mpParentComponent->addErrorMessage("Unable to perform LU-decomposition: Jacobian matrix is singular.");
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
