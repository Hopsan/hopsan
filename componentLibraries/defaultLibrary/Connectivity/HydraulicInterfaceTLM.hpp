#ifndef HYDRAULICINTERFACETLM_HPP
#define HYDRAULICINTERFACETLM_HPP

//!
//! @file   HydraulicInterfaceTLM.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2025-04-16
//!
//! @brief Contains a hydraulic interface component for TLM co-simulation
//!
//$Id$

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup InterfaceComponents
//!
class HydraulicInterfaceTLM : public ComponentC
{
    double Zc, dT;
    double *mpq2, *mpp2, *mpP1_c, *mpP1_Zc;

    double p0, p1, p2, q0, q1, q2, t0, t1, t2, pi, qi, pi0, qi0;

    int mInterpolationMethod;

private:
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new HydraulicInterfaceTLM();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        addInputVariable("q2", "received (delayed) flow", "m^3/s", 0, &mpq2);
        addInputVariable("p2", "receive (delayed) pressure", "Pa", 0, &mpp2);
        addConstant("Zc", "Characteristic Impedance", "sPa/m^3", Zc);
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
        q0 = 0;
        q1 = 0;
        q2 = 0;
        t0 = mTime-2*mTimestep;
        t1 = mTime-mTimestep;
        t2 = mTime;
        pi0 = pi;
        qi0 = q0;
    }

    void simulateOneTimestep()
    {
        if(dT < 1.01*mTimestep && dT > 0.99*mTimestep) {
            pi = (*mpp2);
            qi = (*mpq2);
        }
        else {
            if(p2 != (*mpp2) || q2 != (*mpq2)) {
                p0 = p1;
                p1 = p2;
                p2 = (*mpp2);
                q0 = q1;
                q1 = q2;
                q2 = (*mpq2);
                t0 = t1;
                t1 = t2;
                t2 = mTime;
            }

            double t = mTime-dT;

            if(mInterpolationMethod == 0) {
                pi = interpolateLinear(p1, p2,  t1, t2, t);
                qi = interpolateLinear(q1, q2,  t1, t2, t);
            }
            else {
                pi = interpolateQuadratic(p0, p1, p2,  t0, t1, t2, t);
                qi = interpolateQuadratic(q0, q1, q2,  t0, t1, t2, t);
            }
        }

        (*mpP1_c) = pi + Zc*qi;
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

#endif // HYDRAULICINTERFACETLM_HPP
