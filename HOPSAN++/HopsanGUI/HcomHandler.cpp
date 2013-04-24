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
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ProjectTabWidget.h"

//HopsanGenerator includes
#include "symhop/SymHop.h"

//Dependency includes
#include "qwt_plot.h"


HcomHandler::HcomHandler(TerminalConsole *pConsole)
{
    mAborted = false;
    mLocalVars.insert("ans", 0);
    mRetvalType = Scalar;

    mpConsole = pConsole;

    mCurrentPlotWindowName = "PlotWindow0";

    mPwd = gDesktopHandler.getDocumentsPath();
    mPwd.chop(1);

    HcomCommand helpCmd;
    helpCmd.cmd = "help";
    helpCmd.description.append("Shows help information.");
    helpCmd.help.append("Usage: help [command]");
    helpCmd.fnc = &HcomHandler::executeHelpCommand;
    mCmdList << helpCmd;

    HcomCommand simCmd;
    simCmd.cmd = "sim";
    simCmd.description.append("Simulates current model.");
    simCmd.help.append("Usage: sim [no arguments]");
    simCmd.fnc = &HcomHandler::executeSimulateCommand;
    mCmdList << simCmd;

    HcomCommand chpvCmd;
    chpvCmd.cmd = "chpv";
    chpvCmd.description.append("Change plot variables in current plot.");
    chpvCmd.help.append("Usage: chpv [leftvar1 [leftvar2] ... [-r rightvar1 rightvar2 ... ]]");
    chpvCmd.fnc = &HcomHandler::executePlotCommand;
    mCmdList << chpvCmd;

    HcomCommand exitCmd;
    exitCmd.cmd = "exit";
    exitCmd.description.append("Exits the program.");
    exitCmd.help.append("Usage: exit [no arguments]");
    exitCmd.fnc = &HcomHandler::executeExitCommand;
    mCmdList << exitCmd;

    HcomCommand dipaCmd;
    dipaCmd.cmd = "dipa";
    dipaCmd.description.append("Display parameter value.");
    dipaCmd.help.append("Usage: dipa [parameter]");
    dipaCmd.fnc = &HcomHandler::executeDisplayParameterCommand;
    mCmdList << dipaCmd;

    HcomCommand chpaCmd;
    chpaCmd.cmd = "chpa";
    chpaCmd.description.append("Change parameter value.");
    chpaCmd.help.append("Usage: chpa [parameter value]");
    chpaCmd.fnc = &HcomHandler::executeChangeParameterCommand;
    mCmdList << chpaCmd;

    HcomCommand chssCmd;
    chssCmd.cmd = "chss";
    chssCmd.description.append("Change simulation settings.");
    chssCmd.help.append("Usage: chss [starttime timestep stoptime [samples]]");
    chssCmd.fnc = &HcomHandler::executeChangeSimulationSettingsCommand;
    mCmdList << chssCmd;

    HcomCommand execCmd;
    execCmd.cmd = "exec";
    execCmd.description.append("Executes a script file");
    execCmd.help.append("Usage: exec [filepath]");
    execCmd.fnc = &HcomHandler::executeRunScriptCommand;
    mCmdList << execCmd;

    HcomCommand wrhiCmd;
    wrhiCmd.cmd = "wrhi";
    wrhiCmd.description.append("Writes history to file.");
    wrhiCmd.help.append("Usage: wrhi [filepath]");
    wrhiCmd.fnc = &HcomHandler::executeWriteHistoryToFileCommand;
    mCmdList << wrhiCmd;

    HcomCommand printCmd;
    printCmd.cmd = "print";
    printCmd.description.append("Prints arguments on the screen.");
    printCmd.help.append("Usage: print [\"Text\" (variable)]\n");
    printCmd.help.append("Note: Not implemented yet.");
    printCmd.fnc = &HcomHandler::executePrintCommand;
    mCmdList << printCmd;

    HcomCommand chpwCmd;
    chpwCmd.cmd = "chpw";
    chpwCmd.description.append("Changes current plot window.");
    chpwCmd.help.append("Usage: chpw [number]");
    chpwCmd.fnc = &HcomHandler::executeChangePlotWindowCommand;
    mCmdList << chpwCmd;

    HcomCommand dipwCmd;
    dipwCmd.cmd = "dipw";
    dipwCmd.description.append("Displays current plot window.");
    dipwCmd.help.append(" Usage: dipw [no arguments]");
    dipwCmd.fnc = &HcomHandler::executeDisplayPlotWindowCommand;
    mCmdList << dipwCmd;

    HcomCommand chpvlCmd;
    chpvlCmd.cmd = "chpvl";
    chpvlCmd.description.append("Changes plot variables on left axis in current plot.");
    chpvlCmd.help.append(" Usage: chpvl [var1 var2 ... ]");
    chpvlCmd.fnc = &HcomHandler::executePlotLeftAxisCommand;
    mCmdList << chpvlCmd;

    HcomCommand chpvrCmd;
    chpvrCmd.cmd = "chpvr";
    chpvrCmd.description.append("Changes plot variables on right axis in current plot.");
    chpvrCmd.help.append(" Usage: chpvr [var1 var2 ... ]");
    chpvrCmd.fnc = &HcomHandler::executePlotRightAxisCommand;
    mCmdList << chpvrCmd;

    HcomCommand dispCmd;
    dispCmd.cmd = "disp";
    dispCmd.description.append("Shows a list of all variables matching specified name filter (using asterisks).");
    dispCmd.help.append("Usage: disp [filter]");
    dispCmd.fnc = &HcomHandler::executeDisplayVariablesCommand;
    mCmdList << dispCmd;

    HcomCommand peekCmd;
    peekCmd.cmd = "peek";
    peekCmd.description.append("Shows the value at a specified index in a specified data variable.");
    peekCmd.help.append("Usage: peek [variable index]");
    peekCmd.fnc = &HcomHandler::executePeekCommand;
    mCmdList << peekCmd;

    HcomCommand pokeCmd;
    pokeCmd.cmd = "poke";
    pokeCmd.description.append("Changes the value at a specified index in a specified data variable.");
    pokeCmd.help.append("Usage: poke [variable index newvalue]");
    pokeCmd.fnc = &HcomHandler::executePokeCommand;
    mCmdList << pokeCmd;

    HcomCommand aliasCmd;
    aliasCmd.cmd = "alias";
    aliasCmd.description.append("Defines an alias for a variable.");
    aliasCmd.help.append("Usage: alias [variable alias]");
    aliasCmd.fnc = &HcomHandler::executeDefineAliasCommand;
    mCmdList << aliasCmd;

    HcomCommand setCmd;
    setCmd.cmd = "set";
    setCmd.description.append("Sets Hopsan preferences.");
    setCmd.help.append(" Usage: set [preference value]\n");
    setCmd.help.append(" Available commands:\n");
    setCmd.help.append("  multicore [on/off]\n");
    setCmd.help.append("  threads [number]");
    setCmd.fnc = &HcomHandler::executeSetCommand;
    mCmdList << setCmd;

    HcomCommand saplCmd;
    saplCmd.cmd = "sapl";
    saplCmd.description.append("Saves plot file to .PLO");
    saplCmd.help.append("Usage: sapl [filepath variables]");
    saplCmd.fnc = &HcomHandler::executeSaveToPloCommand;
    mCmdList << saplCmd;

    HcomCommand loadCmd;
    loadCmd.cmd = "load";
    loadCmd.description.append("Loads a model file.");
    loadCmd.help.append("Usage: load [filepath variables]");
    loadCmd.fnc = &HcomHandler::executeLoadModelCommand;
    mCmdList << loadCmd;

    HcomCommand loadrCmd;
    loadrCmd.cmd = "loadr";
    loadrCmd.description.append("Loads most recent model file.");
    loadrCmd.help.append("Usage: loadr [no arguments]");
    loadrCmd.fnc = &HcomHandler::executeLoadRecentCommand;
    mCmdList << loadrCmd;

    HcomCommand pwdCmd;
    pwdCmd.cmd = "pwd";
    pwdCmd.description.append("Displays present working directory.");
    pwdCmd.help.append("Usage: pwd [no arguments]");
    pwdCmd.fnc = &HcomHandler::executePwdCommand;
    mCmdList << pwdCmd;

    HcomCommand cdCmd;
    cdCmd.cmd = "cd";
    cdCmd.description.append("Changes present working directory.");
    cdCmd.help.append(" Usage: cd [directory]");
    cdCmd.fnc = &HcomHandler::executeChangeDirectoryCommand;
    mCmdList << cdCmd;

    HcomCommand lsCmd;
    lsCmd.cmd = "ls";
    lsCmd.description.append("List files in current directory.");
    lsCmd.help.append("Usage: ls [no arguments]");
    lsCmd.fnc = &HcomHandler::executeListFilesCommand;
    mCmdList << lsCmd;

    HcomCommand closeCmd;
    closeCmd.cmd = "close";
    closeCmd.description.append("Closes current model.");
    closeCmd.help.append("Usage: close [no arguments]");
    closeCmd.fnc = &HcomHandler::executeCloseModelCommand;
    mCmdList << closeCmd;

    HcomCommand chtabCmd;
    chtabCmd.cmd = "chtab";
    chtabCmd.description.append("Changes current model tab.");
    chtabCmd.help.append("Usage: chtab [index]");
    chtabCmd.fnc = &HcomHandler::executeChangeTabCommand;
    mCmdList << chtabCmd;

    HcomCommand adcoCmd;
    adcoCmd.cmd = "adco";
    adcoCmd.description.append("Adds a new component to current model.");
    adcoCmd.help.append(" Usage: adco [typename name -flag value]");
    adcoCmd.fnc = &HcomHandler::executeAddComponentCommand;
    mCmdList << adcoCmd;

    HcomCommand cocoCmd;
    cocoCmd.cmd = "coco";
    cocoCmd.description.append("Connect components in current model.");
    cocoCmd.help.append("Usage: coco [comp1 port1 comp2 port2]");
    cocoCmd.fnc = &HcomHandler::executeConnectCommand;
    mCmdList << cocoCmd;

    HcomCommand crmoCmd;
    crmoCmd.cmd = "crmo";
    crmoCmd.description.append("Creates a new model.");
    crmoCmd.help.append("Usage: crmo [no arguments]");
    crmoCmd.fnc = &HcomHandler::executeCreateModelCommand;
    mCmdList << crmoCmd;

    HcomCommand fmuCmd;
    fmuCmd.cmd = "fmu";
    fmuCmd.description.append("Exports current model to Functional Mockup Unit (FMU)");
    fmuCmd.help.append("Usage: fmu [path]");
    fmuCmd.fnc = &HcomHandler::executeExportToFMUCommand;
    mCmdList << fmuCmd;

    HcomCommand averCmd;
    averCmd.cmd = "aver";
    averCmd.description.append("Calculates average value of a variable");
    averCmd.help.append("Usage: aver [variable]");
    averCmd.fnc = &HcomHandler::executeAverageCommand;
    mCmdList << averCmd;

    HcomCommand minCmd;
    minCmd.cmd = "min";
    minCmd.description.append("Calculates minimum value of a variable");
    minCmd.help.append("Usage: min [variable]");
    minCmd.fnc = &HcomHandler::executeMinCommand;
    mCmdList << minCmd;

    HcomCommand maxCmd;
    maxCmd.cmd = "max";
    maxCmd.description.append("Calculates maximum value of a variable");
    maxCmd.help.append("Usage: max [variable]");
    maxCmd.fnc = &HcomHandler::executeMaxCommand;
    mCmdList << maxCmd;

    HcomCommand chtsCmd;
    chtsCmd.cmd = "chts";
    chtsCmd.description.append("Change time step of sub-component");
    chtsCmd.help.append("Usage: chts [comp timestep]");
    chtsCmd.fnc = &HcomHandler::executeChangeTimestepCommand;
    mCmdList << chtsCmd;

    HcomCommand intsCmd;
    intsCmd.cmd = "ints";
    intsCmd.description.append("Inherit time step of sub-component from system time step");
    intsCmd.help.append("Usage: ints [comp]");
    intsCmd.fnc = &HcomHandler::executeInheritTimestepCommand;
    mCmdList << intsCmd;

    HcomCommand randCmd;
    randCmd.cmd = "rand";
    randCmd.description.append("Calculates a random number between min and max");
    randCmd.help.append("Usage: rand [min max]");
    randCmd.fnc = &HcomHandler::executeRandomCommand;
    mCmdList << randCmd;

    HcomCommand floorCmd;
    floorCmd.cmd = "floor";
    floorCmd.description.append("Rounds value to closest smaller integer value");
    floorCmd.help.append("Usage: floor [value]");
    floorCmd.fnc = &HcomHandler::executeFloorCommand;
    mCmdList << floorCmd;

    HcomCommand ceilCmd;
    ceilCmd.cmd = "ceil";
    ceilCmd.description.append("Rounds value to closest larger integer value");
    ceilCmd.help.append("Usage: ceil [value]");
    ceilCmd.fnc = &HcomHandler::executeCeilCommand;
    mCmdList << ceilCmd;

    HcomCommand roundCmd;
    roundCmd.cmd = "round";
    roundCmd.description.append("Rounds value to closest integer value");
    roundCmd.help.append("Usage: round [value]");
    roundCmd.fnc = &HcomHandler::executeRoundCommand;
    mCmdList << roundCmd;

    HcomCommand sizeCmd;
    sizeCmd.cmd = "size";
    sizeCmd.description.append("Returns the size of specified vector");
    sizeCmd.help.append("Usage: size [vector]");
    sizeCmd.fnc = &HcomHandler::executeSizeCommand;
    mCmdList << sizeCmd;

    HcomCommand bodeCmd;
    bodeCmd.cmd = "bode";
    bodeCmd.description.append("Creates a bode plot from specified curves");
    bodeCmd.help.append("Usage: bode [invar outvar maxfreq]");
    bodeCmd.fnc = &HcomHandler::executeBodeCommand;
    mCmdList << bodeCmd;
}


