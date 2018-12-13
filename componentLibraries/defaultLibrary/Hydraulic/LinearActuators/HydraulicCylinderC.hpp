/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HydraulicCylinderC.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-20
//!
//! @brief Contains a Hydraulic Cylinder of C type
//! Written by Petter Krus 9005617
//! Revised by ??? 910410
//! Translated to C++ by Robert Braun 100120
//! Revised by Robert Braun 110429
//!
//$Id$

#ifndef HYDRAULICCYLINDERC_HPP_INCLUDED
#define HYDRAULICCYLINDERC_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicCylinderC : public ComponentC
{

    private:
        double mWfak, mAlpha;
        bool mUseEndStops;

        double ci1, cl1, ci2, cl2;  //Members because old value need to be remembered (c1 and c2 are remembered through nodes)
        double mNum[2];
        double mDen[2];

        //Node data pointers
        std::vector<double*> mvpP1_p, mvpP1_q, mvpP1_c, mvpP1_Zc;
        std::vector<double*> mvpP2_p, mvpP2_q, mvpP2_c, mvpP2_Zc;
        double *mpSl, *mpV01, *mpV02, *mpBp, *mpBetae, *mpCLeak;

        double *mpP3_f, *mpP3_x, *mpP3_v, *mpP3_c, *mpP3_Zx, *mpP3_me;
        size_t mNumPorts1, mNumPorts2;

        double *mpQLeak;

        //Ports
        Port *mpP1, *mpP2, *mpP3;

    protected:
        double *mpA1, *mpA2;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderC();
        }

        void configure()
        {
            // Set member variables
            mWfak = 0.1;
            mAlpha = 0.1;

            // Add constant parameters
            addConstant("use_sl", "Use end stops (stroke limitation)", "", true, mUseEndStops);

            // Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");
            mpP2 = addPowerMultiPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            // Add input variables
            addInputVariable("A_1", "Piston Area 1", "m^2", 0.001, &mpA1);
            addInputVariable("A_2", "Piston Area 2", "m^2", 0.001, &mpA2);
            addInputVariable("s_l", "Stroke", "m", 1.0, &mpSl);
            addInputVariable("V_1", "Dead Volume in Chamber 1", "m^3", 0.0003, &mpV01);
            addInputVariable("V_2", "Dead Volume in Chamber 2", "m^3", 0.0003, &mpV02);
            addInputVariable("B_p", "Viscous Friction", "Ns/m", 1000.0, &mpBp);
            addInputVariable("Beta_e", "Bulk Modulus", "Pa", 1000000000.0, &mpBetae);
            addInputVariable("c_leak", "Leakage Coefficient", "", 0.00000000001, &mpCLeak);

            addOutputVariable("q_leak", "Internal Leakage Flow", "Flow", 0, &mpQLeak);
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mNumPorts2 = mpP2->getNumPorts();

            mvpP1_p.resize(mNumPorts1);
            mvpP1_q.resize(mNumPorts1);
            mvpP1_c.resize(mNumPorts1);
            mvpP1_Zc.resize(mNumPorts1);

            mvpP2_p.resize(mNumPorts2);
            mvpP2_q.resize(mNumPorts2);
            mvpP2_c.resize(mNumPorts2);
            mvpP2_Zc.resize(mNumPorts2);

            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double sl = (*mpSl);
            double V01 = (*mpV01);
            double V02 = (*mpV02);
            double bp = (*mpBp);
            double betae = (*mpBetae);
            double cLeak = (*mpCLeak);

            //Assign node data pointers
            for (size_t i=0; i<mNumPorts1; ++i)
            {

                mvpP1_p[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Pressure, 0.0);
                mvpP1_q[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Flow, 0.0);
                mvpP1_c[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::WaveVariable, 0.0);
                mvpP1_Zc[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpP1_p[i] = getDefaultStartValue(mpP1, NodeHydraulic::Pressure);
                *mvpP1_q[i] = getDefaultStartValue(mpP1, NodeHydraulic::Flow)/double(mNumPorts1);
                *mvpP1_c[i] = getDefaultStartValue(mpP1, NodeHydraulic::Pressure);
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                mvpP2_p[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::Pressure, 0.0);
                mvpP2_q[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::Flow, 0.0);
                mvpP2_c[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::WaveVariable, 0.0);
                mvpP2_Zc[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpP2_p[i] = getDefaultStartValue(mpP2, NodeHydraulic::Pressure);
                *mvpP2_q[i] = getDefaultStartValue(mpP2, NodeHydraulic::Flow)/double(mNumPorts2);
                *mvpP2_c[i] = getDefaultStartValue(mpP2, NodeHydraulic::Pressure);
            }
            mpP3_f = getSafeNodeDataPtr(mpP3, NodeMechanic::Force);
            mpP3_x = getSafeNodeDataPtr(mpP3, NodeMechanic::Position);
            mpP3_v = getSafeNodeDataPtr(mpP3, NodeMechanic::Velocity);
            mpP3_c = getSafeNodeDataPtr(mpP3, NodeMechanic::WaveVariable);
            mpP3_Zx = getSafeNodeDataPtr(mpP3, NodeMechanic::CharImpedance);
            mpP3_me = getSafeNodeDataPtr(mpP3, NodeMechanic::EquivalentMass);

            //Declare local variables;
            double p1, p2, x3, v3;
            double Zc1, Zc2, c3, Zx3;
            double qi1, qi2, V1, V2, qLeak, V1min, V2min;

            //Read variables from nodes
            p1 = (*mvpP1_p[0]);
            p2 = (*mvpP2_p[0]);
            x3 = (*mpP3_x);
            v3 = (*mpP3_v);

            //Size of volumes
            V1 = V01+A1*(-x3);
            V2 = V02+A2*(sl+x3);
            V1min = betae*mTimestep*mTimestep*A1*A1/(mWfak*1.0); //me is not written to node yet.
            V2min = betae*mTimestep*mTimestep*A2*A2/(mWfak*1.0);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;

            Zc1 = (double(mNumPorts1)+2.0) / 2.0 * betae/V1*mTimestep/(1.0-mAlpha);    //Number of ports in volume is 2 internal plus the external ones
            Zc2 = (double(mNumPorts2)+2.0) / 2.0 * betae/V2*mTimestep/(1.0-mAlpha);
            Zx3 = A1*A1*Zc1 +A2*A2*Zc2 + bp;

            //Internal flows
            qi1 = v3*A1;
            qi2 = -v3*A2;

            ci1 = p1 + Zc1*qi1;
            ci2 = p2 + Zc2*qi2;

            //Leakage flow
            qLeak = cLeak*(p1-p2);

            cl1 = p1 + Zc1*(-qLeak);
            cl2 = p2 + Zc2*qLeak;

            c3 = A1*ci1 - A2*ci2;

            //Write to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                *(mvpP1_c[i]) = p1 + Zc1*(*mvpP1_q[i]);
                *(mvpP1_Zc[i]) = Zc1;
            }
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                *(mvpP2_c[i]) = p2 + Zc2*(*mvpP2_q[i]);
                *(mvpP2_Zc[i]) = Zc2;
            }
            (*mpP3_c) = c3;
            (*mpP3_Zx) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare local variables;
            double V1, V2, qLeak, qi1, qi2, c1mean, c2mean, V1min, V2min, CxLim, ZxLim;
            bool p1cav=false, p2cav=false;

            //Read variables from nodes
            double Zc1 = (*mvpP1_Zc[0]);          //All Zc should be the same and Q components shall
            double Zc2 = (*mvpP2_Zc[0]);          //never touch them, so let's just use first value
            double x3 = (*mpP3_x);
            double v3 = (*mpP3_v);
            double me = (*mpP3_me);

            double A1 = (*mpA1);
            double A2 = (*mpA2);
            double sl = (*mpSl);
            double V01 = (*mpV01);
            double V02 = (*mpV02);
            double bp = (*mpBp);
            double betae = (*mpBetae);
            double cLeak = (*mpCLeak);

            //Leakage flow
            qLeak = cLeak*(cl1-cl2)/(1.0+cLeak*(Zc1+Zc2));

            //Internal flows
            qi1 = v3*A1;
            qi2 = -v3*A2;

            //Size of volumes
            V1 = V01+A1*(-x3);
            V2 = V02+A2*(sl+x3);
            if(me <= 0)        //Me must be bigger than zero
            {
                addDebugMessage("Me = "+to_hstring(me));

                //! @todo what the heck is this all about?
                if(mTime > mTimestep*1.5)
                {
                    addErrorMessage("The equivalent mass 'me' has to be greater than 0.");
                    stopSimulation();
                }
                else        //Don't check first time step, because C is executed before Q and Q may not have written me during initialization
                {
                    addWarningMessage("Equivalent mass 'me' not initialized to a value greater than 0.");
                    me = 1;
                }
            }

            V1min = betae*mTimestep*mTimestep*A1*A1/(mWfak*me);
            V2min = betae*mTimestep*mTimestep*A2*A2/(mWfak*me);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;

            // Cylinder volumes are modelled the same way as the multiport volume:
            //   c1 = Wave variable for external port
            //   ci1 = Wave variable for internal (mechanical) port
            //   cl1 = Wave variable for leakage port

            //Volume 1
            Zc1 = double(mNumPorts1+2) / 2.0 * betae/V1*mTimestep/(1.0-mAlpha);    //Number of ports in volume is 2 internal plus the external ones
            c1mean = (ci1 + Zc1*2.0*qi1) + (cl1 + Zc1*2.0*(-qLeak));
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                c1mean += (*mvpP1_c[i]) + 2.0*Zc1*(*mvpP1_q[i]);
                p1cav = p1cav || ((*mvpP1_p[i] == 0) );
            }
            c1mean = c1mean/double(mNumPorts1+2);
            ci1 = mAlpha * ci1 + (1.0 - mAlpha)*(c1mean*2.0 - ci1 - 2.0*Zc1*qi1);
            cl1 = mAlpha * cl1 + (1.0 - mAlpha)*(c1mean*2.0 - cl1 - 2.0*Zc1*(-qLeak));


            //Volume 2
            Zc2 = double(mNumPorts2+2) / 2.0 * betae/V2*mTimestep/(1.0-mAlpha);
            c2mean = (ci2 + Zc2*2.0*qi2) + (cl2 + Zc2*2.0*qLeak);
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                c2mean += (*mvpP2_c[i]) + 2.0*Zc2*(*mvpP2_q[i]);
                p2cav = p2cav || ((*mvpP2_p[i] == 0) );
            }
            c2mean = c2mean/double(mNumPorts2+2);
            ci2 = mAlpha * ci2 + (1.0 - mAlpha)*(c2mean*2.0 - ci2 - 2.0*Zc2*qi2);
            cl2 = mAlpha * cl2 + (1.0 - mAlpha)*(c2mean*2.0 - cl2 - 2.0*Zc2*qLeak);


            // Add extra force and Zc in end positions to simulate stop.
            // Stops could also be implemented in the mass component (connected Q-component)
            if (mUseEndStops)
            {
                limitStroke(CxLim, ZxLim, x3, v3, me, sl);
            }
            else
            {
                CxLim = 0;
                ZxLim = 0;
            }

            // If there is cavitation in the hydraulic nodes, then the hydraulic wave and impedance acting on the mechanics should be set to c>=0 and zc=0
            // The pressure is one time-step old, this introduces a small modelling error, but since cavitation is not modelled accurately anyway,
            // this does not matter
            double ci1m=ci1, ci2m=ci2, Zc1m=Zc1, Zc2m=Zc2;
            if (p1cav)
            {
                ci1m = std::max(ci1, 0.0);
                Zc1m = 0;
            }
            if (p2cav)
            {
                ci2m = std::max(ci2, 0.0);
                Zc2m = 0;
            }

            // Internal mechanical port
            double c3 = A1*ci1m - A2*ci2m + CxLim;
            double Zx3 = A1*A1*Zc1m + A2*A2*Zc2m + bp + ZxLim;


            //Write to nodes
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                *(mvpP1_c[i]) = mAlpha * (*mvpP1_c[i]) + (1.0 - mAlpha)*(c1mean*2 - (*mvpP1_c[i]) - 2*Zc1*(*mvpP1_q[i]));
                *(mvpP1_Zc[i]) = Zc1;
            }
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                *(mvpP2_c[i]) = mAlpha * (*mvpP2_c[i]) + (1.0 - mAlpha)*(c2mean*2 - (*mvpP2_c[i]) - 2*Zc2*(*mvpP2_q[i]));
                *(mvpP2_Zc[i]) = Zc2;
            }
            (*mpP3_c) = c3;
            (*mpP3_Zx) = Zx3;
            (*mpQLeak) = qLeak;
        }


        //This function was translated from old HOPSAN using F2C. A few manual adjustments were necessary.

        /* ---------------------------------------------------------------- */
        /*     Function that simulate the end of the stroke. If X is        */
        /*     smaller than 0 or greater than SL a large spring force will  */
        /*     act to force X into the interval again. The spring constant  */
        /*     is as high possible without numerical instability.           */
        /* ---------------------------------------------------------------- */

        void limitStroke(double &CxLim, double &ZxLim, double x3, double v3, double me, double sl)
        {
            double FxLim=0.0, ZxLim0, NewCxLim, alfa;

            alfa = 0.5;

            //Equations
            if (-x3 > sl)
            {
                ZxLim0 = mWfak*me/mTimestep;
                ZxLim = ZxLim0/(1.0 - alfa);
                FxLim = ZxLim0 * (x3 + sl) / mTimestep;
                NewCxLim = FxLim + ZxLim*v3;
            }
            else if (-x3 < 0.0)
            {
                ZxLim0 = mWfak*me/mTimestep;
                ZxLim = ZxLim0/(1.0 - alfa);
                FxLim = ZxLim0*x3/mTimestep;
                NewCxLim = FxLim + ZxLim*v3;
            }
            else
            {
                ZxLim = 0.0;
                CxLim = 0.0;
                NewCxLim = FxLim + ZxLim*v3;
            }

            // Filtering of the characteristics
            CxLim = alfa * CxLim + (1.0 - alfa) * NewCxLim;
        }
    };
}

