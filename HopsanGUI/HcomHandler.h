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
#include "PlotCurveStyle.h"

class SystemObject;
class TerminalWidget;
class TerminalConsole;
class ModelObject;
class HcomCommand;
class ModelWidget;
class OptimizationHandler;
class Configuration;
class Port;
class PlotWindow;

class HcomHandler : public QObject
{
    Q_OBJECT

    friend class TerminalWidget;
    friend class TerminalConsole;
public:
    typedef QMap<QString, double> LocalVarsMapT;

    // Enums
    enum VariableType{Scalar, DataVector, String, Expression, Wildcard, Undefined};

    // Constructor
    HcomHandler(TerminalConsole *pConsole);
    ~HcomHandler();

    //Set and get functions for pointers
    ModelWidget *getModelPtr() const;
    void setConfigPtr(Configuration *pConfig);
    Configuration *getConfigPtr() const;
    //void setOptHandlerPtr(OptimizationHandler *pOptHandler);

    // Command functions
    QStringList getCommands() const;
    void executeCommand(QString cmd);
    QString runScriptCommands(QStringList &lines, bool *pAbort);

    // Public variable access functions
    LocalVarsMapT getLocalVariables() const;
    void setLocalVariables(const LocalVarsMapT &vars);
    QMap<QString, SymHopFunctionoid *> getLocalFunctionoidPointers() const;
    SharedVectorVariableT getLogVariable(QString fullShortName) const;

    QStringList getAutoCompleteWords() const;

    // Public utilities
    void toShortDataNames(QString &rName) const;
    void toLongDataNames(QString &rName) const;

    void setWorkingDirectory(QString dir);
    QString getWorkingDirectory() const;

    bool hasFunction(const QString &func) const;
    QString getFunctionName(QString expression) const;
    void getFunctionCode(QString funcName, QStringList &funcCode);
    bool isAborted() const;
    double getVar(const QString &var) const;

    OptimizationHandler *mpOptHandler;
    TerminalConsole *mpConsole;

    void evaluateExpression(QString expr, VariableType desiredType=Undefined);
    double evaluateScalarExpression(QString expr, bool &rIsOK);
    typedef double (*ScalarMathFunction_t)(double);
    typedef QMap<QString, ScalarMathFunction_t> FuncMap_t;

    void setAcceptsOptimizationCommands(const bool value);
    bool getAcceptsOptimizationCommands() const;

    // Return values from evaluation
    double mAnsScalar;
    QString mAnsWildcard;
    SharedVectorVariableT mAnsVector;
    SymHop::Expression mAnsExpression;
    VariableType mAnsType;


public slots:
    void abortHCOM();
    void setModelPtr(ModelWidget *pModel);

signals:
    void aborted();

private:

    // Private command functions (to be accessed by public executeCommand function)
    void executeExitCommand(const QString cmd);

    void executeChangeSimulationSettingsCommand(const QString cmd);
    void executeSimulateCommand(const QString cmd);

    void executeChangePlotWindowCommand(const QString cmd);
    void executeDisplayPlotWindowCommand(const QString cmd);
    void executePlotCommand(const QString cmd);
    void executePlotLeftAxisCommand(const QString cmd);
    void executePlotRightAxisCommand(const QString cmd);
    void executePlotXAxisCommand(const QString cmd);
    void executeAddPlotCommand(const QString cmd);
    void executeAddPlotLeftAxisCommand(const QString cmd);
    void executeAddPlotRightAxisCommand(const QString cmd);
    void executeChangeDiagramSizeCommand(const QString cmd);
    void executeChangeDiagramLimitsCommand(const QString cmd);
    void executeChangeDiagramLimitsYLCommand(const QString cmd);
    void executeChangeDiagramLimitsYRCommand(const QString cmd);
    void executeChangeLogarithmicAxisX(const QString cmd);
    void executeChangeLogarithmicAxisYL(const QString cmd);
    void executeChangeLogarithmicAxisYR(const QString cmd);
//    void executeChangePlotScaleCommand(const QString cmd);
//    void executeDisplayPlotScaleCommand(const QString cmd);
    void executeChangeTimePlotOffsetCommand(const QString cmd);
    void executeDisplayTimePlotOffsetCommand(const QString cmd);
    void executeSavePlotWindowCommand(const QString cmd);
    void executeInvertPlotVariableCommand(const QString cmd);
    void executeSetlabelCommand(const QString cmd);

