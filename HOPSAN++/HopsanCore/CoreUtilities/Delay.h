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
//! @brief The Delay class delayes a variable in time, some \f$\mbox{\LaTeX}\f$ example: \f$e^{\pi}=-1\f$
//!
//! Delay delayes a variable, the following example
//! delay a variable 5 timesteps:
//!
//! Delay delayVar;
//! delayVar.initialize(time);
//! delayVar.setStepDelay(5);
//!
//! In every loop a Delay instance has to be called with
//! an argument (a new value like "delayVar.value(newValue)"
//! and/or it can be updated trough the call "delayVar.update(newValue)".
//!
{
public:
    Delay();
    Delay(const std::size_t stepDelay, const double initValue=0.0);
    Delay(const double timeDelay, const double Ts, const double initValue=0.0);
    void initialize(double &rTime, const double initValue=0.0);
    void initializeValues(const double initValue);
    void update(const double value);
    void setStepDelay(const std::size_t stepDelay, const double initValue=0.0);
    void setTimeDelay(const double timeDelay, const double Ts, const double initValue=0.0);
    //! Returns the oldest delayed value and update with the last value.
	//! @see value(double value)
	//! @see valueIdx(const int idx)
	//! @see valueIdx(double value, const int idx)
	double value();
	///TODO: Split this value functions to different names, this is confusing and it is not possible to implement a overloaded function to access a value form a given time ago since time is also double
    //!
    //!
    //! Returns the oldest delayed value and update with a new value.
    //! @param value is the new value of the delayed variable.
	//! @see value()
	//! @see valueIdx(const int idx)
	//! @see valueIdx(double value, const int idx)
	double value(double value);
	//! Returns the delayed value at a specified index.
	//! @param idx tell which value to return, 1 is the last timestep's value 2 is the value from two timsteps ago and so on.
	//! @see value(double value)
	//! @see valueIdx(const int idx)
	//! @see valueIdx(double value, const int idx)
	double valueIdx(const int idx);
    //! Returns the delayed value at a specified index and update with a new value.
    //! @param value is the new value of the delayed variable.
	//! @param idx tell which value to return, 1 is the last timestep's value 2 is the value from two timsteps ago and so on.
	//! @see value()
	//! @see value(double value)
	//! @see valueIdx(const int idx)
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