#endif


















////!
////! @file   HydraulicCylinderC.hpp
////! @author Robert Braun <robert.braun@liu.se>
////! @date   2010-01-20
////!
////! @brief Contains a Hydraulic Cylinder of C type
////! Written by Petter Krus 9005617
////! Revised by ??? 910410
////! Translated to HOPSAN NG by Robert Braun 100120
////!
////$Id$

//#ifndef HYDRAULICCYLINDERC_HPP_INCLUDED
//#define HYDRAULICCYLINDERC_HPP_INCLUDED

//#include <sstream>

//#include "ComponentEssentials.h"
//#include "ComponentUtilities.h"

//namespace hopsan {

////!
////! @brief
////! @ingroup HydraulicComponents
////!
//class HydraulicCylinderC : public ComponentC
//{

//    private:
//        double A1,A2,SL,CIP,BP,ME,BETAE,V01,V02;

//        double V1MIN,V2MIN,ZLIM0;
//        double XP,SXP;
//        double CP1,CP2,CP1D,CP2D;
//        double CP1E,CP2E,CP1ED,CP2ED;

//        double V1,V2,ALFZ1,ALFZ2;
//        double QP1,QP2,QP1E,QP2E;
//        double PP1,PP2,PP1E,PP2E,PM1,PM2,PM1E,PM2E;
//        double C1E,C2E,CP10,CP20,CT1,CT2,CT1E,CT2E;
//        double P1E,P2E;
//        //double QD1, QD2;
//        double CP10E,CP20E;
//        double CLP;
//        double ZLIM,ZC01,ZC02,ZCD1,ZCD2;

