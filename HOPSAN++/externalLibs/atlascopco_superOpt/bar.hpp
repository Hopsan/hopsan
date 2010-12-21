/*****************************************************************

Mechanical Transmission line model
Derived from the former model "subroutine steel".

Maria Pettersson
20101014

Schematic image:
-----------
I         I
-----------
*****************************************************************/
#ifndef bar_HPP_INCLUDED
#define bar_HPP_INCLUDED

#include <math.h>
#include <iostream>
#include <vector>
using namespace std;
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class bar : public ComponentC
    {

    private:

        double D;
        double L;
        double RHOB;
        double EB;
        double DMP;
        double Zx;
        double Alpha;
        double Kappa;
        double Cs;
        double Wavespeed;
        double Wf;
        double LCorr;
        double AreaCorr;
        double Area;
        int NofEl;
        int indexToBeDelayed, indexDelayed;
        double Cx1old;
        double Cx2old;
        double V0, F1S, F2S;
        Delay Cx1NofEl;
        Delay Cx2NofEl;
        Port *pP1, *pP2;
        double Cx1, Cx2;

            //Filter
        double mCx1FilterDelayU;
        double mCx1FilterDelayY;
        double mCx1FilterCoeffU[2];
        double mCx1FilterCoeffY[2];

        double mCx2FilterDelayU;
        double mCx2FilterDelayY;
        double mCx2FilterCoeffU[2];
        double mCx2FilterCoeffY[2];
            //

        double *v1, *v2, *c1, *Zx1, *c2, *Zx2;
        double Cx1new, Cx2new;

    public:
        static Component *Creator()
        {
            return new bar("bar");
        }

        bar(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "bar";
            Alpha=0.0;//Filtering of characteristics if >0
            Kappa=0.035;  //Parameter required for  damping (verified for steel)
            Cs=1.01;      //Parameter required for  damping (verified for steel)

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            D=0.05;
            L=0.5;
            RHOB=7800.0;
            EB=2.1e11;
            DMP=1.0;
            V0=0.0;
            F1S=0.0;
            F2S=0.0;
            registerParameter("D", "Diameter", "m",  D);
            registerParameter("L", "Length", "m",  L);
            registerParameter("RHOB", "Density", "kg/m3", RHOB);
            registerParameter("EB", "Young's modulus", "Pa", EB);
            registerParameter("DMP", ">0 gives damping", "-", DMP);
            registerParameter("v21", "start value velocity 2->1", "m/s", V0);
            registerParameter("f1", "start value force 1", "-", F1S);
            registerParameter("f2", "start value force 2", "-", F2S);
        }


        void initialize()
        {
            v1 = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            v2 = pP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            c1 = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1 = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            c2 = pP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2 = pP2->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Wave speed
            if (DMP>0.0)Wavespeed=sqrt(EB/RHOB)*Cs;
            else Wavespeed=sqrt(EB/RHOB);

            //Modification of length & area according to actual timestep
            int NofEl = max( 1.0 , (L/Wavespeed/mTimestep + 0.5)); // Nearest integer >=1
            LCorr = Wavespeed*NofEl*mTimestep;
            Area = 3.141593/4.0*D*D;
            AreaCorr=Area*L/LCorr;  //Mass will be OK

            //Characteristic impedance
            Zx = RHOB*Wavespeed*AreaCorr;

            //Start values for wave variables
            Cx1 = F1S+Zx*(-V0);
            Cx2 = F2S+Zx*( V0);
            Cx1NofEl.initialize(NofEl, Cx1);
            Cx2NofEl.initialize(NofEl, Cx2);
            Cx1old=F2S-Zx*(-V0);
            Cx2old=F1S-Zx*( V0);

            //Filter frequency and initialization
            if (DMP>0.0)Wf=1./(Kappa*NofEl*mTimestep);
            else Wf=1./mTimestep;
            double num [2] = {0.0, 1.0};
            double den [2] = {1.0/Wf, 1.0}; //{1.0/wCutoff, 1.0};

                //Filter
            mCx1FilterDelayU = Cx1;
            mCx1FilterDelayY = Cx1;

            mCx1FilterCoeffU[0] = num[1]*mTimestep-2.0*num[0];
            mCx1FilterCoeffU[1] = num[1]*mTimestep+2.0*num[0];
            mCx1FilterCoeffY[0] = den[1]*mTimestep-2.0*den[0];
            mCx1FilterCoeffY[1] = den[1]*mTimestep+2.0*den[0];

            mCx2FilterDelayU = Cx2;
            mCx2FilterDelayY = Cx2;

            mCx2FilterCoeffU[0] = num[1]*mTimestep-2.0*num[0];
            mCx2FilterCoeffU[1] = num[1]*mTimestep+2.0*num[0];
            mCx2FilterCoeffY[0] = den[1]*mTimestep-2.0*den[0];
            mCx2FilterCoeffY[1] = den[1]*mTimestep+2.0*den[0];
                //

            //Write characteristics to nodes
            *c1 = Cx2old;
            *Zx1 = Zx;
            *c2 = Cx1old;
            *Zx2 = Zx;

            //Start values...
            *v1 = -V0;
            *v2 = V0;
        }

        void simulateOneTimestep()
        {
            Cx1new = Cx1NofEl.update(Cx2old + 2.*Zx * *v1);
            Cx2new = Cx2NofEl.update(Cx1old + 2.*Zx * *v2);

            //First order filter
            *c2 = 1.0/mCx1FilterCoeffY[1]*(mCx1FilterCoeffU[1]*Cx1new + mCx1FilterCoeffU[0]*mCx1FilterDelayU - mCx1FilterCoeffY[0]*mCx1FilterDelayY);
            mCx1FilterDelayU = Cx1new;
            mCx1FilterDelayY = *c2;

            *c1 = 1.0/mCx2FilterCoeffY[1]*(mCx2FilterCoeffU[1]*Cx2new + mCx2FilterCoeffU[0]*mCx2FilterDelayU - mCx2FilterCoeffY[0]*mCx2FilterDelayY);
            mCx2FilterDelayU = Cx2new;
            mCx2FilterDelayY = *c1;

            //*Zx1 = Zx;
            //*Zx2 = Zx;
            
            //Update the delayed variabels
            Cx1old = *c2;
            Cx2old = *c1;
        }
    };
}

#endif // bar_HPP_INCLUDED
