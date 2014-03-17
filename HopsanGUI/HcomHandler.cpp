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
//! @file   HcomHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a handler for the HCOM scripting language

// Qt includes
#include <QDesktopServices>

//HopsanGUI includes
#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/OptimizationDialog.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "PlotCurve.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "SimulationThreadHandler.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/PlotWidget.h"
#include "ModelHandler.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/ModelWidget.h"
#include "SymHop.h"
#ifndef WIN32
#include <unistd.h>
#endif

//HopsanGenerator includes
#include "SymHop.h"

//Dependency includes
#include "qwt_plot.h"



#define HCOMPRINT(text) mpConsole->print(text);
#define HCOMINFO(text) mpConsole->printInfoMessage(text,"",false)
#define HCOMWARN(text) mpConsole->printWarningMessage(text,"",false)
#define HCOMERR(text) mpConsole->printErrorMessage(text,"",false)

#define GenerationSpecifierChar "@"


class LongShortNameConverter
{
public:
    LongShortNameConverter()
    {
        //! @todo these should not be hardcoded
        registerPair("x", "Position");
        registerPair("v", "Velocity");
        registerPair("f", "Force");
        registerPair("p", "Pressure");
        registerPair("q", "Flow");
        registerPair("y", "Value");
        registerPair("Zc", "CharImpedance");
        registerPair("c", "WaveVariable");
        registerPair("me", "EquivalentMass");
        registerPair("Q", "HeatFlow");
        registerPair("t", "Temperature");
    }

    void registerPair(const QString &rShort, const QString &rLong)
    {
        mShortToLong.insertMulti(rShort, rLong);
        mLongToShort.insertMulti(rLong, rShort);
    }

    QList<QString> shortToLong(const QString &rShort) const
    {
        return mShortToLong.values(rShort);
    }

    QList<QString> shortToLong(const QRegExp &rShort) const
    {
        bool foundAtLeastOne=false;
        QList<QString> results;
        QMap<QString,QString>::const_iterator it;
        for (it=mShortToLong.begin(); it!=mShortToLong.end(); ++it)
        {
            if (rShort.exactMatch(it.key()))
            {
                results.append(it.value());
                foundAtLeastOne=true;
            }
            else if (foundAtLeastOne)
            {
                // We can brek loop since maps should be ordered by key
                break;
            }
        }
        return results;
    }

    QList<QString> longToShort(const QString &rLong) const
    {
        return mLongToShort.values(rLong);
    }

    QList<QString> longToShort(const QRegExp &rLong) const
    {
        bool foundAtLeastOne=false;
        QList<QString> results;
        QMap<QString,QString>::const_iterator it;
        for (it=mLongToShort.begin(); it!=mLongToShort.end(); ++it)
        {
            if (rLong.exactMatch(it.key()))
            {
                results.append(it.value());
                foundAtLeastOne=true;
            }
            else if (foundAtLeastOne)
            {
                // We can brek loop since maps should be ordered by key
                break;
            }
        }
        return results;
    }

private:
    QMap<QString,QString> mShortToLong;
    QMap<QString,QString> mLongToShort;
};

LongShortNameConverter gLongShortNameConverter;

HcomHandler::HcomHandler(TerminalConsole *pConsole) : QObject(pConsole)
{
    mpModel = 0;
    mpConfig = 0;
    mpOptHandler = 0;

    mAcceptsOptimizationCommands = false;

    mAnsType = Undefined;
    mAborted = false;
    mRetvalType = Scalar;

    mpConsole = pConsole;

    mpOptHandler = new OptimizationHandler(this);
    // connect the optimization message handler to the consol for this HCOM handler
    connect(mpOptHandler->getMessageHandler(), SIGNAL(newAnyMessage(GUIMessage)), mpConsole, SLOT(printMessage(GUIMessage)));

    mPwd = gpDesktopHandler->getDocumentsPath();
    mPwd.chop(1);

    //Register internal function descriptions
    registerInternalFunction("lp1", "Applies low-pass filter of first degree to vector","Usage: lp1(vector, frequency) or lp1(vector, timevector, frequency)");
    registerInternalFunction("ddt", "Differentiates vector with respect to time (or to custom vector)","Usage: ddt(vector) or ddt(vector, timevector)");
    registerInternalFunction("int", "Integrates vector with respect to time (or to custom vector)", "Usage: int(vector) or int(vector, timevector)");
    registerInternalFunction("fft", "Generates frequency spectrum plot from vector","Usage: fft(vector), fft(vector, power[true/false]), fft(vector, timevector) or fft(vector, timevector, power[true/false])");
    registerInternalFunction("gt", "Index-wise greater than check between vector and scalar (equivalent to \">\" operator)","Usage: gt(varName, threshold)");
    registerInternalFunction("lt", "Index-wise less than check between vector and scalar  (equivalent to \"<\" operator)","Usage: lt(varName, threshold)");
    registerInternalFunction("linspace", "Linearly spaced vector","Usage: linspace(min, max, numSamples)");
    registerInternalFunction("logspace", "Logarithmicly spaced vector","Usage: logspace(min, max, numSamples)");
    registerInternalFunction("ones", "Create a vector of ones","Usage: ones(size)");
    registerInternalFunction("zeros", "Create a vector of zeros","Usage: zeros(size)");
    registerInternalFunction("maxof", "Returns the element-wise maximum values of x and y vectors","Usage: maxof(x,y)");
    registerInternalFunction("minof", "Returns the element-wise minimum values of x and y vectors","Usage: minof(x,y)");

    //Setup local function pointers (used to evaluate expressions in SymHop)
    registerFunctionoid("aver", new HcomFunctionoidAver(this), "Calculate average value of vector", "Usage: aver(vector)");
    registerFunctionoid("min", new HcomFunctionoidMin(this), "Calculate minimum value of vector", "Usage: min(vector)");
    registerFunctionoid("max", new HcomFunctionoidMax(this), "Calculate maximum value of vector","Usage:max(vector)");
    registerFunctionoid("imin", new HcomFunctionoidIMin(this), "Calculate index of minimum value of vector","Usage: imin(vector)");
    registerFunctionoid("imax", new HcomFunctionoidIMax(this), "Calculate index of maximum value of vector","Usage: imax(vector)");
    registerFunctionoid("size", new HcomFunctionoidSize(this), "Calculate the size of a vector","Usage: size(vector)");
    registerFunctionoid("rand", new HcomFunctionoidRand(this), "Generates a random value between 0 and 1", "Usage: rand()");
    registerFunctionoid("peek", new HcomFunctionoidPeek(this), "Returns vector value at specified index", "Usage: peek(vector)");
    registerFunctionoid("obj", new HcomFunctionoidObj(this), "Returns optimization objective function value with specified index","Usage: obj(idx)");
    registerFunctionoid("time", new HcomFunctionoidTime(this), "Returns last simulation time", "Usage: time()");
    registerFunctionoid("optvar", new HcomFunctionoidOptVar(this), "Returns specified optimization variable", "Usage: optvar(idx)");
    registerFunctionoid("optpar", new HcomFunctionoidOptPar(this), "Returns specified optimization parameter", "Usage: optpar(idx)");
    registerFunctionoid("fc", new HcomFunctionoidFC(this), "Fuzzy compare, returns 1 if the values of the arguments are almost the same", "Usage: fc(expr, expr, tolerance)");

    createCommands();

    mLocalVars.insert("true",1);
    mLocalVars.insert("false",0);
}

HcomHandler::~HcomHandler()
{
    QMapIterator<QString, SymHopFunctionoid*> it(mLocalFunctionoidPtrs);
    while(it.hasNext())
    {
        delete(it.next().value());
    }
}

void HcomHandler::setModelPtr(ModelWidget *pModel)
{
    mpModel = pModel;
}

ModelWidget *HcomHandler::getModelPtr() const
{
    return mpModel;
}

void HcomHandler::setConfigPtr(Configuration *pConfig)
{
    mpConfig = pConfig;
}

Configuration *HcomHandler::getConfigPtr() const
{
    if(mpConfig)
    {
        return mpConfig;
    }
    return gpConfig;
}

//void HcomHandler::setOptHandlerPtr(OptimizationHandler *pOptHandler)
//{
//    mpOptHandler = pOptHandler;
//}

//! @brief Creates all command objects that can be used in terminal
void HcomHandler::createCommands()
{
    HcomCommand helpCmd;
    helpCmd.cmd = "help";
    helpCmd.description.append("Shows help information");
    helpCmd.help.append(" Usage: help [command]");
    helpCmd.fnc = &HcomHandler::executeHelpCommand;
    mCmdList << helpCmd;

    HcomCommand simCmd;
    simCmd.cmd = "sim";
    simCmd.description.append("Simulates current model (or all open models)");
    simCmd.help.append(" Usage: sim [all]");
    simCmd.fnc = &HcomHandler::executeSimulateCommand;
    simCmd.group = "Simulation Commands";
    mCmdList << simCmd;

    HcomCommand chpvCmd;
    chpvCmd.cmd = "chpv";
    chpvCmd.description.append("Change plot variables in current plot");
    chpvCmd.help.append(" Usage: chpv [leftvar1 [leftvar2] ... [-r rightvar1 rightvar2 ... ]]");
    chpvCmd.fnc = &HcomHandler::executePlotCommand;
    chpvCmd.group = "Plot Commands";
    mCmdList << chpvCmd;

    HcomCommand adpvCmd;
    adpvCmd.cmd = "adpv";
    adpvCmd.description.append("Add plot variables in current plot");
    adpvCmd.help.append(" Usage: adpv [leftvar1 [leftvar2] ... [-r rightvar1 rightvar2 ... ]]");
    adpvCmd.fnc = &HcomHandler::executeAddPlotCommand;
    adpvCmd.group = "Plot Commands";
    mCmdList << adpvCmd;

    HcomCommand adpvlCmd;
    adpvlCmd.cmd = "adpvl";
    adpvlCmd.description.append("Adds plot variables on left axis in current plot");
    adpvlCmd.help.append(" Usage: adpvl [var1 var2 ... ]");
    adpvlCmd.fnc = &HcomHandler::executeAddPlotLeftAxisCommand;
    adpvlCmd.group = "Plot Commands";
    mCmdList << adpvlCmd;

    HcomCommand adpvrCmd;
    adpvrCmd.cmd = "adpvr";
    adpvrCmd.description.append("Adds plot variables on right axis in current plot");
    adpvrCmd.help.append(" Usage: adpvr [var1 var2 ... ]");
    adpvrCmd.fnc = &HcomHandler::executeAddPlotRightAxisCommand;
    adpvrCmd.group = "Plot Commands";
    mCmdList << adpvrCmd;

    HcomCommand exitCmd;
    exitCmd.cmd = "exit";
    exitCmd.description.append("Exits the program");
    exitCmd.help.append(" Usage: exit [no arguments]");
    exitCmd.fnc = &HcomHandler::executeExitCommand;
    mCmdList << exitCmd;

    HcomCommand dipaCmd;
    dipaCmd.cmd = "dipa";
    dipaCmd.description.append("Display parameter value");
    dipaCmd.help.append(" Usage: dipa [parameter]");
    dipaCmd.fnc = &HcomHandler::executeDisplayParameterCommand;
    dipaCmd.group = "Parameter Commands";
    mCmdList << dipaCmd;

    HcomCommand adpaCmd;
    adpaCmd.cmd = "adpa";
    adpaCmd.description.append("Add (system) parameter");
    adpaCmd.help.append(" Usage: adpa [parameter] [value]");
    adpaCmd.fnc = &HcomHandler::executeAddParameterCommand;
    adpaCmd.group = "Parameter Commands";
    mCmdList << adpaCmd;

    HcomCommand chpaCmd;
    chpaCmd.cmd = "chpa";
    chpaCmd.description.append("Change parameter value");
    chpaCmd.help.append(" Usage: chpa [parameter value]");
    chpaCmd.fnc = &HcomHandler::executeChangeParameterCommand;
    chpaCmd.group = "Parameter Commands";
    mCmdList << chpaCmd;

    HcomCommand chssCmd;
    chssCmd.cmd = "chss";
    chssCmd.description.append("Change simulation settings");
    chssCmd.help.append(" Usage: chss [starttime timestep stoptime [samples]]");
    chssCmd.fnc = &HcomHandler::executeChangeSimulationSettingsCommand;
    chssCmd.group = "Simulation Commands";
    mCmdList << chssCmd;

    HcomCommand execCmd;
    execCmd.cmd = "exec";
    execCmd.description.append("Executes a script file");
    execCmd.help.append(" Usage: exec [filepath]");
    execCmd.fnc = &HcomHandler::executeRunScriptCommand;
    execCmd.group = "File Commands";
    mCmdList << execCmd;

    HcomCommand wrhiCmd;
    wrhiCmd.cmd = "wrhi";
    wrhiCmd.description.append("Writes history to file");
    wrhiCmd.help.append(" Usage: wrhi [filepath]");
    wrhiCmd.fnc = &HcomHandler::executeWriteHistoryToFileCommand;
    wrhiCmd.group = "File Commands";
    mCmdList << wrhiCmd;

    HcomCommand printCmd;
    printCmd.cmd = "print";
    printCmd.description.append("Prints arguments on the screen");
    printCmd.help.append(" Usage: print [-flag \"string\"]\n");
    printCmd.help.append(" Flags (optional):\n");
    printCmd.help.append(" -i Info message\n");
    printCmd.help.append(" -w Warning message\n");
    printCmd.help.append(" -e Error message\n");
    printCmd.help.append(" Variables can be printed by putting them in dollar signs.\n");
    printCmd.help.append(" Example:\n");
    printCmd.help.append(" >> print -w \"x=$x$\"\n");
    printCmd.help.append(" Warning: x=12");
    printCmd.fnc = &HcomHandler::executePrintCommand;
    mCmdList << printCmd;

    HcomCommand chpwCmd;
    chpwCmd.cmd = "chpw";
    chpwCmd.description.append("Changes current terminal plot window");
    chpwCmd.help.append(" Usage: chpw [name]");
    chpwCmd.fnc = &HcomHandler::executeChangePlotWindowCommand;
    chpwCmd.group = "Plot Commands";
    mCmdList << chpwCmd;

    HcomCommand dipwCmd;
    dipwCmd.cmd = "dipw";
    dipwCmd.description.append("Displays current terminal plot window");
    dipwCmd.help.append(" Usage: dipw [no arguments]");
    dipwCmd.fnc = &HcomHandler::executeDisplayPlotWindowCommand;
    dipwCmd.group = "Plot Commands";
    mCmdList << dipwCmd;

    HcomCommand chpvlCmd;
    chpvlCmd.cmd = "chpvl";
    chpvlCmd.description.append("Changes plot variables on left axis in current plot");
    chpvlCmd.help.append(" Usage: chpvl [var1 var2 ... ]");
    chpvlCmd.fnc = &HcomHandler::executePlotLeftAxisCommand;
    chpvlCmd.group = "Plot Commands";
    mCmdList << chpvlCmd;

    HcomCommand chpvrCmd;
    chpvrCmd.cmd = "chpvr";
    chpvrCmd.description.append("Changes plot variables on right axis in current plot");
    chpvrCmd.help.append(" Usage: chpvr [var1 var2 ... ]");
    chpvrCmd.fnc = &HcomHandler::executePlotRightAxisCommand;
    chpvrCmd.group = "Plot Commands";
    mCmdList << chpvrCmd;

    HcomCommand dispCmd;
    dispCmd.cmd = "disp";
    dispCmd.description.append("Shows a list of all variables matching specified name filter (using asterisks)");
    dispCmd.help.append(" Usage: disp [filter]");
    dispCmd.fnc = &HcomHandler::executeDisplayVariablesCommand;
    dispCmd.group = "Variable Commands";
    mCmdList << dispCmd;

    HcomCommand peekCmd;
    peekCmd.cmd = "peek";
    peekCmd.description.append("Shows the value at a specified index in a specified data variable");
    peekCmd.help.append(" Usage: peek [variable index]");
    peekCmd.group = "Variable Commands";
    peekCmd.fnc = &HcomHandler::executePeekCommand;

    mCmdList << peekCmd;

    HcomCommand pokeCmd;
    pokeCmd.cmd = "poke";
    pokeCmd.description.append("Changes the value at a specified index in a specified data variable");
    pokeCmd.help.append(" Usage: poke [variable index newvalue]");
    pokeCmd.fnc = &HcomHandler::executePokeCommand;
    pokeCmd.group = "Variable Commands";
    mCmdList << pokeCmd;

    HcomCommand aliasCmd;
    aliasCmd.cmd = "alias";
    aliasCmd.description.append("Defines an alias for a variable");
    aliasCmd.help.append(" Usage: alias [variable alias]");
    aliasCmd.fnc = &HcomHandler::executeDefineAliasCommand;
    aliasCmd.group = "Variable Commands";
    mCmdList << aliasCmd;

    HcomCommand rmvarCmd;
    rmvarCmd.cmd = "rmvar";
    rmvarCmd.description.append("Removes specified variable");
    rmvarCmd.help.append(" Usage: rmvar [variable]");
    rmvarCmd.fnc = &HcomHandler::executeRemoveVariableCommand;
    rmvarCmd.group = "Variable Commands";
    mCmdList << rmvarCmd;

    HcomCommand chdfscCmd;
    chdfscCmd.cmd = "chdfsc";
    chdfscCmd.description.append("Change default plot scale of specified variable");
    chdfscCmd.help.append(" Usage: chdfsc [variable scale]");
    chdfscCmd.fnc = &HcomHandler::executeChangeDefaultPlotScaleCommand;
    chdfscCmd.group = "Variable Commands";
    mCmdList << chdfscCmd;

    HcomCommand didfscCmd;
    didfscCmd.cmd = "didfsc";
    didfscCmd.description.append("Display default plot scale of specified variable");
    didfscCmd.help.append(" Usage: didfsc [variable]");
    didfscCmd.fnc = &HcomHandler::executeDisplayDefaultPlotScaleCommand;
    didfscCmd.group = "Variable Commands";
    mCmdList << didfscCmd;

    HcomCommand chscCmd;
    chscCmd.cmd = "chsc";
    chscCmd.description.append("Change plot scale of specified variable");
    chscCmd.help.append(" Usage: chsc [variable scale]");
    chscCmd.fnc = &HcomHandler::executeChangePlotScaleCommand;
    chscCmd.group = "Variable Commands";
    mCmdList << chscCmd;

    HcomCommand discCmd;
    discCmd.cmd = "disc";
    discCmd.description.append("Display plot scale of specified variable");
    discCmd.help.append(" Usage: disc [variable]");
    discCmd.fnc = &HcomHandler::executeDisplayPlotScaleCommand;
    discCmd.group = "Variable Commands";
    mCmdList << discCmd;

    HcomCommand dlogCmd;
    dlogCmd.cmd = "dlog";
    dlogCmd.description.append("Disables logging in specified ports");
    dlogCmd.help.append(" Usage: dlog [ports]");
    dlogCmd.fnc = &HcomHandler::executeDisableLoggingCommand;
    dlogCmd.group = "Variable Commands";
    mCmdList << dlogCmd;

    HcomCommand elogCmd;
    elogCmd.cmd = "elog";
    elogCmd.description.append("Enables logging in specified ports");
    elogCmd.help.append(" Usage: elog [ports]");
    elogCmd.fnc = &HcomHandler::executeEnableLoggingCommand;
    elogCmd.group = "Variable Commands";
    mCmdList << elogCmd;

    HcomCommand setCmd;
    setCmd.cmd = "set";
    setCmd.description.append("Sets Hopsan preferences");
    setCmd.help.append(" Usage: set [preference value]\n");
    setCmd.help.append(" Available commands:\n");
    setCmd.help.append("  multicore [on/off]\n");
    setCmd.help.append("  threads [number]\n");
    setCmd.help.append("  cachetodisk [on/off]\n");
    setCmd.help.append("  generationlimit [number]\n");
    setCmd.help.append("  samples [number]");
    setCmd.fnc = &HcomHandler::executeSetCommand;
    mCmdList << setCmd;

    HcomCommand saplCmd;
    saplCmd.cmd = "sapl";
    saplCmd.description.append("Save log variables to file");
    saplCmd.help.append(" Usage: sapl [filepath -flags variables]");
    saplCmd.help.append("\nFlags:");
    saplCmd.help.append("\n-csv    Use CSV format (default is PLO format)");
    saplCmd.fnc = &HcomHandler::executeSaveToPloCommand;
    saplCmd.group = "Plot Commands";
    mCmdList << saplCmd;

    HcomCommand replCmd;
    replCmd.cmd = "repl";
    replCmd.description.append("Loads plot files from .CSV or .PLO");
    replCmd.help.append(" Usage: repl [filepath]");
    replCmd.fnc = &HcomHandler::executeLoadVariableCommand;
    replCmd.group = "Plot Commands";
    mCmdList << replCmd;

    HcomCommand sapaCmd;
    sapaCmd.cmd = "sapa";
    sapaCmd.description.append("Saves parameter set to .XML");
    sapaCmd.help.append(" Usage: sapa [filepath]");
    sapaCmd.fnc = &HcomHandler::executeSaveParametersCommand;
    sapaCmd.group = "Parameter Commands";
    mCmdList << sapaCmd;

    HcomCommand repaCmd;
    repaCmd.cmd = "repa";
    repaCmd.description.append("Loads parameters from .XML");
    repaCmd.help.append(" Usage: repa [filepath]");
    repaCmd.fnc = &HcomHandler::executeLoadParametersCommand;
    repaCmd.group = "Parameter Commands";
    mCmdList << repaCmd;

    HcomCommand loadCmd;
    loadCmd.cmd = "load";
    loadCmd.description.append("Loads a model file");
    loadCmd.help.append(" Usage: load [filepath variables]");
    loadCmd.fnc = &HcomHandler::executeLoadModelCommand;
    loadCmd.group = "Model Commands";
    mCmdList << loadCmd;

    HcomCommand loadrCmd;
    loadrCmd.cmd = "loadr";
    loadrCmd.description.append("Loads most recent model file");
    loadrCmd.help.append(" Usage: loadr [no arguments]");
    loadrCmd.fnc = &HcomHandler::executeLoadRecentCommand;
    loadrCmd.group = "Model Commands";
    mCmdList << loadrCmd;

    HcomCommand recoCmd;
    recoCmd.cmd = "reco";
    recoCmd.description.append("Renames a component");
    recoCmd.help.append(" Usage: reco [oldname] [newname]");
    recoCmd.fnc = &HcomHandler::executeRenameComponentCommand;
    recoCmd.group = "Model Commands";
    mCmdList << recoCmd;

    HcomCommand rmcoCmd;
    rmcoCmd.cmd = "rmco";
    rmcoCmd.description.append("Removes specified component(s)");
    rmcoCmd.help.append(" Usage: rmco [component]");
    rmcoCmd.fnc = &HcomHandler::executeRemoveComponentCommand;
    rmcoCmd.group = "Model Commands";
    mCmdList << rmcoCmd;

    HcomCommand pwdCmd;
    pwdCmd.cmd = "pwd";
    pwdCmd.description.append("Displays present working directory");
    pwdCmd.help.append(" Usage: pwd [no arguments]");
    pwdCmd.fnc = &HcomHandler::executePwdCommand;
    pwdCmd.group = "File Commands";
    mCmdList << pwdCmd;

    HcomCommand mwdCmd;
    mwdCmd.cmd = "mwd";
    mwdCmd.description.append("Displays working directory of current model");
    mwdCmd.help.append(" Usage: mwd [no arguments]");
    mwdCmd.fnc = &HcomHandler::executeMwdCommand;
    mwdCmd.group = "File Commands";
    mCmdList << mwdCmd;

    HcomCommand cdCmd;
    cdCmd.cmd = "cd";
    cdCmd.description.append("Changes present working directory");
    cdCmd.help.append(" Usage: cd [directory]");
    cdCmd.fnc = &HcomHandler::executeChangeDirectoryCommand;
    cdCmd.group = "File Commands";
    mCmdList << cdCmd;

    HcomCommand lsCmd;
    lsCmd.cmd = "ls";
    lsCmd.description.append("List files in current directory");
    lsCmd.help.append(" Usage: ls [no arguments]");
    lsCmd.fnc = &HcomHandler::executeListFilesCommand;
    lsCmd.group = "File Commands";
    mCmdList << lsCmd;

    HcomCommand closeCmd;
    closeCmd.cmd = "close";
    closeCmd.description.append("Closes current model");
    closeCmd.help.append(" Usage: close [no arguments]");
    closeCmd.fnc = &HcomHandler::executeCloseModelCommand;
    mCmdList << closeCmd;

    HcomCommand chtabCmd;
    chtabCmd.cmd = "chtab";
    chtabCmd.description.append("Changes current model tab");
    chtabCmd.help.append(" Usage: chtab [index]");
    chtabCmd.fnc = &HcomHandler::executeChangeTabCommand;
    chtabCmd.group = "Model Commands";
    mCmdList << chtabCmd;

    HcomCommand adcoCmd;
    adcoCmd.cmd = "adco";
    adcoCmd.description.append("Adds a new component to current model");
    adcoCmd.help.append(" Usage: adco [typename name -flag value]");
    adcoCmd.fnc = &HcomHandler::executeAddComponentCommand;
    adcoCmd.group = "Model Commands";
    mCmdList << adcoCmd;

    HcomCommand cocoCmd;
    cocoCmd.cmd = "coco";
    cocoCmd.description.append("Connect components in current model");
    cocoCmd.help.append(" Usage: coco [comp1 port1 comp2 port2]");
    cocoCmd.fnc = &HcomHandler::executeConnectCommand;
    cocoCmd.group = "Model Commands";
    mCmdList << cocoCmd;

    HcomCommand crmoCmd;
    crmoCmd.cmd = "crmo";
    crmoCmd.description.append("Creates a new model");
    crmoCmd.help.append(" Usage: crmo [no arguments]");
    crmoCmd.fnc = &HcomHandler::executeCreateModelCommand;
    crmoCmd.group = "Model Commands";
    mCmdList << crmoCmd;

    HcomCommand fmuCmd;
    fmuCmd.cmd = "fmu";
    fmuCmd.description.append("Exports current model to Functional Mockup Unit (FMU)");
    fmuCmd.help.append(" Usage: fmu [path]");
    fmuCmd.fnc = &HcomHandler::executeExportToFMUCommand;
    mCmdList << fmuCmd;

    HcomCommand chtsCmd;
    chtsCmd.cmd = "chts";
    chtsCmd.description.append("Change time step of sub-component");
    chtsCmd.help.append(" Usage: chts [comp timestep]");
    chtsCmd.fnc = &HcomHandler::executeChangeTimestepCommand;
    chtsCmd.group = "Simulation Commands";
    mCmdList << chtsCmd;

    HcomCommand intsCmd;
    intsCmd.cmd = "ints";
    intsCmd.description.append("Inherit time step of sub-component from system time step");
    intsCmd.help.append(" Usage: ints [comp]");
    intsCmd.fnc = &HcomHandler::executeInheritTimestepCommand;
    intsCmd.group = "Simulation Commands";
    mCmdList << intsCmd;

    HcomCommand bodeCmd;
    bodeCmd.cmd = "bode";
    bodeCmd.description.append("Creates a bode plot from specified curves");
    bodeCmd.help.append(" Usage: bode [invar outvar maxfreq]");
    bodeCmd.fnc = &HcomHandler::executeBodeCommand;
    bodeCmd.group = "Plot Commands";
    mCmdList << bodeCmd;

    HcomCommand absCmd;
    absCmd.cmd = "abs";
    absCmd.description.append("Calculates absolute value of scalar of variable");
    absCmd.help.append(" Usage: abs [var]");
    absCmd.fnc = &HcomHandler::executeAbsCommand;
    absCmd.group = "Variable Commands";
    mCmdList << absCmd;

    HcomCommand optCmd;
    optCmd.cmd = "opt";
    optCmd.description.append("Initialize an optimization");
    optCmd.help.append(" Usage: opt [algorithm partype parnum parmin parmax -flags]");
    optCmd.help.append("\nAlgorithms:   Flags:");
    optCmd.help.append("\ncomplex       alpha");
    optCmd.fnc = &HcomHandler::executeOptimizationCommand;
    mCmdList << optCmd;

    HcomCommand callCmd;
    callCmd.cmd = "call";
    callCmd.description.append("Calls a pre-defined function");
    callCmd.help.append(" Usage: call [funcname]");
    callCmd.fnc = &HcomHandler::executeCallFunctionCommand;
    mCmdList << callCmd;

    HcomCommand echoCmd;
    echoCmd.cmd = "echo";
    echoCmd.description.append("Sets terminal output on or off");
    echoCmd.help.append(" Usage: echo [on/off]");
    echoCmd.fnc = &HcomHandler::executeEchoCommand;
    mCmdList << echoCmd;

    HcomCommand editCmd;
    editCmd.cmd = "edit";
    editCmd.description.append("Open file in external editor");
    editCmd.help.append(" Usage: edit [filepath]");
    editCmd.fnc = &HcomHandler::executeEditCommand;
    editCmd.group = "File Commands";
    mCmdList << editCmd;

    HcomCommand semtCmd;
    semtCmd.cmd = "semt";
    semtCmd.description.append("Applies low-pass filter of first degree to vector");
    semtCmd.help.append(" Usage: semt [on/off threads algorithm]");
    semtCmd.fnc = &HcomHandler::executeSetMultiThreadingCommand;
    mCmdList << semtCmd;
}

