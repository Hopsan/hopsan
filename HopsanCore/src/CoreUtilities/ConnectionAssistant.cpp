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
//! @file   ConnectionAssistant.cpp
//! @author Peter Nordin
//! @date   2009-12-20
//!
//! @brief Contains the connection assistant help class
//!
//$Id$

#include "CoreUtilities/ConnectionAssistant.h"
#include "ComponentSystem.h"
#include "HopsanEssentials.h"

#include <vector>
#include <limits>

using namespace std;
using namespace hopsan;

bool ConnectionAssistant::ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2)
{
    ConnOKCounters counters;

    // Count the different kind of ports and C,Q components in the node
    vector<Port*>::iterator it;
    for (it=pNode->mConnectedPorts.begin(); it!=pNode->mConnectedPorts.end(); ++it)
    {
        checkPort(*it, counters);

        // Also count how many own systemports are already connected
        //! @todo maybe this counter should always be counted in checkPort()
        if ((*it)->getPortType() == SystemPortType)
        {
            if ((*it)->getComponent() == mpComponentSystem)
            {
                counters.nOwnSystemPorts += 1;
            }
        }
    }

    //Check the kind of ports in the components subjected for connection
    //Don't count port if it is already connected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort1) )
    {
        checkPort(pPort1, counters);
    }

    //Don't count port if it is already connected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort2) )
    {
        checkPort(pPort2, counters);
    }

    //  Check if there are some problems with the connection

    if ((counters.nPowerPorts > 0) && (counters.nOwnSystemPorts > 1))
    {
        mpComponentSystem->addErrorMessage("Trying to connect one powerport to two systemports, this is not allowed");
        return false;
    }
//    if(n_MultiPorts > 1)
//    {
//        addErrorMessage("Trying to connect two MultiPorts to each other");
//        return false;
//    }
    if (counters.nPowerPorts > 2+counters.nInterfacePorts-counters.nSystemPorts)
    {
        mpComponentSystem->addErrorMessage("Trying to connect more than two PowerPorts to same node");
        return false;
    }
    if (counters.nWritePorts > 1+counters.nInterfacePorts-counters.nSystemPorts)
    {
        mpComponentSystem->addErrorMessage("Trying to connect more than one WritePort to same node");
        return false;
    }
    if ((counters.nPowerPorts > 0) && (counters.nWritePorts > 0))
    {
        mpComponentSystem->addErrorMessage("Trying to connect WritePort and PowerPort to same node");
        return false;
    }
    //! @todo maybe this only readport check should give a warning, but only if we do Strict check mode (maybe send in a bool for that), but we want to allow it when loading in case connectors are saved in the wrong order
//    if ((n_PowerPorts == 0) && (n_WritePorts == 0) && (n_SystemPorts == 0))
//    {
//        cout << "Trying to connect only ReadPorts" << endl;
//        mpComponentSystem->addErrorMessage("Trying to connect only ReadPorts");
//        return false;
//    }

    //cout << "nQ: " << n_Qcomponents << " nC: " << n_Ccomponents << endl;

    // We want at most one C and one Q component in a connection
    //! @todo not 100% sure that this will work always. Only work if we assume that the subsystem has the correct cqs type when connecting
    if (counters.nNonInterfaceCPowerPorts > 1)
    {
        mpComponentSystem->addErrorMessage("You can not connect two C-Component power ports to each other");
        return false;
    }
    if (counters.nNonInterfaceQPowerPorts > 1)
    {
        mpComponentSystem->addErrorMessage("You can not connect two Q-Component power ports to each other");
        return false;
    }

//    if( ((pPort1->getPortType() == Port::READPORT) && pPort2->getPortType() == Port::POWERPORT && n_PowerPorts > 1) or
//        ((pPort2->getPortType() == Port::READPORT) && pPort1->getPortType() == Port::POWERPORT && n_PowerPorts > 1) )
//    {
//        addErrorMessage("Trying to connect one ReadPort to more than one PowerPort");
//        return false;
//    }

    // It seems to be OK!
    return true;
}

