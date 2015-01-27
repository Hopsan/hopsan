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
//! @file   HydraulicFourChamberPiston.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-04-29
//!
//! @brief Contains a Hydraulic 4-Chamber Cylinder of C type
//!
//$Id$

#ifndef HYDRAULICFOURCHAMBERPISTON_HPP_INCLUDED
#define HYDRAULICFOURCHAMBERPISTON_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

class HydraulicFourChamberPiston : public ComponentC
{

    private:
        double CxLim, ZxLim, wfak, alpha;

        double ci1, ci2, ci3, ci4, cl1, cl2, cl3, cl4;  //Members because old value need to be remembered (c1 and c2 are remembered through nodes)
        double mNum[2];
        double mDen[2];

        //Node data pointers
        std::vector<double*> mvpND_p1, mvpND_q1, mvpND_c1, mvpND_Zc1;
        std::vector<double*> mvpND_p2, mvpND_q2, mvpND_c2, mvpND_Zc2;
        std::vector<double*> mvpND_p3, mvpND_q3, mvpND_c3, mvpND_Zc3;
        std::vector<double*> mvpND_p4, mvpND_q4, mvpND_c4, mvpND_Zc4;
        double *mpA1, *mpA2, *mpA3, *mpA4, *mpSl, *mpV01, *mpV02, *mpV03, *mpV04, *mpBp, *mpBetae, *mpCLeak12, *mpCLeak13, *mpCLeak14, *mpCLeak23, *mpCLeak24, *mpCLeak34;

        double *mpf5, *mpx5, *mpv5, *mpc5, *mpZx5, *mpme5;
        size_t mNumPorts1, mNumPorts2, mNumPorts3, mNumPorts4;

        //Ports
        Port *mpP1, *mpP2, *mpP3, *mpP4, *mpP5;

    public:
        static Component *Creator()
        {
            return new HydraulicFourChamberPiston();
        }

