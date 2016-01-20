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

#define PORTIDENTIFIER 0x02

void saveSimulationPointInternal(hopsan::ComponentSystem *pRootSystem, const HString namePrefix, std::ofstream &file)
{

    std::vector<Component*> subcomps = pRootSystem->getSubComponents();
    for (size_t c=0; c<subcomps.size(); ++c)
    {
        Component *pComp = subcomps[c];

        std::vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t p=0; p<ports.size(); ++p)
        {
            Port *pPort = ports[p];
            // Generate full name
            HString fullName = namePrefix;
            fullName.append(pComp->getName()+'/'+pPort->getName());

            std::vector<double> *pDataVector = pPort->getDataVectorPtr();
            // OK great, if we have a data vector, lets dump it to file
            if (pDataVector)
            {
                // Write port identifier
                size_t identifier = PORTIDENTIFIER;
                file.write(reinterpret_cast<char*>(&identifier), 2);
                size_t namelen = fullName.size();
                file.write(reinterpret_cast<char*>(&namelen), 2);
                size_t datalen = pDataVector->size();
                file.write(reinterpret_cast<char*>(&datalen), 2);
                file.write(fullName.c_str(), namelen);
                file.write(reinterpret_cast<char*>(pDataVector->data()), datalen*sizeof(double));
            }
        }

        if (pComp->isComponentSystem())
        {
            saveSimulationPointInternal(dynamic_cast<ComponentSystem*>(pComp), namePrefix+pComp->getName()+'/', file);
        }
    }

}


void hopsan::saveSimulationPoint(hopsan::HString fileName, hopsan::ComponentSystem *pRootSystem)
{
    std::ofstream file;
    file.open(fileName.c_str());
    if (file.is_open())
    {
        saveSimulationPointInternal(pRootSystem, "/", file);
    }
    file.close();
}

void hopsan::restoreSimulationPoint(hopsan::HString fileName, hopsan::ComponentSystem *pRootSystem)
{
    char headerbuffer[6];

    std::ifstream file;
    file.open(fileName.c_str());
    if (file.is_open())
    {
        while (!file.eof())
        {
            // Read header
            file.read(headerbuffer, 6);
            //! @todo handle identifiers
            size_t identifier=0, namelength=0, datalength=0;
            memcpy(static_cast<void*>(&identifier), static_cast<void*>(&headerbuffer), 2);
            memcpy(static_cast<void*>(&namelength), static_cast<void*>(&headerbuffer[2]), 2);
            memcpy(static_cast<void*>(&datalength), static_cast<void*>(&headerbuffer[4]), 2);

            // Read full name
            char *pNameBuffer = new char[namelength+1];
            file.read(pNameBuffer, namelength);
            pNameBuffer[namelength] = '\0';
            HString fullName;
            fullName.append(pNameBuffer);
            delete pNameBuffer;

            // Read data
            double *pDataBuffer = new double[datalength];
            file.read(reinterpret_cast<char*>(pDataBuffer), datalength*sizeof(double));

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
    }
    file.close();
}
