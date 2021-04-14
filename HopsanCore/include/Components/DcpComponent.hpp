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
//! @file   DcpComponent.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2021-02-09
//!
//! @brief Contains a wrapper component for DCP servers
//!
//$Id$

#ifndef DCPCOMPONENT_HPP
#define DCPCOMPONENT_HPP

#include "ComponentEssentials.h"

namespace hopsan {

    class DcpComponent : public ComponentSignal
    {
    private:
        //Parameters
        HString mDcpFile;
        HString mVariables;
        HString mValueRefs;
        HString mLastVariables;

        //Internal variables
        std::vector<Port*> mPorts;
        std::vector<double*> mInputs;
        std::vector<double*> mOutputs;
        std::vector<double> mParameters;

    public:
        static Component *Creator()
        {
            return new DcpComponent();
        }

        void configure()
        {
            addConstant("dcpFile", "", "", "", mDcpFile);
            addConstant("valueRefs", "vr1,vr2,vr3...", "", "", mValueRefs);
            addConstant("variables", "in1,in2;out1,out2;par1,par2...", "", "", mVariables);
            setReconfigurationParameter("variables");
            // Do nothing
        }

        void reconfigure()
        {
            if(mVariables == mLastVariables) {
                return; //Path did not change, do nothing
            }
            mLastVariables = mVariables;


            for(size_t i=0; i<mPorts.size(); ++i) {
                removePort(mPorts[i]->getName());
            }
            std::vector<HString> parameters;
            this->getParameterNames(parameters);
            for(size_t i=0; i<parameters.size(); ++i) {
                if(parameters[i] != "hopsan::HString" && parameters[i] != "dcpFile" && parameters[i] != "variables" && parameters[i] != "valueRefs") {
                    this->unRegisterParameter(parameters[i]);
                }
            }
            mPorts.clear();
            mInputs.clear();
            mOutputs.clear();
            mParameters.clear();

            HVector<HString> splitVariables = mVariables.split(';');
            HVector<HString> splitInputs = splitVariables[0].split(',');
            HVector<HString> splitOutputs = splitVariables[1].split(',');
            HVector<HString> splitParameters = splitVariables[2].split(',');

            //The split function will return a single-element vector with an
            //empty string if delimiter was not found, clear vector in this case
            if(splitInputs.size() == 1 && splitInputs[0] == "") {
                splitInputs.clear();
            }
            if(splitOutputs.size() == 1 && splitOutputs[0] == "") {
                splitOutputs.clear();
            }
            if(splitParameters.size() == 1 && splitParameters[0] == "") {
                splitParameters.clear();
            }
            HVector<HString> splitValueRefs = mValueRefs.split(',');

            if(splitValueRefs.size() != splitInputs.size()+splitOutputs.size()+splitParameters.size()) {
                addErrorMessage("Number of value references does not equal number of variables");
                return;
            }

            for(size_t i=0; i<splitInputs.size(); ++i) {
                addDebugMessage("Input: "+HString(splitInputs[i]));
                mInputs.push_back(new double(0.0));
                mPorts.push_back(addInputVariable(splitInputs[i], splitValueRefs[i], "", (*mInputs.back()), &mInputs.back()));
            }

            for(size_t i=0; i<splitOutputs.size(); ++i) {
                addDebugMessage("Output: "+HString(splitOutputs[i]));
                mOutputs.push_back(new double(0.0));
                mPorts.push_back(addOutputVariable(splitOutputs[i], splitValueRefs[i+splitInputs.size()], "", (*mOutputs.back()), &mOutputs.back()));
            }

            for(size_t i=0; i<splitParameters.size(); ++i) {
                addDebugMessage("Parameter: "+HString(splitParameters[i]));
                mParameters.push_back(0.0);
                addConstant(splitParameters[i], splitValueRefs[i+splitInputs.size()+splitOutputs.size()], "", mParameters.back(), mParameters.back());
            }
        }

        void initialize()
        {
            // Do nothing
        }


        void simulateOneTimestep()
        {
            // Do nothing
        }
    };
}

#endif // DCPCOMPONENT_HPP