//! @brief Returns a list of available commands
QStringList HcomHandler::getCommands()
{
    QStringList ret;
    for(int i=0; i<mCmdList.size(); ++i)
    {
        ret.append(mCmdList.at(i).cmd);
    }
    return ret;
}


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

//    ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
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
        mpConsole->printErrorMessage("Unrecognized command: " + majorCmd, "", false);
    }
    else
    {
        mCmdList[idx].runCommand(subCmd, this);
    }
}



void HcomHandler::executeExitCommand(QString /*cmd*/)
{
    gpMainWindow->close();
}


void HcomHandler::executeSimulateCommand(QString /*cmd*/)
{
    ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
    if(pCurrentTab)
    {
        pCurrentTab->simulate_blocking();
    }
}


void HcomHandler::executePlotCommand(QString cmd)
{
    changePlotVariables(cmd, -1);
}

void HcomHandler::executePlotLeftAxisCommand(QString cmd)
{
    changePlotVariables(cmd, 0);
}

void HcomHandler::executePlotRightAxisCommand(QString cmd)
{
    changePlotVariables(cmd, 1);
}

void HcomHandler::executeDisplayParameterCommand(QString cmd)
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
        mpConsole->print(output);
    }
}


void HcomHandler::executeChangeParameterCommand(QString cmd)
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
//    for(int i=0; i<splitCmd.size(); ++i)
//    {
//        splitCmd[i].remove("\"");
//    }

    if(splitCmd.size() == 2)
    {
        ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
        if(!pCurrentTab) { return; }
        SystemContainer *pSystem = pCurrentTab->getTopLevelSystem();
        if(!pSystem) { return; }

        QStringList parameterNames;
        getParameters(splitCmd[0], parameterNames);
        QString newValue = splitCmd[1];

        for(int p=0; p<parameterNames.size(); ++p)
        {
            if(pSystem->getParameterNames().contains(parameterNames[p]))
            {
                pSystem->setParameterValue(parameterNames[p], newValue);
            }
            else
            {
                parameterNames[p].remove("\"");
                QStringList splitFirstCmd = parameterNames[p].split(".");
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
                            components[c]->setParameterValue(parameters[p], evaluateExpression(newValue, &varType, &ok));
                        }
                    }
                }
            }
        }
    }
    else
    {
        mpConsole->printErrorMessage("Wrong number of arguments.","",false);
    }
}


