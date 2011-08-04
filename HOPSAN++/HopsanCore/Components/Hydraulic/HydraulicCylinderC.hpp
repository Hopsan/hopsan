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
        double A1,A2,sl,cLeak,bp,me,betae,V01,V02, CxLim, ZxLim, wfak;

        double c1, ci1, cl1, c2, ci2, cl2;  //Members because old value need to be remembered
        double mNum[2];
        double mDen[2];
        //FirstOrderTransferFunction mFilter;
        Integrator mInt;

        //Node data pointers
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_f3, *mpND_x3, *mpND_v3, *mpND_c3, *mpND_Zx3;

        //Ports
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicCylinderC("CylinderC");
        }

        HydraulicCylinderC(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            wfak = 0.1;
            betae = 1000000000.0;
            me = 100.0;
            V01 = 0.0003;
            V02 = 0.0003;
            A1 = 0.001;
            A2 = 0.001;
            sl = 1.0;
            cLeak = 0.00000000001;
            bp = 1000.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("A_1", "Piston Area 1", "[m^2]", A1);
            registerParameter("A_2", "Piston Area 2", "[m^2]", A2);
            registerParameter("s_l", "Stroke", "[m]", sl);
            registerParameter("m_e", "Equivalent Load Mass", "[kg]", me);
            registerParameter("V_1", "Dead Volume in Chamber 1", "[m^3]", V01);
            registerParameter("V_2", "Dead Volume in Chamber 2", "[m^3]", V02);
            registerParameter("B_p", "Viscous Friction", "[Ns/m]", bp);
            registerParameter("Beta_e", "Bulk Modulus", "[Pa]", betae);
            registerParameter("c_leak", "Leakage Coefficient", "[]", cLeak);

            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_f3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
            mpND_x3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
            mpND_v3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

            //Declare local variables;
            double p1, q1, p2, q2, f3, x3, v3;
            double Zc1, Zc2, c3, Zx3;
            double qi1, qi2, V1, V2, qLeak, V1min, V2min;

            //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            f3 = (*mpND_f3);
            x3 = (*mpND_x3);
            v3 = (*mpND_v3);

            //Limit end of stroke limitations
            CxLim = 0;
            ZxLim = 0;

            //Size of volumes
            V1 = V01+A1*(-x3);
            V2 = V01+A2*(sl+x3);
            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*me);
            V2min = betae*mTimestep*mTimestep*A2*A2/(wfak*me);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;

            //Impedences
            Zc1 = betae*mTimestep/V1;
            Zc2 = betae*mTimestep/V2;
            Zx3 = A1*A1*Zc1 +A2*A2*Zc2 + bp;

            //Internal flows
            qi1 = v3*A1;
            qi2 = -v3*A2;

            //Leakage flow
            qLeak = cLeak*(p1-p2);

            //Wave variables
            c1 = p1 + Zc1*q1;
            ci1 = p1 + Zc1*qi1;
            cl1 = p1 + Zc1*(-qLeak);

            c2 = p2 + Zc2*q2;
            ci2 = p2 + Zc2*qi2;
            cl2 = p2 + Zc2*qLeak;

            c3 = A1*ci1 - A2*ci2;

            //Write to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
        }

        void simulateOneTimestep()
        {
            //Declare local variables;
            double p1, q1, p2, q2, f3, x3, v3;
            double Zc1, Zc2, c3, Zx3;

            double V1, V2, qLeak, qi1, qi2, p1mean, p2mean, V1min, V2min;
            double alpha=0.5;

            //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c1 = (*mpND_c1);
            c2 = (*mpND_c2);
            Zc1 = (*mpND_Zc1);
            Zc2 = (*mpND_Zc2);
            f3 = (*mpND_f3);
            x3 = (*mpND_x3);
            v3 = (*mpND_v3);

            //Leakage flow
            qLeak = cLeak*(cl1-cl2)/(1+cLeak*(Zc1+Zc2));

            //Internal flows
            qi1 = v3*A1;
            qi2 = -v3*A2;

            //Size of volumes
            V1 = V01+A1*(-x3);
            V2 = V01+A2*(sl+x3);
            V1min = betae*mTimestep*mTimestep*A1*A1/(wfak*me);
            V2min = betae*mTimestep*mTimestep*A2*A2/(wfak*me);
            if(V1<V1min) V1 = V1min;
            if(V2<V2min) V2 = V2min;

            //Volume equations
            Zc1 = 3 / 2 * betae/V1*mTimestep/(1-alpha);
            p1mean = ((c1 + Zc1*2*q1) + (ci1 + Zc1*2*qi1) + (cl1 + Zc1*2*(-qLeak))) / 3;
            c1 = alpha * c1 + (1.0 - alpha)*(p1mean*2 - c1 - 2*Zc1*q1);
            ci1 = alpha * ci1 + (1.0 - alpha)*(p1mean*2 - ci1 - 2*Zc1*qi1);
            cl1 = alpha * cl1 + (1.0 - alpha)*(p1mean*2 - cl1 - 2*Zc1*(-qLeak));

            Zc2 = 3 / 2 * betae/V2*mTimestep/(1-alpha);
            p2mean = ((c2 + Zc2*2*q2) + (ci2 + Zc2*2*qi2) + (cl2 + Zc2*2*qLeak)) / 3;
            c2 = alpha * c2 + (1.0 - alpha)*(p2mean*2 - c2 - 2*Zc2*q2);
            ci2 = alpha * ci2 + (1.0 - alpha)*(p2mean*2 - ci2 - 2*Zc2*qi2);
            cl2 = alpha * cl2 + (1.0 - alpha)*(p2mean*2 - cl2 - 2*Zc2*qLeak);


            //limitStroke(CxLim, ZxLim, x3, v3, me, sl);

            //Internal mechanical port
            c3 = A1*ci1 - A2*ci2;// + CxLim;
            Zx3 = A1*A1*Zc1 + A2*A2*Zc2 + bp;// + ZxLim;
            //! @note End of stroke limitation currently turned off, because the piston gets stuck in the end position.
            //! @todo Either implement a working limitation, or remove it completely. It works just as well to have it in the mass component.

            //Write to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
            (*mpND_c3) = c3;
            (*mpND_Zx3) = Zx3;
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
            double FxLim, ZxLim0, NewCxLim, alfa;

            alfa = 0.5;

            //Equations
            if (-x3 > sl)
            {
                ZxLim0 = wfak*me/mTimestep;
                ZxLim = ZxLim0/(1 - alfa);
                FxLim = ZxLim0 * (x3 + sl) / mTimestep;
                NewCxLim = FxLim + ZxLim*v3;
            }
            else if (-x3 < 0.0)
            {
                ZxLim0 = wfak*me/mTimestep;
                ZxLim = ZxLim0/(1 - alfa);
                FxLim = ZxLim0*x3/mTimestep;
                NewCxLim = FxLim + ZxLim*v3;
            }
            else
            {
                ZxLim = 0.0;
                CxLim = 0.0;
            }

            // Filtering of the characteristics
            CxLim = alfa * CxLim + (1 - alfa) * NewCxLim;
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

//#include "../../ComponentEssentials.h"
//#include "../../ComponentUtilities.h"

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
//        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_f3, *mpND_x3, *mpND_v3, *mpND_c3, *mpND_Zx3;

//        //Ports
//        Port *mpP1, *mpP2, *mpP3;

//    public:
//        static Component *Creator()
//        {
//            return new HydraulicCylinderC("CylinderC");
//        }

//        HydraulicCylinderC(const std::string name) : ComponentC(name)
//        {
//            //Set member attributes
//            PI = 3.14159265;
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

//            //Register changable parameters to the HOPSAN++ core
//            registerParameter("A_1", "Piston Area 1", "[m^2]", A1);
//            registerParameter("A_2", "Piston Area 2", "[m^2]", A2);
//            registerParameter("s_l", "Stroke", "[m]", SL);
//            registerParameter("m_e", "Equivalent Load Mass", "[kg]", ME);
//            registerParameter("V_1", "Dead Volume in Chamber 1", "[m^3]", V01);
//            registerParameter("V_2", "Dead Volume in Chamber 2", "[m^3]", V02);
//            registerParameter("B_p", "Viscous Friction", "[Ns/m]", BP);
//            registerParameter("Beta_e", "Bulk Modulus", "[Pa]", BETAE);
//            registerParameter("c_leak", "Leakage Coefficient", "[]", CIP);

//            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
//            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
//        }


//        void initialize()
//        {
//            //Assign node data pointers
//            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
//            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
//            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
//            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
//            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
//            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
//            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
//            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
//            mpND_f3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
//            mpND_x3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
//            mpND_v3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
//            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
//            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

//            //Read variables from nodes
//            double P1 = (*mpND_p1);
//            double Q1 = (*mpND_q1);
//            double P2 = (*mpND_p2);
//            double Q2 = (*mpND_q2);
//            double F1 = (*mpND_f3);
//            double X1 = (*mpND_x3);
//            double SX1 = (*mpND_v3);

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
//            (*mpND_c1) = C1;
//            (*mpND_Zc1) = ZC01;
//            (*mpND_c2) = C2;
//            (*mpND_Zc2) = ZC02;
//            (*mpND_c3) = CX1;
//            (*mpND_Zx3) = ZX1;
//        }

//        void simulateOneTimestep()
//        {
//            //Declare local variables;
//            double P1, Q1, P2, Q2, C1, C2, X1, SX1, CX1, ZX1;

//            //Read variables from nodes
//            P1 = (*mpND_p1);
//            Q1 = (*mpND_q1);
//            P2 = (*mpND_p2);
//            Q2 = (*mpND_q2);
//            X1 = (*mpND_x3);
//            SX1 = (*mpND_v3);

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
//            (*mpND_c1) = C1;
//            (*mpND_Zc1) = ZC01;
//            (*mpND_c2) = C2;
//            (*mpND_Zc2) = ZC02;
//            (*mpND_c3) = CX1;
//            (*mpND_Zx3) = ZX1;
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

//#include "../../ComponentEssentials.h"
//#include "../../ComponentUtilities.h"

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
//        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_f3, *mpND_x3, *mpND_v3, *mpND_c3, *mpND_Zx3;

//        //Ports
//        Port *mpP1, *mpP2, *mpP3;

//    public:
//        static Component *Creator()
//        {
//            return new HydraulicCylinderC("CylinderC");
//        }

//        HydraulicCylinderC(const std::string name) : ComponentC(name)
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

//            //Register changable parameters to the HOPSAN++ core
//            registerParameter("A_1", "Piston Area 1", "[m^2]", A1);
//            registerParameter("A_2", "Piston Area 2", "[m^2]", A2);
//            registerParameter("s_l", "Stroke", "[m]", sl);
//            registerParameter("m_e", "Equivalent Load Mass", "[kg]", me);
//            registerParameter("V_1", "Dead Volume in Chamber 1", "[m^3]", V01);
//            registerParameter("V_2", "Dead Volume in Chamber 2", "[m^3]", V02);
//            registerParameter("B_p", "Viscous Friction", "[Ns/m]", bp);
//            registerParameter("Beta_e", "Bulk Modulus", "[Pa]", betae);
//            registerParameter("c_leak", "Leakage Coefficient", "[]", cLeak);

//            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
//            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
//        }


//        void initialize()
//        {
//            //Assign node data pointers
//            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
//            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
//            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
//            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
//            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
//            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
//            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
//            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
//            mpND_f3 = getSafeNodeDataPtr(mpP3, NodeMechanic::FORCE);
//            mpND_x3 = getSafeNodeDataPtr(mpP3, NodeMechanic::POSITION);
//            mpND_v3 = getSafeNodeDataPtr(mpP3, NodeMechanic::VELOCITY);
//            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanic::WAVEVARIABLE);
//            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanic::CHARIMP);

//            //Read variables from nodes
//            double p1 = (*mpND_p1);
//            double q1 = (*mpND_q1);
//            double p2 = (*mpND_p2);
//            double q2 = (*mpND_q2);
//            double f3 = (*mpND_f3);
//            double x3 = (*mpND_x3);
//            double v3 = (*mpND_v3);

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
//            (*mpND_c1) = c1;
//            (*mpND_Zc1) = Zc1;
//            (*mpND_c2) = c2;
//            (*mpND_Zc2) = Zc2;
//            (*mpND_c3) = c3;
//            (*mpND_Zx3) = Zx3;
//        }

//        void simulateOneTimestep()
//        {
//            //Declare local variables;
//            double pi1, pi2, clim, zlim, qi1, qi2, V1, V2, xi3, vi3, c1_0, ci1_0, c2_0, ci2_0, cp1_0, cp2_0;
//            double p1, q1, p2, q2, c1, c2, x3, v3, c3, Zc1, Zc2, Zx3;

//            //Read variables from nodes
//            p1 = (*mpND_p1);
//            q1 = (*mpND_q1);
//            p2 = (*mpND_p2);
//            q2 = (*mpND_q2);
//            c1 = (*mpND_c1);
//            c2 = (*mpND_c2);
//            x3 = (*mpND_x3);
//            v3 = (*mpND_v3);

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
//           (*mpND_c1) = c1;
//           (*mpND_Zc1) = Zc1;
//           (*mpND_c2) = c2;
//           (*mpND_Zc2) = Zc2;
//           (*mpND_c3) = c3;
//           (*mpND_Zx3) = Zx3;
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