void HcomHandler::generateCommandsHelpText()
{
    QString output;

    QStringList groups;
    for(int c=0; c<mCmdList.size(); ++c)
    {
        if(!groups.contains(mCmdList[c].group))
        {
            groups << mCmdList[c].group;
        }
    }
    groups.removeAll("");
    groups << "";
    for(int g=0; g<groups.size(); ++g)
    {
        if(groups[g].isEmpty())
        {
            output.append("\\section othercommands Other Commands\n\n");
        }
        else
        {
            output.append("\\section "+groups[g].toLower().replace(" ","")+" "+groups[g]+"\n\n");
        }
        for(int c=0; c<mCmdList.size(); ++c)
        {
            if(mCmdList[c].group == groups[g])
            {
                output.append("\\subsection "+mCmdList[c].cmd+" "+mCmdList[c].cmd+"\n");
                output.append(mCmdList[c].description.replace(">>", "\\>\\>")+"<br>\n");
                output.append(mCmdList[c].help.replace(">>", "\\>\\>").replace("\n","\n<br>")+"\n\n");
            }
        }
    }

    output.append("\\section functions Local Functions\n\n");
    QMapIterator<QString,QPair<QString, QString> > fit(mLocalFunctionDescriptions);
    while(fit.hasNext())
    {
        fit.next();
        output.append("\\subsection "+fit.key().toLower()+" "+fit.key()+"()\n");
        output.append(fit.value().first+"\n");
        output.append(fit.value().second+"\n\n");
    }

    mpConsole->print(output);
}


//! @brief Returns a list of available commands
QStringList HcomHandler::getCommands() const
{
    QStringList ret;
    for(int i=0; i<mCmdList.size(); ++i)
    {
        ret.append(mCmdList.at(i).cmd);
    }
    return ret;
}


//! @brief Returns a map with all local variables and their values
HcomHandler::LocalVarsMapT HcomHandler::getLocalVariables() const
{
    return mLocalVars;
}

void HcomHandler::setLocalVariables(const HcomHandler::LocalVarsMapT &vars)
{
    mLocalVars = vars;
}


//! @brief Returns a map with all local functions and pointers to them
QMap<QString, SymHopFunctionoid*> HcomHandler::getLocalFunctionoidPointers() const
{
    return mLocalFunctionoidPtrs;
}

//! @brief Executes a HCOM command
//! @param cmd The command entered by user
void HcomHandler::executeCommand(QString cmd)
{
    cmd = cmd.simplified();

    QString majorCmd = cmd.split(" ").first();
    QString subCmd;
    if(cmd.split(" ").size() == 1)
    {
        subCmd = QString();
    }
    else
    {
        subCmd = cmd.right(cmd.size()-majorCmd.size()-1);
    }

    int idx = -1;
    for(int i=0; i<mCmdList.size(); ++i)
    {
        if(mCmdList[i].cmd == majorCmd) { idx = i; }
    }

    if(idx<0)
    {
        //TicToc timer;
        if(!evaluateArithmeticExpression(cmd))
        {
            //! @todo this text is to generic, a better error should be given someplace else, is it an unknown command or what?
            HCOMERR("Unknown command or failed to evaluate: " + cmd);
        }
        //timer.toc("evaluateArithmeticExpression " + cmd);
    }
    else
    {
        //TicToc timer;
        mCmdList[idx].runCommand(subCmd, this);
        //timer.toc("runCommand "+QString("(%1)  %2").arg(majorCmd).arg(subCmd));
    }
}


//! @brief Execute function for "exit" command
void HcomHandler::executeExitCommand(const QString /*cmd*/)
{
    gpMainWindowWidget->close();
}


