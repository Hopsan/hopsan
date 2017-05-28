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

#ifndef CRANE_HPP_INCLUDED
#define CRANE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Crane : public ComponentQ
    {
    private:
        //Input variable node data pointers
        double *mpL1, *mpM1, *mpJ1, *mpB1, *mpL2, *mpM2, *mpJ2, *mpB2, *mpD1, *mpD2, *mpD3, *mpD4, *mpX01, *mpX02, *mpFload;
        //Output variable node data pointers
        double *mpTh1, *mpW1, *mpTh2, *mpW2, *mpXc2, *mpYc2;    //Last two are only used for animation
        //Power port pointers
        Port *mpP1, *mpP2;
        //Power port node data pointers 
        double *mpP1_x, *mpP1_v, *mpP1_F, *mpP1_me, *mpP1_c, *mpP1_Zc;
        double *mpP2_x, *mpP2_v, *mpP2_F, *mpP2_me, *mpP2_c, *mpP2_Zc;

        SecondOrderTransferFunction mFilter1, mFilter2;
        Integrator mInt1, mInt2;
        double num1[3],num2[3],den1[3],den2[3];

    public:
        static Component *Creator()
        {
            return new Crane();
        }

        void configure()
        {
            //Register constant parameters
            //Register input variables
            addInputVariable("L1", "", "", 0, &mpL1);
            addInputVariable("M1", "", "", 0, &mpM1);
            addInputVariable("J1", "", "", 0, &mpJ1);
            addInputVariable("B1", "", "", 0, &mpB1);
            addInputVariable("L2", "", "", 0, &mpL2);
            addInputVariable("M2", "", "", 0, &mpM2);
            addInputVariable("J2", "", "", 0, &mpJ2);
            addInputVariable("B2", "", "", 0, &mpB2);
            addInputVariable("d1", "", "", 0, &mpD1);
            addInputVariable("d2", "", "", 0, &mpD2);
            addInputVariable("d3", "", "", 0, &mpD3);
            addInputVariable("d4", "", "", 0, &mpD4);
            addInputVariable("x01", "", "", 0, &mpX01);
            addInputVariable("x02", "", "", 0, &mpX02);
            addInputVariable("Fload", "", "", 0, &mpFload);
            //Register output variables
            addOutputVariable("th1", "", "", 0, &mpTh1);
            addOutputVariable("w1", "", "", 0, &mpW1);
            addOutputVariable("th2", "", "", 0, &mpTh2);
            addOutputVariable("w2", "", "", 0, &mpW2);
            addOutputVariable("xc2", "", "", 0, &mpXc2);    //X-coordinate of beam 2, used for animation
            addOutputVariable("yc2", "", "", 0, &mpYc2);    //Y-coordinate of beam 2, used for animation

            //Add power ports
            mpP1 = addPowerPort("P1", "NodeMechanic", "");
            mpP2 = addPowerPort("P2", "NodeMechanic", "");
            //Set default power port start values
        }


        void initialize()
        {
            //Get node data pointers from ports
            mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_F = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP1_me = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_F = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP2_me = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            //Declare local variables
            double L1, M1, J1, B1, L2, M2, J2, B2, d1, d2, d3, d4, x01, x02, Fload;
            double th1, th2;
            double P1_x, P1_v, P1_F, P1_me, P1_c, P1_Zc, P2_x, P2_v, P2_F, P2_me, P2_c, P2_Zc;

            //Read variable values from nodes
            P1_c = (*mpP1_c);
            P1_Zc = (*mpP1_Zc);
            P2_c = (*mpP2_c);
            P2_Zc = (*mpP2_Zc);
            P1_x = -(*mpP1_x);
            P2_x = -(*mpP2_x);
            th1 = (*mpTh1);
            th2 = (*mpTh2);
            L1 = (*mpL1);
            M1 = (*mpM1);
            J1 = (*mpJ1);
            B1 = (*mpB1);
            L2 = (*mpL2);
            M2 = (*mpM2);
            J2 = (*mpJ2);
            B2 = (*mpB2);
            d1 = (*mpD1);
            d2 = (*mpD2);
            d3 = (*mpD3);
            d4 = (*mpD4);
            x01 = (*mpX01);
            x02 = (*mpX02);
            Fload = (*mpFload);

            //WRITE YOUR INITIALIZATION CODE HERE
            num1[0] = 0.0;
            num1[1] = 1.0;
            num1[2] = 0.0;
            den1[0] = 0;
            den1[1] = B1;
            den1[2] = J1;
            mFilter1.initialize(mTimestep, num1, den1, 0, 0);     
            mInt1.initialize(mTimestep, 0, th1);            

            num2[0] = 0.0;
            num2[1] = 1.0;
            num2[2] = 0.0;
            den2[0] = 0;
            den2[1] = B2;
            den2[2] = J2;
            mFilter2.initialize(mTimestep, num2, den2, 0, 0);     
            mInt2.initialize(mTimestep, 0, th2);           


            P1_me = M1;      //! @todo Not correct, fix later
            P2_me = M2;      //! @todo Not correct, fix later

            //Calculate x01,and x02 using law of cosines
            x01 = sqrt(d1*d1+d2*d2-2.0*d1*d2*cos(pi/2.0+th1)) - P1_x;
            x02 = sqrt(d3*d3+d4*d4-2.0*d3*d4*cos(pi/2.0+th2+pi/2.0-th1)) - P2_x;

            //Write new values to nodes
            (*mpP1_me) = P1_me;
            (*mpP2_me) = P2_me;
            (*mpX01) = x01;
            (*mpX02) = x02;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double L1, M1, J1, B1, L2, M2, J2, B2, d1, d2, d3, d4, x01, x02, Fload;
            double th1, w1, th2, w2;
            double P1_x, P1_v, P1_F, P1_me, P1_c, P1_Zc, P2_x, P2_v, P2_F, P2_me, P2_c, P2_Zc;

            //Read variable values from nodes
            P1_x = -(*mpP1_x);
            P1_c = (*mpP1_c);
            P1_Zc = (*mpP1_Zc);
            P2_x = -(*mpP2_x);
            P2_c = (*mpP2_c);
            P2_Zc = (*mpP2_Zc);
            L1 = (*mpL1);
            M1 = (*mpM1);
            J1 = (*mpJ1);
            B1 = (*mpB1);
            L2 = (*mpL2);
            M2 = (*mpM2);
            J2 = (*mpJ2);
            B2 = (*mpB2);
            d1 = (*mpD1);
            d2 = (*mpD2);
            d3 = (*mpD3);
            d4 = (*mpD4);
            x01 = (*mpX01);
            x02 = (*mpX02);
            th1 = (*mpTh1);
            th2 = (*mpTh2);

            //WRITE YOUR EQUATIONS HERE
            double g=9.81;

            //Calculate lever arms (first calculate area using Heron's formula)
            double s1 = (d1+d2+x01+P1_x)/2.0;
            double A1 = sqrt(s1*(s1-d1)*(s1-d1)*(s1-x01-P1_x));
            double h1 = 2.0*A1/(x01+P1_x);

            double s2 = (d3+d4+x02+P2_x)/2.0;
            double A2 = sqrt(s2*(s2-d3)*(s2-d4)*(s2-x02-P2_x));
            double h2 = 2.0*A2/(x02+P2_x);

            //Calculate th1 and th2 by using second order transfer functions
            den1[1] = B1+P1_Zc;
            mFilter1.setDen(den1);
            w1 = mFilter1.update(h1*P1_c - L1*cos(th1)/2.0*M1*g - (L1*cos(th1)+L2*cos(th2)/2.0)*M2*g - (L1*cos(th1)+L2*cos(th2))*Fload);
            th1 = mInt1.update(w1);


            if(th1 > pi/2.0)
            {
                th1=pi/2.0;
                w1=0;
                mFilter1.initialize(mTimestep, num1, den1, 0, 0);
                mInt1.initialize(mTimestep, 0, th1); 
            }
            else if(th1 < -pi/2.0)
            {
                th1=-pi/2.0;
                w1=0;
                mFilter1.initialize(mTimestep, num1, den1, 0, 0);
                mInt1.initialize(mTimestep, 0, th1); 
            }
            if(th2 > th1-0.1)
            {
                th2=th1-0.1;
                w2=0;
                mFilter2.initialize(mTimestep, num2, den2, 0, 0);
                mInt2.initialize(mTimestep, 0, th2); 
            }                
           // else if(th2 < -pi/2.0)
           // {
            //    th2=-pi/2.0;
            //    w2=0;
            //    mFilter2.initialize(mTimestep, num2, den2, 0, 0);
            //    mInt2.initialize(mTimestep, 0, th2); 
            //}        

            den2[1] = B2+P2_Zc;
            mFilter2.setDen(den2);
            w2 = mFilter2.update(h2*P2_c - L2*cos(th2)/2.0*M2*g - L2*cos(th2)*Fload);
            th2 = mInt2.update(w2);


            //Calculate x1,v1,x2 and v2 using law of cosines
            P1_x = sqrt(d1*d1+d2*d2-2.0*d1*d2*cos(pi/2.0+th1)) - x01;
            P2_x = sqrt(d3*d3+d4*d4-2.0*d3*d4*cos(pi/2.0+th2+pi/2.0-th1)) - x02;

            P1_v = 2.0*d1*d2*w1*sin(pi/2.0+th1) / ( 2.0*sqrt(d1*d1+d2*d2-2.0*d1*d2*cos(pi/2.0+th1)) ); 
            P2_v = 2.0*d3*d4*(w2-w1)*sin(pi/2.0+th2+pi/2.0-th1) / ( 2.0*sqrt(d3*d3+d4*d4-2.0*d3*d4*cos(pi/2.0+th2+pi/2.0-th1)) );

            P1_F = P1_c + P1_v*P1_Zc;
            P2_F = P2_c + P2_v*P2_Zc;

            //Calculate center of beam 2, used for animation
            (*mpXc2) = L1*cos(th1);//+L2*cos(th2)/2.0;
            (*mpYc2) = L1*sin(th1);//+L2*sin(th2)/2.0;
            
            P1_me = M1;      //! @todo Not correct, fix later
            P2_me = M2;      //! @todo Not correct, fix later


            //Write new values to nodes
            (*mpP1_x) = -P1_x;
            (*mpP1_v) = -P1_v;
            (*mpP1_F) = P1_F;
            (*mpP1_me) = P1_me;
            (*mpP2_x) = -P2_x;
            (*mpP2_v) = -P2_v;
            (*mpP2_F) = P2_F;
            (*mpP2_me) = P2_me;
            (*mpTh1) = th1;
            (*mpTh2) = th2;
            (*mpW1) = w1;
            (*mpW2) = w2;
        }


        void finalize()
        {
            //WRITE YOUR FINALIZE CODE HERE (OPTIONAL)
        }


        void deconfigure()
        {
            //WRITE YOUR DECONFIGURATION CODE HERE (OPTIONAL)
        }
    };
}

#endif //CRANE_HPP_INCLUDED


