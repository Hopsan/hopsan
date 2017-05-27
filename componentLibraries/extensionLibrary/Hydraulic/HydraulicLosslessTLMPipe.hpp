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

#ifndef HYDRAULICLOSSLESSTLMPIPE_HPP
#define HYDRAULICLOSSLESSTLMPIPE_HPP

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

class HydraulicLosslessTLMPipe : public ComponentC
{

private:
    // Port pointers
    Port *mpP1, *mpP2;
    // Node data pointers
    HydraulicNodeDataPointerStructT mP1, mP2;
    double* mpStiffness, *mpActualLength;

    double mBe, mRho, mDesiredLength, mArea, mLp_wc, mAlpha;
    int mTlmFilterType;

    // Other member variables
    Delay mDelay21, mDelay12;
    FirstOrderLowPassFilter mLP1, mLP2;


public:
    static Component *Creator()
    {
        return new HydraulicLosslessTLMPipe();
    }

    void configure()
    {
        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        addConstant("Be", "Fluid Bulk modulus", "", "", 1e9, mBe);
        addConstant("rho", "Fluid Density", "Density", "", 870, mRho);
        addConstant("length", "Length of part", "Length", 1, mDesiredLength);
        addConstant("area", "Cross sectional area", "Area", "", 0.1, mArea);

        std::vector<HString> conds;
        conds.push_back("Fwd-Euler");
        conds.push_back("Tustin");
        addConditionalConstant("TLMFilterType", "TLM filter type", conds, 0, mTlmFilterType);
        addConstant("alpha", "Euler-fwd LP1 TLM filter (fake damping)", "", "", 0, mAlpha);
        addConstant("lp_wc", "LowPass filter break frequency (fake damping)", "Frequency", "", 1.5E300, mLp_wc);

        addOutputVariable("stiffness", "", "", 0, &mpStiffness);
        addOutputVariable("actual_length", "", "", 0, &mpActualLength);
    }


    void initialize()
    {
        //Assign node data pointers
        getHydraulicPortNodeDataPointers(mpP1, mP1);
        getHydraulicPortNodeDataPointers(mpP2, mP2);

        //Initialization

        // Calculate delay length
        const double wavespeed = sqrt(mBe/mRho);
        const double desiredDelayT = mDesiredLength/wavespeed;

        // (desiredDelayT-mTimestep) since the c-components have one time step delay "built-in"
        mDelay12.initialize(desiredDelayT-mTimestep, mTimestep, mP2.c());
        mDelay21.initialize(desiredDelayT-mTimestep, mTimestep, mP1.c());

        const double numSteps = double(mDelay21.getSize()+1);
        const double perceivedLength = wavespeed*numSteps*mTimestep;
        addWarningMessage("Length diff: "+to_hstring(mDesiredLength-perceivedLength));

        double Zc = mBe/(mDesiredLength*mArea)*mTimestep;
        if (mTlmFilterType == 1)
        {
            mAlpha = 1.0 / (1.0+mTimestep*mLp_wc);
            mLP1.initialize(mTimestep, mLp_wc, mP1.c(), mP1.c());
            mLP2.initialize(mTimestep, mLp_wc, mP2.c(), mP2.c());
            addInfoMessage("WC: "+to_hstring(mLP1.breakFrequency())+" Equivalent alpha: "+to_hstring(mAlpha));
        }

        // Compensation for steady-state gain, in filtered TLM integrator
        //Zc /= (1.0-mAlpha);
        Zc *= (numSteps+mAlpha-numSteps*mAlpha)/(numSteps-numSteps*mAlpha);

        mP1.rZc() = Zc;
        mP2.rZc() = Zc;

        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        double c12 = mDelay12.update(mP2.c()+2.*mP2.q()*mP2.Zc());
        double c21 = mDelay21.update(mP1.c()+2.*mP1.q()*mP1.Zc());
        if (mTlmFilterType == 0)
        {
            mP1.rc() = (1.0-mAlpha)*c12+mAlpha*mP1.c();
            mP2.rc() = (1.0-mAlpha)*c21+mAlpha*mP2.c();
        }
        else
        {
            mP1.rc() = mLP1.update(c12);
            mP2.rc() = mLP2.update(c21);
        }
    }
};
}

#endif // HYDRAULICLOSSLESSTLMPIPE_HPP