    void executeDisplayVariablesCommand(const QString cmd);
    void executeRemoveVariableCommand(const QString cmd);
    void executeDisplayHighestGenerationCommand(const QString cmd);
//    void executeChangeDefaultPlotScaleCommand(const QString cmd);
//    void executeDisplayDefaultPlotScaleCommand(const QString cmd);
//    void executeChangeDefaultPlotOffsetCommand(const QString cmd);
//    void executeDisplayDefaultPlotOffsetCommand(const QString cmd);
    void executeVariableInfoCommand(const QString cmd);
    void executeSetQuantityCommand(const QString args);

    void executeDisplayParameterCommand(const QString cmd);
    void executeAddParameterCommand(const QString cmd);
    void executeChangeParameterCommand(const QString cmd);

    void executeDisplayComponentsCommand(const QString cmd);

    void executeHelpCommand(QString arg);
    void executeRunScriptCommand(const QString cmd);
    void executeWriteHistoryToFileCommand(const QString cmd);
    void executeWriteToFileCommand(const QString cmd);
    void executePrintCommand(const QString cmd);
    void executeEvalCommand(const QString cmd);

    void executePeekCommand(const QString cmd);
    void executePokeCommand(const QString cmd);
    void executeDefineAliasCommand(const QString cmd);

    void executeDisableLoggingCommand(const QString cmd);
    void executeEnableLoggingCommand(const QString cmd);
    void executeSetCommand(const QString cmd);
    void executeGetCommand(const QString cmd);
    void executeSaveToPloCommand(const QString cmd);
    void executeLoadVariableCommand(const QString cmd);
    void executeSaveParametersCommand(const QString cmd);
    void executeLoadParametersCommand(const QString cmd);
    void executeLoadModelCommand(const QString cmd);
    void executeRevertModelCommand(const QString cmd);
    void executeLoadRecentCommand(const QString cmd);
    void executeSaveModelCommand(const QString cmd);
    void executeRenameComponentCommand(const QString cmd);
    void executeRemoveComponentCommand(const QString cmd);
    void executePwdCommand(const QString cmd);
    void executeMwdCommand(const QString cmd);
    void executeChangeDirectoryCommand(const QString cmd);
    void executeMakeDirectoryCommand(const QString cmd);
    void executeListFilesCommand(const QString cmd);
    void executeCloseModelCommand(const QString cmd);
    void executeChangeTabCommand(const QString cmd);
    void executeAddComponentCommand(const QString cmd);
    void executeReplaceComponentCommand(const QString cmd);
    void executeConnectCommand(const QString cmd);
    void executeListUnconnectedCommand(const QString cmd);
    void executeCreateModelCommand(const QString cmd);
    void executeExportToFMUCommand(const QString cmd);
    void executeChangeTimestepCommand(const QString cmd);
    void executeInheritTimestepCommand(const QString cmd);
    void executeBodeCommand(const QString cmd);
    void executeNyquistCommand(const QString cmd);
    void executeOptimizationCommand(const QString cmd);
    void executeCallFunctionCommand(const QString cmd);
    void executeEchoCommand(const QString cmd);
    void executeClearCommand(const QString cmd);
    void executeEditCommand(const QString cmd);
    void executeSetMultiThreadingCommand(const QString cmd);
    void executeLockAllAxesCommand(const QString cmd);
    void executeLockLeftAxisCommand(const QString cmd);
    void executeLockRightAxisCommand(const QString cmd);
    void executeLockXAxisCommand(const QString cmd);
    void executeSleepCommand(const QString cmd);

    // Help functions
    void createCommands();
    void generateCommandsHelpText();
    void registerInternalFunction(const QString &funcName, const QString &description, const QString &help="");
    void registerFunctionoid(const QString &funcName, SymHopFunctionoid *pFunctinoid, const QString &description, const QString &help);

    void changePlotVariables(const QString cmd, const int axis, bool hold=false);
    void changePlotXVariable(const QString varExp);
    void addPlotCurve(QString var, const int axis, PlotCurveStyle style=PlotCurveStyle());
    void addPlotCurve(SharedVectorVariableT data, const int axis, bool autoRefresh=true, PlotCurveStyle style=PlotCurveStyle());
    void removePlotCurves(const int axis) const;
    void extractCurveStyle(QString &value, PlotCurveStyle &style);

    void removeLogVariable(QString fullShortVarNameWithGen) const;

    QString getfullNameFromAlias(const QString &rAlias) const;
    void getMatchingLogVariableNamesWithoutLogDataHandler(QString pattern, QStringList &rVariables) const;
    void getMatchingLogVariableNames(QString pattern, QStringList &rVariables, bool addGenerationSpecifier=true, const int generationOverride=-10) const;
    void getLogVariablesThatStartsWithString(const QString str, QStringList &variables) const;
    int parseAndChopGenerationSpecifier(QString &rStr, bool &rOk) const;

