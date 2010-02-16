//!
//! @file   FileAccess.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-03
//!
//! @brief Contains the file access functions
//!
//$Id$

#include "FileAccess.h"
#include "Component.h"
#include <iostream>
#include <cassert>
#include "../HopsanCore.h"


FileAccess::FileAccess()
{
    //Nothing
}

FileAccess::FileAccess(string filename)
{
    mFilename = filename;
}

void FileAccess::setFilename(string filename)
{
    mFilename = filename;
}

ComponentSystem FileAccess::loadModel(double *startTime, double *stopTime, string *plotComponent, string *plotPort)
{
        //Read from file
    ifstream modelFile (mFilename.c_str());
    if(!modelFile.is_open())
    {
        cout <<"Model file does not exist!\n";
        assert(false);
        //TODO: Cast an exception
    }

        //Necessary declarations
    HopsanEssentials Hopsan;
    ComponentSystem mainModel("mainModel");
    typedef map<string, Component*> mapComponentType;
    typedef map<string, ComponentSystem*> mapSystemType;
	mapComponentType componentMap;
	mapSystemType componentSystemMap;
    string inputLine;
    string inputWord;

    while (! modelFile.eof() )
    {
            //Read the line
        getline(modelFile,inputLine);                                   //Read a line
        stringstream inputStream(inputLine);

            //Extract first word unless stream is empty
        if ( inputStream >> inputWord )
        {
            cout << inputWord << endl;

            //----------- Create New SubSystem -----------//

            if ( inputWord == "SUBSYSTEM" )
            {
                ComponentSystem *tempComponentSystem = new ComponentSystem();
                inputStream >> inputWord;
                tempComponentSystem->setName(inputWord);
                componentMap.insert(pair<string, Component*>(inputWord, &*tempComponentSystem));
                componentSystemMap.insert(pair<string, ComponentSystem*>(inputWord, &*tempComponentSystem));
                inputStream >> inputWord;
                tempComponentSystem->setTypeCQS(inputWord);
                if ( inputStream >> inputWord )
                {
                    componentSystemMap.find(inputWord)->second->addComponent(tempComponentSystem);      //Subsystem belongs to other subsystem
                }
                else
                {
                    mainModel.addComponent(tempComponentSystem);                 //Subsystem belongs to main system
                }
            }

            //----------- Add Port To SubSystem -----------//

            else if ( inputWord == "SYSTEMPORT")
            {
                string subSystemName, portName;
                //Component* getComponent(string name);
                inputStream >> subSystemName;
                inputStream >> portName;
                componentSystemMap.find(subSystemName)->second->addSystemPort(portName);
            }

            //----------- Create New Component -----------//

            else if ( inputWord == "COMPONENT" )
            {
                inputStream >> inputWord;
                Component *tempComponent = Hopsan.CreateComponent(inputWord);
                inputStream >> inputWord;
                tempComponent->setName(inputWord);
                componentMap.insert(pair<string, Component*>(inputWord, &*tempComponent));
                if ( inputStream >> inputWord )
                {
                    componentSystemMap.find(inputWord)->second->addComponent(tempComponent);      //Componenet belongs to subsystem
                }
                else
                {
                    mainModel.addComponent(tempComponent);                 //Component belongs to main system
                }
            }

            //----------- Connect Components -----------//

            else if ( inputWord == "CONNECT" )
            {
                string firstComponent, firstPort, secondComponent, secondPort;
                inputStream >> firstComponent;
                inputStream >> firstPort;
                inputStream >> secondComponent;
                inputStream >> secondPort;

                if ( componentMap.count(firstComponent) > 0 && componentMap.count(secondComponent) > 0 )        //Connecting two components
                {
                    componentMap.find(firstComponent)->second->getSystemparent().connect(*componentMap.find(firstComponent)->second, firstPort, *componentMap.find(secondComponent)->second, secondPort);
                }
                else if ( componentMap.count(firstComponent) > 0 && componentSystemMap.count(secondComponent) > 0 )     //Connecting component with subsystem
                {
                    componentMap.find(firstComponent)->second->getSystemparent().connect(*componentMap.find(firstComponent)->second, firstPort, *componentSystemMap.find(secondComponent)->second, secondPort);
                }
                else if ( componentSystemMap.count(firstComponent) > 0 && componentMap.count(secondComponent) > 0 )     //Connecting subsystem with component
                {
                   componentMap.find(secondComponent)->second->getSystemparent().connect(*componentSystemMap.find(firstComponent)->second, firstPort, *componentMap.find(secondComponent)->second, secondPort);
                }
                else  //Connecting subsystem with subsystem
                {
                     componentMap.find(firstComponent)->second->getSystemparent().connect(*componentSystemMap.find(firstComponent)->second, firstPort, *componentSystemMap.find(secondComponent)->second, secondPort);
                }
            }
                //Execute commands
            //----------- Set Parameter Value -----------//

            else if ( inputWord == "SET" )
            {
                string componentName, parameterName;
                double parameterValue;
                inputStream >> componentName;
                inputStream >> parameterName;
                inputStream >> parameterValue;
                componentMap.find(componentName)->second->setParameter(parameterName, parameterValue);
            }

            //----------- Set Simulation Parameters -----------//

            else if ( inputWord == "SIMULATE" )
            {
                double temp;
                inputStream >> temp;
                *startTime = temp;
                inputStream >> temp;
                *stopTime = temp;
            }

            //----------- Select Plotting Parameters -----------//

            else if ( inputWord == "PLOT" )
            {
                inputStream >> *plotComponent;
                inputStream >> *plotPort;
            }

            //----------- Unrecognized Command -----------//

            else
            {
                cout << "Unidentified command in model file ignored.\n";
            }
        }
        else
        {
            //cout << "Ignoring empty line.\n";
        }

    }
    modelFile.close();

    return mainModel;
}

