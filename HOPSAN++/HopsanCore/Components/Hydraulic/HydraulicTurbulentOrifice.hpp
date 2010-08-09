//!
//! @file   HydraulicTurbulentOrifice.hpp
//! @author Karl Pettersson
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Turbulent Orifice
//! @ingroup HydraulicComponents
//!
#ifndef HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
#define HYDRAULICTURBULENTORIFICE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief Hydraulic orifice with turbulent flow of Q-Type. Uses TurbulentFlowFunction to calculate the flow.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTurbulentOrifice : public ComponentQ
    {
    private:
        double mCq;
        double mA;
        double mKc;
        TurbulentFlowFunction qTurb;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicTurbulentOrifice("TurbulentOrifice");
        }

        HydraulicTurbulentOrifice(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicTurbulentOrificeName";
            mCq = 0.67;
            mA = 0.00001;
            mKc = mCq*mA*sqrt(2.0/890.0);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            registerParameter("Cq", "Flow coefficient", "[-]", mCq);
            registerParameter("A", "Area", "m^3", mA);
        }


        void initialize()
        {
            qTurb.setFlowCoefficient(mKc);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
            double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

            //Orifice equations
            double q2 = qTurb.getFlow(c1,c2,Zc1,Zc2);
            double q1 = -q2;
            double p1 = c1 + q1*Zc1;
            double p2 = c2 + q2*Zc2;

            //Write new values to nodes
            mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
            mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
            mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
            mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);
        }
    };
}

#endif // HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
