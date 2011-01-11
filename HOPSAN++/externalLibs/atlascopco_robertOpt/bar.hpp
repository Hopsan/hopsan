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
        double Cx1old;
        double Cx2old;
        double Cx1new, Cx2new;
//        deque<double> Cx1NofEl;
//        deque<double> Cx2NofEl;
        Delay Cx1NofEl;
        Delay Cx2NofEl;
        FirstOrderFilter mFilterLPCx1, mFilterLPCx2;

        double F1, V1, Cx1, F2, V2, Cx2, Zx2;
        double *F1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *F2_ptr, *V2_ptr, *Cx2_ptr, *Zx2_ptr;
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

            registerParameter("D", "Diameter", "m",  D);
            registerParameter("L", "Length", "m",  L);
            registerParameter("RHOB", "Density", "kg/m3", RHOB);
            registerParameter("EB", "Young's modulus", "Pa", EB);
            registerParameter("DMP", ">0 gives damping", "-", DMP);
        }


        void initialize()
        {
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            F2_ptr = pP2->getNodeDataPtr(NodeMechanic::FORCE);
            V2_ptr = pP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx2_ptr = pP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2_ptr = pP2->getNodeDataPtr(NodeMechanic::CHARIMP);



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
            Cx1=F1+Zx*(V1);
            Cx2=F2+Zx*(V2);
//            Cx1NofEl.assign( int(NofEl-1) , Cx1);
//            Cx2NofEl.assign( int(NofEl-1) , Cx2);
            Cx1NofEl.initialize(NofEl, Cx1);
            Cx2NofEl.initialize(NofEl, Cx2);
            Cx1old=F2-Zx*(V1);
            Cx2old=F1-Zx*(V2);

            //Filter frequency and initialization
            if (DMP>0.0)Wf=1./(Kappa*NofEl*mTimestep);
            else Wf=1./mTimestep;
            double num [2] = {0.0, 1.0};
            double den [2] = {1.0/Wf, 1.0}; //{1.0/wCutoff, 1.0};
            mFilterLPCx1.initialize(mTimestep, num, den,Cx1,Cx1, -1.5E+300, 1.5E+300);
            mFilterLPCx2.initialize(mTimestep, num, den,Cx2,Cx2, -1.5E+300, 1.5E+300);

            //Write characteristics to nodes
            (*Cx1_ptr) = Cx2old;
            (*Zx1_ptr) = Zx;
            (*Cx2_ptr) = Cx1old;
            (*Zx2_ptr) = Zx;
        }

        void simulateOneTimestep()
        {

             //Get variable values from nodes
            V1 = (*V1_ptr);
            V2 = (*V2_ptr);

//            Cx1NofEl.push_back(Cx2old + 2.*Zx*V1);  //Add new value at the end
//            Cx2NofEl.push_back(Cx1old + 2.*Zx*V2);

//            double Cx1new=Cx1NofEl.front(); Cx1NofEl.pop_front();  //Read and remove first value
//            double Cx2new=Cx2NofEl.front(); Cx2NofEl.pop_front();

            Cx1new = Cx1NofEl.update(Cx2old + 2.*Zx*V1);  //Add new value, pop old
            Cx2new = Cx2NofEl.update(Cx1old + 2.*Zx*V2);

             //First order filter
            Cx1=mFilterLPCx1.update(Cx1new);
            Cx2=mFilterLPCx2.update(Cx2new);

            //Write new values to nodes
            (*Cx1_ptr) = Cx2;
            (*Zx1_ptr) = Zx;
            (*Cx2_ptr) = Cx1;
            (*Zx2_ptr) = Zx;

            //Update the delayed variabels
            Cx1old=Cx1;
            Cx2old=Cx2;
        }
    };
}

#endif // bar_HPP_INCLUDED
