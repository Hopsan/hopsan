/*
 *  HydraulicCheckValve.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-07.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

#ifndef HYDRAULICCHECKVALVE_HPP_INCLUDED
#define HYDRAULICCHECKVALVE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"
#include "math.h"
#include "CoreUtilities/TurbulentFlowFunction.h"

class HydraulicCheckValve : public ComponentQ
{
private:
    double mKs;
    TurbulentFlowFunction mQturb;
    enum {P1, P2};

public:
    static Component *Creator()
    {
        std::cout << "running checkvalve creator" << std::endl;
        return new HydraulicCheckValve("DefaultCheckValveName");
    }

    HydraulicCheckValve(const string name,
                                const double ks       = 0.000000025,
                                const double timestep = 0.001)
        : ComponentQ(name, timestep)
    {
        mKs = ks;

        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);

        registerParameter("Ks", "Restrictor Coefficient", "-", mKs);
    }


    void initialize()
    {
        mQturb.setFlowCoefficient(mKs);
    }


    void simulateOneTimestep()
    {
        //Get the nodes
        Node* p1_ptr = mPorts[P1].getNodePtr();
        Node* p2_ptr = mPorts[P2].getNodePtr();

        //Get variable values from nodes
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        //Checkvalve equations

        bool cav;
        double q1, q2;

        if (c1 > c2) { q2 = mQturb.getFlow(c1, c2, Zc1, Zc2); }
        else { q2 = 0.0; }

        q1 = -q2;
        double p1 = c1 + Zc1 * q1;
        double p2 = c2 + Zc2 * q2;

        /* Cavitation */
        cav = false;

        if (p1 < 0.0)
        {
            c1 = 0.0;
            Zc1 = 0.0;
            cav = true;
        }
        if (p2 < 0.0)
        {
            c2 = 0.0;
            Zc2 = 0.0;
            cav = true;
        }
        if (cav)
        {
            if (c1 > c2) { q2 = mQturb.getFlow(c1, c2, Zc1, Zc2); }
            else { q2 = 0.0; }
        }
        q1 = -q2;
        p1 = c1 + Zc1 * q1;
        p2 = c2 + Zc2 * q2;
        if (p1 < 0.0) { p1 = 0.0; }
        if (p2 < 0.0) { p2 = 0.0; }

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, p1);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, p2);
        p2_ptr->setData(NodeHydraulic::MASSFLOW, q2);
    }
};

#endif // HYDRAULICCHECKVALVE_HPP_INCLUDED
