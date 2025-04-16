#ifndef PNEUMATICINTERFACETLM_HPP
#define PNEUMATICINTERFACETLM_HPP
//!
//! @file   PneumaticInterfaceTLM.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2025-04-16
//!
//! @brief Contains a pneumatic interface component for TLM co-simulation
//!
//$Id$

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup InterfaceComponents
//!
class PneumaticInterfaceTLM : public ComponentC
{
    double Zc, dT;
    double *mpe2, *mpp2, *mpP1_c, *mpP1_Zc;

    double p0, p1, p2, e0, e1, e2, t0, t1, t2, pi, ei, pi0, ei0;

    int mInterpolationMethod;

private:
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new PneumaticInterfaceTLM();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        addInputVariable("e2", "received (delayed) energy flow", "J/s", 0, &mpe2);
        addInputVariable("p2", "receive (delayed) pressure", "Pa", 0, &mpp2);
        addConstant("Zc", "Characteristic Impedance", "sPa/J", Zc);
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
        p0 = 0;
        p1 = 0;
        p2 = 0;
        e0 = 0;
        e1 = 0;
        e2 = 0;
        t0 = mTime-2*mTimestep;
        t1 = mTime-mTimestep;
        t2 = mTime;
        pi0 = pi;
        ei0 = e0;
    }

    void simulateOneTimestep()
    {
        if(dT < 1.01*mTimestep && dT > 0.99*mTimestep) {
            pi = (*mpp2);
            ei = (*mpe2);
        }
        else {
            if(p2 != (*mpp2) || e2 != (*mpe2)) {
                p0 = p1;
                p1 = p2;
                p2 = (*mpp2);
                e0 = e1;
                e1 = e2;
                e2 = (*mpe2);
                t0 = t1;
                t1 = t2;
                t2 = mTime;
            }

            double t = mTime-dT;

            if(mInterpolationMethod == 0) {
                pi = interpolateLinear(p1, p2,  t1, t2, t);
                ei = interpolateLinear(e1, e2,  t1, t2, t);
            }
            else {
                pi = interpolateQuadratic(p0, p1, p2,  t0, t1, t2, t);
                ei = interpolateQuadratic(e0, e1, e2,  t0, t1, t2, t);
            }
        }

        (*mpP1_c) = pi + Zc*ei;
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

#endif // PNEUMATICINTERFACETLM_HPP
