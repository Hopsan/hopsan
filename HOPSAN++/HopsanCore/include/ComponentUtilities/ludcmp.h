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
#include "win32dll.h"

namespace hopsan {

    class Matrix;
    class Vec;

    bool ludcmp(Matrix &a, int order[]);
    void DLLIMPORTEXPORT solvlu(const Matrix &a, const Vec &b, Vec &x, const int order[]);
    bool pivot(Matrix &a, int order[], int jcol);
}
#define TINY 1e-200

#endif // LUDCMP_HPP_INCLUDED
