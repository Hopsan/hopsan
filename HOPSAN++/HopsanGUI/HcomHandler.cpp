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
//!
//HopsanGUI includes
#include "common.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "HcomHandler.h"
#include "MainWindow.h"
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

//HopsanGenerator includes
#include "SymHop.h"

//Dependency includes
#include "qwt_plot.h"



#define HCOMPRINT(text) mpConsole->print(text);
#define HCOMINFO(text) mpConsole->printInfoMessage(text,"",false)
#define HCOMWARN(text) mpConsole->printWarningMessage(text,"",false)
#define HCOMERR(text) mpConsole->printErrorMessage(text,"",false)


HcomHandler::HcomHandler(TerminalConsole *pConsole) : QObject(pConsole)
{
    mAborted = false;
    mLocalVars.insert("ans", 0);
    mRetvalType = Scalar;

    mpConsole = pConsole;
    mpOptHandler = new OptimizationHandler(this);

    mCurrentPlotWindowName = "PlotWindow0";

    mPwd = gDesktopHandler.getDocumentsPath();
    mPwd.chop(1);

    //Setup local function pointers (used to evaluate expressions in SymHop)
    registerFunction("aver", "Calculate average value of vector", &(_funcAver));
    registerFunction("min", "Calculate minimum value of vector", &(_funcMin));
    registerFunction("max", "Calculate maximum value of vector", &(_funcMax));
    registerFunction("imin", "Calculate index of minimum value of vector", &(_funcIMin));
    registerFunction("imax", "Calculate index of maximum value of vector", &(_funcIMax));
    registerFunction("size", "Calculate the size of a vector", &(_funcSize));
    registerFunction("rand", "Generates a random value between 0 and 1", &(_funcRand));
    registerFunction("peek", "Returns vector value at specified index", &(_funcPeek));
    registerFunction("obj", "Returns optimization objective function value with specified index", &(_funcObj));
    registerFunction("time", "Returns last simulation time", &(_funcTime));

    createCommands();
}


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
    chpwCmd.description.append("Changes current plot window");
    chpwCmd.help.append(" Usage: chpw [number]");
    chpwCmd.fnc = &HcomHandler::executeChangePlotWindowCommand;
    chpwCmd.group = "Plot Commands";
    mCmdList << chpwCmd;

    HcomCommand dipwCmd;
    dipwCmd.cmd = "dipw";
    dipwCmd.description.append("Displays current plot window");
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
    saplCmd.description.append("Saves plot file to .PLO");
    saplCmd.help.append(" Usage: sapl [filepath variables]");
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

    HcomCommand lp1Cmd;
    lp1Cmd.cmd = "lp1";
    lp1Cmd.description.append("Applies low-pass filter of first degree to vector");
    lp1Cmd.help.append(" Usage: lp1 [var]");
    lp1Cmd.fnc = &HcomHandler::executeLp1Command;
    lp1Cmd.group = "Variable Commands";
    mCmdList << lp1Cmd;

    HcomCommand semtCmd;
    semtCmd.cmd = "semt";
    semtCmd.description.append("Applies low-pass filter of first degree to vector");
    semtCmd.help.append(" Usage: semt [on/off threads algorithm]");
    semtCmd.fnc = &HcomHandler::executeSetMultiThreadingCommand;
    mCmdList << semtCmd;
}

void HcomHandler::generateCommandsHelpText()
{
    QFile htmlFile(gDesktopHandler.getHelpPath()+"userHcomScripting.html");
    htmlFile.open(QFile::ReadOnly | QFile::Text);
    QString htmlString = htmlFile.readAll();
    htmlFile.close();

    QString commands="</p>";
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

            commands.append("\n<h2><a class=\"anchor\" id=\"othercommands\"></a>Other Commands</h2>");
        }
        else
        {
            commands.append("\n<h2><a class=\"anchor\" id=\""+groups[g].toLower().replace(" ","")+"\"></a>"+groups[g]+"</h2>");
        }
        for(int c=0; c<mCmdList.size(); ++c)
        {
            if(mCmdList[c].group == groups[g])
            {
                commands.append("\n<h3><a class=\"anchor\" id=\""+mCmdList[c].cmd+"\"></a>"+mCmdList[c].cmd+"</h3>");
                commands.append("\n<p>"+mCmdList[c].description.replace("\n", "<br>")+"<br>"+mCmdList[c].help.replace("\n", "<br>")+"</p>");
            }
        }
    }

    htmlFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    htmlString.replace("<<<commands>>>", commands);
    htmlFile.write(htmlString.toUtf8());
    htmlFile.close();
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
QMap<QString, double> HcomHandler::getLocalVariables() const
{
    return mLocalVars;
}


//! @brief Returns a map with all local functions and pointers to them
QMap<QString, SymHop::Function> HcomHandler::getLocalFunctionPointers() const
{
    return mLocalFunctionPtrs;
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

//    ModelWidget *pCurrentTab = gpMainWindow->mpModelHandler->getCurrentModel();
//    SystemContainer *pCurrentSystem;
//    if(pCurrentTab)
//    {
//        pCurrentSystem = pCurrentTab->getTopLevelSystem();
//    }
//    else
//    {
//        pCurrentSystem = 0;
//    }

    if(idx<0)
    {
        if(evaluateArithmeticExpression(cmd)) { return; }
        HCOMERR("Unrecognized command: " + majorCmd);
    }
    else
    {
        mCmdList[idx].runCommand(subCmd, this);
    }
}


