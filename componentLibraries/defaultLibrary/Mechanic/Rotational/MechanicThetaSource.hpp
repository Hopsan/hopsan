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

#ifndef MECHANICTHETASOURCE_HPP_INCLUDED
#define MECHANICTHETASOURCE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <math.h>

//!
//! @file MechanicThetaSource.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Sat 30 Jul 2011 00:35:23
//! @brief Angular position source
//! @ingroup MechanicComponents
//!

using namespace hopsan;

class MechanicThetaSource : public ComponentQ
{
private:
     bool mThetaInConnected;

     // Port pointers
     Port *mpPmr1, *mpPthetain, *mpPwin;

     //Port Pmr1 node data pointers
     double *mpPmr1_tor;
     double *mpPmr1_theta;
     double *mpPmr1_w;
     double *mpPmr1_c;
     double *mpPmr1_Zc;
     double *mpPmr1_eqInertia;

     // inputVariables pointers
     double *mpIn_theta;
     double *mpIn_w;

     // The integrator
     Integrator mInt;

public:
     static Component *Creator()
     {
        return new MechanicThetaSource();
     }

     void configure()
     {
        // Add inputVariables ports to the component
        mpPthetain=addInputVariable("thetain","Angle", "rad", 0, &mpIn_theta);
        mpPwin=addInputVariable("omega","Angular Velocity", "rad/s", 0, &mpIn_w);

        // Add ports to the component
        mpPmr1=addPowerPort("Pmr1","NodeMechanicRotational");
     }

     void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pmr1
        mpPmr1_tor=getSafeNodeDataPtr(mpPmr1, NodeMechanicRotational::Torque);
        mpPmr1_theta=getSafeNodeDataPtr(mpPmr1, NodeMechanicRotational::Angle);
        mpPmr1_w=getSafeNodeDataPtr(mpPmr1, NodeMechanicRotational::AngularVelocity);
        mpPmr1_c=getSafeNodeDataPtr(mpPmr1, NodeMechanicRotational::WaveVariable);
        mpPmr1_Zc=getSafeNodeDataPtr(mpPmr1, NodeMechanicRotational::CharImpedance);
        mpPmr1_eqInertia=getSafeNodeDataPtr(mpPmr1, NodeMechanicRotational::EquivalentInertia);

        // Check connection
        mThetaInConnected = mpPthetain->isConnected();
        if(mThetaInConnected && !mpPwin->isConnected())
        {
            addWarningMessage("Angle input is connected but angular velocity is constant, kinematic relationsship must be manually enforced.");
        }
        else if(mThetaInConnected && mpPwin->isConnected())
        {
            addWarningMessage("Both angle and velocity inputs are connected, kinematic relationsship must be manually enforced.");
        }

        //Initialize Integrator
        mInt.initialize(mTimestep, (*mpIn_w), (*mpIn_theta));

        (*mpPmr1_eqInertia) = (*mpPmr1_eqInertia); //!< @todo should we not be able to set this, like in velocity source
        (*mpPmr1_tor)= (*mpPmr1_c) + (*mpPmr1_Zc)*(*mpIn_w);
        (*mpPmr1_theta)=(*mpIn_theta);
        (*mpPmr1_w)=(*mpIn_w);

     }

     void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Pmr1
        const double c = (*mpPmr1_c);
        const double Zc = (*mpPmr1_Zc);

        //Read inputVariables from nodes
        const double w_in = (*mpIn_w);

        //Expressions
        double theta_out;
        if(mThetaInConnected)
        {
            theta_out = (*mpIn_theta);
        }
        else
        {
            theta_out = mInt.update(w_in);
        }

        //Write new values to nodes
        //Port Pmr1
        (*mpPmr1_tor)=c + Zc*w_in;
        (*mpPmr1_theta)=theta_out;
        (*mpPmr1_w)=w_in;
     }
};
#endif // MECHANICTHETASOURCE_HPP_INCLUDED
