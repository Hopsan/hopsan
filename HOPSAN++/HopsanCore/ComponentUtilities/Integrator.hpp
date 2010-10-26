//!
//! @file   Integrator.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Integrator class
//!
//$Id: Integrator.h 1642 2010-08-09 12:54:33Z bjoer $

#ifndef INTEGRATOR_HPP_INCLUDED
#define INTEGRATOR_HPP_INCLUDED

#include <deque>
//#include "../win32dll.h"
#include "Delay.hpp"

namespace hopsan {

    class Integrator
    {
    public:
        Integrator()
        {
            mLastTime = 0.0;
            mIsInitialized = false;
        }

        void initialize(double &rTime, double timestep, double u0=0.0, double y0=0.0)
        {
            mDelayU.setStepDelay(1);
            mDelayY.setStepDelay(1);
            mDelayU.initialize(rTime, u0);
            mDelayY.initialize(rTime, y0);

            mTimeStep = timestep;
            mpTime = &rTime;
            mIsInitialized = true;
        }

        void initializeValues(double u0, double y0)
        {
            mDelayU.initializeValues(u0);
            mDelayY.initializeValues(y0);
        }

        void update(double u)
        {
            if (!mIsInitialized)
            {
                std::cout << "Integrator function has to be initialized" << std::endl;
                assert(false);
            }
            else if (mLastTime != *mpTime)
            {
                //Filter equation
                //Bilinear transform is used
                mDelayY.update(mDelayY.value() + mTimeStep/2.0*(u + mDelayU.value()));
                mDelayU.update(u);

                mLastTime = *mpTime;
            }
        }

    double value(double u)
    {
        update(u);

        return mDelayY.value();
    }

    double value()
    {
        update(mDelayU.valueIdx(1));

        return mDelayY.value();
    }

    private:
        Delay mDelayU, mDelayY;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
    };

}

#endif // INTEGRATOR_H_INCLUDED