//! @brief Execute function for "exit" command
void HcomHandler::executeExitCommand(const QString /*cmd*/)
{
    gpMainWindow->close();
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
        gpMainWindow->mpModelHandler->simulateAllOpenModels_blocking(false);
        return;
    }
    else if(cmd == "")
    {
        ModelWidget *pCurrentModel = gpMainWindow->mpModelHandler->getCurrentModel();
        if(pCurrentModel)
        {
            pCurrentModel->simulate_blocking();
        }
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

    ContainerObject *pContainer = gpMainWindow->mpModelHandler->getCurrentViewContainerObject();
    if(pContainer)
    {
        CoreParameterData paramData(splitCmd[0], splitCmd[1], "double");
        pContainer->setOrAddParameter(paramData);
//        gpMainWindow->mpSystemParametersWidget->update();
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
        ModelWidget *pCurrentTab = gpMainWindow->mpModelHandler->getCurrentModel();
        if(!pCurrentTab) { return; }
        SystemContainer *pSystem = pCurrentTab->getTopLevelSystemContainer();
        if(!pSystem) { return; }

        QStringList parameterNames;
        getParameters(splitCmd[0], parameterNames);
        QString newValue = splitCmd[1];

        if(parameterNames.isEmpty())
        {
            HCOMERR("Parameter(s) not found.");
        }

        int nChanged=0;
        for(int p=0; p<parameterNames.size(); ++p)
        {
            if(pSystem->getParameterNames().contains(parameterNames[p]))
            {
                if(pSystem->setParameterValue(parameterNames[p], newValue))
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
                    if(pComponent->setParameterValue(parName, newValue))
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
                            bool ok;
                            VariableType varType;
                            if(pSystem->getParameterNames().contains(newValue)) //System parameter
                            {
                                if(components[c]->setParameterValue(parameters[p], newValue))
                                    ++nChanged;
                            }
                            else
                            {
                                toLongDataNames(parameters[p]);
                                if(components[c]->setParameterValue(parameters[p], evaluateExpression(newValue, &varType, &ok)))
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

        int samples;
        (void)samples;
        if(splitCmd.size() == 4)
        {
            samples = splitCmd[3].toInt(&ok);
            if(!ok) { allOk=false; }
        }

        if(allOk)
        {
            gpMainWindow->setStartTimeInToolBar(startT);
            gpMainWindow->setStopTimeInToolBar(stopT);
            gpMainWindow->setTimeStepInToolBar(timeStep);
            if(splitCmd.size() == 4)
            {
                ModelWidget *pCurrentTab = gpMainWindow->mpModelHandler->getCurrentModel();
                if(!pCurrentTab) { return; }
                SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystemContainer();
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
        QMapIterator<QString, QString> funcIt(mLocalFunctionDescriptions);
        while(funcIt.hasNext())
        {
            funcIt.next();
            commands.append("   "+funcIt.key()+"()");
            for(int i=0; i<n-funcIt.key().size()-2; ++i)
            {
                commands.append(" ");
            }
            commands.append(funcIt.value()+"\n");
        }

        HCOMPRINT(commands);
        HCOMPRINT(" Type: \"help [command]\" for more information about a specific command.");
        HCOMPRINT("-------------------------------------------------------------------------");
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
            HCOMERR("No help available for this command.");
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
    QStringList splitCmd;
    splitWithRespectToQuotations(cmd, ' ', splitCmd);
    if(splitCmd.size() < 1)
    {
        HCOMERR("Too few arguments.");
        return;
    }


    QString path = splitCmd[0];
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

    for(int i=0; i<splitCmd.size()-1; ++i)  //Replace arguments with their values
    {
        QString str = "$"+QString::number(i+1);
        code.replace(str, splitCmd[i+1]);
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
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString path = cmd;
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
        HCOMERR("Expected a string.");
        return;
    }

    int failed=0;
    while(str.count("$") > 1+failed*2)
    {
        QString varName = str.section("$",1+failed,1+failed);
        bool ok;
        VariableType type;
        QString varStr = evaluateExpression(varName, &type, &ok);
        if(ok && type == Scalar)
        {
            str.replace("$"+varName+"$", varStr);
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

    mCurrentPlotWindowName = cmd;
}


//! @brief Execute function for "dipw" command
void HcomHandler::executeDisplayPlotWindowCommand(const QString /*cmd*/)
{
    HCOMPRINT(mCurrentPlotWindowName);
}


//! @brief Execute function for "disp" command
void HcomHandler::executeDisplayVariablesCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) < 2)
    {
        QStringList output;
        if(cmd.isEmpty())
        {
            getVariables("*.H", output);
        }
        else
        {
            getVariables(cmd, output);
        }
        output.removeDuplicates();

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
    int id = getNumber(split.last(), &ok);
    if(!ok)
    {
        HCOMERR("Illegal value.");
        return;
    }

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        QString err;
        double r = pData->peekData(id, err);
        if (err.isEmpty())
        {
            returnScalar(r);
            HCOMPRINT(QString::number(r));
        }
        else
        {
            HCOMERR(err);
        }
    }
    else
    {
        HCOMERR("Data variable not found");
    }
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
    int id = getNumber(split[1], &ok1);
    double value = getNumber(split.last(), &ok2);
    if(!ok1 || !ok2)
    {
        HCOMERR("Illegal value.");
        return;
    }

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
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
    toShortDataNames(variable);
    variable.remove("\"");
    QString alias = splitCmd[1];

    //SharedLogVariableDataPtrT pVariable = getVariablePtr(variable);

    QString longName = variable;
    toLongDataNames(longName);
    if(/*!pVariable || */!gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->definePlotAlias(alias, longName/*pVariable->getFullVariableName()*/))
    {
        HCOMERR("Failed to assign variable alias.");
    }

    gpMainWindow->mpPlotWidget->mpPlotVariableTree->updateList();

    return;
}

void HcomHandler::executeRemoveVariableCommand(const QString cmd)
{
    //HCOMERR("Not implemented yet :(");
    QStringList varNames = getArguments(cmd);

    for(int s=0; s<varNames.size(); ++s)
    {
//        if(varNames[s].count(".") < 3 && varNames[s].endsWith("*"))     //Ugly hack because generation asterix is handled separately
//        {
//            varNames[s].append(".*");
//        }

        QStringList variables;
        getVariables(varNames[s], variables);
        for(int v=0; v<variables.size(); ++v)
        {
            deletePlotCurve(variables[v]);
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
    getVariables(getArgument(cmd,0),vars);
    if(vars.isEmpty())
    {
        HCOMERR("Unknown variable.");
        return;
    }
    Q_FOREACH(const QString var, vars)
    {
        QString varName = var;
        int size1  = varName.section(".",-1).size()+1;
        varName.chop(size1);
        int size2 = varName.section(".",-1).size()+1;
        varName.chop(size2);
        QList<ModelObject*> components;
        getComponents(varName, components);
        if(components.size() != 1)
            continue;

        //SharedLogVariableDataPtrT pVar = getVariablePtr(var);
        //pVar.data()->setPlotScale(scale);
        QString description = "";
        QString longVarName = var.right(size1+size2-1);
        toLongDataNames(longVarName);
        components[0]->registerCustomPlotUnitOrScale(longVarName, description, scale);
    }
}

void HcomHandler::executeDisplayDefaultPlotScaleCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString var = cmd;

    int size1  =var.section(".",-1).size()+1;
    var.chop(size1);
    int size2 = var.section(".",-1).size()+1;
    var.chop(size2);
    QList<ModelObject*> components;
    getComponents(var, components);
    if(components.size() != 1)
        return;

    UnitScale unitScale;
    QString portAndVarName = cmd.right(size1+size2-1);
    toLongDataNames(portAndVarName);
    components[0]->getCustomPlotUnitOrScale(portAndVarName, unitScale);

    QString scale = unitScale.mScale;
    QString unit = unitScale.mUnit;
    if(!unit.isEmpty())
    {
        HCOMPRINT(scale+" ["+unit+"]");
        returnScalar(scale.toDouble());
    }
    else
    {
        HCOMPRINT(scale);
        returnScalar(scale.toDouble());
    }

    return;
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
    getVariables(varName,vars);
    if(vars.isEmpty())
    {
        HCOMERR("Unknown variable.");
        return;
    }
    Q_FOREACH(const QString var, vars)
    {
        SharedLogVariableDataPtrT pVar = getVariablePtr(var);
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

    SharedLogVariableDataPtrT pVar = getVariablePtr(cmd);
    if(!pVar.isNull())
    {
        QString scale = pVar.data()->getCustomUnitScale().mScale;
        QString unit = pVar.data()->getCustomUnitScale().mUnit;
        if(!scale.isEmpty() && !unit.isEmpty())
        {
            HCOMPRINT(scale+" ["+unit+"]");
            returnScalar(scale.toDouble());
        }
        else if(!scale.isEmpty())
        {
            HCOMPRINT(scale);
            returnScalar(scale.toDouble());
        }
        else
        {
            scale = QString::number(pVar.data()->getPlotScale());
            unit = pVar.data()->getPlotScaleDataUnit();
            if(!scale.isEmpty() && !unit.isEmpty())
            {
                HCOMPRINT(scale+" ["+unit+"]");
                returnScalar(scale.toDouble());
            }
            else if(!scale.isEmpty())
            {
                HCOMPRINT(scale);
                returnScalar(scale.toDouble());
            }
        }
    }
    else
    {
        HCOMERR("Variable not found.");
    }

    return;
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
        gConfig.setUseMultiCore(value=="on");
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
        gConfig.setNumberOfThreads(nThreads);
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
        gConfig.setParallelAlgorithm(algorithm);
    }
    else if(pref == "cachetodisk")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        gConfig.setCacheLogData(value=="on");
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
        gConfig.setGenerationLimit(limit);
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
        gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->setNumberOfLogSamples(samples);
    }
    else
    {
        HCOMERR("Unknown command.");
    }
}


//! @brief Execute function for "sapl" command
void HcomHandler::executeSaveToPloCommand(const QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() < 2)
    {
        HCOMERR("Too few arguments.");
        return;
    }
    QString path = split.first();

    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));


    QString temp = cmd.right(cmd.size()-path.size()-1);

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
    QStringList allVariables;
    for(int i=0; i<splitCmdMajor.size(); ++i)
    {
        splitCmdMajor[i].remove("\"");
        QStringList variables;
        getVariables(splitCmdMajor[i], variables);
        for(int v=0; v<variables.size(); ++v)
        {
            toLongDataNames(variables[v]);
        }
        allVariables.append(variables);
        //splitCmdMajor[i] = getVariablePtr(splitCmdMajor[i])->getFullVariableName();
    }

    gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->exportToPlo(path, allVariables);
}

void HcomHandler::executeLoadVariableCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString filePath = getArgument(cmd,0);

    QFile file(filePath);
    if(!file.exists())
    {
        HCOMERR("File not found!");
        return;
    }

    bool csv;
    if(filePath.endsWith(".csv") || filePath.endsWith(".CSV"))
    {
        csv=true;
    }
    else if(filePath.endsWith(".plo") || filePath.endsWith(".PLO"))
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
        gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->importFromCsv(filePath);
    }
    else
    {
        gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->importFromPlo(filePath);
    }
}


