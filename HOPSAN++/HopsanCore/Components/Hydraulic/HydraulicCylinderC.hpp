//!
//! @file   HydraulicCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-20
//!
//! @brief Contains a Hydraulic Cylinder of C type
//! Written by Petter Krus 9005617
//! Revised by ??? 910410
//! Translated to HOPSAN NG by Robert Braun 100120
//!
//$Id$

#ifndef HYDRAULICCYLINDERC_HPP_INCLUDED
#define HYDRAULICCYLINDERC_HPP_INCLUDED

#include <sstream>

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicCylinderC : public ComponentC
{

    private:
        //Local Constants
        double alfa, wfak;

        // "Global" variables
        double zlim0, V1min, V2min, ci1, ci2;

        //Parameters
        double betae, me, V01, V02, A1, A2, sl, cLeak, bp;

        //Node data pointers
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_f3, *mpND_x3, *mpND_v3, *mpND_c3, *mpND_Zx3;

        //Ports
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderC("CylinderC");
        }

        HydraulicCylinderC(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "HydraulicCylinderC";
            alfa = .01;
            wfak = .1;
            betae = 1000000000.0;
            me = 1000.0;
            V01 = 0.0003;
            V02 = 0.0003;
            A1 = 0.001;
            A2 = 0.001;
            sl = 1.0;
            cLeak = 0.0;
            bp = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("A1", "Piston Area 1", "[m^2]", A1);
            registerParameter("A2", "Piston Area 2", "[m^2]", A2);
            registerParameter("sl", "Stroke", "[m]", sl);
            registerParameter("me", "Equivalent Load Mass", "[kg]", me);
            registerParameter("V01", "Dead Volume in Chamber 1", "[m^3]", V01);
            registerParameter("V02", "Dead Volume in Chamber 2", "[m^3]", V02);
            registerParameter("bp", "Viscous Friction", "[Ns/m]", bp);
            registerParameter("betae", "Bulk Modulus", "[Pa]", betae);
            registerParameter("cLeak", "Leakage Coefficient", "-", cLeak);

            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_f3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
            mpND_x3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
            mpND_v3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

            //Read variables from nodes
            double p1 = (*mpND_p1);
            double q1 = (*mpND_q1);
            double p2 = (*mpND_p2);
            double q2 = (*mpND_q2);
            double f3 = (*mpND_f3);
            double x3 = (*mpND_x3);
            double v3 = (*mpND_v3);

            double c1, Zc1, c2, Zc2, c3, Zx3;
            double qi1, qi2, V1, V2, xi3, vi3;

            zlim0 = wfak * me / mTimestep;
            V1min = betae * mTimestep*mTimestep * A1*A1 / (wfak * me);
            V2min = betae * mTimestep*mTimestep * A2*A2 / (wfak * me);
            xi3 = -x3;
            vi3 = -v3;
            V1 = V01 + A1 * xi3;
            V2 = V01 + A2 * (sl - xi3);
            if (V1 < V1min) { V1 = V1min; }
            if (V2 < V2min) { V2 = V2min; }
            Zc1 = betae * mTimestep / V1;
            Zc2 = betae * mTimestep / V2;

            c1 = p1 - Zc1 * q1;
            c2 = p2 - Zc2 * q2;

            qi1 = -A1 * vi3;
            qi2 = A2 * vi3;
            ci1 = p1 - Zc1 * (qi1 - cLeak * (p1 - p2));
            ci2 = p2 - Zc2 * (qi2 - cLeak * (p2 - p1));

            c3 = f3;
            Zx3 = A1 * A1 * Zc1 + A2 * A2 * Zc2 + bp;

             //Write to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare local variables;
            double pi1, pi2, clim, zlim, qi1, qi2, V1, V2, xi3, vi3, c1_0, ci1_0, c2_0, ci2_0, cp1_0, cp2_0;
            double p1, q1, p2, q2, c1, c2, x3, v3, c3, Zc1, Zc2, Zx3;

            //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c1 = (*mpND_c1);
            c2 = (*mpND_c2);
            x3 = (*mpND_x3);
            v3 = (*mpND_v3);

            //Internal mechanical port
            xi3 = -x3;
            vi3 = -v3;

            //Calculate volumes
            V1 = V01 + A1 * xi3;
            V2 = V01 + A2 * (sl - xi3);
            if (V1 < V1min) { V1 = V1min; }
            if (V2 < V2min) { V2 = V2min; }

            //Volume impedances
            Zc1 = betae * mTimestep / V1;
            Zc2 = betae * mTimestep / V2;

            //Calculate internal flow and pressure
            qi1 = -A1 * vi3;
            qi2 = A2 * vi3;
            pi1 = (ci1 + qi1 * Zc1 + cLeak * (ci2 * Zc1 + ci1 * Zc2)) / (cLeak * (Zc1 + Zc2) + 1);
            pi2 = (ci2 + qi2 * Zc2 + cLeak * (ci2 * Zc1 + ci1 * Zc2)) / (cLeak * (Zc1 + Zc2) + 1);
            if (pi1 < 0.0) { pi1 = 0.0; }
            if (pi2 < 0.0) { pi2 = 0.0; }
            qi1 = qi1 - cLeak * (pi1 - pi2);
            qi2 = qi2 - cLeak * (pi2 - pi1);

            //Calucluate wave variables in chamber 1
            c1_0 = pi1 + 2*Zc1*qi1;
            ci1_0 = p1 + 2*Zc1*q1;
            c1 = alfa*c1 + (1 - alfa)*c1_0;
            ci1 = alfa*c1 + (1 - alfa)*ci1_0;

            //Calucluate wave variables in chamber 2
            c2_0 = pi2 + 2*Zc2*qi2;
            ci2_0 = p2 + 2*Zc2*q2;
            c2 = alfa*c2 + (1 - alfa)*c2_0;
            ci2 = alfa*c2 + (1 - alfa)*ci2_0;

            //Calculate mean pressure in chambers
            if (p1 < 0.0) { p1 = 0.0; }
            cp1_0 = (p1 + Zc1*q1 + pi1 + Zc1*qi1) / 2;
            if (p2 < 0.0) { p2 = 0.0; }
            cp2_0 = (p2 + Zc2*q2 + pi2 + Zc2*qi2) / 2;
            ci1 = (1 - alfa) * cp1_0 + alfa * ci1;
            ci2 = (1 - alfa) * cp2_0 + alfa * ci2;

            //Calculate force (with limitation function)
            this->limitStroke(&clim, &zlim, &xi3, &vi3, &zlim0, &sl, mTime, mTimestep);
            c3 = ci1*A1 - ci2*A2 + clim;
            Zx3 = A1*A1 * Zc1 + A2*A2 * Zc2 + bp + zlim;

            //Write to nodes
           (*mpND_c1) = c1;
           (*mpND_Zc1) = Zc1;
           (*mpND_c2) = c2;
           (*mpND_Zc2) = Zc2;
           (*mpND_c3) = c3;
           (*mpND_Zx3) = Zx3;
        }


        //This function was translated from old HOPSAN using F2C. A few manual adjustments were necessary.

        /* ---------------------------------------------------------------- */
        /*     Function that simulate the end of the stroke. If X is        */
        /*     smaller than 0 or greater than SL a large spring force will  */
        /*     act to force X into the interval again. The spring constant  */
        /*     is as high possible without numerical instability.           */
        /* ---------------------------------------------------------------- */

        void limitStroke(double *clp, double *zlim, double *xp, double *sxp, double *zlim0, double *sl, double time, double timestep)
        {
            static double alfa = .5f;

            /* Local variables */
            static double climp, flimp, climp0, ka, cp, kz;

            //  Initialization
            if (time == 0) {
                kz = *zlim0 / timestep;
                if (*xp > *sl) { flimp = -kz * (*xp - *sl); }
                else if (*xp < 0.0) { flimp = -kz * *xp; }
                else { flimp = 0.0; }
                cp = 0.0;
            }

            //Equations
            ka = 1 / (1 - alfa);
            kz = *zlim0 / timestep;
            if (*xp > *sl)
            {
                *zlim = ka * *zlim0;
                flimp = -kz * (*xp - *sl);
                climp0 = flimp - *zlim * *sxp;
            }
            else if (*xp < 0.0)
            {
                *zlim = ka * *zlim0;
                flimp = -kz * *xp;
                climp0 = flimp - *zlim * *sxp;
            }
            else
            {
                *zlim = 0.0;
                climp0 = 0.0;
            }

            // Filtering of the characteristics
            climp = alfa * cp + (1 - alfa) * climp0;
            cp = climp;
            *clp = climp;
        }
    };
}

#endif
