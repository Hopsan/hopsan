#ifndef ELECTRICINTERFACETLM_HPP
#define ELECTRICINTERFACETLM_HPP
//!
//! @file   ElectricInterfaceTLM.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2025-04-16
//!
//! @brief Contains a electric interface component for TLM co-simulation
//!
//$Id$

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup InterfaceComponents
//!
class ElectricInterfaceTLM : public ComponentC
{
    double Zc, dT;
    double *mpu2, *mpi2, *mpP1_c, *mpP1_Zc;

    double u0, u1, u2, i0, i1, i2, t0, t1, t2, ui, ii, ui0, ii0;

    int mInterpolationMethod;

private:
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new ElectricInterfaceTLM();
    }

    void configure()
    {
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        addInputVariable("u2", "received (delayed) current", "A", 0, &mpu2);
        addInputVariable("i2", "receive (delayed) voltage", "V", 0, &mpi2);
        addConstant("Zc", "Characteristic Impedance", "V/A", Zc);
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
        u0 = 0;
        u1 = 0;
        u2 = 0;
        i0 = 0;
        i1 = 0;
        i2 = 0;
        t0 = mTime-2*mTimestep;
        t1 = mTime-mTimestep;
        t2 = mTime;
        ui0 = ui;
        ii0 = i0;
    }

    void simulateOneTimestep()
    {
        if(dT < 1.01*mTimestep && dT > 0.99*mTimestep) {
            ui = (*mpi2);
            ii = (*mpu2);
        }
        else {
            if(u2 != (*mpi2) || i2 != (*mpu2)) {
                u0 = u1;
                u1 = u2;
                u2 = (*mpi2);
                i0 = i1;
                i1 = i2;
                i2 = (*mpu2);
                t0 = t1;
                t1 = t2;
                t2 = mTime;
            }

            double t = mTime-dT;

            if(mInterpolationMethod == 0) {
                ui = interpolateLinear(u1, u2,  t1, t2, t);
                ii = interpolateLinear(i1, i2,  t1, t2, t);
            }
            else {
                ui = interpolateQuadratic(u0, u1, u2,  t0, t1, t2, t);
                ii = interpolateQuadratic(i0, i1, i2,  t0, t1, t2, t);
            }
        }

        (*mpP1_c) = ui + Zc*ii;
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

#endif // ELECTRICINTERFACETLM_HPP
