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
        double Cq;
        double A;
        double Kc;
        TurbulentFlowFunction qTurb;

        double *p1_ptr, *q1_ptr, *c1_ptr, *Zc1_ptr, *p2_ptr, *q2_ptr, *c2_ptr, *Zc2_ptr;
        double q, c1, Zc1, c2, Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicTurbulentOrifice("TurbulentOrifice");
        }

        HydraulicTurbulentOrifice(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicTurbulentOrificeName";
            Cq = 0.67;
            A = 0.00001;
            Kc = Cq*A*sqrt(2.0/890.0);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            registerParameter("Cq", "Flow coefficient", "[-]", Cq);
            registerParameter("A", "Area", "m^3", A);
        }


        void initialize()
        {
            p1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            c1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc1_ptr = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            p2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::FLOW);
            c2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc2_ptr = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);

            Kc = Cq*A*sqrt(2.0/890.0);
            qTurb.setFlowCoefficient(Kc);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*c1_ptr);
            Zc1 = (*Zc1_ptr);
            c2 = (*c2_ptr);
            Zc2 = (*Zc2_ptr);

            //Orifice equations
            q = qTurb.getFlow(c1,c2,Zc1,Zc2);

            //Write new variables to nodes
            (*p1_ptr) = c1 + q*Zc1;
            (*q1_ptr) = q;
            (*p2_ptr) = c2 - q*Zc2;
            (*q2_ptr) = -q;
        }
    };





    //!
    //! @brief Hydraulic orifice with turbulent flow of Q-Type. Uses TurbulentFlowFunction to calculate the flow.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOptimizedTurbulentOrifice : public ComponentQ
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
            return new HydraulicOptimizedTurbulentOrifice("TurbulentOrifice");
        }

        HydraulicOptimizedTurbulentOrifice(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicOptimizedTurbulentOrifice";
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
            mpP1->writeNode(NodeHydraulic::FLOW, q1);
            mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
            mpP2->writeNode(NodeHydraulic::FLOW, q2);
        }
    };
}

#endif // HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
