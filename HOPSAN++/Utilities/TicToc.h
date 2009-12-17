#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

#include <time.h>
#include <string>

class TicToc
{
#ifdef WIN32
	public:
		TicToc(const std::string prefix=std::string());
		void Tic();
		double Toc();
		double TocPrint(const std::string prefix=std::string());
	private:
		std::string mPrefix;
		int mLastTime;
		double CalcTimeDiff(const int &time_now, const int &time_last);

#else
	public:
		TicToc(const std::string prefix=std::string());
		void Tic();
		double Toc();
		double TocPrint(const std::string prefix=std::string());

	private:
		std::string mPrefix;
		timespec mLastTime;
		double CalcTimeDiff(const timespec &time_now, const timespec &time_last);
#endif

};


#endif // TIME_H_INCLUDED
