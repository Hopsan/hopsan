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

    bool DLLIMPORTEXPORT ludcmp(Matrix &a, int order[]);
    void DLLIMPORTEXPORT solvlu(const Matrix &a, const Vec &b, Vec &x, const int order[]);
    bool DLLIMPORTEXPORT pivot(Matrix &a, int order[], int jcol);
}
#define TINY 1e-200

#endif // LUDCMP_HPP_INCLUDED
