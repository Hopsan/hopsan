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

//Path definitions (development and release)
#define MAINPATH "../"
#define OBJECTICONPATH ":graphics/objecticons/"
#define ICONPATH ":graphics/uiicons/"
#define PORTICONPATH ":graphics/porticons/"
#define GRAPHICSPATH ":graphics/"
#define SOUNDSPATH ":sounds/"
#define HELPPATH "../doc/user/html/"
#ifdef WIN32
#define DATAPATH QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/"
#define DOCSPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/"
#else
#define DATAPATH MAINPATH
#define DOCSPATH MAINPATH
#endif
#ifdef DEVELOPMENT
#define MODELPATH "../Models/"
#define SCRIPTPATH "../Scripts/"
#define BACKUPPATH "../Backup/"
#define COMPONENTPATH "../componentLibraries/defaultLibrary/components/"
#define INCLUDEPATH "../HopsanCore/include/"
#else
#define MODELPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Models/"
#define SCRIPTSPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Scripts/"
#define BACKUPPATH QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/Backup/"
#define COMPONENTPATH "../componentData/"
#define INCLUDEPATH "../Include/"
#endif

//Gui TypeName defines
#define HOPSANGUIGROUPTYPENAME "HopsanGUIGroup"
#define HOPSANGUISYSTEMTYPENAME "Subsystem"
#define HOPSANGUICONTAINERPORTTYPENAME "HopsanGUIContainerPort"

//! @todo all of these dont have to be common
//Enums
enum selectionStatus {DESELECTED, SELECTED};
enum nameVisibility {NAMEVISIBLE, NAMENOTVISIBLE, USEDEFAULT};
enum graphicsType {USERGRAPHICS, ISOGRAPHICS};
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

//Global pointer to the main window
class MainWindow;
extern MainWindow* gpMainWindow;

class Configuration;
extern Configuration gConfig;

class CopyStack;
extern CopyStack gCopyStack;

extern QString gExecPath;

#endif // COMMON_H
