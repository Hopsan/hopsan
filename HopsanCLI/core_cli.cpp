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
void printWaitingMessages(const bool printDebug, bool silent)
{
    if(silent) return;

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