//Change Simulation Settings
//Usage: CHSS [starttime] [timestep] [stoptime] ([samples])
void HcomHandler::executeChangeSimulationSettingsCommand(QString cmd)
{
    cmd.remove("\"");
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() == 3 || splitCmd.size() == 4)
    {
        bool allOk=true;
        bool ok;
        double startT = splitCmd[0].toDouble(&ok);
        if(!ok) { allOk=false; }
        double timeStep = splitCmd[1].toDouble(&ok);
        if(!ok) { allOk=false; }
        double stopT = splitCmd[2].toDouble(&ok);
        if(!ok) { allOk=false; }

        int samples;
        if(splitCmd.size() == 4)
        {
            samples = splitCmd[3].toInt(&ok);
            if(!ok) { allOk=false; }
        }

        if(allOk)
        {
            gpMainWindow->setStartTimeInToolBar(startT);
            gpMainWindow->setTimeStepInToolBar(timeStep);
            gpMainWindow->setStopTimeInToolBar(stopT);
            if(splitCmd.size() == 4)
            {
                ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
                if(!pCurrentTab) { return; }
                SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystem();
                if(!pCurrentSystem) { return; }
                pCurrentSystem->setNumberOfLogSamples(samples);
            }
        }
        else
        {
            mpConsole->printErrorMessage("Failed to apply simulation settings.","",false);
        }
    }
    else
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
    }
}


