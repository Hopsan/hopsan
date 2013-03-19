/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicPumpPiston.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-12
//!
//! @brief Contains a hydraulic piston with one chamber of C type
//!
//$Id$

#ifndef HYDRAULICPUMPPISTON_HPP__INCLUDED
#define HYDRAULICPUMPPISTON_HPP__INCLUDED

#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicPumpPiston : public ComponentC
{

    private:
        double A1,sl,cLeak,bp,betae,V01, CxLim, ZxLim, wfak, alpha, F0;

        double ci1, cl1;  //Members because old value need to be remembered (c1 and c2 are remembered through nodes)
        double mNum[2];
        double mDen[2];

        //Node data pointers
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1;
        double *mpND_f3, *mpND_x3, *mpND_v3, *mpND_c3, *mpND_Zx3, *mpND_me;

        //Ports
        Port *mpP1, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicPumpPiston();
        }

        void configure()
        {
            //Set member attributes
            wfak = 0.1;
            betae = 1000000000.0;
            V01 = 0.0003;
            A1 = 0.001;
            sl = 1.0;
            cLeak = 0.00000000001;
            bp = 1000.0;
            alpha = 0.1;
            F0 = 1000;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("A_1", "Piston Area", "[m^2]", A1);
            registerParameter("s_l", "Stroke", "[m]", sl);
            registerParameter("V_1", "Dead Volume in Chamber 1", "[m^3]", V01);
            registerParameter("B_p", "Viscous Friction", "[Ns/m]", bp);
            registerParameter("Beta_e", "Bulk Modulus", "[Pa]", betae);
            registerParameter("c_leak", "Leakage Coefficient", "[]", cLeak);
            registerParameter("F_0", "Spring Force", "[N]", F0);
        }


        void initialize()
        {

            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure, 0.0);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow, 0.0);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable, 0.0);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance, 0.0);

            mpND_f3 = getSafeNodeDataPtr(mpP3, NodeMechanic::Force);
            mpND_x3 = getSafeNodeDataPtr(mpP3, NodeMechanic::Position);
            mpND_v3 = getSafeNodeDataPtr(mpP3, NodeMechanic::Velocity);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WaveVariable);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CharImpedance);
            mpND_me = getSafeNodeDataPtr(mpP3, NodeMechanic::EquivalentMass);

            //Declare local variables;
            double p1, x3, v3;
            double Zc1, c3, Zx3;
            double qi1, V1, qLeak, V1min;

            //Read variables from nodes
            p1 = (*mpND_p1);
            x3 = (*mpND_x3);
            v3 = (*mpND_v3);

            //Size of volumes
            V1 = V01+A1*(-x3);
            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*1.0);
            if(V1<V1min) V1 = V1min;

            Zc1 = 3.0 / 2.0 * betae/V1*mTimestep/(1.0-alpha);    //Number of ports in volume is 2 internal plus the external ones
            Zx3 = A1*A1*Zc1 + bp;

            //Internal flows
            qi1 = v3*A1;

            ci1 = p1 + Zc1*qi1;

            //Leakage flow
            qLeak = cLeak*(p1);

            cl1 = p1 + Zc1*(-qLeak);

            c3 = A1*ci1+F0;

            //Write to nodes
            (*mpND_c1) = p1 + Zc1*(*mpND_q1);
            (*mpND_Zc1) = Zc1;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare local variables;
            double c3, Zx3;
            double V1, qLeak, qi1, p1mean, V1min;
            //Read variables from nodes
            double q1 = (*mpND_q1);
            double c1 = (*mpND_c1);
            double Zc1 = (*mpND_Zc1);
            double x3 = (*mpND_x3);
            double v3 = (*mpND_v3);
            double me = (*mpND_me);

            //Leakage flow
            qLeak = cLeak*(cl1)/(1.0+cLeak*(Zc1));

            //Internal flows
            qi1 = v3*A1;

            //Size of volumes
            V1 = V01+A1*(sl/2-x3);
            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*me);
            if(V1<V1min) V1 = V1min;

            // Cylinder volumes are modelled the same way as the multiport volume:
            //   c1 = Wave variable for external port
            //   ci1 = Wave variable for internal (mechanical) port
            //   cl1 = Wave variable for leakage port

            //Volume 1
            Zc1 = 3.0 / 2.0 * betae/V1*mTimestep/(1.0-alpha);    //Number of ports in volume is 2 internal plus the external ones

            p1mean = (ci1 + Zc1*2.0*qi1) + (cl1 + Zc1*2.0*(-qLeak)) + (c1 + 2.0*Zc1*q1);
            p1mean = p1mean/3.0;
            ci1 = std::max(0.0, alpha * ci1 + (1.0 - alpha)*(p1mean*2.0 - ci1 - 2.0*Zc1*qi1));
            cl1 = std::max(0.0, alpha * cl1 + (1.0 - alpha)*(p1mean*2.0 - cl1 - 2.0*Zc1*(-qLeak)));

            //Internal mechanical port
            c3 = A1*ci1+F0;
            Zx3 = A1*A1*Zc1+ bp;

            //Write to nodes
            (*mpND_c1) = alpha * c1 + (1.0 - alpha)*(p1mean*2 - c1 - 2*Zc1*q1);
            (*mpND_Zc1) = Zc1;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }
    };
}

#endif
