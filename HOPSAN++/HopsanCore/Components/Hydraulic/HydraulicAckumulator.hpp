//!
//! @file   HydraulicAckumulator.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a Hydraulic Ackumulator component with constant bulk modulus
//!
//$Id$

#ifndef HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED
#define HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicAckumulator : public ComponentQ
    {
    private:
        double Pmin, Vtot, Voil, Vgas, Betae, Kappa, Kce;
        double mPrevP2, mPrevC1, mPrevZc1, mPrevQ2;
        double p2, q2, e0, ct;

        double p1, q1, c1, Zc1, out;
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_out;

        Port *mpP1, *mpOut;

    public:
        static Component *Creator()
        {
            return new HydraulicAckumulator("Ackumulator");
        }

        HydraulicAckumulator(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicAckumulator";
            Pmin                   = 1000000.0;
            Vtot                   = 0.005;
            Voil                   = 0.0;
            Vgas                   = Vtot-Voil;
            Betae                  = 1000000000.0;
            Kappa                  = 1.4;
            Kce                    = 0.0000000001;

            mpP1 = addPowerPort("P1", "NodeHydraulic");     //External port
            mpOut = addWritePort("out", "NodeSignal");     //Internal pressure output

            registerParameter("Pmin", "Minimum Internal Pressure", "Pa", Pmin);
            registerParameter("Vtot", "Total Volume", "m^3", Vtot);
            registerParameter("Betae", "Effective Bulk Modulus", "Pa", Betae);
            registerParameter("Kappa", "Polytropic Exponent", "-", Kappa);
            registerParameter("Kce", "Flow-Pressure Coefficient", "(m^3/s)/Pa", Kce);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);

            if (p1 < Pmin)         //User has selected an initial pressure lower than the minimum pressure, so use minimum pressure instead
            {
                p1 = std::max(Pmin, 0.0);
                Vgas = Vtot;                  //Pressure is minimum, so ackumulator is empty
                Voil = 0;
            }
            else
            {
                Vgas = pow(Pmin*pow(Vtot, Kappa)/p1, 1/Kappa);     //Initial gas volume, calculated from initial pressure
                Voil = Vtot - Vgas;
            }
            mPrevQ2 = -q1;           //"Previous" value for q2 first step
            mPrevP2 = p1;
            mPrevC1 = c1;
            mPrevZc1 = Zc1;
        }


        void simulateOneTimestep()
        {
            //Read variables from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);

            //Ackumulator equations

            p2 = mPrevP2;

            e0 = Pmin * pow(Vtot, Kappa);
            ct = Vgas / (p2*Kappa) + (Vtot-Vgas) / Betae;
            q2 = (mPrevQ2 * (2.0*ct*(Kce*mPrevZc1+1.0) - Kce*mTimestep) +
                  2.0*ct*Kce*(c1-mPrevC1)) / (2.0*ct*(Kce*Zc1+1.0) + Kce*mTimestep);
            p1 = c1 - Zc1*q2;
            p2 = p1 - q2/Kce;

            if (p1 < 0.0)       //Cavitation!
            {
                c1 = 0.0;
                Zc1 = 0.0;
                q2 = (mPrevQ2 * (2.0*ct*(Kce*mPrevZc1+1.0) - Kce*mTimestep) +
                      2*ct*Kce*(c1-mPrevC1)) / (2*ct*(Kce*Zc1+1) + Kce*mTimestep);
                p1 = 0.0;
                p2 = -q2/Kce;
            }

            if (p2 < Pmin)     //Too low pressure (ack cannot be less than empty)
            {
                Vgas = Vtot;
                q2 = 0.0;
                p1 = c1;
                p2 = Pmin;
            }

            Vgas = pow(e0/p2, 1/Kappa);
            Voil = Vtot - Vgas;
            q1 = -q2;

            mPrevP2 = p2;
            mPrevQ2 = q2;
            mPrevC1 = c1;
            mPrevZc1 = Zc1;

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_out) = Voil;
        }
    };
}

#endif // HYDRAULICACKUMULATOR_HPP_INCLUDED
