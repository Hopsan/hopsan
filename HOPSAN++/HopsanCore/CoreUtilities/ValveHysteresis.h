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

#include <deque>
#include "../win32dll.h"


class DLLIMPORTEXPORT ValveHysteresis
{
public:
    ValveHysteresis();
    double getValue(double xs, double xh, double xd);
};

#endif // VALVEHYSTERESIS_H_INCLUDED