        void configure()
        {
            //Set member attributes
            wfak = 0.1;
            alpha = 0.1;

            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");
            mpP2 = addPowerMultiPort("P2", "NodeHydraulic");
            mpP3 = addPowerMultiPort("P3", "NodeHydraulic");
            mpP4 = addPowerMultiPort("P4", "NodeHydraulic");
            mpP5 = addPowerPort("P5", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            addInputVariable("A_1", "Piston Area 1", "m^2", 0.001, &mpA1);
            addInputVariable("A_2", "Piston Area 2", "m^2", 0.001, &mpA2);
            addInputVariable("A_3", "Piston Area 3", "m^2", 0.001, &mpA3);
            addInputVariable("A_4", "Piston Area 4", "m^2", 0.001, &mpA4);
            addInputVariable("s_l", "Stroke", "m", 1.0, &mpSl);
            addInputVariable("V_1", "Dead Volume in Chamber 1", "m^3", 0.0003, &mpV01);
            addInputVariable("V_2", "Dead Volume in Chamber 2", "m^3", 0.0003, &mpV02);
            addInputVariable("V_3", "Dead Volume in Chamber 3", "m^3", 0.0003, &mpV03);
            addInputVariable("V_4", "Dead Volume in Chamber 4", "m^3", 0.0003, &mpV04);
            addInputVariable("B_p", "Viscous Friction", "Ns/m", 1000.0, &mpBp);
            addInputVariable("Beta_e", "Bulk Modulus", "Pa", 1000000000.0, &mpBetae);
            addInputVariable("c_leak12", "Leakage Coefficient Between Chamber 1 and 2", "", 0.00000000001, &mpCLeak12);
            addInputVariable("c_leak13", "Leakage Coefficient Between Chamber 1 and 3", "", 0, &mpCLeak13);
            addInputVariable("c_leak14", "Leakage Coefficient Between Chamber 1 and 4", "", 0.00000000001, &mpCLeak14);
            addInputVariable("c_leak23", "Leakage Coefficient Between Chamber 2 and 3", "", 0, &mpCLeak23);
            addInputVariable("c_leak24", "Leakage Coefficient Between Chamber 2 and 4", "", 0, &mpCLeak24);
            addInputVariable("c_leak34", "Leakage Coefficient Between Chamber 3 and 4", "", 0.00000000001, &mpCLeak34);
        }

        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mNumPorts2 = mpP2->getNumPorts();
            mNumPorts3 = mpP3->getNumPorts();
            mNumPorts4 = mpP4->getNumPorts();

            mvpND_p1.resize(mNumPorts1);
            mvpND_q1.resize(mNumPorts1);
            mvpND_c1.resize(mNumPorts1);
            mvpND_Zc1.resize(mNumPorts1);

            mvpND_p2.resize(mNumPorts2);
            mvpND_q2.resize(mNumPorts2);
            mvpND_c2.resize(mNumPorts2);
            mvpND_Zc2.resize(mNumPorts2);

            mvpND_p3.resize(mNumPorts3);
            mvpND_q3.resize(mNumPorts3);
            mvpND_c3.resize(mNumPorts3);
            mvpND_Zc3.resize(mNumPorts3);
            
            mvpND_p4.resize(mNumPorts4);
            mvpND_q4.resize(mNumPorts4);
            mvpND_c4.resize(mNumPorts4);
            mvpND_Zc4.resize(mNumPorts4);
            
            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double A3 = (*mpA3);
            double A4 = (*mpA4);
            double sl = (*mpSl);
            double V01 = (*mpV01);
            double V02 = (*mpV02);
            double V03 = (*mpV03);
            double V04 = (*mpV04);
            double bp = (*mpBp);
            double betae = (*mpBetae);
            double cLeak12 = (*mpCLeak12);
            double cLeak13 = (*mpCLeak13);
            double cLeak14 = (*mpCLeak14);
            double cLeak23 = (*mpCLeak23);
            double cLeak24 = (*mpCLeak24);
            double cLeak34 = (*mpCLeak34);

            //Assign node data pointers
            for (size_t i=0; i<mNumPorts1; ++i)
            {

                mvpND_p1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Pressure, 0.0);
                mvpND_q1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Flow, 0.0);
                mvpND_c1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::WaveVariable, 0.0);
                mvpND_Zc1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpND_p1[i] = getDefaultStartValue(mpP1, NodeHydraulic::Pressure);
                *mvpND_q1[i] = getDefaultStartValue(mpP1, NodeHydraulic::Flow)/double(mNumPorts1);
                *mvpND_c1[i] = getDefaultStartValue(mpP1, NodeHydraulic::Pressure);
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                mvpND_p2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::Pressure, 0.0);
                mvpND_q2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::Flow, 0.0);
                mvpND_c2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::WaveVariable, 0.0);
                mvpND_Zc2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpND_p2[i] = getDefaultStartValue(mpP2, NodeHydraulic::Pressure);
                *mvpND_q2[i] = getDefaultStartValue(mpP2, NodeHydraulic::Flow)/double(mNumPorts2);
                *mvpND_c2[i] = getDefaultStartValue(mpP2, NodeHydraulic::Pressure);
            }
            for (size_t i=0; i<mNumPorts3; ++i)
            {
                mvpND_p3[i] = getSafeMultiPortNodeDataPtr(mpP3, i, NodeHydraulic::Pressure, 0.0);
                mvpND_q3[i] = getSafeMultiPortNodeDataPtr(mpP3, i, NodeHydraulic::Flow, 0.0);
                mvpND_c3[i] = getSafeMultiPortNodeDataPtr(mpP3, i, NodeHydraulic::WaveVariable, 0.0);
                mvpND_Zc3[i] = getSafeMultiPortNodeDataPtr(mpP3, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpND_p3[i] = getDefaultStartValue(mpP3, NodeHydraulic::Pressure);
                *mvpND_q3[i] = getDefaultStartValue(mpP3, NodeHydraulic::Flow)/double(mNumPorts3);
                *mvpND_c3[i] = getDefaultStartValue(mpP3, NodeHydraulic::Pressure);
            }
            for (size_t i=0; i<mNumPorts4; ++i)
            {
                mvpND_p4[i] = getSafeMultiPortNodeDataPtr(mpP4, i, NodeHydraulic::Pressure, 0.0);
                mvpND_q4[i] = getSafeMultiPortNodeDataPtr(mpP4, i, NodeHydraulic::Flow, 0.0);
                mvpND_c4[i] = getSafeMultiPortNodeDataPtr(mpP4, i, NodeHydraulic::WaveVariable, 0.0);
                mvpND_Zc4[i] = getSafeMultiPortNodeDataPtr(mpP4, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpND_p4[i] = getDefaultStartValue(mpP4, NodeHydraulic::Pressure);
                *mvpND_q4[i] = getDefaultStartValue(mpP4, NodeHydraulic::Flow)/double(mNumPorts4);
                *mvpND_c4[i] = getDefaultStartValue(mpP4, NodeHydraulic::Pressure);
            }
            mpf5 = getSafeNodeDataPtr(mpP5, NodeMechanic::Force);
            mpx5 = getSafeNodeDataPtr(mpP5, NodeMechanic::Position);
            mpv5 = getSafeNodeDataPtr(mpP5, NodeMechanic::Velocity);
            mpc5 = getSafeNodeDataPtr(mpP5, NodeMechanic::WaveVariable);
            mpZx5 = getSafeNodeDataPtr(mpP5, NodeMechanic::CharImpedance);
            mpme5 = getSafeNodeDataPtr(mpP5, NodeMechanic::EquivalentMass);

            //Declare local variables;
            double p1, p2, p3, p4, x5, v5;
            double Zc1, Zc2, Zc3, Zc4, c5, Zx5;
            double qi1, qi2, qi3, qi4, V1, V2, V3, V4, qLeak12, qLeak13, qLeak14, qLeak23, qLeak24, qLeak34, V1min, V2min, V3min, V4min;

            //Read variables from nodes
            p1 = (*mvpND_p1[0]);
            p2 = (*mvpND_p2[0]);
            p3 = (*mvpND_p3[0]);
            p4 = (*mvpND_p4[0]);
            x5 = (*mpx5);
            v5 = (*mpv5);

            //Size of volumes
            V1 = V01+A1*(-x5);
            V2 = V02+A2*(sl+x5);
            V3 = V03+A3*(-x5);
            V4 = V04+A4*(sl+x5);
            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*1.0); //me is not written to node yet.
            V2min = betae*mTimestep*mTimestep*A2*A2/(wfak*1.0);
            V3min = betae*mTimestep*mTimestep*A3*A3/(wfak*1.0);
            V4min = betae*mTimestep*mTimestep*A4*A4/(wfak*1.0);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;
            if(V3<V3min) V3 = V3min;
            if(V4<V4min) V4 = V4min;

            Zc1 = (double(mNumPorts1)+2.0) / 2.0 * betae/V1*mTimestep/(1.0-alpha);    //Number of ports in volume is 2 internal plus the external ones
            Zc2 = (double(mNumPorts2)+2.0) / 2.0 * betae/V2*mTimestep/(1.0-alpha);
            Zc3 = (double(mNumPorts3)+2.0) / 2.0 * betae/V3*mTimestep/(1.0-alpha);
            Zc4 = (double(mNumPorts4)+2.0) / 2.0 * betae/V4*mTimestep/(1.0-alpha);
            Zx5 = A1*A1*Zc1 + A2*A2*Zc2 + A3*A3*Zc3 + A4*A4*Zc4 + bp;

            //Internal flows
            qi1 = v5*A1;
            qi2 = -v5*A2;
            qi3 = v5*A3;
            qi4 = -v5*A4;

            ci1 = p1 + Zc1*qi1;
            ci2 = p2 + Zc2*qi2;
            ci3 = p3 + Zc3*qi3;
            ci4 = p4 + Zc4*qi4;

            //Leakage flow
            qLeak12 = cLeak12*(p1-p2);
            qLeak13 = cLeak13*(p1-p3);
            qLeak14 = cLeak14*(p1-p4);
            qLeak23 = cLeak23*(p2-p3);
            qLeak24 = cLeak24*(p2-p4);
            qLeak34 = cLeak34*(p3-p4);

            cl1 = p1 + Zc1*(-qLeak12) + Zc1*(-qLeak13) + Zc1*(-qLeak14);
            cl2 = p2 + Zc2*qLeak12    + Zc2*(-qLeak23) + Zc2*(-qLeak24);
            cl3 = p3 + Zc3*qLeak13    + Zc3*qLeak23    + Zc3*(-qLeak34);
            cl4 = p4 + Zc3*qLeak14    + Zc4*qLeak24    + Zc3*qLeak34;

            c5 = A1*ci1 - A2*ci2 + A3*ci3 - A4*ci4;

            //Write to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                *(mvpND_c1[i]) = p1 + Zc1*(*mvpND_q1[i]);
                *(mvpND_Zc1[i]) = Zc1;
            }
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                *(mvpND_c2[i]) = p2 + Zc2*(*mvpND_q2[i]);
                *(mvpND_Zc2[i]) = Zc2;
            }
            for(size_t i=0; i<mNumPorts3; ++i)
            {
                *(mvpND_c3[i]) = p3 + Zc3*(*mvpND_q3[i]);
                *(mvpND_Zc3[i]) = Zc3;
            }            
            for(size_t i=0; i<mNumPorts4; ++i)
            {
                *(mvpND_c4[i]) = p4 + Zc4*(*mvpND_q4[i]);
                *(mvpND_Zc4[i]) = Zc4;
            }
            (*mpc5) = c5;
            (*mpZx5) = Zx5;
        }

        void simulateOneTimestep()
        {
            //Declare local variables;
            double V1, V2, V3, V4, qLeak12, qLeak13, qLeak14, qLeak23, qLeak24, qLeak34, qi1, qi2, qi3, qi4, p1mean, p2mean, p3mean, p4mean, V1min, V2min, V3min, V4min;

            //Read variables from nodes
            double Zc1 = (*mvpND_Zc1[0]);          //All Zc should be the same and Q components shall
            double Zc2 = (*mvpND_Zc2[0]);          //never touch them, so let's just use first value
            double Zc3 = (*mvpND_Zc3[0]);
            double Zc4 = (*mvpND_Zc4[0]);
            double x5 = (*mpx5);
            double v5 = (*mpv5);
            double me5 = (*mpme5);

            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double A3 = (*mpA3);
            double A4 = (*mpA4);
            double sl = (*mpSl);
            double V01 = (*mpV01);
            double V02 = (*mpV02);
            double V03 = (*mpV03);
            double V04 = (*mpV04);
            double bp = (*mpBp);
            double betae = (*mpBetae);
            double cLeak12 = (*mpCLeak12);
            double cLeak13 = (*mpCLeak13);
            double cLeak14 = (*mpCLeak14);
            double cLeak23 = (*mpCLeak23);
            double cLeak24 = (*mpCLeak24);
            double cLeak34 = (*mpCLeak34);
            
            //Leakage flow
            qLeak12 = cLeak12*(cl1-cl2)/(1.0+cLeak12*(Zc1+Zc2));
            qLeak13 = cLeak13*(cl1-cl3)/(1.0+cLeak13*(Zc1+Zc3));
            qLeak14 = cLeak14*(cl1-cl4)/(1.0+cLeak14*(Zc1+Zc4));
            qLeak23 = cLeak23*(cl2-cl3)/(1.0+cLeak23*(Zc2+Zc3));
            qLeak24 = cLeak24*(cl2-cl4)/(1.0+cLeak24*(Zc2+Zc4));
            qLeak34 = cLeak34*(cl3-cl4)/(1.0+cLeak34*(Zc3+Zc4));

            //Internal flows
            qi1 = v5*A1;
            qi2 = -v5*A2;
            qi3 = v5*A3;
            qi4 = -v5*A4;
            
            //Size of volumes
            V1 = V01+A1*(-x5);
            V2 = V02+A2*(sl+x5);
            V3 = V03+A3*(-x5);
            V4 = V04+A4*(sl+x5);
            if(me5 <= 0)        //Me must be bigger than zero
            {
                addDebugMessage("Me = "+to_hstring(me5));

                //! @todo what the heck is this all about?
                if(mTime > mTimestep*1.5)
                {
                    addErrorMessage("The equivalent mass 'me' has to be greater than 0.");
                    stopSimulation();
                }
                else        //Don't check first time step, because C is executed before Q and Q may not have written me during initialization
                {
                    addWarningMessage("Equivalent mass 'me' not initialized to a value greater than 0.");
                    me5 = 1;
                }
            }

            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*me5);
            V2min = betae*mTimestep*mTimestep*A2*A2/(wfak*me5);
            V3min = betae*mTimestep*mTimestep*A3*A3/(wfak*me5);
            V4min = betae*mTimestep*mTimestep*A4*A4/(wfak*me5);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;
            if(V3<V3min) V3 = V3min;
            if(V4<V4min) V4 = V4min;

            //Volume 1
            Zc1 = (double(mNumPorts1)+2.0) / 2.0 * betae/V1*mTimestep/(1.0-alpha);    //Number of ports in volume is 2 internal plus the external ones
            p1mean = (ci1 + Zc1*2.0*qi1) + (cl1 + Zc1*2.0*(-qLeak12) + Zc1*2.0*(-qLeak13)+ Zc1*2.0*(-qLeak14));
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                p1mean += (*mvpND_c1[i]) + 2.0*Zc1*(*mvpND_q1[i]);
            }
            p1mean = p1mean/(double(mNumPorts1)+2.0);
            ci1 = std::max(0.0, alpha * ci1 + (1.0 - alpha)*(p1mean*2.0 - ci1 - 2.0*Zc1*qi1));
            cl1 = std::max(0.0, alpha * cl1 + (1.0 - alpha)*(p1mean*2.0 - cl1 - 2.0*Zc1*(-qLeak12) - 2.0*Zc1*(-qLeak13) - 2.0*Zc1*(-qLeak14)));

            //Volume 2
            Zc2 = ((double(mNumPorts2)+2.0) / 2.0) * betae/V2*mTimestep/(1.0-alpha);
            p2mean = (ci2 + Zc2*2.0*qi2) + (cl2 + Zc2*2.0*qLeak12 + Zc2*2.0*(-qLeak23) + Zc2*2.0*(-qLeak24));
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                p2mean += (*mvpND_c2[i]) + 2.0*Zc2*(*mvpND_q2[i]);
            }
            p2mean = p2mean/(double(mNumPorts2)+2.0);
            ci2 = std::max(0.0, alpha * ci2 + (1.0 - alpha)*(p2mean*2.0 - ci2 - 2.0*Zc2*qi2));
            cl2 = std::max(0.0, alpha * cl2 + (1.0 - alpha)*(p2mean*2.0 - cl2 - 2.0*Zc2*qLeak12 - 2.0*Zc2*(-qLeak23) - 2.0*Zc2*(-qLeak24)));

            //Volume 3
            Zc3 = ((double(mNumPorts3)+2.0) / 2.0) * betae/V3*mTimestep/(1.0-alpha);
            p3mean = (ci3 + Zc3*2.0*qi3) + (cl3 + Zc3*2.0*qLeak13 + Zc3*2.0*qLeak23 + Zc3*2.0*(-qLeak34));
            for(size_t i=0; i<mNumPorts3; ++i)
            {
                p3mean += (*mvpND_c3[i]) + 2.0*Zc3*(*mvpND_q3[i]);
            }
            p3mean = p3mean/(double(mNumPorts3)+2.0);
            ci3 = std::max(0.0, alpha * ci3 + (1.0 - alpha)*(p3mean*2.0 - ci3 - 2.0*Zc3*qi3));
            cl3 = std::max(0.0, alpha * cl3 + (1.0 - alpha)*(p3mean*2.0 - cl3 - 2.0*Zc3*qLeak13 - 2.0*Zc3*qLeak23 - 2.0*Zc3*(-qLeak34)));
            
            //Volume 2
            Zc4 = ((double(mNumPorts4)+2.0) / 2.0) * betae/V4*mTimestep/(1.0-alpha);
            p4mean = (ci4 + Zc4*2.0*qi4) + (cl4 + Zc4*2.0*qLeak14 + Zc4*2.0*qLeak24 + Zc4*2.0*qLeak34);
            for(size_t i=0; i<mNumPorts4; ++i)
            {
                p4mean += (*mvpND_c4[i]) + 2.0*Zc4*(*mvpND_q4[i]);
            }
            p4mean = p4mean/(double(mNumPorts4)+2.0);
            ci4 = std::max(0.0, alpha * ci4 + (1.0 - alpha)*(p4mean*2.0 - ci4 - 2.0*Zc4*qi4));
            cl4 = std::max(0.0, alpha * cl4 + (1.0 - alpha)*(p4mean*2.0 - cl4 - 2.0*Zc4*qLeak14 - 2.0*Zc4*qLeak24 - 2.0*Zc4*qLeak34));

            //limitStroke(CxLim, ZxLim, x3, v3, me, sl);

            //Internal mechanical port
            double c5 = A1*ci1 - A2*ci2 + A3*ci3 - A4*ci4;// + CxLim;
            double Zx5 = A1*A1*Zc1 + A2*A2*Zc2 + A3*A3*Zc3 + A4*A4*Zc3 + bp;// + ZxLim;
            //! @note End of stroke limitation currently turned off, because the piston gets stuck in the end position.
            //! @todo Either implement a working limitation, or remove it completely. It works just as well to have it in the mass component.

            //Write to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                *(mvpND_c1[i]) = std::max(0.0, alpha * (*mvpND_c1[i]) + (1.0 - alpha)*(p1mean*2 - (*mvpND_c1[i]) - 2*Zc1*(*mvpND_q1[i])));
                *(mvpND_Zc1[i]) = Zc1;
            }
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                *(mvpND_c2[i]) = std::max(0.0, alpha * (*mvpND_c2[i]) + (1.0 - alpha)*(p2mean*2 - (*mvpND_c2[i]) - 2*Zc2*(*mvpND_q2[i])));
                *(mvpND_Zc2[i]) = Zc2;
            }
            for(size_t i=0; i<mNumPorts3; ++i)
            {
                *(mvpND_c3[i]) = std::max(0.0, alpha * (*mvpND_c3[i]) + (1.0 - alpha)*(p3mean*2 - (*mvpND_c3[i]) - 2*Zc3*(*mvpND_q3[i])));
                *(mvpND_Zc3[i]) = Zc3;
            }
            for(size_t i=0; i<mNumPorts4; ++i)
            {
                *(mvpND_c4[i]) = std::max(0.0, alpha * (*mvpND_c4[i]) + (1.0 - alpha)*(p4mean*2 - (*mvpND_c4[i]) - 2*Zc4*(*mvpND_q4[i])));
                *(mvpND_Zc4[i]) = Zc4;
            }
            (*mpc5) = c5;
            (*mpZx5) = Zx5;
        }
    };
}

#endif //HYDRAULICFOURCHAMBERPISTON_HPP_INCLUDED


