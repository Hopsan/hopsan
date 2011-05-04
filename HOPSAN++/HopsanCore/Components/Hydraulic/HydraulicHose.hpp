//!
//! @file   HydraulicHose.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-05-04
//!
//! @brief Contains a Hydraulic Hose Component
//! Written by Petter Krus 910407
//! Revised 920415
//! Revised by Robert Braun 110504
//$Id$

#ifndef HYDRAULICHOSE_HPP_INCLUDED
#define HYDRAULICHOSE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicHose : public ComponentC
    {

    private:

        double BETAE, RHO, VISC, WVISC, D, L;
        double PI, NTMAX, KAPPA;
        int NTIME, NTIME1;
        double RTOT, RL1D, RL2D, RW, A, TL, NT, TN, AREA, AREAC, ZC0, ALFA, DEN, BC0, BC1, AC1, RQ10, RQ20, RQF1D, RQF2D, RQEF1D, RQEF2D, C1F1, C2F1, RQ1, RL1, RQ2, RL2, RL, W1, W2, W3, W4, C1F, C2F, RQF1, RQF2, RQEF1, RQEF2;
        double c1i[1001];       //1001 because 1000 with starting at 1 (stupid Fortran thing)
        double c2i[1001];
        //double rq(double &RL, double Q, double D, double L, double RHO, double VISC);
        FirstOrderFilter FilterC1F, FilterC2F, FilterC1F1, FilterC2F1;
        double numC1F[2], denC1F[2];
        double numC2F[2], denC2F[2];
        double numC1F1[2], denC1F1[2];
        double numC2F1[2], denC2F1[2];

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicHose("Hose");
        }

        HydraulicHose(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            BETAE = 1e9;
            RHO = 870;
            VISC = 0.03;
            WVISC = 0.03;
            D = 0.03;
            L = 1;

            PI = 3.1415926;
            NTMAX = 1000;
            KAPPA = 1.25;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("beta_e", "Bulk modulus",              "[Pa]",     BETAE);
            registerParameter("rho",    "Density",                   "[kg/m^3]", RHO);
            registerParameter("eta",    "Dynamic oil viscosity",     "[Ns/m^2]", VISC);
            registerParameter("eta_w",  "Equivalent wall viscosity", "[Ns/m^2]", WVISC);
            registerParameter("d",      "Line diameter",             "[m]",      D);
            registerParameter("l",      "Line length",               "[m]",      L);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

                //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

                //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);

                // Line resistance
            RTOT = 128*VISC*L/(PI*D*D*D*D);
            RL1 = RTOT;                         // Added to prevent uninitialized values, but not necessary
            RL2 = RTOT;
            RL1D = RTOT;
            RL2D = RTOT;
            RW = 128*WVISC*L/(PI*D*D*D*D);

                // Speed of sound in the line
            A=sqrt(BETAE/RHO);

                // Characteristic impedance
            TL = L/A;
            NT = TL/mTimestep+0.5;

            TN = NT*mTimestep;

            AREA = PI*D*D/4;
            AREAC = AREA*TL/TN;
            ZC0= RHO*A/AREAC;
            if(NT < 1) NT = 1;
            TN = NT*mTimestep;

            ALFA  = RTOT/(ZC0*TN);

            DEN = (2*KAPPA*NT + 1);
            BC0 = 1/DEN;
            BC1 = 1/DEN;
            AC1 = -(2*KAPPA*NT - 1)/DEN;

            Zc1 = ZC0 + BC0*RTOT;
            Zc2 = ZC0 + BC0*RTOT;

            for(size_t i=1; i<NTMAX+1; ++i)
            {
                c1i[i] = p1 + ZC0*q1;
                c2i[i] = p2 + ZC0*q2;
            }

            NTIME=1;

            RQ10 = RTOT*q1;
            RQ20 = RTOT*q2;
            RQF1D = RQ10*(1 - BC0);
            RQF2D = RQ20*(1 - BC0);
            RQEF1D = 0.0;
            RQEF2D = 0.0;

            C1F1 = p1 + ZC0*q1;
            C2F1 = p2 + ZC0*q2;

            c1 = p1 - Zc1*q1;
            c2 = p2 - Zc2*q2;

            FilterC1F.initialize(mTimestep, numC1F, denC1F, c1i[NTIME], c1i[NTIME]);        //! @todo Not sure if start values for filters are correct
            FilterC2F.initialize(mTimestep, numC2F, denC2F, c2i[NTIME], c2i[NTIME]);
            FilterC1F1.initialize(mTimestep, numC1F1, denC1F1, c1i[NTIME], C1F1);
            FilterC2F1.initialize(mTimestep, numC2F1, denC2F1, c2i[NTIME], C2F1);

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;
        }


        void simulateOneTimestep()
        {
                //Declare local variables
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2;

                //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);

                //Cyclic memory
            NTIME = NTIME+1;
            if(NTIME > NTMAX) NTIME=1;

            NTIME1 = NTIME-1*NT + 1;
            if(NTIME1 < 1) NTIME1 = NTIME1 + NTMAX;

                //-------- Line modell -----------
            RQ1 = rq(RL1,q1,D,L,RHO,VISC);
            RQ2 = rq(RL2,q2,D,L,RHO,VISC);
            RL1 = (RL1 + RL1D)/2;
            RL2 = (RL2 + RL2D)/2;
            RL1D = RL1;
            RL2D = RL2;
            RL = (RL1+RL2)/2;

            Zc1 = ZC0 + BC0*RL1;
            Zc2 = ZC0 + BC0*RL2;

            c1i[NTIME] = c1 + 2*ZC0*q1;
            c2i[NTIME] = c2 + 2*ZC0*q2;

                // Low pass filtering of transmitted signals
            W1 = 1/(KAPPA*TN);
            W2 = W1*exp(RL/(2*ZC0));
            W3 = RW/(2*ZC0*TN);
            W4 = W3*exp(RW/(2*ZC0));


            numC1F[0] = 1/W2;
            numC1F[1] = 1;
            denC1F[0] = 1/W1;
            denC1F[1] = 1;
            FilterC1F.setNumDen(numC1F, denC1F);
            C1F = FilterC1F.update(c1i[NTIME1]);

            numC2F[0] = 1/W2;
            numC2F[1] = 1;
            denC2F[0] = 1/W1;
            denC2F[1] = 1;
            FilterC2F.setNumDen(numC2F, denC2F);
            C2F = FilterC2F.update(c2i[NTIME1]);

            numC1F1[0] = 1/W4;
            numC1F1[1] = 1;
            denC1F1[0] = 1/W3;
            denC1F1[1] = 1;
            FilterC1F1.setNumDen(numC1F1, denC1F1);
            C1F1 = FilterC1F1.update(C1F);

            numC2F1[0] = 1/W4;
            numC2F1[1] = 1;
            denC2F1[0] = 1/W3;
            denC2F1[1] = 1;
            FilterC2F1.setNumDen(numC2F1, denC2F1);
            C2F1 = FilterC2F1.update(C2F);

            RQF1D = RQF1D + BC0*RL1*q1;
            RQF2D = RQF2D + BC0*RL2*q2;

            RQF1 = -AC1*RQF1D + BC1*RL1*q1;
            RQF2 = -AC1*RQF2D + BC1*RL2*q2;

            RQF1D = RQF1;
            RQF2D = RQF2;

            RQEF1 = -AC1*RQEF1D + (BC0+BC1)*(RQ1-RL1*q1);
            RQEF2 = -AC1*RQEF2D + (BC0+BC1)*(RQ2-RL2*q2);

            RQEF1D = RQEF1;
            RQEF2D = RQEF2;

                // Calculation of caracteristics in N1 and N2.
            c1 = C2F1 + RQF1 + BC0*(-RL1*q1) + RQEF1;
            c2 = C1F1 + RQF2 + BC0*(-RL2*q2) + RQEF2;

                // Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc2;

        }


            //! @brief Subroutine for RQ-factorization
            //! Translated from old Hopsan with only syntax changes
        double rq(double &RL, double Q, double D, double L, double RHO, double VISC)
        {
            double PI = 3.14159;
            double RE,F;
            double RECRIT = 2300;
            double RQ;

            RE = fabs(4*RHO*Q/(PI*D*VISC));

            if(RE < RECRIT)
            {
              RL=128*VISC*L/(PI*D*D*D*D);
              RQ=RL*Q;
            }
            else
            {
              F = 0.079*pow(RE,-0.25);
              RL= 1.75*32*RHO*F*L*fabs(Q)/(PI*PI*D*D*D*D*D);
              RQ = 32*RHO*F*L*fabs(Q)*Q/(PI*PI*D*D*D*D*D);
            }

            return RQ;
        }
    };
}

#endif // HYDRAULICHOSE_HPP_INCLUDED

