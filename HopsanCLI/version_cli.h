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
//! @file   HopsanCLI/version_cli.h
//! @author peter.nordin@liu.se
//! @date   2014-04-29
//!
//! @brief Contains version number macro for the Hopsan CLI
//!
//$Id$

#ifndef VERSION_CLI_H
#define VERSION_CLI_H

// If we dont have the revision number then define UNKNOWN
// On real relase  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANCLISVNREVISION
 #define HOPSANCLISVNREVISION UNKNOWN
#endif

#define HOPSANCLIVERSION "0.7.x_r" TO_STR(HOPSANCLISVNREVISION)

#endif // VERSION_CLI_H