//! @brief Executes the "help" command, showing help to the user
//! @param cmd Command with first word removed
void HcomHandler::executeHelpCommand(QString cmd)
{
    cmd.remove(" ");
    if(cmd.isEmpty())
    {
        mpConsole->print("-------------------------------------------------------------------------");
        mpConsole->print(" Hopsan HCOM Terminal v0.1\n");
        mpConsole->print(" Available commands:\n");
        QString commands;
        int n=0;
        for(int c=0; c<mCmdList.size(); ++c)
        {
            n=max(mCmdList[c].cmd.size(), n);
        }
        for(int c=0; c<mCmdList.size(); ++c)
        {
            commands.append(" ");
            commands.append(mCmdList[c].cmd);
            for(int i=0; i<n+3-mCmdList[c].cmd.size(); ++i)
            {
                commands.append(" ");
            }
            commands.append(mCmdList[c].description);
            commands.append("\n");
        }
        mpConsole->print(commands);
        mpConsole->print(" Type: \"help [command]\" for more information about a specific command.");
        mpConsole->print("-------------------------------------------------------------------------");
    }
    else
    {
        int idx = -1;
        for(int i=0; i<mCmdList.size(); ++i)
        {
            if(mCmdList[i].cmd == cmd) { idx = i; }
        }

        if(idx < 0)
        {
            mpConsole->printErrorMessage("No help available for this command.","",false);
        }
        else
        {
            int length=max(mCmdList[idx].description.size(), mCmdList[idx].help.size())+2;
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
            mpConsole->print(delimiterLine+"\n"+descLine+"\n"+helpLine+"\n"+delimiterLine);
        }
    }
}


//! @brief Executes the "exec" command, which excutes a script file
//! @param cmd Command with first word removed
void HcomHandler::executeRunScriptCommand(QString cmd)
{
    QStringList splitCmd = cmd.split(" ");

    if(splitCmd.isEmpty()) { return; }

    QString path = splitCmd[0];
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
        mpConsole->printErrorMessage("Unable to read file.","",false);
        return;
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
    QString gotoLabel = runScriptCommands(lines);
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
                gotoLabel = runScriptCommands(commands);
            }
        }
    }

    file.close();
}


//! @brief Executes the "wrhi" command, that writes history to specified file
//! @param cmd Command with fist word removed
void HcomHandler::executeWriteHistoryToFileCommand(QString cmd)
{
    if(cmd.isEmpty()) { return; }

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
        mpConsole->printErrorMessage("Unable to write to file.","",false);
        return;
    }

    QTextStream t(&file);
    for(int h=mpConsole->mHistory.size()-1; h>-1; --h)
    {
        t << mpConsole->mHistory[h] << "\n";
    }
    file.close();
}


//! @brief Executes print command
//! @todo Implement
void HcomHandler::executePrintCommand(QString /*cmd*/)
{
    mpConsole->printErrorMessage("Function not yet implemented.","",false);
}


//! @brief Executes the chpw command, changing current plot window
//! @param cmd Command with first word removed
void HcomHandler::executeChangePlotWindowCommand(QString cmd)
{
    mCurrentPlotWindowName = cmd;
}


//! @brief Executes the dipw command, displaying current plot window
//! @param cmd Command with first word removed
void HcomHandler::executeDisplayPlotWindowCommand(QString /*cmd*/)
{
    mpConsole->print(mCurrentPlotWindowName);
}


//! @brief Executes the "disp" command, displaying all variables matching a filter pattern
//! @param cmd Name filter
void HcomHandler::executeDisplayVariablesCommand(QString cmd)
{
    QStringList output;
    getVariables(cmd, output);

    for(int o=0; o<output.size(); ++o)
    {
        mpConsole->print(output[o]);
    }
}


//! @brief Executes the "peek" command, allowing user to read the value at a position in a data vector
//! @param cmd Command (with the first command word already removed)
void HcomHandler::executePeekCommand(QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 2)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    QString variable = split.first();
    bool ok;
    int id = getNumber(split.last(), &ok);
    if(!ok)
    {
        mpConsole->printErrorMessage("Illegal value.","",false);
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
        }
        else
        {
            mpConsole->printErrorMessage(err,"",false);
        }
    }
    else
    {
        mpConsole->printErrorMessage("Data variable not found","",false);
    }
}


//! @brief Executes the "poke" command, allowing user to write a new value at a position in a data vector
//! @param cmd Command (with the first command word already removed)
void HcomHandler::executePokeCommand(QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 3)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    QString variable = split.first();
    bool ok1, ok2;
    int id = getNumber(split[1], &ok1);
    double value = getNumber(split.last(), &ok2);
    if(!ok1 || !ok2)
    {
        mpConsole->printErrorMessage("Illegal value.","",false);
        return;
    }

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        QString err;
        double r = pData->pokeData(id, value, err);
        if (err.isEmpty())
        {
            mpConsole->print(QString::number(r));
        }
        else
        {
            mpConsole->printErrorMessage(err,"",false);
        }
    }
    else
    {
        mpConsole->printErrorMessage("Data variable not found.","",false);
    }
    return;
}


