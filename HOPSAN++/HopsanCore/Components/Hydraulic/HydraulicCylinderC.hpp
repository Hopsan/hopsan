//!
//! @file   HydraulicCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-20
//!
//! @brief Contains a Hydraulic Cylinder of C type
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
        int n;
        double alfa, wfak;

        // Local variables
        double clim, zlim, zlim0, V1, V2, V1min, V2min, pi1, qi1, ci1, pi2, qi2, ci2, Zc1, Zc2, c1_0, ci1_0, c2_0, ci2_0, xi3, vi3, cp1_0, cp2_0, cp1, cp2;

        //Parameters
        double betae, me, V01, V02, A1, A2, sl, cLeak, bp;

        //Special variables for Fortran translation
        int ifl;
        int n1total;
        int n2total;

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

            n = 0;
            alfa = .01;
            wfak = .1;

            ifl = 1;
            n1total = 1;
            n2total = 1;

            betae = 1000000000.0;
            me = 1000.0;
            V01 = 0.0003;
            V01 = 0.0003;
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
            registerParameter("bp", "Damping Coefficient", "[Ns/m]", bp);
            registerParameter("betae", "Bulk Modulus", "[Pa]", betae);
            registerParameter("cLeak", "Leakage Coefficient", "-", cLeak);
        }


        void initialize()
        {
            double p1   = mpP1->readNode(NodeHydraulic::PRESSURE);
            double q1   = mpP1->readNode(NodeHydraulic::FLOW);
            double p2   = mpP2->readNode(NodeHydraulic::PRESSURE);
            double q2   = mpP2->readNode(NodeHydraulic::FLOW);
            double f3   = mpP3->readNode(NodeMechanic::FORCE);
            double x3   = mpP3->readNode(NodeMechanic::POSITION);
            double v3   = mpP3->readNode(NodeMechanic::VELOCITY);

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

            double c1 = p1 - Zc1 * q1;
            double c2 = p2 - Zc2 * q2;

            qi1 = -A1 * vi3;
            qi2 = A2 * vi3;
            ci1 = p1 - Zc1 * (qi1 - cLeak * (p1 - p2));
            ci2 = p2 - Zc2 * (qi2 - cLeak * (p2 - p1));
            double c3 = f3;
            double Zx3 = A1 * A1 * Zc1 + A2 * A2 * Zc2 + bp;

            //Write to nodes
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
            mpP1->writeNode(NodeMechanic::CHARIMP,       Zc1);
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
            mpP2->writeNode(NodeMechanic::CHARIMP,       Zc2);
            mpP3->writeNode(NodeMechanic::WAVEVARIABLE,  c3);
            mpP3->writeNode(NodeMechanic::CHARIMP,       Zx3);
        }

        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double p1  = mpP1->readNode(NodeHydraulic::PRESSURE);
            double q1  = mpP1->readNode(NodeHydraulic::FLOW);
            double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);

            double p2  = mpP2->readNode(NodeHydraulic::PRESSURE);
            double q2  = mpP2->readNode(NodeHydraulic::FLOW);
            double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

            double x3   = mpP3->readNode(NodeMechanic::POSITION);
            double v3   = mpP3->readNode(NodeMechanic::VELOCITY);
            double c3  = mpP3->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx3 = mpP3->readNode(NodeMechanic::CHARIMP);

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
            cp1 = (1 - alfa) * cp1_0 + alfa * cp1;
            cp2 = (1 - alfa) * cp2_0 + alfa * cp2;

            //Calculate force (with limitation function)
            this->limitStroke(&clim, &zlim, &xi3, &vi3, &zlim0, &sl, mTime, mTimestep);
            c3 = cp1*A1 - cp2*A2 + clim;
            Zx3 = A1*A1 * Zc1 + A2*A2 * Zc2 + bp + zlim;

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
            mpP1->writeNode(NodeHydraulic::CHARIMP,      Zc1);
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
            mpP2->writeNode(NodeHydraulic::CHARIMP,      Zc2);
            mpP3->writeNode(NodeMechanic::WAVEVARIABLE, c3);
            mpP3->writeNode(NodeMechanic::CHARIMP,      Zx3);
        }


        //This function was translated from old HOPSAN using F2C. A few manual adjustments were necessary.

        /* ---------------------------------------------------------------- */
        /*     Function that simulate the end of the stroke. If X is */
        /*     smaller than 0 or greater than SL a large spring force will */
        /*     act to force X into the interval again. The spring constant */
        /*     is as high possible without numerical instability. */
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
