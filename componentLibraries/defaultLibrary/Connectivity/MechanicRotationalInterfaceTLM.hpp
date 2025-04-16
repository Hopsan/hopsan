#ifndef MECHANICROTATIONALINTERFACETLM_HPP
#define MECHANICROTATIONALINTERFACETLM_HPP

//!
//! @file   MechanicRotationalInterfaceTLM.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2025-04-16
//!
//! @brief Contains a mechanic interface component for TLM co-simulation
//!
//$Id$

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicRotationalInterfaceTLM : public ComponentC
{
    double Zc, dT;
    double *mpw2, *mpT2, *mpP1_c, *mpP1_Zc;

    double T0, T1, T2, w0, w1, w2, t0, t1, t2, Ti, wi, Ti0, wi0;

    int mInterpolationMethod;

private:
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicRotationalInterfaceTLM();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        addInputVariable("w2", "received (delayed) angular velocity", "rad/s", 0, &mpw2);
        addInputVariable("t2", "receive (delayed) torque", "Nm", 0, &mpT2);
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
        T0 = 0;
        T1 = 0;
        T2 = 0;
        w0 = 0;
        w1 = 0;
        w2 = 0;
        t0 = mTime-2*mTimestep;
        t1 = mTime-mTimestep;
        t2 = mTime;
        Ti0 = Ti;
        wi0 = w0;
    }

    void simulateOneTimestep()
    {
        if(dT < 1.01*mTimestep && dT > 0.99*mTimestep) {
            Ti = (*mpT2);
            wi = (*mpw2);
        }
        else {
            if(T2 != (*mpT2) || w2 != (*mpw2)) {
                T0 = T1;
                T1 = T2;
                T2 = (*mpT2);
                w0 = w1;
                w1 = w2;
                w2 = (*mpw2);
                t0 = t1;
                t1 = t2;
                t2 = mTime;
            }

            double t = mTime-dT;

            if(mInterpolationMethod == 0) {
                Ti = interpolateLinear(T1, T2,  t1, t2, t);
                wi = interpolateLinear(w1, w2,  t1, t2, t);
            }
            else {
                Ti = interpolateQuadratic(T0, T1, T2,  t0, t1, t2, t);
                wi = interpolateQuadratic(w0, w1, w2,  t0, t1, t2, t);
            }
        }

        (*mpP1_c) = Ti + Zc*wi;
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

#endif // MECHANICROTATIONALINTERFACETLM_HPP
