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

#ifndef HYDRAULICVARIABLEVOLUMELENGTHMP_HPP
#define HYDRAULICVARIABLEVOLUMELENGTHMP_HPP

#include "ComponentEssentials.h"
#include <vector>
using namespace hopsan;

class HydraulicVariableVolumeLengthMP : public ComponentC
{

private:
    double mBetae;
    double mPh;
    double mAlpha;
    double mWallArea;
    double mDeadVolume;
    Port *mpP1, *mpX, *mpOutVolume;
    size_t mNumPorts;
    std::vector<HydraulicNodeDataPointerStructT> mvpP1;

public:
    static Component *Creator()
    {
        return new HydraulicVariableVolumeLengthMP();
    }

    void configure()
    {
        mpP1 = addPowerMultiPort("P1", "NodeHydraulic");

        addConstant("alpha", "Low pass coefficient to dampen standing delayline waves", "", 0.01 , mAlpha);
        addConstant("Beta_e", "Bulkmodulus", "Pa", 1.0e9, mBetae);
        addConstant("P_high", "High pressure (for animation)", "Pa", 2e7, mPh);
        addConstant("area", "Volume wall area", "Area", 0.001, mWallArea);
        addConstant("deadvol", "Dead volume (when x=0)", "Volume", 1e-6, mDeadVolume);

        mpX = addInputVariable("x", "Displacement (Must be > 0)", "Position", 1e-3);
        mpOutVolume = addOutputVariable("Volume", "Internal volume", "Volume");

        disableStartValue(mpP1, NodeHydraulic::WaveVariable);
        disableStartValue(mpP1, NodeHydraulic::CharImpedance);
    }


    void initialize()
    {
        // Setup node data pointers
        getHydraulicMultiPortNodeDataPointers(mpP1, mvpP1);
        mNumPorts = mpP1->getNumPorts();

        // read displacement, but minimum 0 else negative volume (not possible)
        double x = std::max(readSignalPort(mpX), 0.);
        // Calc volume
        double V = mWallArea * x + mDeadVolume;

        double Zc = double(mNumPorts)*mBetae/(2.0*V)*mTimestep;
        Zc *= 1.0/(1.0-mAlpha); // Compensation for steady-state filter (assuming 1 Ts delay)

        double cAvg=0;
        for (size_t i=0; i<mNumPorts; ++i)
        {
            //! @todo getDefaultStartValue should use start values from the multiparent port not the individual ports, however the getDefaultStartValue function is overloaded for multiports so it will allways bypass the parent port
            mvpP1[i].rp() = getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i);
            mvpP1[i].rq() = getDefaultStartValue(mpP1, NodeHydraulic::Flow, i);
            cAvg         += getDefaultStartValue(mpP1, NodeHydraulic::Pressure, i)+Zc*getDefaultStartValue(mpP1,NodeHydraulic::Flow, i);
            mvpP1[i].rZc() = Zc;
        }
        cAvg = cAvg/double(mNumPorts);

        // Write out wave variables
        for (size_t i=0; i<mNumPorts; ++i)
        {
            mvpP1[i].rc() = cAvg*2.0 - mvpP1[i].p() - Zc*mvpP1[i].q();
        }

        writeOutputVariable(mpOutVolume, V);
    }


    void simulateOneTimestep()
    {
        // Read displacement, but minimum 0 else negative volume (not possible)
        double x = std::max(readSignalPort(mpX), 0.);
        // Calc volume
        double V = mWallArea * x + mDeadVolume;

        // Calculate new Zc
        double Zc = double(mNumPorts)*mBetae/(2.0*V)*mTimestep/(1.0-mAlpha);

        // Volume equations
        double cAvg=0;
        for (size_t i=0; i<mNumPorts; ++i)
        {
            cAvg += mvpP1[i].c() + 2.0*Zc*mvpP1[i].q();
        }
        cAvg = cAvg/double(mNumPorts);

        // Calculate new C and write to nodes in the same loop
        for (size_t i=0; i<mNumPorts; ++i)
        {
            double c0 = cAvg*2.0 - (mvpP1[i].c() + 2.0*Zc*mvpP1[i].q());
            mvpP1[i].rc() = mAlpha*mvpP1[i].c() + (1.0-mAlpha)*c0; // Write filtered value to node
            mvpP1[i].rZc() = Zc;
        }
        writeOutputVariable(mpOutVolume, V);
    }

    void finalize()
    {

    }
};
#endif // HYDRAULICVARIABLEVOLUMELENGTHMP_HPP

