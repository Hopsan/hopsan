/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   Integrator.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Integrator class
//!
//$Id$

#ifndef INTEGRATOR_H_INCLUDED
#define INTEGRATOR_H_INCLUDED

#include "win32dll.h"
#include "Delay.hpp"

namespace hopsan {

//! @ingroup ComponentUtilityClasses
class Integrator
{
public:
    inline void initialize(const double timestep, const double u0=0.0, const double y0=0.0)
    {
        mDelayU = u0;
        mDelayY = y0;
        mTimeStep = timestep;
    }

    inline void initializeValues(const double u0, const double y0)
    {
        mDelayU = u0;
        mDelayY = y0;
    }

    //! @brief Updates the integrator one timestep and returns the new value
    //! @param[in] u New input value
    inline double update(const double u)
    {
        //Bilinear transform is used
        mDelayY = mDelayY + mTimeStep/2.0*(u + mDelayU);
        mDelayU = u;
        return mDelayY;
    }

    //! @brief Returns the integrator value
    //! @return The integrated actual value.
    //! @see update(double u)
    inline double value() const
    {
        return mDelayY;
    }

protected:
    double mDelayU, mDelayY;
    double mTimeStep;
};

//! @ingroup ComponentUtilityClasses
class IntegratorWithBackup : public Integrator
{
public:
    inline void initialize(const double timestep, const double u0=0.0, const double y0=0.0)
    {
        Integrator::initialize(timestep, u0, y0);
        setBackupLength(1);
    }

    //! @brief Setup the number of backup steps to remember (size of the backup buffer)
    //! @param[in] nSteps The number of steps to remember
    void setBackupLength(int nSteps)
    {
        mBackupU.initialize(nSteps, mDelayU);
        mBackupY.initialize(nSteps, mDelayY);
    }

    //! @brief Restore the backup at the given step
    //! @param[in] nSteps The number of steps backwards in time to restore (1=last step) must be >=1
    //! @note The function assumes that the backup buffer has been allocated
    //! @see setBackupLength
    inline void restoreBackup(size_t nSteps=1)
    {
        if (nSteps > 0)
        {
            nSteps -= 1;
        }
        mDelayU = mBackupU.getIdx(nSteps);
        mDelayY = mBackupY.getIdx(nSteps);
    }

    //! @brief Pushes a backup of transfer function states into the backup buffer
    //! @note Only the delayed states are backed up, not the current value or the coefficients
    //! @todo Maybe we should backup more things like coefficients, saturated flag, current value, but that will take time at every timestep
    inline void backup()
    {
        mBackupU.update(mDelayU);
        mBackupY.update(mDelayY);
    }

    //! @brief Make a backup of states and then calls update
    //! @param[in] u The new input value
    //! @returns The current transfer function output value after update
    inline double updateWithBackup(double u)
    {
        backup();
        return update(u);
    }

protected:
    Delay mBackupU, mBackupY;

};

}

#endif // INTEGRATOR_H_INCLUDED
