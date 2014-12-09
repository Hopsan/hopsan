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
//! @file   HopsanCLI/core_cli.cpp
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains core related help functions for CLI
//!
//$Id$

#include "core_cli.h"
#include "CliUtilities.h"
#include "HopsanEssentials.h"

#include <iostream>
using namespace std;

//! @brief Prints all waiting messages
//! @param[in] printDebug Should debug messages also be printed
void printWaitingMessages(const bool printDebug)
{
    hopsan::HString msg, type, tag;
    while (gHopsanCore.checkMessage() > 0)
    {
        gHopsanCore.getMessage(msg,type,tag);
        if ( (type == "error") || ( type == "fatal") )
        {
            setTerminalColor(Red);
            cout << msg.c_str() << endl;
        }
        else if (type == "warning")
        {
            setTerminalColor(Yellow);
            cout << msg.c_str() << endl;
        }
        else if (type == "debug")
        {
            if (printDebug)
            {
                setTerminalColor(Blue);
                cout << msg.c_str() << endl;
            }
        }
        else
        {
            setTerminalColor(White);
            cout << msg.c_str() << endl;
        }
    }
    setTerminalColor(Reset);
}
