//!
//! @file   Component.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Component base classes as well as Component Parameter class
//!
//$Id$

//! @defgroup Components Components

#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <limits>
#include <stdlib.h>
#include "stdio.h"
#include "Component.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/FileAccess.h"
#include "Port.h"
#include "HopsanEssentials.h"

#ifdef USETBB
#include "tbb.h"
#include "tick_count.h"
#include "blocked_range.h"
#include "parallel_for.h"
#endif

using namespace std;
using namespace hopsan;

//! @brief Helper function to create a unique name among names from one Map
//! @todo try to merge these to help functions into one (the next one bellow which is very similar)
template<typename MapType>
string findUniqueName(MapType &rMap, string name)
{
    //New name must not be empty, empty name is "reserved" to be used in the API to indicate that we want to manipulate the current root system
    if (name.empty())
    {
        name = "Untitled";
    }

    size_t ctr = 1; //The suffix number
    while(rMap.count(name) != 0)
    {
        //strip suffix
        size_t foundpos = name.rfind("_");
        if (foundpos != string::npos)
        {
            if (foundpos+1 < name.size())
            {
                unsigned char nr = name.at(foundpos+1);
                //cout << "nr after _: " << nr << endl;
                //Check the ascii code for the charachter
                if ((nr >= 48) && (nr <= 57))
                {
                    //Is number lets assume that the _ found is the beginning of a suffix
                    name.erase(foundpos, string::npos);
                }
            }
        }
        //cout << "ctr: " << ctr << " stripped tempname: " << name << endl;

        //add new suffix
        stringstream suffix;
        suffix << ctr;
        name.append("_");
        name.append(suffix.str());
        ++ctr;
        //cout << "ctr: " << ctr << " appended tempname: " << name << endl;
    }
    //cout << name << endl;

    return name;
}

////! @brief Helper function to create a unique name among names from TWO Map
//template<typename MapType1, typename MapType2>
//string findUniqueName(MapType1 &rMap1, MapType2 &rMap2 , string name)
//{
//    //New name must not be empty, empty name is "reserved" to be used in the API to indicate that we want to manipulate the current root system
//    if (name.empty())
//    {
//        name = "Untitled";
//    }

//    size_t ctr = 1; //The suffix number
//    while( (rMap1.count(name)+rMap2.count(name)) != 0)
//    {
//        //strip suffix
//        size_t foundpos = name.rfind("_");
//        if (foundpos != string::npos)
//        {
//            if (foundpos+1 < name.size())
//            {
//                unsigned char nr = name.at(foundpos+1);
//                //cout << "nr after _: " << nr << endl;
//                //Check the ascii code for the charachter
//                if ((nr >= 48) && (nr <= 57))
//                {
//                    //Is number lets assume that the _ found is the beginning of a suffix
//                    name.erase(foundpos, string::npos);
//                }
//            }
//        }
//        //cout << "ctr: " << ctr << " stripped tempname: " << name << endl;

//        //add new suffix
//        stringstream suffix;
//        suffix << ctr;
//        name.append("_");
//        name.append(suffix.str());
//        ++ctr;
//        //cout << "ctr: " << ctr << " appended tempname: " << name << endl;
//    }
//    //cout << name << endl;
//    return name;
//}

//! @brief Helper function to create a unique name among names from TWO Map
template<typename MapType1, typename MapType2, typename ReservedNamesType>
string findUniqueName(MapType1 &rMap1, MapType2 &rMap2, ReservedNamesType &rReservedMap, string name)
{
    //New name must not be empty, empty name is "reserved" to be used in the API to indicate that we want to manipulate the current root system
    if (name.empty())
    {
        name = "Untitled";
    }

    size_t ctr = 1; //The suffix number
    while( (rMap1.count(name)+rMap2.count(name)+rReservedMap.count(name)) != 0)
    {
        //strip suffix
        size_t foundpos = name.rfind("_");
        if (foundpos != string::npos)
        {
            if (foundpos+1 < name.size())
            {
                unsigned char nr = name.at(foundpos+1);
                //cout << "nr after _: " << nr << endl;
                //Check the ascii code for the charachter
                if ((nr >= 48) && (nr <= 57))
                {
                    //Is number lets assume that the _ found is the beginning of a suffix
                    name.erase(foundpos, string::npos);
                }
            }
        }
        //cout << "ctr: " << ctr << " stripped tempname: " << name << endl;

        //add new suffix
        stringstream suffix;
        suffix << ctr;
        name.append("_");
        name.append(suffix.str());
        ++ctr;
        //cout << "ctr: " << ctr << " appended tempname: " << name << endl;
    }
    //cout << name << endl;
    return name;
}


//Constructor
CompParameter::CompParameter(const string name, const string description, const string unit, double &rValue)
{
    mName = name;
    mDescription = description;
    mUnit = unit;
    mpValue = &rValue;
};


string CompParameter::getName()
{
    return mName;
}

string CompParameter::getDesc()
{
    return mDescription;
}


string CompParameter::getUnit()
{
    return mUnit;
}


double CompParameter::getValue()
{
    return *mpValue;
}


double *CompParameter::getValuePtr()
{
    return mpValue;
}


void CompParameter::setValue(const double value)
{
    *mpValue = value;
}


//! @brief Adds a new System parameter to the system or change the vaölue of an existing one
//!
//! A parameter is added but has no ponters to values.
//! Or if there exist a parameter with this name the value is changed and the ponters are unchanged.
//! The parameter is then mapped to one or more doubles with the mapParameter method.
//! @param[in] sysParName is the name of the new System parameter.
//! @param[in] value is the value of the System parameter.
//! @see mapParameter(std::string sysParName, double *mappedValue)
//! @return true if a it went OK, false otherwise
bool SystemParameters::add(std::string sysParName, double value)
{
    //This is a QD for handling negative values...
    if(sysParName.at(0) == '-')
        return false;
    //This is a QD for handling negative values...

    if(mSystemParameters.count(sysParName) > 0)
    //sysParName is already present, change its value
    {
        mSystemParameters[sysParName].first = value;
        update(sysParName);

        //This is a QD for handling negative values...
        std::ostringstream oss;
        oss << "-" << sysParName;
        std::string negSysParName = oss.str();
        mSystemParameters[negSysParName].first = -value;
        update(negSysParName);
        //This is a QD for handling negative values...
    }
    else
    //sysParName is not present, create a new one
    {
        SystemParameter sysPar;
        sysPar.first = value;
        mSystemParameters[sysParName] = sysPar;

        //This is a QD for handling negative values...
        std::ostringstream oss;
        oss << "-" << sysParName;
        std::string negSysParName = oss.str();
        SystemParameter negSysPar;
        negSysPar.first = -value;
        mSystemParameters[negSysParName] = negSysPar;
        //This is a QD for handling negative values...
    }
    return true;
}

//! @brief Read the value of System parameter
//!
//! @param[in] sysParName is the name of the new System parameter.
//! @param[out] value is the value of the System parameter.
//! @return true if a post with sysParName exsited, false otherwise
bool SystemParameters::getValue(std::string sysParName, double &value)
{
//std::map<std::string, SystemParameter>::iterator it;
//for(it = mSystemParameters.begin(); it != mSystemParameters.end(); ++it)
//{
//    std::cout << it->first << mSystemParameters.count(sysParName) << std::endl;
//}
    if(mSystemParameters.count(sysParName))
    {
        value = mSystemParameters[sysParName].first;
        return true;
    }
    else
        return false;
}

//! @brief Get a map with System parameter names as keys and their values as values
//!
//! @return a map with keys: System parameter names, values: System parameter values
std::map<std::string, double> SystemParameters::getSystemParameterMap()
{
    std::map<std::string, double> sysPar;
    std::map<std::string, SystemParameter>::iterator map_it;
    for(map_it = mSystemParameters.begin(); map_it != mSystemParameters.end(); ++map_it)
    {
        //Create a new map with only the name and value (no pointers)
        //This is a QD for handling negative values...
        //Do not count if it start with '-' because then there is this special hack!
        if(map_it->first.at(0) != '-')
        //This is a QD for handling negative values...
        {
            sysPar[map_it->first] = map_it->second.first;
        }
    }
    return sysPar;
}

//! @brief Finds out if a double has a reference in the System parameters
//!
//! @param[in] mappedValue is a pointer to a double.
//! @return the name of the System parameter which is mapped to the input, an empty std::string if its not.
std::string SystemParameters::findOccurrence(double *mappedValue)
{
    std::string sysParName ="";
    std::list<double*>::iterator list_it;
    std::map<std::string, SystemParameter>::iterator map_it;
    for(map_it = mSystemParameters.begin(); map_it != mSystemParameters.end(); ++map_it)
    {
        //Go through all pointers to see if mappedValue are present somewhere
        for(list_it = map_it->second.second.begin(); list_it != map_it->second.second.end(); ++list_it)
        {
            if(*list_it == mappedValue)
            {
                sysParName = map_it->first;
            }
        }
    }
    return sysParName;
}

//! @brief Delete a System parameter
//!
//! @param[in] sysParName the System parameter to be deleted.
void SystemParameters::erase(std::string sysParName)
{
    mSystemParameters.erase(sysParName);

    //This is a QD for handling negative values...
    std::ostringstream oss;
    oss << "-" << sysParName;
    std::string negSysParName = oss.str();
    mSystemParameters.erase(negSysParName);
    //This is a QD for handling negative values...
}

//! @brief Maps a double to a System parameter
//!
//! After this method has been ran the SystemParameter object have a pointer
//! stored to the double and can write the System parameter value to it directly
//! without the knoledge of the "double owner".
//! If the sysParName does not exist in the SystemParameters nothing will happen.
//!
//! @param[in] sysParName is the name of the System parameter which should point to the double.
//! @param[in] mappedValue is a pointer to a double.
//! @return true if a it went OK, false otherwise
//! @see unMapParameter(std::string sysParName, double *mappedValue)
bool SystemParameters::mapParameter(std::string sysParName, double *mappedValue)
{
    bool success = false;
    //If mappedValue is in the map somwhere else it is removed first
    unMapParameter(mappedValue);

    std::map<std::string, SystemParameter>::iterator it;
    it = mSystemParameters.find(sysParName);
    //If the sysParName exists in the mappedValue is added, if not nothing happens
    if(it != mSystemParameters.end())
    {
        it->second.second.push_back(mappedValue);
        //the System parameter value is written to the mappedValue
        *mappedValue = it->second.first;
        success = true;
    }
    return success;
}

//! @brief Unmaps a double from a System parameter
//!
//! This method removes the pointer to the mappedValue from the SystemParameters.
//! After this is ran the double is free from the SystemParameters
//!
//! @param[in] sysParName is the name of the System parameter which should point to the double.
//! @param[in] mappedValue is a pointer to a double.
//! @see MapParameter(std::string sysParName, double *mappedValue)
void SystemParameters::unMapParameter(std::string sysParName, double *mappedValue)
{
    std::list<double*>::iterator list_it, remove_it;
    bool found = false;
    //Go through all mapped values for the System parameter sysParName
    for(list_it = mSystemParameters[sysParName].second.begin(); list_it !=mSystemParameters[sysParName].second.end(); ++list_it)
    {
        //If it is found it is saved to be removed
        if(*list_it == mappedValue)
        {
            remove_it = list_it;
            found = true;
        }
    }
    if((mSystemParameters.count(sysParName)) && found)
    {
        //remove the occurance of the mappedValue
        mSystemParameters[sysParName].second.erase(remove_it);
    }
}

//! @brief Unmaps a double from a System parameter
//!
//! This method removes the pointer to the mappedValue from the SystemParameters.
//! After this is ran the double is free from the SystemParameters
//!
//! @param[in] mappedValue is a pointer to a double.
//! @see MapParameter(std::string sysParName, double *mappedValue)
void SystemParameters::unMapParameter(double *mappedValue)
{
    std::map<std::string, SystemParameter>::iterator map_it;
    //Go through all mapped values for all the System parameters
    for(map_it = mSystemParameters.begin(); map_it != mSystemParameters.end(); ++map_it)
    {
        //remove the pointer for the mappedValue in System parameters
        unMapParameter(map_it->first, mappedValue);
    }
}

//! @brief Write the all System parameters values to the doubles that they points to.
void SystemParameters::update()
{
    std::map<std::string, SystemParameter>::iterator map_it;
    for(map_it = mSystemParameters.begin(); map_it != mSystemParameters.end(); ++map_it)
    {
        //Write the System parameter value to all pointer addresses
        update(map_it->first);
    }
}

//! @brief Write the System parameter value to the doubles that sysParName points to.
//!
//! @param[in] sysParName the System parameter to update
//! @return true if a it went OK, false otherwise
bool SystemParameters::update(std::string sysParName)
{
    bool success = false;
    std::list<double*>::iterator list_it;
    if(mSystemParameters.count(sysParName) > 0)
    {
        for(list_it = mSystemParameters[sysParName].second.begin(); list_it != mSystemParameters[sysParName].second.end(); ++list_it)
        {
            //Write the System parameter value to all pointer addresses for sysParName
            *(*list_it) = mSystemParameters[sysParName].first;
        }
        success = true;
    }
    return success;
}


//Constructor
Component::Component(string name)
{
    mName = name;
    mTimestep = 0.001;

    mIsComponentC = false;
    mIsComponentQ = false;
    mIsComponentSystem = false;
    mIsComponentSignal = false;
    //mTypeCQS = "";
    mTypeCQS = Component::NOCQSTYPE;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    //registerParameter("Ts", "Sample time", "[s]",   mTimestep);
}


//! Virtual Function, base version which gives you an error if you try to use it.
void Component::initialize(const double /*startT*/, const double /*stopT*/, const size_t /*nSamples*/)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use initialize() instead" << endl;
    assert(false);
}


//! Virtual Function, base version which gives you an error if you try to use it.
void Component::finalize(const double /*startT*/, const double /*stopT*/)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use finalize() instead" << endl;
    assert(false);
}


