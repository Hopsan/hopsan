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
//! @brief Contains HopsanGUI global pointers and objects
//!
//$Id$

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

// Forward declaration of global objects
// Note! If you want to use one of these pointers the actual header for that class need to be included in your cpp file as well
class QWidget;
class MainWindow;
class Configuration;
class DesktopHandler;
class CopyStack;
class PlotHandler;
class LibraryWidget;
class TerminalWidget;
class ModelHandler;
class CentralTabWidget;
class SystemParametersWidget;
class UndoWidget;
class LibraryHandler;
class GUIMessageHandler;
class HelpPopUpWidget;
class SensitivityAnalysisDialog;
class HelpDialog;
class OptimizationDialog;
class QAction;
class OptionsDialog;
class QGridLayout;
class FindWidget;
class PlotWidget2;

// Global pointer to the main window and QWidget cast version
extern MainWindow* gpMainWindow;
extern QWidget *gpMainWindowWidget;

// Global object pointers in main
extern Configuration *gpConfig;
extern DesktopHandler *gpDesktopHandler;
extern CopyStack *gpCopyStack;
extern GUIMessageHandler *gpMessageHandler;

// Global object pointers that are children to main window
extern PlotHandler *gpPlotHandler;
extern LibraryWidget *gpLibraryWidget;
extern TerminalWidget *gpTerminalWidget;
extern ModelHandler *gpModelHandler;
extern PlotWidget2 *gpPlotWidget;
extern CentralTabWidget *gpCentralTabWidget;
extern SystemParametersWidget *gpSystemParametersWidget;
extern UndoWidget *gpUndoWidget;
extern LibraryHandler *gpLibraryHandler;
extern HelpPopUpWidget *gpHelpPopupWidget;
extern SensitivityAnalysisDialog *gpSensitivityAnalysisDialog;
extern HelpDialog *gpHelpDialog;
extern OptimizationDialog *gpOptimizationDialog;
extern OptionsDialog *gpOptionsDialog;
extern QGridLayout *gpCentralGridLayout;
extern FindWidget *gpFindWidget;

//Global actions
extern QAction *gpTogglePortsAction;
extern QAction *gpToggleNamesAction;

// Global variables
extern QString gHopsanCoreVersion;

#endif // GLOBAL_H
