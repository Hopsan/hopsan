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

/*
 *  ValveHysteresis.h
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-13.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

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

#include "win32dll.h"

namespace hopsan {

    class DLLIMPORTEXPORT ValveHysteresis
    {
    public:
        //ValveHysteresis();
        double getValue(double xs, double xh, double xd);
    };
}

#endif // VALVEHYSTERESIS_H_INCLUDED
