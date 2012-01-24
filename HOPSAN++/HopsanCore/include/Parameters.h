/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   Parameters.h
//! @author FluMeS
//! @date   2012-01-05
//! @brief Contains the parameter and parameters classes
//!
//$Id$

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "win32dll.h"
#include <string>
#include <vector>

namespace hopsan {

//Forward declaration
class Component;
class Parameters;

class Parameter
{
public:
    Parameter(std::string parameterName, std::string parameterValue, std::string description, std::string unit, std::string type, void* pDataPtr=0, Parameters* parentParameters=0);
    bool setParameterValue(const std::string value, Parameter **pNeedEvaluation=0);
    bool setParameter(std::string parameterValue, std::string description, std::string unit, std::string type, Parameter **pNeedEvaluation=0, bool force=0);
    void getParameter(std::string &rParameterName, std::string &rParameterValue, std::string &rDescription, std::string &rUnit, std::string &rType);
    bool evaluate(std::string &rResult, Parameter *ignoreMe=0);
    bool evaluate();

    std::string getType() const;
    std::string getName() const;
    std::string getValue() const;

protected:
    std::string mParameterName;
    std::string mParameterValue;
    std::string mDescription;
    std::string mUnit;
    std::string mType;
    void* mpData;
    Parameters* mpParentParameters;
};


class DLLIMPORTEXPORT Parameters
{
public:
    Parameters(Component* parentComponent);
    bool addParameter(std::string parameterName, std::string parameterValue, std::string description="", std::string unit="", std::string type="", void* dataPtr=0, bool force=false);
    void deleteParameter(std::string parameterName);
    void getParameters(std::vector<std::string> &rParameterNames, std::vector<std::string> &rParameterValues, std::vector<std::string> &rDescriptions, std::vector<std::string> &rUnits, std::vector<std::string> &rTypes);
    void getParameterValue(const std::string name, std::string &rValue);
    bool setParameter(std::string name, std::string value, std::string description="", std::string unit="", std::string type="", bool force=0);
    bool setParameterValue(const std::string name, const std::string value, bool force=0);
    bool evaluateParameter(const std::string parameterName, std::string &rEvaluatedParameterValue, const std::string type, Parameter *ignoreMe=0);
    bool evaluateParameters();
    bool exist(std::string parameterName);
    bool checkParameters(std::string &errParName);

protected:
    std::vector<Parameter*> mParameters;
    Component* mParentComponent;
    std::vector<Parameter*> mParametersNeedEvaluation; //! @todo Use this vector to ensure parameters are valid at simulation time e.g. if a used system parameter is deleted before simulation
};

}

#endif // PARAMETERS_H
