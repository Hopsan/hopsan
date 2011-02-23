//!
//! @file   ludcmp.h
//! @author ?? <??@??.??>
//! @date   2010-11-29
//!
//! @brief Contains a functions to solve eq.systems
//!
//!
//! Finds solution to set of linear equations A x = b by LU decomposition.
//!
//! Chapter 2, Programs 3-5, Fig. 2.8-2.10
//! Gerald/Wheatley, APPLIED NUMERICAL ANALYSIS (fourth edition)
//! Addison-Wesley, 1989
//! (found by Petter on the net, url may be nice)
//!
//$Id$

#ifndef LUDCMP_HPP_INCLUDED
#define LUDCMP_HPP_INCLUDED
#include "../win32dll.h"

namespace hopsan {

    class Matrix;
    class Vec;

    int DLLIMPORTEXPORT ludcmp(Matrix &a, int order[]);
    void DLLIMPORTEXPORT solvlu(const Matrix &a, const Vec &b, Vec &x, const int order[]);
    static int pivot(Matrix &a, int order[], int jcol);
}
#define TINY 1e-20

#endif // LUDCMP_HPP_INCLUDED