//! @brief Execute function for "load" command
void HcomHandler::executeLoadModelCommand(const QString cmd)
{
    QString path = cmd;
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    gpMainWindow->mpModelHandler->loadModel(path);
}


//! @brief Execute function for "loadr" command
void HcomHandler::executeLoadRecentCommand(const QString /*cmd*/)
{
    gpMainWindow->mpModelHandler->loadModel(gConfig.getRecentModels().first());
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

    ContainerObject *pContainer = gpMainWindow->mpModelHandler->getCurrentViewContainerObject();
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

    ContainerObject *pContainer = gpMainWindow->mpModelHandler->getCurrentViewContainerObject();
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
    if(gpMainWindow->mpModelHandler->count() > 0)
    {
        HCOMPRINT(gpMainWindow->mpModelHandler->getCurrentModel()->getTopLevelSystemContainer()->getModelFileInfo().absoluteDir().path());
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
        mPwd = gpMainWindow->mpModelHandler->getCurrentModel()->getTopLevelSystemContainer()->getModelFileInfo().absoluteDir().path();
        HCOMPRINT(mPwd);
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

    if(gpMainWindow->mpModelHandler->count() > 0)
    {
        gpMainWindow->mpModelHandler->closeModelByTabIndex(gpMainWindow->mpCentralTabs->currentIndex());
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

    gpMainWindow->mpModelHandler->setCurrentModel(cmd.toInt());
}


//! @brief Execute function for "adco" command
void HcomHandler::executeAddComponentCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) < 5)
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
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelObject(otherName));
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
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelObject(otherName));
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
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelObject(otherName));
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
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelObject(otherName));
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
    Component *pObj = qobject_cast<Component*>(gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->addModelObject(typeName, pos, rot));
    if(!pObj)
    {
        HCOMERR("Failed to add new component. Incorrect typename?");
    }
    else
    {
        HCOMPRINT("Added "+typeName+" to current model.");
        gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->renameModelObject(pObj->getName(), name);
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

    Port *pPort1 = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelObject(args[0])->getPort(args[1]);
    Port *pPort2 = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelObject(args[2])->getPort(args[3]);

    Connector *pConn = gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->createConnector(pPort1, pPort2);

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
    gpMainWindow->mpModelHandler->addNewModel();
}


//! @brief Execute function for "fmu" command
void HcomHandler::executeExportToFMUCommand(const QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
    }

    gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->exportToFMU(getArgument(cmd, 0));
}


//! @brief Execute function for "chts" command
void HcomHandler::executeChangeTimestepCommand(const QString cmd)
{
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString component = split[0];
    QString timeStep = split[1];

    VariableType retType;
    bool isNumber;
    double value = evaluateExpression(split[1], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        HCOMERR("Second argument is not a number.");
    }
    else if(!gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->hasModelObject(component))
    {
        HCOMERR("Component not found.");
    }
    else
    {
        gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getCoreSystemAccessPtr()->setDesiredTimeStep(component, value);
        //gpMainWindow->mpModelHandler->getCurrentContainer()->getCoreSystemAccessPtr()->setInheritTimeStep(false);
        HCOMPRINT("Setting time step of "+component+" to "+QString::number(value));
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

    if(!gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->hasModelObject(component))
    {
        HCOMERR("Component not found.");
    }
    else
    {
        gpMainWindow->mpModelHandler->getCurrentViewContainerObject()->getCoreSystemAccessPtr()->setInheritTimeStep(component, true);
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
    SharedLogVariableDataPtrT pData1 = getVariablePtr(var1);
    SharedLogVariableDataPtrT pData2 = getVariablePtr(var2);
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

    gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Bode plot")->closeAllTabs();
    gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Bode plot")->addPlotTab();
    gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Bode plot")->createBodePlot(pData1, pData2, fMax);
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

    SharedLogVariableDataPtrT var = getVariablePtr(varName);
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
            returnScalar(retval);
            HCOMPRINT(QString::number(retval));
        }
        else
        {
            HCOMERR("Variable not found.");
        }
    }
}


