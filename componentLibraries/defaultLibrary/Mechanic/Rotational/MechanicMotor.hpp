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
//! @file   MechanicMotor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-13
//!
//! @brief Contains a mechanic motor component
//!
//$Id$

#ifndef MECHANICMOTOR_HPP_INCLUDED
#define MECHANICMOTOR_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicMotor : public ComponentC
    {

    private:
        double *mpWref, *mpKp, *mpKi, *mpKd;
        Port *mpP1;

        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zc1;

        Integrator mIntegrator;
        FirstOrderTransferFunction mDerivator;

        double Wmax, Tmax, wmax;

    public:
        static Component *Creator()
        {
            return new MechanicMotor();
        }

        void configure()
        {
            addInputVariable("omega_ref", "Desired Angular Velocity", "-", 0.0, &mpWref);
            addInputVariable("K_p", "Proportional Controller Gain", "-", 10.0, &mpKp);
            addInputVariable("K_i", "Integrating Controller Gain", "-", 100.0, &mpKi);
            addInputVariable("K_d", "Derivating Controller Gain", "-", 100.0, &mpKd);
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
        }


        void initialize()
        {
            Wmax = 1000;
            Tmax = 100;
            wmax = 10;

            mpND_t1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_a1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

            mIntegrator.initialize(mTimestep, 0.0, 0.0);

            double num[2];
            double den[2];
            num[0]=0;
            num[1]=1;
            den[0]=1;
            den[1]=0;

            mDerivator.initialize(mTimestep, num, den, 0, 0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double wref, kp, ki, kd, w1, werror, x_p, x_i, x_d, t, c1, Zc1;
            wref = (*mpWref);
            kp = (*mpKp);
            ki = (*mpKi);
            kd = (*mpKd);
            w1 = -(*mpND_w1);

            //PI-Controller
            werror = wref-w1;
            x_p = werror*kp;
            x_i = ki*mIntegrator.update(werror);
            x_d = kd*mDerivator.update(werror);
            t = x_p+x_i+x_d;

            //Force source
            c1 = t;
            Zc1 = 0.0;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc1;
        }
    };
}

#endif // MECHANICMOTOR_HPP_INCLUDED