//! @todo adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
void Component::simulate(const double startT, const double stopT)
{
    //double dT = stopT-startT;
    double stopTsafe = stopT - mTimestep/2.0;
    mTime = startT;
    while (mTime < stopTsafe)
    {
        simulateOneTimestep();
        mTime += mTimestep;
    }
    //cout << "simulate in: " << this->getName() << endl;
}


void Component::initialize()
{
    cout << "Warning! You should implement your own method" << endl;
    assert(false);
}


void Component::simulateOneTimestep()
{
    cout << "Warning! You should implement your own method: " << mName << endl;
    assert(false);
}


void Component::finalize()
{
    //cout << "Warning! You should implement your own finalize() method" << endl;
    //assert(false);
}

//! @brief A finilize method that contains stuff that the user should not need to care about
//! @todo OK I admit, the name is kind of bad
void Component::secretFinalize()
{
    //delete any created dummy node data variables created and then clear the pointer storage vector
    for (size_t i=0; i<mDummyNDptrs.size(); ++i)
    {
        delete mDummyNDptrs[i];
    }
    mDummyNDptrs.clear();
}


//! @brief Set the desired component name
//! @param [in] name The desired component name
//! @param [in] doOnlyLocalRename Only use this if you know what you are doing, default: false
//!
//! Set the desired component name, if name is already taken in a subsystem the desired name will be modified with a suffix.
//! If you set doOnlyLocalRename to true, the smart rename will not be atempted, avoid doing this as the component storage map will not be updated on anme change
//! This is a somewhat ugly fix for some special situations where we want to make sure that a smart rename is not atempted
void Component::setName(string name, bool doOnlyLocalRename)
{
    //! @todo fix the trailing _ removal
    //First strip any lonely trailing _ from the name (and give a warning)
//    string::iterator lastchar = --(name.end());
//    while (*lastchar == "_")
//    {
//        cout << "Warning underscore is not alowed in the end of a name (to avoid ugly collsion with name suffix)" << endl;
//        name.erase(lastchar);
//        lastchar = --(name.end());
//    }

    //If name same as before do nothing
    if (name != mName)
    {
        //Do we have a system parent
        if (mpSystemParent != 0)
        {
            //Need this to prevent risk of loop between rename and this function (rename cals this one again)
            if (doOnlyLocalRename)
            {
                mName = name;
            }
            else
            {
                //Do smart rename (to prevent same names)
                mpSystemParent->renameSubComponent(mName, name);
            }
        }
        else
        {
            //Ok no systemparent is set yet so lets set our own name
            mName = name;
        }
    }
}


//! Get the component name
const string &Component::getName()
{
    return mName;
}


//! Get the C, Q or S type of the component as enum
Component::typeCQS Component::getTypeCQS()
{
    return mTypeCQS;
}


//! Convert the C, Q or S type from enum to string
//! @todo This function may not need to be meber in Component, (maybe enum should be free aswell), this function may be completely useless
string Component::convertTypeCQS2String(typeCQS type)
{
    switch (type)
    {
    case C :
        return "C";
        break;
    case Q :
        return "Q";
        break;
    case S :
        return "S";
        break;
    case NOCQSTYPE :
        return "NOCQSTYPE";
        break;
    default :
        return "Invalid CQS Type";
    }
}


//! Get the CQStype as string
string Component::getTypeCQSString()
{
    switch (mTypeCQS)
    {
    case C :
        return "C";
        break;
    case Q :
        return "Q";
        break;
    case S :
        return "S";
        break;
    case NOCQSTYPE :
        return "NOCQSTYPE";
        break;
    default :
        return "Invalid CQS Type";
    }
}


//! Get the type name of the component
const string &Component::getTypeName()
{
    return mTypeName;
}


//! @brief Terminate/stop a running simulation
//!
//! Typically used inside components simulateOneTimestep method
void Component::stopSimulation()
{
    this->getSystemParent()->stop();
}


//! Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
void Component::registerParameter(const string name, const string description, const string unit, double &rValue)
{
    //! @todo handle trying to add multiple comppar with same name

    std::stringstream ss;
    ss << getName() << "::registerParameter";
    addLogMess(ss.str());

    CompParameter new_comppar(name, description, unit, rValue);
    mParameters.push_back(new_comppar); //Copy parameters into storage
    mDefaultParameters.insert(std::pair<std::string, double>(description+name, rValue));
}


void Component::listParametersConsole()
{
    cout <<"-----------------------------------------------" << endl << getName() << ":" << endl;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        cout << "Parameter " << i << ": " << mParameters[i].getName() << " = " << mParameters[i].getValue() << " " << mParameters[i].getUnit() << " " << mParameters[i].getDesc() << endl;
    }
    cout <<"-----------------------------------------------" << endl;
}


//! @brief Get the value of a parameter
//! @param[in] name is the name of the wanted parameter
//! @returns the value of the parameter
double Component::getParameterValue(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getValue();
        }
    }
    cout << "No such parameter (return 0): " << name << endl;
    //! @todo We should create a debug warning to user if this happens (not only in this function)
    //! @todo maybe break out find parameter function (maybe even use something else then vector for storage)
    return 0.0;
}


double Component::getDefaultParameterValue(const string name)
{
    return mDefaultParameters.find(name)->second;
}


//! @brief Get a pointer to a parameter
//!
//! This method is useful with mapping and unmapping to System parameters
//!
//! @param[in] name is the name of the wanted parameter
//! @returns a pointer to the parameter, a null ponter if not present
double *Component::getParameterValuePtr(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getValuePtr();
        }
    }
    cout << "No such parameter (return 0): " << name << endl;
    return 0;
}


//! @brief Get the value of a parameter in string format
//!
//! If the parameter is mapped by a System parameter the name of the System parameter is given instead of the value
//!
//! @param[in] name is the name of the wanted parameter
//! @returns the value of the parameter in string format, an empty string if name is not present as a parameter
std::string Component::getParameterValueTxt(const string name)
{
    std::string paramTxt="";
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        //The parameter is present
        if (mParameters[i].getName() == name)
        {
            //Check if the parameter is mapped ba a System parameter, then set the system parameter name to paramTxt
            paramTxt = mpSystemParent->getSystemParameters().findOccurrence(mParameters[i].getValuePtr());
            //The parameter is not mapped to a system parameter
            if(paramTxt.empty())
            {
                //Read out the parameter value to the string
                double value = getParameterValue(name);
                std::ostringstream oss;
                oss << value;
                paramTxt = oss.str();
            }
        }
    }
//    cout << "No such parameter (return 0): " << name << endl;
    //! @todo We should create a debug warning to user if this happens (not only in this function)
    //! @todo maybe break out find parameter function (maybe even use something else then vector for storage)
    return paramTxt;
}


//! @brief Get the parameters of the component, typically "k" in the case of a spring coeff.
//! @returns a vector of the parameters
const vector<string> Component::getParameterNames()
{
    vector<string> names;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        names.push_back(mParameters[i].getName());
    }
    return names;
}


//! @brief Get the unit of the parameter, typically "N/m" in the case of a spring coeff.
//! @param[in] name is the name of the wanted parameter
//! @returns the unit of the parameter
const string Component::getParameterUnit(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getUnit();
        }
    }
    cout << "No such parameter (return empty): " << name << endl;
    return string();
}


//! @brief Get the description of the parameter, typically "Spring coeff." in the case of a spring coeff.
//! @param[in] name is the name of the wanted parameter
//! @returns the description of the parameter
const string Component::getParameterDescription(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getDesc();
        }
    }
    cout << "No such parameter (return empty): " << name << endl;
    return string();
}


//! @brief Access method for the parameter vector
//! @returns the parameter vector
vector<CompParameter> Component::getParameterVector()
{
    return mParameters;
}


//! @brief Access method for the parameters
//! @returns a map with parameter names and values
map<string, double> Component::getParameterMap()
{
    map<string, double> parameterMap;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        parameterMap.insert(pair<string, double>(mParameters[i].getName(), mParameters[i].getValue()));
    }
    return parameterMap;
}


//! @brief Sets a parameter to a value
//! @param name Name of the parameter
//! @param value Value to asign the parameter with
//! @return true if it went OK, false otherwise
bool Component::setParameterValue(const string name, const double value)
{
    bool success = false;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (name == mParameters[i].getName())
        {
            mParameters[i].setValue(value);
            //Unmap the parameter if it is pointed from the System parameters
            mpSystemParent->getSystemParameters().unMapParameter(mParameters.at(i).mpValue);
            success = true;
        }
    }
    if (!success)
    {
        //! @todo Maybe some error handling
        cout << "No such parameter (does nothing): " << name << endl;
    }
    return success;
}


//! @brief Sets a parameter value using a key to a system parameter
//! @param parName Name of the parameter
//! @param sysParName Name name of the system parameter
//! @return true if it went OK, false otherwise
bool Component::setParameterValue(const std::string parName, const std::string sysParName)
{
    bool success = false;
    //Map it to the system parameter
    success = getSystemParent()->getSystemParameters().mapParameter(sysParName, getParameterValuePtr(parName));
    return success;
}

//! @todo Maby not have this function, solve in some other nicer way
vector<Port*> Component::getPortPtrVector()
{
    vector<Port*> vec;
    vec.clear();
    PortPtrMapT::iterator ports_it;

    //Copy every port pointer
    for (ports_it = mPortPtrMap.begin(); ports_it != mPortPtrMap.end(); ++ports_it)
    {
        vec.push_back(ports_it->second);
    }
    return vec;
}


void Component::setDesiredTimestep(const double /*timestep*/)
{
    cout << "Warning this function setDesiredTimestep is only available on subsystem components" << endl;
    assert(false);
}

bool Component::isSimulationOk()
{
    cout << "Warning this function isSimulationOk() is only available on subsystem components" << endl;
    assert(false);
	return false;
}


bool Component::isComponentC()
{
    return mIsComponentC;
}


bool Component::isComponentQ()
{
    return mIsComponentQ;
}


bool Component::isComponentSystem()
{
    return mIsComponentSystem;
}


bool Component::isComponentSignal()
{
    return mIsComponentSignal;
}


double *Component::getTimePtr()
{
    return &mTime;
}


//! @brief Adds a port to the component
//! @param [in] portname The desired name of the port (may be automatically changed)
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPort(const string portname, Port::PORTTYPE porttype, const NodeTypeT nodetype, Port::CONREQ connection_requirement)
{
    std::stringstream ss;
    ss << getName() << "::addPort";
    addLogMess(ss.str());

    //Make sure name is unique before insert
    string newname = this->determineUniquePortName(portname);

    Port* new_port = CreatePort(porttype, nodetype, newname, this);

    //Set wheter the port must be connected before simulation
    if (connection_requirement == Port::NOTREQUIRED)
    {
        //! @todo maybe use a string for OPTIONAL instead, to reduce the number of compiletime dependencies, will need to think about that a bit more
        new_port->mConnectionRequired = false;
    }

    mPortPtrMap.insert(PortPtrPairT(newname, new_port));

    //Signal autmatic name change
    if (newname != portname)
    {
        gCoreMessageHandler.addInfoMessage("Automatically changed name of added port from: {" + portname + "} to {" + newname + "}");
    }

    return new_port;
}


//! @brief Convenience method to add a PowerPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPowerPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::POWERPORT, nodetype, connection_requirement);
}

//! @brief Convenience method to add a PowerMultiPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPowerMultiPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::POWERMULTIPORT, nodetype, connection_requirement);
}

//! @brief Convenience method to add a ReadMultiPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addReadMultiPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::READMULTIPORT, nodetype, connection_requirement);
}

//! @brief Convenience method to add a ReadPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addReadPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::READPORT, nodetype, connection_requirement);
}


//! @brief Convenience method to add a WritePort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addWritePort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::WRITEPORT, nodetype, connection_requirement);
}


//! @todo this could be a template function to use with all rename in map
string Component::renamePort(const string oldname, const string newname)
{
    if (mPortPtrMap.count(oldname) != 0)
    {
        Port* temp_port_ptr;
        PortPtrMapT::iterator it;

        it = mPortPtrMap.find(oldname); //Find iterator to port
        temp_port_ptr = it->second;     //Backup copy of port ptr
        mPortPtrMap.erase(it);          //Erase old value
        string modnewname = determineUniquePortName(newname); //Make sure new name is unique
        temp_port_ptr->mPortName = modnewname;  //Set new name in port
        mPortPtrMap.insert(PortPtrPairT(modnewname, temp_port_ptr)); //Re add to map
        return modnewname;
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to rename port {" + oldname + "}, but not found");
        return oldname;
    }
}

//! @todo Do we ever actually DELETE the ports?, dosnt seem so
void Component::deletePort(const string name)
{
    PortPtrMapT::iterator it;
    it = mPortPtrMap.find(name);
    if (it != mPortPtrMap.end())
    {
        mPortPtrMap.erase(it);
        cout << "===================Erasing Port: " << name << endl;
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to delete port {" + name + "}, but not found");
    }
}

//! @brief This is a help function that returns a pointer to desired NodeData
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultvalue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! It is only ment to be used insed individual component code and automatically handles creation of dummy veriables
//! in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue, int portIdx)
{
    std::stringstream ss;
    ss << getName() << "::getSafeNodeDataPtr";
    addLogMess(ss.str());

    //If this is one of the multiports and we have NOT given a subport idx to use then give an error message to the user sothat they KNOW that they have made a mistake
    //! @todo it would be nice to solve this in some other way to avoid unecessary code, duoble implemntation in the function bellow is one way but that is even worse, this check would still be needed
    if ((pPort->getPortType() >= Port::MULTIPORT) && (portIdx<0))
    {
        gCoreMessageHandler.addErrorMessage(string("Port: ")+pPort->getPortName()+string(" is a multiport. Use getSafeMultiPortNodeDataPtr() instead of getSafeNodeDataPtr()"));
    }
    portIdx = max(portIdx,0); //Avoid underflow in size_t conversion in getNodeDataPtr()

    double *pND;
    //! @todo Maybe we should somehow try to use the startvalue as default instead somehow, need to think about this, seems like double work right now
    if(pPort->isConnected())
    {
        pND = pPort->getNodeDataPtr(dataId, portIdx);
    }
    else
    {
        pND = new double(defaultValue);
        mDummyNDptrs.push_back(pND); //Store the pointer to dummy for automatic finilize removal
    }
    return pND;
}

