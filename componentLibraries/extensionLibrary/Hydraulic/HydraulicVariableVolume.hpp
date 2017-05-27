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

#ifndef HYDRAULICVARIABLEVOLUME_HPP
#define HYDRAULICVARIABLEVOLUME_HPP

#include "ComponentEssentials.h"
using namespace hopsan;

class HydraulicVariableVolume : public ComponentC
{

private:
    double mBetae;
    double mPh;
    double mAlpha;
    double mPrevVolume;
    double mDeltaGain;
    Port *mpP1, *mpP2, *mpV, *mpOutDeltaP, *mpOutDeltaVf;

public:
    static Component *Creator()
    {
        return new HydraulicVariableVolume();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        addConstant("alpha", "Low pass coefficient to dampen standing delayline waves", "", 0.1 , mAlpha);
        addConstant("Beta_e", "Bulkmodulus", "Pa", 1.0e9, mBetae);
        addConstant("P_high", "High pressure (for animation)", "Pa", 2e7, mPh);
        addConstant("deltaGain", "dVf gain, (0=off, 1=on)", "", 1.0, mDeltaGain);


        mpV = addInputVariable("V", "Volume", "Volume", 1.0e-3);
        mpOutDeltaP = addOutputVariable("DeltaP", "Pressure change due to volume change", "Pressure", 0.0);
        mpOutDeltaVf = addOutputVariable("DeltaVf", "Fraction of volume change", "-", 0.0);

        disableStartValue(mpP1, NodeHydraulic::WaveVariable);
        disableStartValue(mpP2, NodeHydraulic::WaveVariable);
        disableStartValue(mpP1, NodeHydraulic::CharImpedance);
        disableStartValue(mpP2, NodeHydraulic::CharImpedance);
    }


    void initialize()
    {
        double V = readSignalPort(mpV);
        mPrevVolume = V;
        double Zc = mBetae/V*mTimestep/(1.0-mAlpha);

        double c1 = getDefaultStartValue(mpP2,NodeHydraulic::Pressure)+Zc*getDefaultStartValue(mpP2,NodeHydraulic::Flow);
        double c2 = getDefaultStartValue(mpP1,NodeHydraulic::Pressure)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow);

        //Write to nodes
        writeHydraulicPort_cZc(mpP1, c1, Zc);
        writeHydraulicPort_cZc(mpP2, c1, Zc);
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
        double V  = readInputVariable(mpV);

        // Fraction of volume change
        double newDeltaVf = (V - mPrevVolume)/mPrevVolume;

        // Delta pressure at both ends du to volume change
        double newDeltaP = -mBetae*newDeltaVf*mDeltaGain;

        // Calculate new Zc
        double newZc = mBetae/V*mTimestep/(1.0-mAlpha);

        // We approximate here, the flow comming from the last timestep remains,
        // it comes from the outside and was not directly affected by the volume change
        // However it is affected by the new Zc
        double c11 = p2.p + p2.q*newZc;
        double c22 = p1.p + p1.q*newZc;

        // Volume equations
        //c10 = c2 + 2.0*Zc * q2;     //These two equations are from old Hopsan
        //c20 = c1 + 2.0*Zc * q1;

        // Apply DeltaP after filtering to avoid filter "forgetting deltaP"
        double c1 = mAlpha*p1.c + (1.0-mAlpha)*c11 + newDeltaP;
        double c2 = mAlpha*p2.c + (1.0-mAlpha)*c22 + newDeltaP;

        // Write new values to nodes
        //mpOutDeltaP->writeNode(NodeSignal::Value, newDeltaP);
        //mpOutDeltaVf->writeNode(NodeSignal::Value, newDeltaVf);
        //mpP1->writeNode(NodeHydraulic::WaveVariable, c1);
        //mpP1->writeNode(NodeHydraulic::CharImpedance, newZc);
        //mpP2->writeNode(NodeHydraulic::WaveVariable, c2);
        //mpP2->writeNode(NodeHydraulic::CharImpedance, newZc);
        writeOutputVariable(mpOutDeltaP, newDeltaP);
        writeOutputVariable(mpOutDeltaVf, newDeltaVf);
        writeHydraulicPort_cZc(mpP1, c1, newZc);
        writeHydraulicPort_cZc(mpP2, c2, newZc);

        mPrevVolume = V;
    }

    void finalize()
    {

    }
};
#endif // HYDRAULICVARIABLEVOLUME_HPP

