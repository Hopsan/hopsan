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

namespace Ops {

    class Matrix;
    class Vec;

    bool ludcmp(Matrix &a, int order[]);
    void solvlu(const Matrix &a, const Vec &b, Vec &x, const int order[]);
    bool pivot(Matrix &a, int order[], int jcol);
}
#define TINY 1e-200

#endif // LUDCMP_HPP_INCLUDED