//        double CD1, CD2;

//        double PI, BK, ALFA, WFAK;

//        //Node data pointers
//        double *mpp1, *mpq1, *mpc1, *mpZc1, *mpp2, *mpq2, *mpc2, *mpZc2, *mpf3, *mpx3, *mpv3, *mpc3, *mpZx3;

//        //Ports
//        Port *mpP1, *mpP2, *mpP3;

//    public:
//        static Component *Creator()
//        {
//            return new HydraulicCylinderC("CylinderC");
//        }

//        HydraulicCylinderC(name) : ComponentC(name)
//        {
//            //Set member attributes
//            BK = 0.2;
//            ALFA = 0.01;
//            WFAK = 0.1;
//            BETAE = 1000000000.0;
//            ME = 1000.0;
//            V01 = 0.0003;
//            V02 = 0.0003;
//            A1 = 0.001;
//            A2 = 0.001;
//            SL = 1.0;
//            CIP = 0.0;
//            BP = 0.0;

//            //Add ports to the component
//            mpP1 = addPowerPort("P1", "NodeHydraulic");
//            mpP2 = addPowerPort("P2", "NodeHydraulic");
//            mpP3 = addPowerPort("P3", "NodeMechanic");

//            //Register changeable parameters to the HOPSAN++ core
//            registerParameter("A_1", "Piston Area 1", "m^2", A1);
//            registerParameter("A_2", "Piston Area 2", "m^2", A2);
//            registerParameter("s_l", "Stroke", "m", SL);
//            registerParameter("m_e", "Equivalent Load Mass", "kg", ME);
//            registerParameter("V_1", "Dead Volume in Chamber 1", "m^3", V01);
//            registerParameter("V_2", "Dead Volume in Chamber 2", "m^3", V02);
//            registerParameter("B_p", "Viscous Friction", "Ns/m", BP);
//            registerParameter("Beta_e", "Bulk Modulus", "Pa", BETAE);
//            registerParameter("c_leak", "Leakage Coefficient", "", CIP);

