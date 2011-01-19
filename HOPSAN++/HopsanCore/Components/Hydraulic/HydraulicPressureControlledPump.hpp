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
        double pdif, speed, qmax, qmin, lp, rp, wp1, Kcp, taov;
        double y0;
        SecondOrderFilter filter;
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_c3;
        Port *mpP1, *mpP2, *mpPREF;

    public:
        static Component *Creator()
        {
            return new HydraulicPressureControlledPump("PressureControlledPump");
        }

        HydraulicPressureControlledPump(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicPressureControlledPump";
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

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpPREF = addPowerPort("PREF", "NodeHydraulic");

            registerParameter("pdif", "Reference pressure difference", "Pa", pdif);
            registerParameter("speed", "Pump speed", "rad/s", speed);
            registerParameter("qmax", "Nomainal maximal flow", "m^3/s", qmax);
            registerParameter("qmin", "Nominal minimal flow", "m^3/s", qmin);
            registerParameter("lp", "Regulator inductance at nominal pressure", "-", lp);
            registerParameter("rp", "Static characteristic at nominal pressure", "-", rp);
            registerParameter("wp1", "Lead frequency of regulator", "rad/s", wp1);
            registerParameter("Kcp", "Leakage coefficient of pump", "-", Kcp);
            registerParameter("taov", "Time constant of control valve", "s", taov);
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

            mpND_c3 = getSafeNodeDataPtr(mpPREF, NodeHydraulic::WAVEVARIABLE);

            double p1 = (*mpND_p1);
            double p2 = (*mpND_p2);
            double q2 = (*mpND_q2);
            double c3 = (*mpND_c3);
            double Zc1 = (*mpND_Zc1);
            double Zc2 = (*mpND_Zc2);

            double gamma, speed1, qp0, lpe, y0;

            gamma = 1 / (Kcp * (Zc1 + Zc2) + 1);
            speed1 = speed;
            if (speed1 < .001) { speed1 = .001; }
            if (p2 < 1.0) { p2 = 1.0; }
            lpe = lp * sqrt(pnom / p2) * (speednom / speed1);
            y0 = q2 * (lpe + rp * taov + Zc2 * gamma / wp1);

            (*mpND_p1) = p1;
            (*mpND_p2) = p2;

            double num[3] = {lpe*taov, lpe+taov*rp, rp};
            double den[3] = {0, 1.0/wp1, 1.0};
            filter.initialize(mTimestep, num, den, pnom-p2+c3, qp0, qmin, -qmax);
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

            double speed1, qmaxl, qminl, lpe, gamma, c1e, c2e, qp, ql, q1, q2;

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

            //Filter
            double num[3] = {lpe*taov, lpe+taov*rp, rp};
            double den[3] = {0, 1.0/wp1, 1.0};
            filter.setMinMax(qminl, qmaxl);
            filter.setNumDen(num, den);
            qp = filter.update(pdif - c2e + c3);

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

            //Write new values to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICPRESSURECONTROLLEDPUMP_HPP_INCLUDED