//! @brief Execute function for "opt" command
void HcomHandler::executeOptimizationCommand(const QString cmd)
{
    QStringList split = getArguments(cmd);
//    if(split.size() == 1 && split[0] == "undo")
//    {
//        if(mOptAlgorithm == Uninitialized)
//        {
//            HCOMERR("Optimization not initialized.");
//            return;
//        }
//        if(mOptAlgorithm != Complex)
//        {
//            HCOMERR("Only available for complex algorithm.");
//            return;
//        }

//        mOptParameters = mOptOldParameters;
//        return;
//    }
    if(split.size() == 1 && split[0] == "worst")
    {
        if(mpOptHandler->mOptAlgorithm == OptimizationHandler::Uninitialized)
        {
            HCOMERR("Optimization not initialized.");
            return;
        }
        if(mpOptHandler->mOptAlgorithm != OptimizationHandler::Complex)
        {
            HCOMERR("Only available for complex algorithm.");
            return;
        }

        returnScalar(mpOptHandler->mOptLastWorstId);
        HCOMPRINT(QString::number(mpOptHandler->mOptLastWorstId));
        return;
    }
    if(split.size() == 1 && split[0] == "best")
    {
        if(mpOptHandler->mOptAlgorithm == OptimizationHandler::Uninitialized)
        {
            HCOMERR("Optimization not initialized.");
            return;
        }
        if(mpOptHandler->mOptAlgorithm != OptimizationHandler::Complex)
        {
            HCOMERR("Only available for complex algorithm.");
            return;
        }

        returnScalar(mpOptHandler->mOptBestId);
        HCOMPRINT(QString::number(mpOptHandler->mOptBestId));
        return;
    }
    if(split[0] == "set")
    {
        if(split.size() == 4 && split[1] == "obj")
        {
            bool ok;
            int nPoint = getNumber(split[2], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 2 must be a number.");
                return;
            }
            if(nPoint < 0 || nPoint > mpOptHandler->mOptObjectives.size()-1)
            {
                HCOMERR("Index out of range.");
                return;
            }

            double val = getNumber(split[3], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 3 must be a number.");
                return;
            }

            mpOptHandler->mOptObjectives[nPoint] = val;
            return;
        }
        else if(split.size() == 5 && split[1] == "limits")
        {
            bool ok;
            int nPoint = getNumber(split[2], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 2 must be a number.");
                return;
            }
            if(nPoint < 0 || nPoint > mpOptHandler->mOptParameters.size()-1)
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

            mpOptHandler->mOptParMin[nPoint] = min;
            mpOptHandler->mOptParMax[nPoint] = max;
            return;
        }
        else if(split.size() == 3 && split[1] == "plotpoints")
        {
            mpOptHandler->mOptPlotPoints = (split[2] == "on");
        }
        else if(split.size() == 3 && split[1] == "plotbestworst")
        {
            mpOptHandler->mOptPlotBestWorst = (split[2] == "on");
        }
        else if(split.size() == 3 && split[1] == "plotvariables")
        {
            mpOptHandler->mOptPlotVariables = (split[2] == "on");
        }
        else
        {
            HCOMERR("Unknown optimization setting: "+split[1]);
        }
    }

    if(split.size() == 3 && split[0] == "init")
    {
        if(split[1] == "complex")
        {
            mpOptHandler->mOptAlgorithm = OptimizationHandler::Complex;
        }
        else if(split[1] == "particleswarm")
        {
            mpOptHandler->mOptAlgorithm = OptimizationHandler::ParticleSwarm;
        }
        else
        {
            HCOMERR("Unknown algorithm. Only complex is supported.");
            return;
        }

        if(split[2] == "int")
        {
            mpOptHandler->mOptParameterType = OptimizationHandler::Int;
        }
        else if(split[2] == "double")
        {
            mpOptHandler->mOptParameterType = OptimizationHandler::Double;
        }
        else
        {
            HCOMERR("Unknown data type. Only int and double are supported.");
            return;
        }



        //Everything is fine, initialize and run optimization

        mpOptHandler->mpOptModel = gpMainWindow->mpModelHandler->loadModel(gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getModelFileInfo().absoluteFilePath(), true, true);

        bool ok;
        if(mpOptHandler->mOptAlgorithm == OptimizationHandler::Complex)
        {
            mpOptHandler->mOptMulticore=false;
            mpOptHandler->mOptNumPoints = getNumber("npoints", &ok);
            mpOptHandler->mOptNumParameters = getNumber("nparams", &ok);
            mpOptHandler->mOptLastWorstId = -1;
            mpOptHandler->mOptWorstCounter = 0;
            mpOptHandler->mOptParameters.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptParMin.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptParMax.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptMaxEvals = getNumber("maxevals", &ok);
            mpOptHandler->mOptAlpha = getNumber("alpha", &ok);
            mpOptHandler->mOptRfak = getNumber("rfak", &ok);
            mpOptHandler->mOptGamma = getNumber("gamma", &ok);
            mpOptHandler->mOptFuncTol = getNumber("functol", &ok);
            mpOptHandler->mOptParTol = getNumber("partol", &ok);
        }
        else if(mpOptHandler->mOptAlgorithm == OptimizationHandler::ParticleSwarm)
        {
            mpOptHandler->mOptMulticore=false;
            mpOptHandler->mOptNumPoints = getNumber("npoints", &ok);
            mpOptHandler->mOptNumParameters = getNumber("nparams", &ok);
            mpOptHandler->mOptParameters.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptVelocities.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptBestKnowns.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptBestPoint.resize(mpOptHandler->mOptNumParameters);
            mpOptHandler->mOptBestObjectives.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptParMin.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptParMax.resize(mpOptHandler->mOptNumPoints);
            mpOptHandler->mOptMaxEvals = getNumber("maxevals", &ok);
            mpOptHandler->mOptOmega = getNumber("omega", &ok);
            mpOptHandler->mOptC1 = getNumber("c1", &ok);
            mpOptHandler->mOptC2 = getNumber("c2", &ok);
            mpOptHandler->mOptFuncTol = getNumber("functol", &ok);
            mpOptHandler->mOptParTol = getNumber("partol", &ok);
        }

        return;
    }

    if(split.size() == 1 && split[0] == "run")
    {
        if(mpOptHandler->mOptAlgorithm == OptimizationHandler::Complex)
        {
            mpOptHandler->optComplexInit();
            mpOptHandler->optComplexRun();
        }
        else if(mpOptHandler->mOptAlgorithm == OptimizationHandler::ParticleSwarm)
        {
            mpOptHandler->optParticleInit();
            mpOptHandler->optParticleRun();
        }
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

    bool *abort = new bool;
    *abort = false;
    runScriptCommands(mFunctions.find(funcName).value(), abort);
    if(*abort)
    {
        HCOMPRINT("Function aborted");
        returnScalar(-1);
        return;
    }
    returnScalar(0);
}


//! @brief Execute function for "echo" command
void HcomHandler::executeEchoCommand(const QString cmd)
{
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


void HcomHandler::executeLp1Command(const QString /*cmd*/)
{

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

    gConfig.setUseMultiCore(useMultiThreading);
    if(nArgs > 1) gConfig.setNumberOfThreads(nThreads);
    if(nArgs > 2) gConfig.setParallelAlgorithm(algorithm);
}



//! @brief Changes plot variables on specified axes
//! @param cmd Command containing the plot variables
//! @param axis Axis specification (0=left, 1=right, -1=both, separeted by "-r")
void HcomHandler::changePlotVariables(const QString cmd, const int axis, bool hold) const
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
            QStringList variables;
            getVariables(varNames[s], variables);
            for(int v=0; v<variables.size(); ++v)
            {
                addPlotCurve(variables[v], axisId);
            }
        }
    }
}


//! @brief Adds a plot curve to specified axis in current plot
//! @param cmd Name of variable
//! @param axis Axis to add curve to
void HcomHandler::addPlotCurve(QString cmd, const int axis) const
{
    SystemContainer *pCurrentSystem = gpMainWindow->mpModelHandler->getCurrentModel()->getTopLevelSystemContainer();
    if(!pCurrentSystem) { return; }

    SharedLogVariableDataPtrT pData = getVariablePtr(cmd);
    if(!pData)
    {
        HCOMERR("Variable not found.");
        return;
    }

    SharedLogVariableDataPtrT pGenData = getVariablePtr(cmd);
    if(pData)
    {
        gpPlotHandler->plotDataToWindow(mCurrentPlotWindowName, pGenData, axis);
    }
}


