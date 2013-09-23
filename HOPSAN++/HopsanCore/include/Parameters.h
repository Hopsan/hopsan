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
#include "HopsanTypes.h"
#include <vector>
#include <string>

namespace hopsan {

//Forward declaration
class Component;
class ParameterEvaluatorHandler;

class DLLIMPORTEXPORT ParameterEvaluator
{
    friend class ParameterEvaluatorHandler;
public:
    ParameterEvaluator(const HString &rName, const HString &rValue, const HString &rDescription, const HString &rUnit,
              const HString &rType, void* pDataPtr=0, ParameterEvaluatorHandler* parentParameters=0);

    bool setParameterValue(const HString &rValue, ParameterEvaluator **pNeedEvaluation=0);
    bool setParameter(const HString &rValue, const HString &rDescription, const HString &rUnit,
                      const HString &rType, ParameterEvaluator **pNeedEvaluation=0, bool force=false);

    void setEnabled(const bool enabled);

    bool evaluate(HString &rResult, ParameterEvaluator *ignoreMe=0);
    bool evaluate();
    bool refreshParameterValueText();

    void* getDataPtr();

    const HString &getType() const;
    const HString &getName() const;
    const HString &getValue() const;
    const HString &getUnit() const;
    const HString &getDescription() const;

    bool isEnabled() const;

protected:
    void resolveSignPrefix(HString &rSignPrefix) const;
    void splitSignPrefix(const HString &rString, HString &rPrefix, HString &rValue);

    bool mEnabled;
    HString mParameterName;
    HString mParameterValue;
    HString mDescription;
    HString mUnit;
    HString mType;
    void* mpData;
    ParameterEvaluatorHandler* mpParentParameters;
    std::vector<std::string> mConditions;
};


class DLLIMPORTEXPORT ParameterEvaluatorHandler
{
public:
    ParameterEvaluatorHandler(Component* parentComponent);
    ~ParameterEvaluatorHandler();

    bool addParameter(const HString &rName, const HString &rValue, const HString &rDescription,
                      const HString &rUnit, const HString &rType, void* pData=0, bool force=false, std::vector<std::string> conditions = std::vector<std::string>());
    void deleteParameter(const HString &rName);
    bool renameParameter(const HString &rOldName, const HString &rNewName);

    void setParameterEnabled(const HString &rName, const bool enable);

    const std::vector<ParameterEvaluator*> *getParametersVectorPtr() const;
    const ParameterEvaluator* getParameter(const HString &rName) const;
    void getParameterNames(std::vector<HString> &rParameterNames);
    bool setParameter(const HString &rName, const HString &rValue, const HString &rDescription="",
                      const HString &rUnit="", const HString &rType="", const bool force=false);

    void getParameterValue(const HString &rName, HString &rValue);
    bool setParameterValue(const HString &rName, const HString &rValue, bool force=false);
    void* getParameterDataPtr(const HString &rName);

    bool evaluateParameter(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType, ParameterEvaluator *ignoreMe=0);
    bool evaluateParameters();
    bool refreshParameterValueText(const HString &rParameterName);

    bool hasParameter(const HString &rName) const;
    bool checkParameters(HString &rErrParName);

    Component *getParentComponent() const;

protected:
    Component* mParentComponent;
    std::vector<ParameterEvaluator*> mParameters;
    std::vector<ParameterEvaluator*> mParametersNeedEvaluation; //! @todo Use this vector to ensure parameters are valid at simulation time e.g. if a used system parameter is deleted before simulation
};

}

#endif // PARAMETERS_H
