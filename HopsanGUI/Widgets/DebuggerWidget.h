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
//! @file   DebuggerWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a class for the debugger widget
//!

#ifndef DEBUGGERWIDGET_H
#define DEBUGGERWIDGET_H

#include <QVariant>
#include <QAction>
#include <QSpinBox>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QFile>

// Forward declaration
class SystemContainer;

class DebuggerWidget : public QDialog
{
    Q_OBJECT
public:
    DebuggerWidget(SystemContainer *pSystem, QWidget *parent = 0);
    void retranslateUi();
    void setInitData();

signals:
    
public slots:
    
private slots:
    void updatePortsList(QString component);
    void updateVariablesList(QString port);
    void addVariable();
    void removeVariable();

    void runInitialization();
    void stepForward();
    void nStepsForward();
    void simulateTo();
    void simulateTo(double targetTime, bool doLog=true);
    void collectPlotData(bool overWriteGeneration=true);
    void logLastData();
    void updateTimeDisplay();
    double getCurrentTime() const;
    double getTimeStep() const;
    int getCurrentStep() const;
    double getStartTime() const;
    double getStopTime() const;


private:
    SystemContainer *mpSystem;

    QTabWidget *mpTabWidget;
    QWidget *mpTraceTab;
    QVBoxLayout *mpTraceTabLayout;
    QTableWidget *mpTraceTable;
    QWidget *mpVariablesTab;
    QGridLayout *mpVariablesTabLayout;
    QListWidget *mpVariablesList;
    QPushButton *mpRemoveButton;
    QListWidget *mpPortsList;
    QPushButton *mpAddButton;
    QListWidget *mpComponentsList;
    QListWidget *mpChoosenVariablesList;
    QLabel *mpCurrentStepLabel;
    QLabel *mTimeIndicatorLabel;

    QPushButton *mpAbortButton;
    QDoubleSpinBox *mpGotoTimeSpinBox;
    QPushButton *mpGotoButton;
    QPushButton *mpForwardButton;
    QSpinBox *mpNumStepsSpinBox;
    QPushButton *mpMultiForwardButton;
    QPushButton *mpInitializeButton;

    QStringList mVariables;
    bool mIsInitialized();
    double mCurrentTime;
    QFile mOutputFile;
};

#endif // DEBUGGERWIDGET_H