double *Component::getSafeMultiPortNodeDataPtr(Port* pPort, const int portIdx, const int dataId, const double defaultValue)
{
    return getSafeNodeDataPtr(pPort, dataId, defaultValue, portIdx);
}


//! @brief a virtual function that detmines a unique port name, needs to be overloaded in ComponentSystem to do this slightly different
std::string Component::determineUniquePortName(std::string portname)
{
    return findUniqueName<PortPtrMapT>(mPortPtrMap, portname);
}


void Component::setSystemParent(ComponentSystem *pComponentSystem)
{
    mpSystemParent = pComponentSystem;
}

void Component::setTypeName(const string typeName)
{
    mTypeName = typeName;
}

//Port &Component::getPortById(const size_t port_idx)
//{
//    //! @todo error handle if request outside of vector
//    return *mPortPtrs[port_idx];
//}


Port *Component::getPort(const string portname)
{
    PortPtrMapT::iterator it;
    //cout << "get Port:" << portname << endl;
    it = mPortPtrMap.find(portname);
    if (it != mPortPtrMap.end())
    {
        return it->second;
    }
    else
    {
        cout << "failed to find port: " << portname << " in component: " << this->mName << endl;
        gCoreMessageHandler.addDebugMessage("Trying to get port {" + portname + "}, but not found, pointer invalid");
        return 0;
    }
}


bool Component::getPort(const string portname, Port* &rpPort)
{
    rpPort = getPort(portname);
    if (rpPort != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void Component::setTimestep(const double timestep)
{
    mTimestep = timestep;
}


//! Sets the measured time variable for the component. This is used to measure time requirements when sorting components for multicore purposes.
//! @see getMeasuredTime()
void Component::setMeasuredTime(double time)
{
        mMeasuredTime = time;
}


//! Returns the measured time variable for the component. This is used to measure time requirements when sorting components for multicore purposes.
double Component::getMeasuredTime()
{
        return mMeasuredTime;
}


//! Write an Debug message, i.e. for debugging purposes.
void Component::addDebugMessage(string message)
{
    gCoreMessageHandler.addDebugMessage(message);
}


//! Write an Error message.
void Component::addErrorMessage(string message)
{
    gCoreMessageHandler.addErrorMessage(message);
}


//! Write an Info message.
void Component::addInfoMessage(string message)
{
    gCoreMessageHandler.addInfoMessage(message);
}


//! @brief Get the an actual start value of a port
//! @param[in] pPort is the port which should be read from
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @returns the start value
double Component::getStartValue(Port* pPort, const size_t idx)
{
    return pPort->getStartValue(idx);
}


//! @brief Set the an actual start value of a port
//! @param[in] pPort is the port which should be written to
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @param[in] value is the start value that should be written
void Component::setStartValue(Port* pPort, const size_t &idx, const double &value)
{
    std::stringstream ss;
    ss << getName() << "::setStartValue";
    addLogMess(ss.str());
    pPort->setStartValue(idx, value);
}


//! @brief Change the cqs type of a stored subsystem component
bool ComponentSystem::changeTypeCQS(const string name, const typeCQS newType)
{
    //First get the component ptr and check if we are requesting new type
    Component* tmpptr = getSubComponent(name);
    if (newType != tmpptr->getTypeCQS())
    {
        //check that it is a system component, in that case change the cqs type
        if ( tmpptr->isComponentSystem() )
        {
            //Cast to system ptr
            //! @todo should have a member function that return systemcomponent ptrs
            ComponentSystem* tmpsysptr = dynamic_cast<ComponentSystem*>(tmpptr);

            //Remove old version
            this->removeSubComponentPtrFromStorage(tmpsysptr);

            //Change cqsType localy in the subcomponent, make sure to set true to avoid looping back to this rename
            tmpsysptr->setTypeCQS(newType, true);

            //readd to system
            this->addSubComponentPtrToStorage(tmpsysptr);
        }
        else
        {
            return false;
        }
    }
    return true;
}


ComponentSystem *Component::getSystemParent()
{
    return mpSystemParent;
}

size_t Component::getModelHierarchyDepth()
{
    return mModelHierarchyDepth;
}


//constructor ComponentSignal
ComponentSignal::ComponentSignal(string name) : Component(name)
{
    //mTypeCQS = "S";
    mTypeCQS = Component::S;
    mIsComponentSignal = true;
}

Component::~Component()
{
    //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted component.
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        mpSystemParent->getSystemParameters().unMapParameter(mParameters[i].getValuePtr());
    }

    //Delete any ports that have been added to the component
    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        delete (*ppmit).second;
    }
}


//! @brief Loads the start values to the connected Node from the "start value node" at each Port of the component
void Component::loadStartValues()
{
    std::vector<Port*> pPortPtrs = getPortPtrVector();
    std::vector<Port*>::iterator portIt;
    for(portIt = pPortPtrs.begin(); portIt != pPortPtrs.end(); ++portIt)
    {
        (*portIt)->loadStartValues();
    }
}


void Component::loadStartValuesFromSimulation()
{
    std::vector<Port*> pPortPtrs = getPortPtrVector();
    std::vector<Port*>::iterator portIt;
    for(portIt = pPortPtrs.begin(); portIt != pPortPtrs.end(); ++portIt)
    {
        (*portIt)->loadStartValuesFromSimulation();
    }

}


//constructor ComponentC
ComponentC::ComponentC(string name) : Component(name)
{
    //mTypeCQS = "C";
    mTypeCQS = Component::C;
    mIsComponentC = true;
}


//Constructor ComponentQ
ComponentQ::ComponentQ(string name) : Component(name)
{
    //mTypeCQS = "Q";
    mTypeCQS = Component::Q;
    mIsComponentQ = true;
}


//Constructor
ComponentSystem::ComponentSystem(string name) : Component(name)
{
    mTypeName = "ComponentSystem";
    mIsComponentSystem = true;
    mDesiredTimestep = 0.001;
}

double ComponentSystem::getDesiredTimeStep()
{
    return mDesiredTimestep;
}


//! Sets a bool which is looked at in initialization and simulation loops.
//! This method can be used by users e.g. GUIs to stop an started initializatiion/simulation process
void ComponentSystem::stop()
{
    mStop = true;
}


//! @brief Get a reference to the System parameters
//!
//! Use this method to manipulate the System parameters, e.g. getSystemParameters().add("myNewSysPar", 42.0);
//!
//! @returns A reference to the System parameters
SystemParameters &ComponentSystem::getSystemParameters()
{
    return this->mSystemParameters;
}


void ComponentSystem::addComponents(vector<Component*> components)
{
    std::vector<Component *>::iterator itx;
    for(itx = components.begin(); itx != components.end(); ++itx)
    {
        addComponent((*itx));
    }
}



void ComponentSystem::addComponent(Component *pComponent)
{
    //First check if the name already exists, in that case change the suffix
    string modname = this->determineUniqueComponentName(pComponent->getName());
    pComponent->setName(modname);

    //Add to the cqs component vectors
    addSubComponentPtrToStorage(pComponent);

    pComponent->setSystemParent(this);
    pComponent->mModelHierarchyDepth = this->mModelHierarchyDepth+1; //Set the ModelHierarchyDepth counter
}


//! Rename a sub component and automatically fix unique names
void ComponentSystem::renameSubComponent(string oldname, string newname)
{
    //cout << "Trying to rename: " << old_name << " to " << new_name << endl;
    //First find the post in the map where the old name resides, copy the data stored there
    SubComponentMapT::iterator it = mSubComponentMap.find(oldname);
    Component* temp_comp_ptr;
    if (it != mSubComponentMap.end())
    {
        //If found erase old record
        temp_comp_ptr = it->second;
        mSubComponentMap.erase(it);

        //insert new (with new name)
        string mod_new_name = this->determineUniqueComponentName(newname);

        //cout << "new name is: " << mod_name << endl;
        mSubComponentMap.insert(pair<string, Component*>(mod_new_name, temp_comp_ptr));

        //Now change the actual component name, without trying to do rename (we are in rename now, would cause infinite loop)
        temp_comp_ptr->setName(mod_new_name, true);
    }
    else
    {
        cout << "Error no component with old_name: " << oldname << " found!" << endl;
        assert(false);
    }
}


//! Remove a dub component from a system, can also be used to actually delete the component
//! @param[in] name The name of the component to remove from the system
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(string name, bool doDelete)
{
    Component* pComp = getSubComponent(name);
    removeSubComponent(pComp, doDelete);
}


//! Remove a sub component from a system, can also be used to actually delete the component
//! @param[in] c_ptr A pointer to the component to remove
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(Component* pComponent, bool doDelete)
{
    std::string compName = pComponent->getName();

    //Disconnect all ports before erase from system
    PortPtrMapT::iterator ports_it;
    vector<Port*>::iterator conn_ports_it;
    for (ports_it = pComponent->mPortPtrMap.begin(); ports_it != pComponent->mPortPtrMap.end(); ++ports_it)
    {
        //! @todo what about multiports here
        vector<Port*> connected_ports = ports_it->second->getConnectedPorts(); //Get a copy of the connected ports ptr vector
        //We can not use an iterator directly connected to the vector inside the port as this will be changed by the disconnect calls
        for (conn_ports_it = connected_ports.begin(); conn_ports_it != connected_ports.end(); ++conn_ports_it)
        {
            disconnect(ports_it->second, *conn_ports_it);
        }
    }

    //Remove from storage
    removeSubComponentPtrFromStorage(pComponent);

    //Shall we also delete the component completely
    if (doDelete)
    {
        delete pComponent; //! @todo can I really delete here or do I need to use the factory for external components
    }

    gCoreMessageHandler.addDebugMessage("Removed component: \"" + compName + "\" from system: \"" + this->getName() + "\"", "removedcomponent");
}

string ComponentSystem::reserveUniqueName(string desiredName)
{
    string newname = this->determineUniqueComponentName(desiredName);
    mReservedNames.insert(std::pair<std::string, int>(newname,0)); //The inte 0 is a dummy value that is never used
    return newname;
}

void ComponentSystem::unReserveUniqueName(string name)
{
    cout << "unReserveUniqueName: " << name;
    cout << " count before: " << mReservedNames.count(name);
    mReservedNames.erase(name);
    cout << " count after: " << mReservedNames.count(name) << std::endl;
}

void ComponentSystem::addSubComponentPtrToStorage(Component* pComponent)
{
    switch (pComponent->getTypeCQS())
    {
    case Component::C :
        mComponentCptrs.push_back(pComponent);
        break;
    case Component::Q :
        mComponentQptrs.push_back(pComponent);
        break;
    case Component::S :
        mComponentSignalptrs.push_back(pComponent);
        break;
    case Component::NOCQSTYPE :
        mComponentUndefinedptrs.push_back(pComponent);
        break;
    default :
            gCoreMessageHandler.addErrorMessage("Trying to add module with unspecified CQS type: " + pComponent->getTypeCQSString()  + ", (Not added)");
        return;
    }

    mSubComponentMap.insert(pair<string, Component*>(pComponent->getName(), pComponent));
}

