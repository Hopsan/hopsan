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
//$Id$

#ifndef TICTOC_HPP_INCLUDED
#define TICTOC_HPP_INCLUDED

#include <time.h>
#include <string>
#include <iostream>

#ifdef WIN32
#include "windows.h"
#endif

#ifdef __APPLE__
#include <sys/time.h>
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
        #elif __APPLE__
            gettimeofday(&m_LastTime,NULL);
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
        #elif defined __APPLE__
            double dt;
            struct timeval now;
            gettimeofday(&now,NULL);
            dt=double(now.tv_sec-m_LastTime.tv_sec);
            dt+=double(now.tv_usec-m_LastTime.tv_usec)*1e-6;
            return dt;
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
#elif __APPLE__
    struct timeval m_LastTime;
#else
    timespec mLastTime;
    double CalcTimeDiff(const timespec &time_now, const timespec &time_last)
    {
        return (double)(time_now.tv_sec - time_last.tv_sec) + ( (double)(time_now.tv_nsec - time_last.tv_nsec) )/1000000000.0;
    }
#endif
};

#endif // TICTOC_HPP_INCLUDED