ComponentSystem FileAccess::loadModel(string filename, double *startTime, double *stopTime, string *plotComponent, string *plotPort)
{
    setFilename(filename);
    return loadModel(&*startTime, &*stopTime, &*plotComponent, &*plotPort);
}

void FileAccess::saveModel(string fileName, ComponentSystem motherOfAllModels, double startTime, double stopTime, string plotComponent, string plotPort)
{
    ofstream modelFile(fileName.c_str());
    saveComponentSystem(modelFile, motherOfAllModels, "");
    modelFile << "SIMULATE " << startTime << " " << stopTime << "\n";
    modelFile << "PLOT " << plotComponent << " " << plotPort << "\n";
    modelFile.close();
    return;
}


void FileAccess::saveComponentSystem(ofstream& modelFile, ComponentSystem& motherModel, string motherSystemName)
{
    map<string, string> mainComponentList = motherModel.getComponentNames();
    map<string, string>::iterator it;
    map<Port*, string> portList;

    for(it = mainComponentList.begin(); it!=mainComponentList.end(); ++it)
    {
        //if (it->second == "ComponentSystem")
        if(motherModel.getComponent(it->first)->isComponentSystem())
        {
            modelFile << "SUBSYSTEM " << " " << it->first << " " << motherModel.getComponentSystem(it->first)->getTypeCQS() << "\n";
            vector<Port*> systemPorts = motherModel.getComponentSystem(it->first)->getPortPtrVector();
            cout << "Subsystem has " << systemPorts.size() << " ports.\n";
            vector<Port*>::iterator itp;
            for (itp=systemPorts.begin(); itp!=systemPorts.end(); ++itp)
            {
                modelFile << "SYSTEMPORT " << it->first << " " << (*itp)->getPortName() << "\n";
            }

            ///TODO: Skriv ut subsystemets portar
            ///TODO: Fixa så man kan komma åt subsystem ur ett component system, så rekursiva anrop kan göras här
            saveComponentSystem(modelFile, *motherModel.getComponentSystem(it->first), motherSystemName + " " + it->first);
        }
        else
        {
                //Write the create component line in the file
            modelFile << "COMPONENT " << it->second << " " << it->first << motherSystemName << "\n";
        }

        map<string,double> componentParameterList = motherModel.getComponent(it->first)->getParameterList();
        map<string, double>::iterator itc;
        for(itc = componentParameterList.begin(); itc!=componentParameterList.end(); ++itc)
        {
            modelFile << "SET " << it->first << " " << itc->first << " " << itc->second << "\n";
        }


            //Store all ports in a map, together with the name of the component they belong to (for use below)
        vector <Port*> portPtrsVector = motherModel.getComponent(it->first)->getPortPtrVector();
        vector <Port*>::iterator itp;
        for (itp=portPtrsVector.begin(); itp!=portPtrsVector.end(); ++itp)
        {
            portList.insert(pair<Port*,string>(*itp, it->first));
        }
        portPtrsVector = motherModel.getPortPtrVector();
        for (itp=portPtrsVector.begin(); itp!=portPtrsVector.end(); ++itp)
        {
            portList.insert(pair<Port*,string>(*itp, motherModel.getName()));
        }
    }

    cout << "Connecting in system " << motherModel.getName() << ", portList.size() = " << portList.size() << endl;

        //Iterate through port map and figure out which ports share the same node, and then write the connect lines
    map<Port*, string>::iterator itp;
    for(itp = portList.begin(); itp != portList.end();)
    {
        map<Port*, string>::iterator itp2;
        for(itp2 = portList.begin(); itp2 != portList.end(); ++itp2)
        {
            Node *ptr1 = itp->first->getNodePublic();
            Node *ptr2 = itp2->first->getNodePublic();
            if (ptr1 == ptr2 && itp != itp2)
            {
                modelFile << "CONNECT " << itp->second << " " << itp->first->getPortName() << " " << itp2->second << " " << itp2->first->getPortName() << "\n";
            }
        }
        portList.erase(itp++);          //Increase itp by 1, then remove previous value from map to prevent double connection
    }
}
