/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef FORKS_HPP_INCLUDED
#define FORKS_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <cmath>

namespace hopsan {

    class Forks : public ComponentQ
    {
    private:
        //Constants
        double mR, mRrot, mM, mJ, mSl;
        
        
        //Output variable node data pointers
        double *mpX, *mpA;
        double *mpDebug;
        
        //Input Variable node data pointers
        double *mpB, *mpK, *mpXMin, *mpXMax;
        double *mpKfrot, *mpBrot, *mpKrot, *mpAMin, *mpAMax;
        
        
        //Power port pointers
        Port *mpP1, *mpP2, *mpP3;
        
        //Power port node data pointers
        double *mpP1_a, *mpP1_w, *mpP1_T, *mpP1_Je, *mpP1_c, *mpP1_Zc;
        double *mpP2_x, *mpP2_v, *mpP2_f, *mpP2_me, *mpP2_c, *mpP2_Zc;
        double *mpP3_x, *mpP3_v, *mpP3_f, *mpP3_me, *mpP3_c, *mpP3_Zc;

        // Other member variables
        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];
        
        // Other member variables
        SecondOrderTransferFunction mFilterA;
        FirstOrderTransferFunction mFilterW;
        double mNumA[3], mNumW[2];
        double mDenA[3], mDenW[2];
        
    public:
        static Component *Creator()
        {
            return new Forks();
        }

        void configure()
        {
            
            addConstant("r", "Radius (traverse)", "", 0.1, mR);
            addConstant("m", "Mass (traverse)", "kg",                      100.0, mM);
            addConstant("sl", "Piston strokes", "m", 1, mSl);
            addInputVariable("B", "Viscous Friction (traverse)", "Ns/m",   10.0, &mpB);
            addInputVariable("k", "Spring Coefficient (traverse)", "N/m", 0.0, &mpK);
            addInputVariable("x_min", "Minimum Position (traverse)", "m", -0.6, &mpXMin);
            addInputVariable("x_max", "Maximum Position (traverse)", "m", 0.6, &mpXMax);
            addOutputVariable("x", "Position (traverse)", "m", -0.6, &mpX);
            
            addConstant("r2", "Radius (rotation)", "", 0.1, mRrot);
            addConstant("J", "Inertia (rotation)", "kgm^2",       0.2, mJ);
            addInputVariable("Kfrot", "Dry Friction (rotation)", "Nm",   10.0, &mpKfrot);
            addInputVariable("Brot", "Viscous Friction (rotation)", "Nms/rad",   10.0, &mpBrot);
            addInputVariable("krot", "Spring Coefficient (rotation)", "N/m", 0.0, &mpKrot);
            addInputVariable("a_min", "Minimum Angle (rotation)", "rad", -1.5707963267949, &mpAMin);
            addInputVariable("a_max", "Maximum Angle (rotation)", "rad", 1.5707963267949, &mpAMax);
            addOutputVariable("a", "Angle (rotation)", "rad", 1.5707963267949, &mpA);
            addOutputVariable("debug", "Debug", "", 0, &mpDebug);
            
            //Add power ports
            mpP1 = addPowerPort("P1", "NodeMechanicRotational", "");
            mpP2 = addPowerPort("P2", "NodeMechanic", "");
            mpP3 = addPowerPort("P3", "NodeMechanic", "");
        }


