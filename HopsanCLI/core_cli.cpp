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
