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
//! @file   Nodes.cpp
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_nodes function that registers all built in nodes
//!
//$Id$
#include "Nodes.h"
#include "Port.h"
#include "ComponentUtilities/num2string.hpp"

//! @defgroup Nodes Nodes

//! @defgroup NodeHydraulic NodeHydraulic
//! @ingroup Nodes

//! @defgroup NodeHydraulicTemperature NodeHydraulicTemperature
//! @ingroup Nodes

//! @defgroup NodePneumatic NodePneumatic
//! @ingroup Nodes

//! @defgroup NodeMechanic NodeMechanic
//! @ingroup Nodes

//! @defgroup NodeMechanicRotational NodeMechanicRotational
//! @ingroup Nodes

//! @defgroup NodeSignal NodeSignal
//! @ingroup Nodes

//! @defgroup NodeElectric NodeElectric
//! @ingroup Nodes

//! @defgroup NodeMechanic2D NodeMechanic2D
//! @ingroup Nodes

//! @defgroup NodePetriNet NodePetriNet
//! @ingroup Nodes

//! @defgroup NodeEmpty NodeEmpty
//! @ingroup Nodes

using namespace hopsan;

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in] pNodeFactory A pointer the the node factory in which to register the nodes
//!
void hopsan::register_default_nodes(NodeFactory* pNodeFactory)
{
    pNodeFactory->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeSignalND", NodeSignalND::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeSignal2D", NodeSignal2D::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeSignal3D", NodeSignal3D::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeHydraulicTemperature", NodeHydraulicTemperature::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodePneumatic", NodePneumatic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeElectric", NodeElectric::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic2D", NodeMechanic2D::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeModelica", NodeModelica::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodePetriNet", NodePetriNet::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeEmpty", NodeEmpty::CreatorFunction);
}

void NodeSignalND::setSignalNumDimensions(size_t numDims)
{
    // Resize
    mDataDescriptions.resize(numDims);
    mDataValues.resize(numDims,0.0);

    // Set name
    HString nicename = "signal"+to_hstring(numDims)+"d";
    setNiceName(nicename);

    // Re-register data variables
    for (size_t i=0; i<numDims; ++i)
    {
        HString longname = "v"+to_hstring(i);
        HString shortname = "y"+to_hstring(i);
        setDataCharacteristics(i, longname, shortname, "");
    }
}
