//!
//! @file   HydraulicPressureControlledValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-13
//!
//! @brief Contains a hydraulic pressure controlled valve with first order dynamics
//!
//$Id$

#ifndef HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureControlledValve : public ComponentQ
    {
    private:
        double x0, x0max, pref, tao, Kcs, Kcf, Cs, Cf, pnom, qnom, ph;
        double mPrevX0;
        TurbulentFlowFunction mTurb;
        ValveHysteresis mHyst;
        SecondOrderTransferFunction mFilterLP;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2,
               *mpND_p_open, *mpND_c_open, *mpND_p_close, *mpND_c_close;

        Port *mpP1, *mpP2, *mpP_OPEN, *mpP_CLOSE;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureControlledValve("PressureControlledValve");
        }

        HydraulicPressureControlledValve(const std::string name) : ComponentQ(name)
        {
            pref = 2000000;
            tao = 0.01;
            Kcs = 0.00000001;
            Kcf = 0.00000001;
            qnom = 0.001;
            ph = 500000;
            pnom = 7e6f;
            Cs = sqrt(pnom)/Kcs;
            Cf = 1/(Kcf * sqrt(pnom));
            x0max = qnom/sqrt(pnom);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP_OPEN = addPowerPort("P_OPEN", "NodeHydraulic");
            mpP_CLOSE = addPowerPort("P_CLOSE", "NodeHydraulic");

            registerParameter("pref", "Reference Opening Pressure", "[Pa]", pref);
            registerParameter("tao", "Time Constant of Spool", "[s]", tao);
            registerParameter("kcs", "Steady State Characteristic due to Spring", "[(m^3/s)/Pa]", Kcs);
            registerParameter("kcf", "Steady State Characteristic due to Flow Forces", "[(m^3/s)/Pa]", Kcf);
            registerParameter("qnom", "Flow with Fully Open Valve and pressure drop Pnom", "[m^3/s]", qnom);
            registerParameter("ph", "Hysteresis Width", "[Pa]", ph);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            mpND_p_open = getSafeNodeDataPtr(mpP_OPEN, NodeHydraulic::PRESSURE);
            mpND_c_open = getSafeNodeDataPtr(mpP_OPEN, NodeHydraulic::WAVEVARIABLE);

            mpND_p_close = getSafeNodeDataPtr(mpP_CLOSE, NodeHydraulic::PRESSURE);
            mpND_c_close = getSafeNodeDataPtr(mpP_CLOSE, NodeHydraulic::WAVEVARIABLE);

            x0 = 0.00001;

            mPrevX0 = 0.0;

            double wCutoff = 1 / tao;      //Ska det vara Timestep/Tao?
            //double wCutoff = 100;     DEBUG
            double num [3] = {1.0, 0.0, 0.0};
            double den [3] = {1.0, 1.0/wCutoff, 0.0};
            mFilterLP.initialize(0.0,0.0);
            mFilterLP.setCoefficients(num, den, mTimestep);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, p_open, c_open, p_close, c_close;
            double b1, xs, xh, xsh;
            bool cav = false;

            //Get variable values from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            p_open = (*mpND_p_open);
            c_open = (*mpND_c_open);
            p_close = (*mpND_p_close);
            c_close = (*mpND_c_close);

            //Equations

            /* Equations */

            b1 = Cs+Cf*(p1-p2);        //Help Variable, equals sqrt(p1-p2)/Kctot

            // Spool position calculation
            xs = (p_open - pref - p_close) / b1;

            xh = ph/b1;
            xsh = mHyst.getValue(xs, xh, mPrevX0);
            mFilterLP.update(xsh);
            x0 = mFilterLP.value();
            if (xsh > x0max)
            {

                xsh = x0max;
            }
            else if (xsh < 0)
            {
                xsh = 0;
            }


            // Turbulent Flow Calculation
            mTurb.setFlowCoefficient(x0);
            q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
            q1 = -q2;

            // Pressure Calulation
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;
            p_open = c_open;
            p_close = c_close;

            // Check for cavitation
            if (p1 < 0.0)
            {
                c1 = 0.f;
                Zc1 = 0.0;
                cav = true;
            }
            if (p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if (cav)        //Cavitatiaon, redo calculations with new c and Zc
            {
                xs = (p_open - pref - p_close)/b1;
                xh = ph / b1;
                //if (mTime == 0) { xs = mX0; }
                xsh = mHyst.getValue(xs, xh, mPrevX0);
                //! @todo This won't work, filter cannot be updated twice in the same time step with different values...
                //mFilterLP.update(xsh);
                //mX0 = mFilterLP.getValue();
                x0 = xsh;
                if (x0 > x0max)
                {
                    x0 = x0max;
                }
                else if (x0 < 0)
                {
                    x0 = 0;
                }
                mTurb.setFlowCoefficient(x0);
                q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
                q1 = -q2;
                p1 = c1 + Zc1 * q1;
                p2 = c2 + Zc2 * q2;
                if (p1 < 0.0) { p1 = 0.0; }
                if (p2 < 0.0) { p2 = 0.0; }
            }

            mPrevX0 = x0;

            //Write new values to nodes

            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_p_open) = p_open;
            (*mpND_p_close) = p_close;
        }
    };
}

#endif // HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED
