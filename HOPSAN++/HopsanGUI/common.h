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

//Development defines, swich commen block with release++ bellow on real release
#define MAINPATH "../../"
#define OBJECTICONPATH "../../HopsanGUI/graphics/objecticons/"
#define ICONPATH "../../HopsanGUI/graphics/uiicons/"
#define PORTICONPATH "../../HopsanGUI/graphics/porticons/"
#define GRAPHICSPATH "../../HopsanGUI/graphics/"
#define COMPONENTPATH "../../HopsanGUI/componentData/"
#define MODELPATH "/../../Models/"

//Relese++ defines
//#define MAINPATH "./"
//#define OBJECTICONPATH ":graphics/objecticons/"
//#define ICONPATH ":graphics/uiicons/"
//#define PORTICONPATH ":graphics/porticons/"
//#define GRAPHICSPATH ":graphics/"
//#define COMPONENTPATH "componentData/"
//#define MODELPATH "Models/"

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