void HcomHandler::executeDefineAliasCommand(QString cmd)
{
    if(splitWithRespectToQuotations(cmd, ' ').size() != 2)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    QString variable = splitWithRespectToQuotations(cmd, ' ')[0];
    toShortDataNames(variable);
    variable.remove("\"");
    QString alias = splitWithRespectToQuotations(cmd, ' ')[1];

    SharedLogVariableDataPtrT pVariable = getVariablePtr(variable);

    if(!pVariable || !gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->definePlotAlias(alias, pVariable->getFullVariableName()))
    {
        mpConsole->printErrorMessage("Failed to assign variable alias.","",false);
    }

    gpMainWindow->mpPlotWidget->mpPlotVariableTree->updateList();

    return;
}


void HcomHandler::executeSetCommand(QString cmd)
{
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() != 2)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }
    QString pref = splitCmd[0];
    QString value = splitCmd[1];

    if(pref == "multicore")
    {
        if(value != "on" && value != "off")
        {
            mpConsole->printErrorMessage("Unknown value.","",false);
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
            mpConsole->printErrorMessage("Unknown value.","",false);
            return;
        }
        gConfig.setNumberOfThreads(nThreads);
    }
}


void HcomHandler::executeSaveToPloCommand(QString cmd)
{
    if(!cmd.contains(" "))
    {
        mpConsole->printErrorMessage("Too few arguments.", "", false);
        return;
    }
    QString path = cmd.split(" ").first();

    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));


    cmd = cmd.right(cmd.size()-path.size()-1);

    QStringList splitCmdMajor;
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
            splitCmdMajor.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmdMajor.append(cmd.right(cmd.size()-start));
    QStringList allVariables;
    for(int i=0; i<splitCmdMajor.size(); ++i)
    {
        splitCmdMajor[i].remove("\"");
        QStringList variables;
        getVariables(splitCmdMajor[i], variables);
        for(int v=0; v<variables.size(); ++v)
        {
            variables[v].replace(".", "#");
            variables[v].remove("\"");

            if(variables[v].endsWith("#x"))
            {
                variables[v].chop(2);
                variables[v].append("#Position");
            }
            else if(variables[v].endsWith("#v"))
            {
                variables[v].chop(2);
                variables[v].append("#Velocity");
            }
            else if(variables[v].endsWith("#f"))
            {
                variables[v].chop(2);
                variables[v].append("#Force");
            }
            else if(variables[v].endsWith("#p"))
            {
                variables[v].chop(2);
                variables[v].append("#Pressure");
            }
            else if(variables[v].endsWith("#q"))
            {
                variables[v].chop(2);
                variables[v].append("#Flow");
            }
            else if(variables[v].endsWith("#val"))
            {
                variables[v].chop(4);
                variables[v].append("#Value");
            }
            else if(variables[v].endsWith("#Zc"))
            {
                variables[v].chop(3);
                variables[v].append("#CharImpedance");
            }
            else if(variables[v].endsWith("#c"))
            {
                variables[v].chop(2);
                variables[v].append("#WaveVariable");
            }
            else if(variables[v].endsWith("#me"))
            {
                variables[v].chop(3);
                variables[v].append("#EquivalentMass");
            }
            else if(variables[v].endsWith("#Q"))
            {
                variables[v].chop(2);
                variables[v].append("#HeatFlow");
            }
            else if(variables[v].endsWith("#t"))
            {
                variables[v].chop(2);
                variables[v].append("#Temperature");
            }

        }
        allVariables.append(variables);
        //splitCmdMajor[i] = getVariablePtr(splitCmdMajor[i])->getFullVariableName();
    }

    gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->exportToPlo(path, allVariables);
}


void HcomHandler::executeLoadModelCommand(QString cmd)
{
    QString path = cmd;
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    path = dir+path.right(path.size()-path.lastIndexOf("/"));

    gpMainWindow->mpProjectTabs->loadModel(path);
}


void HcomHandler::executeLoadRecentCommand(QString /*cmd*/)
{
    gpMainWindow->mpProjectTabs->loadModel(gConfig.getRecentModels().first());
}

void HcomHandler::executePwdCommand(QString /*cmd*/)
{
    mpConsole->print(mPwd);
}

void HcomHandler::executeChangeDirectoryCommand(QString cmd)
{
    mPwd = QDir().cleanPath(mPwd+"/"+cmd);
    mpConsole->print(mPwd);
}

void HcomHandler::executeListFilesCommand(QString cmd)
{
    if(cmd.isEmpty())
    {
        cmd = "*";
    }
    QStringList contents = QDir(mPwd).entryList(QStringList() << cmd);
    for(int c=0; c<contents.size(); ++c)
    {
        mpConsole->print(contents[c]);
    }
}

void HcomHandler::executeCloseModelCommand(QString /*cmd*/)
{
    if(gpMainWindow->mpProjectTabs->count() > 0)
    {
        gpMainWindow->mpProjectTabs->closeProjectTab(gpMainWindow->mpProjectTabs->currentIndex());
    }
}


void HcomHandler::executeChangeTabCommand(QString cmd)
{
    gpMainWindow->mpProjectTabs->setCurrentIndex(cmd.toInt());
}


