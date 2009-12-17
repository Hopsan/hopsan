#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

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
    timespec mLastTime;
    double CalcTimeDiff(const timespec &time_now, const timespec &time_last);
};


#endif // TIME_H_INCLUDED
