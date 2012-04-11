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

#include <QString>

#ifndef COMMON_H
#define COMMON_H

//Numerical definitions
#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10

//Web link definitions
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

// common paths
#define MAINPATH gExecPath+"../"
#define HELPPATH gExecPath+"../doc/user/html/"
#define COMPONENTSPATH gExecPath+"../componentLibraries/defaultLibrary/components/"
#define COREINCLUDEPATH gExecPath+"../HopsanCore/include/"
#define MSVC2008_X86_PATH gExecPath+"MSVC2008_x86/"
#define MSVC2010_X86_PATH gExecPath+"MSVC2010_x86/"
#define MSVC2008_X64_PATH gExecPath+"MSVC2008_x64/"
#define MSVC2010_X64_PATH gExecPath+"MSVC2010_x64/"

#define DATAPATH QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/"
#define DOCUMENTSPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/"
#define BACKUPPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Backup/"

// Paths that needs two alternatives, DEV = development, REL = release
#define MODELS_DEV_PATH gExecPath+"../Models/"
#define SCRIPTS_DEV_PATH gExecPath+"../Scripts/"
#define MODELS_REL_PATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Models/"
#define SCRIPTS_REL_PATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Scripts/"

// Gui TypeName defines
#define HOPSANGUIGROUPTYPENAME "HopsanGUIGroup"
#define HOPSANGUISYSTEMTYPENAME "Subsystem"
#define HOPSANGUICONTAINERPORTTYPENAME "HopsanGUIContainerPort"

//! @todo all of these dont have to be common
//! @todo enums should NOT have all captitals, risc of colission with defines
//Enums
enum selectionStatus {DESELECTED, SELECTED};
enum nameVisibility {NAMEVISIBLE, NAMENOTVISIBLE, USEDEFAULT};
enum graphicsType {USERGRAPHICS, ISOGRAPHICS, NOGRAPHICS};
enum connectorGeometry {VERTICAL, HORIZONTAL, DIAGONAL};
enum renameRestrictions {UNRESTRICTED, CORERENAMEONLY};
enum undoStatus {NOUNDO, UNDO};
enum saveTarget {EXISTINGFILE, NEWFILE};
enum simulationMethod {SINGLECORE, MULTICORE};
enum connectorStyle {POWERCONNECTOR, SIGNALCONNECTOR, UNDEFINEDCONNECTOR};
enum HopsanPlotID {FIRSTPLOT, SECONDPLOT};
enum HopsanPlotCurveType {PORTVARIABLE, FREQUENCYANALYSIS, NYQUIST, BODEGAIN, BODEPHASE, GENERAL};
enum ContainerChildrenZValues {WIDGET_Z, CONNECTOR_Z, MODELOBJECT_Z, HOVEREDCONNECTOR_Z, HOVEREDMODELOBJECT_Z};
enum ModelObjectZValues {PORT_Z, SELECTIONBOX_Z, HOVEREDPORT_Z, LOSSESDISPLAY_Z};
enum PortZValues {MULTIPORTOVERLAY_Z, CQSVERLAY_Z, PORTLABEL_Z};
enum YesNoToAllEnumT {UNDECIDED_TO_ALL, NO_TO_ALL, YES_TO_ALL};
enum InternalExternalEnumT {INTERNAL, EXTERNAL};

//Global pointer to the main window
class MainWindow;
extern MainWindow* gpMainWindow;

class Configuration;
extern Configuration gConfig;

class CopyStack;
extern CopyStack gCopyStack;

extern QString gExecPath;
extern QString gModelsPath;
extern QString gScriptsPath;

#endif // COMMON_H
