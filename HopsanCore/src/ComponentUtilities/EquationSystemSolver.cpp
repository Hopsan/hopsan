/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include "ComponentUtilities/num2string.hpp"
#include "ComponentSystem.h"

//Sundials includes
#include "kinsol/kinsol.h"
#include "nvector/nvector_serial.h"
#include "sunmatrix/sunmatrix_dense.h"
#include "sunlinsol/sunlinsol_dense.h"

#include <cstring>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <iostream>

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
//! @param n Number of states
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
    mSingular = false;                      //Tells whether or not the Jacobian is singular
}


//! @brief Constructor for equation system solver utility with additional arguments
//! @param pParentComponent Pointer to parent component
//! @param n Number of states
//! @param pJacobian Pointer to Jacobian matrix
//! @param pEquations Pointer to vector with equations (RHS)
//! @param pVariables Pointer to vector with state variables
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
    mSingular = false;                      //Tells whether or not the Jacobian is singular

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
    if(!ludcmp(jacobian, mpOrder) && mpParentComponent)
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
    if(!ludcmp(jacobian, mpOrder) && mpParentComponent)
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




//! @brief Solves a system of equations. Requires pre-defined pointers to jacobian, equations and state variables.
void EquationSystemSolver::solve()
{
    //Stop simulation if LU decomposition failed due to singularity
    if(!ludcmp(*mpJacobian, mpOrder) && mpParentComponent)
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



//! @brief Constructor for solver utility using numerical integration methods
//! @param pParentComponent Pointer to parent component
//! @param pStateVars Pointer to vector with state variables
//! @param tolerance Tolerance (for Newton-Rhapson with implicit methods)
//! @param maxIter Maximum number of iterations with Newton-Rhapson
NumericalIntegrationSolver::NumericalIntegrationSolver(Component *pParentComponent, std::vector<double> *pStateVars, double tolerance, size_t maxIter)
{
    mpParentComponent = pParentComponent;
    mTimeStep = mpParentComponent->getTimestep();
    mpStateVars = pStateVars;
    mnStateVars = pStateVars->size();
    mTolerance = tolerance;
    mMaxIter = maxIter;
}


//! @brief Returns a list of available integration methods
const std::vector<HString> NumericalIntegrationSolver::getAvailableSolverTypes()
{
    std::vector<HString> availableSolvers;
    availableSolvers.push_back(HString("Forward Euler"));
    availableSolvers.push_back(HString("Midpoint Method"));
    availableSolvers.push_back(HString("Runge-Kutta"));
    availableSolvers.push_back(HString("Dormand-Prince"));
    availableSolvers.push_back(HString("Backward Euler"));
    availableSolvers.push_back(HString("Trapezoid Rule"));
    return availableSolvers;
}


//! @brief Solves a system using numerical integration
//! @param solverType Integration method to use
void NumericalIntegrationSolver::solve(const int solverType)
{
    //DEBUG
    //solvevariableTimeStep();
    //return;
    //END DEBUG

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
    case 4:
        solveBackwardEuler();
        break;
    case 5:
        solveTrapezoidRule();
        break;
    default:
        mpParentComponent->addErrorMessage("Unknown solver type!");
        mpParentComponent->stopSimulation();
    }
}


//! @brief Solves a system using forward Euler method
void NumericalIntegrationSolver::solveForwardEuler()
{
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*mpParentComponent->getStateVariableDerivative(i);
    }
    mpParentComponent->reInitializeValuesFromNodes();
    mpParentComponent->solveSystem();
}


//! @brief Solves a system using midpoint method
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
    mpParentComponent->reInitializeValuesFromNodes();
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k2[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*k2[i];
    }
    mpParentComponent->solveSystem();
}