void HcomHandler::executeAddComponentCommand(QString cmd)
{
    QStringList args = cmd.split(" ");
    if(args.size() < 5)
    {
        mpConsole->printErrorMessage("Too few arguments.", "", false);
        return;
    }
    QString typeName = args[0];
    QString name = args[1];
    args.removeFirst();
    args.removeFirst();

    double xPos;
    double yPos;
    double rot;

    if(!args.isEmpty())
    {
        if(args.first() == "-a")
        {
            //Absolute
            if(args.size() != 4)
            {
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
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
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
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
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
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
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
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
                mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
                return;
            }
            QString otherName = args[1];
            Component *pOther = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(otherName));
            if(!pOther)
            {
                mpConsole->printErrorMessage("Master component not found.");
                return;
            }
            double offset = args[2].toDouble();
            xPos = pOther->getCenterPos().x();
            yPos = pOther->getCenterPos().y()+offset;
            rot = args[3].toDouble();
        }
    }

    QPointF pos = QPointF(xPos, yPos);
    Component *pObj = qobject_cast<Component*>(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->addModelObject(typeName, pos, rot));
    if(!pObj)
    {
        mpConsole->printErrorMessage("Failed to add new component. Incorrect typename?", "", false);
    }
    else
    {
        mpConsole->print("Added "+typeName+" to current model.");
        gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->renameModelObject(pObj->getName(), name);
    }
}


void HcomHandler::executeConnectCommand(QString cmd)
{
    QStringList args = cmd.split(" ");
    if(args.size() != 4)
    {
        mpConsole->printErrorMessage("Wrong number of arguments", "", false);
        return;
    }

    Port *pPort1 = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(args[0])->getPort(args[1]);
    Port *pPort2 = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(args[2])->getPort(args[3]);

    Connector *pConn = gpMainWindow->mpProjectTabs->getCurrentContainer()->createConnector(pPort1, pPort2);

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


void HcomHandler::executeCreateModelCommand(QString /*cmd*/)
{
    gpMainWindow->mpProjectTabs->addNewProjectTab();
}


void HcomHandler::executeExportToFMUCommand(QString cmd)
{
    if(getNumberOfArguments(cmd) != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.");
    }

    gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->exportToFMU(getArgument(cmd, 0));
}


void HcomHandler::executeAverageCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }
    QString variable = splitWithRespectToQuotations(cmd, ' ')[0];

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        returnScalar(pData->averageOfData());
    }
    else
    {
        mpConsole->printErrorMessage("Data variable not found","",false);
    }
}


void HcomHandler::executeMinCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }
    QString variable = split[0];

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        returnScalar(pData->minOfData());
    }
    else
    {
        mpConsole->printErrorMessage("Data variable not found","",false);
    }
}


void HcomHandler::executeMaxCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }
    QString variable = split[0];

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
        returnScalar(pData->maxOfData());
    }
    else
    {
        mpConsole->printErrorMessage("Data variable not found","",false);
    }
}


void HcomHandler::executeChangeTimestepCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 2)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }
    QString component = split[0];
    QString timeStep = split[1];

    VariableType retType;
    bool isNumber;
    double value = evaluateExpression(split[1], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        mpConsole->printErrorMessage("Second argument is not a number.", "", false);
    }
    else if(!gpMainWindow->mpProjectTabs->getCurrentContainer()->hasModelObject(component))
    {
        mpConsole->printErrorMessage("Component not found.", "", false);
    }
    else
    {
        gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->setDesiredTimeStep(component, value);
        gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->setInheritTimeStep(false);
        mpConsole->print("Setting time step of "+component+" to "+QString::number(value));
    }
}


void HcomHandler::executeInheritTimestepCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }
    QString component = split[0];

    if(!gpMainWindow->mpProjectTabs->getCurrentContainer()->hasModelObject(component))
    {
        mpConsole->printErrorMessage("Component not found.", "", false);
    }
    else
    {
        gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->setInheritTimeStep(component, true);
        mpConsole->print("Setting time step of "+component+" to inherited.");
    }
}

void HcomHandler::executeRandomCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 2)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    VariableType retType;
    bool isNumber;
    double min = evaluateExpression(split[0], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        mpConsole->printErrorMessage("Second argument is not a number.", "", false);
    }
    double max = evaluateExpression(split[1], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        mpConsole->printErrorMessage("Second argument is not a number.", "", false);
    }

    double rd = rand() / (double)RAND_MAX;          //Random value between  0 and 1
    returnScalar(min+rd*(max-min));    //Random value between min and max
}

void HcomHandler::executeFloorCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    VariableType retType;
    bool isNumber;
    double value = evaluateExpression(split[0], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        mpConsole->printErrorMessage("Second argument is not a number.", "", false);
    }

    returnScalar(floor(value));
}

void HcomHandler::executeCeilCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    VariableType retType;
    bool isNumber;
    double value = evaluateExpression(split[0], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        mpConsole->printErrorMessage("Second argument is not a number.", "", false);
    }

    returnScalar(ceil(value));
}

void HcomHandler::executeRoundCommand(QString cmd)
{
    QStringList split = splitWithRespectToQuotations(cmd, ' ');
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    VariableType retType;
    bool isNumber;
    double value = evaluateExpression(split[0], &retType, &isNumber).toDouble();
    if(!isNumber)
    {
        mpConsole->printErrorMessage("Second argument is not a number.", "", false);
    }

    returnScalar(round(value));
}

void HcomHandler::executeSizeCommand(QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() != 1)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    QString variable = split.first();

    SharedLogVariableDataPtrT pData = getVariablePtr(variable);
    if(pData)
    {
         returnScalar(pData->getDataSize());
    }
    else
    {
        mpConsole->printErrorMessage("Data variable not found","",false);
    }
}

void HcomHandler::executeBodeCommand(QString cmd)
{
    QStringList split = cmd.split(" ");
    if(split.size() < 2 || split.size() > 4)
    {
        mpConsole->printErrorMessage("Wrong number of arguments.", "", false);
        return;
    }

    QString var1 = split[0];
    QString var2 = split[1];
    SharedLogVariableDataPtrT pData1 = getVariablePtr(var1);
    SharedLogVariableDataPtrT pData2 = getVariablePtr(var2);
    if(!pData1 || !pData2)
    {
        mpConsole->printErrorMessage("Data variable not found.", "", false);
        return;
    }
    int fMax = 500;
    if(split.size() > 2)
    {
        fMax = split[2].toInt();
    }

    gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Bode plot")->createBodePlot(pData1, pData2, fMax);
}


