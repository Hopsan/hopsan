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

#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10
#define HOPSANLINK "http://tiny.cc/hopsan"
#define NEWSLINK "http://tiny.cc/hopsannews"
#define DOWNLOADLINK "http://tiny.cc/hopsanarchive"


//Development defines, swich commen block with release++ bellow on real release
#define MAINPATH "../"
#define OBJECTICONPATH "../HopsanGUI/graphics/objecticons/"
#define ICONPATH "../HopsanGUI/graphics/uiicons/"
#define PORTICONPATH "../HopsanGUI/graphics/porticons/"
#define GRAPHICSPATH "../HopsanGUI/graphics/"
#define SOUNDSPATH "../HopsanGUI/sounds/"
#define COMPONENTPATH "../HopsanGUI/componentData/"
#define MODELPATH "../Models/"
#define HELPPATH "../HopsanGUI/docs/html/"
#ifdef WIN32
#define DATAPATH QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/"
#else
#define DATAPATH gExecPath
#endif

//Relese++ defines
//#define MAINPATH "../"
//#define OBJECTICONPATH ":graphics/objecticons/"
//#define ICONPATH ":graphics/uiicons/"
//#define PORTICONPATH ":graphics/porticons/"
//#define GRAPHICSPATH ":graphics/"
//#define SOUNDSPATH ":sounds/"
//#define COMPONENTPATH "../componentData/"
//#define MODELPATH "../Models/"
//#define HELPPATH "../Help/"
//#ifdef WIN32
//#define DATAPATH QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/"
//#else
//#define DATAPATH gExecPath
//#endif

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

//Gui TypeName defines
#define HOPSANGUIGROUPTYPENAME "HopsanGUIGroup"
#define HOPSANGUISYSTEMTYPENAME "Subsystem"
#define HOPSANGUICONTAINERPORTTYPENAME "HopsanGUIContainerPort"

//Global pointer to the main window
class MainWindow;
extern MainWindow* gpMainWindow;

class Configuration;
extern Configuration gConfig;

class CopyStack;
extern CopyStack gCopyStack;

extern QString gExecPath;

#endif // COMMON_H
