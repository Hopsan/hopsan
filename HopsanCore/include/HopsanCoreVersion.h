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

#ifndef VERSION_H
#define VERSION_H

#include "HopsanCoreMacros.h"

// We need to use this include because external dependencies will need the revision of the core when it was compiled last time,
// not the latest revision that you get when compiling the external component.
#include "HopsanCoreSVNRevision.h"

#define HOPSANCOREVERSION "0.8.x_r" TO_STR(HOPSANCORESVNREVISION)
#define HOPSANCOREMODELFILEVERSION "0.4"

#ifdef DEBUGCOMPILING
 #define DEBUGRELEASECOMPILED "DEBUG"
#elif defined  RELEASECOMPILING
 #define DEBUGRELEASECOMPILED "RELEASE"
#else
 //#warning You must specify Debug or Release compiling by defining DEBUGCOMPILING or RELEASECOMPILING
 #define DEBUGRELEASECOMPILED "UNDEFINED"
#endif

// Include compiler info macros
#include "compiler_info.h"

#endif // VERSION_H
