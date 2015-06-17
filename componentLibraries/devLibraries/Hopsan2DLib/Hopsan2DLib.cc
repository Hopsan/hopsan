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

// Include headers from HopsanCore
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

// Include component code files
#include "Hopsan2DBody2.hpp"
#include "Hopsan2DBodyTest.hpp"
#include "Hopsan2DJoint.hpp"
#include "Hopsan2DFixedAttachment.hpp"
#include "Hopsan2DForceTorqueSource.hpp"

using namespace hopsan;

//Register components
extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    // Register Components
    pComponentFactory->registerCreatorFunction("Hopsan2DBody2", Hopsan2DBody2::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DBodyTest", Hopsan2DBodyTest::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DJoint", Hopsan2DJoint::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DFixedAttachment", Hopsan2DFixedAttachment::Creator);
    pComponentFactory->registerCreatorFunction("Hopsan2DForceTorqueSource", Hopsan2DForceTorqueSource::Creator);

    // Register Custom Nodes (not yet supported)
    HOPSAN_UNUSED(pNodeFactory);
}

//Provide library information for Hopsan
extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    pHopsanExternalLibInfo->libName = (char*)"My2DTestLib";

    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;
}