//            setStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
//            setStartValue(mpP2, NodeHydraulic::Pressure, 1.0e5);
//        }


//        void initialize()
//        {
//            //Assign node data pointers
//            mpp1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpq1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpZc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
//            mpp2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpq2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpZc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
//            mpf3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
//            mpx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
//            mpv3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
//            mpc3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
//            mpZx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

//            //Read variables from nodes
//            double P1 = (*mpp1);
//            double Q1 = (*mpq1);
//            double P2 = (*mpp2);
//            double Q2 = (*mpq2);
//            double F1 = (*mpf3);
//            double X1 = (*mpx3);
//            double SX1 = (*mpv3);

//            double C1, C2, CX1, ZX1;

//            ZLIM0 = WFAK*ME/mTimestep;
//            V1MIN = BETAE*mTimestep*mTimestep*A1*A1/(WFAK*ME);
//            V2MIN = BETAE*mTimestep*mTimestep*A2*A2/(WFAK*ME);
//            XP  =  -X1;
//            SXP = -SX1;
//            V1 = V01 + A1*XP;
//            V2 = V02 + A2*(SL-XP);
//            if(V1 < V1MIN) V1 = V1MIN;
//            if(V2 < V2MIN) V2 = V2MIN;

//            ZC01 = BETAE*mTimestep/V1;
//            ZC02 = BETAE*mTimestep/V2;
//            ZCD1 = ZC01;
//            ZCD2 = ZC02;

//            C1 = P1 - ZC01*Q1;
//            CD1 = C1;

//            C2 = P2 - ZC02*Q2;
//            CD2 = C2;

//            QP1 = -A1*SXP;
//            QP2 =  A2*SXP;
//            CP1 = P1 - ZC01*(QP1 - CIP*(P1-P2));
//            CP2 = P2 - ZC02*(QP2 - CIP*(P2-P1));

