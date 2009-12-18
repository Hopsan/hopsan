#include "TicToc.h"
#include <iostream>
#ifdef WIN32
	#include "windows.h"
#endif

using namespace std;


TicToc::TicToc(const string prefix)
{
    mPrefix = prefix;
    Tic();
}

void TicToc::Tic()
{
#ifdef WIN32
    mLastTime = GetTickCount();
#elif defined MAC
#else
    clock_gettime(CLOCK_REALTIME, &mLastTime);
#endif
}

double TicToc::Toc()
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

#ifndef WIN32
#ifdef MAC
double TicToc::CalcTimeDiff(const timespec &time_now, const timespec &time_last)
{
    return (double)(time_now.tv_sec - time_last.tv_sec) + ( (double)(time_now.tv_nsec - time_last.tv_nsec) )/1000000000.0;
}
#endif
#endif
