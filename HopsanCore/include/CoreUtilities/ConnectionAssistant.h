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
