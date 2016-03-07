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
//! @file   version_gui.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains version definitions for the HopsanGUI, HMF files and component appearance files
//!
//$Id$
#ifndef VERSION_GUI_H
#define VERSION_GUI_H

// Include compiler info and hopsan core macros
#include "compiler_info.h"

// If we don't have the revision number then define UNKNOWN
// On real release  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANGUISVNREVISION
 #define HOPSANGUISVNREVISION UNKNOWN
#endif

#define HOPSANGUIVERSION "0.8.x_r" TO_STR(HOPSANGUISVNREVISION)
#define HMF_VERSIONNUM "0.4"
#define HMF_REQUIREDVERSIONNUM "0.3"
#define CAF_VERSIONNUM "0.3"



#endif // VERSION_GUI_H
