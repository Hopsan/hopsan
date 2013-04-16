#ifndef DEBUGGERWIDGET_H
#define DEBUGGERWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "GUIObjects/GUISystem.h"

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
    void simulateTo(double targetTime);
    void collectLastData();
    void updateTimeDisplay();
    double getCurrentTime();
    double getTimeStep();


private:
    SystemContainer *mpSystem;

    QVBoxLayout *mpVerticalLayout;
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
    QWidget *mpButtonsWidget;
    QHBoxLayout *mpHorizontalLayout;
    QLabel *mpCurrentStepLabel;
    QLabel *mTimeIndicatorLabel;
    QSpacerItem *mpHorizontalSpacer;
    QPushButton *mpAbortButton;
    QPushButton *mpGotoButton;
    QPushButton *mpForwardButton;
    QPushButton *mpInitializeButton;

    QStringList mVariables;
    bool mIsInitialized();
    double mCurrentTime;
    QFile mOutputFile;
};

#endif // DEBUGGERWIDGET_H
