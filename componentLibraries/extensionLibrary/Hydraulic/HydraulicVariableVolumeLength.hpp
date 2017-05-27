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

//$Id$

#ifndef HYDRAULICVARIABLEVOLUMELENGTH_HPP
#define HYDRAULICVARIABLEVOLUMELENGTH_HPP

#include "ComponentEssentials.h"
using namespace hopsan;

class HydraulicVariableVolumeLength : public ComponentC
{

private:
    double mBetae;
    double mPh;
    double mAlpha;
    double mWallArea;
    double mDeadVolume;
    double mPrevVolume;
    double mDeltaGain;
    double mInternal_c;
    Port *mpP1, *mpP2, *mpX, *mpOutDeltaP, *mpOutDeltaVf, *mpOutVolume;

public:
    static Component *Creator()
    {
        return new HydraulicVariableVolumeLength();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        addConstant("alpha", "Low pass coefficient to dampen standing delayline waves", "", 0.1 , mAlpha);
        addConstant("Beta_e", "Bulkmodulus", "Pa", 1.0e9, mBetae);
        addConstant("P_high", "High pressure (for animation)", "Pa", 2e7, mPh);
        addConstant("area", "Volume wall area", "Area", 0.001, mWallArea);
        addConstant("deadvol", "Dead volume (when x=0)", "Volume", 1e-6, mDeadVolume);
        addConstant("deltaGain", "dVf gain, (0=off, 1=on)", "", 0.0, mDeltaGain);

        mpX = addInputVariable("x", "Displacement (Must be > 0)", "Position", 1e-3);
        mpOutDeltaP = addOutputVariable("DeltaP", "Pressure change due to volume change", "Pressure");
        mpOutDeltaVf = addOutputVariable("DeltaVf", "Fraction of volume change", "-");
        mpOutVolume = addOutputVariable("Volume", "Internal volume", "Volume");

        disableStartValue(mpP1, NodeHydraulic::WaveVariable);
        disableStartValue(mpP2, NodeHydraulic::WaveVariable);
        disableStartValue(mpP1, NodeHydraulic::CharImpedance);
        disableStartValue(mpP2, NodeHydraulic::CharImpedance);
    }


    void initialize()
    {
        // read displacement, but minimum 0 else negative volume (not possible)
        double x = std::max(readSignalPort(mpX), 0.);
        // Calc volume
        double V = mWallArea * x + mDeadVolume;
        mPrevVolume = V;
        double Zc = mBetae/V*mTimestep/(1.0-mAlpha);

        double c1 = getDefaultStartValue(mpP2,NodeHydraulic::Pressure)+Zc*getDefaultStartValue(mpP2,NodeHydraulic::Flow);
        double c2 = getDefaultStartValue(mpP1,NodeHydraulic::Pressure)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow);

        //Write to nodes
        writeHydraulicPort_cZc(mpP1, c1, Zc);
        writeHydraulicPort_cZc(mpP2, c2, Zc);
        writeOutputVariable(mpOutVolume, V);
    }


    void simulateOneTimestep()
    {
        // Declare local variables
        HydraulicNodeDataValueStructT p1, p2;

        // Get variable values from nodes
        readHydraulicPort_all(mpP1, p1);
        readHydraulicPort_all(mpP2, p2);
//        q1 = mpP1->readNode(NodeHydraulic::Flow);
//        q2 = mpP2->readNode(NodeHydraulic::Flow);
//        c1 = mpP1->readNode(NodeHydraulic::WaveVariable);
//        c2 = mpP2->readNode(NodeHydraulic::WaveVariable);
//        p1 = mpP1->readNode(NodeHydraulic::Pressure);
//        p2 = mpP2->readNode(NodeHydraulic::Pressure);

        // read displacement, but minimum 0 else negative volume (not possible)
        double x = std::max(readSignalPort(mpX), 0.);
        // Calc volume
        double V = mWallArea * x + mDeadVolume;

        // Fraction of volume change
        double newDeltaVf = (V - mPrevVolume)/mPrevVolume;

        // Delta pressure at both ends du to volume change
        double newDeltaP = -mBetae*newDeltaVf*mDeltaGain;

        // Calculate new Zc
        double newZc = mBetae/V*mTimestep/(1.0-mAlpha);

        // Volume equations
        // c21 represent the wave traveling from port 2 at (t-T) into port 1 at (t)
        // c12 represent the wave traveling from port 1 at (t-T) into port 2 at (t)
        //double c21 = p2.p + p2.q*newZc;
        //double c12 = p1.p + p1.q*newZc;
        // We reuse c from the last timestep to avoid using p calculated by Q-Components (may be zero if cavitation)
        double c21 = p2.c + 2.0*newZc*p2.q;
        double c12 = p1.c + 2.0*newZc*p1.q;

        // Apply DeltaP after filtering to avoid filter "forgetting deltaP"
        double c1 = mAlpha*p1.c + (1.0-mAlpha)*c21 + newDeltaP;
        double c2 = mAlpha*p2.c + (1.0-mAlpha)*c12 + newDeltaP;

        // Write new values to nodes
        //mpOutDeltaP->writeNode(NodeSignal::Value, newDeltaP);
        //mpOutDeltaVf->writeNode(NodeSignal::Value, newDeltaVf);
        //mpP1->writeNode(NodeHydraulic::WaveVariable, c1);
        //mpP1->writeNode(NodeHydraulic::CharImpedance, newZc);
        //mpP2->writeNode(NodeHydraulic::WaveVariable, c2);
        //mpP2->writeNode(NodeHydraulic::CharImpedance, newZc);
        writeOutputVariable(mpOutDeltaP, newDeltaP);
        writeOutputVariable(mpOutDeltaVf, newDeltaVf);
        writeOutputVariable(mpOutVolume, V);
        writeHydraulicPort_cZc(mpP1, c1, newZc);
        writeHydraulicPort_cZc(mpP2, c2, newZc);

        mPrevVolume = V;
    }

    void finalize()
    {

    }
};
#endif // HYDRAULICVARIABLEVOLUMELENGTH_HPP

