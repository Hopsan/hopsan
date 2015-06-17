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
//! @file   ValveHysteresis.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-13
//!
//! @brief Contains a hysteresis function for valves and signals
//!
//$Id$

////////////////////////////////////
//     Hysteresis in Valves       //
//                                //
//             Hysteresis Width   //
//                <------->       //
//               |                //
//               |*********       //
//               *   x   *        //
//              *|  x   *         //
//             * | x   *          //
//            *  |x   *           //
// ----------*---x---*----------- //
//          *   x|  *             //
//         *   x | *              //
//        *   x  |*               //
//       *   x   *                //
//      *********|                //
//               |                //
////////////////////////////////////

#ifndef VALVEHYSTERESIS_H_INCLUDED
#define VALVEHYSTERESIS_H_INCLUDED

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class ValveHysteresis
{
public:
    //! @todo does this really need to be a class? It could be a function (no member variables, and only one function)
    double getValue(double xs, double xh, double xd)
    {
        if (xd < xs-xh/2.0)
        {
            return xs-xh/2.0;
        }
        else if (xd > xs+xh/2.0)
        {
            return xs+xh/2.0;
        }
        else
        {
            return xd;
        }
    }
};

}

#endif // VALVEHYSTERESIS_H_INCLUDED