void ComponentSystem::removeSubComponentPtrFromStorage(Component* pComponent)
{
    SubComponentMapT::iterator it = mSubComponentMap.find(pComponent->getName());
    if (it != mSubComponentMap.end())
    {
        vector<Component*>::iterator cit; //Component iterator
        switch (it->second->getTypeCQS())
        {
        case Component::C :
            for (cit = mComponentCptrs.begin(); cit != mComponentCptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentCptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::Q :
            for (cit = mComponentQptrs.begin(); cit != mComponentQptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentQptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::S :
            for (cit = mComponentSignalptrs.begin(); cit != mComponentSignalptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentSignalptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::NOCQSTYPE :
            for (cit = mComponentUndefinedptrs.begin(); cit != mComponentUndefinedptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentUndefinedptrs.erase(cit);
                    break;
                }
            }
            break;
        default :
            cout << "This should not happen neither C Q or S type" << endl;
            assert(false);
        }

        mSubComponentMap.erase(it);
    }
    else
    {
        gCoreMessageHandler.addErrorMessage("The component you are trying to remove: " + pComponent->getName() + " does not exist (Does Nothing)");
    }
}


//! @brief Sorts the signal component vector
//! Components are sorted so that they are always simulated after the components they receive signals from. Algebraic loops can be detected, in that case this function does nothing.
void ComponentSystem::sortSignalComponentVector()
{
    std::vector<Component*> newSignalVector;

    bool didSomething = true;
    while(didSomething)
    {
        didSomething = false;
        std::vector<Component*>::iterator it;
        for(it=mComponentSignalptrs.begin(); it!=mComponentSignalptrs.end(); ++it)  //Loop through the unsorted signal component vector
        {
            if(!componentVectorContains(newSignalVector, (*it)))    //Ignore components that are already added to the new vector
            {
                bool readyToAdd=true;
                std::vector<Port*>::iterator itp;
                std::vector<Port*> portVector = (*it)->getPortPtrVector();
                for(itp=portVector.begin(); itp!=portVector.end(); ++itp) //Ask each port for its node, then ask the node for its write port component
                {
                    if(((*itp)->getPortType() == Port::READPORT) &&
                       ((*itp)->isConnected()) &&
                       ((!componentVectorContains(newSignalVector, (*itp)->getNodePtr()->getWritePortComponentPtr())) && (*itp)->getNodePtr()->getWritePortComponentPtr() != 0 &&(*itp)->getNodePtr()->getWritePortComponentPtr()->getTypeCQS() == Component::S) &&
                       ((*itp)->getNodePtr()->getWritePortComponentPtr() != 0) &&
                       ((*itp)->getNodePtr()->getWritePortComponentPtr()->mpSystemParent == this))
                    {
                        readyToAdd=false;   //Flag false if required component is not yet added to signal vector, in case node has a write port
                    }
                }
                if(readyToAdd)  //Add the component if all required write port components was already added
                {
                    newSignalVector.push_back((*it));
                    didSomething = true;
                }
            }
        }
    }

    if(newSignalVector.size() == mComponentSignalptrs.size())   //All components moved to new vector = success!
    {
        mComponentSignalptrs = newSignalVector;
        stringstream ss;
        std::vector<Component*>::iterator it;
        for(it=newSignalVector.begin(); it!=newSignalVector.end(); ++it)
            ss << (*it)->getName() << "\n";                                                                                               //DEBUG
        gCoreMessageHandler.addDebugMessage("Sorted signal components:\n" + ss.str());
    }
    else    //Something went wrong, all components were not moved. This is likely due to an algebraic loop.
    {
        gCoreMessageHandler.addWarningMessage("Found algebraic loop in signal components. Sorting not possible.");
    }
}


//! @brief Figures out whether or not a component vector contains a certain component
bool ComponentSystem::componentVectorContains(std::vector<Component*> vector, Component *pComp)
{
    std::vector<Component*>::iterator it;
    for(it=vector.begin(); it!=vector.end(); ++it)
    {
        if((*it) == pComp)
        {
            return true;
        }
    }
    return false;
}


//! @brief Overloaded function that behaves slightly different when determining unique port names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports dont have the same name as a subcomponent
std::string ComponentSystem::determineUniquePortName(std::string portname)
{
    return findUniqueName<PortPtrMapT, SubComponentMapT, ReservedNamesT>(mPortPtrMap,  mSubComponentMap, mReservedNames, portname);
}

//! @brief Overloaded function that behaves slightly different when determining unique component names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports dont have the same name as a subcomponent
//! @todo the determineUniquePortNAme and ComponentName looks VERY similar maybe we could use the same function for both
std::string ComponentSystem::determineUniqueComponentName(std::string name)
{
    return findUniqueName<SubComponentMapT, PortPtrMapT, ReservedNamesT>(mSubComponentMap, mPortPtrMap, mReservedNames, name);
}


//! @brief Get a Component ptr to the component with supplied name, can also return a ptr to self if no subcomponent found but systemport with name found
//! For this to work we need to make sure that the sub components and systemports have unique names
Component* ComponentSystem::getComponent(string name)
{
//    cout << "getComponent: " << name << " in: " << mName << endl;
    //First try to find among subcomponents
    Component *tmp = getSubComponent(name);
    if (tmp == 0)
    {
        //Now try to find among systemports
        Port* pPort = this->getPort(name);
        if (pPort != 0)
        {
            if (pPort->getPortType() == Port::SYSTEMPORT)
            {
                //Return the systemports owner (the system component)
                tmp = pPort->getComponent();
                //cout << "Found systemport with name: " << name << " returning parent: " << tmp->getName() << endl;
            }
        }
    }
    return tmp;
}


Component* ComponentSystem::getSubComponent(string name)
{
//    cout << "getSubComponent: " << name << " in " <<  this->mName << endl;
    SubComponentMapT::iterator it = mSubComponentMap.find(name);
    if (it != mSubComponentMap.end())
    {
        return it->second;
    }
    else
    {
        //cout << "getSubComponent: The component you requested: " << name << " does not exist in: " << this->mName << endl;
        return 0;
    }
}


ComponentSystem* ComponentSystem::getSubComponentSystem(string name)
{
    Component* temp_component_ptr = getSubComponent(name);
    ComponentSystem* temp_compsys_ptr = dynamic_cast<ComponentSystem*>(temp_component_ptr);

    if (temp_compsys_ptr == NULL)
    {
        cout << "dynamic cast failed, maybe " << name << " is not a component system" << endl;
        assert(false);
    }

    return temp_compsys_ptr;
}


vector<string> ComponentSystem::getSubComponentNames()
{
    //! @todo for now create a vector of the component names, later maybe we should return a pointer to the real internal map
    vector<string> names;
    SubComponentMapT::iterator it;
    for (it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
    {
        names.push_back(it->first);
    }

    return names;
}


bool  ComponentSystem::haveSubComponent(string name)
{
    if (mSubComponentMap.count(name) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//! Adds a node as subnode in the system, if the node is already owned by someone else, trasfere owneship to this system
void ComponentSystem::addSubNode(Node* node_ptr)
{
    if (node_ptr->getOwnerSystem() != 0)
    {
        node_ptr->getOwnerSystem()->removeSubNode(node_ptr);
    }
    mSubNodePtrs.push_back(node_ptr);
    node_ptr->mpOwnerSystem = this;
}


//! Removes a previously added node
void ComponentSystem::removeSubNode(Node* node_ptr)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        if (*it == node_ptr)
        {
            node_ptr->mpOwnerSystem = 0;
            mSubNodePtrs.erase(it);
            break;
        }
    }
    //! @todo some notification if you try to remove something that does not exist (can not check it==mSubNodePtrs.end() ) this check can be OK after an successfull erase
}


//! preAllocates log space (to speed up later access for log writing)
void ComponentSystem::preAllocateLogSpace(const double startT, const double stopT, const size_t nSamples)
{
    cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;

    //Allocate memory for subnodes
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        if (mStop)
            break;

        //! @todo this is an ugly quit hack test
        (*it)->setLogSettingsNSamples(nSamples, startT, stopT, mTimestep);
        //(*it)->setLogSettingsSkipFactor(1, startT, stopT, mTimestep);
        //(*it)->preAllocateLogSpace(needed_slots);
        (*it)->preAllocateLogSpace();
    }
}


//! Tells all subnodes contained within a system to store current data in log
void ComponentSystem::logAllNodes(const double time)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        (*it)->logData(time);
    }
}


//! Adds a transparent SubSystemPort
Port* ComponentSystem::addSystemPort(string portname)
{
    if (portname.empty())
    {
        //Force default portname p, if nothing else specified
        portname = "p";
    }

    //! @todo not hardcode, "undefined_nodetype" maybe define or something, it is used elsevere also
    return addPort(portname, Port::SYSTEMPORT, "undefined_nodetype", Port::REQUIRED);
}


//! Rename system port
string ComponentSystem::renameSystemPort(const string oldname, const string newname)
{
    return renamePort(oldname,newname);
}


//! delete System prot
void ComponentSystem::deleteSystemPort(const string name)
{
    deletePort(name);
}


//! Set the type C, Q, or S of the subsystem by using string
void ComponentSystem::setTypeCQS(const string cqs_type, bool doOnlyLocalSet)
{
    if (cqs_type == string("C"))
    {
        setTypeCQS(Component::C, doOnlyLocalSet);
    }
    else if (cqs_type == string("Q"))
    {
        setTypeCQS(Component::Q, doOnlyLocalSet);
    }
    else if (cqs_type == string("S"))
    {
        setTypeCQS(Component::S, doOnlyLocalSet);
    }
    else
    {
        cout << "Error: Specified type \"" << cqs_type << "\" does not exist!" << endl;
        gCoreMessageHandler.addWarningMessage("Specified type: " + cqs_type + " does not exist!, System CQStype unchanged");
    }
}


//! Set the type C, Q, or S of the subsystem
void ComponentSystem::setTypeCQS(typeCQS cqs_type, bool doOnlyLocalSet)
{
    //! @todo should really try to figure out a better way to do this
    //! @todo need to do erro checking, and make sure that the specified type really is valid, first and last component should be of this type (i think)

    //If type same as before do nothing
    if (cqs_type !=  mTypeCQS)
    {
        //Do we have a system parent
        if ( (mpSystemParent != 0) && (!doOnlyLocalSet) )
        {
            //Request change by our parent (som parent cahnges are neeeded)
            mpSystemParent->changeTypeCQS(mName, cqs_type);
        }
        else
        {
            switch (cqs_type)
            {
            case Component::C :
                mTypeCQS = Component::C;
                mIsComponentC = true;
                mIsComponentQ = false;
                mIsComponentSignal = false;
                break;

            case Component::Q :
                mTypeCQS = Component::Q;
                mIsComponentC = false;
                mIsComponentQ = true;
                mIsComponentSignal = false;
                break;

            case Component::S :
                mTypeCQS = Component::S;
                mIsComponentC = false;
                mIsComponentQ = false;
                mIsComponentSignal = true;
                break;

            case Component::NOCQSTYPE :
                mTypeCQS = Component::NOCQSTYPE;
                mIsComponentC = false;
                mIsComponentQ = false;
                mIsComponentSignal = false;
                break;

            default :
                cout << "Error: Specified type _" << getTypeCQSString() << "_ does not exist!" << endl;
                gCoreMessageHandler.addWarningMessage("Specified type: " + getTypeCQSString() + " does not exist!, System CQStype unchanged");
            }
        }
    }
}


//! Connect two commponents string version
bool ComponentSystem::connect(string compname1, string portname1, string compname2, string portname2)
{
    Port* pPort1;
    Port* pPort2;

    //First some error checking
    stringstream ss; //Error string stream

    //Check if the components exist (and can be found)
    Component* pComp1 = getComponent(compname1);
    Component* pComp2 = getComponent(compname2);

    if (pComp1 == 0)
    {
        ss << "Component1: "<< compname1 << " can not be found when atempting connect";
        gCoreMessageHandler.addErrorMessage(ss.str(), "connectwithoutcomponent");
        return false;
    }

    if (pComp2 == 0)
    {
        ss << "Component2: "<< compname2 << " can not be found when atempting connect";
        gCoreMessageHandler.addErrorMessage(ss.str(), "connectwithoutcomponent");
        return false;
    }

    //Check if commponents have specified ports
    if (!pComp1->getPort(portname1, pPort1))
    {
        ss << "Component: "<< pComp1->getName() << " does not have a port named " << portname1;
        gCoreMessageHandler.addErrorMessage(ss.str(), "portdoesnotexist");
        return false;
    }

    if (!pComp2->getPort(portname2, pPort2)) //Not else if because pPort2 has to be set in getPort
    {
        //raise Exception('type of port does not exist')
        ss << "Component: "<< pComp2->getName() << " does not have a port named " << portname2;
        gCoreMessageHandler.addErrorMessage(ss.str(), "portdoesnotexist");
        return false;
    }

    //Ok components and ports exist, lets atempt the connect
    return connect( pPort1, pPort2 );
}

bool ConnectionAssistant::ensureSameNodeType(Port *pPort1, Port *pPort2)
{
    //Check if both ports have the same node type specified
    if (pPort1->getNodeType() != pPort2->getNodeType())
    {
        stringstream ss;
        ss << "You can not connect a {" << pPort1->getNodeType() << "} port to a {" << pPort2->getNodeType()  << "} port." <<
              "When connecting: {" << pPort1->getComponent()->getName() << "::" << pPort1->getPortName() << "} to {" << pPort2->getComponent()->getName() << "::" << pPort2->getPortName() << "}";
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }
    return true;
}

//! Assumes that nodetype is set in both nodes
bool ConnectionAssistant::createNewNodeConnection(Port *pPort1, Port *pPort2, Node *&rpCreatedNode)
{
    std::cout << "-----------------------------createNewNodeConnection" << std::endl;
    if (!ensureSameNodeType(pPort1, pPort2))
    {
        return false;
    }

    //Create an instance of the node specified in nodespecifications
    //Node* pNode = gCoreNodeFactory.createInstance(pPort1->getNodeType());
    Node* pNode = HopsanEssentials::getInstance()->createNode(pPort1->getNodeType());

    // Check so the ports can be connected
    if (ensureConnectionOK(pNode, pPort1, pPort2))
    {
        //Set node in both components ports and add it to the parent system component
        pPort1->setNode(pNode);
        pPort2->setNode(pNode);

        //Add port pointers to node
        pNode->setPort(pPort1);
        pNode->setPort(pPort2);

        //let the ports know about each other
        pPort1->addConnectedPort(pPort2);
        pPort2->addConnectedPort(pPort1);

        //Return the created node
        rpCreatedNode = pNode;
        return true;
    }
    else
    {
        stringstream ss;
        ss << "Problem occured at connection" << pPort1->getComponentName() << " and " << pPort2->getComponentName();
        gCoreMessageHandler.addErrorMessage(ss.str());
        delete pNode;
        rpCreatedNode = 0;
        return false;
    }
}


bool ConnectionAssistant::mergeOrJoinNodeConnection(Port *pPort1, Port *pPort2, Node *&rpCreatedNode)
{
    std::cout << "-----------------------------mergeOrJoinNodeConnection" << std::endl;
    //! @todo no isok check is performed (not checks at all are performed)
    Port *pMergeFrom, *pMergeTo;
    assert(pPort1->isConnected() || pPort2->isConnected());

    if (!ensureSameNodeType(pPort1, pPort2))
    {
        return false;
    }

    //Ok, should we merge or join node connection
    //lets allways merge, but if node is missing in one port than the "merge" is actually a join
    if (!pPort1->isConnected())
    {
        pMergeFrom = pPort1;
        pMergeTo = pPort2;
    }
    else if (!pPort2->isConnected())
    {
        pMergeFrom = pPort2;
        pMergeTo = pPort1;
    }
    else
    {
        //! @todo maybe we should selcet in some smart way, for now lets merge from port1
        pMergeFrom = pPort1;
        pMergeTo = pPort2;
    }

    //lets keep the node in merge to port
    Node *pKeepNode = pMergeTo->getNodePtr();
    Node *pDiscardNode = pMergeFrom->getNodePtr();

    assert(pKeepNode != pDiscardNode);

    //set the new node recursively in the other port
    recursivelySetNode(pMergeFrom,0, pKeepNode);

    //let the ports know about each other
    pMergeFrom->addConnectedPort(pMergeTo);
    pMergeTo->addConnectedPort(pMergeFrom);

    if (pDiscardNode != 0)
    {
//        std::cout << "node2 ports size: " <<  pDiscardNode->mPortPtrs.size() << std::endl;
//        assert(pDiscardNode->mPortPtrs.size() == 0);
        //! @todo Right now we dont empty the node to be discarded, we just delete it, this should be OK, but if we implement a recursivelyUnSetNode function we could do this is empty check agian, the advantage of this check is to make sure that we are not doing any mistakes in the code
        pDiscardNode->getOwnerSystem()->removeSubNode(pDiscardNode);
        delete pDiscardNode;
    }


    if (ensureConnectionOK(pKeepNode, pMergeFrom, pMergeTo))
    {
        rpCreatedNode = pKeepNode;
        return true;
    }
    else
    {
        unmergeOrUnjoinConnection(pMergeFrom, pMergeTo); //Undo connection
        rpCreatedNode = 0;
        return false;
    }
}

void ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(Node* pNode)
{
    //node ptr should not be zero
    assert(pNode != 0);

    vector<Port*>::iterator pit;
    Component *pMinLevelComp=0;
    size_t min = std::numeric_limits<size_t>::max();
    for (pit=pNode->mPortPtrs.begin(); pit!=pNode->mPortPtrs.end(); ++pit)
    {
        if ((*pit)->getComponent()->getModelHierarchyDepth() < min)
        {
            min = (*pit)->getComponent()->getModelHierarchyDepth();
            pMinLevelComp = (*pit)->getComponent();
        }
    }

    //Now add the node at the minimum level, if minimum is a system (we are connecting to our system parant) then dyncast the pointer
    //! @todo what if we are connecting only subsystems within the same lavel AND they have different timesteps
    if (pMinLevelComp->isComponentSystem())
    {
        ComponentSystem *pRootSys = dynamic_cast<ComponentSystem*>(pMinLevelComp);
        pRootSys->addSubNode(pNode);
    }
    else
    {
        pMinLevelComp->getSystemParent()->addSubNode(pNode);
    }
}

bool ConnectionAssistant::deleteNodeConnection(Port *pPort1, Port *pPort2)
{
    stringstream ss;
    assert(pPort1->getNodePtr() == pPort2->getNodePtr());
    Node* node_ptr = pPort1->getNodePtr();
    cout << "nPorts in node: " << node_ptr->mPortPtrs.size() << endl;

    //Make the ports forget about each other
    pPort1->eraseConnectedPort(pPort2);
    pPort2->eraseConnectedPort(pPort1);

    //Make the node forget about the ports
    node_ptr->removePort(pPort1);
    node_ptr->removePort(pPort2);

    //If no more connections exist, remove the entier node and free the memory
    if (node_ptr->mPortPtrs.size() == 0)
    {
        cout << "No more connections to the node exists, deleteing the node" << endl;
        node_ptr->getOwnerSystem()->removeSubNode(node_ptr);
        delete node_ptr;
        //! @todo maybe need to let the factory remove it insted of manually, in case of user supplied external nodes
    }

    return true;
}

void ConnectionAssistant::recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode)
{
    pPort->setNode(pNode);
    pNode->setPort(pPort);
    vector<Port*>::iterator pit;
    for (pit=pPort->getConnectedPorts().begin(); pit!=pPort->getConnectedPorts().end(); ++pit)
    {
        //dont recures back to parent will get stuck in infinate recursion
        if (*pit == pParentPort)
        {
            continue;
        }
        recursivelySetNode(*pit, pPort, pNode);
    }
}

bool ConnectionAssistant::unmergeOrUnjoinConnection(Port *pPort1, Port *pPort2)
{
    Port *pPortToBecomeEmpty=0, *pPortToKeep=0;

    //make sure not both ports will become empty this is handled by other code
    assert( !((pPort1->getConnectedPorts().size() < 2) && (pPort2->getConnectedPorts().size() < 2))  );

    if (pPort1->getConnectedPorts().size() < 2)
    {
        pPortToBecomeEmpty = pPort1;
        pPortToKeep = pPort2;
    }
    else if (pPort2->getConnectedPorts().size() < 2)
    {
        pPortToBecomeEmpty = pPort2;
        pPortToKeep = pPort1;
    }
    else
    {
        //unmerge
        //Handled by if below
    }

    //Check if we are unjoining
    if (pPortToBecomeEmpty !=0)
    {
        Node* pKeepNode = pPortToKeep->getNodePtr();

        //Make node forget the port to be disconnected
        pKeepNode->removePort(pPortToBecomeEmpty);

        //Make the ports forget each other (the disconnected port will also forget node)
        pPortToBecomeEmpty->eraseConnectedPort(pPortToKeep);
        pPortToKeep->eraseConnectedPort(pPortToBecomeEmpty);

        determineWhereToStoreNodeAndStoreIt(pKeepNode); //Relocaate the node if necessary
    }
    //Else we seems to be unmerging, create anew node for teh "other side" of the broken connection
    else
    {
        //! @todo maybe make sure that the ports are really systemports to avoid code misstakes
        //Lets keep the node from port1 and create a copy for port two
        Node* pNode1 = pPort1->getNodePtr();
        //Node* pNode2 = gCoreNodeFactory.createInstance(pNode1->getNodeType());
        Node* pNode2 = HopsanEssentials::getInstance()->createNode(pNode1->getNodeType());

        pNode1->mPortPtrs.clear(); //Clear all port knowledge from the port, we will reset it bellow

        //Make the ports forget about each other
        pPort1->eraseConnectedPort(pPort2);
        pPort2->eraseConnectedPort(pPort1);

        //Recursively set the node in the port, directly connected ports and infinitely recursive connected ports
        recursivelySetNode(pPort1,0,pNode1);
        recursivelySetNode(pPort2,0,pNode2);

        //Now determine what system should own the node
        determineWhereToStoreNodeAndStoreIt(pNode1);
        determineWhereToStoreNodeAndStoreIt(pNode2);

    }

    return true;
}


//! Helpfunction that clears the nodetype in empty systemports, It will not clear the type if the port is not empty or if the port is not a systemport
void ConnectionAssistant::clearSysPortNodeTypeIfEmpty(Port *pPort)
{
    if ( (pPort->getPortType() == Port::SYSTEMPORT) && (!pPort->isConnected()) )
    {
        pPort->mNodeType = "";
    }
}

//! Connect two components with specified ports to each other, reference version
bool ComponentSystem::connect(Port *pPort1, Port *pPort2)
{
    ConnectionAssistant connAssist;
    Component* pComp1 = pPort1->getComponent();
    Component* pComp2 = pPort2->getComponent();
    bool sucess;

    //First some error checking
    stringstream ss; //Message string stream

    //Prevent connection with self
    if (pPort1 == pPort2)
    {
        ss << "You can not connect a port to it self";
        gCoreMessageHandler.addErrorMessage(ss.str(), "selfconnection");
        return false;
    }

    //Prevent connection if porst are already connected to each other
    //! @todo What will happend with multiports
    if (pPort1->isConnectedTo(pPort2))
    {
        ss << "These two ports are already connected to each other";
        gCoreMessageHandler.addErrorMessage(ss.str(), "allreadyconnected");
        return false;
    }

    if (!connAssist.ensureNotCrossConnecting(pPort1, pPort2))
    {
        ss << "You can not cross-connect between systems";
        gCoreMessageHandler.addErrorMessage(ss.str(), "crossconnection");
        return false;
    }

    //Prevent connection of two blank systemports
    if ( (pPort1->getPortType() == Port::SYSTEMPORT) && (pPort2->getPortType() == Port::SYSTEMPORT) )
    {
        if ( (!pPort1->isConnected()) && (!pPort2->isConnected()) )
        {
            ss << "You are not allowed to connect two blank systemports to each other";
            gCoreMessageHandler.addErrorMessage(ss.str());
            return false;
        }
    }

    Node *pResultingNode = 0;
    //Now lets find out if one of the ports is a blank systemport
    //! @todo better way to find out if systemports are blank might give more clear code
    if ( ( (pPort1->getPortType() == Port::SYSTEMPORT) && (!pPort1->isConnected()) ) || ( (pPort2->getPortType() == Port::SYSTEMPORT) && (!pPort2->isConnected()) ) )
    {
        //Now lets find out wich of the ports that is a blank systemport
        Port *pBlankSysPort;
        Port *pOtherPort;

        //! @todo write help function
        if ( (pPort1->getPortType() == Port::SYSTEMPORT) && (!pPort1->isConnected()) )
        {
            pBlankSysPort = pPort1;
            pOtherPort = pPort2;
        }
        else if ( (pPort2->getPortType() == Port::SYSTEMPORT) && (!pPort2->isConnected()) )
        {
            pBlankSysPort = pPort2;
            pOtherPort = pPort1;
        }
        else
        {
            //this should not happen, assert is making sure we dont code wrong
            assert(false);
        }

        pBlankSysPort->mNodeType = pOtherPort->getNodeType(); //set the nodetype in the sysport
        //! @todo We must be able to handle connecting multiports to blank systemports
        if (!pOtherPort->isConnected())
        {
            sucess = connAssist.createNewNodeConnection(pBlankSysPort, pOtherPort, pResultingNode);
        }
        else
        {
            sucess = connAssist.mergeOrJoinNodeConnection(pBlankSysPort, pOtherPort, pResultingNode);
        }
    }
    //Non of the ports  are blank systemports
    else
    {
        //Check if we are connecting multiports, in that case add new subport, remember original portPointer though so that we can clean up if failure
        Port *pMultiPort1=0, *pMultiPort2=0;
        connAssist.ifMultiportAddSubportAndSwapPtr(pPort1, pMultiPort1);
        connAssist.ifMultiportAddSubportAndSwapPtr(pPort2, pMultiPort2);

        if (!pPort1->isConnected() && !pPort2->isConnected())
        {
            sucess = connAssist.createNewNodeConnection(pPort1, pPort2, pResultingNode);
        }
        else
        {
            sucess = connAssist.mergeOrJoinNodeConnection(pPort1, pPort2, pResultingNode);
        }

        //Handle multiport connection sucess or failure
        connAssist.ifMultiportCleanupAfterConnect(pPort1, pMultiPort1, sucess);
        connAssist.ifMultiportCleanupAfterConnect(pPort2, pMultiPort2, sucess);
    }

    //Abbort conenction if there was a connect failure
    if (!sucess)
    {
        return false;
    }

    //Update the node placement
    connAssist.determineWhereToStoreNodeAndStoreIt(pResultingNode);

    ss << "Connected: {" << pComp1->getName() << "::" << pPort1->getPortName() << "} and {" << pComp2->getName() << "::" << pPort2->getPortName() << "}";
    gCoreMessageHandler.addDebugMessage(ss.str(), "succesfulconnect");
    return true;
}



bool ConnectionAssistant::ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2)
{
    size_t n_ReadPorts = 0;
    size_t n_WritePorts = 0;
    size_t n_PowerPorts = 0;
    size_t n_SystemPorts = 0;

    size_t n_Ccomponents = 0;
    size_t n_Qcomponents = 0;
    size_t n_SYScomponentCs = 0;
    size_t n_SYScomponentQs = 0;

    //Count the different kind of ports and C,Q components in the node
    vector<Port*>::iterator it;
    for (it=(*pNode).mPortPtrs.begin(); it!=(*pNode).mPortPtrs.end(); ++it)
    {
        if ((*it)->getPortType() == Port::READPORT)
        {
            n_ReadPorts += 1;
        }
        else if ((*it)->getPortType() == Port::WRITEPORT)
        {
            n_WritePorts += 1;
        }
        else if ((*it)->getPortType() == Port::POWERPORT)
        {
            n_PowerPorts += 1;
        }
        else if ((*it)->getPortType() == Port::SYSTEMPORT)
        {
            n_SystemPorts += 1;
        }

        if ((*it)->getComponent()->isComponentC())
        {
            n_Ccomponents += 1;
            if ((*it)->getComponent()->isComponentSystem())
            {
                n_SYScomponentCs += 1;
            }
        }
        else if ((*it)->getComponent()->isComponentQ())
        {
            n_Qcomponents += 1;
            if ((*it)->getComponent()->isComponentSystem())
            {
                n_SYScomponentQs += 1;
            }
        }
    }

    //Check the kind of ports in the components subjected for connection
    //Dont count port if it is already conected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort1) )
    {
        if ( pPort1->getPortType() == Port::READPORT )
        {
            n_ReadPorts += 1;
        }
        if ( pPort1->getPortType() == Port::WRITEPORT )
        {
            n_WritePorts += 1;
        }
        if ( pPort1->getPortType() == Port::POWERPORT )
        {
            n_PowerPorts += 1;
        }
        if ( pPort1->getPortType() == Port::SYSTEMPORT )
        {
            n_SystemPorts += 1;
        }
        if ( pPort1->getComponent()->isComponentC() )
        {
            n_Ccomponents += 1;
            if ( pPort1->getComponent()->isComponentSystem() )
            {
                n_SYScomponentCs += 1;
            }
        }
        if ( pPort1->getComponent()->isComponentQ() )
        {
            n_Qcomponents += 1;
            if ( pPort1->getComponent()->isComponentSystem() )
            {
                n_SYScomponentQs += 1;
            }
        }
    }

    //Dont count port if it is already conected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort2) )
    {
        if ( pPort2->getPortType() == Port::READPORT )
        {
            n_ReadPorts += 1;
        }
        if ( pPort2->getPortType() == Port::WRITEPORT )
        {
            n_WritePorts += 1;
        }
        if ( pPort2->getPortType() == Port::POWERPORT )
        {
            n_PowerPorts += 1;
        }
        if ( pPort2->getPortType() == Port::SYSTEMPORT )
        {
            n_SystemPorts += 1;
        }
        if ( pPort2->getComponent()->isComponentC() )
        {
            n_Ccomponents += 1;
            if ( pPort2->getComponent()->isComponentSystem() )
            {
                n_SYScomponentCs += 1;
            }
        }
        if ( pPort2->getComponent()->isComponentQ() )
        {
            n_Qcomponents += 1;
            if ( pPort2->getComponent()->isComponentSystem() )
            {
                n_SYScomponentQs += 1;
            }
        }
    }

    //Check if there are some problems with the connection
    if (n_PowerPorts > 2)
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect more than two PowerPorts to same node");
        return false;
    }
    if (n_WritePorts > 1)
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect more than one WritePort to same node");
        return false;
    }
    if ((n_PowerPorts > 0) && (n_WritePorts > 0))
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect WritePort and PowerPort to same node");
        return false;
    }
    if ((n_PowerPorts == 0) && (n_WritePorts == 0) && (n_SystemPorts == 0))
    {
        cout << "Trying to connect only ReadPorts" << endl;
        gCoreMessageHandler.addErrorMessage("Trying to connect only ReadPorts");
        return false;
    }

    //cout << "nQ: " << n_Qcomponents << " nC: " << n_Ccomponents << endl;

    //Normaly we want at most one c and one q component but if there happen to be a subsystem in the picture allow one extra
    //This is only true if at least one powerport is connected - signal connecetions can be between any types of components
    //! @todo not 100% sure that this will work allways. Only work if we assume that the subsystem has the correct cqs type when connecting
    if ((n_Ccomponents > 1+n_SYScomponentCs) && (n_PowerPorts > 0))
    {
        cout << "Trying to connect two C-Components to each other" << endl;
        gCoreMessageHandler.addErrorMessage("Trying to connect two C-Component ports to each other");
        return false;
    }
    if ((n_Qcomponents > 1+n_SYScomponentQs) && (n_PowerPorts > 0))
    {
        cout << "Trying to connect two Q-Components to each other" << endl;
        gCoreMessageHandler.addErrorMessage("Trying to connect two Q-Component ports to each other");
        return false;
    }
