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
class ModelWidget;

class DebuggerWidget : public QDialog
{
    Q_OBJECT
public:
    DebuggerWidget(ModelWidget *pModel, QWidget *parent = 0);
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
    ModelWidget *mpModel;

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
