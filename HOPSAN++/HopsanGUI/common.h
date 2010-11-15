#ifndef COMMON_H
#define COMMON_H

#define GOLDENRATIO 1.61803399
#define SNAPDISTANCE 10
#define MAINPATH "../../"
#define ICONPATH "../../HopsanGUI/icons/"
#define COMPONENTPATH "../../HopsanGUI/componentData/"
#define PORTICONPATH "../../HopsanGUI/porticons/"
#define MODELPATH "/../../Models/"

//Enums
//! @todo all of these should not be common
enum selectionStatus {DESELECTED, SELECTED};
enum graphicsType {USERGRAPHICS, ISOGRAPHICS};
enum connectorGeometry {VERTICAL, HORIZONTAL, DIAGONAL};
enum renameRestrictions {UNRESTRICTED, CORERENAMEONLY};
enum undoStatus {NOUNDO, UNDO};
enum saveTarget {EXISTINGFILE, NEWFILE};
enum simulationMethod {SINGLECORE, MULTICORE};

//Global pointer to the main window
class MainWindow;
extern MainWindow* gpMainWindow;

#endif // COMMON_H