//! @brief Execute function for "sim" command
void HcomHandler::executeSimulateCommand(const QString cmd)
{
    QStringList splitCmd;
    splitWithRespectToQuotations(cmd, ' ', splitCmd);
    if(splitCmd.size() > 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    else if(splitCmd.size() == 1 && splitCmd[0] == "all")
    {
        gpModelHandler->simulateAllOpenModels_blocking(false);
        return;
    }
    else if(cmd == "")
    {
        //TicToc timer;
        //timer.tic("!!!! Beginning blocking simulation");
        if(mpModel)
        {
            mpModel->simulate_blocking();
        }
        //timer.toc("!!!! Blocking simulation");
    }
    else
    {
        HCOMERR("Unknown argument.");
        return;
    }
}


//! @brief Execute function for "chpv" command
void HcomHandler::executePlotCommand(const QString cmd)
{
    changePlotVariables(cmd, -1);
}



//! @brief Execute function for "chpvl" command
void HcomHandler::executePlotLeftAxisCommand(const QString cmd)
{
    changePlotVariables(cmd, 0);
}


//! @brief Execute function for "chpvr" command
void HcomHandler::executePlotRightAxisCommand(const QString cmd)
{
    changePlotVariables(cmd, 1);
}


//! @brief Execute function for "adpv" command
void HcomHandler::executeAddPlotCommand(const QString cmd)
{
    changePlotVariables(cmd, -1, true);
}


//! @brief Execute function for "adpvl" command
void HcomHandler::executeAddPlotLeftAxisCommand(const QString cmd)
{
    changePlotVariables(cmd, 0, true);
}


//! @brief Execute function for "adpvr" command
void HcomHandler::executeAddPlotRightAxisCommand(const QString cmd)
{
    changePlotVariables(cmd, 1, true);
}


//! @brief Execute function for "dipa" command
void HcomHandler::executeDisplayParameterCommand(const QString cmd)
{
    QStringList parameters;
    getParameters(cmd, parameters);

    int longestParameterName=0;
    for(int p=0; p<parameters.size(); ++p)
    {
        if(parameters.at(p).size() > longestParameterName)
        {
            longestParameterName = parameters.at(p).size();
        }
    }


    for(int p=0; p<parameters.size(); ++p)
    {
        QString output = parameters[p];
        int space = longestParameterName-parameters[p].size()+3;
        for(int s=0; s<space; ++s)
        {
            output.append(" ");
        }
        output.append(getParameterValue(parameters[p]));
        HCOMPRINT(output);
    }
}


void HcomHandler::executeAddParameterCommand(const QString cmd)
{
    QStringList splitCmd;
    splitWithRespectToQuotations(cmd, ' ', splitCmd);
    if(splitCmd.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    ContainerObject *pContainer = mpModel->getViewContainerObject();
    if(pContainer)
    {
        CoreParameterData paramData(splitCmd[0], splitCmd[1], "double");
        pContainer->setOrAddParameter(paramData);
    }
}


//! @brief Execute function for "chpa" command
void HcomHandler::executeChangeParameterCommand(const QString cmd)
{
    QStringList splitCmd;
    splitWithRespectToQuotations(cmd, ' ', splitCmd);
//    QStringList splitCmd;
//    bool withinQuotations = false;
//    int start=0;
//    for(int i=0; i<cmd.size(); ++i)
//    {
//        if(cmd[i] == '\"')
//        {
//            withinQuotations = !withinQuotations;
//        }
//        if(cmd[i] == ' ' && !withinQuotations)
//        {
//            splitCmd.append(cmd.mid(start, i-start));
//            start = i+1;
//        }
//    }
//    splitCmd.append(cmd.right(cmd.size()-start));
//    for(int i=0; i<splitCmd.size(); ++i)
//    {
//        splitCmd[i].remove("\"");
//    }

    if(splitCmd.size() == 2)
    {
        if(!mpModel) { return; }
        SystemContainer *pSystem = mpModel->getTopLevelSystemContainer();
        if(!pSystem) { return; }

        QStringList parameterNames;
        getParameters(splitCmd[0], parameterNames);

        evaluateExpression(splitCmd[1], Scalar);
        if(mAnsType != Scalar)
        {
            HCOMERR("Could not evaluate new value for parameter.");
            return;
        }
        double newValue = mAnsScalar;

        if(parameterNames.isEmpty())
        {
            HCOMERR("Parameter(s) not found.");
            return;
        }

        int nChanged=0;
        QString newValueStr = QString::number(newValue);
        for(int p=0; p<parameterNames.size(); ++p)
        {
            if(pSystem->getParameterNames().contains(parameterNames[p]))
            {
                CoreParameterData data;
                pSystem->getParameter(parameterNames[p], data);     //Convert 1 to true and 0 to false in case of boolean parameters
                if(data.mType == "bool")
                {
                    if(newValueStr == "0") newValueStr = "false";
                    else if(newValueStr == "1") newValueStr = "true";
                }
                if(pSystem->setParameterValue(parameterNames[p], newValueStr))
                    ++nChanged;
            }
            else if(!pSystem->getFullNameFromAlias(parameterNames[p]).isEmpty())
            {
                QString nameFromAlias = pSystem->getFullNameFromAlias(parameterNames[p]);
                QString compName = nameFromAlias.section("#",0,0);
                QString parName = nameFromAlias.right(nameFromAlias.size()-compName.size()-1);
                ModelObject *pComponent = pSystem->getModelObject(compName);
                if(pComponent)
                {
                    CoreParameterData data;
                    pComponent->getParameter(parameterNames[p], data);     //Convert 1 to true and 0 to false in case of boolean parameters
                    if(data.mType == "bool")
                    {
                        if(newValueStr == "0") newValueStr = "false";
                        else if(newValueStr == "1") newValueStr = "true";
                    }
                    if(pComponent->setParameterValue(parName, QString::number(newValue)))
                        ++nChanged;
                }
            }
            else
            {
                parameterNames[p].remove("\"");
                QStringList splitFirstCmd = parameterNames[p].split(".");
                if(splitFirstCmd.size() == 3)
                {
                    QString parName = splitFirstCmd[1]+"."+splitFirstCmd[2];
                    splitFirstCmd.removeLast();
                    splitFirstCmd[1] = parName;
                }
                if(splitFirstCmd.size() == 2)
                {
                    QList<ModelObject*> components;
                    getComponents(splitFirstCmd[0], components);
                    for(int c=0; c<components.size(); ++c)
                    {
                        QStringList parameters;
                        getParameters(splitFirstCmd[1], components[c], parameters);
                        for(int p=0; p<parameters.size(); ++p)
                        {
                            if(pSystem->getParameterNames().contains(QString::number(newValue))) //System parameter
                            {
                                if(components[c]->setParameterValue(parameters[p], QString::number(newValue)))
                                    ++nChanged;
                            }
                            else
                            {
                                toLongDataNames(parameters[p]);
                                evaluateExpression(QString::number(newValue), Scalar);
                                if(mAnsType == Scalar && components[c]->setParameterValue(parameters[p], QString::number(mAnsScalar)))
                                    ++nChanged;
                            }
                        }
                    }
                }
            }
        }
        int nFailed = parameterNames.size()-nChanged;
        if(nChanged>0)
            HCOMPRINT("Changed value for "+QString::number(nChanged)+" parameters.");
        if(nFailed>0)
            HCOMERR("Failed to change value for "+QString::number(parameterNames.size())+" parameters.");
    }
    else
    {
        HCOMERR("Wrong number of arguments.");
    }
}


//! @brief Execute function for "chss" command
void HcomHandler::executeChangeSimulationSettingsCommand(const QString cmd)
{
    QString temp = cmd;
    temp.remove("\"");
    QStringList splitCmd = temp.split(" ");
    if(splitCmd.size() == 3 || splitCmd.size() == 4)
    {
        bool allOk=true;
        bool ok;
        double startT = getNumber(splitCmd[0], &ok);
        if(!ok) { allOk=false; }
        double timeStep = getNumber(splitCmd[1], &ok);
        if(!ok) { allOk=false; }
        double stopT = getNumber(splitCmd[2], &ok);
        if(!ok) { allOk=false; }

        SystemContainer *pCurrentSystem = mpModel->getTopLevelSystemContainer();
        int samples = pCurrentSystem->getNumberOfLogSamples();
        if(splitCmd.size() == 4)
        {
            samples = splitCmd[3].toInt(&ok);
            if(!ok) { allOk=false; }
        }

        if(allOk)
        {
            if(!mpModel) { return; }
            mpModel->setTopLevelSimulationTime(QString::number(startT), QString::number(timeStep), QString::number(stopT));
            if(splitCmd.size() == 4)
            {
                if(!pCurrentSystem) { return; }
                pCurrentSystem->setNumberOfLogSamples(samples);
            }
        }
        else
        {
            HCOMERR("Failed to apply simulation settings.");
        }
    }
    else
    {
        HCOMERR("Wrong number of arguments.");
    }
}


//! @brief Execute function for "help" command
void HcomHandler::executeHelpCommand(const QString cmd)
{
    QString temp=cmd;
    temp.remove(" ");
    if(temp.isEmpty())
    {
        HCOMPRINT("-------------------------------------------------------------------------");
        HCOMPRINT(" Hopsan HCOM Terminal v0.1");
        QString commands;
        int n=0;
        QStringList groups;
        for(int c=0; c<mCmdList.size(); ++c)
        {
            n=max(mCmdList[c].cmd.size(), n);
            if(!groups.contains(mCmdList[c].group))
            {
                groups << mCmdList[c].group;
            }
        }
        n=n+4;
        groups.removeAll("");
        groups << "";
        for(int g=0; g<groups.size(); ++g)
        {
            if(groups[g].isEmpty())
            {
                commands.append("\n Other commands:\n\n");
            }
            else
            {
                commands.append("\n "+groups[g]+":\n\n");
            }
            for(int c=0; c<mCmdList.size(); ++c)
            {
                if(mCmdList[c].group == groups[g])
                {
                    commands.append("   ");
                    commands.append(mCmdList[c].cmd);
                    for(int i=0; i<n-mCmdList[c].cmd.size(); ++i)
                    {
                        commands.append(" ");
                    }
                    commands.append(mCmdList[c].description);
                    commands.append("\n");
                }
            }
        }

        //Print help descriptions for local functions
        commands.append("\n Custom Functions:\n\n");
        QMapIterator<QString, QPair<QString,QString> > funcIt(mLocalFunctionDescriptions);
        while(funcIt.hasNext())
        {
            funcIt.next();
            commands.append("   "+funcIt.key()+"()");
            for(int i=0; i<n-funcIt.key().size()-2; ++i)
            {
                commands.append(" ");
            }
            commands.append(funcIt.value().first+"\n");
        }

        HCOMPRINT(commands);
        HCOMPRINT(" Type: \"help [command]\" for more information about a specific command.");
        HCOMPRINT("-------------------------------------------------------------------------");
    }
    else if(temp == "doxygen")
    {
        generateCommandsHelpText();
        return;
    }
    else if(temp.endsWith("()") && mLocalFunctionDescriptions.contains(temp.remove("()")))
    {
        QString description = mLocalFunctionDescriptions.find(temp).value().first;
        QString help = mLocalFunctionDescriptions.find(temp).value().second;

        QStringList helpLines = help.split("\n");
        int helpLength=0;
        Q_FOREACH(const QString &line, helpLines)
        {
            if(line.size() > helpLength)
                helpLength = line.size();
        }
        int length=max(description.size(), helpLength)+2;
        QString delimiterLine;
        for(int i=0; i<length; ++i)
        {
            delimiterLine.append("-");
        }
        QString descLine = description;
        descLine.prepend(" ");
        QString helpLine = help;
        helpLine.prepend(" ");
        helpLine.replace("\n", "\n ");
        HCOMPRINT(delimiterLine+"\n"+descLine+"\n"+helpLine+"\n"+delimiterLine);
    }
    else
    {
        int idx = -1;
        for(int i=0; i<mCmdList.size(); ++i)
        {
            if(mCmdList[i].cmd == temp) { idx = i; }
        }

        if(idx < 0)
        {
            HCOMERR("Command not found or no help available for this command.");
        }
        else
        {
            QStringList helpLines = mCmdList[idx].help.split("\n");
            int helpLength=0;
            Q_FOREACH(const QString &line, helpLines)
            {
                if(line.size() > helpLength)
                    helpLength = line.size();
            }
            int length=max(mCmdList[idx].description.size(), helpLength)+2;
            QString delimiterLine;
            for(int i=0; i<length; ++i)
            {
                delimiterLine.append("-");
            }
            QString descLine = mCmdList[idx].description;
            descLine.prepend(" ");
            QString helpLine = mCmdList[idx].help;
            helpLine.prepend(" ");
            helpLine.replace("\n", "\n ");
            HCOMPRINT(delimiterLine+"\n"+descLine+"\n"+helpLine+"\n"+delimiterLine);
        }
    }
}


//! @brief Execute function for "exec" command
void HcomHandler::executeRunScriptCommand(const QString cmd)
{
    QStringList args = getArguments(cmd);
    if(args.size() < 1)
    {
        HCOMERR("Too few arguments.");
        return;
    }

    QString path = args[0];
    path.remove("\"");
    path.replace("\\","/");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        file.setFileName(path+".hcom");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            path.prepend(mPwd+"/");
            dir = path.left(path.lastIndexOf("/"));
            dir = getDirectory(dir);
            path = dir+path.right(path.size()-path.lastIndexOf("/"));
            file.setFileName(path);
            if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                file.setFileName(path+".hcom");
                if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    HCOMERR("Unable to read file.");
                    return;
                }
            }
        }
    }

    QString code;

    QTextStream t(&file);

    code = t.readAll();

    for(int i=0; i<args.size()-1; ++i)  //Replace arguments with their values
    {
        QString str = "$"+QString::number(i+1);
        code.replace(str, args[i+1]);
    }

    if(!mAcceptsOptimizationCommands && (code.contains("\nopt init") || code.contains("\nopt run") || code.contains(" opt init") || code.contains(" opt run")))
    {
        //HCOMINFO("This HCOM terminal does not accept optimization scripts.\nUse the optimization dialog instead.");
        //return;

        gpOptimizationDialog->open();
        gpOptimizationDialog->setCode(code);
        return;
    }

    QStringList lines = code.split("\n");
    lines.removeAll("");
    bool *abort = new bool;
    *abort = false;
    QString gotoLabel = runScriptCommands(lines, abort);
    if(*abort)
    {
        delete abort;
        return;
    }
    delete abort;
    while(!gotoLabel.isEmpty())
    {
        if(gotoLabel == "%%%%%EOF")
        {
            break;
        }
        for(int l=0; l<lines.size(); ++l)
        {
            if(lines[l].startsWith("&"+gotoLabel))
            {
                QStringList commands = lines.mid(l, lines.size()-l);
                bool *abort = new bool;
                gotoLabel = runScriptCommands(commands, abort);
            }
        }
    }

    file.close();
}


//! @brief Execute function for "wrhi" command
void HcomHandler::executeWriteHistoryToFileCommand(const QString cmd)
{
    if(!mpConsole) return;

    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString path = getArgument(cmd,0);
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));


    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        HCOMERR("Unable to write to file.");
        return;
    }

    QTextStream t(&file);
    for(int h=mpConsole->mHistory.size()-1; h>-1; --h)
    {
        t << mpConsole->mHistory[h] << "\n";
    }
    file.close();
}


//! @brief Execute function for "print" command
void HcomHandler::executePrintCommand(const QString cmd)
{
    if(cmd.isEmpty())
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString str = cmd;

    QString arg = getArgument(str,0);
    if(arg == "-e" || arg == "-w" || arg == "-i")
    {
        str.remove(0,3);
    }

    if(!str.startsWith("\"") || !str.endsWith("\""))
    {
        HCOMERR("Expected a string enclosed in \" \".");
        return;
    }

    int failed=0;
    while(str.count("$") > 1+failed*2)
    {
        QString varName = str.section("$",1+failed,1+failed);
        evaluateExpression(varName);
        if(mAnsType == Scalar)
        {
            str.replace("$"+varName+"$", QString::number(mAnsScalar));
        }
        else if (mAnsType == DataVector)
        {
            QString array;
            QTextStream ts(&array);
            mAnsVector->sendDataToStream(ts," ");
            str.replace("$"+varName+"$", array);
        }
        else
        {
            ++failed;
        }
    }

    str = str.mid(1,str.size()-2);
    if(arg == "-e")
    {
        HCOMERR(str);
    }
    else if(arg == "-w")
    {
        HCOMWARN(str);
    }
    else if(arg == "-i")
    {
        HCOMINFO(str);
    }
    else
    {
        HCOMPRINT(str);
    }
}


//! @brief Execute function for "chpw" command
void HcomHandler::executeChangePlotWindowCommand(const QString cmd)
{
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    mpCurrentPlotWindow = gpPlotHandler->createNewPlotWindowOrGetCurrentOne(cmd);
}


//! @brief Execute function for "dipw" command
void HcomHandler::executeDisplayPlotWindowCommand(const QString /*cmd*/)
{
    if (mpCurrentPlotWindow)
    {
        HCOMPRINT(mpCurrentPlotWindow->getName());
    }
    else
    {
        HCOMPRINT("Current plotwindow not set");
    }
}


//! @brief Execute function for "disp" command
void HcomHandler::executeDisplayVariablesCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) < 2)
    {
        QStringList output;
        if(cmd.isEmpty())
        {
            getMatchingLogVariableNames("*"GenerationSpecifierChar"H", output);
        }
        else
        {
            getMatchingLogVariableNames(cmd, output);
        }

        for(int o=0; o<output.size(); ++o)
        {
            HCOMPRINT(output[o]);
        }
    }
    else
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
}


//! @brief Execute function for "peek" command
void HcomHandler::executePeekCommand(const QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString variable = split.first();
    bool ok;
    int id = int(getNumber(split.last(), &ok)+0.01);
    if(!ok)
    {
        HCOMERR(QString("Illegal index value: %1").arg(split.last()));
        return;
    }

    SharedVectorVariableT pData = getLogVariable(variable).mpVariable;
    if(pData)
    {
        QString err;
        double r = pData->peekData(id, err);
        if (err.isEmpty())
        {
            HCOMPRINT(QString::number(r));
            mAnsType = Scalar;
            mAnsScalar = r;
            return;
        }
        else
        {
            HCOMERR(err);
            mAnsType = Undefined;
            return;
        }
    }
    HCOMERR(QString("Data variable: %1 not found").arg(variable));
    mAnsType = Undefined;
    return;
}


//! @brief Execute function for "poke" command
void HcomHandler::executePokeCommand(const QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 3)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString variable = split.first();
    bool ok1, ok2;
    int id = int(getNumber(split[1], &ok1)+0.01);
    double value = getNumber(split.last(), &ok2);
    if(!ok1 || !ok2)
    {
        HCOMERR("Illegal value or index!");
        return;
    }

    SharedVectorVariableT pData = getLogVariable(variable).mpVariable;
    if(pData)
    {
        QString err;
        double r = pData->pokeData(id, value, err);
        if (err.isEmpty())
        {
            HCOMPRINT(QString::number(r));
        }
        else
        {
            HCOMERR(err);
        }
    }
    else
    {
        HCOMERR("Data variable not found.");
    }
    return;
}


//! @brief Execute function for "alias" command
void HcomHandler::executeDefineAliasCommand(const QString cmd)
{
    QStringList splitCmd;
    splitWithRespectToQuotations(cmd, ' ', splitCmd);
    if(splitCmd.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString variable = splitCmd[0];
    QString alias = splitCmd[1];
    variable.remove("\"");
    toLongDataNames(variable);
    if(mpModel->getTopLevelSystemContainer()->setVariableAlias(variable, alias))
    {
        HCOMINFO(QString("Sucessfully assigned variable alias %1").arg(alias));
    }
    else
    {
        HCOMERR(QString("Failed to assign variable alias %1").arg(alias));
    }
}

void HcomHandler::executeRemoveVariableCommand(const QString cmd)
{
    QStringList args = getArguments(cmd);
    for(int s=0; s<args.size(); ++s)
    {
        QStringList variables;
        getMatchingLogVariableNames(args[s], variables);
        for(int v=0; v<variables.size(); ++v)
        {
            //! @todo it would be  nice if we could remove a variable directly if no generation information is specified
            removeLogVariable(variables[v]);
        }
    }
}

void HcomHandler::executeChangeDefaultPlotScaleCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString scale = getArgument(cmd,1);

    QStringList vars;
    getMatchingLogVariableNames(getArgument(cmd,0),vars);
    if(vars.isEmpty())
    {
        getMatchingLogVariableNamesWithoutLogDataHandler(getArgument(cmd,0),vars);
    }
    int nChanged=0;
    Q_FOREACH(const QString var, vars)
    {
        QStringList fields = var.split(".");
        // Handle alias
        if (fields.size() == 1)
        {
            QString fullName = getfullNameFromAlias(fields.first());
            toShortDataNames(fullName);
            fields = fullName.split(".");
        }

        if (fields.size() == 3)
        {
            ModelObject *pComponent = mpModel->getTopLevelSystemContainer()->getModelObject(fields.first());
            if(!pComponent)
            {
                HCOMERR(QString("Component not found: %1").arg(fields.first()));
            }

            QString description = "";
            QString longVarName = fields[1]+"."+fields[2];
            toLongDataNames(longVarName);
            if(pComponent->getPort(fields[1]) && NodeInfo(pComponent->getPort(fields[1])->getNodeType()).shortNames.contains(fields[2]))
            {
                pComponent->registerCustomPlotUnitOrScale(longVarName, description, scale);
                ++nChanged;
            }
        }
        else
        {
            HCOMERR(QString("Unknown variable: %1").arg(var));
        }
    }

    if(nChanged > 0)
    {
        HCOMINFO(QString("Changed default scale for %1 variables.").arg(nChanged));
    }
    else
    {
        HCOMERR(QString("No matching variables found."));
    }
}

void HcomHandler::executeDisplayDefaultPlotScaleCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QStringList vars;
    getMatchingLogVariableNames(cmd,vars);
    if(vars.isEmpty())
    {
        getMatchingLogVariableNamesWithoutLogDataHandler(cmd, vars);
    }
    if(vars.isEmpty())
    {
        HCOMERR(QString("Variable not found: %1").arg(cmd));
        mAnsType = Undefined;
        return;
    }
    Q_FOREACH(const QString var, vars)
    {
        QString dispName;
        QStringList fields = var.split(".");
        // Handle alias
        if (fields.size() == 1)
        {
            dispName = fields.first();
            QString fullName = getfullNameFromAlias(fields.first());
            fields = fullName.split("#");
        }

        // Handle comp.port.var variable
        if (fields.size() == 3)
        {
            // Only set dispName if it was not an alias (set above)
            if (dispName.isEmpty())
            {
                dispName = var;
            }

            ModelObject *pComponent = gpModelHandler->getCurrentTopLevelSystem()->getModelObject(fields.first());
            if(!pComponent)
            {
                HCOMERR(QString("Component not found: %1").arg(fields.first()));
                continue;
            }

            UnitScale unitScale;
            QString portAndVarName = fields[1]+"."+fields[2];
            toLongDataNames(portAndVarName);

            pComponent->getCustomPlotUnitOrScale(portAndVarName, unitScale);

            const QString &scale = unitScale.mScale;
            const QString &unit = unitScale.mUnit;
            if(!unit.isEmpty() && !scale.isEmpty())
            {
                HCOMPRINT(QString("%1: %2 [%3]").arg(dispName).arg(scale).arg(unit));
                mAnsType = Scalar;
                mAnsScalar = scale.toDouble();
                continue;
            }
            else if (!scale.isEmpty())
            {
                HCOMPRINT(QString("%1: %2").arg(dispName).arg(scale));
                mAnsType = Scalar;
                mAnsScalar = scale.toDouble();
                continue;
            }
            HCOMPRINT(QString("%1: 1").arg(dispName));
            mAnsType = Scalar;
            mAnsScalar = 1;
        }
        else
        {
            HCOMERR(QString("Unknown variable: %1").arg(var));
            mAnsType = Undefined;
        }
    }
}


void HcomHandler::executeChangePlotScaleCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString varName = getArgument(cmd,0);
    double scale = getArgument(cmd,1).toDouble();

    QStringList vars;
    getMatchingLogVariableNames(varName,vars);
    if(vars.isEmpty())
    {
        HCOMERR("Unknown variable: "+varName);
        return;
    }
    Q_FOREACH(const QString var, vars)
    {
        SharedVectorVariableT pVar = getLogVariable(var).mpVariable;
        pVar.data()->setPlotScale(scale);
    }
}


void HcomHandler::executeDisplayPlotScaleCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QStringList vars;
    getMatchingLogVariableNames(cmd,vars);
    if(vars.isEmpty())
    {
        HCOMERR("Unknown variable: "+cmd);
        return;
    }
    Q_FOREACH(const QString var, vars)
    {
        SharedVectorVariableT pVar = getLogVariable(var).mpVariable;
        if(!pVar.isNull())
        {
            QString scale = pVar.data()->getCustomUnitScale().mScale;
            QString unit = pVar.data()->getCustomUnitScale().mUnit;
            if(!scale.isEmpty() && !unit.isEmpty())
            {
                HCOMPRINT(QString("%1: %2 [%3]").arg(var).arg(scale).arg(unit));
                mAnsType = Scalar;
                mAnsScalar = scale.toDouble();
                continue;
            }
            else if(!scale.isEmpty())
            {
                HCOMPRINT(QString("%1: %2").arg(var).arg(scale));
                mAnsType = Scalar;
                mAnsScalar = scale.toDouble();
                continue;
            }
            else
            {
                scale = QString::number(pVar.data()->getPlotScale());
                unit = pVar.data()->getPlotScaleDataUnit();
                if(!scale.isEmpty() && !unit.isEmpty())
                {
                    HCOMPRINT(QString("%1: %2 [%3]").arg(var).arg(scale).arg(unit));
                    mAnsType = Scalar;
                    mAnsScalar = scale.toDouble();
                    continue;
                }
                else if(!scale.isEmpty())
                {
                    HCOMPRINT(QString("%1: %2").arg(var).arg(scale));
                    mAnsType = Scalar;
                    mAnsScalar = scale.toDouble();
                    continue;
                }
            }
            HCOMERR("Variable not found: "+var);
            mAnsType = Undefined;
        }
    }

    return;
}

