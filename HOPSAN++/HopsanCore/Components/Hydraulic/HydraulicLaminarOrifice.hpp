#ifndef HYDRAULICLAMINARORIFICE_HPP_INCLUDED
#define HYDRAULICLAMINARORIFICE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicLaminarOrifice : public ComponentQ
    {
    private:
        double Kc;

        double *p1_ptr, *q1_ptr, *c1_ptr, *Zc1_ptr, *p2_ptr, *q2_ptr, *c2_ptr, *Zc2_ptr;
        double q, c1, Zc1, c2, Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicLaminarOrifice("LaminarOrifice");
        }

        HydraulicLaminarOrifice(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicLaminarOrifice";
            Kc = 1.0e-11;

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            registerParameter("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", Kc);
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
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*c1_ptr);
            Zc1 = (*Zc1_ptr);
            c2 = (*c2_ptr);
            Zc2 = (*Zc2_ptr);

            //Orifice equations
            q = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));

            //Write new variables to nodes
            (*p1_ptr) = c1 + q*Zc1;
            (*q1_ptr) = q;
            (*p2_ptr) = c2 - q*Zc2;
            (*q2_ptr) = -q;
        }
    };
}

#endif // HYDRAULICLAMINARORIFICE_HPP_INCLUDED
