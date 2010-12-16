//$Id$

#ifndef COMMON_H
#define COMMON_H

#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10

//Development defines
#define MAINPATH "../../"
#define OBJECTICONPATH "../../HopsanGUI/graphics/objecticons/"
#define ICONPATH "../../HopsanGUI/graphics/uiicons/"
#define PORTICONPATH "../../HopsanGUI/graphics/porticons/"
#define GRAPHICSPATH "../../HopsanGUI/graphics/"
#define COMPONENTPATH "../../HopsanGUI/componentData/"
#define MODELPATH "/../../Models/"

//Relese++ defines
//#define MAINPATH "./"
//#define OBJECTICONPATH "graphics/objecticons/"
//#define ICONPATH "graphics/uiicons/"
//#define PORTICONPATH "graphics/porticons/"
//#define GRAPHICSPATH "graphics/"
//#define COMPONENTPATH "componentData/"
//#define MODELPATH "Models/"

//Enums
//! @todo all of these should not be common
enum selectionStatus {DESELECTED, SELECTED};
enum nameVisibility {NAMEVISIBLE, NAMENOTVISIBLE};
enum graphicsType {USERGRAPHICS, ISOGRAPHICS};
enum connectorGeometry {VERTICAL, HORIZONTAL, DIAGONAL};
enum renameRestrictions {UNRESTRICTED, CORERENAMEONLY};
enum undoStatus {NOUNDO, UNDO};
enum saveTarget {EXISTINGFILE, NEWFILE};
enum simulationMethod {SINGLECORE, MULTICORE};

//Global pointer to the main window
class MainWindow;
extern MainWindow* gpMainWindow;

class Configuration;
extern Configuration gConfig;

class CopyStack;
extern CopyStack gCopyStack;

#endif // COMMON_H