void HcomHandler::executeDisableLoggingCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QList<Port*> vPortPtrs;
    getPorts(cmd, vPortPtrs);

    for(int p=0; p<vPortPtrs.size(); ++p)
    {
        mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setLoggingEnabled(vPortPtrs.at(p)->getParentModelObjectName(), vPortPtrs.at(p)->getName(), false);
    }
}

void HcomHandler::executeEnableLoggingCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QList<Port*> vPortPtrs;
    getPorts(cmd, vPortPtrs);

    for(int p=0; p<vPortPtrs.size(); ++p)
    {
        mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setLoggingEnabled(vPortPtrs.at(p)->getParentModelObjectName(), vPortPtrs.at(p)->getName(), true);
    }
}


//! @brief Execute function for "set" command
void HcomHandler::executeSetCommand(const QString cmd)
{
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString pref = splitCmd[0];
    QString value = splitCmd[1];

    if(pref == "multicore")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
            return;
        }
        getConfigPtr()->setUseMultiCore(value=="on");
    }
    else if(pref == "threads")
    {
        bool ok;
        int nThreads = value.toInt(&ok);
        if(!ok)
        {
            HCOMERR("Unknown value.");
            return;
        }
        getConfigPtr()->setNumberOfThreads(nThreads);
    }
    else if(pref == "algorithm")
    {
        bool ok;
        int algorithm = value.toInt(&ok);
        if(!ok)
        {
            HCOMERR("Unknown value.");
            return;
        }
        getConfigPtr()->setParallelAlgorithm(algorithm);
    }
    else if(pref == "cachetodisk")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        getConfigPtr()->setCacheLogData(value=="on");
    }
    else if(pref == "generationlimit")
    {
        bool ok;
        int limit = value.toInt(&ok);
        if(!ok)
        {
            HCOMERR("Unknown value.");
            return;
        }
        getConfigPtr()->setGenerationLimit(limit);
    }
    else if(pref == "samples")
    {
        bool ok;
        int samples = value.toInt(&ok);
        if(!ok)
        {
            HCOMERR("Unknown value.");
            return;
        }
        mpModel->getViewContainerObject()->setNumberOfLogSamples(samples);
    }
    else
    {
        HCOMERR("Unknown command.");
    }
}


//! @brief Execute function for "sapl" command
void HcomHandler::executeSaveToPloCommand(const QString cmd)
{
    QStringList args = getArguments(cmd);
    if(args.size() < 2)
    {
        HCOMERR("Too few arguments.");
        return;
    }

    bool useCsv = false;
    QString temp = cmd;
    if(args.contains("-csv"))
    {
        useCsv = true;
        temp.remove("-csv");
    }

    QString path = args.first();
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    temp = temp.right(temp.size()-path.size()-1);

    QStringList splitCmdMajor;
    bool withinQuotations = false;
    int start=0;
    for(int i=0; i<temp.size(); ++i)
    {
        if(temp[i] == '\"')
        {
            withinQuotations = !withinQuotations;
        }
        if(temp[i] == ' ' && !withinQuotations)
        {
            splitCmdMajor.append(temp.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmdMajor.append(temp.right(temp.size()-start));


    // Handle special case where we want to export a specific generation only
    // Example: *.g (g=15, g=l, g=h)
    QString specialCase = splitCmdMajor.last();
    if ( (splitCmdMajor.size() == 2) && specialCase.startsWith("*"GenerationSpecifierChar) )
    {
        bool parseOK;
        int g=parseAndChopGenerationSpecifier(specialCase, parseOK);
        if (parseOK)
        {
            if (g>=0)
            {
                mpModel->getTopLevelSystemContainer()->getLogDataHandler()->exportGenerationToPlo(path, g);
                return;
            }
            else
            {
                HCOMERR("sapl can not export different generations to the same file");
                return;
            }
        }
        else
        {
            HCOMERR("Could not parse generation specifier");
        }
    }

    // If we get here we should try to read all variable patterns explicitly
    QList<HopsanVariable> allVariables;
    for(int i=1; i<splitCmdMajor.size(); ++i)
    {
        splitCmdMajor[i].remove("\"");
        QStringList variables;
        getMatchingLogVariableNames(splitCmdMajor[i], variables);
        for(int v=0; v<variables.size(); ++v)
        {
            allVariables.append(getLogVariable(variables[v]));
        }
    }

    if(useCsv)
    {
        mpModel->getTopLevelSystemContainer()->getLogDataHandler()->exportToCSV(path, allVariables);
    }
    else
    {
        mpModel->getTopLevelSystemContainer()->getLogDataHandler()->exportToPlo(path, allVariables);
    }
}

void HcomHandler::executeLoadVariableCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString path = getArgument(cmd,0);
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));


    QFile file(path);
    if(!file.exists())
    {
        HCOMERR("File not found!");
        return;
    }

    bool csv;
    if(path.endsWith(".csv") || path.endsWith(".CSV"))
    {
        csv=true;
    }
    else if(path.endsWith(".plo") || path.endsWith(".PLO"))
    {
        csv=false;
    }
    else
    {
        HCOMWARN("Unknown file extension, assuming that it is a PLO file.");
        csv=false;
    }

    if(csv)
    {
        mpModel->getTopLevelSystemContainer()->getLogDataHandler()->importFromCSV_AutoFormat(path);
    }
    else
    {
        mpModel->getTopLevelSystemContainer()->getLogDataHandler()->importFromPlo(path);
    }
}

void HcomHandler::executeSaveParametersCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) > 1)
    {
        HCOMERR("Wrong numer of arguments.");
        return;
    }
    QString path = getArgument(cmd, 0);
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }

    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    mpModel->saveTo(path, ParametersOnly);
}

void HcomHandler::executeLoadParametersCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) > 1)
    {
        HCOMERR("Wrong numer of arguments.");
        return;
    }
    QString path = getArgument(cmd, 0);
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }

    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    qobject_cast<SystemContainer*>(mpModel->getViewContainerObject())->loadParameterFile(path);
}


//! @brief Execute function for "load" command
void HcomHandler::executeLoadModelCommand(const QString cmd)
{
    QString path = getArgument(cmd,0);
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    gpModelHandler->loadModel(path);
}


//! @brief Execute function for "loadr" command
void HcomHandler::executeLoadRecentCommand(const QString /*cmd*/)
{
    gpModelHandler->loadModel(getConfigPtr()->getRecentModels().first());
}


void HcomHandler::executeRenameComponentCommand(const QString cmd)
{
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    ContainerObject *pContainer = mpModel->getViewContainerObject();
    if(pContainer)
    {
        pContainer->renameModelObject(split[0], split[1]);
    }
}

void HcomHandler::executeRemoveComponentCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    QList<ModelObject*> components;
    getComponents(getArgument(cmd, 0), components);

    ContainerObject *pContainer = mpModel->getViewContainerObject();
    for(int c=0; c<components.size(); ++c)
    {
        pContainer->deleteModelObject(components[c]->getName());
    }

    if(!components.isEmpty())
    {
        pContainer->hasChanged();
    }
}


//! @brief Execute function for "pwd" command
void HcomHandler::executePwdCommand(const QString /*cmd*/)
{
    HCOMPRINT(mPwd);
}

void HcomHandler::executeMwdCommand(const QString /*cmd*/)
{
    if(mpModel)
    {
        HCOMPRINT(mpModel->getTopLevelSystemContainer()->getModelFileInfo().absoluteDir().path());
    }
    else
    {
        HCOMERR("No model is open.");
    }
}


//! @brief Execute function for "cd" command
void HcomHandler::executeChangeDirectoryCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    //Handle "cd mwd" command
    if(cmd == "mwd")
    {
        if(mpModel)
        {
            mPwd = mpModel->getTopLevelSystemContainer()->getModelFileInfo().absoluteDir().path();
            HCOMPRINT(mPwd);
        }
        return;
    }

    QDir newDirAbs(cmd);
    QDir newDirRel(mPwd+"/"+cmd);
    if(newDirAbs.exists() && cmd != ".." && cmd != ".")
    {
        mPwd = QDir().cleanPath(cmd);
    }
    else if(newDirRel.exists())
    {
        mPwd = QDir().cleanPath(mPwd+"/"+cmd);
    }
    else
    {
        HCOMERR("Illegal directory.");
        return;
    }

    HCOMPRINT(mPwd);
}


//! @brief Execute function for "ls" command
void HcomHandler::executeListFilesCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 0)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    QStringList contents;
    if(cmd.isEmpty())
    {
        contents = QDir(mPwd).entryList(QStringList() << "*");
    }
    else
    {
        contents = QDir(mPwd).entryList(QStringList() << cmd);
    }
    for(int c=0; c<contents.size(); ++c)
    {
        HCOMPRINT(contents[c]);
    }
}


//! @brief Execute function for "close" command
void HcomHandler::executeCloseModelCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 0)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    if(gpModelHandler->count() > 0)
    {
        gpModelHandler->closeModelByTabIndex(gpCentralTabWidget->currentIndex());
    }
}


//! @brief Execute function for "chtab" command
void HcomHandler::executeChangeTabCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    gpModelHandler->setCurrentModel(cmd.toInt());
}