//    if ((pPort1->getPortType() == Port::READPORT) &&  (pPort2->getPortType() == Port::READPORT))
//    {
//        gCoreMessageHandler.addErrorMessage("Trying to connect ReadPort to ReadPort");
//        return false;
//    }
//    if( ((pPort1->getPortType() == Port::READPORT) && pPort2->getPortType() == Port::POWERPORT && n_PowerPorts > 1) or
//        ((pPort2->getPortType() == Port::READPORT) && pPort1->getPortType() == Port::POWERPORT && n_PowerPorts > 1) )
//    {
//        gCoreMessageHandler.addErrorMessage("Trying to connect one ReadPort to more than one PowerPort");
//        return false;
//    }

    //It seems to be OK!
    return true;
}

bool ConnectionAssistant::ensureNotCrossConnecting(Port *pPort1, Port *pPort2)
{
    //Check so that both components to connect have been added to the same system (or we are connecting to parent system)
    if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()->getSystemParent()) )
    {
        if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()) && (pPort2->getComponent()->getSystemParent() != pPort1->getComponent()) )
        {
            stringstream ss;
            ss << "The components, {"<< pPort1->getComponentName() << "} and {" << pPort2->getComponentName() << "}, "<< "must belong to the same subsystem";
            gCoreMessageHandler.addErrorMessage(ss.str());
            return false;
        }
    }
    return true;
}

//! @brief Detects if a port is a multiport, adds a subport and swaps the pointer, storing original port in argument two ptr
//! @param [in out] rpPort A refrence to a pointer to the port, will be swapped to new subport if multiport
//! @param [in out] rpOriginalPort A refrence to a pointer to the original multiport, will be 0 if not a multiport, will point to the multiport otherwise
void ConnectionAssistant::ifMultiportAddSubportAndSwapPtr(Port *&rpPort, Port *&rpOriginalPort)
{
    rpOriginalPort = 0; //Make sure null if not multiport
    if (rpPort->getPortType() >= Port::MULTIPORT)
    {
        rpOriginalPort = rpPort;
        rpPort = rpPort->addSubPort();
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterConnect(Port *pSubPort, Port *pMultiPort, const bool wasSucess)
{
    if (pMultiPort != 0)
    {
        if (wasSucess)
        {
            //! @todo What do we need to do to handle sucess
        }
        else
        {
            //We need to remove the last created subport
            pMultiPort->removeSubPort(pSubPort);
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterDissconnect(Port *&rpSubPort, Port *pMultiPort, const bool wasSucess)
{
    if (pMultiPort != 0)
    {
        if (wasSucess)
        {
            //If sucessful we should remove the empty port
            pMultiPort->removeSubPort(rpSubPort); //! @todo maybe should set the pointer to 0 inside when deleted, need ref to ptr or ptr ptr
            rpSubPort = pMultiPort; //We copy the multiport pointer back to the support pointer to make sure that it is still working
        }
        else
        {
            //! @todo What do we need to do to handle failure, nothing maybe
        }
    }
}


void ConnectionAssistant::ifMultiportPrepareForDissconnect(Port *&rpPort1, Port *&rpPort2, Port *&rpMultiPort1, Port *&rpMultiPort2)
{
    //First make usre that multiport pointers are zero if no multiports are beeing connected
    rpMultiPort1=0;
    rpMultiPort2=0;

    //either port 1 or port2 is a multiport, or both are
    if (rpPort1->getPortType() >= Port::MULTIPORT && rpPort2->getPortType() < Port::MULTIPORT )
    {
        rpMultiPort1 = rpPort1;
        assert(rpPort2->getConnectedPorts().size() == 1);
        rpPort1 = rpPort2->getConnectedPorts()[0];
    }
    else if (rpPort1->getPortType() < Port::MULTIPORT && rpPort2->getPortType() >= Port::MULTIPORT )
    {
        rpMultiPort2 = rpPort2;
        assert(rpPort1->getConnectedPorts().size() == 1);
        rpPort2 = rpPort1->getConnectedPorts()[0];
    }
    else if (rpPort1->getPortType() >= Port::MULTIPORT && rpPort2->getPortType() >= Port::MULTIPORT )
    {
        assert("Multiport <-> Multiport disconnection has not been implemented yet Aborting!" == 0);
        //! @todo need to search around to find correct subports
    }

}


//! @brief Disconnect two ports, string version
//! @todo need to make sure that components and prots given by name exist here
bool ComponentSystem::disconnect(string compname1, string portname1, string compname2, string portname2)
{
    Component *pComp1, *pComp2;
    Port *pPort1, *pPort2;

    pComp1 = getComponent(compname1);
    pComp2 = getComponent(compname2);

    if ( (pComp1!=0) && (pComp2!=0) )
    {
        pPort1 = pComp1->getPort(portname1);
        pPort2 = pComp2->getPort(portname2);

        if ( (pComp1!=0) && (pComp2!=0) )
        {
            return disconnect(pPort1, pPort2);
        }
    }

    stringstream ss;
    ss << "Disconnect: Could not find either " << compname1 << "->" << portname1 << " or " << compname2 << "->" << portname2 << endl;
    gCoreMessageHandler.addDebugMessage(ss.str());
    return false;
}

//! Disconnects two ports and remove node if no one is using it any more
//! @todo whay about system ports they are somewaht speciall
bool ComponentSystem::disconnect(Port *pPort1, Port *pPort2)
{
    cout << "disconnecting " << pPort1->getComponentName() << " " << pPort1->getPortName() << "  and  " << pPort2->getComponentName() << " " << pPort2->getPortName() << endl;

    ConnectionAssistant disconnAssistant;
    stringstream ss;
    //! @todo some more advanced error handling (are the ports really connected to each other and such)

    if (pPort1->isConnected() && pPort2->isConnected())
    {

        //Check if non of the ports will become empty, multiports will allways return connected ports size == 0 wich is Ok in this case
        if ( (pPort1->getConnectedPorts().size() > 1) && (pPort2->getConnectedPorts().size() > 1) )
        {
            disconnAssistant.unmergeOrUnjoinConnection(pPort1, pPort2);
        }
        else if ( (pPort1->getConnectedPorts().size() > 1) || (pPort2->getConnectedPorts().size() > 1) )
        {
            //! @todo seems like we can merge this case with the one above
            disconnAssistant.unmergeOrUnjoinConnection(pPort1, pPort2);
        }
        //If both ports will become empty, or if one or both is a multiport
        else
        {
            //Handle multiports
            Port* pOriginalPort1=0, *pOriginalPort2=0;
            disconnAssistant.ifMultiportPrepareForDissconnect(pPort1, pPort2, pOriginalPort1, pOriginalPort2);

            bool sucess = disconnAssistant.deleteNodeConnection(pPort1, pPort2);

            //Handle multiport connection sucess or failure
            disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort1, pOriginalPort1, sucess);
            disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort2, pOriginalPort2, sucess);
        }

        disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort1);
        disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort2);
        //! @todo maybe incorporate the clear checks into delete node and unmerge

    }
    else
    {
        gCoreMessageHandler.addWarningMessage("In disconnect: At least one of the ports do not seem to be connected, (does nothing)");
    }

    ss << "Disconnected: {"<< pPort1->getComponent()->getName() << "::" << pPort1->getPortName() << "} and {" << pPort2->getComponent()->getName() << "::" << pPort2->getPortName() << "}";
    cout << ss.str() << endl;
    gCoreMessageHandler.addDebugMessage(ss.str(), "succesfuldisconnect");

    return true; //! @todo we should try to determine if it is really true
}



void ComponentSystem::setDesiredTimestep(const double timestep)
{
    mDesiredTimestep = timestep;
    setTimestep(timestep);
}

//void ComponentSystem::setTimestep(const double timestep)
//{
//    mTimestep = timestep;
//
//    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//    {
//        if (!(mComponentSignalptrs[s]->isComponentSystem()))
//        {
//            mComponentSignalptrs[s]->setTimestep(timestep);
//        }
//    }
//
//    //C components
//    for (size_t c=0; c < mComponentCptrs.size(); ++c)
//    {
//        if (!(mComponentCptrs[c]->isComponentSystem()))
//        {
//            mComponentCptrs[c]->setTimestep(timestep);
//        }
//    }
//
//    //Q components
//    for (size_t q=0; q < mComponentQptrs.size(); ++q)
//    {
//        if (!(mComponentQptrs[q]->isComponentSystem()))
//        {
//            mComponentQptrs[q]->setTimestep(timestep);
//        }
//    }
//}


void ComponentSystem::setTimestep(const double timestep)
{
    mTimestep = timestep;

    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (!(mComponentSignalptrs[s]->isComponentSystem()))
        {
            mComponentSignalptrs[s]->setTimestep(timestep);
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (!(mComponentCptrs[c]->isComponentSystem()))
        {
            mComponentCptrs[c]->setTimestep(timestep);
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (!(mComponentQptrs[q]->isComponentSystem()))
        {
            mComponentQptrs[q]->setTimestep(timestep);
        }
    }
}


void ComponentSystem::adjustTimestep(double timestep, vector<Component*> componentPtrs)
{
    mTimestep = timestep;

    for (size_t c=0; c < componentPtrs.size(); ++c)
    {
        if (componentPtrs[c]->isComponentSystem())
        {
            double subTs = componentPtrs[c]->mDesiredTimestep;
//cout << componentPtrs[c]->mName << ", mTimestep: "<< componentPtrs[c]->mTimestep << endl;

            //If a subsystem's timestep is larger than this sytem's
            //timestep change it to this system's timestep
            if ((subTs > timestep) || (subTs < -0.0))
            {
                subTs = timestep;
            }
            //Check that subRs is a multiple of timestep
            else// if ((timestep/subTs - floor(timestep/subTs)) > 0.00001*subTs)
            {
                //subTs should get the nearest multiple of timestep as possible,
                subTs = timestep/floor(timestep/subTs+0.5);
            }
            componentPtrs[c]->setTimestep(subTs);
//cout << componentPtrs[c]->mName << ", subTs: "<< subTs << endl;
        }
        else
        {
            componentPtrs[c]->setTimestep(timestep);
//cout << componentPtrs[c]->mName << ", timestep: "<< timestep << endl;
        }
    }
}


//! @brief Checks that everything is OK before simulation
bool ComponentSystem::isSimulationOk()
{
    //Make sure that there are no components or systems with an undefined cqs_type present
    if (mComponentUndefinedptrs.size() > 0)
    {
        //! @todo maybe list their names
        cout << "Wrong CQStype: ";
        for (size_t i=0; i<mComponentUndefinedptrs.size(); ++i)
        {
             cout << mComponentUndefinedptrs[i]->getName() << " ";
        }
        cout << endl;

        gCoreMessageHandler.addErrorMessage("There are components without correct CQS type pressent, you need to fix this before simulation");
        return false;
    }

    //Check the this systems own SystemPorts, are they connected (they must be)
    vector<Port*> ports = getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
        {
            gCoreMessageHandler.addErrorMessage("Port " + ports[i]->getPortName() + " in " + getName() + " is not connected!");
            return false;
        }
        else if( ports[i]->isConnected() )
        {
            if(ports[i]->getNodePtr()->getNumberOfPortsByType(Port::POWERPORT) == 1)
            {
                gCoreMessageHandler.addErrorMessage("Port " + ports[i]->getPortName() + " in " + getName() + " is connected to a node with only one power port!");
                return false;
            }
        }
    }

    //Check all subcomponents to make sure that all requirements for simulation are met
    //scmit = The subcomponent map iterator
    SubComponentMapT::iterator scmit = mSubComponentMap.begin();
    for ( ; scmit!=mSubComponentMap.end(); ++scmit)
    {
        Component* pComp = scmit->second; //Component pointer

        //Check that ALL ports that MUST be connected are connected
        vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
            {
                gCoreMessageHandler.addErrorMessage("Port " + ports[i]->getPortName() + " on " + pComp->getName() + " is not connected!");
                return false;
            }
            else if( ports[i]->isConnected() )
            {
                if(ports[i]->getNodePtr()->getNumberOfPortsByType(Port::POWERPORT) == 1)
                {
                    gCoreMessageHandler.addErrorMessage("Port " + ports[i]->getPortName() + " in " + getName() + " is connected to a node with only one power port!");
                    return false;
                }
            }
        }

        //Recures testing into subsystems
        if (pComp->isComponentSystem())
        {
            if (!pComp->isSimulationOk())
            {
                return false;
            }
        }

        //! @todo check that all C-component required ports are connected to Q-component ports

        //! @todo check more stuff
    }

    return true;
}

