#ifndef TICTOC_H_INCLUDED
#define TICTOC_H_INCLUDED

#include <time.h>
#include <string>

class TicToc
{
public:
    TicToc(const std::string prefix=std::string());
    void Tic();
    double Toc();
    double TocPrint(const std::string prefix=std::string());
private:
    std::string mPrefix;
#ifdef WIN32
    int mLastTime;
#else
    timespec mLastTime;
    double CalcTimeDiff(const timespec &time_now, const timespec &time_last);
#endif
};

#endif // TICTOC_H_INCLUDED
