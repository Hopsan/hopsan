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
//#include <iostream>
//#include <vector>
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
        double mZx;
//        double Alpha;
//        double Kappa;
//        double Cs;
//        double Wavespeed;
//        double Wf;
//        double LCorr;
//        double AreaCorr;
//        double Area;
//        int NofEl;
//        int indexToBeDelayed, indexDelayed;
//        double Cx1;
//        double Cx2;
        double mCx1old;
        double mCx2old;
        double mV0;//, F1S, F2S;
//        deque<double> Cx1NofEl;
//        deque<double> Cx2NofEl;
        Delay mCx1NofEl;
        Delay mCx2NofEl;
        FirstOrderFilter mFilterLPCx1, mFilterLPCx2;
        Port *mpP1, *mpP2;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_F1, *mpND_V1, *mpND_Cx1, *mpND_Zx1, *mpND_F2, *mpND_V2, *mpND_Cx2, *mpND_Zx2;

    public:
        static Component *Creator()
        {
            return new bar();
        }

        bar() : ComponentC()
        {
            //Set member attributes
//            Alpha=0.0;      //Filtering of characteristics if >0
//            Kappa=0.035;    //Parameter required for  damping (verified for steel)
//            Cs=1.01;        //Parameter required for  damping (verified for steel)

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core, and set default values
            D=0.05;
            L=0.5;
            RHOB=7800.0;
            EB=2.1e11;
            DMP=1.0;
            mV0=0.0;
//            F1S=0.0;
//            F2S=0.0;
            registerParameter("D", "Diameter", "m",  D);
            registerParameter("L", "Length", "m",  L);
            registerParameter("RHOB", "Density", "kg/m3", RHOB);
            registerParameter("EB", "Young's modulus", "Pa", EB);
            registerParameter("DMP", ">0 gives damping", "-", DMP);
            registerParameter("v21", "start value velocity 2->1", "m/s", mV0);
//            registerParameter("f1", "start value force 1", "-", F1S);
//            registerParameter("f2", "start value force 2", "-", F2S);
        }


        void initialize()
        {
            //Assign node data pointeres
            mpND_F1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_V1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_Cx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
            mpND_F2 = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_V2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_Cx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            //Startvalues, read force and velocity from connected Q-types  DOES NOT WORK!!!!
//            F1S = pP1->readNode(NodeMechanic::FORCE);
//            V1S = pP1->readNode(NodeMechanic::VELOCITY);
//            F2S = pP2->readNode(NodeMechanic::FORCE);
//            V2S = pP2->readNode(NodeMechanic::VELOCITY);
            double F1 = *mpND_F1;
            //double V1 = *V1_ptr;
            double F2 = *mpND_F2;
            //double V2 = *V2_ptr;
            //! @note we use a parameter to determine startvalue any startvalues given to the nodes are overwritten by this paramter (V0)

            //Constants that are local whithin initialize
            //const double Alpha=0.0;      //Filtering of characteristics if >0
            const double Kappa=0.035;    //Parameter required for  damping (verified for steel)
            const double Cs=1.01;        //Parameter required for  damping (verified for steel)

            double Wavespeed, Wf;

            //Wave speed
            if (DMP>0.0)Wavespeed=sqrt(EB/RHOB)*Cs;
            else Wavespeed=sqrt(EB/RHOB);

            //Modification of length & area according to actual timestep
            int NofEl = max( 1.0 , (L/Wavespeed/mTimestep + 0.5)); // Nearest integer >=1
            double LCorr = Wavespeed*NofEl*mTimestep;
            double Area = 3.141593/4.0*D*D;
            double AreaCorr=Area*L/LCorr;  //Mass will be OK

            //Characteristic impedance
            mZx = RHOB*Wavespeed*AreaCorr;

            //Start values for wave variables
            double Cx1=F1+mZx*(-mV0);
            double Cx2=F2+mZx*( mV0);
//            Cx1NofEl.assign( int(NofEl-1) , Cx1);
//            Cx2NofEl.assign( int(NofEl-1) , Cx2);
            mCx1NofEl.initialize(NofEl, Cx1);
            mCx2NofEl.initialize(NofEl, Cx2);
            mCx1old=F2-mZx*(-mV0);
            mCx2old=F1-mZx*( mV0);

            //Filter frequency and initialization
            if (DMP>0.0)Wf=1./(Kappa*NofEl*mTimestep);
            else Wf=1./mTimestep;
            double num [2] = {0.0, 1.0};
            double den [2] = {1.0/Wf, 1.0}; //{1.0/wCutoff, 1.0};
            mFilterLPCx1.initialize(mTimestep, num, den,Cx1,Cx1, -1.5E+300, 1.5E+300);
            mFilterLPCx2.initialize(mTimestep, num, den,Cx2,Cx2, -1.5E+300, 1.5E+300);

            //Write characteristics to nodes
//            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx2old);  //Cx(N1) = Cx2old
//            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
//            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx1old);  //Cx(N2) = Cx1old
//            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);
            *mpND_Cx1 = mCx2old;
            *mpND_Zx1 = mZx;
            *mpND_Cx2 = mCx1old;
            *mpND_Zx2 = mZx;
            //Start values...
//            pP1->writeNode(NodeMechanic::VELOCITY,    -V0);
//            pP2->writeNode(NodeMechanic::VELOCITY,     V0);  //Cx(N2) = Cx1old
            *mpND_V1 = -mV0;
            *mpND_V2 = mV0;
            //! @note we use a parameter to determine startvalue any startvalues given to the nodes are overwritten by this paramter (V0)
        }

        void simulateOneTimestep()
        {

             //Get variable values from nodes
//            double V1 = pP1->readNode(NodeMechanic::VELOCITY);
//            double V2 = pP2->readNode(NodeMechanic::VELOCITY);
            double V1 = *mpND_V1;
            double V2 = *mpND_V2;

//            Cx1NofEl.push_back(Cx2old + 2.*Zx*V1);  //Add new value at the end
//            Cx2NofEl.push_back(Cx1old + 2.*Zx*V2);
//            double Cx1new=Cx1NofEl.front(); Cx1NofEl.pop_front();  //Read and remove first value
//            double Cx2new=Cx2NofEl.front(); Cx2NofEl.pop_front();
            double Cx1new = mCx1NofEl.update(mCx2old + 2.*mZx*V1);  //Add new value, pop old
            double Cx2new = mCx2NofEl.update(mCx1old + 2.*mZx*V2);

             //First order filter
            double Cx1=mFilterLPCx1.update(Cx1new);
            double Cx2=mFilterLPCx2.update(Cx2new);

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
//            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
//            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
//            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);
            *mpND_Cx1 = Cx2;
            *mpND_Zx1 = mZx;
            *mpND_Cx2 = Cx1;
            *mpND_Zx2 = mZx;

            //Update the delayed variabels
            mCx1old=Cx1;
            mCx2old=Cx2;
        }
    };
}

#endif // bar_HPP_INCLUDED
