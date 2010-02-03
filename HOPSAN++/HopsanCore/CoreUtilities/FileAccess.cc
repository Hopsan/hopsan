//!
//! @file   FileAccess.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-03
//!
//! @brief Contains the file access functions
//!
//$Id:$

#include "FileAccess.h"
#include "Component.h"
#include <iostream>
#include "HopsanCore.h"


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
    HopsanEssentials Hopsan;
    ComponentSystem mainModel("mainModel");

        //Read data from file
    typedef map<string, Component*> mapType;                            //File stuff, should maybe be cleaned up
	mapType componentMap;
    string inputLine;
    string inputWord;

    ifstream modelFile (mFilename.c_str());
    while (! modelFile.eof() )
    {
            //Read the line
        getline(modelFile,inputLine);                                   //Read a line
        stringstream inputStream(inputLine);

            //Extract first word unless stream is empty
        if ( inputStream >> inputWord )
        {
                //Execute commands
            if ( inputWord == "COMPONENT" )                         //Create a component
            {
                inputStream >> inputWord;
                Component *tempComponent = Hopsan.CreateComponent(inputWord);
                inputStream >> inputWord;
                tempComponent->setName(inputWord);
                componentMap.insert(pair<string, Component*>(inputWord, &*tempComponent));
                mainModel.addComponent(componentMap.find(inputWord)->second);
            }
            else if ( inputWord == "CONNECT" )                       //Connect components
            {
                string firstComponent, firstPort, secondComponent, secondPort;
                inputStream >> firstComponent;
                inputStream >> firstPort;
                inputStream >> secondComponent;
                inputStream >> secondPort;
                mainModel.connect(*componentMap.find(firstComponent)->second, firstPort, *componentMap.find(secondComponent)->second, secondPort);
            }
            else if ( inputWord == "SET" )
            {
                string componentName, parameterName;
                double parameterValue;
                inputStream >> componentName;
                inputStream >> parameterName;
                inputStream >> parameterValue;
                componentMap.find(componentName)->second->setParameter(parameterName, parameterValue);
            }
            else if ( inputWord == "SIMULATE" )
            {
                cout << "Debug!" << endl;
                double temp;
                inputStream >> temp;
                cout << "Debug 2, temp = " << temp << endl;
                *startTime = temp;
                cout << "Debug 3!" << endl;
                inputStream >> temp;
                *stopTime = temp;
                cout << "Reading simulation parameters.\n";
            }
            else if ( inputWord == "PLOT" )
            {
                inputStream >> *plotComponent;
                inputStream >> *plotPort;
                cout << "Reading plotting parameters.\n";
            }
            else
            {
                cout << "Unidentified command in model file ignored.\n";
            }
        }
        else
        {
            cout << "Ignoring empty line.\n";
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

void FileAccess::saveModel(ComponentSystem mainModel)
{
    //Do something here!
    return;
}
