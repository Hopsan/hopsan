/*
 *  Delay.h
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-19.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */

#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#include <deque>
#include "win32dll.h"

/*
 * Följande exempel fördröjer variablen var i 5 tidssteg
 *
 * Delay delayedVar(5); // instansiering av en fördröjning
 *
 * För att komma åt det fördröjda värdet:
 *
 * delayedVar.value()
 * delayedVar.update(var) // sista som sker i tidssteget
 */

class DLLIMPORTEXPORT Delay
{
public:
    Delay();
    Delay(const std::size_t stepDelay, const double initValue=0.0);
    Delay(const double timeDelay, const double Ts, const double initValue=0.0);
    void initilizeValues(const double initValue);
    void update(const double value);
    void setStepDelay(const std::size_t stepDelay, const double initValue=0.0);
    void setTimeDelay(const double timeDelay, const double Ts, const double initValue=0.0);
	double value();
	double value(const std::size_t idx);
private:
    double mInitialValue;
	double mFracStep;
	std::size_t mStepDelay;
	std::deque<double> mValues;
};

#endif // DELAY_H_INCLUDED
