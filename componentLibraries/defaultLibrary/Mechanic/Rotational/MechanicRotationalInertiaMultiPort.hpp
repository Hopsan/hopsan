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

//$Id$

#ifndef MECHANICROTATIONALINERTIAMULTIPORT_HPP_INCLUDED
#define MECHANICROTATIONALINERTIAMULTIPORT_HPP_INCLUDED

#include <sstream>
#include <math.h>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInertiaMultiPort : public ComponentQ
    {

    private:
        double *mpJ, *B, *k, *mpAMin, *mpAMax;
        double t1, w1, c1, Zx1, t2, w2, c2, Zx2;                                                    //Node data variables
        double mNum[3];
        double mDen[3];
        DoubleIntegratorWithDamping mIntegrator;
        std::vector<double*> mvpN_t1, mvpN_a1, mvpN_w1, mvpN_me1, mvpN_c1, mvpN_Zx1, mvpN_t2, mvpN_a2, mvpN_w2, mvpN_me2, mvpN_c2, mvpN_Zx2;
        std::vector<double> a1, a2, mvpStartA1, mvpStartA2;
        size_t mNumPorts1, mNumPorts2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertiaMultiPort();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerMultiPort("P2", "NodeMechanicRotational");

            //Register changeable parameters to the HOPSAN++ core
            addInputVariable("J", "Inertia", "MomentOfInertia",                100.0, &mpJ);
            addInputVariable("B", "Viscous Friction", "Nms/rad",  10.0,  &B);
            //addInputVariable("k", "Spring Coefficient", "Nm/rad", 0.0,   &k);
            addInputVariable("a_min", "Minimum Angle of Port P2", "rad", -1.0e+300, &mpAMin);
            addInputVariable("a_max", "Maximum Angle of Port P2", "rad", 1.0e+300, &mpAMax);
            //! @todo what about k
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mNumPorts2 = mpP2->getNumPorts();

            mvpN_t1.resize(mNumPorts1);
            mvpN_a1.resize(mNumPorts1);
            mvpN_w1.resize(mNumPorts1);
            mvpN_me1.resize(mNumPorts1);
            mvpN_c1.resize(mNumPorts1);
            mvpN_Zx1.resize(mNumPorts1);

            mvpN_t2.resize(mNumPorts2);
            mvpN_a2.resize(mNumPorts2);
            mvpN_w2.resize(mNumPorts2);
            mvpN_me2.resize(mNumPorts2);
            mvpN_c2.resize(mNumPorts2);
            mvpN_Zx2.resize(mNumPorts2);

            a1.resize(mNumPorts1);
            a2.resize(mNumPorts2);
            mvpStartA1.resize(mNumPorts1);
            mvpStartA2.resize(mNumPorts2);

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                mvpN_t1[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanicRotational::Torque);
                mvpN_a1[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanicRotational::Angle);
                mvpN_w1[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanicRotational::AngularVelocity);
                mvpN_me1[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanicRotational::EquivalentInertia);
                mvpN_c1[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanicRotational::WaveVariable);
                mvpN_Zx1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanicRotational::CharImpedance);
            }

            for (size_t i=0; i<mNumPorts2; ++i)
            {
                mvpN_t2[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanicRotational::Torque);
                mvpN_a2[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanicRotational::Angle);
                mvpN_w2[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanicRotational::AngularVelocity);
                mvpN_me2[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanicRotational::EquivalentInertia);
                mvpN_c2[i]  = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanicRotational::WaveVariable);
                mvpN_Zx2[i] = getSafeMultiPortNodeDataPtr(mpP2, i, NodeMechanicRotational::CharImpedance);
            }



            //Initialization
            t1 = 0;
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                t1 += (*mvpN_t1[i]);
            }

            t2 = 0;
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                t2 += (*mvpN_t2[i]);
            }

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                a1[i] = (*mvpN_a1[i]);
                mvpStartA1[i]= (*mvpN_a1[i]);
            }

            for (size_t i=0; i<mNumPorts2; ++i)
            {
                a2[i] = (*mvpN_a2[i]);
                mvpStartA2[i] = (*mvpN_a2[i]);
            }

            w1 = (*mvpN_w1[0]);
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                if(w1 != (*mvpN_w1[i]))
                {
                    addErrorMessage("Velocities in multiport does not match, {"+getName()+"::"+mpP1->getName());
                    stopSimulation();
                }
            }

            w2 = (*mvpN_w2[0]);
            for(size_t i=0; i<mNumPorts2; ++i)
            {
                if(w2 != (*mvpN_w2[i]))
                {
                    addWarningMessage("Velocities in multiport does not match, {"+getName()+"::"+mpP2->getName());
                    stopSimulation();
                }
            }

            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);

            //Print debug message if velocities do not match
            if(w1 != -w2)
            {
                addWarningMessage("Start velocities does not match, {"+getName()+"::"+mpP1->getName()+
                                  "} and {"+getName()+"::"+mpP2->getName()+"}.");
                stopSimulation();
            }

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpN_me1[i]) = (*mpJ);
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpN_me2[i]) = (*mpJ);
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = 0;
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                c1 += (*mvpN_c1[i]);
            }
            Zx1 = 0;
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                Zx1 += (*mvpN_Zx1[i]);
            }
            c2 = 0;
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                c2 += (*mvpN_c2[i]);
            }
            Zx2 = 0;
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                Zx2 += (*mvpN_Zx2[i]);
            }

            const double J = (*mpJ);

            mIntegrator.setDamping(((*B)+Zx1+Zx2) / J * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/J);
            w2 = mIntegrator.valueFirst();
            double a_nom = mIntegrator.valueSecond();

            if(a_nom<(*mpAMin))
            {
                a_nom=(*mpAMin);
                w2=0.0;
                mIntegrator.initializeValues(0.0, a_nom, w2);
            }
            if(a_nom>(*mpAMax))
            {
                a_nom=(*mpAMax);
                w2=0.0;
                mIntegrator.initializeValues(0.0, a_nom, w2);
            }


            w1 = -w2;

            //Write new values to nodes

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpN_t1[i]) = (*mvpN_c1[i]) + (*mvpN_Zx1[i])*w1;
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpN_t2[i]) = (*mvpN_c2[i]) + (*mvpN_Zx2[i])*w2;
            }

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpN_a1[i]) = mvpStartA1[i]+mvpStartA2[0]-a_nom;
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpN_a2[i]) = mvpStartA2[i]-mvpStartA2[0]+a_nom;
            }

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpN_w1[i]) = w1;
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpN_w2[i]) = w2;
            }
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpN_me1[i]) = J;
            }
            for (size_t i=0; i<mNumPorts2; ++i)
            {
                (*mvpN_me2[i]) = J;
            }
        }
    };
}

#endif // MECHANICROTATIONALINERTIAMULTIPORT_HPP_INCLUDED

