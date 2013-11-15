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
//! @file   HcomHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a handler for the HCOM scripting language
//!
//$Id$

#ifndef HCOMHANDLER_H
#define HCOMHANDLER_H

#include "LogVariable.h"
#include "SymHop.h"

class TerminalWidget;
class TerminalConsole;
class ModelObject;
class HcomCommand;
class ModelWidget;
class OptimizationHandler;

class HcomHandler : public QObject
{
    Q_OBJECT

    friend class TerminalWidget;
    friend class TerminalConsole;
public:
    typedef QMap<QString, double> LocalVarsMapT;

    // Enums
    enum VariableType{Scalar, DataVector, Wildcard, Undefined};

    // Constructor
    HcomHandler(TerminalConsole *pConsole);

    // Command functions
    QStringList getCommands() const;
    void executeCommand(QString cmd);
    QString runScriptCommands(QStringList &lines, bool *pAbort);

    // Public variable access functions
    LocalVarsMapT getLocalVariables() const;
    QMap<QString, SymHop::FunctionPtr> getLocalFunctionPointers() const;
    SharedLogVariableDataPtrT getLogVariablePtr(QString fullShortName, bool &rFoundAlias) const;
    SharedLogVariableDataPtrT getLogVariablePtr(QString fullShortName) const;

    // Public utilities
    void toShortDataNames(QString &rName) const;
    void toLongDataNames(QString &rName) const;

    void setWorkingDirectory(QString dir);
    QString getWorkingDirectory() const;

    bool hasFunction(const QString &func) const;
    bool isAborted() const;
    double getVar(const QString &var) const;

    OptimizationHandler *mpOptHandler;
    TerminalConsole *mpConsole;

    void evaluateExpression(QString expr, VariableType desiredType=Undefined);

    // Return values from evaluation
    double mAnsScalar;
    QString mAnsWildcard;
    SharedLogVariableDataPtrT mAnsVector;
    VariableType mAnsType;

public slots:
    void abortHCOM();

private:

    // Private command functions (to be accessed by public executeCommand function)
    void executeExitCommand(const QString cmd);
    void executeSimulateCommand(const QString cmd);
    void executePlotCommand(const QString cmd);
    void executePlotLeftAxisCommand(const QString cmd);
    void executePlotRightAxisCommand(const QString cmd);
    void executeAddPlotCommand(const QString cmd);
    void executeAddPlotLeftAxisCommand(const QString cmd);
    void executeAddPlotRightAxisCommand(const QString cmd);
    void executeDisplayParameterCommand(const QString cmd);
    void executeAddParameterCommand(const QString cmd);
    void executeChangeParameterCommand(const QString cmd);
    void executeChangeSimulationSettingsCommand(const QString cmd);
    void executeHelpCommand(const QString cmd);
    void executeRunScriptCommand(const QString cmd);
    void executeWriteHistoryToFileCommand(const QString cmd);
    void executePrintCommand(const QString cmd);
    void executeChangePlotWindowCommand(const QString cmd);
    void executeDisplayPlotWindowCommand(const QString cmd);
    void executeDisplayVariablesCommand(const QString cmd);
    void executePeekCommand(const QString cmd);
    void executePokeCommand(const QString cmd);
    void executeDefineAliasCommand(const QString cmd);
    void executeRemoveVariableCommand(const QString cmd);
    void executeChangeDefaultPlotScaleCommand(const QString cmd);
    void executeDisplayDefaultPlotScaleCommand(const QString cmd);
    void executeChangePlotScaleCommand(const QString cmd);
    void executeDisplayPlotScaleCommand(const QString cmd);
    void executeSetCommand(const QString cmd);
    void executeSaveToPloCommand(const QString cmd);
    void executeLoadVariableCommand(const QString cmd);
    void executeLoadModelCommand(const QString cmd);
    void executeLoadRecentCommand(const QString cmd);
    void executeRenameComponentCommand(const QString cmd);
    void executeRemoveComponentCommand(const QString cmd);
    void executePwdCommand(const QString cmd);
    void executeMwdCommand(const QString cmd);
    void executeChangeDirectoryCommand(const QString cmd);
    void executeListFilesCommand(const QString cmd);
    void executeCloseModelCommand(const QString cmd);
    void executeChangeTabCommand(const QString cmd);
    void executeAddComponentCommand(const QString cmd);
    void executeConnectCommand(const QString cmd);
    void executeCreateModelCommand(const QString cmd);
    void executeExportToFMUCommand(const QString cmd);
    void executeChangeTimestepCommand(const QString cmd);
    void executeInheritTimestepCommand(const QString cmd);
    void executeBodeCommand(const QString cmd);
    void executeAbsCommand(const QString cmd);
    void executeOptimizationCommand(const QString cmd);
    void executeCallFunctionCommand(const QString cmd);
    void executeEchoCommand(const QString cmd);
    void executeEditCommand(const QString cmd);
    void executeLp1Command(const QString cmd);
    void executeSetMultiThreadingCommand(const QString cmd);

    // Help functions
    void createCommands();
    void generateCommandsHelpText();
    void changePlotVariables(const QString cmd, const int axis, bool hold=false);
    void addPlotCurve(QString cmd, const int axis) const;
    void removePlotCurves(const int axis) const;
    void removeLogVariable(QString fullShortVarNameWithGen) const;
    void getComponents(QString str, QList<ModelObject*> &components);
    void getParameters(QString str, ModelObject* pComponent, QStringList &parameters);
    void getParameters(QString str, QStringList &parameters);
    QString getParameterValue(QString parameter) const;
    void getMatchingLogVariableNames(QString pattern, QStringList &rVariables) const;
    void getLogVariablesThatStartsWithString(const QString str, QStringList &variables) const;
    bool evaluateArithmeticExpression(QString cmd);
    void splitAtFirst(QString str, QString c, QString &left, QString &right);
    bool containsOutsideParentheses(QString str, QString c);
    double getNumber(const QString &rStr, bool *pOk);
    QString getDirectory(const QString &cmd) const;
    QStringList getArguments(const QString &cmd) const;
    int getNumberOfArguments(const QString &cmd) const;
    QString getArgument(const QString &cmd, const int idx) const;
    void registerFunction(const QString func, const QString description, const SymHop::FunctionPtr fptr);

    // Used to abort HCOM evaluation
    bool mAborted;

    // Working directory
    QString mPwd;

    // Commands
    QList<HcomCommand> mCmdList;

    // Plotting
    QString mCurrentPlotWindowName;

    // Local variables
    LocalVarsMapT mLocalVars;
    QMap<QString, SymHop::FunctionPtr> mLocalFunctionPtrs;
    QMap<QString, QString> mLocalFunctionDescriptions;

    VariableType mRetvalType;

    // Functions
    QMap<QString, QStringList> mFunctions;
};


class HcomCommand
{
public:
    void runCommand(QString cmd, HcomHandler *pHandler)
    {
        //(fnc)(cmd);
        (pHandler->*HcomCommand::fnc)(cmd);
    }

    QString cmd;
    QString description;
    QString help;
    QString group;
    void (HcomHandler::*fnc)(QString);
};


double _funcAver(QString str, bool &ok);
double _funcMin(QString str, bool &ok);
double _funcMax(QString str, bool &ok);
double _funcIMin(QString str, bool &ok);
double _funcIMax(QString str, bool &ok);
double _funcPeek(QString str, bool &ok);
double _funcRand(QString str, bool &ok);
double _funcSize(QString str, bool &ok);
double _funcObj(QString str, bool &ok);
double _funcTime(QString, bool &ok);

#endif // HCOMHANDLER_H