//! @brief Execute function for "adco" command
void HcomHandler::executeAddComponentCommand(const QString cmd)
{
    if(!mpModel || getNumberOfArguments(cmd) < 5)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }
    QStringList args = getArguments(cmd);

    QString typeName = args[0];
    QString name = args[1];
    args.removeFirst();
    args.removeFirst();

    double xPos=0;
    double yPos=0;
    double rot=0;

    if(!args.isEmpty())
    {
        if(args.first() == "-a")
        {
            //Absolute
            if(args.size() != 4)
            {
                HCOMERR("Wrong number of arguments.");
                return;
            }
            xPos = args[1].toDouble();
            yPos = args[2].toDouble();
            rot = args[3].toDouble();
        }
        else if(args.first() == "-e")
        {
            //East of
            if(args.size() != 4)
            {
                HCOMERR("Wrong number of arguments.");
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(mpModel->getTopLevelSystemContainer()->getModelObject(otherName));
            if(!pOther)
            {
                HCOMERR("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x()+offset;
            yPos = pOther->getCenterPos().y();
            rot = args[3].toDouble();
        }
        else if(args.first() == "-w")
        {
            //West of
            if(args.size() != 4)
            {
                HCOMERR("Wrong number of arguments.");
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(mpModel->getTopLevelSystemContainer()->getModelObject(otherName));
            if(!pOther)
            {
                HCOMERR("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x()-offset;
            yPos = pOther->getCenterPos().y();
            rot = args[3].toDouble();
        }
        else if(args.first() == "-n")
        {
            //North of
            if(args.size() != 4)
            {
                HCOMERR("Wrong number of arguments.");
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(mpModel->getTopLevelSystemContainer()->getModelObject(otherName));
            if(!pOther)
            {
                HCOMERR("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x();
            yPos = pOther->getCenterPos().y()-offset;
            rot = args[3].toDouble();
        }
        else if(args.first() == "-s")
        {
            //South of
            if(args.size() != 4)
            {
                HCOMERR("Wrong number of arguments.");
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(mpModel->getTopLevelSystemContainer()->getModelObject(otherName));
            if(!pOther)
            {
                HCOMERR("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x();
            yPos = pOther->getCenterPos().y()+offset;
            rot = args[3].toDouble();
        }
        else
        {
            HCOMERR("Unknown argument: " +args.first());
            return;
        }
    }
    else
    {
        HCOMERR("Missing argument.");
        return;
    }

    QPointF pos = QPointF(xPos, yPos);
    Component *pObj = qobject_cast<Component*>(mpModel->getTopLevelSystemContainer()->addModelObject(typeName, pos, rot));
    if(!pObj)
    {
        HCOMERR("Failed to add new component. Incorrect typename?");
    }
    else
    {
        HCOMPRINT("Added "+typeName+" to current model.");
        mpModel->getTopLevelSystemContainer()->renameModelObject(pObj->getName(), name);
    }
}


//! @brief Execute function for "coco" command
void HcomHandler::executeConnectCommand(const QString cmd)
{
    QStringList args = getArguments(cmd);
    if(args.size() != 4)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    Port *pPort1 = mpModel->getViewContainerObject()->getModelObject(args[0])->getPort(args[1]);
    Port *pPort2 = mpModel->getViewContainerObject()->getModelObject(args[2])->getPort(args[3]);

    Connector *pConn = mpModel->getViewContainerObject()->createConnector(pPort1, pPort2);

    if (pConn != 0)
    {
        QVector<QPointF> pointVector;
        pointVector.append(pPort1->pos());
        pointVector.append(pPort2->pos());

        QStringList geometryList;
        geometryList.append("diagonal");

        pConn->setPointsAndGeometries(pointVector, geometryList);
        pConn->refreshConnectorAppearance();
    }
}


//! @brief Execute function for "crmo" command
void HcomHandler::executeCreateModelCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 0)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }
    gpModelHandler->addNewModel();
}


//! @brief Execute function for "fmu" command
void HcomHandler::executeExportToFMUCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
    }

    //! @todo Add argument for me or cs
    mpModel->getTopLevelSystemContainer()->exportToFMU(getArgument(cmd, 0), false);
}


//! @brief Execute function for "chts" command
void HcomHandler::executeChangeTimestepCommand(const QString cmd)
{
    if(!mpModel)
    {
        HCOMERR("No model is open.");
        return;
    }
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString component = split[0];

    evaluateExpression(split[1], Scalar);
    if(mAnsType != Scalar)
    {
        HCOMERR("Second argument is not a number.");
    }
    else if(!mpModel->getViewContainerObject()->hasModelObject(component))
    {
        HCOMERR("Component not found.");
    }
    else
    {
        mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setDesiredTimeStep(component, mAnsScalar);
        //gpModelHandler->getCurrentContainer()->getCoreSystemAccessPtr()->setInheritTimeStep(false);
        HCOMPRINT("Setting time step of "+component+" to "+QString::number(mAnsScalar));
    }
}


//! @brief Execute function for "ihts" command
void HcomHandler::executeInheritTimestepCommand(const QString cmd)
{
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString component = split[0];

    if(!mpModel)
    {
        HCOMERR("No model is open.");
        return;
    }
    else if(!mpModel->getViewContainerObject()->hasModelObject(component))
    {
        HCOMERR("Component not found.");
    }
    else
    {
        mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setInheritTimeStep(component, true);
        HCOMPRINT("Setting time step of "+component+" to inherited.");
    }
}


//! @brief Execute function for "bode" command
void HcomHandler::executeBodeCommand(const QString cmd)
{
    int nArgs = getNumberOfArguments(cmd);
    if(nArgs < 2 || nArgs > 4)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString var1 = getArgument(cmd,0);
    QString var2 = getArgument(cmd,1);
    SharedVectorVariableT pData1 = getLogVariable(var1).mpVariable;
    SharedVectorVariableT pData2 = getLogVariable(var2).mpVariable;
    if(!pData1 || !pData2)
    {
        HCOMERR("Data variable not found.");
        return;
    }
    int fMax = 500;
    if(nArgs > 2)
    {
        fMax = getArgument(cmd,2).toInt();
    }

    PlotWindow *pWindow = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Bode plot");
    pWindow->closeAllTabs();
    pWindow->createBodePlot(pData1, pData2, fMax);
}


//! @brief Execute function for "abs" command
void HcomHandler::executeAbsCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString varName = getArgument(cmd,0);

    SharedVectorVariableT var = getLogVariable(varName).mpVariable;
    if(var)
    {
        var.data()->absData();
    }
    else
    {
        bool ok;
        double retval = fabs(getNumber(varName, &ok));
        if(ok)
        {
            HCOMPRINT(QString::number(retval));
            mAnsType = Scalar;
            mAnsScalar = retval;
            return;
        }
        else
        {
            HCOMERR("Variable not found.");
            mAnsType = Undefined;
            return;
        }
    }
}


//! @brief Execute function for "opt" command
void HcomHandler::executeOptimizationCommand(const QString cmd)
{
    if(!mAcceptsOptimizationCommands)
    {
        HCOMINFO("This console does not accept optimization commands.\nUse the optimization dialog instead.");
        return;
    }

    if(!mpModel)
    {
        HCOMERR("No model is open.");
        return;
    }
    QStringList split = getArguments(cmd);

    if(split[0] == "set")
    {
        if(split.size() == 4 && split[1] == "obj")
        {
            bool ok;
            evaluateExpression(split[2], Scalar);
            if(mAnsType != Scalar)
            {
                HCOMERR("Argument number 2 must be a number.");
                return;
            }
            int idx = mAnsScalar;
//            if(idx < 0 || idx > mpOptHandler->getOptVar("npoints")-1)
//            {
//                HCOMERR("Index out of range.");
//                return;
//            }

            double val = getNumber(split[3], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 3 must be a number.");
                return;
            }

            mpOptHandler->setOptimizationObjectiveValue(idx, val);
            return;
        }
        else if(split.size() == 5 && split[1] == "limits")
        {
            bool ok;
            int optParIdx = getNumber(split[2], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 2 must be a number.");
                return;
            }
            if(optParIdx < 0 || optParIdx > mpOptHandler->getOptVar("nparams")-1)
            {
                HCOMERR("Index out of range.");
                return;
            }


            double min = getNumber(split[3], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 3 must be a number.");
                return;
            }

            double max = getNumber(split[4], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 4 must be a number.");
                return;
            }

            mpOptHandler->setParMin(optParIdx, min);
            mpOptHandler->setParMax(optParIdx, max);
            return;
        }
        else if(split.size() == 3)
        {
            bool ok=true;
            evaluateExpression(split[2]);
            if(mAnsType == Scalar)
            {
                mpOptHandler->setOptVar(split[1], QString::number(mAnsScalar), ok);
            }
            else if(mAnsType == Wildcard)
            {
                mpOptHandler->setOptVar(split[1], mAnsWildcard, ok);
            }
            else
            {
                HCOMERR("Unknown parameter value: "+split[2]);
            }
            if(!ok)
            {
                HCOMERR("Unknown optimization setting: "+split[1]);
                return;
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments.");
        }
    }

    if(split.size() == 3 && split[0] == "init")
    {
        return;
    }

    if(split.size() == 1 && split[0] == "run")
    {
        //Create detatched HcomHandler and copy local variables
        //! @todo Delete these when finished!
        //TerminalWidget *pOptTerminal = new TerminalWidget(gpMainWindowWidget);

        //Everything is fine, initialize and run optimization

        //Load hidden copy of model to run optimization against
        QString name = mpModel->getTopLevelSystemContainer()->getName();
        QString appearanceDataBasePath = mpModel->getTopLevelSystemContainer()->getAppearanceData()->getBasePath();
        QDir().mkpath(gpDesktopHandler->getDataPath()+"/optimization/");
        QString savePath = gpDesktopHandler->getDataPath()+"/optimization/"+name+".hmf";
        mpModel->saveTo(savePath);
        mpModel->getTopLevelSystemContainer()->setAppearanceDataBasePath(appearanceDataBasePath);

        if(mpOptHandler->mAlgorithm == OptimizationHandler::ComplexRF ||
           mpOptHandler->mAlgorithm == OptimizationHandler::ComplexRFM)
        {
            mpOptHandler->clearModels();
            mpOptHandler->addModel(gpModelHandler->loadModel(savePath, true, true));
            mpOptHandler->getModelPtrs()->last()->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->addSearchPath(appearanceDataBasePath);
        }
        else if(mpOptHandler->mAlgorithm == OptimizationHandler::ParameterSweep)
        {
            mpOptHandler->clearModels();

            int nThreads = gpConfig->getNumberOfThreads();
            if(nThreads == 0)
            {
        #ifdef WIN32
                std::string temp = getenv("NUMBER_OF_PROCESSORS");
                nThreads = atoi(temp.c_str());
        #else
                nThreads = std::max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
        #endif
            }

            for(int i=0; i<nThreads; ++i)
            {
                mpOptHandler->addModel(gpModelHandler->loadModel(savePath, true, true));

                //Make sure logging is disabled in all nodes if they were disabled in original model
                QStringList compNames = mpModel->getTopLevelSystemContainer()->getModelObjectNames();
                for(int c=0; c<compNames.size(); ++c)
                {
                    QList<Port*> portPtrs = mpModel->getTopLevelSystemContainer()->getModelObject(compNames[c])->getPortListPtrs();
                    for(int p=0; p<portPtrs.size(); ++p)
                    {
                        bool doLog = mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->isLoggingEnabled(compNames[c], portPtrs[p]->getName());
                        mpOptHandler->getModelPtrs()->last()->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->setLoggingEnabled(compNames[c], portPtrs[p]->getName(),doLog);
                    }
                }
            }
        }
        else if(mpOptHandler->mAlgorithm == OptimizationHandler::ParticleSwarm || mpOptHandler->mAlgorithm == OptimizationHandler::ComplexRFP)
        {
            mpOptHandler->clearModels();
            if(getConfigPtr()->getUseMulticore())
            {
                for(int i=0; i<mpOptHandler->getOptVar("npoints"); ++i)
                {
                    mpOptHandler->addModel(gpModelHandler->loadModel(savePath, true, true));
                }
            }
            else
            {
                mpOptHandler->addModel(gpModelHandler->loadModel(savePath, true, true));
            }
        }

        mpOptHandler->startOptimization();
    }
}


//! @brief Execute function for "call" command
void HcomHandler::executeCallFunctionCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString funcName = getArgument(cmd,0);

    if(!mFunctions.contains(funcName))
    {
        HCOMERR("Undefined function.");
        return;
    }

    //TicToc timer;
    //timer.tic(" >>>>>>>>>>>>> In executeCallFunctionCommand: Starting runScriptCommands for: "+cmd+" func: "+funcName);

    bool abort = false;
    runScriptCommands(mFunctions.find(funcName).value(), &abort);
    //timer.toc(" <<<<<<<<<<<<< In executeCallFunctionCommand: Finnished runScriptCommands for: "+cmd+" func: "+funcName);
    if(abort)
    {
        HCOMPRINT("Function aborted");
        mAnsType = Undefined;
        return;
    }
    mAnsType = Undefined;
}


//! @brief Execute function for "echo" command
void HcomHandler::executeEchoCommand(const QString cmd)
{
    if(!mpConsole) return;

    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString arg = getArgument(cmd,0);


    if(arg == "on")
    {
        mpConsole->setDontPrint(false);
    }
    else if(arg == "off")
    {
        mpConsole->setDontPrint(true);
    }
    else
    {
        HCOMERR("Unknown argument, use \"on\" or \"off\"");
    }
}


//! @brief Execute function for "edit" command
void HcomHandler::executeEditCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString path = getArgument(cmd,0);
    path.prepend(mPwd+"/");
    QDesktopServices::openUrl(QUrl(path));
}


void HcomHandler::executeSetMultiThreadingCommand(const QString cmd)
{
    QStringList args = getArguments(cmd);
    int nArgs = args.size();
    if(nArgs < 1 || nArgs > 3)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    bool useMultiThreading;
    if(args[0] == "on")
    {
        useMultiThreading=true;
    }
    else if(args[0] == "off")
    {
        useMultiThreading=false;
    }
    else
    {
        HCOMERR("Unknown argument, use \"on\" or \"off\"");
        return;
    }

    bool ok;
    int nThreads=0;
    if(nArgs > 1)
    {
        nThreads = args[1].toInt(&ok);
        if(!ok)
        {
            HCOMERR("Unknown data type. Only int is supported for argument 2.");
            return;
        }
    }

    int algorithm=0;
    if(nArgs > 2)
    {
        algorithm = args[2].toInt(&ok);
        if(!ok)
        {
            HCOMERR("Unknown data type. Only int is supported for argument 3.");
            return;
        }
    }

    getConfigPtr()->setUseMultiCore(useMultiThreading);
    if(nArgs > 1) getConfigPtr()->setNumberOfThreads(nThreads);
    if(nArgs > 2) getConfigPtr()->setParallelAlgorithm(algorithm);
}


//! @brief Changes plot variables on specified axes
//! @param cmd Command containing the plot variables
//! @param axis Axis specification (0=left, 1=right, -1=both, separeted by "-r")
void HcomHandler::changePlotVariables(const QString cmd, const int axis, bool hold)
{
    QStringList varNames = getArguments(cmd);

    if((axis == -1 || axis == 0) && !hold)
    {
        removePlotCurves(QwtPlot::yLeft);
    }
    if((axis == -1 || axis == 1) && !hold)
    {
        removePlotCurves(QwtPlot::yRight);
    }

    int axisId;
    if(axis == -1 || axis == 0)
    {
        axisId = QwtPlot::yLeft;
    }
    else
    {
        axisId = QwtPlot::yRight;
    }
    for(int s=0; s<varNames.size(); ++s)
    {
        if(axis == -1 && varNames[s] == "-r")
        {
            axisId = QwtPlot::yRight;
        }
        else
        {
            bool found=false;
            QStringList variables;
            getMatchingLogVariableNames(varNames[s], variables);
            if (variables.isEmpty())
            {
                evaluateExpression(varNames[s], DataVector);
                if(mAnsType == DataVector)
                {
                    addPlotCurve(mAnsVector, axisId);
                    found = true;
                }
            }
            else
            {
                found = true;
                for(int v=0; v<variables.size(); ++v)
                {
                    addPlotCurve(variables[v], axisId);

                }
            }

            if (!found)
            {
                HCOMERR(QString("Could not find variable or evaluate expression: %1").arg(varNames[s]));
            }
        }
    }
}


//! @brief Adds a plot curve to specified axis in current plot
//! @param cmd Name of variable
//! @param axis Axis to add curve to
void HcomHandler::addPlotCurve(QString cmd, const int axis)
{
    HopsanVariable data = getLogVariable(cmd);
    if(!data)
    {
        HCOMERR(QString("Variable not found: %1").arg(cmd));
        return;
    }
    else
    {
        addPlotCurve(data, axis);
    }
}

void HcomHandler::addPlotCurve(HopsanVariable data, const int axis)
{
    // If mpCurrentPlotWindow is 0, then we wil lset it to the window that is actually created
    // else we will just set to same
    mpCurrentPlotWindow = gpPlotHandler->plotDataToWindow(mpCurrentPlotWindow, data, axis);
}


//! @brief Adds a plot curve to specified axis in current plot
//! @param [in] fullShortVarNameWithGen Full short name of varaiable to remove (inlcuding optional generation specifier)
void HcomHandler::removeLogVariable(QString fullShortVarNameWithGen) const
{
    if(!mpModel) return;
    SystemContainer *pCurrentSystem = mpModel->getTopLevelSystemContainer();
    if(!pCurrentSystem) { return; }

    bool allGens=false;
    bool parseOK;
    int generation = parseAndChopGenerationSpecifier(fullShortVarNameWithGen, parseOK);
    if (generation < -1)
    {
        allGens = true;
    }

    if (parseOK)
    {
        HopsanVariable data = getLogVariable(fullShortVarNameWithGen);
        if(!data)
        {
            HCOMERR("Variable not found: "+fullShortVarNameWithGen);
            return;
        }

        if( allGens )
        {
            if (data.getLogDataHandler())
            {
                data.getLogDataHandler()->deleteVariable(data.mpVariable->getFullVariableName());
            }
        }
        else
        {
            if (data.mpContainer)
            {
                data.mpContainer->removeDataGeneration(generation, true);
            }
        }
    }
    else
    {
        HCOMERR("Could not parse generation specifier");
    }
}


//! @brief Removes all curves at specified axis in current plot
//! @param axis Axis to remove from
void HcomHandler::removePlotCurves(const int axis) const
{
    if(mpCurrentPlotWindow && mpCurrentPlotWindow->getCurrentPlotTab())
    {
        mpCurrentPlotWindow->getCurrentPlotTab()->removeAllCurvesOnAxis(axis);
    }
}


void HcomHandler::evaluateExpression(QString expr, VariableType desiredType)
{
    //TicToc timer;

    //Remove parentheses around expression
    //Remove all excessive parentheses
    while(expr.startsWith("(") && expr.endsWith(")"))
    {
        QString tempStr = expr.mid(1, expr.size()-2);
        if(SymHop::Expression::verifyParantheses(tempStr)) { expr = tempStr; }
        else { break; }
    }

    // Check if "ans"
    if (expr == "ans")
    {
        if (desiredType == mAnsType ||
            (desiredType == Undefined && mAnsType != Undefined) )
        {
            return;
        }
        else
        {
            mAnsType = Undefined;
            return;
        }
    }

    if(desiredType != DataVector)
    {
        //Numerical value, return it
        bool ok;
        expr.toDouble(&ok);
        if(ok)
        {
            mAnsType = Scalar;
            mAnsScalar = expr.toDouble();
            return;
        }

        //Pre-defined variable, return its value
        LocalVarsMapT::iterator it = mLocalVars.find(expr);
        if(it != mLocalVars.end())
        {
            mAnsType = Scalar;
            mAnsScalar = it.value();
            return;
        }

        //Optimization parameter
//        if(expr.startsWith("par(") && expr.endsWith(")"))
//        {
//            QString argStr = expr.section("(",1,1).section(")",-2,-2);
//            QString nPointsStr = expr.section(",",0,0);
//            QString nParStr = expr.section(",",1,1).section(")",-1,-1);
//            if(nPointsStr.isEmpty() || nParStr.isEmpty())
//            {
//                mAnsType = Scalar;
//                mAnsScalar =0;
//                return;
//            }
//            bool ok1, ok2;
//            int nPoint = getNumber(nPointsStr,&ok1);
//            int nPar = getNumber(nParStr, &ok2);
//            if(ok1 && ok2 && nPoint>=0 && nPoint < mpOptHandler->mParameters.size() && nPar>= 0 && nPar < mpOptHandler->mParameters[nPoint].size())
//            {
//                mAnsType = Scalar;
//                mAnsScalar = mpOptHandler->mParameters[nPoint][nPar];
//                return;
//            }
        //}
    }

    if(desiredType != Scalar)
    {
        //Data variable, return it
        SharedVectorVariableT data = getLogVariable(expr).mpVariable;
        if(data)
        {
            mAnsType = DataVector;
            mAnsVector = data;
            return;
        }
    }

    if(desiredType != DataVector)
    {
        //Parameter name, return its value
        //timer.tic();
        QString parVal = getParameterValue(expr);
        //timer.tocDbg("getParameterValue");
        if( parVal != "NaN")
        {
            bool ok;
            mAnsScalar = parVal.toDouble(&ok);
            if(!ok)     //It is not a numerical, so assume it is a system parameter
            {
                parVal = getParameterValue(parVal);
                mAnsScalar = parVal.toDouble();
            }
            mAnsType = Scalar;
            return;
        }
    }

    // Vector functions
    //timer.tic();
    LogDataHandler *pLogData=0;
    if(mpModel && mpModel->getTopLevelSystemContainer())
    {
        pLogData = mpModel->getTopLevelSystemContainer()->getLogDataHandler();
    }
    if(desiredType != Scalar && expr.startsWith("ddt(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args,',');
        if(splitArgs.size() == 1)
        {
            evaluateExpression(args.trimmed(),DataVector);
            SharedVectorVariableT pVar = mAnsVector;
            if (mAnsType == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pLogData->diffVariables(pVar, pVar->getSharedTimeOrFrequencyVector());
                return;
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(args.trimmed()));
                mAnsType = Undefined;
                return;
            }
        }
        else if(splitArgs.size() == 2)
        {
            const QString var1 = splitArgs[0];
            evaluateExpression(var1, DataVector);
            SharedVectorVariableT pVar1 = mAnsVector;
            if (mAnsType == DataVector)
            {
                const QString var2 = splitArgs[1];
                evaluateExpression(var2, DataVector);
                SharedVectorVariableT pVar2 = mAnsVector;
                if (mAnsType == DataVector)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogData->diffVariables(pVar1, pVar2);
                    return;
                }
                else
                {
                    HCOMERR(QString("Variable: %1 was not found!").arg(var2));
                    mAnsType = Undefined;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(var1));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments for ddt function.\n"+mLocalFunctionDescriptions.find("ddt").value().second);
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && expr.startsWith("lp1(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args,',');
        if(splitArgs.size() == 2)
        {
            const QString varName = splitArgs[0];
            const QString arg2 = splitArgs[1];
            bool isok;
            double freq = arg2.toDouble(&isok);
            if (isok)
            {
                evaluateExpression(varName, DataVector);
                SharedVectorVariableT pVar = mAnsVector;
                if (mAnsType == DataVector)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogData->lowPassFilterVariable(pVar, pVar->getSharedTimeOrFrequencyVector(), freq);
                    return;
                }
                else
                {
                    mAnsType = Undefined;
                    HCOMERR(QString("Variable: %1 was not found!").arg(varName));
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Failed to parse frequency: %1").arg(arg2));
                mAnsType = Undefined;
                return;
            }
        }
        else if(splitArgs.size() == 3)
        {
            const QString varName = splitArgs[0];
            const QString arg2 = splitArgs[2];
            bool isok;
            double freq = arg2.toDouble(&isok);
            if (isok)
            {
                evaluateExpression(varName, DataVector);
                SharedVectorVariableT pVar = mAnsVector;
                if (mAnsType == DataVector)
                {
                    const QString timeVarName = splitArgs[1];
                    evaluateExpression(timeVarName);
                    SharedVectorVariableT pTimeVar = mAnsVector;
                    if (mAnsType == DataVector)
                    {
                        mAnsType = DataVector;
                        mAnsVector = pLogData->lowPassFilterVariable(pVar, pTimeVar, freq);
                        return;
                    }
                    else
                    {
                        HCOMERR(QString("Time variable: %1 was not found!").arg(timeVarName));
                        mAnsType = Undefined;
                        return;
                    }
                }
                else
                {
                    HCOMERR(QString("Variable: %1 was not found!").arg(varName));
                    mAnsType = Undefined;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Failed to parse frequency: %1").arg(arg2));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for lp1 function.\n"+mLocalFunctionDescriptions.find("lp1").value().second);
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && expr.startsWith("int(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args,',');
        if(splitArgs.size() == 1)
        {
            evaluateExpression(args.trimmed(),DataVector);
            if(mAnsType == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pLogData->integrateVariables(mAnsVector, mAnsVector->getSharedTimeOrFrequencyVector());
                return;
            }
            else
            {
                HCOMERR(QString("Argument 1 is not a vector."));
                mAnsType = Undefined;
                return;
            }
        }
        else if(splitArgs.size() == 2)
        {

            const QString var1 = splitArgs[0];
            const QString var2 = splitArgs[1];
            evaluateExpression(var1, DataVector);
            SharedVectorVariableT pVar1 = mAnsVector;
            if(mAnsType != DataVector)
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(var1));
                mAnsType = Undefined;
                return;
            }
            evaluateExpression(var2, DataVector);
            SharedVectorVariableT pVar2 = mAnsVector;
            if(mAnsType != DataVector)
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(var2));
                mAnsType = Undefined;
                return;
            }
            else
            {
                mAnsType = DataVector;
                mAnsVector = pLogData->integrateVariables(pVar1, pVar2);
                return;
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for int function.\n"+mLocalFunctionDescriptions.find("int").value().second);
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && expr.startsWith("fft(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args, ',');
        if(splitArgs.size() == 1)
        {
            mAnsType = DataVector;
            const QString varName = args.section(",",0,0).trimmed();
            evaluateExpression(varName, DataVector);
            SharedVectorVariableT pVar = mAnsVector;
            if (mAnsType == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pVar->toFrequencySpectrum(pVar->getSharedTimeOrFrequencyVector(), false);
                return;
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(varName));
                mAnsType = Undefined;
                return;
            }
        }
        else if(splitArgs.size() == 2)
        {
            const QString varName = splitArgs[0].trimmed();
            evaluateExpression(varName, DataVector);
            SharedVectorVariableT pVar = mAnsVector;
            if (mAnsType == DataVector)
            {
                bool power=false;
                QString arg2 = splitArgs[1].trimmed();
                SharedVectorVariableT pTimeVar;
                if( (arg2=="true") || (arg2=="false") )
                {
                    power = (arg2 == "true");
                    pTimeVar = pVar->getSharedTimeOrFrequencyVector();
                    mAnsType = DataVector;
                }
                else
                {
                    evaluateExpression(arg2, DataVector);
                    pTimeVar = mAnsVector;
                }

                if (mAnsType == DataVector)
                {
                    mAnsType = DataVector;
                    mAnsVector = pVar->toFrequencySpectrum(pTimeVar, power);
                    return;
                }
                else
                {
                    HCOMERR(QString("Time variable: %1 was not found!").arg(arg2));
                    mAnsType = Undefined;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(varName));
                mAnsType = Undefined;
                return;
            }

        }
        else if(splitArgs.size() == 3)
        {
            bool power = (splitArgs[2].trimmed() == "true");
            const QString varName = splitArgs[0].trimmed();
            evaluateExpression(varName, DataVector);
            SharedVectorVariableT pVar = mAnsVector;
            if (mAnsType == DataVector)
            {
                const QString timeVarName = splitArgs[1].trimmed();
                evaluateExpression(timeVarName, DataVector);
                SharedVectorVariableT pTimeVar = mAnsVector;
                if (mAnsType == DataVector)
                {
                    mAnsType = DataVector;
                    mAnsVector = pVar->toFrequencySpectrum(pTimeVar, power);
                    return;
                }
                else
                {
                    HCOMERR(QString("Time variable: %1 was not found!").arg(timeVarName));
                    mAnsType = Undefined;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(varName));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for fft function.\n"+mLocalFunctionDescriptions.find("fft").value().second);
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && (expr.startsWith("greaterThan(") || expr.startsWith("gt(")) && expr.endsWith(")"))
    {
        int funcSize = expr.section("(",0,0).size()+1;
        QString args = expr.mid(funcSize, expr.size()-funcSize-1);
        if(args.count(",")==1)
        {
            const QString arg1 = args.section(",",0,0).trimmed();
            const QString arg2 = args.section(",",1,1).trimmed();
            bool success;
            double limit = arg2.toDouble(&success);
            if (success)
            {
                SharedVectorVariableT pVar = getLogVariable(arg1).mpVariable;
                if (pVar)
                {
                    SharedVectorVariableT pTemp = pLogData->elementWiseGT(pVar, limit);
                    if (pTemp)
                    {
                        mAnsType = DataVector;
                        mAnsVector = pTemp;
                        return;
                    }
                }
                else
                {
                    HCOMERR(QString("Variable: %1 was not found!").arg(arg1));
                    mAnsType = Undefined;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Failed to parse threshold: %1").arg(arg2));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR(QString("Wrong number of arguments provided to gt function.\n"+mLocalFunctionDescriptions.find("gt").value().second));
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && (expr.startsWith("smallerThan(") || expr.startsWith("lt(")) && expr.endsWith(")"))
    {
        int funcSize = expr.section("(",0,0).size()+1;
        QString args = expr.mid(funcSize, expr.size()-funcSize-1);
        if(args.count(",")==1)
        {
            const QString arg1 = args.section(",",0,0).trimmed();
            const QString arg2 = args.section(",",1,1).trimmed();
            bool success;
            double limit = arg2.toDouble(&success);

            if (success)
            {
                SharedVectorVariableT pVar = getLogVariable(arg1).mpVariable;
                if (pVar)
                {
                    SharedVectorVariableT pTemp = pLogData->elementWiseLT(pVar, limit);
                    if (pTemp)
                    {
                        mAnsType = DataVector;
                        mAnsVector = pTemp;
                        return;
                    }
                }
                else
                {
                    HCOMERR(QString("Variable: %1 was not found!").arg(arg1));
                    mAnsType = Undefined;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Failed to parse threshold: %1").arg(arg2));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR(QString("Wrong number of arguments provided for lt function.\n"+mLocalFunctionDescriptions.find("lt").value().second));
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && (expr.startsWith("linspace(") && expr.endsWith(")")))
    {
        QString args = expr.mid(9, expr.size()-10);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args,',');
        if (splitArgs.size() == 3)
        {
            bool minOK, maxOK, nOK;
            double min = splitArgs.first().toDouble(&minOK);
            double max = splitArgs[1].toDouble(&maxOK);
            int nSamp = int(splitArgs.last().toDouble(&nOK)+0.5);

            if (minOK && maxOK && nOK)
            {
                if (nSamp>1)
                {
                    if (mpModel)
                    {
                        QVector<double> data(nSamp);
                        for (int i=0; i<data.size(); ++i)
                        {
                            data[i] = min+double(i)*(max-min)/double(nSamp-1);
                        }
                        mAnsVector = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->createOrphanVariable("linspace");
                        mAnsVector->assignFrom(data);
                        mAnsType = DataVector;
                        return;
                    }
                    else
                    {
                        HCOMERR("Could not create new vector, no model open");
                    }
                }
                else
                {
                    HCOMERR("Number of samples must be > 1");
                }
            }
            else
            {
                HCOMERR("Could not parse arguments");
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for linspace function");
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && (expr.startsWith("logspace(") && expr.endsWith(")")))
    {
        QString args = expr.mid(9, expr.size()-10);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args,',');
        if (splitArgs.size() == 3)
        {
            bool minOK, maxOK, nOK;
            double min = splitArgs.first().toDouble(&minOK);
            double max = splitArgs[1].toDouble(&maxOK);
            int nSamp = int(splitArgs.last().toDouble(&nOK)+0.5);

            if (minOK && maxOK && nOK)
            {
                if (nSamp>1)
                {
                    if (mpModel)
                    {
                        QVector<double> data(nSamp);
                        for (int i=0; i<data.size(); ++i)
                        {
                            data[i] = pow(10, min+double(i)*(max-min)/double(nSamp-1));
                        }
                        mAnsVector = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->createOrphanVariable("linspace");
                        mAnsVector->assignFrom(data);
                        mAnsType = DataVector;
                        return;
                    }
                    else
                    {
                        HCOMERR("Could not create new vector, no model open");
                    }
                }
                else
                {
                    HCOMERR("Number of samples must be > 1");
                }
            }
            else
            {
                HCOMERR("Could not parse arguments");
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for logspace function");
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && (expr.startsWith("ones(") && expr.endsWith(")")))
    {
        QString arg = expr.mid(5, expr.size()-6);
        bool parseOK;
        int nElem = int(arg.toDouble(&parseOK)+0.5);
        if (parseOK)
        {
            if (nElem > 0)
            {
                if (mpModel)
                {
                    QVector<double> data(nElem, 1.0);
                    mAnsVector = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->createOrphanVariable("ones");
                    mAnsVector->assignFrom(data);
                    mAnsType = DataVector;
                    return;
                }
                else
                {
                    HCOMERR("Could not create new vector, no model open");
                }
            }
            else
            {
                HCOMERR("Size must be > 0");
            }
        }
        else
        {
            HCOMERR("Could not parse size argument");
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && (expr.startsWith("zeros(") && expr.endsWith(")")))
    {
        QString arg = expr.mid(6, expr.size()-7);
        bool parseOK;
        int nElem = int(arg.toDouble(&parseOK)+0.5);
        if (parseOK)
        {
            if (nElem > 0)
            {
                if (mpModel)
                {
                    QVector<double> data(nElem, 0.0);
                    mAnsVector = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->createOrphanVariable("zeros");
                    mAnsVector->assignFrom(data);
                    mAnsType = DataVector;
                    return;
                }
                else
                {
                    HCOMERR("Could not create new vector, no model open");
                }
            }
            else
            {
                HCOMERR("Size must be > 0");
            }
        }
        else
        {
            HCOMERR("Could not parse size argument");
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && expr.startsWith("maxof(") && expr.endsWith(")"))
    {
        QString args = expr.mid(6, expr.size()-7);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args, ',');
        if(splitArgs.size() == 2)
        {
            const QString varName1 = splitArgs.first().trimmed();
            const QString varName2 = splitArgs.last().trimmed();

            HopsanVariable var1 = getLogVariable(varName1);
            if (var1)
            {
                HopsanVariable var2 = getLogVariable(varName2);
                if (var2)
                {
                    mAnsType = DataVector;
                    mAnsVector = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->createOrphanVariable(QString("maxof%1%2").arg(var1.mpVariable->getSmartName()).arg(var2.mpVariable->getSmartName()));
                    QVector<double> a = var1.mpVariable->getDataVectorCopy();
                    QVector<double> b = var2.mpVariable->getDataVectorCopy();
                    QVector<double> c(qMin(a.size(), b.size()));
                    for (int i=0; i<c.size(); ++i)
                    {
                        c[i] = qMax(a[i],b[i]);
                    }
                    mAnsVector->assignFrom(c);
                    return;
                }
                else
                {
                    HCOMERR(QString("Variable %1 could not be found").arg(varName2));
                }
            }
            else
            {
                HCOMERR(QString("Variable %1 could not be found").arg(varName1));
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for maxof function.\n"+mLocalFunctionDescriptions.find("maxof").value().second);
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && expr.startsWith("minof(") && expr.endsWith(")"))
    {
        QString args = expr.mid(6, expr.size()-7);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args, ',');
        if(splitArgs.size() == 2)
        {
            const QString varName1 = splitArgs.first().trimmed();
            const QString varName2 = splitArgs.last().trimmed();

            HopsanVariable var1 = getLogVariable(varName1);
            if (var1)
            {
                HopsanVariable var2 = getLogVariable(varName2);
                if (var2)
                {
                    mAnsType = DataVector;
                    mAnsVector = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->createOrphanVariable(QString("minof%1%2").arg(var1.mpVariable->getSmartName()).arg(var2.mpVariable->getSmartName()));
                    QVector<double> a = var1.mpVariable->getDataVectorCopy();
                    QVector<double> b = var2.mpVariable->getDataVectorCopy();
                    QVector<double> c(qMin(a.size(), b.size()));
                    for (int i=0; i<c.size(); ++i)
                    {
                        c[i] = qMin(a[i],b[i]);
                    }
                    mAnsVector->assignFrom(c);
                    return;
                }
                else
                {
                    HCOMERR(QString("Variable %1 could not be found").arg(varName2));
                }
            }
            else
            {
                HCOMERR(QString("Variable %1 could not be found").arg(varName1));
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for minof function.\n"+mLocalFunctionDescriptions.find("minof").value().second);
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && expr.count("<")==1 && getLogVariable(expr.section("<",0,0)))
    {
        const QString arg1 = expr.section("<",0,0).trimmed();
        const QString arg2 = expr.section("<",1,1).trimmed();
        bool success;
        double limit = arg2.toDouble(&success);

        if (success)
        {
            SharedVectorVariableT pVar = getLogVariable(arg1).mpVariable;
            if (pVar)
            {
                SharedVectorVariableT pTemp = pLogData->elementWiseLT(pVar, limit);
                if (pTemp)
                {
                    mAnsType = DataVector;
                    mAnsVector = pTemp;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(arg1));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR(QString("Failed to parse threshold: %1").arg(arg2));
            mAnsType = Undefined;
            return;
        }
    }
    else if(desiredType != Scalar && expr.count(">")==1 && getLogVariable(expr.section(">",0,0)))
    {
        const QString arg1 = expr.section(">",0,0).trimmed();
        const QString arg2 = expr.section(">",1,1).trimmed();
        bool success;
        double limit = arg2.toDouble(&success);

        if (success)
        {
            SharedVectorVariableT pVar = getLogVariable(arg1).mpVariable;
            if (pVar)
            {
                SharedVectorVariableT pTemp = pLogData->elementWiseGT(pVar, limit);
                if (pTemp)
                {
                    mAnsType = DataVector;
                    mAnsVector = pTemp;
                    return;
                }
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(arg1));
                mAnsType = Undefined;
                return;
            }
        }
        else
        {
            HCOMERR(QString("Failed to parse threshold: %1").arg(arg2));
            mAnsType = Undefined;
            return;
        }
    }
    //timer.toc("Vector functions", 1);

    //Evaluate expression using SymHop
    SymHop::Expression symHopExpr = SymHop::Expression(expr, SymHop::Expression::TrivialSimplifications);

    //Multiplication between data vector and scalar
    //timer.tic();
    //! @todo this code does pointer lookup, then does it again, and then get names to use string versions of logdatahandler functions, it could lookup once and then use the pointer versions instead
    //! @todo If SymHop simplifies expression to a constant, the code below won't find it
    if(desiredType != Scalar && symHopExpr.isMultiplyOrDivide() && (symHopExpr.getDivisors().isEmpty() || symHopExpr.getFactors().size() > 1) && pLogData)
    {
        SymHop::Expression f0 = symHopExpr.getFactors()[0];
        SymHop::Expression f1 = symHopExpr;
        f1.removeFactor(f0);

        VariableType varType0, varType1;
        double scalar0, scalar1;
        SharedVectorVariableT vec0, vec1;
        evaluateExpression(f0.toString());
        if(mAnsType != DataVector && mAnsType != Scalar)
        {
            mAnsType = Undefined;
            return;
        }
        varType0 = mAnsType;
        if(varType0 == Scalar)
        {
            scalar0 = mAnsScalar;
        }
        else
        {
            vec0 = mAnsVector;
        }
        evaluateExpression(f1.toString());
        if(mAnsType != DataVector && mAnsType != Scalar)
        {
            mAnsType = Undefined;
            return;
        }
        varType1 = mAnsType;
        if(varType1 == Scalar)
        {
            scalar1 = mAnsScalar;
        }
        else
        {
            vec1 = mAnsVector;
        }

        if(varType0 == Scalar && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->mulVariableWithScalar(vec1, scalar0);
            return;
        }
        else if(varType0 == DataVector && varType1 == Scalar)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->mulVariableWithScalar(vec0, scalar1);
            return;
        }
        else if(varType0 == DataVector && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->multVariables(vec0, vec1);
            return;
        }
    }
    if(desiredType != Scalar && pLogData && symHopExpr.isPower())
    {
        SymHop::Expression b = *symHopExpr.getBase();
        SymHop::Expression p = *symHopExpr.getPower();

        evaluateExpression(b.toString());

        if(mAnsType == DataVector && p.toDouble() == 2.0)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->multVariables(mAnsVector, getLogVariable(b.toString()).mpVariable);
            return;
        }
    }
    if(desiredType != Scalar && pLogData && symHopExpr.isMultiplyOrDivide() && symHopExpr.getFactors().size() == 1)
    {
        SymHop::Expression f = symHopExpr.getFactors()[0];
        SymHop::Expression d = SymHop::Expression::fromFactorsDivisors(symHopExpr.getDivisors(), QList<SymHop::Expression>());

        VariableType varType0, varType1;
        double scalar1;
        SharedVectorVariableT vec0, vec1;
        evaluateExpression(f.toString(), DataVector);
        if(mAnsType == DataVector)
        {
            varType0 = mAnsType;
            vec0 = mAnsVector;
            evaluateExpression(d.toString());
            if(mAnsType != DataVector && mAnsType != Scalar)
            {
                mAnsType = Undefined;
                return;
            }
            varType1 = mAnsType;
            if(varType1 == Scalar)
            {
                scalar1 = mAnsScalar;
            }
            else
            {
                vec1 = mAnsVector;
            }


            if(varType0 == DataVector && varType1 == Scalar)
            {
                mAnsType = DataVector;
                mAnsVector = pLogData->divVariableWithScalar(vec0, scalar1);
                return;
            }
            else if(varType0 == DataVector && varType1 == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pLogData->divVariables(vec0, vec1);
                return;
            }
        }
    }
    if(desiredType != Scalar && pLogData && symHopExpr.isAdd())
    {
        SymHop::Expression t0 = symHopExpr.getTerms()[0];
        SymHop::Expression t1 = symHopExpr;
        t1.subtractBy(t0);
        t0._simplify(SymHop::Expression::FullSimplification);
        t1._simplify(SymHop::Expression::FullSimplification);

        VariableType varType0, varType1;
        double scalar0, scalar1;
        SharedVectorVariableT vec0, vec1;
        evaluateExpression(t0.toString());
        if(mAnsType != DataVector && mAnsType != Scalar)
        {
            mAnsType = Undefined;
            return;
        }
        varType0 = mAnsType;
        if(varType0 == Scalar)
        {
            scalar0 = mAnsScalar;
        }
        else
        {
            vec0 = mAnsVector;
        }
        evaluateExpression(t1.toString());
        if(mAnsType != DataVector && mAnsType != Scalar)
        {
            mAnsType = Undefined;
            return;
        }
        varType1 = mAnsType;
        if(varType1 == Scalar)
        {
            scalar1 = mAnsScalar;
        }
        else
        {
            vec1 = mAnsVector;
        }

        if(varType0 == DataVector && varType1 == Scalar)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->addVariableWithScalar(vec0, scalar1);
            return;
        }
        else if(varType0 == Scalar && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->addVariableWithScalar(vec1, scalar0);
            return;
        }
        else if(varType0 == DataVector && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogData->addVariables(vec0, vec1);
            return;
        }
    }
    //timer.toc("Multiplication between data vector and scalar", 0);

    if(desiredType != DataVector)
    {
        //timer.tic();
        QMap<QString, double> localVars = mLocalVars;
        QStringList localPars;
        getParameters("*", localPars);
        for(int p=0; p<localPars.size(); ++p)
        {
            localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
        }
        //timer.toc("local pars to local vars");

        bool ok;
        double scalar = symHopExpr.evaluate(localVars, &mLocalFunctionoidPtrs, &ok);
        if(ok)
        {
            mAnsType = Scalar;
            mAnsScalar = scalar;
            return;
        }
    }

    mAnsType = Wildcard;
    mAnsWildcard = expr;
    return;
}

void HcomHandler::setAcceptsOptimizationCommands(const bool value)
{
    mAcceptsOptimizationCommands = value;
}

bool HcomHandler::getAcceptsOptimizationCommands() const
{
    return mAcceptsOptimizationCommands;
}


void HcomHandler::splitAtFirst(QString str, QString c, QString &left, QString &right)
{
    int idx=0;
    int parBal=0;
    for(; idx<str.size(); ++idx)
    {
        if(str[idx] == '(') { ++parBal; }
        if(str[idx] == ')') { --parBal; }
        if(str.mid(idx, c.size()) == c && parBal == 0)
        {
            break;
        }
    }

    left = str.left(idx);
    right = str.right(str.size()-idx-c.size());
    return;
}


bool HcomHandler::containsOutsideParentheses(QString str, QString c)
{
    int idx=0;
    int parBal=0;
    for(; idx<str.size(); ++idx)
    {
        if(str[idx] == '(') { ++parBal; }
        if(str[idx] == ')') { --parBal; }
        if(str.mid(idx, c.size()) == c && parBal == 0)
        {
            return true;
        }
    }
    return false;
}


QString HcomHandler::runScriptCommands(QStringList &lines, bool *pAbort)
{
    mAborted = false; //Reset if pushed when script didn't run

    QString funcName="";
    QStringList funcCommands;

    for(int l=0; l<lines.size(); ++l)
    {
        qApp->processEvents();
        if(mAborted)
        {
            HCOMPRINT("Script aborted.");
            //mAborted = false;
            *pAbort=true;
            return "";
        }

        // Remove indentation
        while(lines[l].startsWith(" "))
        {
            lines[l] = lines[l].right(lines[l].size()-1);
        }

        // Check how each line starts call appropriate commands
        if(lines[l].isEmpty() || lines[l].startsWith("#") || lines[l].startsWith("&"))
        {
            // Ignore blank lines comments and labels
            continue;
        }
        else if(lines[l].startsWith("stop"))
        {
            return "%%%%%EOF";
        }
        else if(lines[l].startsWith("define "))
        {
            funcName = lines[l].section(" ",1).trimmed();
            ++l;
            while(!lines[l].startsWith("enddefine"))
            {
                funcCommands << lines[l];
                ++l;
            }
            mFunctions.insert(funcName, funcCommands);
            HCOMPRINT("Defined function: "+funcName);
            funcName.clear();
            funcCommands.clear();
        }
        else if(lines[l].startsWith("goto"))
        {
            QString argument = lines[l].section(" ",1);
            return argument;
        }
        else if(lines[l].startsWith("while"))        //Handle while loops
        {
            QString argument = lines[l].section("(",1);
            argument.chop(1);
            QStringList loop;
            int nLoops=1;
            while(nLoops > 0)
            {
                ++l;
                lines[l] = lines[l].trimmed();
                if(l>lines.size()-1)
                {
                    HCOMERR("Missing REPEAT in while loop.");
                    return QString();
                }

                if(lines[l].startsWith("while")) { ++nLoops; }
                if(lines[l].startsWith("repeat")) { --nLoops; }

                loop.append(lines[l]);
            }
            loop.removeLast();

            //Evaluate expression using SymHop
            SymHop::Expression symHopExpr = SymHop::Expression(argument);
            //TicToc timer;
            QMap<QString, double> localVars = mLocalVars;
            QStringList localPars;
            getParameters("*", localPars);
            for(int p=0; p<localPars.size(); ++p)
            {
                localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
            }
            //timer.toc("runScriptCommand: pars to local vars");
            //timer.tic();
            bool ok = true;
            while(symHopExpr.evaluate(localVars, &mLocalFunctionoidPtrs, &ok) > 0 && ok)
            {
                qApp->processEvents();
                if(mAborted)
                {
                    HCOMPRINT("Script aborted.");
                    //mAborted = false;
                    *pAbort=true;
                    return "";
                }
                QString gotoLabel = runScriptCommands(loop, pAbort);
                if(*pAbort)
                {
                    return "";
                }
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }

                //Update local variables for SymHop in case they have changed
                //TicToc timer2;
                localVars = mLocalVars;
                QStringList localPars;
                getParameters("*", localPars);
                for(int p=0; p<localPars.size(); ++p)
                {
                    localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
                }
                //timer2.toc("runScriptCommand: pars to local vars 2");
            }
            //timer.toc("runScriptCommand: While loop");
        }
        else if(lines[l].startsWith("if"))        //Handle if statements
        {
            QString argument = lines[l].section("(",1);
            argument=argument.trimmed();
            argument.chop(1);
            QStringList ifCode;
            QStringList elseCode;
            bool inElse=false;
            while(true)
            {
                ++l;
                lines[l] = lines[l].trimmed();
                if(l>lines.size()-1)
                {
                    HCOMERR("Missing ENDIF in if-statement.");
                    return QString();
                }
                if(lines[l].startsWith("endif"))
                {
                    break;
                }
                if(lines[l].startsWith("else"))
                {
                    inElse=true;
                }
                else if(!inElse)
                {
                    ifCode.append(lines[l]);
                }
                else
                {
                    elseCode.append(lines[l]);
                }
            }

            evaluateExpression(argument, Scalar);
            if(mAnsType != Scalar)
            {
                HCOMERR("Evaluation of if-statement argument failed.");
                return QString();
            }
            if(mAnsScalar > 0)
            {
                QString gotoLabel = runScriptCommands(ifCode, pAbort);
                if(*pAbort)
                {
                    return "";
                }
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
            else
            {
                QString gotoLabel = runScriptCommands(elseCode, pAbort);
                if(*pAbort)
                {
                    return "";
                }
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
        }
        else if(lines[l].startsWith("foreach"))        //Handle foreach loops
        {
            QString var = lines[l].section(" ",1,1);
            QString filter = lines[l].section(" ",2,2);
            QStringList vars;
            getMatchingLogVariableNames(filter, vars);
            QStringList loop;
            while(!lines[l].startsWith("endforeach"))
            {
                ++l;
                loop.append(lines[l]);
            }
            loop.removeLast();
            //TicToc timer;
            for(int v=0; v<vars.size(); ++v)
            {
                //Append quotations around spaces
                QStringList splitVar = vars[v].split(".");
                vars[v].clear();
                for(int s=0; s<splitVar.size(); ++s)
                {
                    if(splitVar[s].contains(" "))
                    {
                        splitVar[s].append("\"");
                        splitVar[s].prepend("\"");
                    }
                    vars[v].append(splitVar[s]);
                    vars[v].append(".");
                }
                vars[v].chop(1);

                //Execute command
                QStringList tempCmds;
                for(int l=0; l<loop.size(); ++l)
                {
                    QString tempCmd = loop[l];
                    tempCmd.replace("$"+var, vars[v]);
                    tempCmds.append(tempCmd);
                }
                QString gotoLabel = runScriptCommands(tempCmds, pAbort);
                if(*pAbort)
                {
                    return "";
                }
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
            //timer.toc("runScriptCommand: foreach vars loop");
        }
        else
        {
            this->executeCommand(lines[l]);
        }
    }
    return QString();
}


//! @brief Help function that returns a list of components depending on input (with support for asterisks)
//! @param[in] rStr Component name to look for
//! @param[out] rComponents Reference to list of found components
void HcomHandler::getComponents(const QString &rStr, QList<ModelObject*> &rComponents) const
{
    if(!mpModel) { return; }
    SystemContainer *pCurrentSystem = mpModel->getTopLevelSystemContainer();
    if(!pCurrentSystem) { return; }

    if (rStr.contains("*"))
    {
        QString left = rStr.split("*").first();
        QString right = rStr.split("*").last();

        QStringList mo_names = pCurrentSystem->getModelObjectNames();
        for(int n=0; n<mo_names.size(); ++n)
        {
            if(mo_names[n].startsWith(left) && mo_names[n].endsWith(right))
            {
                rComponents.append(pCurrentSystem->getModelObject(mo_names[n]));
            }
        }
    }
    else
    {
        rComponents.append(pCurrentSystem->getModelObject(rStr));
    }
}


//! @brief Help function that returns a list of ports depending on input (with support for asterisks)
//! @param[in] rStr Port name to look for
//! @param[out] rPorts Reference to list of found components
void HcomHandler::getPorts(const QString &rStr, QList<Port*> &rPorts) const
{
    if(!mpModel) { return; }
    SystemContainer *pCurrentSystem = mpModel->getTopLevelSystemContainer();
    if(!pCurrentSystem) { return; }

    if (rStr.contains("*"))
    {
        QStringList compNames = pCurrentSystem->getModelObjectNames();
        Q_FOREACH(const QString &compName, compNames)
        {
            for(int p=0; p<pCurrentSystem->getModelObject(compName)->getPortListPtrs().size(); ++p)
            {
                Port *pPort = pCurrentSystem->getModelObject(compName)->getPortListPtrs().at(p);
                QString testStr = compName+"."+pPort->getName();
                QRegExp rx(rStr);
                rx.setPatternSyntax(QRegExp::Wildcard);
                if(rx.exactMatch(testStr))
                {
                    rPorts.append(pPort);
                }
            }
        }
    }
    else
    {
        QString compName = rStr.split(".").first();
        QString portName = rStr.split(".").last();
        rPorts.append(pCurrentSystem->getModelObject(compName)->getPort(portName));
    }
}


QString HcomHandler::getfullNameFromAlias(const QString &rAlias) const
{
    if(mpModel)
    {
        SystemContainer *pCurrentSystem = mpModel->getTopLevelSystemContainer();
        if(pCurrentSystem)
        {
            return pCurrentSystem->getFullNameFromAlias(rAlias);
        }
    }
    return "";
}


//! @brief Help function that returns a list of parameters according to input (with support for asterisks)
//! @param str String to look for
//! @param pComponent Pointer to component to look in
//! @param parameterse Reference to list of found parameters
void HcomHandler::getParameters(QString str, ModelObject* pComponent, QStringList &parameters)
{
    if(str.contains("*"))
    {
        QString left = str.split("*").first();
        QString right = str.split("*").last();

        for(int n=0; n<pComponent->getParameterNames().size(); ++n)
        {
            if(pComponent->getParameterNames().at(n).startsWith(left) && pComponent->getParameterNames().at(n).endsWith(right))
            {
                parameters.append(pComponent->getParameterNames().at(n));
            }
        }
    }
    else
    {
        parameters.append(str);
    }
}


//! @brief Generates a list of parameters based on wildcards
//! @param str String with (or without) wildcards
//! @param parameters Reference to list of parameters
void HcomHandler::getParameters(const QString str, QStringList &parameters)
{
    if(!mpModel) { return; }

    SystemContainer *pSystem = mpModel->getTopLevelSystemContainer();

    QStringList componentNames = pSystem->getModelObjectNames();

    QStringList allParameters;

    //Add quotation marks around component name if it contains spaces
    for(int n=0; n<componentNames.size(); ++n)
    {
        QStringList parameterNames = pSystem->getModelObject(componentNames[n])->getParameterNames();

        if(componentNames[n].contains(" "))
        {
            componentNames[n].prepend("\"");
            componentNames[n].append("\"");
        }

        for(int p=0; p<parameterNames.size(); ++p)
        {
            parameterNames[p].replace("#", ".");
            toShortDataNames(parameterNames[p]);
            allParameters.append(componentNames[n]+"."+parameterNames[p]);
        }
    }

    QStringList systemParameters = pSystem->getParameterNames();
    for(int s=0; s<systemParameters.size(); ++s)
    {
        if(systemParameters[s].contains(" "))
        {
            systemParameters[s].prepend("\"");
            systemParameters[s].append("\"");
        }
        allParameters.append(systemParameters[s]);
    }

    QStringList aliasNames = pSystem->getAliasNames();
    Q_FOREACH(const QString &alias, aliasNames)
    {
        QString fullName = pSystem->getFullNameFromAlias(alias);
        fullName.replace("#",".");
        toShortDataNames(fullName);
        if(allParameters.contains(fullName))
        {
            allParameters.append(alias);
        }
    }


    if(str.contains("*"))
    {
        QString temp = str;
        QStringList splitStr = temp.split("*");
        for(int p=0; p<allParameters.size(); ++p)
        {
            bool ok=true;
            QString name = allParameters[p];
            for(int s=0; s<splitStr.size(); ++s)
            {
                if(s==0)
                {
                    if(!name.startsWith(splitStr[s]))
                    {
                        ok=false;
                        break;
                    }
                    name.remove(0, splitStr[s].size());
                }
                else if(s==splitStr.size()-1)
                {
                    if(!name.endsWith(splitStr[s]))
                    {
                        ok=false;
                        break;
                    }
                }
                else
                {
                    if(!name.contains(splitStr[s]))
                    {
                        ok=false;
                        break;
                    }
                    name.remove(0, name.indexOf(splitStr[s])+splitStr[s].size());
                }
            }
            if(ok)
            {
                parameters.append(allParameters[p]);
            }
        }
    }
    else
    {
        if(allParameters.contains(str))
        {
            parameters.append(str);
        }
    }
}


//! @brief Returns the value of specified parameter
QString HcomHandler::getParameterValue(QString parameter) const
{
    toLongDataNames(parameter);
    parameter.remove("\"");
    QString compName = parameter.split("#").first();
    QString parName = parameter.right(parameter.size()-compName.size()-1);

    QString shortParName = parName;
    shortParName.prepend(".");
    toShortDataNames(shortParName);
    shortParName.remove(0,1);

    if(!mpModel)
    {
        return "NaN";
    }

    //! @todo is it working with the correct model here ?
    ContainerObject *pContainer = mpModel->getViewContainerObject();
    ModelObject *pComp = pContainer->getModelObject(compName);
    QString fullNameFromAlias = pContainer->getFullNameFromAlias(parName);
    ModelObject *pCompFromAlias = pContainer->getModelObject(fullNameFromAlias.section("#",0,0));
    if(pComp && pComp->getParameterNames().contains(parName))
    {
        return pComp->getParameterValue(parName);
    }
    else if(pComp && pComp->getParameterNames().contains(shortParName))
    {
        return pComp->getParameterValue(shortParName);
    }
    else if(pCompFromAlias && pCompFromAlias->getParameterNames().contains(fullNameFromAlias.section("#",1)))
    {
        return pCompFromAlias->getParameterValue(fullNameFromAlias.section("#",1));
    }
    else if(pContainer->getParameterNames().contains(parameter))
    {
        return pContainer->getParameterValue(parameter);
    }
    return "NaN";
}


//! @brief Returns variable names matching a pattern by communicating directly with the model
//! @param pattern Pattern using wildcards
//! @param rVariables Reference list used for returning matching variables
void HcomHandler::getMatchingLogVariableNamesWithoutLogDataHandler(QString pattern, QStringList &rVariables) const
{
    QStringList unFilteredVariables;

    QStringList components = mpModel->getTopLevelSystemContainer()->getModelObjectNames();
    Q_FOREACH(const QString &component, components)
    {
        ModelObject *pComponent = mpModel->getTopLevelSystemContainer()->getModelObject(component);
        QList<Port*> portPtrs = pComponent->getPortListPtrs();
        QStringList ports;
        Q_FOREACH(const Port *pPort, portPtrs)
        {
            ports.append(pPort->getName());
        }
        Q_FOREACH(const QString &port, ports)
        {
            Port *pPort = pComponent->getPort(port);
            NodeInfo nodeInfo(pPort->getNodeType());
            QStringList vars = nodeInfo.shortNames;

            Q_FOREACH(const QString &var, vars)
            {
                unFilteredVariables.append(component+"."+port+"."+var);
            }
        }
    }

    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);
    rVariables = unFilteredVariables.filter(rx);
}


//! @brief Help function that returns a list of variables according to input (with support for * regexp search)
//! @param [in] pattern Name to look for
//! @param [out] rVariables Reference to list that will contain the found variable names with generation appended
//! @warning If you make changes to this function you MUST MAKE SURE that all other Hcom functions using this is are still working for all cases. Many depend on the behavior of this function.
void HcomHandler::getMatchingLogVariableNames(QString pattern, QStringList &rVariables) const
{
    //TicToc timer;
    rVariables.clear();

    // Abort if no model found
    if(!mpModel) { return; }

    // Get pointers to logdatahandler
    LogDataHandler *pLogDataHandler = mpModel->getTopLevelSystemContainer()->getLogDataHandler();

    // Parse and chop the desired generation
    bool parseOK;
    int desiredGen = parseAndChopGenerationSpecifier(pattern, parseOK);

    // Convert short hcom format to long format
    QString pattern_long = pattern;
    toLongDataNames(pattern_long);

    // If we know generation then search for it directly
    if (desiredGen >= 0)
    {
        QList<SharedVectorVariableContainerT> varConts = pLogDataHandler->getVariableContainersMatching(QRegExp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard), desiredGen);
        for (int d=0; d<varConts.size(); ++d)
        {
            QString name = varConts[d]->getName();
            toShortDataNames(name);
            rVariables.append(name+QString(GenerationSpecifierChar"%1").arg(desiredGen+1));
        }
    }
    // Generate for latest in each variable
    else if (desiredGen == -1)
    {
        QList<SharedVectorVariableContainerT> varConts = pLogDataHandler->getVariableContainersMatching(QRegExp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard));
        for (int d=0; d<varConts.size(); ++d)
        {
            QString name = varConts[d]->getName();
            toShortDataNames(name);
            rVariables.append(name+QString(GenerationSpecifierChar"%1").arg(varConts[d]->getHighestGeneration()+1));
        }
    }
    // Do more costly name lookup, generate a list of all variables and all generations
    else if (desiredGen == -2)
    {
        QList<SharedVectorVariableContainerT> variables = pLogDataHandler->getVariableContainersMatching(QRegExp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard));
        for (int d=0; d<variables.size(); ++d)
        {
            QString name = variables[d]->getName();
            toShortDataNames(name);
            QList<int> gens = variables[d]->getGenerations();
            for (int g=0; g<gens.size(); ++g)
            {
                QString name2 = QString("%1"GenerationSpecifierChar"%2").arg(name).arg(gens[g]+1);
                rVariables.append(name2);
            }
        }
    }
    // Else generation number was not specified, lookup on names directly and do not append generations numbers to names
    else
    {
        QList<SharedVectorVariableContainerT> variableContainers = pLogDataHandler->getVariableContainersMatching(QRegExp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard));
        for (int d=0; d<variableContainers.size(); ++d)
        {
            QString name = variableContainers[d]->getName();
            toShortDataNames(name);
            rVariables.append(name);
        }
    }
    //timer.toc("getMatchingLogVariableNames("+pattern+")");
}

//! @brief Parses the generation specifier of a name and chops it from the name
//! @param[in,out] rStr A reference to the name string, the generation specifier will be chopped
//! @param[out] rOk A bool descibing if parsing was sucessfull
//! @returns The desired generation, or: -1 = Latest available, -2 = All generations, -3 = Not generation specified, -4 on failure
int HcomHandler::parseAndChopGenerationSpecifier(QString &rStr, bool &rOk) const
{
    rOk=true;
    int i = rStr.lastIndexOf(GenerationSpecifierChar);
    if ((i > -1) && mpModel)
    {
        QStringRef genStr = rStr.rightRef(rStr.size()-i-1);

        if( (genStr == "l") || (genStr == "L") )
        {
            rStr.chop(2);
            return mpModel->getTopLevelSystemContainer()->getLogDataHandler()->getLowestGenerationNumber();
        }
        else if( (genStr == "h") || (genStr == "H") )
        {
            rStr.chop(2);
            return mpModel->getTopLevelSystemContainer()->getLogDataHandler()->getHighestGenerationNumber();
        }
        else if( (genStr == "*") || (genStr == "a") || (genStr == "A") )
        {
            rStr.chop(2);
            return -2;
        }
        else
        {
            int g;
            if (toInt(genStr.toString(), g))
            {
                rStr.truncate(i);
                //! @todo what about handling zero, maybe should disp nothing
                // Here we take g-1 in order to "undo" the generations being displayed +1
                return qMax(g-1, -1);
            }
            else
            {
                return -4;
                rOk=false;
            }
        }
    }

    return -3;
}

//! @brief Help function that returns a list of variables according to input (with support for asterisks)
//! @param str String to look for
//! @param variables Reference to list of found variables
void HcomHandler::getLogVariablesThatStartsWithString(const QString str, QStringList &variables) const
{
    if(!mpModel) { return; }

    SystemContainer *pSystem = mpModel->getTopLevelSystemContainer();
    QStringList names = pSystem->getLogDataHandler()->getVariableFullNames(".");
    names.append(pSystem->getAliasNames());

    //Add quotation marks around component name if it contains spaces
    for(int n=0; n<names.size(); ++n)
    {
        if(names[n].split(".").first().contains(" "))
        {
            names[n].prepend("\"");
            names[n].insert(names[n].indexOf("."),"\"");
        }
    }

    //Translate long data names to short equivalents
    for(int n=0; n<names.size(); ++n)
    {
        toShortDataNames(names[n]);
    }

    for(int n=0; n<names.size(); ++n)
    {
        QString name = names[n];
        if(name.startsWith(str))
        {
            variables.append(names[n]);
        }
    }
}

void HcomHandler::setWorkingDirectory(QString dir)
{
    mPwd = dir;
}


QString HcomHandler::getWorkingDirectory() const
{
    return mPwd;
}

bool HcomHandler::hasFunction(const QString &func) const
{
    return mFunctions.contains(func);
}

void HcomHandler::getFunctionCode(QString funcName, QStringList &funcCode)
{
    funcCode = mFunctions.find(funcName).value();
}

bool HcomHandler::isAborted() const
{
    return mAborted;
}

double HcomHandler::getVar(const QString &var) const
{
    return mLocalVars.find(var).value();
}


//! @brief Checks if a command is an arithmetic expression and evaluates it if possible
//! @param cmd Command to evaluate
//! @returns True if it is a correct exrpession, otherwise false
bool HcomHandler::evaluateArithmeticExpression(QString cmd)
{
    //cmd.replace("**", "%%%%%");

    if(cmd.endsWith("*")) { return false; }

    SymHop::Expression expr = SymHop::Expression(cmd);

    //Assignment  (handle separately to update local variables not known to SymHop)
    if(expr.isAssignment())
    {
        QString left = expr.getLeft()->toString();

        QString right = expr.getRight()->toString();
        evaluateExpression(right);

        QStringList pars;
        if (mAnsType==Scalar)
        {
            getParameters(left, pars);
            SharedVectorVariableT data = getLogVariable(left).mpVariable;

            if(pars.isEmpty() && data)
            {
                //! @note Start values and data vectors may have same name, so we don't complain about this if it could also be a start value (parameter)
                HCOMERR("Not very clever to assign a data vector with a scalar.");
                return true;
            }
        }

        //Make sure left side is an acceptable variable name
        bool leftIsOk = left[0].isLetter();
        for(int i=1; i<left.size(); ++i)
        {
            if(!(left.at(i).isLetterOrNumber() || left.at(i) == '_' || left.at(i) == '.' || left.at(i) == ':'))
            {
                leftIsOk = false;
            }
        }

        if(!leftIsOk && (!mpModel || !getLogVariable(left)))
        {
            HCOMERR("Illegal variable name.");
            return false;
        }

        if(mAnsType==Scalar)
        {
            if(!pars.isEmpty())
            {
                executeCommand("chpa "+left+" "+QString::number(mAnsScalar));
                return true;
            }
            mLocalVars.insert(left, mAnsScalar);
            HCOMPRINT("Assigning scalar "+left+" with "+QString::number(mAnsScalar));
            return true;
        }
        else if(mAnsType==DataVector)
        {
            SharedVectorVariableT pLeftData = getLogVariable(left).mpVariable;
            if (pLeftData && mAnsVector)
            {
                if (pLeftData->getVariableType() != mAnsVector->getVariableType())
                {
                    HCOMWARN(QString("Variable type missmatch: %1 != %2, you may have lost information!").arg(variableTypeAsString(pLeftData->getVariableType())).arg(variableTypeAsString(mAnsVector->getVariableType())));
                }
                pLeftData->assignFrom(mAnsVector);
                return true;
            }
            else if (mpModel && mAnsVector)
            {
                // Value given but left does not exist, create it
                pLeftData = mpModel->getTopLevelSystemContainer()->getLogDataHandler()->defineNewVariable(left, mAnsVector->getVariableType());
                if (pLeftData)
                {
                    pLeftData->assignFrom(mAnsVector);
                    pLeftData->preventAutoRemoval();
                    mLocalVars.remove(left);    //Remove scalar variable with same name if it exists
                    return true;
                }
            }
            return false;
        }
        else
        {
            return false;
        }
    }
    else  //Not an assignment, evaluate with SymHop
    {
        //! @todo Should we allow pure expessions without assignment?
        //TicToc timer;
        evaluateExpression(cmd);
        //timer.toc("Evaluate expression "+cmd);
        if(mAnsType == Scalar)
        {
            HCOMPRINT(QString::number(mAnsScalar));
            return true;
        }
        else if(mAnsType==DataVector)
        {
            HCOMPRINT(mAnsVector.data()->getFullVariableName());
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}



//! @brief Returns data pointers for the variable with given full short name format (may include generation)
//! @param fullShortName Full concatinated name of the variable, (short name format expected)
//! @returns Pointers to the data variable and container
HopsanVariable HcomHandler::getLogVariable(QString fullShortName) const
{
    if(!mpModel)
    {
        return HopsanVariable();
    }

    int generation=-1;
    if (fullShortName.contains(GenerationSpecifierChar))
    {
        bool parseOK;
        generation = parseAndChopGenerationSpecifier(fullShortName, parseOK);
        if (!parseOK)
        {
            HCOMERR("Could not parse generation specifier");
        }
        else if (generation < -1)
        {
            HCOMWARN("Could not parse unique generation number, choosing highest");
            generation = -1;
        }
    }

    // Convert to long name
    toLongDataNames(fullShortName);
    return mpModel->getTopLevelSystemContainer()->getLogDataHandler()->getHopsanVariable(fullShortName, generation);
}


//! @brief Parses a string into a number
//! @param[in] rStr String to parse, should be a number of a variable name
//! @param[out] pOk Pointer to boolean that tells if parsing was successful
double HcomHandler::getNumber(const QString &rStr, bool *pOk)
{
    const double val = rStr.toDouble(pOk);
    if(*pOk)
    {
        return val;
    }

    *pOk=true;
    LocalVarsMapT::iterator it=mLocalVars.find(rStr);
    if(it!=mLocalVars.end())
    {
        return it.value();
    }
    else
    {
        evaluateExpression(rStr, Scalar);
        if(mAnsType==Scalar)
        {
            return mAnsScalar;
        }
    }
    *pOk = false;
    return -1;
}


//! @brief Converts long data names to short data names (e.g. "Component#Port#Pressure" -> "Component.Port.p")
//! @param rName Reference to variable name string
void HcomHandler::toShortDataNames(QString &rName) const
{
    rName.replace("#",".");

    int li = rName.lastIndexOf(".");
    if (li>=0)
    {
        QList<QString> shortVarName = gLongShortNameConverter.longToShort(rName.right(rName.size()-li-1));
        if (!shortVarName.isEmpty())
        {
            rName.chop(rName.size()-li-1);
            rName.append(shortVarName.first());
        }
    }

//    if(variable.endsWith(".Position"))
//    {
//        variable.chop(9);
//        variable.append(".x");
//    }
//    else if(variable.endsWith(".Velocity"))
//    {
//        variable.chop(9);
//        variable.append(".v");
//    }
//    else if(variable.endsWith(".Force"))
//    {
//        variable.chop(6);
//        variable.append(".f");
//    }
//    else if(variable.endsWith(".Pressure"))
//    {
//        variable.chop(9);
//        variable.append(".p");
//    }
//    else if(variable.endsWith(".Flow"))
//    {
//        variable.chop(5);
//        variable.append(".q");
//    }
//    else if(variable.endsWith(".Value"))
//    {
//        variable.chop(6);
//        variable.append(".val");
//    }
//    else if(variable.endsWith(".CharImpedance"))
//    {
//        variable.chop(14);
//        variable.append(".Zc");
//    }
//    else if(variable.endsWith(".WaveVariable"))
//    {
//        variable.chop(13);
//        variable.append(".c");
//    }
//    else if(variable.endsWith(".EquivalentMass"))
//    {
//        variable.chop(15);
//        variable.append(".me");
//    }
//    else if(variable.endsWith(".HeatFlow"))
//    {
//        variable.chop(9);
//        variable.append(".Q");
//    }
//    else if(variable.endsWith(".Temperature"))
//    {
//        variable.chop(12);
//        variable.append(".t");
//    }
}


//! @brief Converts short data names to long data names (e.g. "Component.Port.p" -> "Component#Port#Pressure")
void HcomHandler::toLongDataNames(QString &rName) const
{
    rName.replace(".", "#");
    rName.remove("\"");

    int li = rName.lastIndexOf("#");
    if (li>=0)
    {
        QStringList longNames = gLongShortNameConverter.shortToLong(rName.right(rName.size()-li-1));
        if (!longNames.isEmpty())
        {
            rName.chop(rName.size()-li-1);
            rName.append(longNames.first());

            //! @todo what if we get multiple matches
            if (longNames.size()>1)
            {
                qWarning() << "longNames.size() > 1 in HcomHandler::toLongDataNames. Is currently not IMPLEMENTED";
            }
        }
    }

//    if ( !rName.endsWith("#*") )
//    {
//        QList<QString> longNames;
//        int li = rName.lastIndexOf("#");
//        if (li>=0)
//        {
//            QString shortName = rName.right(rName.size()-li-1);
//            if (shortName.endsWith("*"))
//            {
//                longNames = gLongShortNameConverter.shortToLong(QRegExp(shortName, Qt::CaseSensitive, QRegExp::Wildcard));
//            }
//            else
//            {
//                longNames = gLongShortNameConverter.shortToLong(shortName);
//            }
//        }

//        if (!longNames.isEmpty())
//        {
//            rName.chop(rName.size()-li-1);
//            rName.append(longNames[0]);

//            //! @todo what if we get multiple matches
//            if (longNames.size()>1)
//            {
//                qWarning() << "longNames.size() > 1 in HcomHandler::toLongDataNames. Is currently not IMPLEMENTED";
//            }
//        }
//    }

//    if(var.endsWith("#x"))
//    {
//        var.chop(2);
//        var.append("#Position");
//    }
//    else if(var.endsWith("#v"))
//    {
//        var.chop(2);
//        var.append("#Velocity");
//    }
//    else if(var.endsWith("#f"))
//    {
//        var.chop(2);
//        var.append("#Force");
//    }
//    else if(var.endsWith("#p"))
//    {
//        var.chop(2);
//        var.append("#Pressure");
//    }
//    else if(var.endsWith("#q"))
//    {
//        var.chop(2);
//        var.append("#Flow");
//    }
//    else if(var.endsWith("#val"))
//    {
//        var.chop(4);
//        var.append("#Value");
//    }
//    else if(var.endsWith("#Zc"))
//    {
//        var.chop(3);
//        var.append("#CharImpedance");
//    }
//    else if(var.endsWith("#c"))
//    {
//        var.chop(2);
//        var.append("#WaveVariable");
//    }
//    else if(var.endsWith("#me"))
//    {
//        var.chop(3);
//        var.append("#EquivalentMass");
//    }
//    else if(var.endsWith("#Q"))
//    {
//        var.chop(2);
//        var.append("#HeatFlow");
//    }
//    else if(var.endsWith("#t"))
//    {
//        var.chop(2);
//        var.append("#Temperature");
//    }
}


//! @brief Converts a command to a directory path, or returns an empty string if command is invalid
QString HcomHandler::getDirectory(const QString &cmd) const
{
    if(QDir().exists(QDir().cleanPath(mPwd+"/"+cmd)))
    {
        return QDir().cleanPath(mPwd+"/"+cmd);
    }
    else if(QDir().exists(cmd))
    {
        return cmd;
    }
    else
    {
        return "";
    }
}


//! @brief Returns a list of arguments in a command with respect to quotation marks
QStringList HcomHandler::getArguments(const QString &cmd) const
{
    QStringList splitCmd;
    bool withinQuotations = false;
    int start=0;
    for(int i=0; i<cmd.size(); ++i)
    {
        if(cmd[i] == '\"')
        {
            withinQuotations = !withinQuotations;
        }
        if(cmd[i] == ' ' && !withinQuotations)
        {
            splitCmd.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmd.append(cmd.right(cmd.size()-start));
    //splitCmd.removeFirst();
    splitCmd.removeAll("");

    return splitCmd;
}


//! @brief Returns number of arguments in command with respect to quotation marks
int HcomHandler::getNumberOfArguments(const QString &cmd) const
{
    return getArguments(cmd).size();
}


//! @brief Returns argument in command at specified index with respect to quotation marks
//! @param cmd Command
//! @param idx Index
QString HcomHandler::getArgument(const QString &cmd, const int idx) const
{
    return getArguments(cmd).at(idx);
}


//! @brief Slot that aborts any HCOM script currently running
void HcomHandler::abortHCOM()
{
    mAborted = true;
}


void HcomHandler::registerInternalFunction(const QString &funcName, const QString &description, const QString &help)
{
    mLocalFunctionDescriptions.insert(funcName, QPair<QString, QString>(description, help));
}


//! @brief Registers a functionoid object with a function name in the functionoid map
//! @param funcName Name of function call from terminal
//! @param description Description shown in help text
//! @param pFunctionoid Pointer to functionoid object
void HcomHandler::registerFunctionoid(const QString &funcName, SymHopFunctionoid *pFunctinoid, const QString &description, const QString &help="")
{
    mLocalFunctionoidPtrs.insert(funcName, pFunctinoid);
    mLocalFunctionDescriptions.insert(funcName, QPair<QString, QString>(description, help));
}


//! @brief Function operator for the "aver" functionoid
double HcomFunctionoidAver::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str).mpVariable;
    if(!pData)
    {
        mpHandler->evaluateExpression(str);

        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = mpHandler->mAnsVector;
        }
        else if (mpHandler->mAnsType == HcomHandler::Scalar)
        {
            ok=true;
            return mpHandler->mAnsScalar;
        }
    }

    if(pData)
    {
        ok=true;
        return(pData->averageOfData());
    }

    mpHandler->mpConsole->printErrorMessage(QString("Failed to find variable %1").arg(str), "", false);
    ok=false;
    return 0;
}


//! @brief Function operator for the "peek" functionoid
double HcomFunctionoidPeek::operator()(QString &str, bool &ok)
{
    QString var = str.section(",",0,0);
    SharedVectorVariableT pData = mpHandler->getLogVariable(var).mpVariable;

    if(!pData)
    {
        mpHandler->evaluateExpression(str, HcomHandler::DataVector);
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = mpHandler->mAnsVector;
        }
        else
        {
            mpHandler->mpConsole->printErrorMessage(QString("%1, is not a vector").arg(str), "", false);
            ok = false;
            return 0;
        }
    }

    QString idxStr = str.section(",",1,1);

    SymHop::Expression idxExpr = SymHop::Expression(idxStr);
    QMap<QString, double> localVars = mpHandler->getLocalVariables();
    QMap<QString, SymHopFunctionoid*> localFuncs = mpHandler->getLocalFunctionoidPointers();
    bool evalOk;
    int idx = int(idxExpr.evaluate(localVars, &localFuncs, &evalOk)+0.1);
    if(pData && evalOk)
    {
        QString err;
        double val = pData->peekData(idx,err);
        ok = err.isEmpty();
        return val;
    }
    ok=false;
    return 0;
}


//! @brief Function operator for the "size" functionoid
double HcomFunctionoidSize::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str).mpVariable;

    if(!pData)
    {
        mpHandler->evaluateExpression(str, HcomHandler::DataVector);
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = mpHandler->mAnsVector;
        }
        else if (mpHandler->mAnsType == HcomHandler::Scalar)
        {
            ok=true;
            return 1;
        }
    }

    if(pData)
    {
        ok=true;
        return(pData->getDataSize());
    }
    mpHandler->mpConsole->printErrorMessage(QString("Failed to find variable %1").arg(str), "", false);
    ok=false;
    return 0;
}


//! @brief Function operator for the "time" functionoid
double HcomFunctionoidTime::operator()(QString &str, bool &ok)
{
    Q_UNUSED(str);
    if(mpHandler->getModelPtr())
    {
        ok=true;
        return mpHandler->getModelPtr()->getLastSimulationTime();
    }
    ok=false;
    return 0;
}


//! @brief Function operator for the "obj" functionoid
//! @todo Should be renamed to "optObj"
double HcomFunctionoidObj::operator()(QString &str, bool &ok)
{
    int idx = str.toDouble();
    ok=true;
    return mpHandler->mpOptHandler->getOptimizationObjectiveValue(idx);
}


//! @brief Function operator for the "min" functionoid
double HcomFunctionoidMin::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str).mpVariable;
    if(!pData)
    {
        mpHandler->evaluateExpression(str, HcomHandler::DataVector);
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = mpHandler->mAnsVector;
        }
        else if (mpHandler->mAnsType == HcomHandler::Scalar)
        {
            ok=true;
            return mpHandler->mAnsScalar;
        }
    }

    if(pData)
    {
        ok=true;
        return(pData->minOfData());
    }
    mpHandler->mpConsole->printErrorMessage(QString("Failed to find variable %1").arg(str), "", false);
    ok=false;
    return 0;
}


//! @brief Function operator for the "max" functionoid
double HcomFunctionoidMax::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str).mpVariable;
    if(!pData)
    {
        mpHandler->evaluateExpression(str, HcomHandler::DataVector);
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = mpHandler->mAnsVector;
        }
        else if (mpHandler->mAnsType == HcomHandler::Scalar)
        {
            ok=true;
            return mpHandler->mAnsScalar;
        }
    }

    if(pData)
    {
        ok=true;
        return(pData->maxOfData());
    }
    mpHandler->mpConsole->printErrorMessage(QString("Failed to find variable %1").arg(str), "", false);
    ok=false;
    return 0;
}


//! @brief Function operator for the "imin" functionoid
double HcomFunctionoidIMin::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str).mpVariable;

    if(!pData)
    {
        mpHandler->evaluateExpression(str, HcomHandler::DataVector);
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = mpHandler->mAnsVector;
        }
        else
        {
            mpHandler->mpConsole->printErrorMessage(QString("%1, is not a vector").arg(str), "", false);
            ok = false;
            return 0;
        }
    }

    if(pData)
    {
        ok=true;
        int idx;
        pData->minOfData(idx);
        return double(idx);
    }
    ok=false;
    return -1;
}


//! @brief Function operator for the "imax" functionoid
double HcomFunctionoidIMax::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str).mpVariable;

    if(!pData)
    {
        gpTerminalWidget->mpHandler->evaluateExpression(str, HcomHandler::DataVector);
        if(gpTerminalWidget->mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData = gpTerminalWidget->mpHandler->mAnsVector;
        }
        else
        {
            mpHandler->mpConsole->printErrorMessage(QString("%1, is not a vector").arg(str), "", false);
            ok = false;
            return 0;
        }
    }

    if(pData)
    {
        ok=true;
        int idx;
        pData->maxOfData(idx);
        return double(idx);
    }
    ok=false;
    return -1;
}


//! @brief Function operator for the "rand" functionoid
double HcomFunctionoidRand::operator()(QString &str, bool &ok)
{
    Q_UNUSED(str);
    ok=true;
    return rand() / (double)RAND_MAX;          //Random value between  0 and 1
}


//! @brief Function operator for the "optvar" functionoid
double HcomFunctionoidOptVar::operator()(QString &str, bool &ok)
{
    return mpHandler->mpOptHandler->getOptVar(str, ok);
}


//! @brief Function operator for the "optpar" functionoid
double HcomFunctionoidOptPar::operator()(QString &str, bool &ok)
{
    ok=true;
    QStringList args = SymHop::Expression::splitWithRespectToParentheses(str, ',');
    double pointIdx, parIdx;
    if(args.size() != 2)
    {
        ok = false;
        return 0;
    }
    mpHandler->evaluateExpression(args[0]);
    if(mpHandler->mAnsType != HcomHandler::Scalar)
    {
        ok = false;
        return 0;
    }
    else
    {
        pointIdx = mpHandler->mAnsScalar;
    }
    mpHandler->evaluateExpression(args[1]);
    if(mpHandler->mAnsType != HcomHandler::Scalar)
    {
        ok = false;
        return 0;
    }
    else
    {
        parIdx = mpHandler->mAnsScalar;
    }
    return mpHandler->mpOptHandler->getParameter(pointIdx,parIdx);
}

    //! @brief Function operator for the "fc" functionoid
double HcomFunctionoidFC::operator()(QString &str, bool &ok)
{
    QStringList args = str.split(',');
    if (args.size() != 3)
    {
        ok = false;
        mpHandler->mpConsole->printErrorMessage("Wrong number of arguments", "", false);
        return 0;
    }

    const double tol = args.last().toDouble(&ok);
    if (!ok)
    {
        mpHandler->mpConsole->printErrorMessage(QString("Could not evaluate tolerance %1").arg(args.last()), "", false);
        return 0;
    }

    ok=true;

    bool isScalar1=false, isScalar2=false;
    double scalar1, scalar2;
    SharedVectorVariableT pData1 = mpHandler->getLogVariable(args.first()).mpVariable;
    if(!pData1)
    {
        mpHandler->evaluateExpression(args.first());
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData1 = mpHandler->mAnsVector;
        }
        else if (mpHandler->mAnsType == HcomHandler::Scalar)
        {
            isScalar1 = true;
            scalar1 = mpHandler->mAnsScalar;
        }
        else
        {
            mpHandler->mpConsole->printErrorMessage(QString("Could not evaluate %1").arg(args.first()), "", false);
            ok = false;
            return 0;
        }
    }


    SharedVectorVariableT pData2 = mpHandler->getLogVariable(args[1]).mpVariable;
    if(!pData2)
    {
        mpHandler->evaluateExpression(args[1]);
        if(mpHandler->mAnsType == HcomHandler::DataVector)
        {
            pData2 = mpHandler->mAnsVector;
        }
        else if (mpHandler->mAnsType == HcomHandler::Scalar)
        {
            isScalar2 = true;
            scalar2 = mpHandler->mAnsScalar;
        }
        else
        {
            mpHandler->mpConsole->printErrorMessage(QString("Could not evaluate %1").arg(args[1]), "", false);
            ok = false;
            return 0;
        }
    }

    // If both are vector variables, then compare the data
    if (pData1 && pData2)
    {
        if ( pData1->compare(pData2, tol) )
        {
            return 1;
        }
    }
    // If both are scalars compare the scalars
    else if (isScalar1 && isScalar2)
    {
        if ( fuzzyEqual(scalar1, scalar2, tol) )
        {
            return 1;
        }
    }
    else if ((pData1 || pData2) && (isScalar1 || isScalar2))
    {
        //! @todo maybe support this as a elementwise comparisson
        mpHandler->mpConsole->printWarningMessage("Comparing scalar with vector (will fail)", "", false);
    }

    // Else they are not the same
    return 0;
}
