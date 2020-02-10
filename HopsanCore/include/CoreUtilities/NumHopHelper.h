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

#ifndef NUMHOPHELPER_H
#define NUMHOPHELPER_H

#include "HopsanTypes.h"
#include "win32dll.h"

namespace hopsan {

class Component;
class ComponentSystem;
class NumHopHelperPrivate;

class HOPSANCORE_DLLAPI NumHopHelper
{
public:
    NumHopHelper();
    ~NumHopHelper();

    void setSystem(ComponentSystem *pSystem);
    void setComponent(Component *pComponent);

    void registerDataPtr(const HString &name, double *pData);

    bool evalNumHopScript(const HString &script, double &rValue, bool doPrintOutput, HString &rOutput);
    bool interpretNumHopScript(const HString &script, bool doPrintOutput, HString &rOutput);
    bool eval(double &rValue, bool doPrintOutput, HString &rOutput);

    HVector<HString> extractVariableNames(const HString &expression) const;
    static HVector<HString> extractNamedValues(const HString &expression);
    static HString replaceNamedValue(const HString& expression, const HString &oldName, const HString& newName);

private:
    ComponentSystem *mpSystem;
    Component *mpComponent;
    NumHopHelperPrivate *mpPrivate;
};

}

#endif // NUMHOPHELPER_H
