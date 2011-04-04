#ifndef MYWICKEDORIFICE_H
#define MYWICKEDORIFICE_H

#include "../../HopsanCore/ComponentEssentials.h"

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
            return new MyWickedOrifice("WickedOrifice");
        }

        MyWickedOrifice(const std::string name) : ComponentQ(name)
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