//            CP1D = CP1;
//            CP2D = CP2;
//            CP1ED = CP1;
//            CP2ED = CP2;

//            CX1 = F1;
//            ZX1 = A1*A1*ZC01 +A2*A2*ZC02 + BP;

//             //Write to nodes
//            (*mpc1) = C1;
//            (*mpZc1) = ZC01;
//            (*mpc2) = C2;
//            (*mpZc2) = ZC02;
//            (*mpc3) = CX1;
//            (*mpZx3) = ZX1;
//        }

//        void simulateOneTimestep()
//        {
//            //Declare local variables;
//            double P1, Q1, P2, Q2, C1, C2, X1, SX1, CX1, ZX1;

//            //Read variables from nodes
//            P1 = (*mpp1);
//            Q1 = (*mpq1);
//            P2 = (*mpp2);
//            Q2 = (*mpq2);
//            X1 = (*mpx3);
//            SX1 = (*mpv3);

//            XP  =  -X1;
//            SXP = -SX1;

//       // Volumetric impedances
//            V1 = V01 + A1*XP;
//            V2 = V02 + A2*(SL-XP);
//            if(V1 < V1MIN) V1 = V1MIN;
//            if(V2 < V2MIN) V2 = V2MIN;

//            ZC01 = BETAE*mTimestep/V1;
//            ZC02 = BETAE*mTimestep/V2;
//            ALFZ1 = ZC01/ZCD1;
//            ALFZ2 = ZC02/ZCD2;

//            //! DEBUG
//            if(mTime<0.1)
//            {
//                std::stringstream ss;
//                ss << "V1 = " << V1 << ", V2 = " << V2 << ", ALFZ1 = " << ALFZ1 << ", ALFZ2 = " << ALFZ2;
//                addDebugMessage(ss.str());
//            }
//            //! END DEBUG

//            QP1 = -A1*SXP;
//            QP2 =  A2*SXP;

//            PP1 = (CP1 + QP1*ZC01 + CIP*(CP2*ZC01 + CP1*ZC02 + QP1*ZC01*ZC02 + QP2*ZC01*ZC02 ))/(1 + CIP*(ZC01 + ZC02));
//            PP2 = (CP2 + QP2*ZC02 + CIP*(CP2*ZC01 + CP1*ZC02 + QP1*ZC01*ZC02 + QP2*ZC01*ZC02 ))/(1 + CIP*(ZC01 + ZC02));

//            PP1E = PP1;
//            PP2E = PP2;
//            if(PP1E < 0) PP1E = 0.0;
//            if(PP2E < 0) PP2E = 0.0;

//            QP1E = QP1 - CIP*(PP1E-PP2E);
//            QP2E = QP2 - CIP*(PP2E-PP1E);

//            //! DEBUG
//            if(mTime<0.1)
//            {
//                std::stringstream ss;
//                ss << "QP1 = " << QP1 << ", PP1 = " << PP1 << ", PP1E = " << PP1E << ", QP1E = " << QP1E;
//                addDebugMessage(ss.str());
//            }
//            //! END DEBUG

//       //  Characteristics

//            CT1 = C1 + 2*ZC01*Q1;
//            CT1 = CT1 + PP1 + ZC01*QP1E;
//            PM1 = CT1/2;

//            CT2 = C2 + 2*ZC02*Q2;
//            CT2 = CT2 + PP2 + ZC02*QP2E;
//            PM2 = CT2/2;

//            CP10 = PM1*(1+ALFZ1) - ALFZ1*PP1 -ALFZ1*ZC01*QP1E;
//            CP1  = (1-ALFA)*CP10 + ALFA*(CP1D +(ZCD1-ZC01)*QP1E) ;
//            CP1D = CP1;

//            //! DEBUG
//            if(mTime<0.1)
//            {
//                std::stringstream ss;
//                ss << "CT1 = " << CT1 << ", CP10 = " << CP10 << ", CP1 = " << CP1;
//                addDebugMessage(ss.str());
//            }
//            //! END DEBUG

//            C1E = PM1*(1+ALFZ1) - ALFZ1*C1 - 2*ALFZ1*ZCD1*Q1;
//            C1 = (1-ALFA)*C1E + ALFA*(CD1+(ZCD1-ZC01)*Q1);
//            CD1 = C1;
//            //QD1 = Q1;

//            //! DEBUG
//            if(mTime<0.1)
//            {
//                std::stringstream ss;
//                ss << "C1E = " << C1E << ", C1 = " << C1 << ", ZC01 = " << ZC01 << "\n";
//                addDebugMessage(ss.str());
//            }
//            //! END DEBUG

//            CP20 = PM2*(1+ALFZ2) - ALFZ2*PP2 -ALFZ2*ZC02*QP2E;
//            CP2  = (1-ALFA)*CP20 + ALFA*(CP2D +(ZCD2-ZC02)*QP2E) ;
//            CP2D = CP2;