//! @brief Solves a system using implicit Euler
void NumericalIntegrationSolver::solveBackwardEuler()
{
    std::vector<double> yorg;
    yorg.resize(mnStateVars);
    yorg= *mpStateVars;

    std::vector<double> y0;
    y0.resize(mnStateVars);

    std::vector<double> y1;
    y1.resize(mnStateVars);

    for(int i=0; i<mnStateVars; ++i)
    {
       y0[i] = yorg[i] + mTimeStep*mpParentComponent->getStateVariableDerivative(i);
    }
    mpParentComponent->reInitializeValuesFromNodes();
    mpParentComponent->solveSystem();

    bool doBreak = false;
    size_t i;
    for(i=0; i<mMaxIter; ++i)
    {
        (*mpStateVars) = y0;
        for(int j=0; j<mnStateVars; ++j)
        {
            y1[j] = yorg[j] + mTimeStep*mpParentComponent->getStateVariableDerivative(j);
        }
        mpParentComponent->reInitializeValuesFromNodes();
        mpParentComponent->solveSystem();

        doBreak = true;
        for(int j=0; j<mnStateVars; ++j)
        {
            if(fabs( fabs(y1[j]-y0[j])/y0[j] ) > mTolerance)
            {
                doBreak = false;
            }
        }

        if(doBreak) break;

        y0 = y1;
    }
    if(!doBreak)
    {
        std::stringstream ss;
        ss << "Backward Euler solver failed to converge after " << i << " iterations.";
        mpParentComponent->addWarningMessage(ss.str().c_str());
    }

    (*mpStateVars) = y1;
}


//! @brief Solves a system using trapezoid rule of integration
void NumericalIntegrationSolver::solveTrapezoidRule()
{
    //Store original state variables = y(t)
    std::vector<double> yorg;
    yorg.resize(mnStateVars);
    yorg= *mpStateVars;

    //Store original state variable derivatives = f(t,y(t))
    std::vector<double> dorg;
    dorg.resize(mnStateVars);
    for(int i=0; i<mnStateVars; ++i)
    {
        dorg[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    std::vector<double> y0;
    y0.resize(mnStateVars);

    std::vector<double> y1;
    y1.resize(mnStateVars);

    for(int i=0; i<mnStateVars; ++i)
    {
        y0[i] = yorg[i] + 0.5*mTimeStep*(dorg[i] + mpParentComponent->getStateVariableDerivative(i));
    }
    mpParentComponent->reInitializeValuesFromNodes();
    mpParentComponent->solveSystem();

    bool doBreak=true;
    size_t i;
    for(i=0; i<mMaxIter; ++i)
    {
        (*mpStateVars) = y0;
        for(int j=0; j<mnStateVars; ++j)
        {
            y1[j] = yorg[j] + 0.5*mTimeStep*(dorg[j] + mpParentComponent->getStateVariableDerivative(j));
        }
        mpParentComponent->reInitializeValuesFromNodes();
        mpParentComponent->solveSystem();

        doBreak = true;
        for(int j=0; j<mnStateVars; ++j)
        {
            if(fabs( fabs(y1[j]-y0[j])/y0[j] ) > mTolerance)
            {
                doBreak = false;
            }
        }

        if(doBreak) break;

        y0 = y1;
    }
    if(!doBreak)
    {
        std::stringstream ss;
        ss << "Trapezoid Rule solver failed to converge after " << i << " iterations.";
        mpParentComponent->addWarningMessage(ss.str().c_str());
    }

    (*mpStateVars) = y1;
}


//! @brief Solves a system using Runge-Kutta (RK4)
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
    mpParentComponent->reInitializeValuesFromNodes();
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k2[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/2.0*k2[i];
    }
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k3[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*k3[i];
    }
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k4[i] = mpParentComponent->getStateVariableDerivative(i);
    }
    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/6.0*(k1[i]+2.0*k2[i]+2.0*k3[i]+k4[i]);
    }
    mpParentComponent->solveSystem();
}


