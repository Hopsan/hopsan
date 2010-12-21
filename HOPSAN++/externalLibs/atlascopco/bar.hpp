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
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
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
        double Cx1;
        double Cx2;
        double Cx1old;
        double Cx2old;
        double V0, F1S, F2S;
//        deque<double> Cx1NofEl;
//        deque<double> Cx2NofEl;
        Delay Cx1NofEl;
        Delay Cx2NofEl;
        FirstOrderFilter mFilterLPCx1, mFilterLPCx2;
        Port *pP1, *pP2;

    public:
        static Component *Creator()
        {
            return new bar();
        }

        bar() : ComponentC()
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
            //Startvalues, read force and velocity from connected Q-types  DOES NOT WORK!!!!
            //F1S = pP1->readNode(NodeMechanic::FORCE);
            //V1S = pP1->readNode(NodeMechanic::VELOCITY);
            //F2S = pP2->readNode(NodeMechanic::FORCE);
            //V2S = pP2->readNode(NodeMechanic::VELOCITY);

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
            Cx1=F1S+Zx*(-V0);
            Cx2=F2S+Zx*( V0);
//            Cx1NofEl.assign( int(NofEl-1) , Cx1);
//            Cx2NofEl.assign( int(NofEl-1) , Cx2);
            Cx1NofEl.initialize(NofEl, Cx1);
            Cx2NofEl.initialize(NofEl, Cx2);
            Cx1old=F2S-Zx*(-V0);
            Cx2old=F1S-Zx*( V0);

            //Filter frequency and initialization
            if (DMP>0.0)Wf=1./(Kappa*NofEl*mTimestep);
            else Wf=1./mTimestep;
            double num [2] = {0.0, 1.0};
            double den [2] = {1.0/Wf, 1.0}; //{1.0/wCutoff, 1.0};
            mFilterLPCx1.initialize(mTimestep, num, den,Cx1,Cx1, -1.5E+300, 1.5E+300);
            mFilterLPCx2.initialize(mTimestep, num, den,Cx2,Cx2, -1.5E+300, 1.5E+300);

            //Write characteristics to nodes
            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx2old);  //Cx(N1) = Cx2old
            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx1old);  //Cx(N2) = Cx1old
            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);
            //Start values...
            pP1->writeNode(NodeMechanic::VELOCITY,    -V0);
            pP2->writeNode(NodeMechanic::VELOCITY,     V0);  //Cx(N2) = Cx1old


        }

        void simulateOneTimestep()
        {

             //Get variable values from nodes
            double V1 = pP1->readNode(NodeMechanic::VELOCITY);
            double V2 = pP2->readNode(NodeMechanic::VELOCITY);

//            Cx1NofEl.push_back(Cx2old + 2.*Zx*V1);  //Add new value at the end
//            Cx2NofEl.push_back(Cx1old + 2.*Zx*V2);

//            double Cx1new=Cx1NofEl.front(); Cx1NofEl.pop_front();  //Read and remove first value
//            double Cx2new=Cx2NofEl.front(); Cx2NofEl.pop_front();

            double Cx1new = Cx1NofEl.update(Cx2old + 2.*Zx*V1);  //Add new value, pop old
            double Cx2new = Cx2NofEl.update(Cx1old + 2.*Zx*V2);

             //First order filter
            Cx1=mFilterLPCx1.update(Cx1new);
            Cx2=mFilterLPCx2.update(Cx2new);

            //Write new values to nodes
            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);

            //Update the delayed variabels
            Cx1old=Cx1;
            Cx2old=Cx2;
        }
    };
}

#endif // bar_HPP_INCLUDED
