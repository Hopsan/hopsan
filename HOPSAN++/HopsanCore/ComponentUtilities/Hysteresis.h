//!
//! @file   Hysteresis.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-01
//!
//! @brief Contains a hysteresis function
//!
//$Id$

#ifndef HYSTERESIS_H_INCLUDED
#define HYSTERESIS_H_INCLUDED

#include <deque>
#include "../win32dll.h"

#include "Delay.h"

namespace hopsan {

class DLLIMPORTEXPORT Hysteresis
{
public:
    Hysteresis();
    void initialize(double &rTime, double initValue=0.0, double hysteresis=0.0);
    void setHysteresis(double hysteresis);
    double value(double u);
    double value();
    void update(double u);

private:
    Delay mDelay;
    double mHysteresis;
};
}

#endif // VALVEHYSTERESIS_H_INCLUDED
