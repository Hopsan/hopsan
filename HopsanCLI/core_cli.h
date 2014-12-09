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
//! @file   HopsanCLI/core_cli.h
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains core related declarations and help functions for CLI
//!
//$Id$

#ifndef CORE_CLI_H
#define CORE_CLI_H

// Forward Declaration
namespace hopsan {
class Port;
class ComponentSystem;
class HopsanEssentials;
class HString;
}

extern hopsan::HopsanEssentials gHopsanCore;

void printWaitingMessages(const bool printDebug=true);

#endif // CORE_CLI_H