        void initialize()
        {
            //Get node data pointers from ports
            mpP1_a = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpP1_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpP1_T = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpP1_Je = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::EquivalentInertia);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);
            
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_f = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP2_me = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            mpP3_x = getSafeNodeDataPtr(mpP3, NodeMechanic::Position);
            mpP3_v = getSafeNodeDataPtr(mpP3, NodeMechanic::Velocity);
            mpP3_f = getSafeNodeDataPtr(mpP3, NodeMechanic::Force);
            mpP3_me = getSafeNodeDataPtr(mpP3, NodeMechanic::EquivalentMass);
            mpP3_c = getSafeNodeDataPtr(mpP3, NodeMechanic::WaveVariable);
            mpP3_Zc = getSafeNodeDataPtr(mpP3, NodeMechanic::CharImpedance);

            //Read variable values from nodes
            const double t1 = (*mpP1_a);
            const double r = mR;
            
            const double f2 = (*mpP2_f);
            const double f3 = (*mpP3_f);
            const double r2 = mRrot;

            const double x = (*mpX);
            const double a = (*mpA);
                        
            //WRITE YOUR INITIALIZATION CODE HERE

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = (*mpK);
            mDenX[1] = (*mpB);
            mDenX[2] = mM;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = (*mpB);
            mDenV[1] = mM;

            mFilterX.initialize(mTimestep, mNumX, mDenX, t1/r, x, -1.5e300, 1.5e300, x);
            mFilterV.initialize(mTimestep, mNumV, mDenV, t1/r, 0);
            
            mNumA[0] = 1.0;
            mNumA[1] = 0.0;
            mNumA[2] = 0.0;
            mDenA[0] = (*mpKrot);
            mDenA[1] = (*mpBrot);
            mDenA[2] = mJ;
            mNumW[0] = 1.0;
            mNumW[1] = 0.0;
            mDenW[0] = (*mpBrot);
            mDenW[1] = mJ;

            mFilterA.initialize(mTimestep, mNumA, mDenA, (f2-f3)*r2, a, -1.5e300, 1.5e300, a);
            mFilterW.initialize(mTimestep, mNumW, mDenW, (f2-f3)*r2 - (*mpKrot)*a, 0);
            
            const double x2 = a*r2-mSl/2;
            const double x3 = a*r2-mSl/2;
                        
            (*mpP2_x) = x2;
            (*mpP3_x) = x3;
        }


        void simulateOneTimestep()
        {
            double t1, a1, w1, x, v;
            double f2, x2, v2, f3, x3=0, v3, a, w;

            //Get variable values from nodes
            const double r = mR;
            const double c1 = (*mpP1_c);
            const double Zx1 = (*mpP1_Zc);
            const double k = (*mpK);
            const double B = (*mpB);
            
            const double r2 = mRrot;
            const double c2 = (*mpP2_c);
            const double Zx2 = (*mpP2_Zc);
            const double c3 = (*mpP3_c);
            const double Zx3 = (*mpP3_Zc);
            const double krot = (*mpKrot);
            const double Brot = (*mpBrot);
            const double Kfrot = (*mpKfrot);

            a1 = (*mpP1_a);
            w1 = (*mpP1_a);

            //Traverse equations
            mDenX[0] = k;
            mDenX[1] = B+Zx1;
            mDenV[0] = B+Zx1;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            x = mFilterX.update(c1/r);
            v = mFilterV.update(c1/r - k*a1*r);

            if(x<(*mpXMin))
            {
                x=(*mpXMin);
                v=0.0;
                mFilterX.initializeValues(c1/r, x);
                mFilterV.initializeValues(c1/r, 0.0);
            }
            if(x>(*mpXMax))
            {
                x=(*mpXMax);
                v=0.0;
                mFilterX.initializeValues(c1/r, x);
                mFilterV.initializeValues(c1/r, 0.0);
            }

            a1 = -x/r;
            w1 = -v/r;
            t1 = c1 + Zx1*w1;
            
            //Rotation equations
            double F_dry = std::max(Kfrot*tanh(10000*(*mpP2_v)/r2),(c2-c3)*r2);
            (*mpDebug) = F_dry;

            mDenA[0] = krot;
            mDenA[1] = Brot+Zx2+Zx3;
            mDenW[0] = Brot+Zx2+Zx3;
            mFilterA.setDen(mDenA);
            mFilterW.setDen(mDenW);

            a = mFilterA.update((c2-c3)*r2 + F_dry);
            w = mFilterW.update((c2-c3)*r2 + F_dry - krot*x3/r2);

            if(a<(*mpAMin))
            {
                a=(*mpAMin);
                w=0.0;
                mFilterA.initializeValues((c2-c3)*r2 + F_dry, a);
                mFilterW.initializeValues((c2-c3)*r2 + F_dry, 0.0);
            }
            if(a>(*mpAMax))
            {
                a=(*mpAMax);
                w=0.0;
                mFilterA.initializeValues((c2-c3)*r2 + F_dry, a);
                mFilterW.initializeValues((c2-c3)*r2 + F_dry, 0.0);
            }

            x2 = -a*r2-mSl/2;
            v2 = -w*r2;
            f2 = c2 + Zx2*v2;

            x3 = a*r2-mSl/2;
            v3 = w*r2;
            f3 = c3 + Zx3*v3;
            
            //Write new values to nodes
            (*mpP1_T) = t1;
            (*mpP1_a) = a1;
            (*mpP1_w) = w1;
            
            (*mpX) = x;
            
            (*mpP2_f) = f2;
            (*mpP2_x) = x2;
            (*mpP2_v) = v2;
            
            (*mpP3_f) = f3;
            (*mpP3_x) = x3;
            (*mpP3_v) = v3;
            
            (*mpA) = a;
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

#endif //FORKS_HPP_INCLUDED


