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

//!
//! @file   ProjectTabWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

#ifndef PROJECTTABWIDGET_H
#define PROJECTTABWIDGET_H

#include <QTabWidget>
#include "GUIRootSystem.h"

//Forward declaration
class GraphicsScene;
class GraphicsView;
class MainWindow;
class ProjectTab;

class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(MainWindow *parent = 0);

    ProjectTab *getCurrentTab();
    MainWindow *mpParentMainWindow;
    size_t mNumberOfUntitledTabs;

public slots:
    void addProjectTab(ProjectTab *projectTab, QString tabName="Untitled");
    void addNewProjectTab(QString tabName="Untitled");
    void saveProjectTab();
    void saveProjectTabAs();
    void saveProjectTab(int index, bool saveAs);
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void simulateCurrent();
    void loadModel();
    void saveModel(bool saveAs);
    void setIsoGraphics(bool value);

    void resetZoom();
    void zoomIn();
    void zoomOut();
    void hideNames();
    void showNames();
    void updateSimulationSetupWidget();
    void updateCurrentStartTime();
    void updateCurrentTimeStep();
    void updateCurrentStopTime();
    void hidePortsInCurrentTab(bool doIt);
    void exportCurrentToPDF();
    void centerView();
    void disableUndo();
    void updateUndoStatus();

signals:
    void checkMessages();

};

class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(ProjectTabWidget *parent = 0);
    bool mIsSaved;
    QString mModelFileName;
    bool useIsoGraphics;
    ProjectTabWidget *mpParentProjectTabWidget;
    GUIRootSystem mGUIRootSystem;
    GraphicsView *mpGraphicsView;
    GraphicsScene *mpGraphicsScene;
    double getStartTime();
    double getTimeStep();
    double getStopTime();
    QString getIsoIconPath();
    QString getUserIconPath();
    void setIsoIconPath(QString path);
    void setUserIconPath(QString path);

public slots:
    void hasChanged();
    void updateStartTime();
    void updateTimeStep();
    void updateStopTime();

signals:
    void checkMessages();

private:
    double mStartTime;
    double mStopTime;
    double mTimeStep;
    QString mUserIconPath;
    QString mIsoIconPath;
};

#endif // PROJECTTABWIDGET_H