//! @brief Changes plot variables on specified axes
//! @param cmd Command containing the plot variables
//! @param axis Axis specification (0=left, 1=right, -1=both, separeted by "-r")
void HcomHandler::changePlotVariables(QString cmd, int axis)
{
    QStringList splitCmdMajor;
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
            splitCmdMajor.append(cmd.mid(start, i-start));
            start = i+1;
        }
    }
    splitCmdMajor.append(cmd.right(cmd.size()-start));

    if(axis == -1 || axis == 0)
    {
        removePlotCurves(QwtPlot::yLeft);
    }
    if(axis == -1 || axis == 1)
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
    for(int s=0; s<splitCmdMajor.size(); ++s)
    {
        if(axis == -1 && splitCmdMajor[s] == "-r")
        {
            axisId = QwtPlot::yRight;
        }
        else
        {
            QStringList variables;
            getVariables(splitCmdMajor[s], variables);
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
void HcomHandler::addPlotCurve(QString cmd, int axis)
{
    cmd.remove("\"");
    //QStringList splitCmd = cmd.split(".");

    SystemContainer *pCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentTab()->getTopLevelSystem();
    if(!pCurrentSystem) { return; }

    SharedLogVariableDataPtrT pData = getVariablePtr(cmd);
    if(!pData)
    {
        mpConsole->printErrorMessage("Variable not found.","",false);
        return;
    }

    gpPlotHandler->plotDataToWindow(mCurrentPlotWindowName, pData, axis);
}


//! @brief Removes all curves at specified axis in current plot
//! @param axis Axis to remove from
void HcomHandler::removePlotCurves(int axis)
{
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow(mCurrentPlotWindowName);
    if(pPlotWindow)
    {
        pPlotWindow->getCurrentPlotTab()->removeAllCurvesOnAxis(axis);
    }
}


QString HcomHandler::evaluateExpression(QString expr, VariableType *returnType, bool *evalOk)
{
//    if(expr == "ans")
//    {
//        *returnType = mRetvalType;
//        *evalOk = true;
//        return mLocalVars.find(a);
//    }

    *evalOk = true;
    *returnType = Scalar;

    //expr.replace("==", "§§§§§");
    expr.replace("**", "%%%%%");

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

    //Evaluate expression using SymHop
    SymHop::Expression symHopExpr = SymHop::Expression(expr);

    //Multiplication between data vector and scalar
    LogDataHandler *pLogData=0;
    if(gpMainWindow->mpProjectTabs->count() > 0)
    {
        pLogData = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler();
    }
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
    return QString::number(symHopExpr.evaluate(mLocalVars));
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


QString HcomHandler::runScriptCommands(QStringList &lines)
{
    mAborted = false; //Reset if pushed when script didn't run

    qDebug() << "Number of commands to run: " << lines.size();

    for(int l=0; l<lines.size(); ++l)
    {
        if(lines[l].startsWith("#") || lines[l].startsWith("&")) { continue; }  //Ignore comments and labels

        if(lines[l].startsWith("stop"))
        {
            return "%%%%%EOF";
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
                if(l>lines.size()-1)
                {
                    mpConsole->printErrorMessage("Missing REPEAT in while loop.","",false);
                    return QString();
                }

                if(lines[l].startsWith("while")) { ++nLoops; }
                if(lines[l].startsWith("repeat")) { --nLoops; }

                loop.append(lines[l]);
            }
            loop.removeLast();

            //Evaluate expression using SymHop
            SymHop::Expression symHopExpr = SymHop::Expression(argument);
            while(symHopExpr.evaluate(mLocalVars) > 0)
            {
                if(mAborted)
                {
                    mpConsole->print("Script aborted.");
                    mAborted = false;
                    break;
                }
                QString gotoLabel = runScriptCommands(loop);
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
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
                if(l>lines.size()-1)
                {
                    mpConsole->printErrorMessage("Missing ENDIF in if-statement.","",false);
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
                    mpConsole->printErrorMessage("Evaluation of if-statement argument failed.","",false);
                    return QString();
                }
                QString gotoLabel = runScriptCommands(ifCode);
                if(!gotoLabel.isEmpty())
                {
                    return gotoLabel;
                }
            }
            else
            {
                if(!evalOk)
                {
                    mpConsole->printErrorMessage("Evaluation of if-statement argument failed.","",false);
                    return QString();
                }
                QString gotoLabel = runScriptCommands(elseCode);
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
                QString gotoLabel = runScriptCommands(tempCmds);
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

    ProjectTab *pCurrentTab = gpMainWindow->mpProjectTabs->getCurrentTab();
    if(!pCurrentTab) { return; }
    SystemContainer *pCurrentSystem = pCurrentTab->getTopLevelSystem();
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


void HcomHandler::getParameters(QString str, QStringList &parameters)
{
    if(gpMainWindow->mpProjectTabs->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();

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

    QStringList splitStr = str.split("*");
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



QString HcomHandler::getParameterValue(QString parameter) const
{
    parameter.remove("\"");
    QString compName = parameter.split(".").first();
    QString parName = parameter.split(".").last();

    if(gpMainWindow->mpProjectTabs->count() == 0)
    {
        return "NaN";
    }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    ModelObject *pComp = pSystem->getModelObject(compName);
    if(pComp && pComp->getParameterNames().contains(parName))
    {
        return pComp->getParameterValue(parName);
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
void HcomHandler::getVariables(const QString str, QStringList &variables) const
{
    if(gpMainWindow->mpProjectTabs->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    QStringList names = pSystem->getLogDataHandler()->getPlotDataNames();
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

    QStringList splitStr = str.split("*");
    for(int n=0; n<names.size(); ++n)
    {
        bool ok=true;
        QString name = names[n];
        if(splitStr.size() == 1)   //Special case, no asterixes
        {
            ok = (name == splitStr[0]);
        }
        else        //Other cases, loop substrings and check them
        {
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
        }
        if(ok)
        {
            variables.append(names[n]);
        }
    }
}

//! @brief Help function that returns a list of variables according to input (with support for asterisks)
//! @param str String to look for
//! @param variables Reference to list of found variables
void HcomHandler::getVariablesThatStartsWithString(const QString str, QStringList &variables) const
{
    if(gpMainWindow->mpProjectTabs->count() == 0) { return; }

    SystemContainer *pSystem = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem();
    QStringList names = pSystem->getLogDataHandler()->getPlotDataNames();
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


QString HcomHandler::getWorkingDirectory() const
{
    return mPwd;
}


//! @brief Checks if a command is an arithmetic expression and evaluates it if possible
//! @param cmd Command to evaluate
//! @returns True if it is a correct exrpession, otherwise false
bool HcomHandler::evaluateArithmeticExpression(QString cmd)
{
    //cmd.replace("==", "§§§§§");
    cmd.replace("**", "%%%%%");

/*    if(cmd.count("=") > 1)
    {
        mpConsole->print("Multiple assignments not allowed.");
        return false;
    }
    else */

//    cmd.replace(">=", "!!!gtoe!!!");
//    cmd.replace("<=", "!!!stoe!!!");

//    cmd.replace("!!!gtoe!!!", ">=");
//    cmd.replace("!!!stoe!!!", "<=");
    if(cmd.endsWith("*")) { return false; }

    SymHop::Expression expr = SymHop::Expression(cmd);

    //if(cmd.count("=") == 1)     //Assignment
    if(expr.isAssignment())
    {
        QString left = expr.getLeft()->toString();

        bool leftIsOk = left[0].isLetter();
        for(int i=1; i<left.size(); ++i)
        {
            if(!left.at(i).isLetterOrNumber())
            {
                leftIsOk = false;
            }
        }

        QStringList plotDataNames;
        if(gpMainWindow->mpProjectTabs->count() > 0)
        {
            LogDataHandler *pLogData = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler();
            plotDataNames = pLogData->getPlotDataNames();
        }
        if(!leftIsOk && !plotDataNames.contains(left))
        if(!leftIsOk && (gpMainWindow->mpProjectTabs->count() == 0 || !getVariablePtr(left)))
        {
            mpConsole->printErrorMessage("Illegal variable name.","",false);
            return false;
        }

        VariableType type;
        bool evalOk;
        QString value = evaluateExpression(expr.getRight()->toString(), &type, &evalOk);

        //QString right = cmd.split("=").at(1);
        //right.remove(" ");

        //bool evalOk;
        //VariableType type;
        //QString value=evaluateExpression(right,&type,&evalOk);

        if(evalOk && type==Scalar)
        {
            mLocalVars.insert(left, value.toDouble());
            mpConsole->print("Assigning "+left+" with "+value);
            mLocalVars.insert("ans", value.toDouble());
            return true;
        }
        else if(evalOk && type==DataVector)
        {
            SharedLogVariableDataPtrT pLeftData = getVariablePtr(left);
            SharedLogVariableDataPtrT pValueData = getVariablePtr(value);
            if(pLeftData != 0) { left = pLeftData->getFullVariableName(); }
            if(pValueData != 0) { value = pValueData->getFullVariableName(); }

            gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->assignVariable(left, value);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
//        cmd.replace("!!!gtoe!!!", ">=");
//        cmd.replace("!!!stoe!!!", "<=");

        //! @todo Should we allow pure expessions without assignment?
        bool evalOk;
        VariableType type;
        QString value=evaluateExpression(cmd, &type, &evalOk);
        if(evalOk && type==Scalar)
        {
            returnScalar(value.toDouble());
            return true;
        }
        else if(evalOk && type==DataVector)
        {
            mRetvalType = DataVector;
            mpConsole->print(value);
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
    fullName.replace(".","#");
    int generation = -1;

    if(fullName.count("#") == 1 || fullName.count("#") == 3)
    {
        generation = fullName.split("#").last().toInt();
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

//    SharedLogVariableDataPtrT pRetVal = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotDataByAlias(fullName,generation);
//    if(!pRetVal)
//    {
        SharedLogVariableDataPtrT pRetVal = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler()->getPlotData(fullName,generation);
//    }
    return pRetVal;
}


//! @brief Parses a string into a number
//! @param str String to parse, should be a number of a variable name
//! @param ok Pointer to boolean that tells if parsing was successful
double HcomHandler::getNumber(const QString str, bool *ok) const
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
    *ok = false;
    return 0;
}


void HcomHandler::toShortDataNames(QString &variable) const
{
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
        variable.chop(8);
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


QString HcomHandler::getDirectory(const QString cmd) const
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

QStringList HcomHandler::getArguments(const QString cmd) const
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
    splitCmd.removeFirst();

    return splitCmd;
}


int HcomHandler::getNumberOfArguments(const QString cmd) const
{
    return getArguments(cmd).size();
}

QString HcomHandler::getArgument(const QString cmd, const int idx) const
{
    return getArguments(cmd).at(idx);
}

void HcomHandler::abortHCOM()
{
    mAborted = true;
}

void HcomHandler::returnScalar(const double retval)
{
    mLocalVars.insert("ans", retval);
    mRetvalType = Scalar;
    mpConsole->print(QString::number(retval));
}