//! @brief Adds a plot curve to specified axis in current plot
//! @param cmd Name of variable
//! @param axis Axis to add curve to
void HcomHandler::deletePlotCurve(QString cmd) const
{
    bool allGens=false;
    int generation=-1;
    if(cmd.contains("."))
    {
        QString end = cmd.section(".",-1);
        bool ok;
        generation = end.toInt(&ok)-1;
        if(ok)
        {
            cmd.chop(end.size()+1);
        }
    }



    cmd.remove("\"");

    SystemContainer *pCurrentSystem = gpMainWindow->mpModelHandler->getCurrentModel()->getTopLevelSystemContainer();
    if(!pCurrentSystem) { return; }

    SharedLogVariableDataPtrT pData = getVariablePtr(cmd);
    if(!pData)
    {
        if(cmd.endsWith(".*"))
        {
            cmd.chop(2);
            pData = getVariablePtr(cmd);
        }
        if(!pData)
        {
            HCOMERR("Variable not found: "+cmd);
            return;
        }
    }

    if(allGens || generation < 0)
    {
        pData->getLogVariableContainer()->getLogDataHandler()->deleteVariable(pData->getLogVariableContainer()->getFullVariableName());
    }
    else
    {
        pData->getLogVariableContainer()->removeDataGeneration(generation);
    }
}


//! @brief Removes all curves at specified axis in current plot
//! @param axis Axis to remove from
void HcomHandler::removePlotCurves(const int axis) const
{
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow(mCurrentPlotWindowName);
    if(pPlotWindow)
    {
        pPlotWindow->getCurrentPlotTab()->removeAllCurvesOnAxis(axis);
    }
}


