/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

class DLLIMPORTEXPORT ParameterEvaluator
{
    friend class ParameterEvaluatorHandler;
public:
    ParameterEvaluator(const HString &rName, const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit,
                       const HString &rType, void* pDataPtr=0, ParameterEvaluatorHandler* parentParameters=0);

    bool setParameterValue(const HString &rValue, ParameterEvaluator **ppNeedEvaluation=0);
    bool setParameter(const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit,
                      const HString &rType, ParameterEvaluator **pNeedEvaluation=0, bool force=false);

    bool evaluate(HString &rResult, ParameterEvaluator *ignoreMe=0);
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


class DLLIMPORTEXPORT ParameterEvaluatorHandler
{
public:
    ParameterEvaluatorHandler(Component* parentComponent);
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

    bool evaluateParameter(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType, ParameterEvaluator *ignoreMe=0);
    bool evaluateParameters();
    bool refreshParameterValueText(const HString &rParameterName);

    bool evaluateInSystemParent(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType);
    bool evaluateParameterExpression(const HString &rExpression, HString &rEvaluatedParameterValue);

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