bool ConnectionAssistant::ensureNotCrossConnecting(Port *pPort1, Port *pPort2)
{
    // Check so that both components to connect have been added to the same system (or we are connecting to parent system)
    if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()->getSystemParent()) )
    {
        if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()) && (pPort2->getComponent()->getSystemParent() != pPort1->getComponent()) )
        {
            mpComponentSystem->addErrorMessage("The components, {"+pPort1->getComponentName()+"} and {"+pPort2->getComponentName()+"}, "+"must belong to the same subsystem");
            return false;
        }
    }
    return true;
}

//! @brief Detects if a port is a multiport and then adds and returns a subport
//! @param [in] pMaybeMultiport A pointer to the port that may be a multiport
//! @returns A pointer to a new subport in the multiport, or the pMaybeMultiport itself if it was not a multiport
Port *ConnectionAssistant::ifMultiportAddSubport(Port *pMaybeMultiport)
{
    // If the port is a multiport then create a new subport and then return it (as the actual port)
    if (pMaybeMultiport->getPortType() >= MultiportType)
    {
        return pMaybeMultiport->addSubPort();
    }

    // As the port was not a multiport lets return it
    return pMaybeMultiport;
}

void ConnectionAssistant::ifMultiportPrepareDissconnect(Port *pMaybeMultiport1, Port *pMaybeMultiport2, Port *&rpActualPort1, Port *&rpActualPort2)
{
    if ((pMaybeMultiport1->getPortType() >= MultiportType) && (pMaybeMultiport2->getPortType() >= MultiportType))
    {
        mpComponentSystem->addFatalMessage("ifMultiportFindActualPort():Both ports can not be multiports");
        rpActualPort1 = 0;
        rpActualPort2 = 0;
        return;
    }

    // if pMaybeMultiport1 is a multiport, but not other port
    if (pMaybeMultiport1->getPortType() >= MultiportType)
    {
        rpActualPort2 = pMaybeMultiport2;
        rpActualPort1 = findMultiportSubportFromOtherPort(pMaybeMultiport1, rpActualPort2);
        if(rpActualPort1 == 0)
        {
            mpComponentSystem->addFatalMessage("ifMultiportFindActualPort(): pActualPort1 == 0");
        }
    }


    // if pMaybeMultiport2 is a multiport, but not other port
    if (pMaybeMultiport2->getPortType() >= MultiportType)
    {
        rpActualPort1 = pMaybeMultiport1;
        rpActualPort2 = findMultiportSubportFromOtherPort(pMaybeMultiport2, rpActualPort1);
        if(rpActualPort2 == 0)
        {
            mpComponentSystem->addFatalMessage("ifMultiportFindActualPort(): pActualPort2 == 0");
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterConnect(Port *pMaybeMultiport, Port **ppActualPort, const bool wasSucess)
{
    if (pMaybeMultiport && (pMaybeMultiport == (*ppActualPort)->getParentPort()) )
    {
        if (wasSucess)
        {
            //! @todo What do we need to do to handle success
        }
        else
        {
            //We need to remove the last created subport
            pMaybeMultiport->removeSubPort(*ppActualPort);
            delete *ppActualPort;
            *ppActualPort = 0;
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterDissconnect(Port *pMaybeMultiport, Port **ppActualPort, const bool wasSucess)
{
    if (pMaybeMultiport && (pMaybeMultiport == (*ppActualPort)->getParentPort()) )
    {
        if (wasSucess)
        {
            //If successful we should remove the empty port
            pMaybeMultiport->removeSubPort(*ppActualPort);
            delete *ppActualPort;
            *ppActualPort = 0;
        }
        else
        {
            //! @todo What do we need to do to handle failure, nothing maybe
        }
    }
}

void ConnectionAssistant::checkPort(const Port *pPort, ConnectionAssistant::ConnOKCounters &rCounters)
{
    if (pPort->isInterfacePort())
    {
        rCounters.nInterfacePorts += 1;
    }

    if ( pPort->getPortType() == ReadPortType )
    {
        rCounters.nReadPorts += 1;
    }
    if ( pPort->getPortType() == WritePortType )
    {
        rCounters.nWritePorts += 1;
    }
    if ( pPort->getPortType() == PowerPortType )
    {
        rCounters.nPowerPorts += 1;
        if (pPort->getComponent()->isComponentC())
        {
            rCounters.nNonInterfaceCPowerPorts += 1;
        }
        else if (pPort->getComponent()->isComponentQ())
        {
            rCounters.nNonInterfaceQPowerPorts += 1;
        }
    }
    if ( pPort->getPortType() == SystemPortType )
    {
        rCounters.nSystemPorts += 1;
    }
//        if( pPort->getPortType() > MULTIPORT)
//        {
//            rCounters.n_MultiPorts += 1;
//        }
    if ( pPort->getComponent()->isComponentC() )
    {
        rCounters.nCComponents += 1;
        if ( pPort->getComponent()->isComponentSystem() )
        {
            rCounters.nSYScomponentCs += 1;
        }
    }
    else if ( pPort->getComponent()->isComponentQ() )
    {
        rCounters.nQComponents += 1;
        if ( pPort->getComponent()->isComponentSystem() )
        {
            rCounters.nSYScomponentQs += 1;
        }
    }
}

void ConnectionAssistant::removeNode(Node *pNode)
{
    if (pNode->getOwnerSystem())
    {
        pNode->getOwnerSystem()->removeSubNode(pNode);
    }
    mpComponentSystem->getHopsanEssentials()->removeNode(pNode);
}

ConnectionAssistant::ConnectionAssistant(ComponentSystem *pComponentSystem)
{
    mpComponentSystem = pComponentSystem;
}

//! @brief Helpfunction that clears the nodetype in empty systemports, It will not clear the type if the port is not empty or if the port is not a systemport
void ConnectionAssistant::clearSysPortNodeTypeIfEmpty(Port *pPort)
{
    if ( pPort && (pPort->getPortType() == SystemPortType) && (!pPort->isConnected()) )
    {
        Node *pOldNode = pPort->getNodePtr();
        pPort->setNode(mpComponentSystem->getHopsanEssentials()->createNode("NodeEmpty"));
        removeNode(pOldNode);
        pPort->mNodeType = "NodeEmpty";
    }
}

bool ConnectionAssistant::ensureSameNodeType(Port *pPort1, Port *pPort2)
{
    // Check if both ports have the same node type specified
    if (pPort1->getNodeType() != pPort2->getNodeType())
    {
        HString ss;
        ss+="You can not connect a {"+pPort1->getNodeType()+"} port to a {"+pPort2->getNodeType()+"} port."+
              " When connecting: {"+pPort1->getComponent()->getName()+"::"+pPort1->getName()+"} to {"+pPort2->getComponent()->getName()+"::"+pPort2->getName()+"}";
        mpComponentSystem->addErrorMessage(ss);
        return false;
    }
    return true;
}

//! @note requires that input ports are not multiports (they can be subports in multiports)
bool ConnectionAssistant::mergeNodeConnection(Port *pPort1, Port *pPort2)
{
    if (!ensureSameNodeType(pPort1, pPort2))
    {
        return false;
    }

    Node *pOldNode1 = pPort1->getNodePtr();
    Node *pOldNode2 = pPort2->getNodePtr();

    // Check for very rare occurrence, (Looping a subsystem, and connecting an out port to an in port that are actually directly connected to each other)
    if (pOldNode1 == pOldNode2)
    {
        mpComponentSystem->addErrorMessage("This connection would mean that a node is joined with it self, this does not make any sense and is not allowed");
        return false;
    }

    // Create a new node and recursively set in all ports
    Node *pNewNode = mpComponentSystem->getHopsanEssentials()->createNode(pPort1->getNodeType().c_str());
    recursivelySetNode(pPort1, 0, pNewNode);
    recursivelySetNode(pPort2, 0, pNewNode);

    // Let the ports know about each other
    pPort1->addConnectedPort(pPort2);
    pPort2->addConnectedPort(pPort1);

    // Now delete the old nodes
    removeNode(pOldNode1);
    removeNode(pOldNode2);

    // Update the node placement
    determineWhereToStoreNodeAndStoreIt(pNewNode);

    if (ensureConnectionOK(pNewNode, pPort1, pPort2))
    {
        return true;
    }
    else
    {
        splitNodeConnection(pPort1, pPort2); //Undo connection
        return false;
    }
}

//! @brief Find the system highest up in the model hierarchy for the ports connected to this node and store the node there
//! @param[in] pNode The node to store
void ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(Node* pNode)
{
    // Node ptr should not be zero
    if(pNode == 0)
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): Node pointer is zero.");
        return;
    }

    Component *pMinLevelComp=0;
    //size_t min = (size_t)-1;
    size_t min = std::numeric_limits<size_t>::max();
    vector<Port*>::iterator pit;
    for (pit=pNode->mConnectedPorts.begin(); pit!=pNode->mConnectedPorts.end(); ++pit)
    {
        if ((*pit)->getComponent()->getModelHierarchyDepth() < min)
        {
            min = (*pit)->getComponent()->getModelHierarchyDepth();
            pMinLevelComp = (*pit)->getComponent();
        }
    }

    // Now add the node to the system owning the minimum level component
    if (pMinLevelComp)
    {
        if (pMinLevelComp->getSystemParent())
        {
            pMinLevelComp->getSystemParent()->addSubNode(pNode);
        }
        else if (pMinLevelComp->isComponentSystem())
        {
            // This will trigger if we are connecting to our parent system which happens to be the top level system
            ComponentSystem *pRootSystem = dynamic_cast<ComponentSystem*>(pMinLevelComp);
            pRootSystem->addSubNode(pNode);
        }
        else
        {
            mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): No system found for node storage!");
        }
    }
    else
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): No system found!");
    }
}

void ConnectionAssistant::recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode)
{
    pPort->setNode(pNode);
    vector<Port*>::iterator pit;
    vector<Port*> conn_ports = pPort->getConnectedPorts();
    for (pit=conn_ports.begin(); pit!=conn_ports.end(); ++pit)
    {
        //don't recurse back to parent will get stuck in infinite recursion
        if (*pit == pParentPort)
        {
            continue;
        }
        recursivelySetNode(*pit, pPort, pNode);
    }
}