QString HcomHandler::evaluateExpression(QString expr, VariableType *returnType, bool *evalOk)
{
    *evalOk = true;
    *returnType = Scalar;

    //expr.replace("==", "§§§§§");
    //expr.replace("**", "%%%%%");

    //Remove parentheses around expression
    QString tempStr = expr.mid(1, expr.size()-2);
    if(expr.count("(") == 1 && expr.count(")") == 1 && expr.startsWith("(") && expr.endsWith(")"))
    {
        expr = tempStr;
    }
    else if(expr.count("(") > 1 && expr.count(")") > 1 && expr.startsWith("(") && expr.endsWith(")"))
    {

        if(tempStr.indexOf("(") < tempStr.indexOf(")"))
        {
            expr = tempStr;
        }
    }

    //Numerical value, return it
    bool ok;
    expr.toDouble(&ok);
    if(ok)
    {
        return expr;
    }

    //Pre-defined variable, return its value
    if(mLocalVars.contains(expr))
    {
        return QString::number(mLocalVars.find(expr).value());
    }

    //Optimization parameter
    if(expr.startsWith("par(") && expr.endsWith(")"))
    {
        QString nPointsStr = expr.section("(",1,1).section(",",0,0);
        QString nParStr = expr.section(",",1,1).section(")",0,0);
        if(nPointsStr.isEmpty() || nParStr.isEmpty())
        {
            return QString::number(0);
        }
        bool ok1, ok2;
        int nPoint = getNumber(nPointsStr,&ok1);
        int nPar = getNumber(nParStr, &ok2);
        if(ok1 && ok2 && nPoint>=0 && nPoint < mpOptHandler->mOptParameters.size() && nPar>= 0 && nPar < mpOptHandler->mOptParameters[nPoint].size())
        {
            return QString::number(mpOptHandler->mOptParameters[nPoint][nPar]);
        }
    }

//    //Optimization objective
//    if(expr.startsWith("obj(") && expr.endsWith(")"))
//    {
//        QString nPointsStr = expr.section("(",1,1).section(")",0,0);

//        bool ok;
//        int nPoint = nPointsStr.toInt(&ok);
//        if(ok && nPoint>=0 && nPoint < mOptObjectives.size())
//        {
//            return QString::number(mOptObjectives[nPoint]);
//        }
//    }

    //Parameter name, return its value
    if(getParameterValue(expr) != "NaN")
    {
        return getParameterValue(expr);
    }

    //Data variable, return it
    QStringList variables;
    getVariables(expr,variables);
    if(!variables.isEmpty())
    {
        *returnType = DataVector;
        return variables.first();
    }

    //Vector functions
    LogDataHandler *pLogData=0;
    if(gpMainWindow->mpModelHandler->count() > 0)
    {
        pLogData = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler();
    }
    if(expr.startsWith("ddt(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        if(!args.contains(","))
        {
            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args);
            return pLogData->diffVariables(var.data()->getFullVariableName(), TIMEVARIABLENAME);
        }
        else
        {
            *returnType = DataVector;
            SharedLogVariableDataPtrT var1 = getVariablePtr(args.section(",",0,0));
            SharedLogVariableDataPtrT var2 = getVariablePtr(args.section(",",1,1));
            return pLogData->diffVariables(var1.data()->getFullVariableName(), var2.data()->getFullVariableName());
        }
    }
    else if(expr.startsWith("lp1(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        if(args.count(",")==1)
        {
            double freq = args.section(",",1,1).toDouble();
            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            return pLogData->lowPassFilterVariable(var.data()->getFullVariableName(), TIMEVARIABLENAME, freq);
        }
        else if(args.count(",") == 2)
        {
            double freq = args.section(",",2,2).toDouble();
            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            SharedLogVariableDataPtrT timeVar = getVariablePtr(args.section(",",1,1));
            return pLogData->lowPassFilterVariable(var.data()->getFullVariableName(), timeVar.data()->getFullVariableName(), freq);
        }
    }
    else if(expr.startsWith("fft(") && expr.endsWith(")"))
    {
        QString args = expr.mid(4, expr.size()-5);
        if(args.count(",")==0)
        {
            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            return pLogData->fftVariable(var.data()->getFullVariableName(), TIMEVARIABLENAME, false);
        }
        if(args.count(",")==1)
        {
            QString arg2=args.section(",",1,1);
            bool power=false;
            QString timeVec = TIMEVARIABLENAME;
            if(arg2 == "true" || arg2 == "false")
            {
                power = (args.section(",",1,1) == "true");
            }
            else
            {
                timeVec = arg2;
            }
            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            return pLogData->fftVariable(var.data()->getFullVariableName(), timeVec, power);
        }
        else if(args.count(",") == 2)
        {
            bool power = (args.section(",",2,2) == "true");
            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            SharedLogVariableDataPtrT timeVar = getVariablePtr(args.section(",",1,1));
            return pLogData->fftVariable(var.data()->getFullVariableName(), timeVar.data()->getFullVariableName(), power);
        }
    }
    else if(expr.startsWith("gt(") && expr.endsWith(")"))
    {
        QString args = expr.mid(3, expr.size()-4);
        if(args.count(",")==1)
        {
            QString arg2=args.section(",",1,1);
            bool success;
            double limit = arg2.toDouble(&success);

            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            SharedLogVariableDataPtrT resVar = pLogData->defineTempVariable(var.data()->getFullVariableName()+"gt");
            resVar.data()->assignFrom(var);
            int size = var.data()->getDataSize();
            QString error;
            for(int i=0; i<size; ++i)
            {
                if(var.data()->peekData(i, error) > limit)
                {
                    resVar.data()->pokeData(i, 1, error);
                }
                else
                {
                    resVar.data()->pokeData(i, 0, error);
                }
            }
            return resVar.data()->getFullVariableName();
        }
        else
        {
            return "0";
        }
    }
    else if(expr.startsWith("lt(") && expr.endsWith(")"))
    {
        QString args = expr.mid(3, expr.size()-4);
        if(args.count(",")==1)
        {
            QString arg2=args.section(",",1,1);
            bool success;
            double limit = arg2.toDouble(&success);

            *returnType = DataVector;
            SharedLogVariableDataPtrT var = getVariablePtr(args.section(",",0,0));
            SharedLogVariableDataPtrT resVar = pLogData->defineTempVariable(var.data()->getFullVariableName()+"lt");
            resVar.data()->assignFrom(var);
            int size = var.data()->getDataSize();
            QString error;
            for(int i=0; i<size; ++i)
            {
                if(var.data()->peekData(i, error) < limit)
                {
                    resVar.data()->pokeData(i, 1, error);
                }
                else
                {
                    resVar.data()->pokeData(i, 0, error);
                }
            }
            return resVar.data()->getFullVariableName();
        }
        else
        {
            return "0";
        }
    }

    //Evaluate expression using SymHop
    SymHop::Expression symHopExpr = SymHop::Expression(expr);

    //Multiplication between data vector and scalar
    *returnType = DataVector;
    if(symHopExpr.getFactors().size() == 2 && pLogData)
    {
        SymHop::Expression f0 = symHopExpr.getFactors()[0];
        SymHop::Expression f1 = symHopExpr.getFactors()[1];
        if(!getVariablePtr(f0.toString()).isNull() && f1.isNumericalSymbol())
            return pLogData->mulVariableWithScalar(getVariablePtr(f0.toString()), f1.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(f1.toString()).isNull() && f0.isNumericalSymbol())
            return pLogData->mulVariableWithScalar(getVariablePtr(f1.toString()), f0.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(f0.toString()).isNull() && !getVariablePtr(f1.toString()).isNull())
            return pLogData->multVariables(getVariablePtr(f0.toString()), getVariablePtr(f1.toString())).data()->getFullVariableName();
    }
    if(symHopExpr.isPower() && pLogData)
    {
        SymHop::Expression b = *symHopExpr.getBase();
        SymHop::Expression p = *symHopExpr.getPower();
        if(!getVariablePtr(b.toString()).isNull() && p.toDouble() == 2.0)
            return pLogData->multVariables(getVariablePtr(b.toString()), getVariablePtr(b.toString())).data()->getFullVariableName();
    }
    if(symHopExpr.getFactors().size() == 1 && symHopExpr.getDivisors().size() == 1 && pLogData)
    {
        SymHop::Expression f = symHopExpr.getFactors()[0];
        SymHop::Expression d = symHopExpr.getDivisors()[0];
        if(!getVariablePtr(f.toString()).isNull() && d.isNumericalSymbol())
            return pLogData->divVariableWithScalar(getVariablePtr(f.toString()), d.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(f.toString()).isNull() && !getVariablePtr(d.toString()).isNull())
            return pLogData->divVariables(getVariablePtr(f.toString()), getVariablePtr(d.toString())).data()->getFullVariableName();
    }
    if(symHopExpr.getTerms().size() == 2 && pLogData)
    {
        SymHop::Expression t0 = symHopExpr.getTerms()[0];
        SymHop::Expression t1 = symHopExpr.getTerms()[1];
        if(!getVariablePtr(t0.toString()).isNull() && t1.isNumericalSymbol())
            return pLogData->addVariableWithScalar(getVariablePtr(t0.toString()), t1.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(t1.toString()).isNull() && t0.isNumericalSymbol())
            return pLogData->addVariableWithScalar(getVariablePtr(t1.toString()), t0.toDouble()).data()->getFullVariableName();
        else if(!getVariablePtr(t0.toString()).isNull() && !getVariablePtr(t1.toString()).isNull())
            return pLogData->addVariables(getVariablePtr(t0.toString()), getVariablePtr(t1.toString())).data()->getFullVariableName();

        if(t1.isNegative())
        {
            t1.changeSign();
            if(!getVariablePtr(t0.toString()).isNull() && t1.isNumericalSymbol())
                return pLogData->subVariableWithScalar(getVariablePtr(t0.toString()), t1.toDouble()).data()->getFullVariableName();
            else if(!getVariablePtr(t1.toString()).isNull() && t0.isNumericalSymbol())
                return pLogData->subVariableWithScalar(getVariablePtr(t1.toString()), t0.toDouble()).data()->getFullVariableName();
            else if(!getVariablePtr(t0.toString()).isNull() && !getVariablePtr(t1.toString()).isNull())
                return pLogData->subVariables(getVariablePtr(t0.toString()), getVariablePtr(t1.toString())).data()->getFullVariableName();
        }
    }

    *returnType = Scalar;

    QMap<QString, double> localVars = mLocalVars;
    QStringList localPars;
    getParameters("*", localPars);
    for(int p=0; p<localPars.size(); ++p)
    {
        localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
    }
    return QString::number(symHopExpr.evaluate(localVars, mLocalFunctionPtrs));
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


QString HcomHandler::runScriptCommands(QStringList &lines, bool *abort)
{
    mAborted = false; //Reset if pushed when script didn't run

    QString funcName="";
    QStringList funcCommands;

    qDebug() << "Number of commands to run: " << lines.size();

    for(int l=0; l<lines.size(); ++l)
    {
        qApp->processEvents();
        if(mAborted)
        {
            HCOMPRINT("Script aborted.");
            mAborted = false;
            *abort=true;
        }

        while(lines[l].startsWith(" "))
        {
            lines[l] = lines[l].right(lines[l].size()-1);
        }
        if(lines[l].startsWith("#") || lines[l].startsWith("&")) { continue; }  //Ignore comments and labels

        if(lines[l].startsWith("stop"))
        {
            return "%%%%%EOF";
        }
        else if(lines[l].startsWith("define "))
        {
            funcName = lines[l].section(" ",1);
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
            QString argument = lines[l].section("(",1).remove(")");
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
            QMap<QString, double> localVars = mLocalVars;
            QStringList localPars;
            getParameters("*", localPars);
            for(int p=0; p<localPars.size(); ++p)
            {
                localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
            }
            while(symHopExpr.evaluate(localVars) > 0)
            {
                qApp->processEvents();
                if(mAborted)
                {
                    HCOMPRINT("Script aborted.");
                    mAborted = false;
                    *abort=true;
                    return "";
                }
                QString gotoLabel = runScriptCommands(loop, abort);
                if(*abort)
                {
                    return "";
                }
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }

                //Update local variables for SymHop in case they have changed
                localVars = mLocalVars;
                QStringList localPars;
                getParameters("*", localPars);
                for(int p=0; p<localPars.size(); ++p)
                {
                    localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
                }
            }
        }
        else if(lines[l].startsWith("if"))        //Handle if statements
        {
            QString argument = lines[l].section("(",1).remove(")");
            qDebug() << "Argument: " << argument;
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

            bool evalOk;
            VariableType type;
            if(evaluateExpression(argument, &type, &evalOk).toInt() > 0)
            {
                if(!evalOk)
                {
                    HCOMERR("Evaluation of if-statement argument failed.");
                    return QString();
                }
                QString gotoLabel = runScriptCommands(ifCode, abort);
                if(*abort)
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
                if(!evalOk)
                {
                    HCOMERR("Evaluation of if-statement argument failed.");
                    return QString();
                }
                QString gotoLabel = runScriptCommands(elseCode, abort);
                if(*abort)
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
            getVariables(filter, vars);
            QStringList loop;
            while(!lines[l].startsWith("endforeach"))
            {
                ++l;
                loop.append(lines[l]);
            }
            loop.removeLast();

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
                QString gotoLabel = runScriptCommands(tempCmds, abort);
                if(*abort)
                {
                    return "";
                }
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
        }
        else
        {
            this->executeCommand(lines[l]);
        }
    }
    return QString();
}


//! @brief Help function that returns a list of components depending on input (with support for asterisks)
//! @param str String to look for
//! @param components Reference to list of found components
void HcomHandler::getComponents(QString str, QList<ModelObject*> &components)
{
    QString left = str.split("*").first();
    QString right = str.split("*").last();

    ModelWidget *pCurrentTab = gpMainWindow->mpModelHandler->getCurrentModel();
    if(!pCurrentTab) { return; }
    SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystemContainer();
    if(!pCurrentSystem) { return; }
    for(int n=0; n<pCurrentSystem->getModelObjectNames().size(); ++n)
    {
        if(pCurrentSystem->getModelObjectNames().at(n).startsWith(left) && pCurrentSystem->getModelObjectNames().at(n).endsWith(right))
        {
            components.append(pCurrentSystem->getModelObject(pCurrentSystem->getModelObjectNames().at(n)));
        }
    }
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
    if(gpMainWindow->mpModelHandler->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();

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

    if(gpMainWindow->mpModelHandler->count() == 0)
    {
        return "NaN";
    }

    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();
    ModelObject *pComp = pSystem->getModelObject(compName);
    QString fullNameFromAlias = pSystem->getFullNameFromAlias(parName);
    ModelObject *pCompFromAlias = pSystem->getModelObject(fullNameFromAlias.section("#",0,0));
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
    else if(pSystem->getParameterNames().contains(parameter))
    {
        return pSystem->getParameterValue(parameter);
    }
    return "NaN";
}

//! @brief Help function that returns a list of variables according to input (with support for asterisks)
//! @param str String to look for
//! @param variables Reference to list of found variables
void HcomHandler::getVariables(QString str, QStringList &variables) const
{
    if(gpMainWindow->mpModelHandler->count() == 0) { return; }

    if(str.endsWith(".L"))
    {
        str.chop(1);
        int generation = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->getLowestGenerationNumber();
        str.append(QString::number(generation+1));
    }
    else if(str.endsWith(".H"))
    {
        str.chop(1);
        int generation = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->getHighestGenerationNumber();
        str.append(QString::number(generation+1));
    }

    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();
    QStringList names = pSystem->getLogDataHandler()->getLogDataVariableNames("#", -1);
    names.append(pSystem->getAliasNames());

    QStringList namesWithGeneration;
    for(int i=pSystem->getLogDataHandler()->getLowestGenerationNumber(); i<pSystem->getLogDataHandler()->getHighestGenerationNumber()+1; ++i)
    {
        for(int n=0; n<names.size(); ++n)
        {
            QString shortName = names[n];
            toShortDataNames(shortName);
            if(!pSystem->getLogDataHandler()->getPlotData(names[n],i).isNull())
            {
                namesWithGeneration.append(shortName+"."+QString::number(i+1));
            }
        }
    }

    QRegExp re(str, Qt::CaseSensitive, QRegExp::Wildcard);
    Q_FOREACH(const QString name, namesWithGeneration)
    {
        if(re.exactMatch(name))
            variables.append(name);
    }

    //Found no variables, try without generations
    if(variables.isEmpty())
    {
        Q_FOREACH(const QString name, namesWithGeneration)
        {
            QString choppedName = name;
            choppedName.chop(name.section(".",-1,-1).size()+1);
            if(re.exactMatch(choppedName))
                variables.append(choppedName);
        }
    }
    variables.removeDuplicates();
}

//! @brief Help function that returns a list of variables according to input (with support for asterisks)
//! @param str String to look for
//! @param variables Reference to list of found variables
void HcomHandler::getVariablesThatStartsWithString(const QString str, QStringList &variables) const
{
    if(gpMainWindow->mpModelHandler->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem();
    QStringList names = pSystem->getLogDataHandler()->getLogDataVariableNames(".");
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

        VariableType type;
        bool evalOk;
        QString right = expr.getRight()->toString();
        QString value = evaluateExpression(right, &type, &evalOk);

        QStringList vars;
        getVariables(left, vars);
        if(!vars.isEmpty() && type==Scalar)
        {
            HCOMERR("Not very clever to assign a data vector with a scalar.");
            return true;
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

//        QStringList plotDataNames;
//        if(gpMainWindow->mpModelWidgets->count() > 0)
//        {
//            LogDataHandler *pLogData = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler();
//            plotDataNames = pLogData->getPlotDataNames();
//        }
//        if(!leftIsOk && !plotDataNames.contains(left))
        if(!leftIsOk && (gpMainWindow->mpModelHandler->count() == 0 || !getVariablePtr(left)))
        {
            HCOMERR("Illegal variable name.");
            return false;
        }

        if(evalOk && type==Scalar)
        {
            QStringList pars;
            getParameters(left, pars);
            if(!pars.isEmpty())
            {
                executeCommand("chpa "+left+" "+value);
                HCOMPRINT("Assigning "+left+" with "+value);
                return true;
            }
            mLocalVars.insert(left, value.toDouble());
            HCOMPRINT("Assigning "+left+" with "+value);
            mLocalVars.insert("ans", value.toDouble());
            return true;
        }
        else if(evalOk && type==DataVector)
        {
            SharedLogVariableDataPtrT pLeftData = getVariablePtr(left);
            SharedLogVariableDataPtrT pValueData = getVariablePtr(value);
            if(pLeftData != 0) { left = pLeftData->getFullVariableName(); }
            if(pValueData != 0) { value = pValueData->getFullVariableName(); }

            gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->assignVariable(left, value);
            gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotData(left,-1).data()->preventAutoRemoval();
            //! @todo maybe we should remove value if it is a temporary variable, or else it will remain for ever
            return true;
        }
        else
        {
            return false;
        }
    }
    else  //Not an assignment, evaluate with SymHop
    {
        //! @todo Should we allow pure expessions without assignment?
        bool evalOk;
        VariableType type;
        QString value=evaluateExpression(cmd, &type, &evalOk);
        if(evalOk && type==Scalar)
        {
            returnScalar(value.toDouble());
            HCOMPRINT(QString::number(value.toDouble()));
            return true;
        }
        else if(evalOk && type==DataVector)
        {
            mRetvalType = DataVector;
            HCOMPRINT(value);
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}



//! @brief Returns a pointer to a data variable for given full data name
//! @param fullName Full concatinated name of the variable
//! @returns Pointer to the data variable
SharedLogVariableDataPtrT HcomHandler::getVariablePtr(QString fullName) const
{
    if(gpMainWindow->mpModelHandler->count() == 0)
    {
        return SharedLogVariableDataPtrT(0);
    }

    fullName.replace(".","#");

    int generation=-1;

    if(fullName.count("#") == 1 || fullName.count("#") == 3)
    {
        generation = fullName.split("#").last().toInt()-1;      //Subtract 1 due to zero indexing
        fullName.chop(fullName.split("#").last().size()+1);
    }

    if(fullName.endsWith("#x"))
    {
        fullName.chop(2);
        fullName.append("#Position");
    }
    else if(fullName.endsWith("#v"))
    {
        fullName.chop(2);
        fullName.append("#Velocity");
    }
    else if(fullName.endsWith("#f"))
    {
        fullName.chop(2);
        fullName.append("#Force");
    }
    else if(fullName.endsWith("#p"))
    {
        fullName.chop(2);
        fullName.append("#Pressure");
    }
    else if(fullName.endsWith("#q"))
    {
        fullName.chop(2);
        fullName.append("#Flow");
    }
    else if(fullName.endsWith("#val"))
    {
        fullName.chop(4);
        fullName.append("#Value");
    }
    else if(fullName.endsWith("#Zc"))
    {
        fullName.chop(3);
        fullName.append("#CharImpedance");
    }
    else if(fullName.endsWith("#c"))
    {
        fullName.chop(2);
        fullName.append("#WaveVariable");
    }
    else if(fullName.endsWith("#me"))
    {
        fullName.chop(3);
        fullName.append("#EquivalentMass");
    }
    else if(fullName.endsWith("#Q"))
    {
        fullName.chop(2);
        fullName.append("#HeatFlow");
    }
    else if(fullName.endsWith("#t"))
    {
        fullName.chop(2);
        fullName.append("#Temperature");
    }

//    SharedLogVariableDataPtrT pRetVal = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotDataByAlias(fullName,generation);
//    if(!pRetVal)
//    {
        SharedLogVariableDataPtrT pRetVal = gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotData(fullName,generation);
//    }
    return pRetVal;
}


//! @brief Parses a string into a number
//! @param str String to parse, should be a number of a variable name
//! @param ok Pointer to boolean that tells if parsing was successful
double HcomHandler::getNumber(const QString str, bool *ok)
{
    *ok = true;
    if(str.toDouble())
    {
        return str.toDouble();
    }
    else if(mLocalVars.contains(str))
    {
        return mLocalVars.find(str).value();
    }
    else
    {
        VariableType retType;
        bool isNumber;
        double retval = evaluateExpression(str, &retType, &isNumber).toDouble();
        if(isNumber && retType==Scalar)
        {
            return retval;
        }
    }
    *ok = false;
    return 0;
}


//! @brief Converts long data names to short data names (e.g. "Pressure" -> "p")
//! @param variable Reference to variable string
void HcomHandler::toShortDataNames(QString &variable) const
{
    variable.replace("#",".");

    if(variable.endsWith(".Position"))
    {
        variable.chop(9);
        variable.append(".x");
    }
    else if(variable.endsWith(".Velocity"))
    {
        variable.chop(9);
        variable.append(".v");
    }
    else if(variable.endsWith(".Force"))
    {
        variable.chop(6);
        variable.append(".f");
    }
    else if(variable.endsWith(".Pressure"))
    {
        variable.chop(9);
        variable.append(".p");
    }
    else if(variable.endsWith(".Flow"))
    {
        variable.chop(5);
        variable.append(".q");
    }
    else if(variable.endsWith(".Value"))
    {
        variable.chop(6);
        variable.append(".val");
    }
    else if(variable.endsWith(".CharImpedance"))
    {
        variable.chop(14);
        variable.append(".Zc");
    }
    else if(variable.endsWith(".WaveVariable"))
    {
        variable.chop(13);
        variable.append(".c");
    }
    else if(variable.endsWith(".EquivalentMass"))
    {
        variable.chop(15);
        variable.append(".me");
    }
    else if(variable.endsWith(".HeatFlow"))
    {
        variable.chop(9);
        variable.append(".Q");
    }
    else if(variable.endsWith(".Temperature"))
    {
        variable.chop(12);
        variable.append(".t");
    }
}



void HcomHandler::toLongDataNames(QString &var) const
{
    var.replace(".", "#");
    var.remove("\"");

    if(var.endsWith("#x"))
    {
        var.chop(2);
        var.append("#Position");
    }
    else if(var.endsWith("#v"))
    {
        var.chop(2);
        var.append("#Velocity");
    }
    else if(var.endsWith("#f"))
    {
        var.chop(2);
        var.append("#Force");
    }
    else if(var.endsWith("#p"))
    {
        var.chop(2);
        var.append("#Pressure");
    }
    else if(var.endsWith("#q"))
    {
        var.chop(2);
        var.append("#Flow");
    }
    else if(var.endsWith("#val"))
    {
        var.chop(4);
        var.append("#Value");
    }
    else if(var.endsWith("#Zc"))
    {
        var.chop(3);
        var.append("#CharImpedance");
    }
    else if(var.endsWith("#c"))
    {
        var.chop(2);
        var.append("#WaveVariable");
    }
    else if(var.endsWith("#me"))
    {
        var.chop(3);
        var.append("#EquivalentMass");
    }
    else if(var.endsWith("#Q"))
    {
        var.chop(2);
        var.append("#HeatFlow");
    }
    else if(var.endsWith("#t"))
    {
        var.chop(2);
        var.append("#Temperature");
    }
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


//! @brief Auxiliary function used by command functions to "return" a scalar
void HcomHandler::returnScalar(const double retval)
{
    mLocalVars.insert("ans", retval);
    mRetvalType = Scalar;
    //HCOMPRINT(QString::number(retval));
}

void HcomHandler::registerFunction(const QString func, const QString description, const SymHop::Function fptr)
{
    mLocalFunctionPtrs.insert(func, fptr);
    mLocalFunctionDescriptions.insert(func, description);
}

double _funcAver(QString str, bool &ok)
{
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(str);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    if(pData)
    {
        ok=true;
        return(pData->averageOfData());
    }
    ok=false;
    return 0;
}

double _funcSize(QString str, bool &ok)
{
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(str);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    if(pData)
    {
        ok=true;
        return(pData->getDataSize());
    }
    ok=false;
    return 0;
}

double _funcTime(QString /*str*/, bool &ok)
{
    if(gpMainWindow->mpModelHandler->count() > 0)
    {
        ok=true;
        return gpMainWindow->mpModelHandler->getCurrentModel()->getLastSimulationTime();
    }
    ok=false;
    return 0;
}


double _funcObj(QString str, bool &ok)
{
    int idx = str.toDouble();
    ok=true;
    return gpMainWindow->mpTerminalWidget->mpHandler->mpOptHandler->getOptimizationObjectiveValue(idx);
}

double _funcMin(QString str, bool &ok)
{
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(str);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    if(pData)
    {
        ok=true;
        return(pData->minOfData());
    }
    ok=false;
    return 0;
}

double _funcMax(QString str, bool &ok)
{
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(str);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    if(pData)
    {
        ok=true;
        return(pData->maxOfData());
    }
    ok=false;
    return 0;
}

double _funcIMin(QString str, bool &ok)
{
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(str);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    if(pData)
    {
        ok=true;
        return(pData->indexOfMinOfData());
    }
    ok=false;
    return 0;
}

double _funcIMax(QString str, bool &ok)
{
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(str);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    if(pData)
    {
        ok=true;
        return(pData->indexOfMaxOfData());
    }
    ok=false;
    return 0;
}

double _funcPeek(QString str, bool &ok)
{
    QString var = str.section(",",0,0);
    SharedLogVariableDataPtrT pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(var);

    if(!pData)
    {
        bool success;
        HcomHandler::VariableType type;
        QString evalStr = gpMainWindow->mpTerminalWidget->mpHandler->evaluateExpression(str, &type, &success);
        pData = HcomHandler(gpMainWindow->mpTerminalWidget->mpConsole).getVariablePtr(evalStr);
    }

    QString idxStr = str.section(",",1,1);

    SymHop::Expression idxExpr = SymHop::Expression(idxStr);
    QMap<QString, double> localVars = gpMainWindow->mpTerminalWidget->mpHandler->getLocalVariables();
    QMap<QString, SymHop::Function> localFuncs = gpMainWindow->mpTerminalWidget->mpHandler->getLocalFunctionPointers();
    int idx = idxExpr.evaluate(localVars, localFuncs);

    if(pData)
    {
        ok=true;
        QString err;
        return(pData->peekData(idx,err));
    }
    ok=false;
    return 0;
}

double _funcRand(QString str, bool &ok)
{
    Q_UNUSED(str);
    ok=true;
    return rand() / (double)RAND_MAX;          //Random value between  0 and 1
}
