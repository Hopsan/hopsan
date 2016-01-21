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

//$Id$

#ifndef NUMHOPHELPER_H
#define NUMHOPHELPER_H

#include "HopsanTypes.h"
#include "win32dll.h"

namespace hopsan {

class Component;
class ComponentSystem;
class NumHopHelperPrivate;

class DLLIMPORTEXPORT NumHopHelper
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

private:
    ComponentSystem *mpSystem;
    Component *mpComponent;
    NumHopHelperPrivate *mpPrivate;
};

}

#endif // NUMHOPHELPER_H
