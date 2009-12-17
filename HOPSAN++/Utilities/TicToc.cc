#include "TicToc.h"
#include <iostream>
using namespace std;


#ifdef WIN32

TicToc::TicToc(const string prefix)
{
    mPrefix = prefix;
    Tic();
}

void TicToc::Tic()
{
    mLastTime = GetTickCount();
}

double TicToc::Toc()
{
    int now_time;
    now_time = GetTickCount();
    return now_time - mLastTime;
}

double TicToc::TocPrint(const string prefix)
{
    double dt = Toc();
    if (prefix.empty())
    {
        cout << mPrefix << ": " << dt << endl;
    }
    else
    {
        cout << prefix << ": " << dt << endl;
    }

    return dt;
}


#else

TicToc::TicToc(const string prefix)
{
    mPrefix = prefix;
    Tic();
}

void TicToc::Tic()
{
    clock_gettime(CLOCK_REALTIME, &mLastTime);
}

double TicToc::Toc()
{
    timespec now_time;
    clock_gettime(CLOCK_REALTIME, &now_time);
    return CalcTimeDiff(now_time, mLastTime);
}

double TicToc::TocPrint(const string prefix)
{
    double dt = Toc();
    if (prefix.empty())
    {
        cout << mPrefix << ": " << dt << endl;
    }
    else
    {
        cout << prefix << ": " << dt << endl;
    }

    return dt;
}

double TicToc::CalcTimeDiff(const timespec &time_now, const timespec &time_last)
{
    return (double)(time_now.tv_sec - time_last.tv_sec) + ( (double)(time_now.tv_nsec - time_last.tv_nsec) )/1000000000.0;
}

#endif