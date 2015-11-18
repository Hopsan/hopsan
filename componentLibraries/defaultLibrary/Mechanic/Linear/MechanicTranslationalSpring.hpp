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

//$Id$

#ifndef MECHANICTRANSLATIONALSPRING_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRING_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalSpring : public ComponentC
    {

    private:
        double *mpK;
        double *mpP1_f, *mpP2_f, *mpP1_v, *mpP1_c, *mpP1_Zc, *mpP2_v, *mpP2_c, *mpP2_Zc;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalSpring();
        }

        void configure()
        {
            // Add power ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            // Add input variables
            addInputVariable("k", "Spring Coefficient", "N/m", 100.0,  &mpK);
        }


        void initialize()
        {
            mpP1_f =  getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpP2_f =  getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpP1_v = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);
            mpP2_v = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            const double Zc = (*mpK)*mTimestep;
            (*mpP1_c) = (*mpP2_f)+Zc*(*mpP2_v);
            (*mpP2_c) = (*mpP1_f)+Zc*(*mpP1_v);
            (*mpP1_Zc) = Zc;
            (*mpP2_Zc) = Zc;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            const double v1 = (*mpP1_v);
            const double lastc1 = (*mpP1_c);
            const double v2 = (*mpP2_v);
            const double lastc2 = (*mpP2_c);

            //Spring equations
            const double Zc = (*mpK)*mTimestep;
            const double c1 = lastc2 + 2.0*Zc*v2;
            const double c2 = lastc1 + 2.0*Zc*v1;

            //Write new values to nodes
            (*mpP1_c) = c1;
            (*mpP1_Zc) = Zc;
            (*mpP2_c) = c2;
            (*mpP2_Zc) = Zc;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


