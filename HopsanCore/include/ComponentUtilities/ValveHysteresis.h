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
    //! @todo does this realy need to be aclass, it could be a function (no member variables, and only one function)
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