    void getComponents(const QString &rStr, QList<ModelObject *> &rComponents, SystemObject *pSystem=0) const;
    void getPorts(const QString &rStr, QList<Port *> &rPorts) const;

    void getParameters(QString str, QStringList &rParameters) const;
    void getParametersFromContainer(SystemObject *pSystem, QStringList &rParameters) const;
    QString getParameterValue(QString parameterName, QString &rParameterType, bool searchFromTopLevel=false) const;
    QString getParameterValue(QString parameterName) const;

    bool evaluateArithmeticExpression(QString cmd);

    void executeGtBuiltInFunction(QString functionCall);
    void executeLtBuiltInFunction(QString functionCall);
    void executeEqBuiltInFunction(QString functionCall);
    void executeCutBuiltInFunction(QString functionCall);
    void executeSsiBuiltInFunction(QString functionCall);

    QString getDirectory(const QString &cmd) const;
    double getNumber(const QString &rStr, bool *pOk);

    bool containsOutsideParentheses(QString str, QString c);
    void splitAtFirst(QString str, QString c, QString &left, QString &right);

    //Current model pointer
    ModelWidget *mpModel;

    //Custom configuration pointer
    Configuration *mpConfig;

    // Used to abort HCOM evaluation
    bool mAborted;

    // Working directory
    QString mPwd;

    // Commands
    QList<HcomCommand> mCmdList;

    // Plotting
    QPointer<PlotWindow> mpCurrentPlotWindow;

    // Local variables
    LocalVarsMapT mLocalVars;
    QMap<QString, SymHop::Expression> mLocalExpressions;
    QMap<QString, SymHopFunctionoid*> mLocalFunctionoidPtrs;
    QMap<QString, QPair<QString, QString> > mLocalFunctionDescriptions;

    VariableType mRetvalType;

    bool mAcceptsOptimizationCommands;

    // Functions
    QMap<QString, QStringList> mFunctions;

    //Private get functions

    void updatePwd();
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



//! @class HcomFunctionoid
//! @brief Functionoid class used for sending member function pointers to SymHop
//! @author Robert Braun <robert.braun@liu.se>
//!
//! Inherits the pure virtual base class defined by SymHop. Adds a constructor and a member pointer to the HcomHandler object.
//!
class HcomFunctionoid : public SymHopFunctionoid
{
public:
    HcomFunctionoid(HcomHandler *pHandler)
    {
        mpHandler = pHandler;
    }
protected:
    HcomHandler *mpHandler;
};

class HcomFunctionoidAver : public HcomFunctionoid
{
public:
    HcomFunctionoidAver(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidSize : public HcomFunctionoid
{
public:
    HcomFunctionoidSize(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidTime : public HcomFunctionoid
{
public:
    HcomFunctionoidTime(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidObj : public HcomFunctionoid
{
public:
    HcomFunctionoidObj(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidMin : public HcomFunctionoid
{
public:
    HcomFunctionoidMin(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidMax : public HcomFunctionoid
{
public:
    HcomFunctionoidMax(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidIMin : public HcomFunctionoid
{
public:
    HcomFunctionoidIMin(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidIMax : public HcomFunctionoid
{
public:
    HcomFunctionoidIMax(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidPeek : public HcomFunctionoid
{
public:
    HcomFunctionoidPeek(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidRand : public HcomFunctionoid
{
public:
    HcomFunctionoidRand(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidOptVar : public HcomFunctionoid
{
public:
    HcomFunctionoidOptVar(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidOptPar : public HcomFunctionoid
{
public:
    HcomFunctionoidOptPar(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};


class HcomFunctionoidFC : public HcomFunctionoid
{
public:
    HcomFunctionoidFC(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidMaxPar : public HcomFunctionoid
{
public:
    HcomFunctionoidMaxPar(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidMinPar : public HcomFunctionoid
{
public:
    HcomFunctionoidMinPar(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidHg : public HcomFunctionoid
{
public:
    HcomFunctionoidHg(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidAns : public HcomFunctionoid
{
public:
    HcomFunctionoidAns(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidExists : public HcomFunctionoid
{
public:
    HcomFunctionoidExists(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};

class HcomFunctionoidCount : public HcomFunctionoid
{
public:
    HcomFunctionoidCount(HcomHandler *pHandler) : HcomFunctionoid(pHandler) {}
    double operator()(QString &str, bool &ok);
};
#endif // HCOMHANDLER_H
