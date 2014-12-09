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