//! @brief Load start values by telling each component to load their start values
void ComponentSystem::loadStartValues()
{
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
}


//! @brief Load start values from last simulation to start value container
void ComponentSystem::loadStartValuesFromSimulation()
{
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
}


//! Initializes a system component and all its contained components, also allocates log data memory
void ComponentSystem::initialize(const double startT, const double stopT, const size_t nSamples)
{
    cout << "Initializing SubSystem: " << this->mName << endl;
    mStop = false; //This variable can not be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    //preAllocate local logspace
    this->preAllocateLogSpace(startT, stopT, nSamples);

    adjustTimestep(mTimestep, mComponentSignalptrs);
    adjustTimestep(mTimestep, mComponentCptrs);
    adjustTimestep(mTimestep, mComponentQptrs);

    this->sortSignalComponentVector();

    loadStartValues();

    //Init
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (mStop)
            break;

        if (mComponentSignalptrs[s]->isComponentSystem())
        {
            mComponentSignalptrs[s]->initialize(startT, stopT, nSamples);
        }
        else
        {
            mComponentSignalptrs[s]->initialize();
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (mStop)
            break;

        if (mComponentCptrs[c]->isComponentSystem())
        {
            mComponentCptrs[c]->initialize(startT, stopT, nSamples);
        }
        else
        {
            mComponentCptrs[c]->initialize();
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (mStop)
            break;

        if (mComponentQptrs[q]->isComponentSystem())
        {
            mComponentQptrs[q]->initialize(startT, stopT, nSamples);
        }
        else
        {
            mComponentQptrs[q]->initialize();
        }
    }
}


#ifdef USETBB
class taskQ
{
    vector<Component*> vectorQ;
public:
    taskQ(vector<Component*> inputVector, double startTime, double stopTime) : vectorQ(inputVector)
    {
        mStartTime = startTime;
        mStopTime = stopTime;
    }
    void operator() () const
    {
        for(size_t i=0; i<vectorQ.size(); ++i)
        {
            vectorQ[i]->simulate(mStartTime, mStopTime);
        }
    }
private:
    double mStartTime;
    double mStopTime;
};

class taskC
{
    vector<Component*> vectorC;
public:
    taskC(vector<Component*> inputVector, double startTime, double stopTime) : vectorC(inputVector)
    {
        mStartTime = startTime;
        mStopTime = stopTime;
    }
    void operator() () const
    {
        for(size_t i=0; i<vectorC.size(); ++i)
        {
            vectorC[i]->simulate(mStartTime, mStopTime);
        }
    }
private:
    double mStartTime;
    double mStopTime;
};


//! @brief Class for slave simlation threads, which must be syncronized from a master simulation thread
class taskSimSlave
{
public:
    //! @brief Constructor for simulation thread class.
    //! @param sVector Vector with signal components executed from this thread
    //! @param cVector Vector with C-type components executed from this thread
    //! @param qVector Vector with Q-type components executed from this thread
    //! @param nVector Vector with nodes which is logged from this thread
    //! @param startTime Start time of simulation
    //! @param timeStep Step time of simulation
    //! @param stopTime Stop Time of simulation
    //! @param nCores Number of threads used in simulation
    //! @param coreN Number of this thread
    //! @param *pBarrier_s Pointer to barrier before executing signal components (atomic)
    //! @param *pBarrier_c Pointer to barrier before executing C-type components (atomic)
    //! @param *pBarrier_q Pointer to barrier before executing Q-type components (atomic)
    //! @param *pBarrier_n Pointer to barrier before executing node logging (atomic)
    //! @param *pLock_s Pointer to lock boolean used before executing signal components (atomic)
    //! @param *pLock_c Pointer to lock boolean used before executing C-type components (atomic)
    //! @param *pLock_q Pointer to lock boolean used before executing Q-type components (atomic)
    //! @param *pLock_n Pointer to lock boolean used before executing node logging (atomic)

    taskSimSlave(vector<Component*> sVector, vector<Component*> cVector, vector<Component*> qVector, vector<Node*> nVector,
                 double startTime, double timeStep, double stopTime, size_t nCores, size_t coreN,
                 tbb::atomic<size_t> *pBarrier_s, tbb::atomic<size_t> *pBarrier_c, tbb::atomic<size_t> *pBarrier_q, tbb::atomic<size_t> *pBarrier_n,
                 tbb::atomic<bool> *pLock_s, tbb::atomic<bool> *pLock_c, tbb::atomic<bool> *pLock_q, tbb::atomic<bool> *pLock_n)
    {
        mVectorS = sVector;
        mVectorC = cVector;
        mVectorQ = qVector;
        mVectorN = nVector;
        mTime = startTime;
        mStopTime = stopTime;
        mTimeStep = timeStep;
        mnCores = nCores;
        mCoreN = coreN;
        mpBarrier_s = pBarrier_s;
        mpBarrier_c = pBarrier_c;
        mpBarrier_q = pBarrier_q;
        mpBarrier_n = pBarrier_n;
        mpLock_s = pLock_s;
        mpLock_c = pLock_c;
        mpLock_q = pLock_q;
        mpLock_n = pLock_n;
    }

    //! @brief Executable code for slave simulation thread
    void operator() ()
    {
        while(mTime < mStopTime)
        {

            //! Signal Components !//

            ++(*mpBarrier_s);
            while(*mpLock_s){}

            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime, mTime+mTimeStep);
            }


            //! C Components !//

            ++(*mpBarrier_c);
            while(*mpLock_c){}

            for(size_t i=0; i<mVectorC.size(); ++i)
            {
                mVectorC[i]->simulate(mTime, mTime+mTimeStep);
            }


            //! Q Components !//

            ++(*mpBarrier_q);
            while(*mpLock_q){}

            for(size_t i=0; i<mVectorQ.size(); ++i)
            {
                mVectorQ[i]->simulate(mTime, mTime+mTimeStep);
            }


            //! Log Nodes !//

            ++(*mpBarrier_n);
            while(*mpLock_n){}

            for(size_t i=0; i<mVectorN.size(); ++i)
            {
                mVectorN[i]->logData(mTime);
            }

            mTime += mTimeStep;
        }
    }
private:
    vector<Component*> mVectorS;
    vector<Component*> mVectorC;
    vector<Component*> mVectorQ;
    vector<Node*> mVectorN;
    double mStopTime;
    double mTimeStep;
    double mTime;
    double *mpSimTime;
    size_t mnCores;
    size_t mCoreN;
    tbb::atomic<size_t> *mpBarrier_s;
    tbb::atomic<size_t> *mpBarrier_c;
    tbb::atomic<size_t> *mpBarrier_q;
    tbb::atomic<size_t> *mpBarrier_n;
    tbb::atomic<bool> *mpLock_s;
    tbb::atomic<bool> *mpLock_c;
    tbb::atomic<bool> *mpLock_q;
    tbb::atomic<bool> *mpLock_n;
};


//! @brief Class for master simulation thread, that is responsible for syncronizing the simulation
class taskSimMaster
{
public:

    //! @brief Constructor for master simulation thead class.
    //! @param sVector Vector with signal components executed from this thread
    //! @param cVector Vector with C-type components executed from this thread
    //! @param qVector Vector with Q-type components executed from this thread
    //! @param nVector Vector with nodes which is logged from this thread
    //! @param *pSimtime Pointer to the simulation time in the component system
    //! @param startTime Start time of simulation
    //! @param timeStep Step time of simulation
    //! @param stopTime Stop Time of simulation
    //! @param nCores Number of threads used in simulation
    //! @param coreN Number of this thread
    //! @param *pBarrier_s Pointer to barrier before executing signal components (atomic)
    //! @param *pBarrier_c Pointer to barrier before executing C-type components (atomic)
    //! @param *pBarrier_q Pointer to barrier before executing Q-type components (atomic)
    //! @param *pBarrier_n Pointer to barrier before executing node logging (atomic)
    //! @param *pLock_s Pointer to lock boolean used before executing signal components (atomic)
    //! @param *pLock_c Pointer to lock boolean used before executing C-type components (atomic)
    //! @param *pLock_q Pointer to lock boolean used before executing Q-type components (atomic)
    //! @param *pLock_n Pointer to lock boolean used before executing node logging (atomic)
    taskSimMaster(vector<Component*> sVector, vector<Component*> cVector, vector<Component*> qVector, vector<Node*> nVector, double *pSimTime,
                  double startTime, double timeStep, double stopTime, size_t nCores, size_t coreN,
                  tbb::atomic<size_t> *pBarrier_s, tbb::atomic<size_t> *pBarrier_c, tbb::atomic<size_t> *pBarrier_q, tbb::atomic<size_t> *pBarrier_n,
                  tbb::atomic<bool> *pLock_s, tbb::atomic<bool> *pLock_c, tbb::atomic<bool> *pLock_q, tbb::atomic<bool> *pLock_n)
    {
        mVectorS = sVector;
        mVectorC = cVector;
        mVectorQ = qVector;
        mVectorN = nVector;
        mpSimTime = pSimTime;
        mTime = startTime;
        mStopTime = stopTime;
        mTimeStep = timeStep;
        mnCores = nCores;
        mCoreN = coreN;
        mpBarrier_s = pBarrier_s;
        mpBarrier_c = pBarrier_c;
        mpBarrier_q = pBarrier_q;
        mpBarrier_n = pBarrier_n;
        mpLock_s = pLock_s;
        mpLock_c = pLock_c;
        mpLock_q = pLock_q;
        mpLock_n = pLock_n;
    }

    //! @brief Executable code for master simulation thread
    void operator() ()
    {
        while(mTime < mStopTime)
        {

            //! Signal Components !//

            while(*mpBarrier_s < mnCores-1) {}      //Wait for all other threads to reach signal component code
            *mpBarrier_s = 0;                       //Reset signal component barrier
            *mpLock_s = false;                      //Unlock signal component code
            *mpLock_q = true;                       //Lock Q-type component code (must be done in advance to prevent lockup

            for(size_t i=0; i<mVectorS.size(); ++i)
            {
                mVectorS[i]->simulate(mTime, mTime+mTimeStep);
            }

            //! C Components !//

            while(*mpBarrier_c < mnCores-1) {}
            *mpBarrier_c = 0;
            *mpLock_c = false;
            *mpLock_n = true;

            for(size_t i=0; i<mVectorC.size(); ++i)
            {
                mVectorC[i]->simulate(mTime, mTime+mTimeStep);
            }


            //! Q Components !//

            while(*mpBarrier_q < mnCores-1) {}
            *mpBarrier_q = 0;
            *mpLock_q = false;
            *mpLock_s = true;

            for(size_t i=0; i<mVectorQ.size(); ++i)
            {
                mVectorQ[i]->simulate(mTime, mTime+mTimeStep);
            }

            //! Log Nodes !//

            while(*mpBarrier_n < mnCores-1) {}
            *mpBarrier_n = 0;
            *mpLock_n = false;
            *mpLock_c = true;

            for(size_t i=0; i<mVectorN.size(); ++i)
            {
                mVectorN[i]->logData(mTime);
            }

            *mpSimTime = mTime;     //Update time in component system, so that progress bar can use it

            mTime += mTimeStep;
        }
    }
private:
    vector<Component*> mVectorS;
    vector<Component*> mVectorC;
    vector<Component*> mVectorQ;
    vector<Node*> mVectorN;
    double mStopTime;
    double mTimeStep;
    double mTime;
    double *mpSimTime;
    size_t mnCores;
    size_t mCoreN;
    tbb::atomic<size_t> *mpBarrier_s;
    tbb::atomic<size_t> *mpBarrier_c;
    tbb::atomic<size_t> *mpBarrier_q;
    tbb::atomic<size_t> *mpBarrier_n;
    tbb::atomic<bool> *mpLock_s;
    tbb::atomic<bool> *mpLock_c;
    tbb::atomic<bool> *mpLock_q;
    tbb::atomic<bool> *mpLock_n;
};

#endif


