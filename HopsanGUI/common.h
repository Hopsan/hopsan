/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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
#define VERSIONLINK "http://tiny.cc/hopsannews"
#define NEWSLINK "http://tiny.cc/hopsannewsfeed"
#define DOWNLOADLINK "http://tiny.cc/hopsanarchive"
#define AUTOUPDATELINK "http://tiny.cc/hopsanupdate"

// Path definitions (development and release)
// qrc paths
#define OBJECTICONPATH ":graphics/objecticons/"
#define BUILTINCAFPATH ":graphics/builtinCAF/"
#define ICONPATH ":graphics/uiicons/"
#define PORTICONPATH ":graphics/porticons/"
#define GRAPHICSPATH ":graphics/"
#define SOUNDSPATH ":sounds/"

// Gui TypeName defines
#define HOPSANGUIGROUPTYPENAME "HopsanGUIGroup"
#define HOPSANGUISYSTEMTYPENAME "Subsystem"
#define HOPSANGUICONDITIONALSYSTEMTYPENAME "ConditionalSubsystem"
#define HOPSANGUICONTAINERPORTTYPENAME "HopsanGUIContainerPort"
#define HOPSANGUISCOPECOMPONENTTYPENAME "SignalSink"

#ifdef _WIN32
#define LIBEXT ".dll"
#define LIBPREFIX ""
#else
#define LIBEXT ".so"
#define LIBPREFIX "lib"
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

extern const char* getHopsanGUIBuildTime();
extern bool isHospanGUIVersionHigherThan(const char*);

#endif // COMMON_H
