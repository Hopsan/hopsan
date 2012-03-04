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

//Development flag
#define DEVELOPMENT

//Numerical definitions
#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10

//Web link definitions
#define HOPSANLINK "http://tiny.cc/hopsan"
#define NEWSLINK "http://tiny.cc/hopsannews"
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
#define MAINPATH "../"
#define HELPPATH "../doc/user/html/"
#define MSVC2008PATH "MSVC2008/"
#define MSVC2010PATH "MSVC2010/"

// windows specific paths
#ifdef WIN32
#define DATAPATH QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/"
#define DOCSPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/"
#else
#define DATAPATH gExecPath+MAINPATH
#define DOCSPATH gExecPath+MAINPATH
#endif

// Paths that needs two alternatives
#define MODELS_DEV_PATH gExecPath+"../Models/"
#define SCRIPTS_DEV_PATH gExecPath+"../Scripts/"
#define MODELS_REL_PATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Models/"
#define SCRIPTS_REL_PATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Scripts/"

// Paths that have one alternative depending on development OR release mode
#ifdef DEVELOPMENT
#define BACKUPPATH gExecPath+"../Backup/"
#define COMPONENTPATH gExecPath+"../componentLibraries/defaultLibrary/components/"
#define INCLUDEPATH gExecPath+"../HopsanCore/include/"
#else
#define BACKUPPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Backup/"
#define COMPONENTPATH gExecPath+"../components/"
#define INCLUDEPATH gExecPath+"../include/"
#endif

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
