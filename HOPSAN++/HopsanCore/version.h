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

#ifndef VERSION_H
#define VERSION_H

#define HOPSANCOREVERSION "0.4.x_trunk"
#define HOPSANCOREMODELFILEVERSION "0.4"

#ifdef DEBUGCOMPILING
#define DEBUGRELEASECOMPILED "DEBUG"
#elif defined  RELEASECOMPILING
#define DEBUGRELEASECOMPILED "RELEASE"
#else
#warning You must specify Debug or Release compiling by defining DEBUGCOMPILING or RELEASECOMPILING
#define DEBUGRELEASECOMPILED "UNDEFINED"
#endif

#endif // VERSION_H
