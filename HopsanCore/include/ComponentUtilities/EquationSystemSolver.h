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

#ifndef EQUATIONSYSTEMSOLVER_H
#define EQUATIONSYSTEMSOLVER_H

#include "Component.h"
#include "matrix.h"
#include "ludcmp.h"

#include <vector>

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class HOPSANCORE_DLLAPI EquationSystemSolver
{
public:

    EquationSystemSolver(Component *pParentComponent, int n);
    EquationSystemSolver(Component *pParentComponent, int n, Matrix *pJacobian, Vec *pEquations, Vec *pVariables);
    void solve(Matrix &jacobian, Vec &equations, Vec &variables, int iteration);
    void solve(Matrix &jacobian, Vec &equations, Vec &variables);
    void solve();

private:
    Component *mpParentComponent;
    double mSystemEquationWeight[4];
    int *mpOrder;
    Vec *mpDeltaStateVar;
    int mnVars;
    bool mSingular;

    Matrix *mpJacobian;
    Vec *mpEquations;
    Vec *mpVariables;
};


//! @ingroup ComponentUtilityClasses
class HOPSANCORE_DLLAPI NumericalIntegrationSolver
{
public:
    typedef double (NumericalIntegrationSolver::*callback_function)(int);

    NumericalIntegrationSolver(Component *pParentComponent, std::vector<double> *pStateVars, double tolerance=1e-6, size_t maxIter=1000);
    static const std::vector<HString> getAvailableSolverTypes();

    void solve(int solver);
    void solveForwardEuler();
    void solveMidpointMethod();
    void solveBackwardEuler();
    void solveTrapezoidRule();
    void solveRungeKutta();
    void solveDormandPrince();

    void solvevariableTimeStep();

    double findRoot(int i);

private:
    Component *mpParentComponent;

    //Numerical integration members
    double mTimeStep;
    std::vector<double> *mpStateVars;
    int mnStateVars;

    //Implicit methods members
    double mTolerance;
    size_t mMaxIter;
};


class HOPSANCORE_DLLAPI KinsolSolver
{
public:
    enum SolverTypeEnum {NewtonIteration=0,FixedPointIteration=1};

    KinsolSolver(Component *pComponent, double tol, int n, SolverTypeEnum type);
    ~KinsolSolver();
    void solve();
    double getState(int i);
    void setState(int i, double value);
    void setTolerance(double value);
private:
    class Impl;
    Impl* impl;
};

}

#endif // EQUATIONSYSTEMSOLVER_H