//            C2E = PM2*(1+ALFZ2) - ALFZ2*C2 - 2*ALFZ2*ZCD2*Q2;
//            C2 = (1-ALFA)*C2E + ALFA*(CD2+(ZCD2-ZC02)*Q2);
//            CD2 = C2;
//            //QD2 = Q2;

//       //  Effective characteristics

//            P1E = P1;
//            if(P1E < 0.0) P1E = 0.0;


//            CT1E = 0.0;
//            CT1E = CT1E + P1E + ZC01*Q1;
//            CT1E = CT1E + PP1E + ZC01*QP1E;
//            PM1E = CT1E/2;

//            P2E = P2;
//            if(P2E < 0.0) P2E = 0.0;

//            CT2E = 0.0;
//            CT2E = CT2E + P2E + ZC02*Q2;

//            CT2E = CT2E + PP2E + ZC02*QP2E;
//            PM2E = CT2E/2;

//       //  Effective characteristics at the piston taking account for cavitation

//            CP10E = PM1E*(1+ALFZ1) - ALFZ1*PP1E -ALFZ1*ZC01*QP1E;
//            CP20E = PM2E*(1+ALFZ2) - ALFZ2*PP2E -ALFZ2*ZC02*QP2E;

//            CP1E  = (1-ALFA)*CP10E + ALFA*(CP1ED +(ZCD1-ZC01)*QP1E);
//            CP2E  = (1-ALFA)*CP20E + ALFA*(CP2ED +(ZCD2-ZC02)*QP2E);

//            CP1ED = CP1E;
//            CP2ED = CP2E;

//            ZCD1 = ZC01;
//            ZCD2 = ZC02;

//       //  Force characteristics
//            limitStroke(&CLP,&ZLIM,&XP,&SXP,&ZLIM0,&SL,mTime, mTimestep);

//            CX1 = CP1E*A1 - CP2E*A2;// + CLP;
//            ZX1 = A1*A1*ZC01 +A2*A2*ZC02 + BP;// + ZLIM;

//            //Write to nodes
//            (*mpc1) = C1;
//            (*mpZc1) = ZC01;
//            (*mpc2) = C2;
//            (*mpZc2) = ZC02;
//            (*mpc3) = CX1;
//            (*mpZx3) = ZX1;
//        }


//        //This function was translated from old HOPSAN using F2C. A few manual adjustments were necessary.

//        /* ---------------------------------------------------------------- */
//        /*     Function that simulate the end of the stroke. If X is        */
//        /*     smaller than 0 or greater than SL a large spring force will  */
//        /*     act to force X into the interval again. The spring constant  */
//        /*     is as high possible without numerical instability.           */
//        /* ---------------------------------------------------------------- */

//        void limitStroke(double *clp, double *zlim, double *xp, double *sxp, double *zlim0, double *sl, double time, double timestep)
//        {
//            static double alfa = .5f;

//            /* Local variables */
//            static double climp, flimp, climp0, ka, cp, kz;

//            //  Initialization
//            if (time == 0) {
//                kz = *zlim0 / timestep;
//                if (*xp > *sl) { flimp = -kz * (*xp - *sl); }
//                else if (*xp < 0.0) { flimp = -kz * *xp; }
//                else { flimp = 0.0; }
//                cp = 0.0;
//            }

//            //Equations
//            ka = 1 / (1 - alfa);
//            kz = *zlim0 / timestep;
//            if (*xp > *sl)
//            {
//                *zlim = ka * *zlim0;
//                flimp = -kz * (*xp - *sl);
//                climp0 = flimp - *zlim * *sxp;
//            }
//            else if (*xp < 0.0)
//            {
//                *zlim = ka * *zlim0;
//                flimp = -kz * *xp;
//                climp0 = flimp - *zlim * *sxp;
//            }
//            else
//            {
//                *zlim = 0.0;
//                climp0 = 0.0;
//            }

//            // Filtering of the characteristics
//            climp = alfa * cp + (1 - alfa) * climp0;
//            cp = climp;
//            *clp = climp;
//        }
//    };
//}

//#endif














//Old attempt

////!
////! @file   HydraulicCylinderC.hpp
////! @author Robert Braun <robert.braun@liu.se>
////! @date   2010-01-20
////!
////! @brief Contains a Hydraulic Cylinder of C type
////! Written by Petter Krus 9005617
////! Revised by ??? 910410
////! Translated to HOPSAN NG by Robert Braun 100120
////!
////$Id$

//#ifndef HYDRAULICCYLINDERC_HPP_INCLUDED
//#define HYDRAULICCYLINDERC_HPP_INCLUDED

//#include <sstream>

//#include "ComponentEssentials.h"
//#include "ComponentUtilities.h"

//namespace hopsan {

////!
////! @brief
////! @ingroup HydraulicComponents
////!
//class HydraulicCylinderC : public ComponentC
//{

