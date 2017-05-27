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

#ifndef MECHANICSTEEL_HPP
#define MECHANICSTEEL_HPP

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

inline double square(const double val) {return val*val;}

class MechanicSteel : public ComponentC
{

private:
    // Port pointers
    Port *mpP1, *mpP2;
    // Node data pointers
    MechanicNodeDataPointerStructT mP1, mP2;
    double* mpStiffness, *mpStiffnessPerStep, *mpActualLength, *mpMeanVel, *mpMeanC12, *mpMeanC21,  *mpInternalWaveKineticEnergy;

    double mKs, mRho, mLength, mArea, mE, mLPwc, mMass, mAlpha;
    double mMeanC12, mMeanC21, mInternalWaveKineticEnergy;
    int mForceNumDelaySteps;
    int mPreserveType;
    int mTlmFilterType;

    // Other member variables
    Delay mDelay12, mDelay21;
    FirstOrderLowPassFilter mLP12, mLP21;

public:
    static Component *Creator()
    {
        return new MechanicSteel();
    }

    void configure()
    {
        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeMechanic");
        mpP2 = addPowerPort("P2", "NodeMechanic");

        addConstant("E", "Youngs modulus", "", "", 210e9, mE);
        //addConstant("Ks", "Bulk modulus (not used)", "", "", 160e9, mKs);
        addConstant("rho", "Density", "Density", "", 7850, mRho);
        addConstant("Length", "Length of part", "Length", 1, mLength);
        addConstant("Area", "Cross sectional area of part", "Area", "", 0.1, mArea);

        addConstant("nsteps", "Internal delaybuffer length (-1 = auto), actual delay will be nsteps+1", "", -1, mForceNumDelaySteps);

        std::vector<HString> conds;
        conds.push_back("Fwd-Euler");
        conds.push_back("Tustin");
        addConditionalConstant("TLMFilterType", "TLM filter type", conds, 0, mTlmFilterType);
        addConstant("alpha", "Euler-fwd LP1 TLM filter (fake damping)", "", "", 0, mAlpha);
        addConstant("lp_wc", "LowPass filter break frequency (fake damping)", "Frequency", "", 1.5E300, mLPwc);

        conds.clear();
        conds.push_back("Char.Imp");
        conds.push_back("Inductance");
        conds.push_back("Capacitance");
        addConditionalConstant("PreserveType", "Preserve Type", conds, 0, mPreserveType);

        addOutputVariable("stiffness", "", "", &mpStiffness);
        addOutputVariable("stiffness_per_step", "", "", &mpStiffnessPerStep);
        addOutputVariable("actual_length", "", "Length", &mpActualLength);
        addOutputVariable("mean_vel", "", "Velocity", &mpMeanVel);
        addOutputVariable("mean_c12", "", "", &mpMeanC12);
        addOutputVariable("mean_c21", "", "", &mpMeanC21);
        addOutputVariable("internalMeanKE", "", "Energy", &mpInternalWaveKineticEnergy);
    }


