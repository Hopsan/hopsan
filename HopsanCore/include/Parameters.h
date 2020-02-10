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

namespace hopsan {

//Forward declaration
class Component;
class ParameterEvaluatorHandler;

class HOPSANCORE_DLLAPI ParameterEvaluator
{
    friend class ParameterEvaluatorHandler;
public:
    ParameterEvaluator(const HString &rName, const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit,
                       const HString &rType, void* pDataPtr=0, ParameterEvaluatorHandler* parentParameters=0);

    bool setParameterValue(const HString &rValue, ParameterEvaluator **ppNeedEvaluation=0);
    bool setParameter(const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit,
                      const HString &rType, ParameterEvaluator **pNeedEvaluation=0, bool force=false);

    bool evaluate(HString &rResult);
    bool evaluate();
    bool refreshParameterValueText();

    void* getDataPtr();

    const HString &getType() const;
    const HString &getName() const;
    const HString &getValue() const;
    const HString &getUnit() const;
    const HString &getDescription() const;
    const HString &getQuantity() const;
    const std::vector<HString> &getConditions() const;

protected:
    void resolveSignPrefix(HString &rSignPrefix) const;
    void splitSignPrefix(const HString &rString, HString &rPrefix, HString &rValue);

    HString mParameterName;
    HString mParameterValue;
    HString mDescription;
    HString mUnit;
    HString mQuantity;
    HString mType;
    void* mpData;
    size_t mDepthCounter;
    ParameterEvaluatorHandler* mpParentParameters;
    std::vector<HString> mConditions;
};


class HOPSANCORE_DLLAPI ParameterEvaluatorHandler
{
public:
    ParameterEvaluatorHandler(Component* pComponent);
    ~ParameterEvaluatorHandler();

    bool addParameter(const HString &rName, const HString &rValue, const HString &rDescription,
                      const HString &rQuantity, const HString &rUnit, const HString &rType,
                      void* pData=0, bool force=false, std::vector<HString> conditions = std::vector<HString>());
    void deleteParameter(const HString &rName);
    bool renameParameter(const HString &rOldName, const HString &rNewName);

    const std::vector<ParameterEvaluator*> *getParametersVectorPtr() const;
    const ParameterEvaluator* getParameter(const HString &rName) const;
    void getParameterNames(std::vector<HString> &rParameterNames);
    bool setParameter(const HString &rName, const HString &rValue, const HString &rDescription="", const HString &rQuantity="",
                      const HString &rUnit="", const HString &rType="", const bool force=false);

    void getParameterValue(const HString &rName, HString &rValue);
    bool setParameterValue(const HString &rName, const HString &rValue, bool force=false);
    void* getParameterDataPtr(const HString &rName);

    bool evaluateInLocalComponent(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType);
    bool evaluateParameters();
    bool refreshParameterValueText(const HString &rParameterName);

    bool evaluateInSystemParent(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType);
    bool evaluateParameterExpression(const HString &rExpression, HString &rEvaluatedParameterValue);

    bool hasParameter(const HString &rName) const;
    bool checkParameters(HString &rErrParName);

    Component *getComponent() const;

protected:
    Component* mComponent;
    std::vector<ParameterEvaluator*> mParameters;
    std::vector<ParameterEvaluator*> mParametersNeedEvaluation; //! @todo Use this vector to ensure parameters are valid at simulation time e.g. if a used system parameter is deleted before simulation
};

}

#endif // PARAMETERS_H