#ifdef USETBB
//! The system component version of simulate
void ComponentSystem::simulateMultiThreadedOld(const double startT, const double stopT)
{    
    mStop = false; //This variable can not be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.
    mTime = startT;
    double stopTsafe = stopT - mTimestep/2.0; //minus halv a timestep is here to ensure that no numerical issues occure

    logAllNodes(mTime);

        //Simulate signal components one time step, necessary because C and Q are simulated one time step bellow
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
    }

        //Simulate C and Q components one time step on single core and meassure the required time
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentCptrs[c]->setMeasuredTime(double((comp_end-comp_start).seconds()));
    }
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentQptrs[q]->setMeasuredTime(double((comp_end-comp_start).seconds()));
    }

    mTime += mTimestep;

        //Sort the components from longest to shortest time requirement (this is a bubblesort, we should probably use something faster...)
    size_t i, j;
    bool flag = true;
    Component *temp;
    for(i = 1; (i < mComponentCptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentCptrs.size()-1); ++j)
        {
            if (mComponentCptrs[j+1]->getMeasuredTime() > mComponentCptrs[j]->getMeasuredTime())
            {
                temp = mComponentCptrs[j];             //Swap elements
                mComponentCptrs[j] = mComponentCptrs[j+1];
                mComponentCptrs[j+1] = temp;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }
    flag = true;
    Component *tempQ;
    for(i = 1; (i < mComponentQptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentQptrs.size()-1); ++j)
        {
            if (mComponentQptrs[j+1]->getMeasuredTime() > mComponentQptrs[j]->getMeasuredTime())
            {
                tempQ = mComponentQptrs[j];             //Swap elements
                mComponentQptrs[j] = mComponentQptrs[j+1];
                mComponentQptrs[j+1] = tempQ;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }

        //Obtain number of processor cores from environment variable
    int nCores = 1; //At least on single core Ubuntu 10.04 getenv("NUMBER_OF_PROCESSORS") returns NULL and crash, solved by this if block
    if (getenv("NUMBER_OF_PROCESSORS")!=0)
    {
        string nCoresString = getenv("NUMBER_OF_PROCESSORS");
        nCores = atoi(nCoresString.c_str());
        std::stringstream ss;
        ss << nCores;
        gCoreMessageHandler.addDebugMessage("NUMBER_OF_PROCESSORS = " + nCoresString + ", setting to " + ss.str());
    }

        //Attempt to distribute C component equally over vectors (one for each core)
    vector< vector<Component*> > splitCVector;      //Vector containing the C vectors
    splitCVector.resize(nCores);
    size_t cCompNum=0;
    while(true)
    {
        for(int coreNumber=0; coreNumber<nCores; ++coreNumber)
        {
            if(cCompNum == mComponentCptrs.size())
                break;
            splitCVector[coreNumber].push_back(mComponentCptrs[cCompNum]);
            ++cCompNum;
        }
        if(cCompNum == mComponentCptrs.size())
            break;
    }

        //Attempt to distribute Q component equally over vectors (one for each core)
    vector< vector<Component*> > splitQVector;
    splitQVector.resize(nCores);
    size_t qCompNum=0;
    while(true)
    {
        for(int coreNumber=0; coreNumber<nCores; ++coreNumber)
        {
            if(qCompNum == mComponentQptrs.size())
                break;
            splitQVector[coreNumber].push_back(mComponentQptrs[qCompNum]);
            ++qCompNum;
        }
        if(qCompNum == mComponentQptrs.size())
            break;
    }

        //Initialize TBB routines for parallel  simulation
    tbb::task_group *c;
    tbb::task_group *q;
    c = new tbb::task_group;
    q = new tbb::task_group;

        //Execute simulation
    while ((mTime < stopTsafe) && (!mStop))
    {
        logAllNodes(mTime);

            //Simulate signal components on single core
        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
        {
            mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
        }

            //Simulate C component vectors in parallel
        for(int coreNumber=0; coreNumber<(nCores-1); ++coreNumber)
        {
            c->run(taskC(splitCVector[coreNumber], mTime, mTime+mTimestep));
        }
        for(size_t i=0; i<splitCVector[nCores-1].size(); ++i)       //Keep one of the vectors in current thread, to reduce overhead costs
        {
            splitCVector[nCores-1][i]->simulate(mTime, mTime+mTimestep);
        }
        c->wait();

            //Simulate Q component vectors in parallel
        for(int coreNumber=0; coreNumber<(nCores-1); ++coreNumber)
        {
            q->run(taskQ(splitQVector[coreNumber], mTime, mTime+mTimestep));
        }
        for(size_t i=0; i<splitQVector[nCores-1].size(); ++i)       //Keep one of the vectors in current thread, to reduce overhead costs
        {
            splitQVector[nCores-1][i]->simulate(mTime, mTime+mTimestep);
        }
        q->wait();

        mTime += mTimestep;
    }
}


void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nThreads)
{
    //! @todo Perhaps make mStop atomic?
    mStop = false; //This variable can not be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.
    mTime = startT;
    double stopTsafe = stopT - mTimestep/2.0; //Minus half a timestep is here to ensure that no numerical issues occure

    logAllNodes(mTime);

    tbb::tick_count measurement_start = tbb::tick_count::now();

        //Simulate S, C and Q components one time step on single core and meassure the required time
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentCptrs[c]->setMeasuredTime(double((comp_end-comp_start).seconds()));
    }
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        tbb::tick_count comp_start = tbb::tick_count::now();
        mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentQptrs[q]->setMeasuredTime(double((comp_end-comp_start).seconds()));
    }

    mTime += mTimestep; //First time step is finished!

        //Sort the components from longest to shortest time requirement (this is a bubblesort, we should probably use something faster...)
    size_t i, j;
    bool flag = true;
    Component *tempC;
    for(i = 1; (i < mComponentCptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentCptrs.size()-1); ++j)
        {
            if (mComponentCptrs[j+1]->getMeasuredTime() > mComponentCptrs[j]->getMeasuredTime())
            {
                tempC = mComponentCptrs[j];             //Swap elements
                mComponentCptrs[j] = mComponentCptrs[j+1];
                mComponentCptrs[j+1] = tempC;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }
    flag = true;
    Component *tempQ;
    for(i = 1; (i < mComponentQptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentQptrs.size()-1); ++j)
        {
            if (mComponentQptrs[j+1]->getMeasuredTime() > mComponentQptrs[j]->getMeasuredTime())
            {
                tempQ = mComponentQptrs[j];             //Swap elements
                mComponentQptrs[j] = mComponentQptrs[j+1];
                mComponentQptrs[j+1] = tempQ;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }

        //Obtain number of processor cores from environment variable, or use user specified value if not zero
    size_t nCores;
    size_t nSystemCores;
    if(getenv("NUMBER_OF_PROCESSORS") != 0)
    {
        string temp = getenv("NUMBER_OF_PROCESSORS");   //! @todo This appears to be a Windows only environment variable. Figure out how to do it on Unix (and Mac OS)
        nSystemCores = atoi(temp.c_str());
    }
    else
    {
        nSystemCores = 1;       //If Unix system, make sure there is at least one thread
    }
    if(nThreads != 0)
    {
        nCores = nThreads;
        if(nCores > nSystemCores)       //Limit number of threads to the number of system cores
        {
            nCores = nSystemCores;
        }
    }
    else
    {
        nCores = nSystemCores;
    }


        //Create vector used for time measurement (DEBUG)
    vector<double> timeVector;                                                                              //DEBUG
    timeVector.resize(nCores);                                                                              //DEBUG
    for(size_t i=0; i<nCores; ++i)                                                                          //DEBUG
    {                                                                                                       //DEBUG
        timeVector[i] = 0;                                                                                  //DEBUG
    }                                                                                                       //DEBUG

        //Attempt to distribute C component equally over vectors (one for each core)
    vector< vector<Component*> > splitCVector;
    splitCVector.resize(nCores);
    size_t cCompNum=0;
    while(true)
    {
        for(size_t coreNumber=0; coreNumber<nCores; ++coreNumber)
        {
            if(cCompNum == mComponentCptrs.size())
                break;
            splitCVector[coreNumber].push_back(mComponentCptrs[cCompNum]);
            timeVector[coreNumber] += mComponentCptrs[cCompNum]->getMeasuredTime();                         //DEBUG
            ++cCompNum;
        }
        if(cCompNum == mComponentCptrs.size())
            break;
    }

    for(size_t i=0; i<nCores; ++i)                                                                                              //DEBUG
    {                                                                                                                           //DEBUG
        stringstream ss;                                                                                                        //DEBUG
        ss << timeVector[i]*1000;                                                                                               //DEBUG
        gCoreMessageHandler.addDebugMessage("Creating C-type thread vector, measured time = " + ss.str() + " ms", "cvector");   //DEBUG
        timeVector[i] = 0;                                                                                                      //DEBUG
    }                                                                                                                           //DEBUG

        //Attempt to distribute Q component equally over vectors (one for each core)
    vector< vector<Component*> > splitQVector;
    splitQVector.resize(nCores);
    size_t qCompNum=0;
    while(true)
    {
        for(size_t coreNumber=0; coreNumber<nCores; ++coreNumber)
        {
            if(qCompNum == mComponentQptrs.size())
                break;
            splitQVector[coreNumber].push_back(mComponentQptrs[qCompNum]);
            timeVector[coreNumber] += mComponentQptrs[qCompNum]->getMeasuredTime();                         //DEBUG
            ++qCompNum;
        }
        if(qCompNum == mComponentQptrs.size())
            break;
    }

    for(size_t i=0; i<nCores; ++i)                                                                                              //DEBUG
    {                                                                                                                           //DEBUG
        stringstream ss;                                                                                                        //DEBUG
        ss << timeVector[i]*1000;                                                                                               //DEBUG
        gCoreMessageHandler.addDebugMessage("Creating Q-type thread vector, measured time = " + ss.str() + " ms", "qvector");   //DEBUG
        timeVector[i] = 0;                                                                                                      //DEBUG
    }                                                                                                                           //DEBUG

        //Distribute node pointers equally over vectors (no sorting necessary)
    vector< vector<Node*> > splitNodeVector;
    for(size_t c=0; c<nCores; ++c)
    {
        vector<Node*> tempVector;
        splitNodeVector.push_back(tempVector);
    }
    size_t currentCore = 0;
    for(size_t n=0; n<mSubNodePtrs.size(); ++n)
    {
        splitNodeVector.at(currentCore).push_back(mSubNodePtrs.at(n));
        ++currentCore;
        if(currentCore>nCores-1)
            currentCore = 0;
    }


    tbb::tick_count measurement_end = tbb::tick_count::now();
    stringstream ss;                                                                                                        //DEBUG
    ss << (double)((measurement_end-measurement_start).seconds());                                                                                               //DEBUG
    gCoreMessageHandler.addDebugMessage("Measurement time = " + ss.str() + " ms");   //DEBUG



        //Initialize TBB routines for parallel  simulation
    tbb::task_group *coreTasks;
    coreTasks = new tbb::task_group;

        //Initialize barriers
    static tbb::atomic<size_t> barrier_s;
    static tbb::atomic<size_t> barrier_c;
    static tbb::atomic<size_t> barrier_q;
    static tbb::atomic<size_t> barrier_n;
    barrier_s = 0;
    barrier_c = 0;
    barrier_q = 0;
    barrier_n = 0;

        //Initialize locks
    static tbb::atomic<bool> lock_s;
    static tbb::atomic<bool> lock_c;
    static tbb::atomic<bool> lock_q;
    static tbb::atomic<bool> lock_n;
    lock_s = true;      //Lock the locks initially, to make sure nothing goes wrong
    lock_c = true;
    lock_q = true;
    lock_n = true;

        //! @todo Make a better solution to this; we must decide if it shall be possible or not to simulate without sorting the signal components
    std::vector<Component*> dummySignalVector;   //This is used because signal components shall be simulated single-threaded (to make sure they are simulated in correct order).

        //Execute simulation
    //coreTasks->run(taskSimMaster(splitSVector[0], splitCVector[0], splitQVector[0], splitNodeVector[0], &mTime, mTime, mTimestep, stopTsafe, nCores, 0, &barrier_s, &barrier_c, &barrier_q, &barrier_n, &lock_s, &lock_c, &lock_q, &lock_n));
    coreTasks->run(taskSimMaster(mComponentSignalptrs, splitCVector[0], splitQVector[0], splitNodeVector[0], &mTime, mTime, mTimestep, stopTsafe, nCores, 0, &barrier_s, &barrier_c, &barrier_q, &barrier_n, &lock_s, &lock_c, &lock_q, &lock_n));
    for(size_t coreNumber=1; coreNumber < nCores; ++coreNumber)
    {
        coreTasks->run(taskSimSlave(dummySignalVector, splitCVector[coreNumber], splitQVector[coreNumber], splitNodeVector[coreNumber], mTime, mTimestep, stopTsafe, nCores, coreNumber, &barrier_s, &barrier_c, &barrier_q, &barrier_n, &lock_s, &lock_c, &lock_q, &lock_n));
    }
    coreTasks->wait();

    delete(coreTasks);
}

#else
    //This overrides the multi-threaded simulation call with a single-threaded simulation if TBB is not installed.
void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t /*nThreads*/)
{
    this->simulate(startT, stopT);
}
#endif


void ComponentSystem::simulate(const double startT, const double stopT)
{
    mStop = false; //This variable can not be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    mTime = startT;

    //Simulate
    double stopTsafe = stopT - mTimestep/2.0; //minus halv a timestep is here to ensure that no numerical issues occure

    while ((mTime < stopTsafe) && (!mStop))
    {
        //! @todo maybe use iterators instead
        //Signal components
        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
        {
            mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
        }

        //C components
        for (size_t c=0; c < mComponentCptrs.size(); ++c)
        {
            mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
        }

        //Q components
        for (size_t q=0; q < mComponentQptrs.size(); ++q)
        {
            mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
        }

        logAllNodes(mTime);

        mTime += mTimestep;
    }
}


//! Finalizes a system component and all its contained components
void ComponentSystem::finalize(const double startT, const double stopT)
{
    //! @todo take the final simulation step is suitable here

    //Finalize
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (mComponentSignalptrs[s]->isComponentSystem())
        {
            mComponentSignalptrs[s]->finalize(startT, stopT);
        }
        else
        {
            mComponentSignalptrs[s]->finalize();
        }
        mComponentSignalptrs[s]->secretFinalize();
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (mComponentCptrs[c]->isComponentSystem())
        {
            mComponentCptrs[c]->finalize(startT, stopT);
        }
        else
        {
            mComponentCptrs[c]->finalize();
        }
        mComponentCptrs[c]->secretFinalize();
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (mComponentQptrs[q]->isComponentSystem())
        {
            mComponentQptrs[q]->finalize(startT,stopT);
        }
        else
        {
            mComponentQptrs[q]->finalize();
        }
        mComponentQptrs[q]->secretFinalize();
    }

    //loadStartValuesFromSimulation();
}


//ComponentFactory hopsan::gCoreComponentFactory;
//DLLIMPORTEXPORT ComponentFactory* hopsan::getCoreComponentFactoryPtr()
//{
//    return &gCoreComponentFactory;
//}
