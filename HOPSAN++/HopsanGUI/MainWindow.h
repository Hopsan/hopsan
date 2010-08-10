/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$

//!
//! \mainpage
//! Library used in Hopsan NG is 'qwt-5'
//!

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

//#include <QComboBox>
//#include <QLabel>
//class QGridLayout;
//class QHBoxLayout;
//class QMenuBar;
//class QMenu;
//class QStatusBar;
//class QAction;
//class QString;
//class QPlainTextEdit;

class SimulationSetupWidget;
class ProjectTabWidget;
class GraphicsView;
class GraphicsScene;
class LibraryWidget;
class PreferenceWidget;
class OptionsWidget;
class UndoWidget;
class MessageWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QWidget *mpCentralwidget;
    QGridLayout *mpCentralgrid;
    UndoWidget *mpUndoWidget;
    ProjectTabWidget *mpProjectTabs;
    QGridLayout *mpTabgrid;
    LibraryWidget *mpLibrary;
    //SimulationSetupWidget *mpSimulationSetupWidget;
    OptionsWidget *mpOptionsWidget;
    PreferenceWidget *mpPreferenceWidget;
    bool mInvertWheel;

    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuTools;
    QMenu *menuPlot;
    MessageWidget *mpMessageWidget;
    QStatusBar *statusBar;

    QPushButton *mpBackButton;

    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *closeAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *openUndoAction;
    QAction *disableUndoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *simulateAction;
    QAction *plotAction;
    QAction *loadLibsAction;
    QAction *preferencesAction;
    QAction *optionsAction;
    QAction *resetZoomAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *centerViewAction;
    QAction *hideNamesAction;
    QAction *showNamesAction;
    QAction *hidePortsAction;
    QAction *showPortsAction;
    QAction *exportPDFAction;
    QLineEdit *mpStartTimeLineEdit;
    QLineEdit *mpTimeStepLineEdit;
    QLineEdit *mpFinishTimeLineEdit;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *simToolBar;
    QToolBar *mpSimulationToolBar;
    QToolBar *viewToolBar;

    void setStartTimeLabel(double startTime);
    void setTimeStepLabel(double timeStep);
    void setFinishTimeLabel(double finishTime);

    double getStartTimeLabel();
    double getTimeStepLabel();
    double getFinishTimeLabel();


    //QComboBox *viewScaleCombo;


    //QString libDir;

    //GraphicsScene *scene;
    //GraphicsView *view;

    //VariableListDialog *variableList;

    void closeEvent(QCloseEvent *event);

public slots:
    void show();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void fixSimulationParameterValues();
    void saveSettings();

private slots:
    //void addLibs(QString libDir, QString parentLib=QString());
    //void addLibs();
    void plot();
    void openPreferences();
    void openOptions();
    void openUndo();
    void loadSettings();

private:
    void createActions();
    void createMenus();
    void createToolbars();
    QDockWidget *messagedock;
    QDockWidget *libdock;
    QDockWidget *mPlotVariablesDock;
    QDockWidget *mUndoDock;

    void setValue(QLineEdit *lineEdit, double value);
    double getValue(QLineEdit *lineEdit);
    void fixFinishTime();
    void fixTimeStep();
    bool mPlotVariableListOpen;

};

#endif // MAINWINDOW_H
