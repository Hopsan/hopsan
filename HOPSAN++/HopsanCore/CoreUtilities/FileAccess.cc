//!
//! @file   FileAccess.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-03
//!
//! @brief Contains the file access functions
//!
//$Id$

#include <iostream>
#include <cassert>
#include "FileAccess.h"
#include "../Component.h"
#include "../HopsanCore.h"

using namespace std;
using namespace hopsan;

////! @todo clean up this readname and quote stuff

////! @brief This function extracts the name from a text stream
////! @return The extracted name without quotes, empty string if failed
////! It is assumed that the name was saved OK. but error indicated by empty string
//string hopsan::readName(stringstream &rTextStream)
//{
//    string tempName;
//    rTextStream >> tempName;

//    if (tempName.compare(0,1,"\"") == 0)
//    {
//        //cout << "begin quote" << endl;
//        int last = tempName.size()-1; //! @todo find better way to keep track of last character for comparison (not really that important)
//        while (!tempName.compare(last,1,"\"") == 0)
//        {
//            //cout << "not last quote" << endl;
//            //cout << "tempname: " << tempName << endl;
//            if (rTextStream.eof())
//            {
//                return string(""); //Empty string (failed)
//            }
//            else
//            {
//                string tmpstr;
//                rTextStream >> tmpstr;
//                tempName.append(" " + tmpstr);
//            }
//            last = tempName.size()-1;
//        }
//        //cout << "last quote" << endl;
//        //return tempName.remove("\"").trimmed(); //Remove quotes and trimm (just to be sure)
//        //Trim the ends
//        //! @todo make sure to trim white spaces also
//        tempName.erase(tempName.begin());
//        tempName.erase(--tempName.end());
//        return tempName;
//    }
//    else
//    {
//        return string(""); //Empty string (failed)
//    }
//}

////! @brief Convenience function if you dont have a stream to read from
////! @return The extracted name without quotes, empty string if failed
////! It is assumed that the name was saved OK. but error indicated by empty string
//string hopsan::readName(string namestring)
//{
//    stringstream namestream(namestring);
//    return readName(namestream);
//}

////! @brief This function may be used to add quotes around string, usefull for saving names. Ex: "string"
//string hopsan::addQuotes(string str)
//{
//    str.insert(0,"\""); //prepend
//    str.append("\"");
//    return str;
//}


FileAccess::FileAccess()
{
    //Nothing
}

//! @todo Update this code
void FileAccess::loadModel(string filename, ComponentSystem* pModelSystem, double *startTime, double *stopTime)
{
        //Read from file
    cout << "Trying to open model: " << filename.c_str() << endl;
    ifstream modelFile (filename.c_str());
    if(!modelFile.is_open())
    {
        cout << "Model file does not exist!" << endl;
        assert(false);
        //! @todo Cast an exception
    }

    modelFile.close();
}


//! @brief This function can be used to load subsystem contents from a stream into an existing subsystem
//! @todo Update this code
void FileAccess::loadSystemContents(stringstream &rLoaddatastream, ComponentSystem* pSubsystem)
{

}

//! @todo Update this code
void FileAccess::saveModel(string fileName, ComponentSystem* pMotherOfAllModels, double startTime, double stopTime)
{

}

//! @todo Update this code
void FileAccess::saveComponentSystem(ofstream& modelFile, ComponentSystem* pMotherModel, string motherSystemName)
{

}
