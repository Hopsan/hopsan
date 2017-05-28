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
//! @file   ConnectionAssistant.h
//! @author Peter Nordin
//! @date   2009-12-20
//!
//! @brief Contains the connection assistant help class
//!
//$Id$

#ifndef CONNECTIONASSISTANT_H
#define CONNECTIONASSISTANT_H

#include <cstddef>

namespace hopsan {

// Froward declarations
class ComponentSystem;
class Port;
class Node;

class ConnectionAssistant
{
public:
    ConnectionAssistant(ComponentSystem *pComponentSystem);

    bool mergeNodeConnection(Port *pPort1, Port *pPort2);
    bool splitNodeConnection(Port *pPort1, Port *pPort2);

    void determineWhereToStoreNodeAndStoreIt(Node* pNode);
    void clearSysPortNodeTypeIfEmpty(Port *pPort);

    bool ensureNotCrossConnecting(Port *pPort1, Port *pPort2);
    bool ensureSameNodeType(Port *pPort1, Port *pPort2);
    bool ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2);

    Port* ifMultiportAddSubport(Port *pMaybeMultiport);
    void ifMultiportPrepareDissconnect(Port *pMaybeMultiport1, Port *pMaybeMultiport2, Port *&rpActualPort1, Port *&rpActualPort2);

    void ifMultiportCleanupAfterConnect(Port *pMaybeMultiport, Port **ppActualPort, const bool wasSucess);
    void ifMultiportCleanupAfterDissconnect(Port *pMaybeMultiport, Port **ppActualPort, const bool wasSucess);

private:
    class ConnOKCounters
    {
    public:
        size_t nReadPorts;
        size_t nWritePorts;
        size_t nPowerPorts;
        size_t nSystemPorts;
        size_t nOwnSystemPorts; // Number of systemports that belong to the connecting system
        size_t nInterfacePorts; // This can be system ports or other ports acting as interface ports in systems
        //size_t nMultiPorts;

        size_t nCComponents;
        size_t nQComponents;
        size_t nSYScomponentCs;
        size_t nSYScomponentQs;

        size_t nNonInterfaceQPowerPorts;
        size_t nNonInterfaceCPowerPorts;

        ConnOKCounters()
        {
            nReadPorts = 0;
            nWritePorts = 0;
            nPowerPorts = 0;
            nSystemPorts = 0;
            nOwnSystemPorts = 0;
            nInterfacePorts = 0;
            //nMultiPorts = 0;

            nCComponents = 0;
            nQComponents = 0;
            nSYScomponentCs = 0;
            nSYScomponentQs = 0;

            nNonInterfaceQPowerPorts = 0;
            nNonInterfaceCPowerPorts = 0;
        }
    };

    void checkPort(const Port *pPort, ConnOKCounters &rCounters);
    void removeNode(Node *pNode);
    void recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode);
    Port* findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort);
    ComponentSystem *mpComponentSystem; //The system to assist
};

}

#endif // CONNECTIONASSISTANT_H
