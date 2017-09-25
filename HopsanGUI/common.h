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
//! @file   common.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains HopsanGUI common global definitions, enums and objects
//!
//$Id$

#ifndef COMMON_H
#define COMMON_H

// Numerical definitions
#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10
#define XMLINDENTATION 2

// Web link definitions
#define HOPSANLINK "http://tiny.cc/hopsan"
#define VERSIONLINK "http://tiny.cc/hopsanreleases"
#define NEWSLINK "http://tiny.cc/hopsannewsfeed"
#define DOWNLOADLINK "http://tiny.cc/hopsanarchive"

// Path definitions (development and release)
// qrc paths
#define OBJECTICONPATH ":graphics/objecticons/"
#define BUILTINCAFPATH ":graphics/builtinCAF/"
#define ICONPATH ":graphics/uiicons/"
#define PORTICONPATH ":graphics/porticons/"
#define GRAPHICSPATH ":graphics/"
#define SOUNDSPATH ":sounds/"

// Gui TypeName defines
#define HOPSANGUISYSTEMTYPENAME "Subsystem"
#define HOPSANGUICONDITIONALSYSTEMTYPENAME "ConditionalSubsystem"
#define HOPSANGUICONTAINERPORTTYPENAME "HopsanGUIContainerPort"
#define HOPSANGUISCOPECOMPONENTTYPENAME "SignalSink"

#ifdef _WIN32
#define LIBEXT ".dll"
#define LIBPREFIX ""
#else
#ifdef Q_OS_OSX
#define LIBEXT ".dylib"
#define LIBPREFIX "lib"
#else
#define LIBEXT ".so"
#define LIBPREFIX "lib"
#endif
#endif

//! @todo all of these don't have to be common
//Enums
enum SelectionStatusEnumT {Deselected, Selected};
enum NameVisibilityEnumT {NameVisible, NameNotVisible, UseDefault};
enum GraphicsTypeEnumT {UserGraphics, ISOGraphics, NoGraphics};
enum ConnectorGeometryEnumT {Vertical, Horizontal, Diagonal};
enum RenameRestrictionEnumT {Unrestricted, CoreRenameOnly};
enum UndoStatusEnumT {NoUndo, Undo};
enum SaveTargetEnumT {ExistingFile, NewFile};
enum SaveContentsEnumT {FullModel, ParametersOnly, CurrentContainerOnly};
enum ConnectorStyleEnumT {PowerConnectorStyle, SignalConnectorStyle, BrokenConnectorStyle, UndefinedConnectorStyle};
enum ContainerChildrenZValuesEnumT {WidgetZValue, ConnectorZValue, ModelobjectZValue, HoveredConnectorZValue, HoveredModelobjectZValue, BrokenConnectorZValue};
enum ModelObjectZValuesEnumT {PortZValue, SelectionboxZValue, HoveredPortZValue, LossesDisplayZValue};
enum PortZValuesEnumT {MultiportOverlayZValue, CQSOverlayZValue, PortLabelZValue};
enum YesNoToAllEnumT {UndecidedToAll, NoToAll, YesToAll};
enum LibraryTypeEnumT {InternalLib, ExternalLib, FmuLib};
enum HiddenVisibleEnumT {Hidden, Visible};
enum LocklevelEnumT {NotLocked, LimitedLock, FullyLocked};

extern const char* getHopsanGUIBuildTime();
extern bool isVersionHigherThanCurrentHospanGUI(const char*);

#endif // COMMON_H
