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

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class DLLIMPORTEXPORT EquationSystemSolver
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
class DLLIMPORTEXPORT NumericalIntegrationSolver
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


}

#endif // EQUATIONSYSTEMSOLVER_H
