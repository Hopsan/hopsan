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

//$Id$

#include "CoreUtilities/SaveRestoreSimulationPoint.h"
#include "ComponentSystem.h"
#include "Component.h"

#include <vector>
#include <fstream>
#include <cstring>

using namespace hopsan;

/*
 * Port head
 *
 * DataIdentifier   FullNameLength  NumDataElements (double)
 * 2-byte           2-byte          2-byte
 *
 *
 *
 * */

#define TIMEIDENTIFIER 0x02
#define PORTIDENTIFIER 0x03

void writePortData(Port *pPort, const HString &namePrefix, std::ofstream &rFile)
{
    // Generate full name
    HString fullName = namePrefix+pPort->getName();
    std::vector<double> *pDataVector = pPort->getDataVectorPtr();
    // OK great, if we have a data vector, lets dump it to file
    if (pDataVector)
    {
        // Write port identifier
        size_t identifier = PORTIDENTIFIER;
        rFile.write(reinterpret_cast<char*>(&identifier), 2);
        size_t namelen = fullName.size();
        rFile.write(reinterpret_cast<char*>(&namelen), 2);
        size_t datalen = pDataVector->size();
        rFile.write(reinterpret_cast<char*>(&datalen), 2);
        rFile.write(fullName.c_str(), namelen);
        rFile.write(reinterpret_cast<char*>(pDataVector->data()), datalen*sizeof(double));
    }
}

void writeTimeData(double time, std::ofstream &rFile)
{
    // Write port identifier
    size_t identifier = TIMEIDENTIFIER;
    rFile.write(reinterpret_cast<char*>(&identifier), 2);
    rFile.write(reinterpret_cast<char*>(&time), sizeof(double));
}

size_t readIdentifier(std::ifstream &rFile)
{
    size_t identifier=0;
    rFile.read(reinterpret_cast<char*>(&identifier), 2);
    return identifier;
}

void readPortData(std::ifstream &rFile, ComponentSystem *pRootSystem)
{
    // Read rest of header
    size_t namelength=0, datalength=0;
    rFile.read(reinterpret_cast<char*>(&namelength), 2);
    rFile.read(reinterpret_cast<char*>(&datalength), 2);

    // Read full name
    char *pNameBuffer = new char[namelength+1];
    rFile.read(pNameBuffer, namelength);
    pNameBuffer[namelength] = '\0';
    HString fullName;
    fullName.append(pNameBuffer);
    delete[] pNameBuffer;

    // Read data
    double *pDataBuffer = new double[datalength];
    rFile.read(reinterpret_cast<char*>(pDataBuffer), datalength*sizeof(double));

    // Parse fullName and find port to write into
    ComponentSystem *pSystem=pRootSystem;
    Component* pComponent=0;
    size_t b,e;
    b = fullName.find_first_of('/');
    while (true)
    {
        e = fullName.find_first_of('/', b+1);
        // We did not find a component, then treat as port
        if (e == HString::npos)
        {
            HString pname = fullName.substr(b+1,e);
            if (pComponent)
            {
                Port* pPort = pComponent->getPort(pname);
                if (pPort)
                {
                    std::vector<double> *pData = pPort->getDataVectorPtr();
                    for (size_t d=0; d<std::min(datalength, pData->size()); ++d)
                    {
                        pData->at(d) = pDataBuffer[d];
                    }
                }
            }
        }
        // We have found a component name
        else if (e < fullName.size()-1)
        {
            HString cname = fullName.substr(b+1,e-1);
            pComponent = pSystem->getSubComponent(cname);
            if (pComponent && pComponent->isComponentSystem())
            {
                pSystem = dynamic_cast<ComponentSystem*>(pComponent);
            }
            b=e;
        }

        // Exit loop when we are done
        if (e == HString::npos)
        {
            break;
        }
    }

    //! @todo need lots of error handling in code above
}

double readTimeData(std::ifstream &rFile)
{
    // Read time
    double time=0;
    rFile.read(reinterpret_cast<char*>(&time), sizeof(double));
    return time;
}


void saveSimulationPointInternal(ComponentSystem *pRootSystem, const HString &namePrefix, std::ofstream &rFile)
{
    //! @todo not sure if we should actually save/load the simulation time
    writeTimeData(pRootSystem->getTime(), rFile);
    std::vector<Component*> subcomps = pRootSystem->getSubComponents();
    for (size_t c=0; c<subcomps.size(); ++c)
    {
        Component *pComp = subcomps[c];

        std::vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t p=0; p<ports.size(); ++p)
        {
            writePortData(ports[p], namePrefix+pComp->getName()+'/', rFile);
        }

        if (pComp->isComponentSystem())
        {
            saveSimulationPointInternal(dynamic_cast<ComponentSystem*>(pComp), namePrefix+pComp->getName()+'/', rFile);
        }
    }
}


void hopsan::saveSimulationPoint(HString fileName, ComponentSystem *pRootSystem)
{
    std::ofstream file;
    file.open(fileName.c_str());
    if (file.is_open())
    {
        saveSimulationPointInternal(pRootSystem, "/", file);
    }
    file.close();
}

void hopsan::restoreSimulationPoint(HString fileName, ComponentSystem *pRootSystem, double &rTimeOffset)
{
    std::ifstream file;
    file.open(fileName.c_str());
    if (file.is_open())
    {
        while (!file.eof())
        {
            // Read identifier from next package
            size_t id = readIdentifier(file);
            switch (id)
            {
            case TIMEIDENTIFIER:
                rTimeOffset = readTimeData(file);
                break;
            case PORTIDENTIFIER:
                readPortData(file, pRootSystem);
                break;
            default:
                break;
            }
        }
    }
    file.close();
}
