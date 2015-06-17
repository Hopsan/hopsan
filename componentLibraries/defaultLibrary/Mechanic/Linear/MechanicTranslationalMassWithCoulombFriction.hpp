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

//!
//! @file   MechanicTranslationalMassWithCoulumbFriction.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-05
//!
//! @brief Contains a translational mass with coulomb friction and damper
//$Id$

#ifndef MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTION_HPP_INCLUDED
#define MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTION_HPP_INCLUDED

#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalMassWithCoulombFriction : public ComponentQ
    {

    private:
        double m;
        double *mpB, *mpFs, *mpFk, *xMin, *xMax;                                                                        //Changeable parameters
        double wx, u0, f, be, fe;                                                                              //Local Variables
        double mLength;                                                                                     //This length is not accessible by the user, it is set from the start values by the c-components in the ends
        double mNum[3];
        double mDen[3];
        DoubleIntegratorWithDampingAndCoulombFriction mIntegrator;

        // Ports and node data pointers
        Port *mpP1, *mpP2;
        double *mpP1_f, *mpP1_x, *mpP1_v, *mpP1_c, *mpP1_Zx, *mpP1_me;
        double *mpP2_f, *mpP2_x, *mpP2_v, *mpP2_c, *mpP2_Zx, *mpP2_me;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalMassWithCoulombFriction();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changeable parameters to the HOPSAN++ core
            addConstant("m", "Mass", "kg", 100.0, m);

            addInputVariable("b", "Viscous Friction Coefficient", "Ns/m", 10, &mpB);
            addInputVariable("f_s", "Static Friction Force", "N", 50,  &mpFs);
            addInputVariable("f_k", "Kinetic Friction Force", "N", 45,  &mpFk);
            addInputVariable("x_min", "Lower Limit of Position of Port P2", "m", -1.0e+300,  &xMin);
            addInputVariable("x_max", "Upper Limit of Position of Port P2", "m", 1.0e+300,  &xMax);
        }


        void initialize()
        {
            //Assign node data pointers
            mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpP1_me = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);

            mpP2_f = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);
            mpP2_me = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);

            double f1, x1, /*v1, c1,*/ f2, x2, v2/*, c2*/;

            f1 = (*mpP1_f);
            x1 = (*mpP1_x);
            //v1 = (*mpP1_v);
            //c1 = (*mpP1_c);

            f2 = (*mpP2_f);
            x2 = (*mpP2_x);
            v2 = (*mpP2_v);
            //c2 = (*mpP2_c);

            mLength = x1+x2;

            //Initialize integrator
            mIntegrator.initialize(mTimestep, 0, (*mpFs)/m, (*mpFk)/m, f1-f2, x2, v2);

            (*mpP1_me) = m;
            (*mpP2_me) = m;

            //Print debug message if start velocities doe not match
            if((*mpP1_v) != -(*mpP2_v))
            {
                this->addDebugMessage("Start velocities does not match, {"+getName()+"::"+mpP1->getName()+
                                      "} and {"+getName()+"::"+mpP2->getName()+"}.");
            }
        }


        void simulateOneTimestep()
        {
            double f1, x1, v1, c1, Zx1, f2, x2, v2, c2, Zx2;

            //Get variable values from nodes
            x1 = (*mpP1_x);
            c1 = (*mpP1_c);
            Zx1 = (*mpP1_Zx);
            x2 = (*mpP2_x);
            c2 = (*mpP2_c);
            Zx2 = (*mpP2_Zx);

            mIntegrator.setFriction((*mpFs)/m, (*mpFk)/m);

            mIntegrator.setDamping(((*mpB)+Zx1+Zx2) / m * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/m);
            v2 = mIntegrator.valueFirst();
            x2 = mIntegrator.valueSecond();

            if(x2<(*xMin))
            {
                x2=(*xMin);
                v2=std::max(0.0, v2);
                mIntegrator.initializeValues(0.0, x2, v2);
            }
            if(x2>(*xMax))
            {
                x2=(*xMax);
                v2=std::min(0.0, v2);
                mIntegrator.initializeValues(0.0, x2, v2);
            }

            v1 = -v2;
            x1 = -x2 + mLength;
            f1 = c1 + Zx1*v1;
            f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*mpP1_f) = f1;
            (*mpP1_x) = x1;
            (*mpP1_v) = v1;
            (*mpP2_f) = f2;
            (*mpP2_x) = x2;
            (*mpP2_v) = v2;
        }
    };
}

#endif // MECHANICTRANSLATIONALMASSWITHCOULOMBFRICTION_HPP_INCLUDED

