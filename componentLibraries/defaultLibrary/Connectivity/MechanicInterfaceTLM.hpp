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
//! @file   MechaniccInterfaceTLM.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2025-04-16
//!
//! @brief Contains a mechanical interface component for TLM co-simulation
//!
//$Id$

#ifndef MECHANICINTERFACETLM_HPP_INCLUDED
#define MECHANICINTERFACETLM_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicInterfaceTLM : public ComponentC
{
    double Zc, dT;
    double *mpv2, *mpf2, *mpP1_c, *mpP1_Zc;

    double f0, f1, f2, v0, v1, v2, t0, t1, t2, fi, vi, fi0, vi0;

    int mInterpolationMethod;

private:
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicInterfaceTLM();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeMechanic");
        addInputVariable("v2", "received (delayed) velocity", "m/s", 0, &mpv2);
        addInputVariable("f2", "receive (delayed) force", "N", 0, &mpf2);
        addConstant("Zc", "Characteristic Impedance", "Ns/m", Zc);
        addConstant("Td", "Time delay", "s", dT);

        std::vector<HString> interpolationMethods;
        interpolationMethods.push_back("Linear Interpolation");
        interpolationMethods.push_back("Quadratic Interpolation");
        addConditionalConstant("interpolation", "Interpolation Method", interpolationMethods, mInterpolationMethod);
    }

    void initialize()
    {
        mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
        mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
        f0 = 0;
        f1 = 0;
        f2 = 0;
        v0 = 0;
        v1 = 0;
        v2 = 0;
        t0 = mTime-2*mTimestep;
        t1 = mTime-mTimestep;
        t2 = mTime;
        fi0 = fi;
        vi0 = v0;
    }

    void simulateOneTimestep()
    {
        if(dT < 1.01*mTimestep && dT > 0.99*mTimestep) {
            fi = (*mpf2);
            vi = (*mpv2);
        }
        else {
            if(f2 != (*mpf2) || v2 != (*mpv2)) {
                f0 = f1;
                f1 = f2;
                f2 = (*mpf2);
                v0 = v1;
                v1 = v2;
                v2 = (*mpv2);
                t0 = t1;
                t1 = t2;
                t2 = mTime;
            }

            double t = mTime-dT;

            if(mInterpolationMethod == 0) {
                fi = interpolateLinear(f1, f2,  t1, t2, t);
                vi = interpolateLinear(v1, v2,  t1, t2, t);
            }
            else {
                fi = interpolateQuadratic(f0, f1, f2,  t0, t1, t2, t);
                vi = interpolateQuadratic(v0, v1, v2,  t0, t1, t2, t);
            }
        }

        (*mpP1_c) = fi + Zc*vi;
        (*mpP1_Zc) = Zc;
    }

    double interpolateLinear(double x1, double x2, double t1, double t2, double t) {
        return x1+(x2-x1)/(t2-t1)*(t-t1);
    }

    double interpolateQuadratic(double x0, double x1, double x2, double t0, double t1, double t2, double t) {
        (void)t0;
        double theta = (t-t1)/(t2-t1);
        return x0*(theta*(theta-1.0))/2.0 + x1*(1.0-theta*theta) + x2*(theta*(theta+1))/2.0;
    }
};
}

#endif // MECHANICINTERFACETLM_HPP_INCLUDED