//    private:
//        //Local Constants
//        double alfa, wfak;

//        // "Global" variables
//        double zlim0, V1min, V2min, ci1, ci2;

//        //Parameters
//        double betae, me, V01, V02, A1, A2, sl, cLeak, bp;

//        //Node data pointers
//        double *mpp1, *mpq1, *mpc1, *mpZc1, *mpp2, *mpq2, *mpc2, *mpZc2, *mpf3, *mpx3, *mpv3, *mpc3, *mpZx3;

//        //Ports
//        Port *mpP1, *mpP2, *mpP3;

//    public:
//        static Component *Creator()
//        {
//            return new HydraulicCylinderC("CylinderC");
//        }

//        HydraulicCylinderC(name) : ComponentC(name)
//        {
//            //Set member attributes
//            alfa = .01;
//            wfak = .1;
//            betae = 1000000000.0;
//            me = 1000.0;
//            V01 = 0.0003;
//            V02 = 0.0003;
//            A1 = 0.001;
//            A2 = 0.001;
//            sl = 1.0;
//            cLeak = 0.0;
//            bp = 0.0;

//            //Add ports to the component
//            mpP1 = addPowerPort("P1", "NodeHydraulic");
//            mpP2 = addPowerPort("P2", "NodeHydraulic");
//            mpP3 = addPowerPort("P3", "NodeMechanic");

//            //Register changeable parameters to the HOPSAN++ core
//            registerParameter("A_1", "Piston Area 1", "m^2", A1);
//            registerParameter("A_2", "Piston Area 2", "m^2", A2);
//            registerParameter("s_l", "Stroke", "m", sl);
//            registerParameter("m_e", "Equivalent Load Mass", "kg", me);
//            registerParameter("V_1", "Dead Volume in Chamber 1", "m^3", V01);
//            registerParameter("V_2", "Dead Volume in Chamber 2", "m^3", V02);
//            registerParameter("B_p", "Viscous Friction", "Ns/m", bp);
//            registerParameter("Beta_e", "Bulk Modulus", "Pa", betae);
//            registerParameter("c_leak", "Leakage Coefficient", "", cLeak);

//            setStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
//            setStartValue(mpP2, NodeHydraulic::Pressure, 1.0e5);
//        }


//        void initialize()
//        {
//            //Assign node data pointers
//            mpp1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
//            mpq1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
//            mpc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
//            mpZc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
//            mpp2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
//            mpq2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
//            mpc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
//            mpZc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
//            mpf3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
//            mpx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
//            mpv3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
//            mpc3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
//            mpZx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

//            //Read variables from nodes
//            double p1 = (*mpp1);
//            double q1 = (*mpq1);
//            double p2 = (*mpp2);
//            double q2 = (*mpq2);
//            double f3 = (*mpf3);
//            double x3 = (*mpx3);
//            double v3 = (*mpv3);

//            double c1, Zc1, c2, Zc2, c3, Zx3;
//            double qi1, qi2, V1, V2, xi3, vi3;

//            zlim0 = wfak * me / mTimestep;
//            V1min = betae * mTimestep*mTimestep * A1*A1 / (wfak * me);
//            V2min = betae * mTimestep*mTimestep * A2*A2 / (wfak * me);
//            xi3 = -x3;
//            vi3 = -v3;
//            V1 = V01 + A1 * xi3;
//            V2 = V01 + A2 * (sl - xi3);
//            if (V1 < V1min) { V1 = V1min; }
//            if (V2 < V2min) { V2 = V2min; }
//            Zc1 = betae * mTimestep / V1;
//            Zc2 = betae * mTimestep / V2;

//            c1 = p1 - Zc1 * q1;
//            c2 = p2 - Zc2 * q2;

//            qi1 = -A1 * vi3;
//            qi2 = A2 * vi3;
//            ci1 = p1 - Zc1 * (qi1 - cLeak * (p1 - p2));
//            ci2 = p2 - Zc2 * (qi2 - cLeak * (p2 - p1));

//            c3 = f3;
//            Zx3 = A1 * A1 * Zc1 + A2 * A2 * Zc2 + bp;

//             //Write to nodes
//            (*mpc1) = c1;
//            (*mpZc1) = Zc1;
//            (*mpc2) = c2;
//            (*mpZc2) = Zc2;
//            (*mpc3) = c3;
//            (*mpZx3) = Zx3;
//        }

//        void simulateOneTimestep()
//        {
//            //Declare local variables;
//            double pi1, pi2, clim, zlim, qi1, qi2, V1, V2, xi3, vi3, c1_0, ci1_0, c2_0, ci2_0, cp1_0, cp2_0;
//            double p1, q1, p2, q2, c1, c2, x3, v3, c3, Zc1, Zc2, Zx3;

//            //Read variables from nodes
//            p1 = (*mpp1);
//            q1 = (*mpq1);
//            p2 = (*mpp2);
//            q2 = (*mpq2);
//            c1 = (*mpc1);
//            c2 = (*mpc2);
//            x3 = (*mpx3);
//            v3 = (*mpv3);

