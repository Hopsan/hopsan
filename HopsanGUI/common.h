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
//! @file common.h
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
namespace hopsanweblinks {
    constexpr auto homepage = "http://tiny.cc/hopsan";
    constexpr auto news = "http://tiny.cc/hopsannewsfeed";
    constexpr auto releases = "http://tiny.cc/hopsanreleases";
    constexpr auto releases_archive = "http://tiny.cc/hopsanarchive";
    constexpr auto tutorials = "https://hopsan.github.io/tutorials";
}

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
#define HOPSANGUISYSTEMPORTTYPENAME "HopsanGUISystemPort"
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
enum SaveContentsEnumT {FullModel, ParametersOnly};
enum ConnectorStyleEnumT {PowerConnectorStyle, SignalConnectorStyle, BrokenConnectorStyle, UndefinedConnectorStyle};
enum ContainerChildrenZValuesEnumT {WidgetZValue, ConnectorZValue, ModelobjectZValue, HoveredConnectorZValue, HoveredModelobjectZValue, BrokenConnectorZValue};
enum ModelObjectZValuesEnumT {PortZValue, SelectionboxZValue, HoveredPortZValue, LossesDisplayZValue};
enum PortZValuesEnumT {MultiportOverlayZValue, CQSOverlayZValue, PortLabelZValue};
enum YesNoToAllEnumT {UndecidedToAll, NoToAll, YesToAll};
enum LibraryTypeEnumT {AnyLib, InternalLib, ExternalLib, FmuLib};
enum HiddenVisibleEnumT {Hidden, Visible};
enum EnabledDisabledEnumT {Enabled, Disabled};
enum LocklevelEnumT {NotLocked, LimitedLock, FullyLocked};
enum class ArchitectureEnumT {x86, x64};
enum DeleteOrKeepFilesEnumT {KeepFiles, DeleteFiles};
enum RecompileEnumT {NoRecompile, Recompile};
enum WindowingFunctionEnumT {RectangularWindow, HannWindow, FlatTopWindow};
enum FrequencySpectrumEnumT {Undefined, PowerSpectrum, EnergySpectrum, RMSSpectrum};

extern const char* getHopsanGUIBuildTime();

#endif // COMMON_H
