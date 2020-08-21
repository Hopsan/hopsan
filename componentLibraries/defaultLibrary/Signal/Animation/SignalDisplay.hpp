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

//!
//! @file   SignalDisplay.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-11-15
//!
//! @brief Contains a signal display component
//!
//$Id$

#ifndef SIGNALDISPLAY_HPP_INCLUDED
#define SIGNALDISPLAY_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class SignalDisplay : public ComponentC
{

    private:
        HString mUnit, mDescription, mBackgroundColor, mTextColor;
        double mUnitScaling;
        int mPrecision;

    public:
        static Component *Creator()
        {
            return new SignalDisplay();
        }

        void configure()
        {
            //Add ports to the component
            addInputVariable("in","","",0.0);
            addConstant("description","Description label", "", mDescription);
            addConstant("unit","Unit","", mUnit);
            addConstant("unitscaling", "Unit scaling (from SI unit)", "", 1, mUnitScaling);
            addConstant("precision", "Maximum number of total digits", "", 8, mPrecision);
            addConstant("backgroundcolor", "Background color (red,green,blue)", "", "255,255,255", mBackgroundColor);
            addConstant("textcolor", "Text color (red,green,blue)", "", "0,50,80", mTextColor);
        }


        void initialize()
        {

        }

        void simulateOneTimestep()
        {

        }
    };
}

#endif