    void initialize()
    {
        // Assign node data pointers
        getMechanicPortNodeDataPointers(mpP1, mP1);
        getMechanicPortNodeDataPointers(mpP2, mP2);

        // Initialization

        // Wave speed wavespeed = sqrt(Ks/rho)
        // delayT = Length/wavespeed
        // stiffness = E*A0/L0
        const double wavespeed = sqrt(mE/mRho);
        const double delayT_desired = mLength/wavespeed;
        const double stiffness_desired = mE*mArea/mLength;
        const double mass_desired = mRho*mArea*mLength;
        const double zc_desired = sqrt(mass_desired*stiffness_desired);
        mMass = mass_desired;

        // (delayT_desired-mTimestep) since the component has one timestep delay "built-in"
        mDelay12.initialize(delayT_desired-mTimestep, mTimestep, 0); // Dummy init, real init below
        // If set, force override
        if (mForceNumDelaySteps > 0)
        {
            mDelay12.initialize(mForceNumDelaySteps, 0); // Dummy init, real init below
            addWarningMessage("Forcing num delay steps to: "+to_hstring(mForceNumDelaySteps+1));
        }
        // We add one, to the length, since we have a "inherent" TLM delay in C-components
        const double numSteps = double(mDelay12.getSize()+1);

        // Calculate perceived  mass and length
        const double Length_percived = wavespeed*numSteps*mTimestep;
        const double Mass_percived = Length_percived*mArea*mRho;

        const double Zc_org = sqrt(mass_desired*stiffness_desired);
        const double Zc_preserve_capacitance = numSteps*mTimestep * mE*mArea/mLength;
        const double Zc_preserve_inductance = mass_desired / (numSteps*mTimestep);

        addInfoMessage("Zc_org: "+to_hstring(Zc_org));
        addInfoMessage("Zc_preserve_inductance: "+to_hstring(Zc_preserve_inductance));
        addInfoMessage("Zc_preserve_capacitance: "+to_hstring(Zc_preserve_capacitance));

        double stiffness_percived = mE*mArea/Length_percived;
        double Zx;

        // Compensation for steady-state gain, in filtered TLM integrator
        if (mTlmFilterType == 1)
        {
            mAlpha = 1.0 / (1.0+mTimestep*mLPwc);
        }

        if (mPreserveType == 0) /* Preserve Zc */
        {
            Zx = Zc_org;
            //Zx *= (numSteps+mAlpha-numSteps*mAlpha)/(numSteps-numSteps*mAlpha); //! @todo this will actually distort Zc
        }
        else if (mPreserveType == 1) /* Preserve Inductance */
        {
            mMass = mass_desired;
            Zx = Zc_preserve_inductance;
            Zx /= (numSteps+mAlpha-numSteps*mAlpha)/(numSteps-numSteps*mAlpha);
        }
        else /* Preserve Capacitance */
        {
            mMass = Mass_percived;
            Zx = Zc_preserve_capacitance;
            stiffness_percived = Zx/(numSteps*mTimestep);
            Zx *= (numSteps+mAlpha-numSteps*mAlpha)/(numSteps-numSteps*mAlpha);
        }

        mP1.rZc() = Zx;
        mP2.rZc() = Zx;

        addInfoMessage("WaveSpeed: "+to_hstring(wavespeed));
        addInfoMessage("Perceived Mass: "+to_hstring(Mass_percived));
        addInfoMessage("Perceived Length: "+to_hstring(Length_percived));

        addInfoMessage("Zc diff:"+to_hstring(Zx-zc_desired));
        addInfoMessage("Stiffness: "+to_hstring(stiffness_percived));

        addInfoMessage("Length diff: "+to_hstring(Length_percived-mLength));
        addInfoMessage("Mass diff: "+to_hstring(Mass_percived-mass_desired));
        addInfoMessage("Stiffness diff: "+to_hstring(stiffness_percived-stiffness_desired));

        // Calculate initial characteristics C based on start values of v and f
        double c12i = mP2.c();
        double c21i = mP1.c();
        if ( (fabs(mP1.v()) > 0) || (fabs(mP2.v()) > 0) ||
             (fabs(mP1.f()) > 0) || (fabs(mP2.f()) > 0) )
        {
            c12i = mP2.f() + mP2.v()*Zx;
            c21i = mP1.f() + mP1.v()*Zx;
            mP1.rc() = c12i;
            mP2.rc() = c21i;
        }
        mMeanC12 = mP1.c();
        mMeanC21 = mP2.c();

        // (delayT_desired-mTimestep) since the component has one timestep delay "built-in"
        mDelay12.initialize(delayT_desired-mTimestep, mTimestep, c12i);
        mDelay21.initialize(delayT_desired-mTimestep, mTimestep, c21i);
        if (mForceNumDelaySteps > 0)
        {
            mDelay12.initialize(mForceNumDelaySteps, c12i);
            mDelay21.initialize(mForceNumDelaySteps, c21i);
        }

        if (mTlmFilterType == 0)
        {
            addInfoMessage("Euler-fwd filter alpha: "+to_hstring(mAlpha));
        }
        else
        {
            mLP12.initialize(mTimestep, mLPwc, mDelay12.getNewest(), mDelay12.getNewest());
            mLP21.initialize(mTimestep, mLPwc, mDelay21.getNewest(), mDelay21.getNewest());
            addInfoMessage("Tustin filter wc: "+to_hstring(mLP12.breakFrequency())+" Equivalent alpha: "+to_hstring(mAlpha));
        }

        addInfoMessage("DelayBufferLength: "+to_hstring(mDelay12.getSize()));
        addInfoMessage("Desired delayT: "+to_hstring(delayT_desired));
        addInfoMessage("Perceived delayT: "+to_hstring(numSteps*mTimestep));

        writeOutputVariable(mpMeanVel, ( (mMeanC21-mMeanC12)/(2.0*mP1.Zc()) ) );
        writeOutputVariable(mpStiffness, stiffness_percived);
        writeOutputVariable(mpStiffnessPerStep, stiffness_percived*numSteps);
        writeOutputVariable(mpActualLength, -(mP1.x()+mP2.x())); // Both - due to sign convention
        writeOutputVariable(mpMeanC12, mMeanC12);

//        mSumV12 = (c12i/Zx)*(c12i/Zx)*n;
//        mSumV21 = (c21i/Zx)*(c21i/Zx)*n;

//        mSumV12 = (c12i/Zx)*(c12i/Zx)*n*mMass/(2.0*n);
//        mSumV21 = (c21i/Zx)*(c21i/Zx)*n*mMass/(2.0*n);

        const double pmass = mMass/(numSteps*2.0); // The mass of every "individual particle" in the delay line
        mInternalWaveKineticEnergy = (square(c12i/Zx)+square(c21i/Zx))*pmass/2.0*numSteps;
        writeOutputVariable(mpInternalWaveKineticEnergy, mInternalWaveKineticEnergy);

        //simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        const double c12prev = mP1.c();
        const double c21prev = mP2.c();

        const double c2in = mP2.f()+mP2.v()*mP2.Zc();
        const double c1in = mP1.f()+mP1.v()*mP1.Zc();
//        const double c2in = mP2.c()+2*mP2.v()*mP2.Zc();
//        const double c1in = mP1.c()+2*mP1.v()*mP1.Zc();

        const double c12 = mDelay12.update(c2in);
        const double c21 = mDelay21.update(c1in);

        const double n = mDelay12.getSize()+1;
//        mMeanC12 = (mMeanC12*n - c12 + c2in)/n;
//        mMeanC21 = (mMeanC21*n - c21 + c1in)/n;
        mMeanC12 = (mMeanC12*n - c12prev + c2in)/n;
        mMeanC21 = (mMeanC21*n - c21prev + c1in)/n;

//        mSumV12 += (c2in/mP1.Zc())*(c2in/mP1.Zc()) - (c12/mP1.Zc())*(c12/mP1.Zc());
//        mSumV21 += (c1in/mP1.Zc())*(c1in/mP1.Zc()) - (c21/mP1.Zc())*(c21/mP1.Zc());

//        mSumV12 += (c2in/mP1.Zc())*(c2in/mP1.Zc())*mMass/(2.0*n) - (c12/mP1.Zc())*(c12/mP1.Zc())*mMass/(2.0*n);
//        mSumV21 += (c1in/mP1.Zc())*(c1in/mP1.Zc())*mMass/(2.0*n) - (c21/mP1.Zc())*(c21/mP1.Zc())*mMass/(2.0*n);

//        double meanCin = (c2in-c1in)/2.0;
//        double meanCout = (c21-c12)/2.0;
//        mSumV12 += (meanCin/mP1.Zc())*(meanCin/mP1.Zc())*mMass/(2.0*n) - (meanCout/mP1.Zc())*(meanCout/mP1.Zc())*mMass/(2.0*n);
//        mSumV21 += (meanCin/mP1.Zc())*(meanCin/mP1.Zc())*mMass/(2.0*n) - (meanCout/mP1.Zc())*(meanCout/mP1.Zc())*mMass/(2.0*n);

        double zc = mP1.Zc();
        const double pmass = mMass/(2.0*n); // The mass of every "individual particle" in the delay line
//        mInternalWaveKineticEnergy += ((c2in/zc)*(c2in/zc) + (c1in/zc)*(c1in/zc) - (c12/zc)*(c12/zc) - (c21/zc)*(c21/zc))*pmass/2.0;
        mInternalWaveKineticEnergy += (square(c2in/zc) + square(c1in/zc) - square(c12prev/zc) - square(c21prev/zc))*pmass/2.0;

        if (mTlmFilterType == 0)
        {
            mP1.rc() = c12*(1-mAlpha)+mP1.c()*mAlpha;
            mP2.rc() = c21*(1-mAlpha)+mP2.c()*mAlpha;
        }
        else
        {
            mP1.rc() = mLP12.update(c12);
            mP2.rc() = mLP21.update(c21);
        }

        writeOutputVariable(mpActualLength, -(mP1.x()+mP2.x())); // Both - due to sign convention
        writeOutputVariable(mpMeanVel, ( (mMeanC21-mMeanC12)/(2.0*mP1.Zc()) ) );
        writeOutputVariable(mpMeanC12, mMeanC12);
        writeOutputVariable(mpMeanC21, mMeanC21);
        writeOutputVariable(mpInternalWaveKineticEnergy, mInternalWaveKineticEnergy);
    }
};
}

#endif // MECHANICSTEEL_HPP

