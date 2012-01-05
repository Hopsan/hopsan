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
//! @file   WhiteGaussianNuise.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-06-09
//!
//! @brief Contains a helper function that generates white gaussian noise
//!


#ifndef WHITEGAUSSIANNOISE_H_INCLUDED
#define WHITEGAUSSIANNOISE_H_INCLUDED

#include "win32dll.h"
#include <math.h>

namespace hopsan {

    class DLLIMPORTEXPORT WhiteGaussianNoise
    {
    public:
        WhiteGaussianNoise();
        double getValue();
    };
}

#endif // WHITEGAUSSIANNOISE_H_INCLUDED
