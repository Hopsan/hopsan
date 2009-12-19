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

/*
 * Följande exempel fördröjer variablen var i 5 tidssteg
 * 
 * Delay delayedVar(var, 5); // instansiering av en fördröjning
 * 
 * För att komma åt det fördröjda värdet:
 *
 * delayedVar.value()
 */

class Delay
{
public:
    Delay();
    Delay(double& var, const std::size_t stepDelay); //OBS! Init for all not set values are always 0
    Delay(double& var, const double timeDelay, const double Ts);
    void simulateOneTimestep();
    void setDelayVariable(double& var);
    void setStepDelay(const std::size_t stepDelay);
    void setTimeDelay(const double timeDelay, const double Ts);
	double value();
	double value(const std::size_t idx);
private:
	double* mVar;
	std::size_t mStepDelay;
	std::deque<double> mValues;
};

#endif // DELAY_H_INCLUDED
