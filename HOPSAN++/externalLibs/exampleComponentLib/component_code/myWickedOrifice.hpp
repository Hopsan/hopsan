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

#ifndef MYWICKEDORIFICE_H
#define MYWICKEDORIFICE_H

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class MyWickedOrifice : public ComponentQ
    {
    private:
        double mKc;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MyWickedOrifice();
        }

        MyWickedOrifice() : ComponentQ()
        {
            mKc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            registerParameter("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", mKc);
        }


        void initialize()
        {
            //Nothing to initialize
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
            double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

            //Orifice equations
            double q2 = mKc*(c1-c2)/(1.0+mKc*(Zc1+Zc2));
            double q1 = -q2;
            double p1 = c1 + q1*Zc1;
            double p2 = c2 + q2*Zc2;

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
            mpP1->writeNode(NodeHydraulic::FLOW, q1);
            mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
            mpP2->writeNode(NodeHydraulic::FLOW, q2);
        }
    };
}

#endif // MYWICKEDORIFICE_H