Port* ConnectionAssistant::findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort)
{
    if(pOtherPort->getPortType() >= MultiportType)
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::findMultiportSubportFromOtherPort(): Other port shall not be a multiport.");
        return 0;
    }

    std::vector<Port*> otherConnPorts = pOtherPort->getConnectedPorts();
    for (size_t i=0; i<otherConnPorts.size(); ++i)
    {
        // We assume that a port can not be connected multiple times to the same multiport
        if (otherConnPorts[i]->mpParentPort == pMultiPort)
        {
            return otherConnPorts[i];
        }
    }
    return 0;
}


//! @note Requires that the input ports are not multiports
bool ConnectionAssistant::splitNodeConnection(Port *pPort1, Port *pPort2)
{
    if ((pPort1==0) || (pPort2==0))
    {
        mpComponentSystem->addFatalMessage("splitNodeConnection(): One of the ports is NULL");
        return false;
    }

    Node *pOldNode = pPort1->getNodePtr();
    Node *pNewNode1 = mpComponentSystem->getHopsanEssentials()->createNode(pOldNode->getNodeType().c_str());
    Node *pNewNode2 = mpComponentSystem->getHopsanEssentials()->createNode(pOldNode->getNodeType().c_str());

    // Make the ports forget about each other, If the ports becomes empty the nodes will be reset
    pPort1->eraseConnectedPort(pPort2);
    pPort2->eraseConnectedPort(pPort1);

    // Recursively set new nodes
    recursivelySetNode(pPort1, 0, pNewNode1);
    recursivelySetNode(pPort2, 0, pNewNode2);

    // Remove the old node
    removeNode(pOldNode);

    // Now determine what system should own the node
    determineWhereToStoreNodeAndStoreIt(pNewNode1);
    determineWhereToStoreNodeAndStoreIt(pNewNode2);

    return true;
}