//            //Internal mechanical port
//            xi3 = -x3;
//            vi3 = -v3;

//            //Calculate volumes
//            V1 = V01 + A1 * xi3;
//            V2 = V01 + A2 * (sl - xi3);
//            if (V1 < V1min) { V1 = V1min; }
//            if (V2 < V2min) { V2 = V2min; }

//            //Volume impedances
//            Zc1 = betae * mTimestep / V1;
//            Zc2 = betae * mTimestep / V2;

//            //Calculate internal flow and pressure
//            qi1 = -A1 * vi3;
//            qi2 = A2 * vi3;
//            pi1 = (ci1 + qi1 * Zc1 + cLeak * (ci2 * Zc1 + ci1 * Zc2)) / (cLeak * (Zc1 + Zc2) + 1);
//            pi2 = (ci2 + qi2 * Zc2 + cLeak * (ci2 * Zc1 + ci1 * Zc2)) / (cLeak * (Zc1 + Zc2) + 1);
//            if (pi1 < 0.0) { pi1 = 0.0; }
//            if (pi2 < 0.0) { pi2 = 0.0; }
//            qi1 = qi1 - cLeak * (pi1 - pi2);
//            qi2 = qi2 - cLeak * (pi2 - pi1);

//            //Calucluate wave variables in chamber 1
//            c1_0 = pi1 + 2*Zc1*qi1;
//            ci1_0 = p1 + 2*Zc1*q1;
//            c1 = alfa*c1 + (1 - alfa)*c1_0;
//            ci1 = alfa*c1 + (1 - alfa)*ci1_0;

//            //Calucluate wave variables in chamber 2
//            c2_0 = pi2 + 2*Zc2*qi2;
//            ci2_0 = p2 + 2*Zc2*q2;
//            c2 = alfa*c2 + (1 - alfa)*c2_0;
//            ci2 = alfa*c2 + (1 - alfa)*ci2_0;

//            //Calculate mean pressure in chambers
//            if (p1 < 0.0) { p1 = 0.0; }
//            cp1_0 = (p1 + Zc1*q1 + pi1 + Zc1*qi1) / 2;
//            if (p2 < 0.0) { p2 = 0.0; }
//            cp2_0 = (p2 + Zc2*q2 + pi2 + Zc2*qi2) / 2;
//            ci1 = (1 - alfa) * cp1_0 + alfa * ci1;
//            ci2 = (1 - alfa) * cp2_0 + alfa * ci2;

//            //Calculate force (with limitation function)
//            this->limitStroke(&clim, &zlim, &xi3, &vi3, &zlim0, &sl, mTime, mTimestep);
//            c3 = ci1*A1 - ci2*A2;// + clim;
//            Zx3 = A1*A1 * Zc1 + A2*A2 * Zc2 + bp;// + zlim;

//            //Write to nodes
//           (*mpc1) = c1;
//           (*mpZc1) = Zc1;
//           (*mpc2) = c2;
//           (*mpZc2) = Zc2;
//           (*mpc3) = c3;
//           (*mpZx3) = Zx3;
//        }


//        //This function was translated from old HOPSAN using F2C. A few manual adjustments were necessary.

//        /* ---------------------------------------------------------------- */
//        /*     Function that simulate the end of the stroke. If X is        */
//        /*     smaller than 0 or greater than SL a large spring force will  */
//        /*     act to force X into the interval again. The spring constant  */
//        /*     is as high possible without numerical instability.           */
//        /* ---------------------------------------------------------------- */

//        void limitStroke(double *clp, double *zlim, double *xp, double *sxp, double *zlim0, double *sl, double time, double timestep)
//        {
//            static double alfa = .5f;

//            /* Local variables */
//            static double climp, flimp, climp0, ka, cp, kz;

//            //  Initialization
//            if (time == 0) {
//                kz = *zlim0 / timestep;
//                if (*xp > *sl) { flimp = -kz * (*xp - *sl); }
//                else if (*xp < 0.0) { flimp = -kz * *xp; }
//                else { flimp = 0.0; }
//                cp = 0.0;
//            }

//            //Equations
//            ka = 1 / (1 - alfa);
//            kz = *zlim0 / timestep;
//            if (*xp > *sl)
//            {
//                *zlim = ka * *zlim0;
//                flimp = -kz * (*xp - *sl);
//                climp0 = flimp - *zlim * *sxp;
//            }
//            else if (*xp < 0.0)
//            {
//                *zlim = ka * *zlim0;
//                flimp = -kz * *xp;
//                climp0 = flimp - *zlim * *sxp;
//            }
//            else
//            {
//                *zlim = 0.0;
//                climp0 = 0.0;
//            }

//            // Filtering of the characteristics
//            climp = alfa * cp + (1 - alfa) * climp0;
//            cp = climp;
//            *clp = climp;
//        }
//    };
//}

//#endif

