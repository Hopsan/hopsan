//!
//! @file   Hydraulic22Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 2/2-valve of Q-type

#ifndef HYDRAULIC22VALVE_HPP_INCLUDED
#define HYDRAULIC22VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 2/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22Valve : public ComponentQ
    {
    private:
        double mCq;
        double md;
        double mf;
        double mxvmax;
        double mOverlap;
        double momegah;
        double mdeltah;
        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        double xv, xpanom, Kcpa, qpa;

        double *cp_ptr, *Zcp_ptr, *ca_ptr, *Zca_ptr, *xvmpND_in, *pp_ptr, *qp_ptr, *pa_ptr, *qa_ptr;
        double cp, Zcp, ca, Zca, xvin, pp, qp, pa, qa;

        Port *mpPP, *mpPA, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic22Valve("Hydraulic 2/2 Valve");
        }

        Hydraulic22Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic22Valve";
            mCq = 0.67;
            md = 0.01;
            mf = 1.0;
            mxvmax = 0.01;
            mOverlap = 0.0;
            momegah = 100.0;
            mdeltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", mCq);
            registerParameter("d", "Diameter", "[m]", md);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", mf);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", mxvmax);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", mOverlap);
            registerParameter("omegah", "Resonance Frequency", "[rad/s]", momegah);
            registerParameter("deltah", "Damping Factor", "[-]", mdeltah);
        }


        void initialize()
        {
            pp_ptr = getSafeNodeDataPtr(mpPP, NodeHydraulic::PRESSURE);
            qp_ptr = getSafeNodeDataPtr(mpPP, NodeHydraulic::FLOW);
            cp_ptr = getSafeNodeDataPtr(mpPP, NodeHydraulic::WAVEVARIABLE);
            Zcp_ptr = getSafeNodeDataPtr(mpPP, NodeHydraulic::CHARIMP);

            pa_ptr = getSafeNodeDataPtr(mpPA, NodeHydraulic::PRESSURE);
            qa_ptr = getSafeNodeDataPtr(mpPA, NodeHydraulic::FLOW);
            ca_ptr = getSafeNodeDataPtr(mpPA, NodeHydraulic::WAVEVARIABLE);
            Zca_ptr = getSafeNodeDataPtr(mpPA, NodeHydraulic::CHARIMP);

            xvmpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);

            //Initiate second order low pass filter
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(momegah*momegah), 2.0*mdeltah/momegah, 1.0};
            myFilter.initialize(mTimestep, num, den, 0, 0, 0, mxvmax);
        }


        void simulateOneTimestep()
        {
            cp = (*cp_ptr);
            Zcp = (*Zcp_ptr);
            ca = (*ca_ptr);
            Zca = (*Zca_ptr);
            xvin = (*xvmpND_in);

            //Dynamics of spool position (second order low pass filter)
            myFilter.update(xvin);
            xv = myFilter.value();

            //Determine flow coefficient
            xpanom = xv;
            Kcpa = mCq*mf*pi*md*xpanom*sqrt(2.0/890.0);

            //Calculate flow
            mQturbpa.setFlowCoefficient(Kcpa);
            qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            if (xv >= 0.0)
            {
                qp = -qpa;
                qa = qpa;
            }
            else
            {
                qp = 0;
                qa = 0;
            }

            //Calculate pressures from flow and impedance
            (*pp_ptr) = cp + qp * Zcp;
            (*qp_ptr) = qp;
            (*pa_ptr) = ca + qa * Zca;
            (*qa_ptr) = qa;
        }
    };
}

#endif // HYDRAULIC22VALVE_HPP_INCLUDED
