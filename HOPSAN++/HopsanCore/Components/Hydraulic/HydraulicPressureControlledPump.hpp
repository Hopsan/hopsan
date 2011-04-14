//!
//! @file   Hydraulic22Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a pressure controlled pump with second order dynamics
//! Written by Petter Krus 9005617
//! Translated to HOPSAN NG by Robert Braun 101220
//!
//$Id$

#ifndef HYDRAULICPRESSURECONTROLLEDPUMP_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLLEDPUMP_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include <math.h>
#define M_PI       3.14159265358979323846

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicPressureControlledPump : public ComponentQ
    {
    private:
        double pnom, speednom;
        double pdif, speed, qmax, qmin, lp, rp, wp1, Kcp, taov, tp, tm;
        double a1, a2, b1, b2, b3, y1, y2, u1, u2, ud, vd, yd;
        double y0;
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_p3, *mpND_q3, *mpND_c3;
        Port *mpP1, *mpP2, *mpPREF;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureControlledPump("PressureControlledPump");
        }

        HydraulicPressureControlledPump(const std::string name) : ComponentQ(name)
        {
            pnom = 7e6;
            speednom = 125;
            pdif = 1000000;
            speed = 125;
            qmax = 0.00125;
            qmin = 0;
            lp = 70000000;
            rp = 1000000000;
            wp1 = 200;
            Kcp = 0.000000000001;
            taov = 0.001;
            tp = 0.15;
            tm = 0.12;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpPREF = addPowerPort("PREF", "NodeHydraulic");

            registerParameter("p_dif", "Reference pressure difference", "[Pa]", pdif);
            registerParameter("omega_p", "Pump speed", "[rad/s]", speed);
            registerParameter("q_max", "Nomainal maximal flow", "[m^3/s]", qmax);
            registerParameter("q_min", "Nominal minimal flow", "[m^3/s]", qmin);
            registerParameter("l_p", "Regulator inductance at nominal pressure", "[]", lp);
            registerParameter("r_p", "Static characteristic at nominal pressure", "[]", rp);
            registerParameter("omega_p,1", "Lead frequency of regulator", "[rad/s]", wp1);
            registerParameter("C_l,p", "Leakage coefficient of pump", "[]", Kcp);
            registerParameter("tao_v", "Time constant of control valve", "[s]", taov);
            registerParameter("t_p", "Time from min to full displacement", "[s]", tp);
            registerParameter("t_m", "Time from full to min displacement", "[s]", tm);
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

            mpND_p3 = getSafeNodeDataPtr(mpPREF, NodeHydraulic::PRESSURE);
            mpND_q3 = getSafeNodeDataPtr(mpPREF, NodeHydraulic::FLOW);
            mpND_c3 = getSafeNodeDataPtr(mpPREF, NodeHydraulic::WAVEVARIABLE);

            double p1 = (*mpND_p1);
            double c1 = (*mpND_c1);
            double p2 = (*mpND_p2);
            double q2 = (*mpND_q2);
            double c2 = (*mpND_c2);
            double c3 = (*mpND_c3);
            double Zc1 = (*mpND_Zc1);
            double Zc2 = (*mpND_Zc2);

            double gamma, speed1, lpe, y0, qminl, qmaxl, vmin, vmax;

            gamma = 1 / (Kcp * (Zc1 + Zc2) + 1);
            speed1 = speed;
            if (speed1 < .001) { speed1 = .001; }
            if (p2 < 1.0) { p2 = 1.0; }
            lpe = lp * sqrt(pnom / p2) * (speednom / speed1);
            y0 = q2 * (lpe + rp * taov + Zc2 * gamma / wp1);

            (*mpND_p1) = p1;
            (*mpND_p2) = p2;

            qmaxl = qmax * (speed1 / speednom);
            qminl = qmin * (speed1 / speednom);

            vmax = qmaxl * sqrt(fabs(p2 - 1e5) / (pnom * tp));
            vmin = -qmaxl * sqrt(fabs(p2 - 1e5) / (pnom * tm));

            gamma = 1 / (Kcp * (Zc1 + Zc2) + 1);
            double c2e = (Kcp * Zc1 + 1) * gamma * c2 + Kcp * Zc2 * gamma * c1;
            double u = pdif - c2e + c3;
            yd = y0;
            ud = u;
            y1 = y0;
            u1 = u;
            vd = 0.0;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double p1 = (*mpND_p1);
            double c1 = (*mpND_c1);
            double Zc1 = (*mpND_Zc1);
            double p2 = (*mpND_p2);
            double c2 = (*mpND_c2);
            double Zc2 = (*mpND_Zc2);
            double c3 = (*mpND_c3);

            double speed1, qmaxl, qminl, lpe, gamma, c1e, c2e, qp, ql, q1, q2, ymin, ymax, vmin, vmax;

            speed1 = speed;
            if (p2 < 1.0) { p2 = 1.0; }
            if (speed1 < .001) { speed1 = .001; }
            qmaxl = qmax * (speed1 / speednom);
            qminl = qmin * (speed1 / speednom);
            lpe = lp * sqrt(pnom / p2) * (speednom / speed1);
            if (c3 < 0.0) { c3 = 0.0; }
            gamma = 1 / (Kcp * (Zc1 + Zc2) + 1);

            c1e = (Kcp * Zc2 + 1) * gamma * c1 + Kcp * Zc1 * gamma * c2;
            c2e = (Kcp * Zc1 + 1) * gamma * c2 + Kcp * Zc2 * gamma * c1;

            double denom = lpe + rp * taov + Zc2 * gamma / wp1;
            double g1 = lpe * taov / denom;
            double g2 = (rp + Zc2 * gamma) / denom;
            double u = pdif - c2e + c3;
            ymax = qmaxl * denom;
            ymin = qminl * denom;
            vmax = qmaxl * denom * sqrt(fabs(p2 - 1e5) / (pnom * tp));
            vmin = -qmaxl * denom * sqrt(fabs(p2 - 1e5) / (pnom * tm));
            qp = fltppu(u, y0, wp1, g1, g2, ymin, ymax, vmin, vmax) / denom;
//            double z = qp / qmaxl;
//            double vz = v / (qmaxl * denom);

            //Calucluate pressures
            p1 = c1e - Zc1 * gamma * qp;
            p2 = c2e + Zc2 * gamma * qp;

            //Leakage flow
            ql = Kcp * (p2 - p1);
            q2 = qp - ql;
            q1 = -q2;

            //Cavitation check
            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }
            if (c3 < 0.0) { c3 = 0.0; }

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
            (*mpND_p3) = c3;
            (*mpND_q3) = 0.0;
        }


        //High pass filter times an integration, with separate minimum and maximum values for input and output variables. Converted from old Hopsan.

        double fltppu(double u, double y0, double w01,
                      double c1, double c2, double ymin, double ymax, double vmin,
                      double vmax)
        {

                /* Local variables */
            double v;
            double ret_val;
            double alf1, alf2, g, ws;

            y2 = y1;
            y1 = yd;
            u2 = u1;
            u1 = ud;

            /*  Calculation of constants */

            ws = 1 / mTimestep;
            alf2 = c1 * 4*ws*ws;
            alf1 = 2 / (w01 * mTimestep);
            g = c2 + alf2 + ws*2;
            a1 = (c2 - alf2)*2/g;
            a2 = (c2 + alf2 - ws*2)/g;
            b1 = (alf1 + 1)/g;
            b2 = 2 / g;
            b3 = (1 - alf1)/g;

            ret_val = -a1 * y1 - a2 * y2 + b1 * u + b2 * u1 + b3 * u2;
            v = (ret_val - y1) / mTimestep;
            if (v > vmax)
            {
                ret_val = y1 + vmax * mTimestep;
                v = vmax;
            }
            else if (v <= vmin)
            {
                ret_val = y1 + vmin * mTimestep;
                v = vmin;
            }
            if (ret_val > ymax)
            {
                ret_val = ymax;
                v = 0.0;
            }
            else if (ret_val < ymin)
            {
                ret_val = ymin;
                v = 0.0;
            }
            ud = u;
            yd = ret_val;
            vd = v;

            return ret_val;
        }
    };
}

#endif // HYDRAULICPRESSURECONTROLLEDPUMP_HPP_INCLUDED