//! @brief Solves a system using Dormand-Prince
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
    mpParentComponent->reInitializeValuesFromNodes();
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k2[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep/40.0*(3*k1[i] + 9*k2[i]);
    }
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k3[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(44.0/45.0*k1[i] - 56.0/15.0*k2[i] + 32.0/9.0*k3[i]);
    }
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k4[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(19372.0/6561.0*k1[i] - 25360.0/2187.0*k2[i] + 64448.0/6561.0*k3[i] - 212.0/729.0*k4[i]);
    }
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k5[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {

        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(9017.0/3168.0*k1[i] -  355.0/33.0*k2[i] + 46732.0/5247.0*k3[i] + 49.0/176.0*k4[i] - 5103.0/18656.0*k5[i]);
    }
    mpParentComponent->solveSystem();
    for(int i=0; i<mnStateVars; ++i)
    {
        k6[i] = mpParentComponent->getStateVariableDerivative(i);
    }

    *mpStateVars = orgStateVars;
    mpParentComponent->reInitializeValuesFromNodes();
    for(int i=0; i<mnStateVars; ++i)
    {
        (*mpStateVars)[i] = (*mpStateVars)[i] + mTimeStep*(35.0/384.0*k1[i] + 500.0/1113.0*k3[i] + 125.0/192.0*k4[i] - 2187.0/6784.0*k5[i] + 11.0/84.0*k6[i]);
    }
    mpParentComponent->solveSystem();
}


//! @brief Solves a system using trapezoid rule with variable step size (experimental, do not use)
void NumericalIntegrationSolver::solvevariableTimeStep()
{
    //Store original state variables = y(t)
    std::vector<double> yorg;
    yorg.resize(mnStateVars);
    yorg= *mpStateVars;

    solveBackwardEuler();

    std::vector<double> eulerStateVars = *mpStateVars;
    *mpStateVars = yorg;

    solveTrapezoidRule();

    double maxErr = 0;
    for(int i=0; i<mnStateVars; ++i)
    {
        maxErr = std::max(maxErr, fabs(eulerStateVars[i] - (*mpStateVars)[i]));
    }

    if(maxErr == 0)
        return;

    double h = mpParentComponent->getTimestep();
    double h_new = std::max(1e-5, std::min(1e-2, 0.9*h*1e-8/maxErr));
    mpParentComponent->setTimestep(h_new);

    if(h_new < h)
    {
        *mpStateVars = yorg;
        solveTrapezoidRule();
    }
}


static int kinsolResidualCallback(N_Vector y, N_Vector f, void *user_data)
{
    Component *pComponent = static_cast<Component*>(user_data);
    pComponent->getResiduals(NV_DATA_S(y), NV_DATA_S(f));
    return(0);
}


static int kinsolJacobianCallback(N_Vector y, N_Vector f, SUNMatrix J, void *user_data,N_Vector tmp1, N_Vector tmp2)
{
    Component *pComponent = static_cast<Component*>(user_data);
    pComponent->getJacobian(NV_DATA_S(y), NV_DATA_S(f), SM_DATA_D(J));
    return 0;
}


class KinsolSolver::Impl
{
public:
    Impl(Component *pParentComponent, double tol, int n, SolverTypeEnum solverType=NewtonIteration);
    ~Impl();
    void solve();
    double getState(int i);
    void setState(int i, double value);
    void setTolerance(double value);

    Component *mpComponent;
    void *mem;
    N_Vector y;
    N_Vector scale;
    SUNLinearSolver LS;
    SUNMatrix J;
    double mSolverTime;
    SolverTypeEnum mType;
};


KinsolSolver::Impl::Impl(Component *pComponent, double tol, int n, SolverTypeEnum type)
    : mpComponent(pComponent),
      mSolverTime(pComponent->getTime()),
      mType(type)
{
    int flag;

    y = 0;
    scale = 0;
    LS = 0;
    J = 0;

    //Initialize vectors
    y = N_VNew_Serial(n);
    scale = N_VNew_Serial(n);
    N_VConst(1, scale);

    // Create solver memory
    mem = KINCreate();
    if(!mem) {
        mpComponent->stopSimulation("KINCreate() return null pointer.");
        return;
    }

    flag = KINSetUserData(mem, static_cast<void*>(mpComponent));
    if(flag < 0) {
        mpComponent->stopSimulation("KINSetUserData() failed with flag "+to_hstring(flag)+".");
        return;
    }

    if(type == FixedPointIteration) {
        flag = KINSetMAA(mem, 2);
        if (flag < 0) {
            mpComponent->stopSimulation("KINSetMAA() failed with flag "+to_hstring(flag)+".");
            return;
        }
    }

    flag = KINInit(mem, kinsolResidualCallback, y);
    if (flag < 0) {
        mpComponent->stopSimulation("KINInit() failed with flag "+to_hstring(flag)+".");
        return;
    }

    setTolerance(tol);

    if(type == NewtonIteration) {
        J = SUNDenseMatrix(n, n);
        if(!J) {
            mpComponent->stopSimulation("SUNDenseMatrix() return null pointer.");
            return;
        }

        LS = SUNLinSol_Dense(y, J);
        if(!LS) {
            mpComponent->stopSimulation("SUNLinSol_Dense() return null pointer.");
            return;
        }

        flag = KINSetLinearSolver(mem, LS, J);
        if (flag < 0) {
            mpComponent->stopSimulation("KINSetLinearSolver() failed with flag "+to_hstring(flag)+".");
            return;
        }

        flag = KINSetMaxSetupCalls(mem, 1);
        if (flag < 0) {
            mpComponent->stopSimulation("KINSetMaxSetupCalls() failed with flag "+to_hstring(flag)+".");
            return;
        }

        flag = KINSetNumMaxIters(mem,100000);
        if(flag<0) {
            mpComponent->stopSimulation("KINSetNumMaxIters() failed with flag "+to_hstring(flag)+".");
            return;
        }

        flag = KINSetNumMaxIters(mem,100000);
        if(flag<0) {
            mpComponent->stopSimulation("KINSetNumMaxIters() failed with flag "+to_hstring(flag)+".");
            return;
        }

        flag = KINSetJacFn(mem, kinsolJacobianCallback);
        if (flag < 0) {
            mpComponent->stopSimulation("KINSetJacFn() failed with flag "+to_hstring(flag)+".");
            return;
        }
    }
    return;
}

KinsolSolver::Impl::~Impl()
{
    KINFree(&mem);
    N_VDestroy(y);
    N_VDestroy(scale);
    y = 0;
    scale = 0;
    if(mType == NewtonIteration) {
        SUNLinSolFree(LS);
        SUNMatDestroy(J);
        LS = 0;
        J = 0;
    }
}

void KinsolSolver::Impl::solve()
{
    int strategy = KIN_LINESEARCH;
    if(mType == FixedPointIteration) {
        strategy = KIN_FP;
    }

    int flag = KINSol(mem, y, strategy, scale, scale);
    if (flag < KIN_SUCCESS && flag != KIN_LINESEARCH_NONCONV && flag != KIN_MXNEWT_5X_EXCEEDED) {
        mpComponent->stopSimulation("KINSol() failed with flag "+HString(KINGetReturnFlagName(flag))+".");
        return;
    }
}

double KinsolSolver::Impl::getState(int i)
{
    return NV_Ith_S(y,i);
}

void KinsolSolver::Impl::setState(int i, double value)
{
    NV_Ith_S(y,i) = value;
}

void KinsolSolver::Impl::setTolerance(double value)
{
    int flag = KINSetFuncNormTol(mem, value);
    if (flag < 0) {
        mpComponent->stopSimulation("KINSetFuncNormTol() failed with flag "+to_hstring(flag)+".");
        return;
    }

    if(mType == NewtonIteration) {
        flag = KINSetScaledStepTol(mem, value);
        if (flag < 0) {
            mpComponent->stopSimulation("KINSetScaledStepTol() failed with flag "+to_hstring(flag)+".");
            return;
        }
    }
}



KinsolSolver::KinsolSolver(Component *pComponent, double tol, int n, SolverTypeEnum type=NewtonIteration) : impl(new Impl(pComponent, tol, n, type)) {}

KinsolSolver::~KinsolSolver()
{
    delete impl;
}

void KinsolSolver::solve()
{
    impl->solve();
}

double KinsolSolver::getState(int i)
{
    return impl->getState(i);
}

void KinsolSolver::setState(int i, double value)
{
    impl->setState(i, value);
}

void KinsolSolver::setTolerance(double value)
{
    impl->setTolerance(value);
}
