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
//! @file   HopsanCoreMacros.h
//! @author <peter.nordin@liu.se>
//! @date   2013-11-20
//!
//! @brief Contains HopsanCore global macro definitions
//!
//$Id$

#ifndef HOPSANCOREMACROS_H
#define HOPSANCOREMACROS_H

//! @brief This macro can be used to indicate that a function argument is unused and avoids a compiler warning
#define HOPSAN_UNUSED(x) (void)x;

// Stringify Macro
#ifndef TO_STR_2
 #define TO_STR_2(s) #s
 //! @brief This macro is used to turn a defined value into a string
 #define TO_STR(s) TO_STR_2(s)
#endif

#endif // HOPSANCOREMACROS_H
