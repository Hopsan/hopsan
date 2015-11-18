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
//! @file   MechanicRotationalInertia.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-13
//!
//! @brief Contains a mechanic translational lossless connector (i.e. a moving body without inertia)
//!
//$Id$

#ifndef MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED
#define MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED

#include <sstream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalLosslessConnector : public ComponentQ
    {

    private:
        double mLength;         //This length is not accessible by the user,
                                //it is set from the start values by the c-components in the ends
        double *mpP1_f, *mpP1_x, *mpP1_v, *mpP1_c, *mpP1_Zx, *mpP2_f, *mpP2_x, *mpP2_v, *mpP2_c, *mpP2_Zx;  //Node data pointers
        Integrator mInt;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalLosslessConnector();
        }

        void configure()
        {
            //Set member attributes

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");
        }


        void initialize()
        {
            //Assign node data pointers
            mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            mpP2_f = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            //Initialization
            double x1 = (*mpP1_x);
            double v1 = (*mpP1_v);
            double x2 = (*mpP2_x);

            mLength = x1+x2;

            mInt.initialize(mTimestep, -v1, -x1+mLength);

            //Print debug message if velocities do not match
            if(mpP1->readNode(NodeMechanic::Velocity) != -mpP2->readNode(NodeMechanic::Velocity))
            {
                addDebugMessage("Start velocities does not match, {"+getName()+"::"+mpP1->getName()+
                                "} and {"+getName()+"::"+mpP2->getName()+"}.");
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double c1 = (*mpP1_c);
            double Zx1 = (*mpP1_Zx);
            double c2 = (*mpP2_c);
            double Zx2 = (*mpP2_Zx);

            //Connector equations
            double v2 = (c1-c2)/(Zx1+Zx2);
            double v1 = -v2;
            double x2 = mInt.update(v2);
            double x1 = -x2 + mLength;
            double f1 = c1 + Zx1*v1;
            double f2 = c2 + Zx2*v2;

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

#endif // MECHANICTRANSLATIONALLOSSLESSCONNECTOR_HPP_INCLUDED

