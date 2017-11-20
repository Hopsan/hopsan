/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   version_gui.h
//!
//! @brief Contains version definitions for the HopsanGUI, HMF files and component appearance files
//!
//$Id$
#ifndef VERSION_GUI_H
#define VERSION_GUI_H

// Include compiler info and hopsan core macros
#include "HopsanCoreVersion.h"

// If we don't have the revision number then define UNKNOWN
// On real release  builds, UNKNOWN will be replaced by actual revnum by external script
#ifndef HOPSANGUI_COMMIT_TIMESTAMP
#define HOPSANGUI_COMMIT_TIMESTAMP UNKNOWN
#endif

#define HOPSANGUIVERSION HOPSANBASEVERSION "." TO_STR(HOPSANGUI_COMMIT_TIMESTAMP)
// The actual Hopsan release version will be set by external script
#define HOPSANRELEASEVERSION HOPSANGUIVERSION

#define HMF_VERSIONNUM "0.4"
#define HMF_REQUIREDVERSIONNUM "0.3"
#define CAF_VERSIONNUM "0.3"

#endif // VERSION_GUI_H
