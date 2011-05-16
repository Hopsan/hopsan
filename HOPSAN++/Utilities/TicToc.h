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

#ifndef TICTOC_HPP_INCLUDED
#define TICTOC_HPP_INCLUDED

#include <time.h>
#include <string>
#include <iostream>
#ifdef WIN32
        #include "windows.h"
#endif

class TicToc
{
public:
    TicToc(const std::string prefix=std::string())
    {
        mPrefix = prefix;
        Tic();
    }

    void Tic()
    {
        #ifdef WIN32
            mLastTime = GetTickCount();
        #elif defined MAC
        #else
            clock_gettime(CLOCK_REALTIME, &mLastTime);
        #endif
    }

    double Toc()
    {
        #ifdef WIN32
            int now_time;
            now_time = GetTickCount();
            return (now_time - mLastTime)/1000.0; //Convert to seconds
        #elif defined MAC
                return 0;
        #else
            timespec now_time;
            clock_gettime(CLOCK_REALTIME, &now_time);
            return CalcTimeDiff(now_time, mLastTime);
        #endif
    }

    double TocPrint(const std::string prefix=std::string())
    {
        double dt = Toc();
        if (prefix.empty())
        {
            std::cout << mPrefix << ": " << dt << std::endl;
        }
        else
        {
            std::cout << prefix << ": " << dt << std::endl;
        }

        return dt;
    }

private:
    std::string mPrefix;
#ifdef WIN32
    int mLastTime;
#else
    timespec mLastTime;
    double CalcTimeDiff(const timespec &time_now, const timespec &time_last)
    {
        return (double)(time_now.tv_sec - time_last.tv_sec) + ( (double)(time_now.tv_nsec - time_last.tv_nsec) )/1000000000.0;
    }
#endif
};

#endif // TICTOC_HPP_INCLUDED
