//!
//! @file   Delay.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains the Core Utility Delay class
//!
//$Id$

#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#include <deque>
#include "win32dll.h"


class DLLIMPORTEXPORT Delay
{
public:
    Delay();
    Delay(const std::size_t stepDelay, const double initValue=0.0);
    Delay(const double timeDelay, const double Ts, const double initValue=0.0);
    void initialize(double &rTime, const double initValue);
    void initialize(double &rTime);
    void initializeValues(const double initValue);
    void update(const double value);
    void setStepDelay(const std::size_t stepDelay);
    void setStepDelay(const std::size_t stepDelay, const double initValue);
    void setTimeDelay(const double timeDelay, const double Ts);
    void setTimeDelay(const double timeDelay, const double Ts, const double initValue);
	double value();
	double value(double value);
	double valueIdx(const int idx);
	double valueIdx(double value, const int idx);
	///TODO: Implement void valueTime(double time); A function which returns delayed value of time
private:
    double *mpTime;
    double mLastTime;
    double mInitialValue;
	double mFracStep;
	std::size_t mStepDelay;
	std::deque<double> mValues;
	bool mIsInitialized;
};

#endif // DELAY_H_INCLUDED
