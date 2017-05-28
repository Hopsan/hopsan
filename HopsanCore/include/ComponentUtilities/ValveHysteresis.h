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
