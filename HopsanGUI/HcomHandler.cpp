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
//! @file   HcomHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013
//! @version $Id$
//!
//! @brief Contains a handler for the HCOM scripting language

// Qt includes
#include <QDesktopServices>
#include <QApplication>
#include <QPair>

//HopsanGUI includes
#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/OptimizationDialog.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "PlotCurve.h"
#include "PlotCurveStyle.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotArea.h"
#include "PlotWindow.h"
#include "SimulationThreadHandler.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/HcomWidget.h"
#include "ModelHandler.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/ModelWidget.h"
#include "SymHop.h"
#include "CoreUtilities/SimulationHandler.h"
#include "LogDataGeneration.h"
#ifndef _WIN32
#include <unistd.h>
#endif

//HopsanGenerator includes
#include "SymHop.h"

//Dependency includes
#include "qwt_plot.h"

#include <cmath>


#define HCOMPRINT(text) mpConsole->print(text)
#define HCOMINFO(text) mpConsole->printInfoMessage(text,"",false)
#define HCOMWARN(text) mpConsole->printWarningMessage(text,"",false)
#define HCOMERR(text) mpConsole->printErrorMessage(text,"",false)

namespace generationspecifier {
  constexpr auto separatorString = "@";
  constexpr auto separatorChar = '@';
  constexpr int currentGeneration = -1;
  constexpr int allGenerations = -2;
  constexpr int noGenerationSpecified = -3;
  constexpr int invalidGeneration = -4;
}
constexpr int fromDisplayGeneration(int displayGeneration) {
    return displayGeneration - 1;
}
constexpr int toDisplayGeneration(int dataGeneration) {
    return  dataGeneration + 1;
}
constexpr bool isValidGenerationValue(int g) {
    return g > generationspecifier::noGenerationSpecified;
}
constexpr bool isSingleGenerationValue(int g) {
    return g >= generationspecifier::currentGeneration;
}
constexpr bool isSingleExplicitGenerationValue(int g) {
    return g >= 0;
}

//----------------------------------------------------------------------------------

//! @brief Check if an expression is a single function call
bool isHcomFunctionCall(const QString &fname, const QString &expr)
{
    if (expr.startsWith(fname+"("))
    {
        int nOpen=0;
        // Lets count parenthesis to figure out when function closes
        for (int i=fname.size(); i<expr.size(); ++i)
        {
            if (expr[i]=='(')
            {
                nOpen++;
            }
            else if (expr[i]==')')
            {
                nOpen--;
            }
            // When nOpen == 0 we should have closed the function, and we should be at end of expr, else this is an expression and not a clean function call
            if (nOpen==0)
            {
                return (i==(expr.size()-1));
            }
        }
    }
    return false;
}

//! @brief Make a list from a space separated argument string, respecting (not splitting within) quotation marks
QStringList splitCommandArguments(const QString &rArgs)
{
//    //! @todo maybe use splitWithRespectToQuotations here instead
//    QStringList splitCmd;
//    bool withinQuotations = false;
//    int start=0;
//    for(int i=0; i<rArgs.size(); ++i)
//    {
//        if(rArgs[i] == '\"')
//        {
//            withinQuotations = !withinQuotations;
//        }
//        if(rArgs[i] == ' ' && !withinQuotations)
//        {
//            splitCmd.append(rArgs.mid(start, i-start));
//            start = i+1;
//        }
//    }
//    splitCmd.append(rArgs.right(rArgs.size()-start));
//    //splitCmd.removeFirst();
//    splitCmd.removeAll("");

//    return splitCmd;

    QStringList splitArgs;
    splitRespectingQuotationsAndParanthesis(rArgs, ' ', splitArgs);
    splitArgs.removeAll("");
    return splitArgs;
}


//! @brief Returns the number of arguments in a space separated argument string, respecting (not splitting within) quotation marks
int getNumberOfCommandArguments(const QString &rArgs)
{
    return splitCommandArguments(rArgs).size();
}

//! @brief Extract everything between the first ( and last )
QStringList extractFunctionCallExpressionArguments(const QString& functionCallExpression)
{
    QStringList args;
    int s = functionCallExpression.indexOf('(');
    if (s>=0)
    {
        int e = functionCallExpression.lastIndexOf(')');
        if (e>=0)
        {
            QString argString = functionCallExpression.mid(s+1, e-s-1);
            splitRespectingQuotationsAndParanthesis(argString, ',', args);
        }
    }

    // Remove unnecessary leading and trailing spaces for each arg
    for(int i=0; i<args.size(); ++i)
    {
        args[i] = args[i].trimmed();
    }

    return args;
}

//! @brief Get value of flag:value argument, from a previously split argument string
//! @details Example: Retrieve filepath from arguments: -loadstates c:\myFile, It is assumed that the value comes after the flag
//! @param [in] arguments A QStringList representing the arguments
//! @param [in] flag The flag to search for (including - prefix)
//! @returns The value or empty if nothing found
QString getFlagArgValue(const QStringList &arguments, const QString &flag)
{
    QString flagvalue;
    int idx = arguments.indexOf(flag);
    if (idx >= 0)
    {
        ++idx;
        if ( idx < arguments.size() )
        {
            flagvalue = arguments[idx];
        }
    }
    return flagvalue;
}

inline bool isString(const QString &expr)
{
    return (expr.startsWith('"') && expr.endsWith('"'));
}

SystemObject* searchIntoSubsystem(SystemObject* pRootSystem, const QStringList& systemNames) {
    auto pSystem = pRootSystem;
    for (const QString &sysname : systemNames) {
        pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(sysname));
        if (!pSystem) {
            break;
        }
    }
    return pSystem;
}

//! @brief HelpFunction that will look for paramName#Value if paramName is not found, paramName will be updated if found
//! @param[in] pModelObject The model object that should contain the parameter
//! @param[in,out] rParameterName The parameter name
//! @returns true if found
bool findParameterEvenIfValueNotSpecified(ModelObject *pModelObject, QString &rParamName) {
    QString tmpParamName = rParamName;
    if (pModelObject) {
        if (pModelObject->hasParameter(tmpParamName)) {
            return true;
        } else {
            tmpParamName = rParamName+"#Value";
        }
        if (pModelObject->hasParameter(tmpParamName)) {
            rParamName = tmpParamName;
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------------

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
                // We can break loop since maps should be ordered by key
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
                // We can break loop since maps should be ordered by key
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
    // connect the optimization message handler to the console for this HCOM handler
    connect(mpOptHandler->getMessageHandler(), SIGNAL(newAnyMessage(GUIMessage)), mpConsole, SLOT(printMessage(GUIMessage)));

    mPwd = gpDesktopHandler->getDocumentsPath();
    mPwd.chop(1);

    //Register internal function descriptions
    registerInternalFunction("lp1", "Applies low-pass filter of first degree to vector","Usage: lp1(vector, frequency)\nUsage: lp1(vector, timevector, frequency)");
    registerInternalFunction("ddt", "Differentiates vector with respect to time (or to custom vector)","Usage: ddt(vector)\nUsage: ddt(vector, timevector)");
    registerInternalFunction("int", "Integrates vector with respect to time (or to custom vector)", "Usage: int(vector)\nUsage: int(vector, timevector)");
    registerInternalFunction("fft", "Generates frequency spectrum plot from vector (deprecated)","Usage: fft(vector, [type]([power]/energy/rms), [windowing]([rectangular]/flattop/hann), [min], [max])\nUsage: fft(vector, [timevector], [type]([power]/energy/rms), [windowing]([rectangular]/flattop/hann), [min], [max])");
    registerInternalFunction("esd", "Generates energy spectral density from vector","Usage: esd(vector, [timevector], [windowing]([rectangular]/flattop/hann), [mintime], [maxtime])\n");
    registerInternalFunction("psd", "Generates power spectral density from vector","Usage: psd(vector, [timevector], [windowing]([rectangular]/flattop/hann), [mintime], [maxtime])\n");
    registerInternalFunction("rmsd", "Generates root mean square spectral density from vector","Usage: rmsd(vector, [timevector], [windowing]([rectangular]/flattop/hann), [mintime], [maxtime])\n");
    registerInternalFunction("rms", "Computes the root mean square of given vector","Usage: rms(vector)");
    registerInternalFunction("gt", "Index-wise greater than check between vectors and/or scalars (equivalent to \">\" operator)","Usage: gt(varName, threshold)\nUsage: gt(var1, var2)");
    registerInternalFunction("lt", "Index-wise less than check between vectors and/or scalars  (equivalent to \"<\" operator)","Usage: lt(varName, threshold)\nUsage: lt(var1,var2)");
    registerInternalFunction("eq", "Index-wise fuzzy equal check between vectors and/or scalars  (equivalent to \"==\" operator)","Usage: eq(varName, threshold, eps)\nUsage: eq(var1, var2, eps)");
    registerInternalFunction("linspace", "Linearly spaced vector","Usage: linspace(min, max, numSamples)");
    registerInternalFunction("logspace", "Logarithmicly spaced vector","Usage: logspace(min, max, numSamples)");
    registerInternalFunction("ones", "Create a vector of ones","Usage: ones(size)");
    registerInternalFunction("zeros", "Create a vector of zeros","Usage: zeros(size)");
    registerInternalFunction("vector", "Create a vector with specified values","Usage: vector(1,2,3,4,5,6,...)");
    registerInternalFunction("maxof", "Returns the element-wise maximum values of x and y vectors","Usage: maxof(x,y)");
    registerInternalFunction("minof", "Returns the element-wise minimum values of x and y vectors","Usage: minof(x,y)");
    registerInternalFunction("abs", "The absolute value of each vector element", "Usage: abs(vector)");
    registerInternalFunction("round", "Rounds the value of each vector element to closest integer value", "Usage: round(vector)");
    registerInternalFunction("ceil", "Rounds the value of each vector element to the smallest integer larger than the value", "Usage: ceil(vector)");
    registerInternalFunction("floor", "Rounds the value of each vector element to the largest integer smaller than the value", "Usage: floor(vector)");
    registerInternalFunction("x", "Returns the X-vector of the specified variable.","Usage: x(vector)");
    registerInternalFunction("td", "Converts variable to time domain.", "Usage: y = td(x)");
    registerInternalFunction("fd", "Converts variable to frequency domain.", "Usage: y = fd(x)");
    registerInternalFunction("expr", "Defines a new symbolic expression", "Usage: e = expr(...)");
    registerInternalFunction("eval", "Evaluates a symbolic expression using local variables", "Usage: y = expr(f)");
    registerInternalFunction("der", "Differentiates a symbolic expression with respect to another one", "Usage: e = der(f1,f2)");
    registerInternalFunction("factor", "Factors a symbolic expression with respect to another one", "Usage: e = factor(f1,f2)");
    registerInternalFunction("simplify", "Simplifies a symbolic expression", "Usage: e = simplify(f)");
    registerInternalFunction("linearize", "Linearizes a SymHop equation by multiplying with all divisors", "Usage: e = linearize(f)");
    registerInternalFunction("expand", "Expands all parentheses in a symbolic expression", "Usage: e = expand(f)");
    registerInternalFunction("expandPowers", "Expands all powers in a symbolic expression to multiplications", "Usage: e = expandPowers(f)");
    registerInternalFunction("removeDivisors", "Removes all divisors in a symbolic expression", "Usage: e = removeDivisors(f)");
    registerInternalFunction("left", "Returns the left-hand side of a SymHop equation", "Usage: e = left(f)");
    registerInternalFunction("right", "Returns the right-hand side of a SymHop equation", "Usage: e = right(f)");
    registerInternalFunction("trapezoid", "Transforms derivatives in a symbolic expression using the trapezoid method", "Usage: e = trapezoid(f)");
    registerInternalFunction("euler", "Transforms derivatives in a symbolic expression using the forward Euler method", "Usage: e = euler(f)");
    registerInternalFunction("bdf1", "Transforms derivatives in a symbolic expression using the first order backward differentiation formula (implicit Euler)", "Usage: e = bdf1(f)");
    registerInternalFunction("bdf2", "Transforms derivatives in a symbolic expression using the second order backward differentiation formula", "Usage: e = bdf2(f)");
    registerInternalFunction("latex", "Prints a symbolic expression with LaTeX syntax", "Usage: latex(e)");
    registerInternalFunction("cut", "Removes all samples x[i] for which y[i]<0.5", "Usage: cut(x, y)");
    registerInternalFunction("ssi", "Identifies steady-state for specified variable", "Usage: ssi(vector, method, arguments)\n       Method 0 (rectangular window):\n         ssi(vector, 0, tolerance, windowlength)\n       Method 1 (ratio of differently estimated variances):\n         ssi(vector, 1, tolerance, windowlength, noiseamplitude\n       Method 2 (ratio of differently estimated variances using weighted moving average):\n         ssi(vector, 2, tolerance, lambda1, lambda2, lambda3, noiseamplitude)");

    //Setup local function pointers (used to evaluate expressions in SymHop)
    registerFunctionoid("aver", new HcomFunctionoidAver(this), "Calculate average value of vector", "Usage: aver(vector)");
    registerFunctionoid("min", new HcomFunctionoidMin(this), "Calculate minimum value of vector", "Usage: min(vector)");
    registerFunctionoid("max", new HcomFunctionoidMax(this), "Calculate maximum value of vector","Usage:max(vector)");
    registerFunctionoid("imin", new HcomFunctionoidIMin(this), "Calculate index of minimum value of vector","Usage: imin(vector)");
    registerFunctionoid("imax", new HcomFunctionoidIMax(this), "Calculate index of maximum value of vector","Usage: imax(vector)");
    registerFunctionoid("size", new HcomFunctionoidSize(this), "Calculate the size of a vector","Usage: size(vector)");
    registerFunctionoid("rand", new HcomFunctionoidRand(this), "Generates a random value between 0 and 1", "Usage: rand()");
    registerFunctionoid("peek", new HcomFunctionoidPeek(this), "Returns vector value at specified index", "Usage: peek(vector, idx)");
    registerFunctionoid("obj", new HcomFunctionoidObj(this), "Returns optimization objective function value with specified index","Usage: obj(idx)");
    registerFunctionoid("time", new HcomFunctionoidTime(this), "Returns last simulation time", "Usage: time()");
    registerFunctionoid("optvar", new HcomFunctionoidOptVar(this), "Returns specified optimization variable", "Usage: optvar(idx)");
    registerFunctionoid("optpar", new HcomFunctionoidOptPar(this), "Returns specified optimization parameter", "Usage: optpar(idx)");
    registerFunctionoid("fc", new HcomFunctionoidFC(this), "Fuzzy compare, returns 1 if the values of the arguments are almost the same", "Usage: fc(expr, expr, tolerance)");
    registerFunctionoid("exists", new HcomFunctionoidExists(this), "Returns whether or not specified component exists in model", "Usage: exists(name)");
    registerFunctionoid("maxpar", new HcomFunctionoidMaxPar(this), "Returns the maximum value of specified parameter for specified component type", "Usage: maxpar(type,par)");
    registerFunctionoid("minpar", new HcomFunctionoidMinPar(this), "Returns the minimum value of specified parameter for specified component type", "Usage: minpar(type,par)");
    registerFunctionoid("hg", new HcomFunctionoidHg(this), "Returns highest generation number", "Usage: hg()");
    registerFunctionoid("ans", new HcomFunctionoidAns(this), "Returns the answer from the previous computation", "Usage: ans()");
    registerFunctionoid("count", new HcomFunctionoidCount(this), "Counts number of components in model with specified type name.", "Usage: count(typename)");
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

    updatePwd();
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
    simCmd.help.append(" Usage: sim\n");
    simCmd.help.append(" Usage: sim all\n");
    simCmd.help.append(" Usage: sim -loadstate file\n");
    simCmd.help.append(" Usage: sim -loadsv file\n");
    simCmd.help.append("  -loadsv will load start values from a saved simulation state file\n");
    simCmd.help.append("  -loadstate will do the same but also offset the simulation time");
    simCmd.fnc = &HcomHandler::executeSimulateCommand;
    simCmd.group = "Simulation Commands";
    mCmdList << simCmd;

    HcomCommand chpvCmd;
    chpvCmd.cmd = "chpv";
    chpvCmd.description.append("Change plot variables in current plot");
    chpvCmd.help.append(" Usage: chpv [leftvar1{s,c,t} leftvar2{s,c,t} ...] -r [rightvar1{s,c,t} rightvar2{s,c,t} ... ]\n");
    chpvCmd.help.append(" Usage: chpv [leftvar1{s,c,t} leftvar2{s,c,t} ...]\n");
    chpvCmd.help.append(" Usage: chpv -r [rightvar1{s,c,t} rightvar2{s,c,t} ... ]\n\n");
    chpvCmd.help.append(" Line appearance (optional):\n");
    chpvCmd.help.append("  Style (s): 1 = solid, 2 = dotted, 3 = dashed, 4 = dash-dotted\n");
    chpvCmd.help.append("  Color (c): red, darkblue, lightgreen...\n");
    chpvCmd.help.append("  Thickness (t): Thickness of line (integer value)\n");
    chpvCmd.help.append("  Symbol (y): 1 = cross, 2 = ellipse, 3=xcross, 4=triangle...\n\n");
    chpvCmd.help.append(" Example:\n");
    chpvCmd.help.append("  >> chpv Pump.P1.p -r Pump.P1.q{2,blue,2}");
    chpvCmd.fnc = &HcomHandler::executePlotCommand;
    chpvCmd.group = "Plot Commands";
    mCmdList << chpvCmd;

    HcomCommand adpvCmd;
    adpvCmd.cmd = "adpv";
    adpvCmd.description.append("Add plot variables in current plot");
    adpvCmd.help.append(" Usage: adpv [leftvar1{s,c,t} leftvar2{s,c,t} ...] -r [rightvar1{s,c,t} rightvar2{s,c,t} ... ]\n");
    adpvCmd.help.append(" Usage: adpv [leftvar1{s,c,t} leftvar2{s,c,t} ...]\n");
    adpvCmd.help.append(" Usage: adpv -r [rightvar1{s,c,t} rightvar2{s,c,t} ... ]\n\n");
    adpvCmd.help.append(" Line appearance (optional):\n");
    adpvCmd.help.append("  Style (s): 1 = solid, 2 = dotted, 3 = dashed, 4 = dash-dotted\n");
    adpvCmd.help.append("  Color (c): red, darkblue, lightgreen...\n");
    adpvCmd.help.append("  Thickness (t): Thickness of line (integer value)\n");
    adpvCmd.help.append("  Symbol (y): 1 = cross, 2 = ellipse, 3=xcross, 4=triangle...\n\n");
    adpvCmd.help.append(" Example:\n");
    adpvCmd.help.append("  >> adpv Pump.P1.p -r Pump.P1.q{2,blue,2}");
    adpvCmd.fnc = &HcomHandler::executeAddPlotCommand;
    adpvCmd.group = "Plot Commands";
    mCmdList << adpvCmd;

    HcomCommand adpvlCmd;
    adpvlCmd.cmd = "adpvl";
    adpvlCmd.description.append("Adds plot variables on left axis in current plot");
    adpvlCmd.help.append(" Usage: adpvl [var1{s,c,t} var2{s,c,t} ... ]\n\n");
    adpvlCmd.help.append(" Line appearance (optional):\n");
    adpvlCmd.help.append("  Style (s): 1 = solid, 2 = dotted, 3 = dashed, 4 = dash-dotted\n");
    adpvlCmd.help.append("  Color (c): red, darkblue, lightgreen...\n");
    adpvlCmd.help.append("  Thickness (t): Thickness of line (integer value)\n");
    adpvlCmd.help.append("  Symbol (y): 1 = cross, 2 = ellipse, 3=xcross, 4=triangle...\n\n");
    adpvlCmd.help.append(" Example:\n");
    adpvlCmd.help.append("  >> adpvl Pump.P1.p Pump.P1.q{2,blue,2}");
    adpvlCmd.fnc = &HcomHandler::executeAddPlotLeftAxisCommand;
    adpvlCmd.group = "Plot Commands";
    mCmdList << adpvlCmd;

    HcomCommand adpvrCmd;
    adpvrCmd.cmd = "adpvr";
    adpvrCmd.description.append("Adds plot variables on right axis in current plot");
    adpvrCmd.help.append(" Usage: adpvr [var1{s,c,t} var2{s,c,t} ... ]\n\n");
    adpvrCmd.help.append(" Line appearance (optional):\n");
    adpvrCmd.help.append("  Style (s): 1 = solid, 2 = dotted, 3 = dashed, 4 = dash-dotted\n");
    adpvrCmd.help.append("  Color (c): red, darkblue, lightgreen...\n");
    adpvrCmd.help.append("  Thickness (t): Thickness of line (integer value)\n");
    adpvrCmd.help.append("  Symbol (y): 1 = cross, 2 = ellipse, 3=xcross, 4=triangle...\n\n");
    adpvrCmd.help.append(" Example:\n");
    adpvrCmd.help.append("  >> adpvl Pump.P1.p Pump.P1.q{2,blue,2}");
    adpvrCmd.fnc = &HcomHandler::executeAddPlotRightAxisCommand;
    adpvrCmd.group = "Plot Commands";
    mCmdList << adpvrCmd;

    HcomCommand chdsCmd;
    chdsCmd.cmd = "chds";
    chdsCmd.description.append("Change diagram size (and position)");
    chdsCmd.help.append(" Usage: chds [width] [height]\n");
    chdsCmd.help.append(" Usage: chds [x] [y] [width] [height]\n");
    chdsCmd.fnc = &HcomHandler::executeChangeDiagramSizeCommand;
    chdsCmd.group = "Plot Commands";
    mCmdList << chdsCmd;

    HcomCommand chdlCmd;
    chdlCmd.cmd = "chdl";
    chdlCmd.description.append("Change (and lock) diagram limits in the current plot");
    chdlCmd.help.append(" Usage: chdl [xLow] [xHigh]\n");
    chdlCmd.help.append(" Usage: chdl [xLow] [xHigh] [xTicks] \n");
    chdlCmd.help.append(" Usage: chdl [xLow] [xHigh] [xTicks] [ylLow] [ylHigh] [ylTicks] \n");
    chdlCmd.help.append(" Usage: chdl [xLow] [xHigh] [xTicks] [ylLow] [ylHigh] [ylTicks] [yrLow] [yrHigh] [yrTicks] \n");
    chdlCmd.help.append(" Usage: chdl reset");
    chdlCmd.fnc = &HcomHandler::executeChangeDiagramLimitsCommand;
    chdlCmd.group = "Plot Commands";
    mCmdList << chdlCmd;

    HcomCommand chdlylCmd;
    chdlylCmd.cmd = "chdlyl";
    chdlylCmd.description.append("Change (and lock) diagram left y-axis limits in the current plot");
    chdlylCmd.help.append(" Usage: chdlyl [ylLow] [ylHigh]\n");
    chdlylCmd.help.append(" Usage: chdlyl [ylLow] [ylHigh] [ylTicks]\n");
    chdlylCmd.help.append(" Usage: chdlyl reset");
    chdlylCmd.fnc = &HcomHandler::executeChangeDiagramLimitsYLCommand;
    chdlylCmd.group = "Plot Commands";
    mCmdList << chdlylCmd;

    HcomCommand chdlyrCmd;
    chdlyrCmd.cmd = "chdlyr";
    chdlyrCmd.description.append("Change (and lock) diagram right y-axis limits in the current plot");
    chdlyrCmd.help.append(" Usage: chdlyr [yrLow] [yrHigh]\n");
    chdlyrCmd.help.append(" Usage: chdlyr [yrLow] [yrHigh] [yrTicks]\n");
    chdlyrCmd.help.append(" Usage: chdlyr reset");
    chdlyrCmd.fnc = &HcomHandler::executeChangeDiagramLimitsYRCommand;
    chdlyrCmd.group = "Plot Commands";
    mCmdList << chdlyrCmd;

    HcomCommand logx;
    logx.cmd = "logx";
    logx.description.append("Turn on and off logarithmic x-axis scale");
    logx.help.append(" Usage: logx on \n");
    logx.help.append(" Usage: logx off");
    logx.fnc = &HcomHandler::executeChangeLogarithmicAxisX;
    logx.group = "Plot Commands";
    mCmdList << logx;

    HcomCommand logyl;
    logyl.cmd = "logyl";
    logyl.description.append("Turn on and off logarithmic left y-axis scale");
    logyl.help.append(" Usage: logyl on \n");
    logyl.help.append(" Usage: logyl off");
    logyl.fnc = &HcomHandler::executeChangeLogarithmicAxisYL;
    logyl.group = "Plot Commands";
    mCmdList << logyl;

    HcomCommand logyr;
    logyr.cmd = "logyr";
    logyr.description.append("Turn on and off logarithmic right y-axis scale");
    logyr.help.append(" Usage: logyr on \n");
    logyr.help.append(" Usage: logyr off");
    logyr.fnc = &HcomHandler::executeChangeLogarithmicAxisYR;
    logyr.group = "Plot Commands";
    mCmdList << logyr;

    HcomCommand sapw;
    sapw.cmd = "sapw";
    sapw.description.append("Save the current plotwinow as an image");
    sapw.help.append(" Usage: sapw [fileName.ext] [width] [height] [dimUnit] [dpi] \n");
    sapw.help.append(" Usage: sapw [fileName.ext] [width] [height] [dimUnit]       \n");
    sapw.help.append(" Usage: sapw [fileName.ext] [width] [height]                 Pixels size\n");
    sapw.help.append(" Usage: sapw [fileName.ext]                                  Use screen size\n");
    sapw.help.append("  ext:     png, pdf, svg, ps, jpeg\n");
    sapw.help.append("  dimUnit: px, mm, cm, in\n");
    sapw.help.append("  'dpi' (actually pixels/inch) is ignored for vector formats or when dimunit is 'px' (pixels)");
    sapw.fnc = &HcomHandler::executeSavePlotWindowCommand;
    sapw.group = "Plot Commands";
    mCmdList << sapw;

    HcomCommand infoCmd;
    infoCmd.cmd = "info";
    infoCmd.description.append("Show information about specified variable");
    infoCmd.help.append("Usage: info [variable]");
    infoCmd.fnc = &HcomHandler::executeVariableInfoCommand;
    mCmdList << infoCmd;


    HcomCommand exitCmd;
    exitCmd.cmd = "exit";
    exitCmd.description.append("Exits the program");
    exitCmd.help.append(" Usage: exit [no arguments]");
    exitCmd.fnc = &HcomHandler::executeExitCommand;
    mCmdList << exitCmd;

    HcomCommand dipaCmd;
    dipaCmd.cmd = "dipa";
    dipaCmd.description.append("Display parameter value");
    dipaCmd.help.append(" Usage: dipa\n");
    dipaCmd.help.append(" Usage: dipa [parameter]\n");
    dipaCmd.help.append(" Usage: dipa [parameter] [condition]\n\n");
    dipaCmd.help.append(" Example: Display all parameters starting");
    dipaCmd.help.append(" with \"Mass\" and a value greater than 10:\n");
    dipaCmd.help.append(" >> dipa Mass* >10");
    dipaCmd.fnc = &HcomHandler::executeDisplayParameterCommand;
    dipaCmd.group = "Parameter Commands";
    mCmdList << dipaCmd;

    HcomCommand dicoCmd;
    dicoCmd.cmd = "dico";
    dicoCmd.description.append("Display components");
    dicoCmd.help.append(" Usage: dico\n");
    dicoCmd.help.append(" Usage: dico [component]\n");
    dicoCmd.help.append(" Example: Display all components starting with \"x\":");
    dicoCmd.help.append(" >> dico x*");
    dicoCmd.fnc = &HcomHandler::executeDisplayComponentsCommand;
    dicoCmd.group = "Model Commands";
    mCmdList << dicoCmd;

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
    chpaCmd.help.append(" Usage: chpa [parameter] [value]");
    chpaCmd.fnc = &HcomHandler::executeChangeParameterCommand;
    chpaCmd.group = "Parameter Commands";
    mCmdList << chpaCmd;

    HcomCommand chssCmd;
    chssCmd.cmd = "chss";
    chssCmd.description.append("Change simulation settings");
    chssCmd.help.append(" Usage: chss [starttime] [timestep] [stoptime] [samples]\n");
    chssCmd.help.append(" Usage: chss [starttime] [timestep] [stoptime]");
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

    HcomCommand wrtfCmd;
    wrtfCmd.cmd = "wrtf";
    wrtfCmd.description.append("Writes text string to file");
    wrtfCmd.help.append(" Usage: wrtf [-flag] [filepath] [\"string\"]\n");
    wrtfCmd.help.append("  Flags (optional):\n");
    wrtfCmd.help.append("   -a Append at end of file\n");
    wrtfCmd.help.append("   -e Erase existing contents before writing\n");
    wrtfCmd.help.append("  Variables can be written by putting them in dollar signs.\n");
    wrtfCmd.help.append("  Example:\n");
    wrtfCmd.help.append("   >> wrtf -a output.txt \"x=$x$\"\n");
    wrtfCmd.fnc = &HcomHandler::executeWriteToFileCommand;
    wrtfCmd.group = "File Commands";
    mCmdList << wrtfCmd;

    HcomCommand printCmd;
    printCmd.cmd = "print";
    printCmd.description.append("Prints arguments on the screen");
    printCmd.help.append(" Usage: print [-flag] [\"string\"]\n");
    printCmd.help.append(" Usage: print [-flag] [expression]\n");
    printCmd.help.append("  Flags (optional):\n");
    printCmd.help.append("   -i Info message\n");
    printCmd.help.append("   -w Warning message\n");
    printCmd.help.append("   -e Error message\n");
    printCmd.help.append("  Variables can be included in strings by putting them in dollar signs.\n");
    printCmd.help.append("  Example:\n");
    printCmd.help.append("   >> print -w \"x=$x$\"\n");
    printCmd.help.append("   Warning: x=12");
    printCmd.fnc = &HcomHandler::executePrintCommand;
    mCmdList << printCmd;

    HcomCommand evalCmd;
    evalCmd.cmd = "eval";
    evalCmd.description.append("Evaluate string expression");
    evalCmd.help.append(" Usage: eval [\"expression\"]\n");
    evalCmd.help.append("  Variable evaluation can be included by putting them within dollar signs\n");
    evalCmd.help.append("  Example:\n");
    evalCmd.help.append("   >> eval \"chpv Gain$i$.out.y\"");
    evalCmd.fnc = &HcomHandler::executeEvalCommand;
    mCmdList << evalCmd;


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
    chpvlCmd.help.append(" Usage: chpvl [var1{s,c,t} var2{s,c,t} ... ]\n\n");
    chpvlCmd.help.append(" Line appearance (optional):\n");
    chpvlCmd.help.append("  Style (s): 1 = solid, 2 = dotted, 3 = dashed, 4 = dash-dotted\n");
    chpvlCmd.help.append("  Color (c): red, darkblue, lightgreen...\n");
    chpvlCmd.help.append("  Thickness (t): Thickness of line (integer value)\n");
    chpvlCmd.help.append("  Symbol (y): 1 = cross, 2 = ellipse, 3=xcross, 4=triangle...\n\n");
    chpvlCmd.help.append(" Example:\n");
    chpvlCmd.help.append("  >> chpvl Pump.P1.p Pump.P1.q{2,blue,2}");
    chpvlCmd.fnc = &HcomHandler::executePlotLeftAxisCommand;
    chpvlCmd.group = "Plot Commands";
    mCmdList << chpvlCmd;

    HcomCommand chpvrCmd;
    chpvrCmd.cmd = "chpvr";
    chpvrCmd.description.append("Changes plot variables on right axis in current plot");
    chpvrCmd.help.append(" Usage: chpvr [var1{s,c,t} var2{s,c,t,y} ... ]");
    chpvrCmd.help.append(" Line appearance (optional):\n");
    chpvrCmd.help.append("  Style (s): 1 = solid, 2 = dotted, 3 = dashed, 4 = dash-dotted\n");
    chpvrCmd.help.append("  Color (c): red, darkblue, lightgreen...\n");
    chpvrCmd.help.append("  Thickness (t): Thickness of line (integer value)\n");
    chpvrCmd.help.append("  Symbol (y): 1 = cross, 2 = ellipse, 3=xcross, 4=triangle...\n\n");
    chpvrCmd.help.append(" Example:\n");
    chpvrCmd.help.append("  >> chpvr Pump.P1.p Pump.P1.q{2,blue,2}");
    chpvrCmd.fnc = &HcomHandler::executePlotRightAxisCommand;
    chpvrCmd.group = "Plot Commands";
    mCmdList << chpvrCmd;

    HcomCommand chpvxCmd;
    chpvxCmd.cmd = "chpvx";
    chpvxCmd.description.append("Changes xdata plot variable in current plot");
    chpvxCmd.help.append(" Usage: chpvx [varname]\n");
    chpvxCmd.help.append(" Usage: chpvx -c      Clear Custom x-data");
    chpvxCmd.fnc = &HcomHandler::executePlotXAxisCommand;
    chpvxCmd.group = "Plot Commands";
    mCmdList << chpvxCmd;

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
    peekCmd.help.append(" Usage: peek [variable] [index]");
    peekCmd.group = "Variable Commands";
    peekCmd.fnc = &HcomHandler::executePeekCommand;

    mCmdList << peekCmd;

    HcomCommand pokeCmd;
    pokeCmd.cmd = "poke";
    pokeCmd.description.append("Changes the value at a specified index in a specified data variable");
    pokeCmd.help.append(" Usage: poke [variable] [index] [newvalue]");
    pokeCmd.fnc = &HcomHandler::executePokeCommand;
    pokeCmd.group = "Variable Commands";
    mCmdList << pokeCmd;

    HcomCommand aliasCmd;
    aliasCmd.cmd = "alias";
    aliasCmd.description.append("Defines an alias for a variable");
    aliasCmd.help.append(" Usage: alias [variable] [alias]");
    aliasCmd.fnc = &HcomHandler::executeDefineAliasCommand;
    aliasCmd.group = "Variable Commands";
    mCmdList << aliasCmd;

    HcomCommand rmvarCmd;
    rmvarCmd.cmd = "rmvar";
    rmvarCmd.description.append("Removes specified variable");
    rmvarCmd.help.append(" Usage: rmvar [variables]\n");
    rmvarCmd.help.append(" Usage: rmvar [variables] -n [variables]\n");
    rmvarCmd.help.append("  Flags (optional):\n");
    rmvarCmd.help.append("   -n Exclude specified variables");
    rmvarCmd.help.append("   -noalias Exclude all alias variables");
    rmvarCmd.fnc = &HcomHandler::executeRemoveVariableCommand;
    rmvarCmd.group = "Variable Commands";
    mCmdList << rmvarCmd;

    HcomCommand dihgCmd;
    dihgCmd.cmd = "dihg";
    dihgCmd.description.append("Display highest generation");
    dihgCmd.help.append(" Usage: dihg\n");
    dihgCmd.fnc = &HcomHandler::executeDisplayHighestGenerationCommand;
    dihgCmd.group = "Variable Commands";
    mCmdList << dihgCmd;
//    HcomCommand chdfscCmd;
//    chdfscCmd.cmd = "chdfsc";
//    chdfscCmd.description.append("Change default plot scale of specified variable");
//    chdfscCmd.help.append(" Usage: chdfsc [variable] [scale]");
//    chdfscCmd.fnc = &HcomHandler::executeChangeDefaultPlotScaleCommand;
//    chdfscCmd.group = "Variable Commands";
//    mCmdList << chdfscCmd;

//    HcomCommand didfscCmd;
//    didfscCmd.cmd = "didfsc";
//    didfscCmd.description.append("Display default plot scale of specified variable");
//    didfscCmd.help.append(" Usage: didfsc [variable]");
//    didfscCmd.fnc = &HcomHandler::executeDisplayDefaultPlotScaleCommand;
//    didfscCmd.group = "Variable Commands";
//    mCmdList << didfscCmd;

//    HcomCommand chdfosCmd;
//    chdfosCmd.cmd = "chdfos";
//    chdfosCmd.description.append("Change default plot offset of specified variable");
//    chdfosCmd.help.append(" Usage: chdfsc [variable] [offset]");
//    chdfosCmd.fnc = &HcomHandler::executeChangeDefaultPlotOffsetCommand;
//    chdfosCmd.group = "Variable Commands";
//    mCmdList << chdfosCmd;

//    HcomCommand didfosCmd;
//    didfosCmd.cmd = "didfos";
//    didfosCmd.description.append("Display default plot offset of specified variable");
//    didfosCmd.help.append(" Usage: didfsc [variable]");
//    didfosCmd.fnc = &HcomHandler::executeDisplayDefaultPlotOffsetCommand;
//    didfosCmd.group = "Variable Commands";
//    mCmdList << didfosCmd;

    HcomCommand chtoCmd;
    chtoCmd.cmd = "chto";
    chtoCmd.description.append("Change time plot offset for the current or specified generation");
    chtoCmd.help.append(" Usage: chto [offset] [generation]\n");
    chtoCmd.help.append(" Time offset should be given in the unit selected as the default time unit\n");
    chtoCmd.help.append(" The generation specifier is optional, you can use c,a,*,h,l specifiers");
    chtoCmd.fnc = &HcomHandler::executeChangeTimePlotOffsetCommand;
    chtoCmd.group = "Plot Commands";
    mCmdList << chtoCmd;

    HcomCommand sequCmd;
    sequCmd.cmd = "sequ";
    sequCmd.description.append("Set quantity on specified variable(s)");
    sequCmd.help.append(" Usage: sequ [variable] [quantityname]\n");
    sequCmd.help.append(" Usage: sequ [variable] -\n");
    sequCmd.help.append("  The second example clears the quantity");
    sequCmd.fnc = &HcomHandler::executeSetQuantityCommand;
    sequCmd.group = "Variable Commands";
    mCmdList << sequCmd;

    HcomCommand ivpvCmd;
    ivpvCmd.cmd = "ivpv";
    ivpvCmd.description.append("Toggle invert plot of specified variable");
    ivpvCmd.help.append(" Usage: ivpv [variable]");
    ivpvCmd.fnc = &HcomHandler::executeInvertPlotVariableCommand;
    ivpvCmd.group = "Variable Commands";
    mCmdList << ivpvCmd;

    HcomCommand seplCmd;
    seplCmd.cmd = "sepl";
    seplCmd.description.append("Set plot label");
    seplCmd.help.append(" Usage: sepl [variable] [label]");
    seplCmd.fnc = &HcomHandler::executeSetlabelCommand;
    seplCmd.group = "Variable Commands";
    mCmdList << seplCmd;

//    HcomCommand chscCmd;
//    chscCmd.cmd = "chsc";
//    chscCmd.description.append("Change plot scale of specified variable");
//    chscCmd.help.append(" Usage: chsc [variable] [scale]");
//    chscCmd.fnc = &HcomHandler::executeChangePlotScaleCommand;
//    chscCmd.group = "Variable Commands";
//    mCmdList << chscCmd;

//    HcomCommand discCmd;
//    discCmd.cmd = "disc";
//    discCmd.description.append("Display plot scale of specified variable");
//    discCmd.help.append(" Usage: disc [variable]");
//    discCmd.fnc = &HcomHandler::executeDisplayPlotScaleCommand;
//    discCmd.group = "Variable Commands";
//    mCmdList << discCmd;

    HcomCommand ditoCmd;
    ditoCmd.cmd = "dito";
    ditoCmd.description.append("Display time plot offset of specified generation");
    ditoCmd.help.append(" Usage: dito [generation]\n");
    ditoCmd.help.append(" Time offset will be shown in the chose default time unit\n");
    ditoCmd.help.append(" The \"ans\" variable will be offset in seconds (the base unit)");
    ditoCmd.fnc = &HcomHandler::executeDisplayTimePlotOffsetCommand;
    ditoCmd.group = "Plot Commands";
    mCmdList << ditoCmd;

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
    setCmd.help.append(" Usage: set [preference] [value]\n");
    setCmd.help.append("  Available preferences:\n");
    setCmd.help.append("   multicore        [on/off]\n");
    setCmd.help.append("   threads          [number]\n");
    setCmd.help.append("   cachetodisk      [on/off]\n");
    setCmd.help.append("   generationlimit  [number]\n");
    setCmd.help.append("   samples          [number]\n");
    setCmd.help.append("   undo             [on/off]\n");
    setCmd.help.append("   backup           [on/off]\n");
    setCmd.help.append("   progressbar      [on/off]\n");
    setCmd.help.append("   progressbarstep  [number]");
    setCmd.fnc = &HcomHandler::executeSetCommand;
    mCmdList << setCmd;

    HcomCommand getCmd;
    getCmd.cmd = "get";
    getCmd.description.append("Shows current Hopsan preferences");
    getCmd.help.append(" Usage: get [preference]\n");
    getCmd.help.append("  No argument prints all preferences.\n");
    getCmd.help.append("  Available preferences:\n");
    getCmd.help.append("   multicore      \n");
    getCmd.help.append("   threads        \n");
    getCmd.help.append("   cachetodisk    \n");
    getCmd.help.append("   generationlimit\n");
    getCmd.help.append("   samples        \n");
    getCmd.help.append("   undo           \n");
    getCmd.help.append("   backup         \n");
    getCmd.help.append("   progressbar    \n");
    getCmd.help.append("   progressbarstep\n");
    getCmd.fnc = &HcomHandler::executeGetCommand;
    mCmdList << getCmd;

    HcomCommand saplCmd;
    saplCmd.cmd = "sapl";
    saplCmd.description.append("Save log variables to file. Filename suffix determins format");
    saplCmd.help.append(" Usage: sapl [filepath] [-flags] [variables]\n");
    saplCmd.help.append("  Flags (optional):\n");
    saplCmd.help.append("   -csv    Force CSV format\n");
    saplCmd.help.append("   -plo    Force PLO format\n");
    saplCmd.help.append("   -h5     Force H5 (HDF5) format");
    saplCmd.fnc = &HcomHandler::executeSaveToPloCommand;
    saplCmd.group = "Plot Commands";
    mCmdList << saplCmd;

    HcomCommand replCmd;
    replCmd.cmd = "repl";
    replCmd.description.append("Loads plot files from .csv or .plo");
    replCmd.help.append(" Usage: repl [-flags] [filepath]\n");
    replCmd.help.append("  Flags (optional):\n");
    replCmd.help.append("   -csv    Force CSV (, or ;) format\n");
    replCmd.help.append("   -ssp    Force CSV (space separated) format\n");
    replCmd.help.append("   -plo    Force PLO format");
    replCmd.fnc = &HcomHandler::executeLoadVariableCommand;
    replCmd.group = "Plot Commands";
    mCmdList << replCmd;

    HcomCommand sapaCmd;
    sapaCmd.cmd = "sapa";
    sapaCmd.description.append("Save model or component parameter values to XML file (.hpf)");
    sapaCmd.help.append(" Usage: sapa [filepath]\n");
    sapaCmd.help.append(" Usage: sapa [filepath] [componentname]\n");
    sapaCmd.help.append(" Usage: sapa [filepath] [-c]\n"
                        "  Flag -c for current visible system");
    sapaCmd.fnc = &HcomHandler::executeSaveParametersCommand;
    sapaCmd.group = "Parameter Commands";
    mCmdList << sapaCmd;

    HcomCommand repaCmd;
    repaCmd.cmd = "repa";
    repaCmd.description.append("Load model or component parameters values from XML file (.hpf)");
    repaCmd.help.append(" Usage: repa [filepath]\n");
    repaCmd.help.append(" Usage: repa [filepath] [componentname]\n");
    repaCmd.help.append(" Usage: repa [filepath] [-c] \n"
                        "  Flag -c for current visible system");
    repaCmd.fnc = &HcomHandler::executeLoadParametersCommand;
    repaCmd.group = "Parameter Commands";
    mCmdList << repaCmd;

    HcomCommand loadCmd;
    loadCmd.cmd = "load";
    loadCmd.description.append("Loads a model file");
    loadCmd.help.append(" Usage: load [filepath]");
    loadCmd.fnc = &HcomHandler::executeLoadModelCommand;
    loadCmd.group = "Model Commands";
    mCmdList << loadCmd;

    HcomCommand revertCmd;
    revertCmd.cmd = "revert";
    revertCmd.description.append("Revert the current model");
    revertCmd.help.append(" Usage: revert");
    revertCmd.fnc = &HcomHandler::executeRevertModelCommand;
    revertCmd.group = "Model Commands";
    mCmdList << revertCmd;

    HcomCommand loadrCmd;
    loadrCmd.cmd = "loadr";
    loadrCmd.description.append("Loads most recent model file");
    loadrCmd.help.append(" Usage: loadr [no arguments]");
    loadrCmd.fnc = &HcomHandler::executeLoadRecentCommand;
    loadrCmd.group = "Model Commands";
    mCmdList << loadrCmd;

    HcomCommand saveCmd;
    saveCmd.cmd = "save";
    saveCmd.description.append("Saves current model");
    saveCmd.help.append(" Usage: save\n");
    saveCmd.help.append(" Usage: save [filepath]\n");
    saveCmd.help.append("  If no file path is specified, current model file is overwritten.");
    saveCmd.fnc = &HcomHandler::executeSaveModelCommand;
    saveCmd.group = "Model Commands";
    mCmdList << saveCmd;

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
    cdCmd.help.append(" Path may be relative or absolute and must be contained withing quotes \" \" if it contains spaces\n");
    cdCmd.help.append(" Usage: cd [directory]\n");
    cdCmd.help.append(" Usage: cd -mwd        Switch to current model working directory");
    cdCmd.fnc = &HcomHandler::executeChangeDirectoryCommand;
    cdCmd.group = "File Commands";
    mCmdList << cdCmd;

    HcomCommand mkdirCmd;
    mkdirCmd.cmd = "mkdir";
    mkdirCmd.description.append("Creates directory and all parent directories");
    mkdirCmd.help.append(" Path may be relative or absolute and must be contained withing quotes \" \" if it contains spaces\n");
    mkdirCmd.help.append(" Usage: mkdir [path]");
    mkdirCmd.fnc = &HcomHandler::executeMakeDirectoryCommand;
    mkdirCmd.group = "File Commands";
    mCmdList << mkdirCmd;

    HcomCommand lsCmd;
    lsCmd.cmd = "ls";
    lsCmd.description.append("List files in current directory");
    lsCmd.help.append(" Usage: ls");
    lsCmd.help.append(" Usage: ls [wildcard]");
    lsCmd.fnc = &HcomHandler::executeListFilesCommand;
    lsCmd.group = "File Commands";
    mCmdList << lsCmd;

    HcomCommand closeCmd;
    closeCmd.cmd = "close";
    closeCmd.description.append("Closes current model");
    closeCmd.help.append(" Usage: close");
    closeCmd.help.append(" Usage: close all");
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
    adcoCmd.help.append(" Usage: adco [typename] [name] -a [x-coord] [y-coord] [rot-angel]\n");
    adcoCmd.help.append(" Usage: adco [typename] [name] -e [other] [east-offset] [rot-angel]\n");
    adcoCmd.help.append(" Usage: adco [typename] [name] -w [other] [west-offset] [rot-angel]\n");
    adcoCmd.help.append(" Usage: adco [typename] [name] -n [other] [north-offset] [rot-angel]\n");
    adcoCmd.help.append(" Usage: adco [typename] [name] -s [other] [south-offset] [rot-angel]");
    adcoCmd.fnc = &HcomHandler::executeAddComponentCommand;
    adcoCmd.group = "Model Commands";
    mCmdList << adcoCmd;

    HcomCommand rpcoCmd;
    rpcoCmd.cmd = "rpco";
    rpcoCmd.description.append("Replaces a component with a different type");
    rpcoCmd.help.append(" Usage: rpco [name] [typename]");
    rpcoCmd.fnc = &HcomHandler::executeReplaceComponentCommand;
    rpcoCmd.group = "Model Commands";
    mCmdList << rpcoCmd;

    HcomCommand cocoCmd;
    cocoCmd.cmd = "coco";
    cocoCmd.description.append("Connect components in current model");
    cocoCmd.help.append(" Usage: coco [comp1] [port1] [comp2] [port2]");
    cocoCmd.fnc = &HcomHandler::executeConnectCommand;
    cocoCmd.group = "Model Commands";
    mCmdList << cocoCmd;

    HcomCommand uncoCmd;
    uncoCmd.cmd = "unco";
    uncoCmd.description.append("List unconnected ports in current model");
    uncoCmd.help.append(" Usage: unco [wildcard]");
    uncoCmd.fnc = &HcomHandler::executeListUnconnectedCommand;
    uncoCmd.group = "Model Commands";
    mCmdList << uncoCmd;

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
    chtsCmd.help.append(" Usage: chts [component] [timestep]");
    chtsCmd.fnc = &HcomHandler::executeChangeTimestepCommand;
    chtsCmd.group = "Simulation Commands";
    mCmdList << chtsCmd;

    HcomCommand intsCmd;
    intsCmd.cmd = "ints";
    intsCmd.description.append("Inherit time step of sub-component from system time step");
    intsCmd.help.append(" Usage: ints [component]");
    intsCmd.fnc = &HcomHandler::executeInheritTimestepCommand;
    intsCmd.group = "Simulation Commands";
    mCmdList << intsCmd;

    HcomCommand bodeCmd;
    bodeCmd.cmd = "bode";
    bodeCmd.description.append("Creates a bode plot from specified curves");
    bodeCmd.help.append(" Usage: bode [invar] [outvar] [maxfreq] [windowfunction] [mintime] [maxtime]");
    bodeCmd.fnc = &HcomHandler::executeBodeCommand;
    bodeCmd.group = "Plot Commands";
    mCmdList << bodeCmd;

    HcomCommand nyquistCmd;
    nyquistCmd.cmd = "nyquist";
    nyquistCmd.description.append("Creates a Nyquist plot from specified curves");
    nyquistCmd.help.append(" Usage: nyquist [invar] [outvar]");
    nyquistCmd.fnc = &HcomHandler::executeNyquistCommand;
    nyquistCmd.group = "Plot Commands";
    mCmdList << nyquistCmd;

    HcomCommand optCmd;
    optCmd.cmd = "opt";
    optCmd.description.append("Initialize an optimization");
    optCmd.help.append(" Usage: opt [algorithm] [partype] [parnum] [parmin] [parmax] -flags]\n");
    optCmd.help.append("  Algorithms:   Flags:\n");
    optCmd.help.append("  complex       alpha");
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

    HcomCommand clearCmd;
    clearCmd.cmd = "clear";
    clearCmd.description.append("Clears terminal output");
    clearCmd.help.append(" Usage: echo");
    clearCmd.fnc = &HcomHandler::executeClearCommand;
    mCmdList << clearCmd;

    HcomCommand editCmd;
    editCmd.cmd = "edit";
    editCmd.description.append("Open file in external editor");
    editCmd.help.append(" Usage: edit [filepath]");
    editCmd.fnc = &HcomHandler::executeEditCommand;
    editCmd.group = "File Commands";
    mCmdList << editCmd;

    HcomCommand semtCmd;
    semtCmd.cmd = "semt";
    semtCmd.description.append("Change mutli-trheading settings");
    semtCmd.help.append(" Usage: semt [on/off] [numThreads] [algorithm]\n");
    semtCmd.help.append(" Usage: semt [on/off] [numThreads]");
    semtCmd.fnc = &HcomHandler::executeSetMultiThreadingCommand;
    mCmdList << semtCmd;

    HcomCommand lockCmd;
    lockCmd.cmd = "lock";
    lockCmd.description.append("Locks or unlocks all axes in current plot window");
    lockCmd.help.append(" Usage: lock [flag] [on/off]\n");
    lockCmd.help.append(" See also: lockyl, lockyr, lockr");
    lockCmd.fnc = &HcomHandler::executeLockAllAxesCommand;
    lockCmd.group = "Plot Commands";
    mCmdList << lockCmd;

    HcomCommand lockylCmd;
    lockylCmd.cmd = "lockyl";
    lockylCmd.description.append("Locks or unlocks left y-axis in current plot window");
    lockylCmd.help.append(" Usage: lockyl [on/off]\n");
    lockylCmd.help.append(" See also: lock, lockyr, lockr");
    lockylCmd.fnc = &HcomHandler::executeLockLeftAxisCommand;
    lockylCmd.group = "Plot Commands";
    mCmdList << lockylCmd;

    HcomCommand lockyrCmd;
    lockyrCmd.cmd = "lockyr";
    lockyrCmd.description.append("Locks or unlocks right y-axis in current plot window");
    lockyrCmd.help.append(" Usage: lockyr [on/off]\n");
    lockyrCmd.help.append(" See also: lock, lockyl, lockr");
    lockyrCmd.fnc = &HcomHandler::executeLockRightAxisCommand;
    lockyrCmd.group = "Plot Commands";
    mCmdList << lockyrCmd;

    HcomCommand lockxCmd;
    lockxCmd.cmd = "lockx";
    lockxCmd.description.append("Locks or unlocks x-axis in current plot window");
    lockxCmd.help.append(" Usage: lockx [on/off]\n");
    lockxCmd.help.append(" See also: lock, lockyl, lockyr");
    lockxCmd.fnc = &HcomHandler::executeLockXAxisCommand;
    lockxCmd.group = "Plot Commands";
    mCmdList << lockxCmd;

    HcomCommand sleepCmd;
    sleepCmd.cmd = "sleep";
    sleepCmd.description.append("Pause execution for a number of seconds");
    sleepCmd.help.append(" Usage: sleep [seconds]\n");
    sleepCmd.help.append(" The sleep argument has millisecond accuracy");
    sleepCmd.fnc = &HcomHandler::executeSleepCommand;
    mCmdList << sleepCmd;
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
                QString desc = mCmdList[c].description;
                QString help = mCmdList[c].help;
                output.append(desc.replace(">>", "\\>\\>")).append("<br>\n");
                output.append(help.replace(">>", "\\>\\>").replace("\n","<br>\n"));
                output.append("\n\n");
            }
        }
    }

    output.append("\\section functions Local Functions\n\n");
    QMapIterator<QString,QPair<QString, QString> > fit(mLocalFunctionDescriptions);
    while(fit.hasNext())
    {
        fit.next();
        output.append("\\subsection "+fit.key().toLower()+" "+fit.key()+"()\n");
        output.append(fit.value().first+"<br>\n");
        QString desc = fit.value().second;
        output.append(desc.replace("\n","<br>\n"));
        output.append("\n\n");
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
    //Ignore everything after first comment symbol
    cmd = cmd.section("#",0,0);
    if(cmd.isEmpty()) return;

    //Allow several commands on one line, separated by semicolon
    if(containsOutsideQuotes(cmd, ';'))
    {
        QStringList cmdList;
        splitWithRespectToQuotations(cmd, ';', cmdList);
        for(const QString &tempCmd : qAsConst(cmdList)) {
            executeCommand(tempCmd);
        }
        return;
    }

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
        if(mCmdList[i].cmd == majorCmd)
        {
            idx = i;
            break;
        }
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
void HcomHandler::executeExitCommand(const QString cmd)
{
    Q_UNUSED(cmd);
    // Using a single shot timer seems to prevent freeze/crash if a script ending in exit is executed immediately when Hopsan starts (called from command line)
    QTimer::singleShot(0, gpMainWindowWidget, SLOT(close()));
    //gpMainWindowWidget->close();
}


//! @brief Execute function for "sim" command
void HcomHandler::executeSimulateCommand(const QString cmd)
{
    QStringList arguments = splitCommandArguments(cmd);
    if (arguments.contains("-loadstate") || arguments.contains("-loadsv"))
    {
        if (arguments.size() < 2)
        {
            HCOMERR("Wrong number of arguments.");
            return;
        }

        if (mpModel && mpModel->getTopLevelSystemContainer())
        {
            bool doOffsetTime;
            QString statefile = getFlagArgValue(arguments, "-loadstate");
            if (statefile.isEmpty())
            {
                doOffsetTime=false;
                statefile = getFlagArgValue(arguments, "-loadsv");
                HCOMPRINT(QString("Loading start values from file: %1").arg(statefile));
            }
            else
            {
                doOffsetTime=true;
                HCOMPRINT(QString("Loading states (start values and time) from file: %1").arg(statefile));
            }

            double timeOffset;
            QString prevStartT = mpModel->getStartTime();
            QString prevStopT = mpModel->getStopTime();
            mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->loadSimulationState(statefile,timeOffset);
            mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->setKeepValuesAsStartValues(true);
            if (doOffsetTime)
            {
                HCOMPRINT(QString("Offsetting simulation time: %1 seconds").arg(timeOffset));
                mpModel->setTopLevelSimulationTime(QString("%1").arg(prevStartT.toDouble()+timeOffset),
                                                   mpModel->getTimeStep(),
                                                   QString("%1").arg(prevStopT.toDouble()+timeOffset));
            }

            mpModel->simulate_blocking();
            // Restore settings
            mpModel->setTopLevelSimulationTime(prevStartT,
                                               mpModel->getTimeStep(),
                                               prevStopT);
            mpModel->getTopLevelSystemContainer()->getCoreSystemAccessPtr()->setKeepValuesAsStartValues(false);

        }
    }
    else
    {
        if(arguments.size() > 1)
        {
            HCOMERR("Wrong number of arguments.");
            return;
        }
        else if(arguments.size() == 1 && arguments[0] == "all")
        {
            gpModelHandler->simulateAllOpenModels_blocking(false);
        }
        else if(arguments.isEmpty())
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
        }
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

//! @brief Execute function for "chpvx" command
void HcomHandler::executePlotXAxisCommand(const QString cmd)
{
    changePlotXVariable(cmd);
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


//! @brief Execute function for "chds" command
void HcomHandler::executeChangeDiagramSizeCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if (args.size() != 2 && args.size() != 4) {
        HCOMERR("Wrong number of arguments in chds, should be 2 or 4");
        return;
    }

    if (mpCurrentPlotWindow) {
        bool ok;
        if(args.size() == 4) {
            evaluateExpression(args[0]);
            if(mAnsType != Scalar) {
                HCOMERR("Argument is not a scalar: "+args[0]);
                return;
            }
            if(mAnsScalar < 0) {
                HCOMERR("X position must be equal to or greater than zero");
                return;
            }
            double x = mAnsScalar;

            evaluateExpression(args[1]);
            if(mAnsType != Scalar) {
                HCOMERR("Argument is not a scalar: "+args[1]);
                return;
            }
            if(mAnsScalar < 0) {
                HCOMERR("Y position must be equal to or greater than zero");
                return;
            }
            double y = mAnsScalar;

            evaluateExpression(args[2]);
            if(mAnsType != Scalar) {
                HCOMERR("Argument is not a scalar: "+args[2]);
                return;
            }
            if(mAnsScalar <= 0) {
                HCOMERR("Width must be greater than zero");
                return;
            }
            double w = mAnsScalar;

            evaluateExpression(args[3]);
            if(mAnsType != Scalar) {
                HCOMERR("Argument is not a scalar: "+args[3]);
                return;
            }
            if(mAnsScalar <= 0) {
                HCOMERR("Height must be greater than zero");
                return;
            }
            double h = mAnsScalar;

            mpCurrentPlotWindow->move(x,y);
            mpCurrentPlotWindow->resize(w,h);
        }
        else {
            evaluateExpression(args[0]);
            if(mAnsType != Scalar) {
                HCOMERR("Argument is not a scalar: "+args[0]);
                return;
            }
            if(mAnsScalar <= 0) {
                HCOMERR("Width must be greater than zero");
                return;
            }
            double w = mAnsScalar;

            evaluateExpression(args[1]);
            if(mAnsType != Scalar) {
                HCOMERR("Argument is not a scalar: "+args[1]);
                return;
            }
            if(mAnsScalar <= 0) {
                HCOMERR("Height must be greater than zero");
                return;
            }
            double h = mAnsScalar;

            mpCurrentPlotWindow->resize(w,h);
        }
    }
    else {
        HCOMERR("No plot window is open");
    }
}

//! @brief Execute function for "chdl" command
void HcomHandler::executeChangeDiagramLimitsCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if (args.size() == 1 && args.front() != "reset")
    {
        HCOMERR("Single argument must be 'reset' in chdl command");
        return;
    }
    else if (args.size() != 1 && args.size() != 2 && args.size() != 3 && args.size() != 6 && args.size() != 9)
    {
        HCOMERR("Wrong number of arguments in chdl, should be 1, 2, 3, 6 or 9");
        return;
    }
    else
    {
        //All OK
    }

    PlotArea *pArea=0;
    if (mpCurrentPlotWindow)
    {
        PlotTab *pTab = mpCurrentPlotWindow->getCurrentPlotTab();
        if (pTab)
        {
            pArea = pTab->getPlotArea();
        }
    }
    if (!pArea)
    {
        HCOMERR("Unable to find current plot window plot area");
        return;
    }

    if (args.size() == 1)
    {
        pArea->setAxisLocked(QwtPlot::xBottom, false);
        pArea->setAxisLocked(QwtPlot::yLeft, false);
        pArea->setAxisLocked(QwtPlot::yRight, false);
        pArea->rescaleAxesToCurves();
    }
    else
    {
        bool minOK,maxOK,ticksOK=true;
        // Set x-values
        if (args.size() >= 2)
        {
            double min = args[0].toDouble(&minOK);
            double max = args[1].toDouble(&maxOK);
            double ticks = 0;
            if (args.size() >= 3)
            {
                ticks = args[2].toDouble(&ticksOK);
            }
            if (minOK && maxOK && ticksOK)
            {
                pArea->setAxisLimits(QwtPlot::xBottom, min, max, ticks);
            }
            else
            {
                HCOMERR("Failed to parse double argument for x-axis in chdl");
                return;
            }
        }

        // Set yl-values
        if (args.size() >= 6)
        {
            double min = args[3].toDouble(&minOK);
            double max = args[4].toDouble(&maxOK);
            double ticks = args[5].toDouble(&ticksOK);
            if (minOK && maxOK && ticksOK)
            {
                pArea->setAxisLimits(QwtPlot::yLeft, min, max, ticks);
            }
            else
            {
                HCOMERR("Failed to parse double argument for the left y-axis in chdl");
                return;
            }
        }

        // Set yr-values
        if (args.size() == 9)
        {
            double min = args[6].toDouble(&minOK);
            double max = args[7].toDouble(&maxOK);
            double ticks = args[8].toDouble(&ticksOK);
            if (minOK && maxOK && ticksOK)
            {
                pArea->setAxisLimits(QwtPlot::yRight, min, max, ticks);
            }
            else
            {
                HCOMERR("Failed to parse double argument for the right y-axis in chdl");
                return;
            }
        }
    }
}

void HcomHandler::executeChangeDiagramLimitsYLCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if (args.size() == 1 && args.front() != "reset")
    {
        HCOMERR("Single argument must be 'reset' in chdlyl command");
        return;
    }
    else if (args.size() > 3)
    {
        HCOMERR("Wrong number of arguments in chdlyl, should be 1, 2 or 3");
        return;
    }
    else
    {
        //All OK
    }


    PlotArea *pArea=0;
    if (mpCurrentPlotWindow)
    {
        PlotTab *pTab = mpCurrentPlotWindow->getCurrentPlotTab();
        if (pTab)
        {
            pArea = pTab->getPlotArea();
        }
    }
    if (!pArea)
    {
        HCOMERR("Unable to find current plot window plot area");
        return;
    }

    if (args.size() == 1)
    {
        pArea->setAxisLocked(QwtPlot::yLeft, false);
        pArea->rescaleAxesToCurves();
    }
    else
    {
        // Set yl-values
        bool minOK,maxOK,ticksOK=true;
        double min = args[0].toDouble(&minOK);
        double max = args[1].toDouble(&maxOK);
        if (args.size() == 3)
        {
            double ticks = args[2].toDouble(&ticksOK);
        }
        if (minOK && maxOK && ticksOK)
        {
            pArea->setAxisLimits(QwtPlot::yLeft, min, max);
        }
        else
        {
            HCOMERR("Failed to parse double argument for yl-axis in chdlyl");
            return;
        }
    }
}

void HcomHandler::executeChangeDiagramLimitsYRCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if (args.size() == 1 && args.front() != "reset")
    {
        HCOMERR("Single argument must be 'reset' in chdlyr command");
        return;
    }
    else if (args.size() > 3)
    {
        HCOMERR("Wrong number of arguments in chdlyr, should be 1, 2 or 3");
        return;
    }
    else
    {
        //All OK
    }

    PlotArea *pArea=0;
    if (mpCurrentPlotWindow)
    {
        PlotTab *pTab = mpCurrentPlotWindow->getCurrentPlotTab();
        if (pTab)
        {
            pArea = pTab->getPlotArea();
        }
    }
    if (!pArea)
    {
        HCOMERR("Unable to find current plot window plot area");
        return;
    }

    if (args.size() == 1)
    {
        pArea->setAxisLocked(QwtPlot::yRight, false);
        pArea->rescaleAxesToCurves();
    }
    else
    {
        // Set yr-values
        bool minOK,maxOK,ticksOK=true;
        double min = args[0].toDouble(&minOK);
        double max = args[1].toDouble(&maxOK);
        if (args.size() == 3)
        {
            double ticks = args[2].toDouble(&ticksOK);
        }
        if (minOK && maxOK && ticksOK)
        {
            pArea->setAxisLimits(QwtPlot::yRight, min, max);
        }
        else
        {
            HCOMERR("Failed to parse double argument for yr-axis in chdlyr");
            return;
        }
    }
}

void HcomHandler::executeChangeLogarithmicAxisX(const QString cmd)
{
    bool isLog=false;
    if (cmd.trimmed() == "on")
    {
        isLog = true;
    }
    else if (cmd.trimmed() == "off")
    {
        isLog = false;
    }
    else
    {
        HCOMERR(QString("Wrong argument %1, must be 'on' or 'off'").arg(cmd));
        return;
    }


    if (mpCurrentPlotWindow)
    {
        PlotTab *pTab = mpCurrentPlotWindow->getCurrentPlotTab();
        if (pTab)
        {
            PlotArea *pArea = pTab->getPlotArea();
            if (pArea)
            {
                pArea->setBottomAxisLogarithmic(isLog);
            }
        }
    }
}

void HcomHandler::executeChangeLogarithmicAxisYL(const QString cmd)
{
    bool isLog=false;
    if (cmd.trimmed() == "on")
    {
        isLog = true;
    }
    else if (cmd.trimmed() == "off")
    {
        isLog = false;
    }
    else
    {
        HCOMERR(QString("Wrong argument %1, must be 'on' or 'off'").arg(cmd));
        return;
    }


    if (mpCurrentPlotWindow)
    {
        PlotTab *pTab = mpCurrentPlotWindow->getCurrentPlotTab();
        if (pTab)
        {
            PlotArea *pArea = pTab->getPlotArea();
            if (pArea)
            {
                pArea->setLeftAxisLogarithmic(isLog);
            }
        }
    }
}

void HcomHandler::executeChangeLogarithmicAxisYR(const QString cmd)
{
    bool isLog=false;
    if (cmd.trimmed() == "on")
    {
        isLog = true;
    }
    else if (cmd.trimmed() == "off")
    {
        isLog = false;
    }
    else
    {
        HCOMERR(QString("Wrong argument %1, must be 'on' or 'off'").arg(cmd));
        return;
    }


    if (mpCurrentPlotWindow)
    {
        PlotTab *pTab = mpCurrentPlotWindow->getCurrentPlotTab();
        if (pTab)
        {
            PlotArea *pArea = pTab->getPlotArea();
            if (pArea)
            {
                pArea->setRightAxisLogarithmic(isLog);
            }
        }
    }
}


//! @brief Execute function for "dipa" command
void HcomHandler::executeDisplayParameterCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    QString nameWildcard = "*";
    QString valueWildcard = "*";
    if(args.size() > 0)
    {
        nameWildcard = args[0];
    }
    if(args.size() > 1)
    {
        valueWildcard = args[1];
    }
    if(args.size() > 2)
    {
        HCOMERR("Wrong number of arguments, should be 0, 1 or 2");
        return;
    }

    QStringList parameters;
    getParameters(nameWildcard, parameters);

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
        QString outputValue = getParameterValue(parameters[p]);
        output.append(outputValue);
        if(valueWildcard.startsWith(">") && isNumber(valueWildcard.right(valueWildcard.size()-1)))
        {
            double limit = valueWildcard.right(valueWildcard.size()-1).toDouble();
            if(isNumber(outputValue) && outputValue.toDouble() > limit)
            {
                HCOMPRINT(output);
            }
        }
        else if(valueWildcard.startsWith("<") && isNumber(valueWildcard.right(valueWildcard.size()-1)))
        {
            double limit = valueWildcard.right(valueWildcard.size()-1).toDouble();
            if(isNumber(outputValue) && outputValue.toDouble() < limit)
            {
                HCOMPRINT(output);
            }
        }
        else
        {
            QRegExp rx(valueWildcard);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if(rx.exactMatch(outputValue))
            {
                HCOMPRINT(output);
            }
        }

    }
}


void HcomHandler::executeAddParameterCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    SystemObject *pContainer = mpModel->getViewContainerObject();
    if(pContainer)
    {
        QString type = "double";
        if (isString(args[1]))
        {
            type = "string";
            args[1] = removeQuotes(args[1]);
        }
        CoreParameterData paramData(args[0], args[1], type);
        pContainer->setOrAddParameter(paramData);

        //Make sure to remove local variables with same name
        mLocalVars.remove(args[0]);
    }
}


//! @brief Execute function for "chpa" command
void HcomHandler::executeChangeParameterCommand(const QString cmd)
{
    if(mpModel->isEditingFullyDisabled()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    QStringList splitCmd;
    splitWithRespectToQuotations(cmd, ' ', splitCmd);

    if(splitCmd.size() == 2)
    {
        if(!mpModel) { return; }
        SystemObject *pSystem = dynamic_cast<SystemObject*>(mpModel->getViewContainerObject());
        if(!pSystem) { return; }

        QStringList parameterNames;
        getParameters(splitCmd[0], parameterNames);

        //If no parameters found, try if component name can be evaluated by component index instead
        if(parameterNames.isEmpty())
        {
            evaluateExpression(splitCmd[0].section(".",0,0));
            if(mAnsType == Scalar)
            {
                QString compName = pSystem->getModelObjects().at(mAnsScalar)->getName();
                splitCmd[0].replace(0, splitCmd[0].section(".",0,0).size(), compName);
                getParameters(splitCmd[0], parameterNames);
            }
        }

        bool containsSystemParameter = false;
        SymHop::Expression expr(splitCmd[1]);
        for(const auto &var : expr.getVariables()) {
            if(mpModel->getViewContainerObject()->hasParameter(var.toString())) {
                containsSystemParameter = true;
                abort;
            }
        }

        QString newValueStr;
        if(containsSystemParameter)
        {
            newValueStr = splitCmd[1];  //If string contains a system parameter, don't evaluate it!
                                        //We want to assign the parameter value with the
                                        //name of the system parameter, not the value.
        }
        else
        {
            //! @todo it would be better if expressions could result in strings
            // Check if value is a string
            if (isString(splitCmd[1]))
            {
                newValueStr = removeQuotes(splitCmd[1]);
            }
            else
            {
                evaluateExpression(splitCmd[1], Scalar);
                if(mAnsType != Scalar)
                {
                    HCOMERR("Could not evaluate new value for parameter.");
                    return;
                }
                newValueStr = QString::number(mAnsScalar, 'g', 17);
            }
        }

        if(parameterNames.isEmpty())
        {
            HCOMERR("Parameter(s) not found: "+splitCmd[0]);
            return;
        }

        int nChanged=0;
        for(int p=0; p<parameterNames.size(); ++p)
        {
            parameterNames[p].remove("\"");
            toLongDataNames(parameterNames[p]);

            // Handled subsystem names
            QStringList sysnames;
            QString compname, parname;
            splitFullParameterName(parameterNames[p], sysnames, compname, parname);

            pSystem = qobject_cast<SystemObject*>(getModelPtr()->getViewContainerObject());
            for(const QString &sysname : sysnames)
            {
                pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(sysname));
            }

            // First check if this is a system parameter
            if(pSystem->hasParameter(parname))
            {
                CoreParameterData data;
                pSystem->getParameter(parname, data);     //Convert 1 to true and 0 to false in case of boolean parameters
                if(data.mType == "bool")
                {
                    if(newValueStr == "0") newValueStr = "false";
                    else if(newValueStr == "1") newValueStr = "true";
                }
                if(pSystem->setParameterValue(parname, newValueStr))
                    ++nChanged;
            }
            // Else check if this is an alias
            else if(!pSystem->getFullNameFromAlias(parname).isEmpty())
            {
                pSystem = qobject_cast<SystemObject*>(getModelPtr()->getViewContainerObject());
                QString nameFromAlias = pSystem->getFullNameFromAlias(parname);
                QStringList subsystems = nameFromAlias.split("$");
                subsystems.removeLast();
                nameFromAlias = nameFromAlias.split("$").last();
                QString compName = nameFromAlias.section("#",0,0);
                QString parName = nameFromAlias.right(nameFromAlias.size()-compName.size()-1);
                for(const QString &subsystem : subsystems) {
                    pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(subsystem));
                }

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
                    if(pComponent->setParameterValue(parName, newValueStr))
                        ++nChanged;
                }
            }
            else
            {
                ModelObject* pMO = pSystem->getModelObject(compname);
                if (pMO && pMO->setParameterValue(parname, newValueStr))
                {
                    ++nChanged;
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

void HcomHandler::executeDisplayComponentsCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    QString nameWildcard = "*";
    QString typeNameWildcard = "*";
    if(args.size() > 0)
    {
        nameWildcard = args[0];
    }
    if(args.size() > 1)
    {
        typeNameWildcard = args[1];
    }
    if(args.size() > 2)
    {
        HCOMERR("Wrong number of arguments, should be 0, 1 or 2");
        return;
    }

    QList<ModelObject *> components;
    getComponents(nameWildcard, components);

    int longestComponentName = 0;
    for(const auto &component : components)
    {
        if(component->getName().size() > longestComponentName)
        {
            longestComponentName = component->getName().size();
        }
    }


    for(const auto &component : components)
    {
        QString output = component->getName();
        int space = longestComponentName - output.size()+3;
        for(int s=0; s<space; ++s)
        {
            output.append(" ");
        }
        output.append("(");
        QString outputTypeName = component->getTypeName();
        output.append(outputTypeName);
        output.append(")");

        QRegExp rx(typeNameWildcard);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if(rx.exactMatch(outputTypeName)) {
            HCOMPRINT(output);
        }
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

        SystemObject *pCurrentSystem = dynamic_cast<SystemObject*>(mpModel->getViewContainerObject());
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
void HcomHandler::executeHelpCommand(QString arg)
{
    bool isFunction=false;
    int commandId=-1;
    arg.remove(" ");
    if(arg.isEmpty())
    {
        HCOMPRINT("-------------------------------------------------------------------------");
        HCOMPRINT(" Hopsan HCOM Terminal");
        QString commands;
        int n=0;
        QStringList groups;
        for(int c=0; c<mCmdList.size(); ++c)
        {
            n=qMax(mCmdList[c].cmd.size(), n);
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
        return;
    }
    else if(arg == "doxygen")
    {
        generateCommandsHelpText();
        return;
    }
    else if(arg.endsWith("()") && mLocalFunctionDescriptions.contains(arg.remove("()")))
    {
        isFunction = true;
    }
    else
    {
        for(int i=0; i<mCmdList.size(); ++i)
        {
            if(mCmdList[i].cmd == arg)
            {
                commandId = i;
                break;
            }
        }

        // If command was not found, search among functions
        if (commandId<0 && mLocalFunctionDescriptions.contains(arg) )
        {
            //HCOMINFO("Command not found, but showing help for function with same name");
            isFunction = true;
        }
    }

    if (isFunction)
    {
        QString description = mLocalFunctionDescriptions.find(arg).value().first;
        QString help = mLocalFunctionDescriptions.find(arg).value().second;

        QStringList helpLines = help.split("\n");
        int helpLength=0;
        for(const QString &line : helpLines) {
            if(line.size() > helpLength)
                helpLength = line.size();
        }
        int length=qMax(description.size(), helpLength)+2;
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
    else if (commandId>=0)
    {
        QStringList helpLines = mCmdList[commandId].help.split("\n");
        int helpLength=0;
        for(const QString &line : helpLines) {
            if(line.size() > helpLength)
                helpLength = line.size();
        }
        int length=qMax(mCmdList[commandId].description.size(), helpLength)+2;
        QString delimiterLine;
        for(int i=0; i<length; ++i)
        {
            delimiterLine.append("-");
        }
        QString descLine = mCmdList[commandId].description;
        descLine.prepend(" ");
        QString helpLine = mCmdList[commandId].help;
        helpLine.prepend(" ");
        helpLine.replace("\n", "\n ");
        HCOMPRINT(delimiterLine+"\n"+descLine+"\n"+helpLine+"\n"+delimiterLine);
    }
    else
    {
        HCOMERR("Command or function not found, or no help available for this command or function.");
    }
}


//! @brief Execute function for "exec" command
void HcomHandler::executeRunScriptCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() < 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString interpreter;
    if(args.size() > 1 && args[0].startsWith("-interpreter=")) {
        interpreter = args[0];
        interpreter.remove(0,13);
        args.removeFirst();
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
#ifdef _WIN32
    if(file.exists() && (path.endsWith(".bat") || interpreter == "cmd"))
    {
        QString cmd = path;
        if(!interpreter.isEmpty()) {
            cmd.prepend("CMD.exe /C "+interpreter+" ");
        }
        else {
            cmd.prepend("CMD.exe /C ");
        }
#else //Mac or Linux
    if(file.exists() && (path.endsWith(".sh") || interpreter == "sh" || interpreter == "bash"))
    {
        QString cmd = path;
        if(!interpreter.isEmpty()) {
            cmd.prepend(interpreter+" ");
        }
        else {
            cmd.prepend("sh ");
        }
#endif
        args.removeFirst();
        for(QString& arg : args) {
            evaluateExpression(arg);
            if(mAnsType == Scalar) {
                cmd.append(" "+QString::number(mAnsScalar));
            }
            else {
                cmd.append(" "+arg);
            }
        }
        HCOMINFO("Launching "+file.fileName()+"...");
        QDir prevDir = QDir::current();
        QDir::setCurrent(mPwd);
        system(cmd.toStdString().c_str());
        QDir::setCurrent(prevDir.absolutePath());
        HCOMINFO(file.fileName()+" finished.");
        return;
    }
    if (!file.exists() && !path.endsWith(".hcom"))
    {
        file.setFileName(path+".hcom");
    }
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        HCOMERR(QString("Unable to read file: %1").arg(file.fileName()));
        return;
    }

    QTextStream t(&file);
    QString code = t.readAll();
    file.close();

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
        if(code.contains("opt autostart")) {
            gpOptimizationDialog->run();
        }
        return;
    }

    QStringList lines = code.split("\n");
    lines.removeAll("");
    bool abort = false;
    QString gotoLabel = runScriptCommands(lines, &abort);
    if(abort)
    {
        return;
    }

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
                bool abort = false;
                gotoLabel = runScriptCommands(commands, &abort);
            }
        }
    }
}


//! @brief Execute function for "wrhi" command
void HcomHandler::executeWriteHistoryToFileCommand(const QString cmd)
{
    if(!mpConsole) return;

    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString path = args[0];
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


//! @brief Execute function for "wrtf" command
void HcomHandler::executeWriteToFileCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if(args.size() < 2 || args.size() > 3)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    //Parse the arguments, use flags i 3 arguments, else only filename and string
    QString flag;
    QString filePath;
    QString str;
    if(args.size() > 2)
    {
        flag = args[0];
        if(flag != "-a" && flag != "-e")
        {
            HCOMERR("Unknown flag: "+flag+", only \"-a\" or \"-e\" are allowed.");
            return;
        }
        filePath = args[1];
        str = args[2];
    }
    else
    {
        filePath = args[0];
        str = args[1];
    }

    //Convert filepath to absolute path (if relative is used)
    filePath.remove("\"");
    if(!filePath.contains("/"))
    {
        filePath.prepend("./");
    }
    QString dir = filePath.left(filePath.lastIndexOf("/"));
    dir = getDirectory(dir);
    filePath = dir+filePath.right(filePath.size()-filePath.lastIndexOf("/"));

    //Make sure string is enclosed in quotation marks
    if(!str.startsWith("\"") || !str.endsWith("\""))
    {
        HCOMERR("Expected a string enclosed in \" \".");
        return;
    }

    //Replace variables in string with their values (if enclosed in dollar signs)
    //! @todo This replace variable code is duplicated from print command, make a common function of it?
    int failed=0;
    while(str.count("$") > 1+failed*2)
    {
        QString varName = str.section("$",1+failed,1+failed);
        evaluateExpression(varName);
        if(mAnsType == Scalar)
        {
            str.replace("$"+varName+"$", QString::number(mAnsScalar, 'g', 17));
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

    //Remove quotation marks from string
    str = str.mid(1,str.size()-2);

    //Open file, depending on flags
    QFile file(filePath);
    if(file.exists() && flag != "-e" && flag != "-a")
    {
        HCOMERR("File already exist. Use \"-e\" flag to erase it or \"-a\" flag to append.");
        return;
    }
    if(flag == "-a")
    {
        if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Append))
        {
            HCOMERR("Unable to open file: \""+filePath+"\".");
            return;
        }
    }
    else if(flag == "-e")
    {
        if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
        {
            HCOMERR("Unable to open file: \""+filePath+"\".");
            return;
        }
    }
    else
    {
        if(!file.open(QFile::WriteOnly | QFile::Text))
        {
            HCOMERR("Unable to open file: \""+filePath+"\".");
            return;
        }
    }

    //Write to file
    file.write(QString(str+"\n").toLocal8Bit());

    //Close file
    file.close();
}


//! @brief Execute function for "print" command
void HcomHandler::executePrintCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if(args.size() == 0 || args.size() > 2)
    {
        HCOMERR("Expects one or two arguments! Enclose string in \" \".");
        return;
    }

    QString flag;
    QString str = args[0];
    if(str == "-e" || str == "-w" || str == "-i")
    {
        flag = str;
        if (args.size()>1)
        {
            str = args[1];
        }
    }

    bool treatAsExpression = (!str.startsWith("\"") || !str.endsWith("\""));
    if(treatAsExpression)
    {
        evaluateExpression(str);
        if(mAnsType == Scalar)
        {
            str = QString::number(mAnsScalar, 'g', 17);
        }
        else if (mAnsType == String)
        {
            str = mAnsWildcard;
        }
        else if (mAnsType == DataVector)
        {
            QString array;
            QTextStream ts(&array);
            mAnsVector->sendDataToStream(ts," ");
            str = array;
        }
    }
    else
    {
        int failed=0;
        while(str.count("$") > 1+failed*2)
        {
            QString varName = str.section("$",1+failed,1+failed);
            evaluateExpression(varName);
            if(mAnsType == Scalar)
            {
                str.replace("$"+varName+"$", QString::number(mAnsScalar, 'g', 17));
            }
            else if (mAnsType == String)
            {
                str.replace("$"+varName+"$", mAnsWildcard);
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
    }

    if(flag == "-e")
    {
        mpConsole->printErrorMessage(str,"",false,true);
    }
    else if(flag == "-w")
    {
        mpConsole->printWarningMessage(str,"",false,true);
    }
    else if(flag == "-i")
    {
        mpConsole->printInfoMessage(str,"",false,true);
    }
    else
    {
        mpConsole->print(str,true);
    }
}

void HcomHandler::executeEvalCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    if(args.size() != 1)
    {
        HCOMERR("Expects one argument!");
        return;
    }

    if (isString(args.front()))
    {
        QString inexpr = removeQuotes(args.front());
        QString outexpr;
        QStringList exprs;
        QList<int> sectionIds;
        int nDollar = inexpr.count('$');
        extractSections(inexpr, '$', exprs, sectionIds);
        if (nDollar % 2 != 0)
        {
            HCOMWARN("Mismatch $ in expression");
        }

        for (int i=0; i<exprs.size(); ++i)
        {
            if (sectionIds.contains(i))
            {
                evaluateExpression(exprs[i]);
                if(mAnsType == Scalar)
                {
                    outexpr.append(QString("%1").arg(mAnsScalar));
                }
                else if (mAnsType == String)
                {
                    outexpr.append(mAnsWildcard);
                }
                else if (mAnsType == DataVector)
                {
                    QString array;
                    QTextStream ts(&array);
                    mAnsVector->sendDataToStream(ts," ");
                    outexpr.append(array);
                }
                else
                {
                    HCOMERR("Failed to evaluate: "+exprs[i]);
                    return;
                }
            }
            else
            {
                outexpr.append(exprs[i]);
            }
        }
        executeCommand(outexpr);
    }
    else
    {
        HCOMERR("Command expects a \"string\"");
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
    gpMainWindowWidget->activateWindow();
    mpCurrentPlotWindow->raise();
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
    if(getNumberOfCommandArguments(cmd) < 2)
    {
        QStringList output;
        if(cmd.isEmpty())
        {
            getMatchingLogVariableNames(QString("*%1H").arg(generationspecifier::separatorString), output, false);
        }
        else
        {
            getMatchingLogVariableNames(cmd, output);
        }

        for(auto &o : output)
        {
            HCOMPRINT(o);
        }
    }
    else
    {
        HCOMERR("Wrong number of arguments.");
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

    SharedVectorVariableT pData = getLogVariable(variable);
    if(pData)
    {
        QString err;
        double r = pData->peekData(id, err);
        if (err.isEmpty())
        {
            HCOMPRINT(QString::number(r, 'g', 17));
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

    SharedVectorVariableT pData = getLogVariable(variable);
    if(pData)
    {
        QString err;
        double r = pData->pokeData(id, value, err);
        if (err.isEmpty())
        {
            HCOMPRINT(QString::number(r, 'g', 17));
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
    QStringList hierarchy = variable.section("#",0).split("$");
    hierarchy.removeLast();
    SystemObject *pSystem = searchIntoSubsystem(mpModel->getViewContainerObject(),hierarchy);
    if(pSystem->setVariableAlias(variable, alias))
    {
        HCOMINFO(QString("Sucessfully assigned variable alias %1").arg(alias));
    }
    else
    {
        HCOMERR(QString("Failed to assign variable alias %1").arg(alias));
    }
}


//! @brief Execute function for "rmvar" command
void HcomHandler::executeRemoveVariableCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);

    bool excludeAliases = args.contains("-noalias");
    QStringList aliasNames;
    if(excludeAliases)
    {
        args.removeAll("-noalias");
        aliasNames = mpModel->getViewContainerObject()->getAliasNames();
    }

    QStringList excludes;
    while(args.contains("-n"))
    {
        int i=args.indexOf("-n")+1;
        if (i < args.size())
        {
            QString exclude_pattern = args[i];
            bool parseGenOk;
            int gen = parseAndChopGenerationSpecifier(exclude_pattern, parseGenOk);
            // If generation is not specified, then assume current generation
            if (parseGenOk && !isValidGenerationValue(gen))
            {
                exclude_pattern.append(generationspecifier::separatorChar).append("c");
            }
            QStringList excludeNames;
            getMatchingLogVariableNames(exclude_pattern, excludeNames);
            excludes.append(excludeNames);
            args.removeAt(i);
        }
        args.removeAt(i-1);
    }

    for(int s=0; s<args.size(); ++s)
    {
        QString tmp = args[s];
        bool parseGenOk;
        int gen = parseAndChopGenerationSpecifier(tmp, parseGenOk);
        // If generation is not specified, then assume current generation
        if (parseGenOk && !isValidGenerationValue(gen))
        {
            args[s].append(generationspecifier::separatorChar).append("c");
        }

        QStringList variables;
        getMatchingLogVariableNames(args[s], variables, true);
        for(int v=0; v<variables.size(); ++v)
        {
            if(excludes.contains(variables[v]) || (excludeAliases && aliasNames.contains(variables[v])) )
            {
                continue;
            }
            else
            {
                removeLogVariable(variables[v]);
            }
        }
    }
}

//! @brief Execute function for "dihg" command
void HcomHandler::executeDisplayHighestGenerationCommand(const QString cmd)
{
    Q_UNUSED(cmd);

    if(!mpModel) {
        HCOMERR("No model is open.");
        mAnsType = Undefined;
        return;
    }

    int gen = mpModel->getLogDataHandler()->getHighestGenerationNumber();
    gen++; //convert from zero-indexing to one-indexing
    HCOMPRINT(QString::number(gen));
    mAnsScalar = gen;
    mAnsType = Scalar;
}




////! @brief Execute function for "chdfsc" command
//void HcomHandler::executeChangeDefaultPlotScaleCommand(const QString cmd)
//{
//    QStringList args = splitCommandArguments(cmd);
//    if(args.size() != 2)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    const QString &name = args[0];
//    const QString &scale = args[1];

//    QStringList vars;
//    getMatchingLogVariableNames(name,vars);
//    if(vars.isEmpty())
//    {
//        getMatchingLogVariableNamesWithoutLogDataHandler(name,vars);
//    }
//    int nChanged=0;
//    foreach(const QString var, vars)
//    {
//        QStringList fields = var.split(".");
//        // Handle alias
//        if (fields.size() == 1)
//        {
//            QString fullName = getfullNameFromAlias(fields.first());
//            toShortDataNames(fullName);
//            fields = fullName.split(".");
//        }

//        if (fields.size() == 3)
//        {
//            ModelObject *pComponent = mpModel->getViewContainerObject()->getModelObject(fields.first());
//            if(!pComponent)
//            {
//                HCOMERR(QString("Component not found: %1").arg(fields.first()));
//            }

//            QString description = "";
//            QString longVarName = fields[1]+"."+fields[2];
//            toLongDataNames(longVarName);
//            if(pComponent->getPort(fields[1]) && NodeInfo(pComponent->getPort(fields[1])->getNodeType()).shortNames.contains(fields[2]))
//            {
//                //pComponent->registerCustomPlotUnitOrScale(longVarName, description, scale);
//                HCOMERR("This function is no longer implemented"); //! @todo fixa /Peter
//                ++nChanged;
//            }
//        }
//        else
//        {
//            HCOMERR(QString("Unknown variable: %1").arg(var));
//        }
//    }

//    if(nChanged > 0)
//    {
//        HCOMINFO(QString("Changed default scale for %1 variables.").arg(nChanged));
//    }
//    else
//    {
//        HCOMERR(QString("No matching variables found."));
//    }
//}


////! @brief Execute function for "didfsc" command
//void HcomHandler::executeDisplayDefaultPlotScaleCommand(const QString cmd)
//{
//    QStringList args = splitCommandArguments(cmd);
//    if(args.size() != 1)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    QStringList vars;
//    getMatchingLogVariableNames(args[0],vars);
//    if(vars.isEmpty())
//    {
//        getMatchingLogVariableNamesWithoutLogDataHandler(cmd, vars);
//    }
//    if(vars.isEmpty())
//    {
//        HCOMERR(QString("Variable not found: %1").arg(cmd));
//        mAnsType = Undefined;
//        return;
//    }
//    foreach(const QString var, vars)
//    {
//        QString dispName;
//        QStringList fields = var.split(".");
//        // Handle alias
//        if (fields.size() == 1)
//        {
//            dispName = fields.first();
//            QString fullName = getfullNameFromAlias(fields.first());
//            fields = fullName.split("#");
//        }

//        // Handle comp.port.var variable
//        if (fields.size() == 3)
//        {
//            // Only set dispName if it was not an alias (set above)
//            if (dispName.isEmpty())
//            {
//                dispName = var;
//            }

//            ModelObject *pComponent = gpModelHandler->getCurrentViewContainerObject()->getModelObject(fields.first());
//            if(!pComponent)
//            {
//                HCOMERR(QString("Component not found: %1").arg(fields.first()));
//                continue;
//            }

//            UnitConverter unitScale;
//            QString portAndVarName = fields[1]+"."+fields[2];
//            toLongDataNames(portAndVarName);

//            //pComponent->getCustomPlotUnitOrScale(portAndVarName, unitScale);
//            HCOMERR("This function is no  longer implemented"); //! @todo fixa /Peter

//            const QString &scale = unitScale.mScale;
//            const QString &unit = unitScale.mUnit;
//            if(!unit.isEmpty() && !scale.isEmpty())
//            {
//                HCOMPRINT(QString("%1: %2 [%3]").arg(dispName).arg(scale).arg(unit));
//                mAnsType = Scalar;
//                mAnsScalar = scale.toDouble();
//                continue;
//            }
//            else if (!scale.isEmpty())
//            {
//                HCOMPRINT(QString("%1: %2").arg(dispName).arg(scale));
//                mAnsType = Scalar;
//                mAnsScalar = scale.toDouble();
//                continue;
//            }
//            HCOMPRINT(QString("%1: 1").arg(dispName));
//            mAnsType = Scalar;
//            mAnsScalar = 1;
//        }
//        else
//        {
//            HCOMERR(QString("Unknown variable: %1").arg(var));
//            mAnsType = Undefined;
//        }
//    }
//}


////! @brief Execute function for "chdfos" command
//void HcomHandler::executeChangeDefaultPlotOffsetCommand(const QString cmd)
//{
//    QStringList args = splitCommandArguments(cmd);
//    if(args.size() != 2)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    const QString &name = args[0];
//    evaluateExpression(args[1]);
//    if(mAnsType != Scalar)
//    {
//        HCOMERR("Second argument must be a scalar variable.");
//        return;
//    }
//    double offset = mAnsScalar;

//    QStringList vars;
//    getMatchingLogVariableNames(name,vars);
//    if(vars.isEmpty())
//    {
//        getMatchingLogVariableNamesWithoutLogDataHandler(name,vars);
//    }
//    int nChanged=0;
//    foreach(const QString var, vars)
//    {
//        QStringList fields = var.split(".");
//        // Handle alias
//        if (fields.size() == 1)
//        {
//            QString fullName = getfullNameFromAlias(fields.first());
//            toShortDataNames(fullName);
//            fields = fullName.split(".");
//        }

//        if (fields.size() == 3)
//        {
//            ModelObject *pComponent = mpModel->getViewContainerObject()->getModelObject(fields.first());
//            if(!pComponent)
//            {
//                HCOMERR(QString("Component not found: %1").arg(fields.first()));
//            }

//            QString description = "";
//            QString longVarName = fields[1]+"."+fields[2];
//            toLongDataNames(longVarName);
//            if(pComponent->getPort(fields[1]) && NodeInfo(pComponent->getPort(fields[1])->getNodeType()).shortNames.contains(fields[2]))
//            {
//                //pComponent->registerCustomPlotOffset(longVarName, offset);
//                HCOMERR("This function is no longer implemented"); //! @todo fixa /Peter
//                ++nChanged;
//            }
//        }
//        else
//        {
//            HCOMERR(QString("Unknown variable: %1").arg(var));
//        }
//    }

//    if(nChanged > 0)
//    {
//        HCOMINFO(QString("Changed default scale for %1 variables.").arg(nChanged));
//    }
//    else
//    {
//        HCOMERR(QString("No matching variables found."));
//    }
//}


////! @brief Execute function for "didfos" command
//void HcomHandler::executeDisplayDefaultPlotOffsetCommand(const QString cmd)
//{
//    QStringList args = splitCommandArguments(cmd);
//    if(args.size() != 1)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    QStringList vars;
//    getMatchingLogVariableNames(args[0],vars);
//    if(vars.isEmpty())
//    {
//        getMatchingLogVariableNamesWithoutLogDataHandler(cmd, vars);
//    }
//    if(vars.isEmpty())
//    {
//        HCOMERR(QString("Variable not found: %1").arg(cmd));
//        mAnsType = Undefined;
//        return;
//    }
//    foreach(const QString var, vars)
//    {
//        QString dispName;
//        QStringList fields = var.split(".");
//        // Handle alias
//        if (fields.size() == 1)
//        {
//            dispName = fields.first();
//            QString fullName = getfullNameFromAlias(fields.first());
//            fields = fullName.split("#");
//        }

//        // Handle comp.port.var variable
//        if (fields.size() == 3)
//        {
//            // Only set dispName if it was not an alias (set above)
//            if (dispName.isEmpty())
//            {
//                dispName = var;
//            }

//            ModelObject *pComponent = gpModelHandler->getCurrentViewContainerObject()->getModelObject(fields.first());
//            if(!pComponent)
//            {
//                HCOMERR(QString("Component not found: %1").arg(fields.first()));
//                continue;
//            }

//            double offset;
//            QString portAndVarName = fields[1]+"."+fields[2];
//            toLongDataNames(portAndVarName);

//            //offset = pComponent->getCustomPlotOffset(portAndVarName);
//            HCOMERR("This function is no longer implemented"); //! @todo fixa /Peter

//            HCOMPRINT(QString("%1: %2").arg(dispName).arg(offset));

//            mAnsType = Scalar;
//            mAnsScalar = offset;
//        }
//        else
//        {
//            HCOMERR(QString("Unknown variable: %1").arg(var));
//            mAnsType = Undefined;
//        }
//    }
//}


//! @brief Execute function for "info" command
void HcomHandler::executeVariableInfoCommand(const QString cmd)
{
    SharedVectorVariableT pVar = getLogVariable(cmd);
    if (pVar)
    {
        QString type = variableTypeAsString(pVar->getVariableType());
        QString gen = QString("%1").arg(pVar->getGeneration());
        QString length = QString("%1").arg(pVar->getDataSize());

        QString infotext("\n");
        infotext.append("       Name: ").append(pVar->getFullVariableName()).append("\n");
        infotext.append("       Type: ").append(type).append("\n");
        infotext.append("   Quantity: ").append(pVar->getDataQuantity()).append("\n");
        if (pVar->getDataQuantity() == TIMEVARIABLENAME || pVar->getDataQuantity() == FREQUENCYVARIABLENAME)
        {
            infotext.append(QString("Plot offset: %1").arg(pVar->getGenerationPlotOffsetIfTime())).append("\n");
        }
        infotext.append("     Length: ").append(length).append("\n");
        infotext.append(" Generation: ").append(gen);

        HCOMPRINT(infotext);
    }
    else if(mLocalVars.contains(cmd))
    {
        QString type = "Scalar";

        QString infotext("\n");
        infotext.append("       Name: ").append(cmd).append("\n");
        infotext.append("       Type: ").append(type).append("\n");
        infotext.append("      Value: ").append(QString::number(mLocalVars.find(cmd).value(), 'g', 17));

        HCOMPRINT(infotext);
    }
    else
    {
        HCOMERR(QString("Could not find a variable matching: %1").arg(cmd));
    }
}

void HcomHandler::executeSetQuantityCommand(const QString args)
{
    QStringList arglist = splitCommandArguments(args);
    if(arglist.size() != 2)
    {
        HCOMERR("Needs two arguments, variable name and quantity (or - to clear)");
        return;
    }
    if(!mpModel) { return; }
    SystemObject *pSystem = qobject_cast<SystemObject*>(mpModel->getViewContainerObject());
    if(!pSystem) { return; }


    QString variableName = arglist.first();
    QString quantity = arglist.last();

    bool genOK;
    int gen = parseAndChopGenerationSpecifier(variableName, genOK);
    if (!genOK) {
        HCOMERR("Could not parse generation specifier");
        return;
    }

    // Convert to model name format
    toLongDataNames(variableName);

    // Check if we should clear quantity
    CoreQuantityAccess cqa;
    if (quantity == "-") {
        quantity.clear();
    }
    // Else check if quantity is valid
    else if (!cqa.haveQuantity(quantity)) {
        HCOMERR("Invalid quantity: "+quantity);
        return;
    }

    // Handle no specified generation as if the current generation was chosen
    if (gen == generationspecifier::noGenerationSpecified) {
        gen = generationspecifier::currentGeneration;
    }

    // First look for variable in logdata
    QVector<int> logDataVariablesGenerations;
    if (isSingleGenerationValue(gen)) {
        auto pLogDataVariable = mpModel->getLogDataHandler()->getVectorVariable(variableName, gen);
        if (pLogDataVariable) {
            logDataVariablesGenerations.append(pLogDataVariable->getGeneration());
        }
    }
    else if (gen == generationspecifier::allGenerations) {
        QRegExp regexp(variableName, Qt::CaseSensitive, QRegExp::FixedString);
        auto logDataVariables = mpModel->getLogDataHandler()->getMatchingVariablesFromAllGenerations(regexp);
        for (const auto& pVar : logDataVariables) {
            logDataVariablesGenerations.append(pVar->getGeneration());
        }
    }

    // If specifier was not given, set to current gen, or set to all, then also search for variable in model
    bool foundInModel=false;
    if ((gen == generationspecifier::currentGeneration) || gen == generationspecifier::allGenerations) {
        // Split full name
        QStringList sysnames;
        QString comp,port,var;
        bool splitOK = splitFullVariableName(variableName, sysnames, comp, port, var);
        if (!splitOK) {
            HCOMERR("Invalid or incomplete variable name: "+variableName);
            return;
        }

        // Search into subsystem
        pSystem = searchIntoSubsystem(pSystem, sysnames);
        if (pSystem) {
            // Get component
            ModelObject *pComponent = pSystem->getModelObject(comp);
            if (pComponent) {
                foundInModel=true;
                // Set the quantity in the model
                // Note! The quantity change will be signaled to the log data handler as well
                bool rc = pComponent->setModifyableSignalQuantity(port+"#"+var, quantity);
                if (!rc) {
                    HCOMERR("Could not set quantity: "+quantity+" on model variable: "+variableName);
                }
            }
        }

        // If we did not find it in the model and we do not have an alternate log data variable, then give an error message
        if(!foundInModel && logDataVariablesGenerations.isEmpty()) {
            HCOMERR("Could not find model variable with full name: "+variableName);
            return;
        }
    }

    // Set quantity for specified log data variable at each desired generation
    for (const int gen : logDataVariablesGenerations) {
        mpModel->getLogDataHandler()->registerQuantity(variableName, quantity, gen);
    }
}



////! @brief Execute function for "chsc" command
//void HcomHandler::executeChangePlotScaleCommand(const QString cmd)
//{
//    QStringList args = splitCommandArguments(cmd);
//    if(args.size() != 2)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    const QString &varName = args[0];
//    evaluateExpression(args[1]);
//    if(mAnsType != Scalar)
//    {
//        HCOMERR("Second argument must be a scalar variable.");
//        return;
//    }
//    double scale = mAnsScalar;

//    QStringList vars;
//    getMatchingLogVariableNames(varName,vars);
//    if(vars.isEmpty())
//    {
//        HCOMERR("Unknown variable: "+varName);
//        return;
//    }
//    Q_FOREACH(const QString var, vars)
//    {
//        SharedVectorVariableT pVar = getLogVariable(var);
//        //pVar.data()->setPlotScale(scale);
//        //! @todo fixa /Peter
//    }
//}


////! @brief Execute function for "disc" command
//void HcomHandler::executeDisplayPlotScaleCommand(const QString cmd)
//{
//    if(getNumberOfCommandArguments(cmd) != 1)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    QStringList vars;
//    getMatchingLogVariableNames(cmd,vars);
//    if(vars.isEmpty())
//    {
//        HCOMERR("Unknown variable: "+cmd);
//        return;
//    }
//    Q_FOREACH(const QString var, vars)
//    {
//        SharedVectorVariableT pVar = getLogVariable(var);
//        if(!pVar.isNull())
//        {
////            QString scale = pVar.data()->getCustomUnitScale().mScale;
////            QString unit = pVar.data()->getCustomUnitScale().mUnit;
//            QString scale = "NULL";
//            QString unit = "NULL";
//            if(!scale.isEmpty() && !unit.isEmpty())
//            {
//                HCOMPRINT(QString("%1: %2 [%3]").arg(var).arg(scale).arg(unit));
//                mAnsType = Scalar;
//                mAnsScalar = scale.toDouble();
//                continue;
//            }
//            else if(!scale.isEmpty())
//            {
//                HCOMPRINT(QString("%1: %2").arg(var).arg(scale));
//                mAnsType = Scalar;
//                mAnsScalar = scale.toDouble();
//                continue;
//            }
//            else
//            {
////                scale = QString::number(pVar.data()->getPlotScale());
////                unit = pVar.data()->getPlotScaleDataUnit();
//                QString scale = "NULL";
//                QString unit = "NULL";
//                if(!scale.isEmpty() && !unit.isEmpty())
//                {
//                    HCOMPRINT(QString("%1: %2 [%3]").arg(var).arg(scale).arg(unit));
//                    mAnsType = Scalar;
//                    mAnsScalar = scale.toDouble();
//                    continue;
//                }
//                else if(!scale.isEmpty())
//                {
//                    HCOMPRINT(QString("%1: %2").arg(var).arg(scale));
//                    mAnsType = Scalar;
//                    mAnsScalar = scale.toDouble();
//                    continue;
//                }
//            }
//            HCOMERR("Variable not found: "+var);
//            mAnsType = Undefined;
//        }
//    }

//    return;
//}


////! @brief Execute function for "chtos" command
//void HcomHandler::executeChangeTimePlotOffsetCommand(const QString cmd)
//{
//    QStringList args = splitCommandArguments(cmd);
//    if(args.size() != 2)
//    {
//        HCOMERR("Wrong number of arguments.");
//        return;
//    }

//    const QString &varName = args[0];
//    evaluateExpression(args[1]);
//    if(mAnsType != Scalar)
//    {
//        HCOMERR("Second argument must be a scalar variable.");
//        return;
//    }
//    double offset = mAnsScalar;

//    QStringList vars;
//    getMatchingLogVariableNames(varName,vars);
//    if(vars.isEmpty())
//    {
//        HCOMERR("Unknown variable: "+varName);
//        return;
//    }
//    for(const QString &var : vars)
//    {
//        SharedVectorVariableT pVar = getLogVariable(var);
//        if (pVar)
//        {
//            pVar->setPlotOffsetIfTime(offset);
//        }
//    }
//}


//! @brief Execute function for "dito" command
void HcomHandler::executeDisplayTimePlotOffsetCommand(const QString cmd)
{
    QStringList arglist = splitCommandArguments(cmd);
    if(arglist.size() > 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    int generation=-1;
    bool genOK=true;
    if (arglist.size() == 1)
    {
        QString genstring = "@"+arglist[0];
        generation = parseAndChopGenerationSpecifier(genstring, genOK);
    }

    if (mpModel && mpModel->getLogDataHandler())
    {
        if (generation == generationspecifier::currentGeneration) {
            generation = mpModel->getLogDataHandler()->getCurrentGenerationNumber();
        }
        else if (!isSingleGenerationValue(generation)) {
            HCOMERR("Incorrect generation value, or could not parse generation");
            return;
        }

        const LogDataGeneration *pGen = mpModel->getLogDataHandler()->getGeneration(generation);
        if (pGen)
        {
            mAnsScalar = pGen->getTimeOffset();
            UnitConverter uc;
            gpConfig->getUnitScale("Time", gpConfig->getDefaultUnit("Time"), uc);
            HCOMPRINT(QString("%1 %2").arg(uc.convertFromBase(pGen->getTimeOffset())).arg(gpConfig->getDefaultUnit("Time")));
        }
        else
        {
            HCOMERR("Specified generation not found");
        }
    }
}


void HcomHandler::executeChangeTimePlotOffsetCommand(const QString cmd)
{
    QStringList arglist = splitCommandArguments(cmd);
    if(arglist.size() < 1 || arglist.size() > 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    evaluateExpression(arglist[0]);
    if(mAnsType != Scalar)
    {
        HCOMERR("First argument must be a scalar variable.");
        return;
    }
    double offset = mAnsScalar;

    int generation=-1;
    bool genOK=true;
    if (arglist.size() == 2)
    {
        QString genstring = "@"+arglist[1];
        generation = parseAndChopGenerationSpecifier(genstring, genOK);
    }

    if (mpModel && mpModel->getLogDataHandler())
    {
        if (generation == generationspecifier::currentGeneration)
        {
            generation = mpModel->getLogDataHandler()->getCurrentGenerationNumber();
        }
        else if (!isSingleGenerationValue(generation))
        {
            HCOMERR("Incorrect generation value, or could not parse generation");
            return;
        }

        if (mpModel->getLogDataHandler()->hasGeneration(generation))
        {
            UnitConverter uc;
            gpConfig->getUnitScale("Time", gpConfig->getDefaultUnit("Time"), uc);
            mpModel->getLogDataHandler()->setGenerationTimePlotOffset(generation, uc.convertToBase(offset));
        }
        else
        {
            HCOMERR("Specified generation not found");
            return;
        }
    }
}

//! @brief Execute function for "sapw" command
void HcomHandler::executeSavePlotWindowCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if (args.size() > 0)
    {
        QString dpi;
        QString dim = "px";
        QString filename = args.front();
        filename.remove("\"");
        QString ext = extractFilenameExtension(filename);

        // Make an absolute path relative mPwd if given path is not already absolute
        QFileInfo file(filename);
        if (!file.isAbsolute())
        {
            file.setFile(mPwd+"/"+filename);
            filename = file.absoluteFilePath();
        }

        // Check so that destination directory exists and is writable
        QDir dstDir(file.absolutePath());
        if (!dstDir.exists())
        {
            HCOMERR(QString("The directory: '%1' does not exist!").arg(dstDir.absolutePath()));
            return;
        }

        // Read dpi argument if specified
        if (args.size() > 4)
        {
            dpi = args[4];
        }

        // Read dim unit argument if specified
        if (args.size() > 3)
        {
            dim = args[3];
        }

        QString width, height;
        // If both width and height specified then use both
        if (args.size() > 2)
        {
            width = args[1];
            height = args[2];
        }
        else if (args.size() == 2)
        {
            HCOMERR("You must specify both width and height");
            return;
        }

        if (mpCurrentPlotWindow && mpCurrentPlotWindow->getCurrentPlotTab())
        {
            if (width.isEmpty() && mpCurrentPlotWindow->getCurrentPlotTab()->getQwtPlot(0))
            {
                width = QString("%1").arg(mpCurrentPlotWindow->getCurrentPlotTab()->getQwtPlot(0)->width());
                height = QString("%1").arg(mpCurrentPlotWindow->getCurrentPlotTab()->getQwtPlot(0)->height());
            }
            mpCurrentPlotWindow->getCurrentPlotTab()->exportAsImage(filename, ext, width, height, dim, dpi);
        }
        else
        {
            HCOMERR("No current plotwindow to plot");
        }
    }
    else
    {
        HCOMERR("At least one argumetn rewquired (filename)");
    }
}

void HcomHandler::executeInvertPlotVariableCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 1)
    {
        HCOMERR("Wrong number of arguments. At most one arguemnt supported.");
        return;
    }

    QStringList variables;
    getMatchingLogVariableNames(cmd, variables);

    if(variables.isEmpty())
    {
        HCOMERR("Could not find variable matching: "+cmd);
        return;
    }
    for(const QString &var : variables)
    {
        SharedVectorVariableT pVar = getLogVariable(var);
        pVar->togglePlotInverted();
    }
}

void HcomHandler::executeSetlabelCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 2)
    {
        HCOMERR("Wrong number of arguments. Must specify variable and label");
        return;
    }

    QStringList variables;
    getMatchingLogVariableNames(args.first(), variables);

    if(variables.isEmpty())
    {
        HCOMERR("Could not find variable matching: "+cmd);
        return;
    }
    for(const QString &var : variables)
    {
        SharedVectorVariableT pVar = getLogVariable(var);
        pVar->setCustomLabel(removeQuotes(args.last()));
    }
}


//! @brief Execute function for "dlog" command
void HcomHandler::executeDisableLoggingCommand(const QString cmd)
{
    int nArgs = getNumberOfCommandArguments(cmd);
    if(nArgs < 1 || nArgs > 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    //Parse second argument (if existing)
    bool noAlias = false;
    if(getNumberOfCommandArguments(cmd) == 2)
    {
        if(splitCommandArguments(cmd).at(1) == "-noalias")
        {
            noAlias = true;
        }
        else
        {
            HCOMERR("Unknown argument.");   //Two arguments, but second one is not "-noalias", which is the only allowed one
            return;
        }
    }

    //Disable all nodes matching specified port name wildcard
    QList<Port*> vPortPtrs;
    getPorts(splitCommandArguments(cmd).at(0), vPortPtrs);
    for(int p=0; p<vPortPtrs.size(); ++p)
    {
        mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setLoggingEnabled(vPortPtrs.at(p)->getParentModelObjectName(), vPortPtrs.at(p)->getName(), false);
    }


    //If alias shall be ignored, we must enable all nodes with alias variables again
    if(noAlias)
    {
        for(const QString &aliasName : mpModel->getViewContainerObject()->getAliasNames()) {
            QString var = mpModel->getViewContainerObject()->getFullNameFromAlias(aliasName);
            QString comp = var.section("#",0,0);
            QString port = var.section("#",1,1);
            mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setLoggingEnabled(comp, port, true);
        }
    }
}


//! @brief Execute function for "elog" command
void HcomHandler::executeEnableLoggingCommand(const QString cmd)
{
    QStringList args = cmd.split(" ");

    QList<Port*> vPortPtrs;
    for(const QString &arg : args) {
        getPorts(arg, vPortPtrs);
    }

    for(const auto &pPort : vPortPtrs)
    {
        mpModel->getViewContainerObject()->getCoreSystemAccessPtr()->setLoggingEnabled(pPort->getParentModelObjectName(), pPort->getName(), true);
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
        getConfigPtr()->setBoolSetting(cfg::multicore, value=="on");
    }
    else if(pref == "threads")
    {
        evaluateExpression(value, Scalar);
        if(mAnsType != Scalar)
        {
            HCOMERR("Unknown value.");
            return;
        }
        else if(mAnsScalar < 0)
        {
            HCOMERR("Number of simulation threads must be a positive integer.");
            return;
        }
        getConfigPtr()->setIntegerSetting(cfg::numberofthreads, mAnsScalar);
    }
    else if(pref == "algorithm")
    {
        evaluateExpression(value, Scalar);
        if(mAnsType != Scalar)
        {
            HCOMERR("Unknown value.");
            return;
        }
        getConfigPtr()->setParallelAlgorithm(mAnsScalar);
    }
    else if(pref == "cachetodisk")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        getConfigPtr()->setBoolSetting(cfg::cachelogdata, value=="on");
    }
    else if(pref == "generationlimit")
    {
        evaluateExpression(value, Scalar);
        if(mAnsType != Scalar)
        {
            HCOMERR("Unknown value.");
            return;
        }
        else if(mAnsScalar < 0)
        {
            HCOMERR("Generation limit must be a positive integer.");
            return;
        }
        getConfigPtr()->setIntegerSetting(cfg::generationlimit, mAnsScalar);
    }
    else if(pref == "samples")
    {
        evaluateExpression(value, Scalar);
        if(mAnsType != Scalar)
        {
            HCOMERR("Unknown value.");
            return;
        }
        else if(mAnsScalar < 0)
        {
            HCOMERR("Number of log samples must be a positive integer.");
            return;
        }
        mpModel->getViewContainerObject()->setNumberOfLogSamples(mAnsScalar);
    }
    else if(pref == "undo")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        mpModel->getViewContainerObject()->setUndoEnabled(value == "on", true);
    }
    else if(pref == "backup")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        getConfigPtr()->setBoolSetting(cfg::autobackup, value=="on");
    }
    else if(pref == "progressbar")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        getConfigPtr()->setBoolSetting(cfg::progressbar, value=="on");
    }
    else if(pref == "progressbarstep")
    {
        evaluateExpression(value, Scalar);
        if(mAnsType != Scalar)
        {
            HCOMERR("Unknown value.");
            return;
        }
        else if(mAnsScalar < 0)
        {
            HCOMERR("Progress bar step size must be a positive integer.");
            return;
        }
        getConfigPtr()->setIntegerSetting(cfg::progressbarstep, mAnsScalar);
    }
    else if(pref == "logduringsimulation")
    {
        if(value != "on" && value != "off")
        {
            HCOMERR("Unknown value.");
        }
        getConfigPtr()->setBoolSetting(cfg::logduringsimulation, value=="on");
    }
    else
    {
        HCOMERR("Unknown command.");
    }
}

void HcomHandler::executeGetCommand(const QString cmd)
{
    QStringList splitCmd = cmd.split(" ");
    if(splitCmd.size() != 1 && splitCmd.size() != 0)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }
    QString pref = "";
    bool all=false;
    if(splitCmd[0] != "")
    {
        pref = splitCmd[0];
    }
    else
    {
        all=true;
    }

    if(all || pref == "multicore")
    {
        QString output = "Multi-threaded simulation: ";
        if(getConfigPtr()->getBoolSetting(cfg::multicore))
            output.append("ON");
        else
            output.append("OFF");
        HCOMPRINT(output);
    }
    if(all || pref == "threads")
    {
        QString output = "Simulation threads:        ";
        output.append(QString::number(getConfigPtr()->getIntegerSetting(cfg::numberofthreads))+"");
        HCOMPRINT(output);
    }
    if(all || pref == "algorithm")
    {
        QString output = "Parallel algorithm:        ";

        switch (getConfigPtr()->getParallelAlgorithm())
        {
        case hopsan::APrioriScheduling :
            output.append("a priori scheduling");
            break;
        case hopsan::TaskPoolAlgorithm :
            output.append("task pool scheduling");
            break;
        case hopsan::TaskStealingAlgorithm :
            output.append("task-stealing");
            break;
        case hopsan::ForkJoinAlgorithm :
            output.append("fork-join scheduling");
            break;
        case hopsan::ClusteredForkJoinAlgorithm :
            output.append("clustered fork-join scheduling");
            break;
        default :
            output.append("unknown ("+QString::number(getConfigPtr()->getParallelAlgorithm())+")");
            break;
        }
        HCOMPRINT(output);
    }
    if(all || pref == "cachetodisk")
    {
        QString output = "Cache log data to disk:    ";
        if(getConfigPtr()->getBoolSetting(cfg::cachelogdata))
            output.append("ON");
        else
            output.append("OFF");
        HCOMPRINT(output);
    }
    if(all || pref == "generationlimit")
    {
        QString output = "Generation limit:          ";
        output.append(QString::number(getConfigPtr()->getIntegerSetting(cfg::generationlimit))+"");
        HCOMPRINT(output);
    }
    if(all || pref == "samples")
    {
        if(!mpModel && !all)
        {
            HCOMERR("Setting is model-specific, but no model is open");
            return;
        }
        if(mpModel)
        {
            QString output = "Number of log samples:     ";
            output.append(QString::number(mpModel->getViewContainerObject()->getNumberOfLogSamples())+"");
            HCOMPRINT(output);
        }
    }
    if(all || pref == "undo")
    {
        if(!mpModel && !all)
        {
            HCOMERR("Setting is model-specific, but no model is open");
            return;
        }
        if(mpModel)
        {
            QString output = "Undo history:              ";
            if(mpModel->getViewContainerObject()->isUndoEnabled())
                output.append("ON");
            else
                output.append("OFF");
            HCOMPRINT(output);
        }
    }
    if(all || pref == "backup")
    {
        QString output = "Auto-backup:               ";
        if(getConfigPtr()->getBoolSetting(cfg::autobackup))
            output.append("ON");
        else
            output.append("OFF");
        HCOMPRINT(output);
    }
    if(all || pref == "progressbar")
    {
        QString output = "Progres bar:               ";
        if(getConfigPtr()->getBoolSetting(cfg::progressbar))
            output.append("ON");
        else
            output.append("OFF");
        HCOMPRINT(output);
    }
    if(all || pref == "progressbarstep")
    {
        QString output = "Progress bar step size:    ";
        output.append(QString::number(getConfigPtr()->getIntegerSetting(cfg::progressbarstep))+"");
        HCOMPRINT(output);
    }
    if(all || pref == "logduringsimulation")
    {
        QString output = "Collect log data during simulation:               ";
        if(getConfigPtr()->getBoolSetting(cfg::logduringsimulation))
            output.append("ON");
        else
            output.append("OFF");
        HCOMPRINT(output);
    }
}


//! @brief Execute function for "sapl" command
void HcomHandler::executeSaveToPloCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() < 2)
    {
        HCOMERR("Too few arguments.");
        return;
    }

    QString path = args.first(); args.removeFirst();
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }

    QString dir, fname, format;
    splitFilepath(path, dir, fname, format);
    format = format.toLower();

    // Check if mwd or pwd should be used with dir
    dir = getDirectory(dir);
    path = dir+fname;

    // Determine format from file extension unless override in arg
    if(args.contains("-csv"))
    {
        format = "csv";
        args.removeFirst();
    }
    else if (args.contains("-h5"))
    {
        format = "h5";
        args.removeFirst();
    }
    else if (args.contains("-plo"))
    {
        format="plo";
        args.removeFirst();
    }

    if (!args.isEmpty())
    {
        // Handle special case where we want to export a specific generation only
        // Example: *.g (g=15, g=l, g=h)
        QString specialCase = args.last();
        if ( (args.size() == 1) && specialCase.startsWith(QString("*%1").arg(generationspecifier::separatorString) ) )
        {
            bool parseOK;
            int g=parseAndChopGenerationSpecifier(specialCase, parseOK);
            if (parseOK)
            {
                if (isSingleGenerationValue(g))
                {
                    if (g == generationspecifier::currentGeneration)
                    {
                        g = mpModel->getLogDataHandler()->getCurrentGenerationNumber();
                    }

                    if (mpModel->getLogDataHandler()->hasGeneration(g))
                    {
                        if (format == "plo")
                        {
                            mpModel->getLogDataHandler()->exportGenerationToPlo(path, g);
                        }
                        else if (format == "csv")
                        {
                            mpModel->getLogDataHandler()->exportGenerationToCSV(path, g);
                        }
                        else if (format == "h5")
                        {
                            mpModel->getLogDataHandler()->exportGenerationToHDF5(path, g);
                        }
                    }
                    else
                    {
                        HCOMERR(QString("Generation %1 does not exist").arg(g+1));
                    }
                    return;
                }
                else if(g == generationspecifier::allGenerations)
                {
                    HCOMERR("sapl can not export different full generations to the same file");
                    return;
                }
                else
                {
                    HCOMERR("Generation could not be parsed or does not exist");
                    return;
                }
            }
            else
            {
                HCOMERR("Could not parse generation specifier");
                return;
            }
        }

        // If we get here we should try to read all variable patterns explicitly
        QList<SharedVectorVariableT> allVariables;
        for(QString &arg : args)
        {
            arg.remove("\"");
            if (arg.startsWith(QString("*%1").arg(generationspecifier::separatorString)))
            {
                HCOMWARN("Ignoring "+arg+" you can only use this as a single argument to sapl");
            }
            else
            {
                QStringList variables;
                getMatchingLogVariableNames(arg, variables);
                if (variables.isEmpty())
                {
                    HCOMERR("No matching variables found: "+arg);
                }
                for(QString &var : variables)
                {
                    allVariables.append(getLogVariable(var));
                }
            }
        }

        if (allVariables.isEmpty())
        {
            HCOMERR("No matching variables found");
        }
        else
        {
            if (format == "plo")
            {
                mpModel->getLogDataHandler()->exportToPlo(path, allVariables);
            }
            else if (format == "csv")
            {
                mpModel->getLogDataHandler()->exportToCSV(path, allVariables);
            }
            else if (format == "h5")
            {
                mpModel->getLogDataHandler()->exportToHDF5(path, allVariables);
            }
        }
    }
}

void HcomHandler::executeLoadVariableCommand(const QString cmd)
{
    if(!mpModel) {
        HCOMERR("Loading variables from file requires an open model.");
        return;
    }
    QStringList args = splitCommandArguments(cmd);
    if(args.size() < 1 || args.size() > 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString flagarg,path;
    if (args.size() > 1)
    {
        flagarg = args.first();
        path = args.last();
    }
    else
    {
        path = args.first();
    }

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

    bool csv,ssv,plo;
    csv=(flagarg=="-csv");
    ssv=(flagarg=="-ssv");
    plo=(flagarg=="-plo");

    if( flagarg.isEmpty() && (path.endsWith(".csv") || path.endsWith(".CSV")) )
    {
        csv=true;
    }
    else if(flagarg.isEmpty() && (path.endsWith(".plo") || path.endsWith(".PLO")) )
    {
        plo=true;
    }
    else if (flagarg.isEmpty())
    {
        HCOMWARN("Unknown file extension, assuming that it is a PLO file.");
        plo=true;
    }

    if(csv)
    {
        mpModel->getViewContainerObject()->getLogDataHandler()->importFromCSV_AutoFormat(path);
    }
    else if (plo)
    {
        mpModel->getViewContainerObject()->getLogDataHandler()->importFromPlo(path);
    }
    else if (ssv)
    {
        mpModel->getLogDataHandler()->importFromPlainColumnCsv(path,' ');
    }
    else
    {
        HCOMERR("Incorrect format");
    }
}

void HcomHandler::executeSaveParametersCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() < 1 || args.size() > 2)
    {
        HCOMERR("Wrong number of arguments");
        return;
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

    if (args.size() > 1) {
        if (args.last() == "-c") {
            auto pSystem = qobject_cast<SystemObject*>(mpModel->getViewContainerObject());
            if (pSystem) {
                pSystem->saveParameterValuesToFile(path);
            }
            else {
                HCOMERR("Current view is not a system");
            }
        }
        else {
            QString sysOrCompFullName = args.last();
            toLongDataNames(sysOrCompFullName);
            QStringList systemHierarchy;
            QString sysOrCompName;
            splitFullComponentName(sysOrCompFullName, systemHierarchy, sysOrCompName);
            auto pSystem = qobject_cast<SystemObject*>(mpModel->getViewContainerObject());
            pSystem = searchIntoSubsystem(pSystem, systemHierarchy);
            if (pSystem) {
                ModelObject* pModelObject = pSystem->getModelObject(sysOrCompName);
                if (pModelObject) {
                    pModelObject->saveParameterValuesToFile(path);
                }
                else {
                    HCOMERR("No such component found");
                }
            }
        }
    }
    else {
        mpModel->saveTo(path, ParametersOnly);
    }
}

void HcomHandler::executeLoadParametersCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() < 1 || args.size() > 2)
    {
        HCOMERR("Wrong number of arguments");
        return;
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

    if (args.size() > 1) {
        if (args.last() == "-c") {
            auto pSystem = qobject_cast<SystemObject*>(mpModel->getViewContainerObject());
            if (pSystem) {
                pSystem->loadParameterValuesFromFile(path);
            }
            else {
                HCOMERR("Current view is not a system");
            }
        }
        else {
            QString sysOrCompFullName = args.last();
            toLongDataNames(sysOrCompFullName);
            QStringList systemHierarchy;
            QString sysOrCompName;
            splitFullComponentName(sysOrCompFullName, systemHierarchy, sysOrCompName);
            auto pSystem = qobject_cast<SystemObject*>(mpModel->getViewContainerObject());
            pSystem = searchIntoSubsystem(pSystem, systemHierarchy);
            if (pSystem) {
                ModelObject* pModelObject = pSystem->getModelObject(sysOrCompName);
                if (pModelObject) {
                    pModelObject->loadParameterValuesFromFile(path);
                }
                else {
                    HCOMERR("No such component found");
                }
            }
        }
    }
    else {
       mpModel->importModelParametersFromHpf(path);
    }
}


//! @brief Execute function for "load" command
void HcomHandler::executeLoadModelCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 1)
    {
        HCOMERR("Wrong numer of arguments.");
        return;
    }

    QStringList contents = QDir(mPwd).entryList(QStringList() << cmd);


    QString path = args[0];
    path.remove("\"");
    if(!path.contains("/"))
    {
        path.prepend("./");
    }
    QString dir = path.left(path.lastIndexOf("/"));
    dir = getDirectory(dir);
    QString wildcard = path.right(path.size()-path.lastIndexOf("/")-1);
    QStringList files = QDir(dir).entryList(QStringList() << wildcard,QDir::Files);

    for(const QString &file : files) {
        path = dir+"/"+file;
        gpModelHandler->loadModel(path);
    }
}

//! @brief Execute revert model command
void HcomHandler::executeRevertModelCommand(const QString cmd)
{
    Q_UNUSED(cmd)
    if (mpModel)
    {
        if (mpModel == gpModelHandler->getCurrentModel())
        {
            mpModel->mpParentModelHandler->revertCurrentModel();
        }
        else
        {
            mpModel->revertModel();
        }
    }
}


//! @brief Execute function for "loadr" command
void HcomHandler::executeLoadRecentCommand(const QString cmd)
{
    Q_UNUSED(cmd)
    gpModelHandler->loadModel(getConfigPtr()->getRecentModels().first());
}


//! @brief Execute function for "save" command
void HcomHandler::executeSaveModelCommand(const QString cmd)
{
    QString modelPath;
    QStringList args = splitCommandArguments(cmd);
    if(args.size() == 1)
    {
        modelPath = args[0];
        modelPath.remove("\"");
        modelPath.replace("\\","/");
        if(!modelPath.contains("/"))
        {
            modelPath.prepend("./");
        }

        QString dir = modelPath.left(modelPath.lastIndexOf("/"));
        dir = getDirectory(dir);
        if(dir.isEmpty())
        {
            HCOMERR("Illegal file path.");
            return;
        }
        modelPath = dir+modelPath.right(modelPath.size()-modelPath.lastIndexOf("/"));
    }
    else if(args.isEmpty())
    {
        modelPath = getModelPtr()->getTopLevelSystemContainer()->getModelFilePath();
        if(modelPath.isEmpty())
        {
            HCOMERR("Model is not saved yet. Please specify a file path.");
            return;
        }
    }
    else
    {
        HCOMERR("Wrong numer of arguments.");
        return;
    }

    getModelPtr()->saveTo(modelPath, FullModel);
    getModelPtr()->setSaved(true);
    getModelPtr()->getTopLevelSystemContainer()->setModelFile(modelPath);
    getModelPtr()->getTopLevelSystemContainer()->setName(QFileInfo(modelPath).baseName());
    gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(getModelPtr()), QFileInfo(modelPath).baseName());
}


//! @brief Execute function for "reco" command
void HcomHandler::executeRenameComponentCommand(const QString cmd)
{
    QStringList split;
    splitWithRespectToQuotations(cmd, ' ', split);
    if(split.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    if(mpModel->isEditingLimited()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    SystemObject *pContainer = mpModel->getViewContainerObject();
    if(pContainer)
    {
        pContainer->renameModelObject(split[0], split[1]);
    }
}

void HcomHandler::executeRemoveComponentCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    if(mpModel->isEditingLimited()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    QList<ModelObject*> components;
    getComponents(args[0], components);

    SystemObject *pContainer = mpModel->getViewContainerObject();
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
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    //Handle "cd -mwd" command
    if(cmd == "-mwd")
    {
        if(mpModel)
        {
            mPwd = mpModel->getTopLevelSystemContainer()->getModelFileInfo().absoluteDir().path();
            HCOMPRINT(mPwd);
        }
        return;
    }

    QString path = cmd;
    path.remove("\"");
    path.replace("\\","/");
    QDir newDirAbs(path);
    QDir newDirRel(mPwd+"/"+path);

    if(newDirAbs.isAbsolute() && newDirAbs.exists())
    {
        mPwd = newDirAbs.canonicalPath();
    }
    else if(newDirRel.exists())
    {
        mPwd = newDirRel.canonicalPath();
    }
    else
    {
        HCOMERR("Illegal directory.");
        return;
    }

    HCOMPRINT(mPwd);
}

void HcomHandler::executeMakeDirectoryCommand(const QString cmd)
{
    QFileInfo fi(cmd);
    if (!fi.isAbsolute())
    {
        fi.setFile(mPwd+"/"+fi.filePath());
    }
    if (!fi.exists())
    {
        HCOMINFO("Creating: "+fi.absoluteFilePath());
        QDir d;
        bool rc = d.mkpath(fi.absoluteFilePath());
        if (!rc)
        {
            HCOMERR("Failed to create: "+fi.absoluteFilePath());
        }
    }
}


//! @brief Execute function for "ls" command
void HcomHandler::executeListFilesCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() > 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    QStringList contents;
    if(args.isEmpty())
    {
        contents = QDir(mPwd).entryList(QStringList() << "*");
    }
    else
    {
        contents = QDir(mPwd).entryList(QStringList() << args[0]);
    }
    for(int c=0; c<contents.size(); ++c)
    {
        HCOMPRINT(contents[c]);
    }
}


//! @brief Execute function for "close" command
void HcomHandler::executeCloseModelCommand(const QString cmd)
{
    if(mpModel->isEditingLimited()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    QStringList args = splitCommandArguments(cmd);
    if(args.size() > 2)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }
    bool force=false;
    bool all=false;
    for(const QString &arg : args) {
        if(arg == "all") {
            all = true;
        }
        else if(arg == "-f") {
            force = true;
        }
        else {
            HCOMERR("Unknown argument: "+args[0]);
            return;
        }
    }

    if(all)
    {
        gpModelHandler->closeAllModels(force);
    }
    else
    {
        if(gpModelHandler->count() > 0 && gpModelHandler->getCurrentModel() != 0)
        {
            gpModelHandler->closeModelByTabIndex(gpCentralTabWidget->currentIndex(),force);
        }
    }
}


//! @brief Execute function for "chtab" command
void HcomHandler::executeChangeTabCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    gpModelHandler->setCurrentModel(cmd.toInt());
}


//! @brief Execute function for "adco" command
void HcomHandler::executeAddComponentCommand(const QString cmd)
{
    if(!mpModel || getNumberOfCommandArguments(cmd) < 5)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    if(mpModel->isEditingLimited()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    QStringList args = splitCommandArguments(cmd);

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

            evaluateExpression(args[1]);
            if(mAnsType == Scalar)
            {
                xPos = mAnsScalar;
            }
            else
            {
                HCOMERR("Argument is not a scalar.");
                return;
            }

            evaluateExpression(args[2]);
            if(mAnsType == Scalar)
            {
                yPos = mAnsScalar;
            }
            else
            {
                HCOMERR("Argument is not a scalar.");
                return;
            }

            evaluateExpression(args[3]);
            if(mAnsType == Scalar)
            {
                rot = mAnsScalar;
            }
            else
            {
                HCOMERR("Argument is not a scalar.");
                return;
            }
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
            Component *pOther = qobject_cast<Component*>(mpModel->getViewContainerObject()->getModelObject(otherName));
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
            Component *pOther = qobject_cast<Component*>(mpModel->getViewContainerObject()->getModelObject(otherName));
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
            Component *pOther = qobject_cast<Component*>(mpModel->getViewContainerObject()->getModelObject(otherName));
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
            Component *pOther = qobject_cast<Component*>(mpModel->getViewContainerObject()->getModelObject(otherName));
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
    ModelObject *pObj = mpModel->getViewContainerObject()->addModelObject(typeName, pos, rot);
    if(!pObj)
    {
        HCOMERR("Failed to add new component. Incorrect typename?");
    }
    else
    {
        HCOMPRINT("Added "+typeName+" to current model.");
        mpModel->getViewContainerObject()->renameModelObject(pObj->getName(), name);
    }

    // Return the index of the new component
    mAnsScalar = mpModel->getViewContainerObject()->getModelObjects().size()-1;
    mAnsType = Scalar;
}

void HcomHandler::executeReplaceComponentCommand(const QString cmd)
{
    if(!mpModel || getNumberOfCommandArguments(cmd) != 2)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    if(mpModel->isEditingLimited()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    QStringList args = splitCommandArguments(cmd);

    QString name = args[0];
    QString typeName = args[1];

    mpModel->getTopLevelSystemContainer()->replaceComponent(name, typeName);
}


//! @brief Execute function for "coco" command
void HcomHandler::executeConnectCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 4)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }

    if(mpModel->isEditingLimited()) {
        HCOMERR("Model editing is currently disabled");
        return;
    }

    ModelObject *pComponent1, *pComponent2;
    pComponent1 = mpModel->getViewContainerObject()->getModelObject(args[0]);
    if(pComponent1 == nullptr) {
        HCOMERR("Cannot find component \""+args[0]+"\"");
        return;
    }
    pComponent2 = mpModel->getViewContainerObject()->getModelObject(args[2]);
    if(pComponent2 == nullptr) {
        HCOMERR("Cannot find component \""+args[2]+"\"");
        return;
    }
    Port *pPort1 = pComponent1->getPort(args[1]);
    if(pPort1 == nullptr) {
        HCOMERR("Cannot find port \""+args[1]+"\" in component \""+args[0]+"\"");
        return;
    }
    Port *pPort2 = pComponent2->getPort(args[3]);
    if(pPort1 == nullptr) {
        HCOMERR("Cannot find port \""+args[3]+"\" in component \""+args[2]+"\"");
        return;
    }

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


//! @brief Execute function for "unco" command
void HcomHandler::executeListUnconnectedCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    QString wildcard = "*";
    if(args.size() == 1)
    {
        wildcard = args[0];
    }
    else if(args.size() > 1)
    {
        HCOMERR("Wrong number of arguments, should  be 0 or 1");
        return;
    }

    QList<Port*> ports;
    getPorts(wildcard,ports);

    for(Port* pPort : ports)
    {
        if(!pPort->isConnected())
        {
            HCOMPRINT(pPort->getParentModelObjectName()+"."+pPort->getName());
        }
    }
}


//! @brief Execute function for "crmo" command
void HcomHandler::executeCreateModelCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 0)
    {
        HCOMERR("Wrong number of arguments");
        return;
    }
    gpModelHandler->addNewModel();
}


//! @brief Execute function for "fmu" command
void HcomHandler::executeExportToFMUCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 3)
    {
        HCOMERR("Wrong number of arguments.");
    }

    bool me;
    if(args[1] == "me")
    {
        me = true;
    }
    else if(args[1] == "cs")
    {
        me = false;
    }
    else
    {
        HCOMERR("Unknown FMU kind. Only \"me\" or \"cs\" is accepted.");
        return;
    }

    ArchitectureEnumT arch;
    if(args[2] == "32")
    {
        arch = ArchitectureEnumT::x86;
    }
    else if(args[2] == "64")
    {
        arch = ArchitectureEnumT::x64;
    }
    else
    {
        HCOMERR("Unknown architecture. Only \"32\" or \"64\" is accepted.");
        return;
    }

    mpModel->getTopLevelSystemContainer()->exportToFMU(args[0], me, arch);
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
    QStringList args = splitCommandArguments(cmd);
    if(args.size() < 2 || args.size() > 6)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString var1 = args[0];
    QString var2 = args[1];
    SharedVectorVariableT pData1 = getLogVariable(var1);
    SharedVectorVariableT pData2 = getLogVariable(var2);
    if(!pData1 || !pData2)
    {
        HCOMERR("Data variable not found.");
        return;
    }
    int fMax = 500*2*M_PI;
    if(args.size() > 2)
    {
        fMax = args[2].toInt()/(2*M_PI); //!< @todo parse check needed
    }

    WindowingFunctionEnumT windowType = RectangularWindow;
    if(args.size() > 3) {
        if(args[3].toLower() == "hann") {
            windowType = HannWindow;
        }
        else if(args[3].toLower() == "rectangular") {
            windowType = RectangularWindow;
        }
        else if(args[3].toLower() == "flattop") {
            windowType = FlatTopWindow;
        }
        else {
            HCOMERR("Unknown window function type: " + args[3]);
            return;
        }
    }

    if(args.size() == 5) {
        HCOMERR("If minimum time is specified, maximum time is also required.");
        return;
    }

    double minTime = -std::numeric_limits<double>::max();
    double maxTime = std::numeric_limits<double>::max();
    if(args.size() > 5) {
        bool ok;
        minTime = args[4].toDouble(&ok);
        if(!ok) {
            HCOMERR("Unknown minimum time value: " + args[4]);
            return;
        }
        maxTime = args[5].toDouble(&ok);
        if(!ok) {
            HCOMERR("Unknown maximum time value: " + args[5]);
            return;
        }
    }

    PlotWindow *pWindow = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Bode plot");
    pWindow->closeAllTabs();
    pWindow->createBodePlot(pData1, pData2, fMax, true, false, windowType, minTime, maxTime);
}


//! @brief Execute function for "nyquist" command
void HcomHandler::executeNyquistCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QString var1 = args[0];
    QString var2 = args[1];
    SharedVectorVariableT pData1 = getLogVariable(var1);
    SharedVectorVariableT pData2 = getLogVariable(var2);
    if(!pData1 || !pData2)
    {
        HCOMERR("Data variable not found.");
        return;
    }

    PlotWindow *pWindow = gpPlotHandler->createNewPlotWindowOrGetCurrentOne("Nyquist plot");
    pWindow->closeAllTabs();
    pWindow->createBodePlot(pData1, pData2, 0, false, true);
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
    QStringList split = splitCommandArguments(cmd);

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

            mpOptHandler->setCandidateObjectiveValue(idx, val);
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

            mpOptHandler->setParameterLimits(optParIdx, min, max);
            return;
        }
        else if(split.size() == 3 && split[1] == "output")
        {
            if(split[2] == "on" || split[2] == "off")
            {
                gpOptimizationDialog->setOutputDisabled(split[2] == "off");
            }
            else
            {
                HCOMERR("Unknown argument: "+split[2]);
            }
        }
        else if(split.size() == 5 && split[1] == "startvalue")
        {
            bool ok;
            int pointId = getNumber(split[2], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 2 must be a number.");
                return;
            }
            int parId = getNumber(split[3], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 3 must be a number.");
                return;
            }
            double value = getNumber(split[4], &ok);
            if(!ok)
            {
                HCOMERR("Argument number 4 must be a number.");
                return;
            }
            mpOptHandler->setStartValue(pointId, parId, value);
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
        QDir().mkpath(gpDesktopHandler->getDataPath()+"optimization/");
        QString savePath = gpDesktopHandler->getDataPath()+"optimization/"+name+".hmf";
        mpModel->saveTo(savePath);
        mpModel->getTopLevelSystemContainer()->setAppearanceDataBasePath(appearanceDataBasePath);

        mpOptHandler->startOptimization(mpModel, savePath);
    }
}


//! @brief Execute function for "call" command
void HcomHandler::executeCallFunctionCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
    if(args.size() != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    QString funcName = args[0];

    if(!mFunctions.contains(funcName))
    {
        HCOMERR("Undefined function: "+funcName);
        return;
    }

    //TicToc timer;
    //timer.tic(" >>>>>>>>>>>>> In executeCallFunctionCommand: Starting runScriptCommands for: "+cmd+" func: "+funcName);

    bool abort = false;
    runScriptCommands(mFunctions.find(funcName).value(), &abort);
    //timer.toc(" <<<<<<<<<<<<< In executeCallFunctionCommand: Finished runScriptCommands for: "+cmd+" func: "+funcName);
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

    QStringList args = splitCommandArguments(cmd);
    if(args.size() > 2)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    const QString &arg = args[0];
    bool ignoreErrors = false;
    if(args.size() > 1)
    {
        ignoreErrors = (args[1] == "-nonerrors");
    }


    if(arg == "on")
    {
        mpConsole->setDontPrint(false, ignoreErrors);
    }
    else if(arg == "off")
    {
        mpConsole->setDontPrint(true, ignoreErrors);
    }
    else
    {
        HCOMERR("Unknown argument, use \"on\" or \"off\"");
    }
}

void HcomHandler::executeClearCommand(const QString cmd)
{
    Q_UNUSED(cmd);
    //mpConsole->clear();
    qobject_cast<QTextEdit*>(mpConsole)->clear();
}


//! @brief Execute function for "edit" command
void HcomHandler::executeEditCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }

    QStringList args;
    splitWithRespectToQuotations(cmd, ' ', args);
    QString path = args[0];
    path.remove("\"");
    path.prepend(mPwd+"/");

    QString suffix = QFileInfo(path).suffix();
    QStringList textFileSuffixes = QStringList() << "hcom" << "hpp" << "cpp" << "h" << "c" << "cc" << "cci" << "xml" << "plo" << "csv" << "txt";
    if(textFileSuffixes.contains(suffix)) {
        gpModelHandler->loadTextFile(path);
    }
    else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}


//! @brief Execute function for "semt" command
void HcomHandler::executeSetMultiThreadingCommand(const QString cmd)
{
    QStringList args = splitCommandArguments(cmd);
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

    getConfigPtr()->setBoolSetting(cfg::multicore, useMultiThreading);
    if(nArgs > 1) getConfigPtr()->setIntegerSetting(cfg::numberofthreads, nThreads);
    if(nArgs > 2) getConfigPtr()->setParallelAlgorithm(algorithm);
}


//! @brief Execute function for "lock" command
void HcomHandler::executeLockAllAxesCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    else if(cmd != "on" && cmd != "off")
    {
        HCOMERR("Unknown argument. Use \"on\" or \"off\".");
        return;
    }
    mpCurrentPlotWindow->getCurrentPlotTab()->getPlotArea()->setAxisLocked(QwtPlot::yLeft, cmd=="on");
    mpCurrentPlotWindow->getCurrentPlotTab()->getPlotArea()->setAxisLocked(QwtPlot::yRight, cmd=="on");
    mpCurrentPlotWindow->getCurrentPlotTab()->getPlotArea()->setAxisLocked(QwtPlot::xBottom, cmd=="on");
}


//! @brief Execute function for "lockyl" command
void HcomHandler::executeLockLeftAxisCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    else if(cmd != "on" && cmd != "off")
    {
        HCOMERR("Unknown argument. Use \"on\" or \"off\".");
        return;
    }
    mpCurrentPlotWindow->getCurrentPlotTab()->getPlotArea()->setAxisLocked(QwtPlot::yLeft, cmd=="on");
}


//! @brief Execute function for "lockyr" command
void HcomHandler::executeLockRightAxisCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    else if(cmd != "on" && cmd != "off")
    {
        HCOMERR("Unknown argument. Use \"on\" or \"off\".");
        return;
    }
    mpCurrentPlotWindow->getCurrentPlotTab()->getPlotArea()->setAxisLocked(QwtPlot::yRight, cmd=="on");
}


//! @brief Execute function for "lockx" command
void HcomHandler::executeLockXAxisCommand(const QString cmd)
{
    if(getNumberOfCommandArguments(cmd) != 1)
    {
        HCOMERR("Wrong number of arguments.");
        return;
    }
    else if(cmd != "on" && cmd != "off")
    {
        HCOMERR("Unknown argument. Use \"on\" or \"off\".");
        return;
    }
    mpCurrentPlotWindow->getCurrentPlotTab()->getPlotArea()->setAxisLocked(QwtPlot::xBottom, cmd=="on");
}

void HcomHandler::executeSleepCommand(const QString cmd)
{
    if (!cmd.isEmpty())
    {
        bool ok;
        double s = cmd.toDouble(&ok);
        if (ok)
        {
            QElapsedTimer timer;
            timer.start();
            HCOMINFO("Sleeping for: "+cmd+" seconds");
            while ( timer.elapsed() < qint64(s*1000) )
            {
                qApp->processEvents();
            }
        }
        else
        {
            HCOMERR("Could not interpret: '"+cmd+"' as a number");
        }
    }
    else
    {
        HCOMERR("sleep requires at least one argument (the number of seconds to sleep)");
    }
}


//! @brief Changes plot variables on specified axes
//! @param cmd Command containing the plot variables
//! @param axis Axis specification (0=left, 1=right, -1=both, separated by "-r")
void HcomHandler::changePlotVariables(const QString cmd, const int axis, bool hold)
{
    QStringList varNames = splitCommandArguments(cmd);

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
            PlotCurveStyle style;
            extractCurveStyle(varNames[s], style);

            // Check if no generation is given, then specify "current" to avoid costly lookup in all generations
            QString tempVarName = varNames[s];
            bool parseOK;
            int desiredGen = parseAndChopGenerationSpecifier(tempVarName, parseOK);
            if (isValidGenerationValue(desiredGen)) {
                getMatchingLogVariableNames(varNames[s], variables);
            }
            else if (mpModel && mpModel->getLogDataHandler()) {
                getMatchingLogVariableNames(varNames[s], variables, false, mpModel->getLogDataHandler()->getCurrentGenerationNumber());
            }

            if (variables.isEmpty())
            {
                evaluateExpression(varNames[s], DataVector);
                if(mAnsType == DataVector)
                {
                    addPlotCurve(mAnsVector, axisId, true, style);
                    found = true;
                }
            }
            else
            {
                found = true;
                for(auto &varname : variables)
                {
                    addPlotCurve(varname, axisId, style);
                }
            }

            if (!found)
            {
                HCOMERR(QString("Could not find variable or evaluate expression: %1").arg(varNames[s]));
            }
        }
    }
}

void HcomHandler::changePlotXVariable(const QString varExp)
{
    // Reset
    if (varExp == "-c")
    {
        // If mpCurrentPlotWindow is 0, then we will set it to the window that is actually created
        // else we will just set to same
        mpCurrentPlotWindow = gpPlotHandler->setPlotWindowXData(mpCurrentPlotWindow, SharedVectorVariableT(), true);
    }
    // Else set new
    else
    {
        bool found=false;
        QStringList variables;

        // Check if no generation is given, then specify "current" to avoid costly lookup in all generations
        QString tempVarName = varExp;
        bool parseOK;
        int desiredGen = parseAndChopGenerationSpecifier(tempVarName, parseOK);
        if (isValidGenerationValue(desiredGen)) {
            getMatchingLogVariableNames(varExp, variables);
        }
        else if (mpModel && mpModel->getLogDataHandler()) {
            getMatchingLogVariableNames(varExp, variables, false, mpModel->getLogDataHandler()->getCurrentGenerationNumber());
        }

        if (variables.isEmpty())
        {
            evaluateExpression(varExp, DataVector);
            if(mAnsType == DataVector)
            {
                // If mpCurrentPlotWindow is 0, then we will set it to the window that is actually created
                // else we will just set to same
                mpCurrentPlotWindow = gpPlotHandler->setPlotWindowXData(mpCurrentPlotWindow, mAnsVector, true);
                found = true;
            }
        }
        else
        {
            if (variables.size() > 1)
            {
                HCOMWARN(QString("Multiple matches to xdata pattern: %1; %2").arg(varExp).arg(variables.join(", ")));
            }
            // If mpCurrentPlotWindow is 0, then we will set it to the window that is actually created
            // else we will just set to same
            mpCurrentPlotWindow = gpPlotHandler->setPlotWindowXData(mpCurrentPlotWindow, getLogVariable(variables.first()), true);
            found = true;
        }

        if (!found)
        {
            HCOMERR(QString("Could not find xdata variable or evaluate expression: %1").arg(varExp));
        }
    }
}


//! @brief Adds a plot curve to specified axis in current plot
//! @param cmd Name of variable
//! @param axis Axis to add curve to
void HcomHandler::addPlotCurve(QString var, const int axis, PlotCurveStyle style)
{
    SharedVectorVariableT data = getLogVariable(var);
    if(!data)
    {
        HCOMERR(QString("Variable not found: %1").arg(var));
        return;
    }
    else
    {
        // If plot curve contains gen specifier, then we want that generation to remain in the plot and not auto refresh
        if (var.contains(generationspecifier::separatorChar))
        {
            addPlotCurve(data, axis, false, style);
        }
        else
        {
            addPlotCurve(data, axis, true, style);
        }
    }
}

void HcomHandler::addPlotCurve(SharedVectorVariableT data, const int axis, bool autoRefresh, PlotCurveStyle style)
{
    // If mpCurrentPlotWindow is 0, then we will set it to the window that is actually created
    // else we will just set to same
    mpCurrentPlotWindow = gpPlotHandler->plotDataToWindow(mpCurrentPlotWindow, data, axis, autoRefresh,style);
    mpCurrentPlotWindow->raise();

    if(qApp->activeWindow() || qApp->focusWidget())
    {
        gpMainWindowWidget->activateWindow();
    }
}


//! @brief Adds a plot curve to specified axis in current plot
//! @param [in] fullShortVarNameWithGen Full short name of variable to remove (including optional generation specifier)
void HcomHandler::removeLogVariable(QString fullShortVarNameWithGen) const
{
    if(!mpModel || !mpModel->getLogDataHandler())
        return;

    bool parseOK;

    QString name = fullShortVarNameWithGen;
    int generation = parseAndChopGenerationSpecifier(name, parseOK);
    if (generation == generationspecifier::allGenerations || generation == generationspecifier::noGenerationSpecified)
    {
        // Remove all if 'all' or 'no' returned (all if no gen specified)
        generation = generationspecifier::allGenerations;
    }

    if (parseOK)
    {
        toLongDataNames(name);

        bool rc = mpModel->getLogDataHandler()->removeVariable(name, generation);
        if (!rc)
        {
            HCOMERR(QString("Variable %1 could not be deleted").arg(fullShortVarNameWithGen));
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

void HcomHandler::extractCurveStyle(QString &value, PlotCurveStyle &style)
{
    if(!value.contains("{"))
    {
        style.line_style = Qt::SolidLine;
        style.symbol = QwtSymbol::NoSymbol;
        style.color = QColor();
        style.thickness = 2;
        return;
    }

    QString styleStr = value.section("{",1,1).section("}",0,0);
    value.remove("{"+styleStr+"}");
    qDebug() << "Style: " << styleStr;

    QString typeStr= styleStr.section(",",0,0);
    if(typeStr == "solid")
        style.line_style = Qt::SolidLine;
    else if(typeStr == "dashed")
        style.line_style = Qt::DashLine;
    else if(typeStr == "dotted")
        style.line_style = Qt::DotLine;
    else
    {
        bool is_ok;
        const int val = typeStr.toInt(&is_ok);;
        if (is_ok) {
            style.line_style = static_cast<Qt::PenStyle>(val);
        }
    }


    QString colorStr = styleStr.section(",",1,1);
    style.color = QColor(colorStr);

    QString thicknessStr = styleStr.section(",",2,2);
    style.thickness = thicknessStr.toInt();

    style.symbol = QwtSymbol::NoSymbol;
    if(styleStr.count(",") >= 3)
    {
        QString symbolStr = styleStr.section(",",3,3);
        if (isNumber(symbolStr)) {
            bool parse_ok;
            int index = symbolStr.toInt(&parse_ok);
            if (parse_ok && index < PlotCurveStyle::symbol_enums.size()) {
                // Use corresponding value
                style.symbol = PlotCurveStyle::symbol_enums[index];
                //! @todo We should not use index (order) value in HCOM, we should use corresponding QwtSymbol::Style value directly
            }
        }
        else {
            style.symbol = PlotCurveStyle::toSymbolEnum(symbolStr);
        }
    }
}

void HcomHandler::evaluateExpression(QString expr, VariableType desiredType)
{
    //TicToc timer;

    //Check that parentheses are ok, else give error message
    if(!SymHop::Expression::verifyParantheses(expr))
    {
        HCOMERR("Syntax error in expression.");
        return;
    }


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

    if(desiredType != DataVector && desiredType != Expression)
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

    if(desiredType != Scalar && desiredType != Expression)
    {
        //Data variable, return it
        SharedVectorVariableT data = getLogVariable(expr);
        if(data)
        {
            mAnsType = DataVector;
            mAnsVector = data;
            return;
        }
    }

    if(desiredType != Scalar && desiredType != DataVector) {
        //Expression variable, return it
        if(mLocalExpressions.contains(expr)) {
            mAnsType = Expression;
            mAnsExpression = mLocalExpressions[expr];
            return;
        }
    }

    if(desiredType != DataVector && mpModel)
    {
        // Parameter name, return its value
        QString parType;

        SystemObject *pContainer = mpModel->getViewContainerObject();

        QString fullName = expr;
        while(!pContainer->isTopLevelContainer())
        {
            fullName.prepend("$");
            fullName.prepend(pContainer->getName());
            pContainer = pContainer->getParentSystemObject();
        }

        bool ok;
        QString parVal;

        double orgAnsScalar = mAnsScalar;
        while(true)
        {
            parVal = getParameterValue(fullName, parType, true);

            if(parType == "string")
            {
                mAnsType = String;
                mAnsWildcard = parVal;
                return;
            }

            mAnsScalar = parVal.toDouble(&ok);

            if(!ok)
            {
                if(fullName.count("$") < 1 && parVal != "NaN" && parVal != "nan")
                {
                    evaluateExpression(parVal);
                    return;
                }
                else if(fullName.contains("$"))
                {
                  //Find container where variable is located
                  if(fullName.section("$",-1,-1).contains("."))
                  {
                      //Subcomponent parameter
                      fullName = fullName.section("$",0,-2)+"$"+parVal;
                  }
                  else
                  {
                      //System parameter
                      fullName = fullName.section("$",0,-3)+"$"+parVal;
                  }
                }
            }
            else if(parVal != "NaN" && parVal != "nan")
            {
                mAnsType = Scalar;
                return;
            }
            else
            {
                mAnsScalar = orgAnsScalar;
                mAnsType = Scalar;
                break;
            }
        }
    }

    // Vector functions
    //timer.tic();
    LogDataHandler2 *pLogDataHandler=0;
    if(mpModel && mpModel->getViewContainerObject())
    {
        pLogDataHandler = mpModel->getViewContainerObject()->getLogDataHandler().data();
    }

    //Simple mathematical vector functions with no arguments
    FuncMap_t funcMap;
    funcMap.insert("sin", static_cast<ScalarMathFunction_t>(sin));
    funcMap.insert("cos", static_cast<ScalarMathFunction_t>(cos));
    funcMap.insert("tan", static_cast<ScalarMathFunction_t>(tan));
    funcMap.insert("asin", static_cast<ScalarMathFunction_t>(asin));
    funcMap.insert("acos", static_cast<ScalarMathFunction_t>(acos));
    funcMap.insert("atan", static_cast<ScalarMathFunction_t>(atan));
    funcMap.insert("log", static_cast<ScalarMathFunction_t>(log));
    funcMap.insert("log2", static_cast<ScalarMathFunction_t>(log2));
    funcMap.insert("log10", static_cast<ScalarMathFunction_t>(log10));
    funcMap.insert("exp", static_cast<ScalarMathFunction_t>(exp));
    funcMap.insert("sqrt", static_cast<ScalarMathFunction_t>(sqrt));
    funcMap.insert("round", static_cast<ScalarMathFunction_t>(round));
    funcMap.insert("floor", static_cast<ScalarMathFunction_t>(floor));
    funcMap.insert("ceil", static_cast<ScalarMathFunction_t>(ceil));
    funcMap.insert("abs", static_cast<ScalarMathFunction_t>(fabs));
    if(desiredType != Scalar && funcMap.contains(getFunctionName(expr))) {
        QString funcName = getFunctionName(expr);
        QString argStr = expr.mid(funcName.size()+1, expr.size()-funcName.size()-2).trimmed();
        ScalarMathFunction_t func = funcMap.value(funcName);
        SharedVectorVariableT pData = getLogVariable(argStr);
        if(!pData) {
            evaluateExpression(argStr);

            if(mAnsType == HcomHandler::DataVector) {
                pData = mAnsVector;
            }
            else if (mAnsType == HcomHandler::Scalar) {
                mAnsScalar = func(mAnsScalar);
                return;
            }
        }
        if(pData) {
            mAnsType = HcomHandler::DataVector;
            mAnsVector = pLogDataHandler->createOrphanVariable(QString(funcName+"%1").arg(pData->getSmartName()), pData->getVariableType());
            mAnsVector->assignFrom(pData->getSharedTimeOrFrequencyVector(), pData.data()->invokeMathFunctionOnData(func));
            return;
        }
    }

    //More complex vector functions
    if(desiredType != Scalar && isHcomFunctionCall("ddt", expr))
    {
        QStringList args = extractFunctionCallExpressionArguments(expr);
        if(args.size() == 1)
        {
            evaluateExpression(args[0],DataVector);
            SharedVectorVariableT pVar = mAnsVector;
            if (mAnsType == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pLogDataHandler->diffVariables(pVar, pVar->getSharedTimeOrFrequencyVector());
                return;
            }
            else
            {
                HCOMERR(QString("Variable: %1 was not found!").arg(args[0]));
                mAnsType = Undefined;
                return;
            }
        }
        else if(args.size() == 2)
        {
            const QString var1 = args[0];
            evaluateExpression(var1, DataVector);
            SharedVectorVariableT pVar1 = mAnsVector;
            if (mAnsType == DataVector)
            {
                const QString var2 = args[1];
                evaluateExpression(var2, DataVector);
                SharedVectorVariableT pVar2 = mAnsVector;
                if (mAnsType == DataVector)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->diffVariables(pVar1, pVar2);
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
    //else if(desiredType != Scalar && expr.startsWith("lp1(") && expr.endsWith(")"))
    else if(desiredType != Scalar && isHcomFunctionCall("lp1", expr))
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
                    mAnsVector = pLogDataHandler->lowPassFilterVariable(pVar, pVar->getSharedTimeOrFrequencyVector(), freq);
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
                        mAnsVector = pLogDataHandler->lowPassFilterVariable(pVar, pTimeVar, freq);
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
    //else if(desiredType != Scalar && expr.startsWith("int(") && expr.endsWith(")"))
    else if(desiredType != Scalar && isHcomFunctionCall("int", expr))
    {
        QString args = expr.mid(4, expr.size()-5);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args,',');
        if(splitArgs.size() == 1)
        {
            evaluateExpression(args.trimmed(),DataVector);
            if(mAnsType == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pLogDataHandler->integrateVariables(mAnsVector, mAnsVector->getSharedTimeOrFrequencyVector());
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

            const QString var1 = splitArgs[0].trimmed();
            const QString var2 = splitArgs[1].trimmed();
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
                mAnsVector = pLogDataHandler->integrateVariables(pVar1, pVar2);
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
    //else if(desiredType != Scalar && expr.startsWith("x(") && expr.endsWith(")"))
    else if(desiredType != Scalar && isHcomFunctionCall("x", expr))
    {
        QString arg = expr.mid(2, expr.size()-3);
        evaluateExpression(arg, DataVector);
        if(mAnsType == DataVector)
        {
            SharedVectorVariableT pVar = mAnsVector;
            if(!mAnsVector->getSharedTimeOrFrequencyVector().isNull())
            {
                mAnsVector = pVar->getSharedTimeOrFrequencyVector();
                mAnsType = DataVector;
                return;
            }
        }
        mAnsType = Undefined;
    }
    else if(desiredType != Scalar && isHcomFunctionCall("fft", expr)) {
        HCOMWARN("This function is deprecated and might be removed in the future. Use esd(), psd() or rmsd() instead.");

        QString argStr = expr.mid(4, expr.size()-5);
        QStringList args = SymHop::Expression::splitWithRespectToParentheses(argStr,',');
        for(int a=0; a<args.size(); ++a) {
            args[a] = args[a].trimmed();
        }
        QString dataVecArg;
        QString timeVecArg;
        QString typeArg = "power";  //default value
        QString windowingFuncArg;
        QString minTimeArg;
        QString maxTimeArg;

        //Figure out which arguments mean what
        dataVecArg = args[0];
        if(args.size()>1) {
            if(args[1] == "energy" || args[1] == "power" || args[1] == "rms") {
                typeArg = args[1];
            }
            else {
                timeVecArg = args[1];
            }
        }
        if(args.size()>2) {
            if(!timeVecArg.isEmpty()) {
                typeArg = args[2];
            }
            else {
                windowingFuncArg = args[2];
            }
        }
        if(args.size() > 3) {
            if(!timeVecArg.isEmpty()) {
                windowingFuncArg = args[3];
            }
            else {
                minTimeArg = args[3];
            }
        }
        if(args.size() > 4) {
            if(!timeVecArg.isEmpty()) {
                minTimeArg = args[4];
            }
            else {
                maxTimeArg = args[4];
            }
        }
        if(args.size() > 5) {
            maxTimeArg = args[5];
        }


        QString funcName = "psd";
        if(typeArg == "energy") {
            funcName = "esd";
        }
        else if(typeArg == "rms") {
            funcName = "rmsd";
        }

        QString timeVecName = timeVecArg;
        if(timeVecArg.isEmpty()) {
            //Data vector
            evaluateExpression(dataVecArg, DataVector);
            if(mAnsType != DataVector) {
                HCOMERR(QString("Variable: %1 was not found!").arg(dataVecArg));
                mAnsType = Undefined;
                return;
            }
            SharedVectorVariableT pVar = mAnsVector;
            timeVecName = pVar->getSharedTimeOrFrequencyVector()->getFullVariableName();
        }

        //Call other functions based on arguments to this (deprecated) function
        if(windowingFuncArg.isEmpty()) {
            evaluateExpression(funcName+"("+dataVecArg+","+timeVecName+")");
        }
        else if(!windowingFuncArg.isEmpty() && maxTimeArg.isEmpty()) {
            evaluateExpression(funcName+"("+dataVecArg+","+timeVecName+","+windowingFuncArg+")");
        }
        else if(!minTimeArg.isEmpty() && maxTimeArg.isEmpty()) {
            evaluateExpression(funcName+"("+dataVecArg+","+timeVecName+","+windowingFuncArg+","+minTimeArg+")");
        }
        else {
            evaluateExpression(funcName+"("+dataVecArg+","+timeVecName+","+windowingFuncArg+","+minTimeArg+","+maxTimeArg+")");
        }
        return;
    }
    else if(desiredType != Scalar && (isHcomFunctionCall("esd", expr) || isHcomFunctionCall("psd", expr) || isHcomFunctionCall("rmsd", expr)))
    {
        QStringList splitArgs = extractFunctionCallExpressionArguments(expr);
        if(splitArgs.size() < 1 || splitArgs.size() > 5) {
            QString funcName = getFunctionName(expr);
            HCOMERR("Wrong number of arguments provided for "+funcName+" function.\n"+mLocalFunctionDescriptions.find(funcName).value().second);
            mAnsType = Undefined;
            return;
        }

        //Parse power spectrum type
        FrequencySpectrumEnumT type;
        if(isHcomFunctionCall("psd", expr)) {
            type = PowerSpectrum;
        }
        else if(isHcomFunctionCall("esd", expr)) {
            type = EnergySpectrum;
        }
        else if(isHcomFunctionCall("rmsd", expr)) {
            type = RMSSpectrum;
        }

        //Data vector
        int i=0;
        const QString varName = splitArgs[i].trimmed();
        evaluateExpression(varName, DataVector);
        if(mAnsType != DataVector) {
            HCOMERR(QString("Variable: %1 was not found!").arg(varName));
            mAnsType = Undefined;
            return;
        }
        SharedVectorVariableT pVar = mAnsVector;
        ++i;

        //Time vector
        SharedVectorVariableT pTimeVar;
        if(splitArgs.size() > i) {
            QString arg2 = splitArgs[i].trimmed();
            evaluateExpression(arg2, DataVector);
            if(mAnsType == DataVector) {
                ++i;
                pTimeVar = mAnsVector;
            }
        }
        else {
            pTimeVar = pVar->getSharedTimeOrFrequencyVector();
        }



        //Parse windowing function argument
        WindowingFunctionEnumT windowingFunction = RectangularWindow;
        if(splitArgs.size() > i) {
            if(splitArgs[i].toLower() == "hann") {
                windowingFunction = HannWindow;
                ++i;
            }
            else if(splitArgs[i].toLower() == "rectangular") {
                windowingFunction = RectangularWindow;
                ++i;
            }
            else if(splitArgs[i].toLower() == "flattop") {
                windowingFunction = FlatTopWindow;
                ++i;
            }
        }

        double minTime=-std::numeric_limits<double>::max();
        double maxTime=std::numeric_limits<double>::max();

        if(splitArgs.size() > i+1) {
            bool ok;
            minTime = getNumber(splitArgs[i], &ok);
            if(!ok) {
                HCOMERR("Unknown minimum time limit: "+splitArgs[i]);
                mAnsType = Undefined;
                return;
            }
            maxTime = getNumber(splitArgs[i+1], &ok);
            if(!ok) {
                HCOMERR("Unknown maximum time limit: "+splitArgs[i+1]);
                mAnsType = Undefined;
                return;
            }
            i+=2;
        }

        if(splitArgs.size() > i) {
            HCOMERR("Unknown argument: "+splitArgs[i]);
            mAnsType = Undefined;
            return;
        }

        mAnsType = DataVector;
        mAnsVector = pVar->toFrequencySpectrum(pTimeVar, type, windowingFunction, minTime, maxTime);
        return;
    }
    else if(isHcomFunctionCall("rms", expr))
    {
        QStringList splitArgs = extractFunctionCallExpressionArguments(expr);
        if(splitArgs.size() != 1) {
            HCOMERR("Wrong number of arguments provided for rms function.\n"+mLocalFunctionDescriptions.find("rms").value().second);
            mAnsType = Undefined;
            return;
        }
        mAnsType = Scalar;
        const QString varName = splitArgs[0].trimmed();
        evaluateExpression(varName, DataVector);
        SharedVectorVariableT pVar = mAnsVector;
        if (mAnsType == DataVector) {
            mAnsType = Scalar;
            mAnsScalar = pVar->rmsOfData();
            return;
        }
        else
        {
            HCOMERR(QString("Variable: %1 was not found!").arg(varName));
            mAnsType = Undefined;
            return;
        }
    }
    //else if(desiredType != Scalar && (expr.startsWith("greaterThan(") || expr.startsWith("gt(")) && expr.endsWith(")"))
    else if(desiredType != Scalar && (isHcomFunctionCall("greaterThan", expr) || isHcomFunctionCall("gt", expr)) )
    {
        executeGtBuiltInFunction(expr);
        return;
    }
    //else if(desiredType != Scalar && (expr.startsWith("smallerThan(") || expr.startsWith("lt(")) && expr.endsWith(")"))
    else if(desiredType != Scalar && (isHcomFunctionCall("smallerThan", expr) || isHcomFunctionCall("lt", expr)) )
    {
        executeLtBuiltInFunction(expr);
        return;
    }
    //else if(desiredType != Scalar && (expr.startsWith("eq(") && expr.endsWith(")")))
    else if(desiredType != Scalar && isHcomFunctionCall("eq", expr))
    {
        executeEqBuiltInFunction(expr);
        return;
    }
    //else if(desiredType != Scalar && (expr.startsWith("linspace(") && expr.endsWith(")")))
    else if(desiredType != Scalar && isHcomFunctionCall("linspace", expr))
    {
        QStringList args = extractFunctionCallExpressionArguments(expr);
        if (args.size() == 3)
        {
            bool minOK, maxOK, nOK;
            double min = evaluateScalarExpression(args.first(), minOK);
            double max = evaluateScalarExpression(args[1], maxOK);
            int nSamp = int(evaluateScalarExpression(args.last(), nOK)+0.5);

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
                        mAnsVector = mpModel->getLogDataHandler()->createOrphanVariable("linspace");
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
                HCOMERR("Could not parse arguments, (scalars expected)");
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for linspace function");
        }
        mAnsType = Undefined;
        return;
    }
    //else if(desiredType != Scalar && (expr.startsWith("logspace(") && expr.endsWith(")")))
    else if(desiredType != Scalar && isHcomFunctionCall("logspace", expr))
    {
        QStringList args = extractFunctionCallExpressionArguments(expr);
        if (args.size() == 3)
        {
            bool minOK, maxOK, nOK;
            double min = evaluateScalarExpression(args.first(), minOK);
            double max = evaluateScalarExpression(args[1], maxOK);
            int nSamp = int(evaluateScalarExpression(args.last(), nOK)+0.5);

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
                        mAnsVector = mpModel->getLogDataHandler()->createOrphanVariable("logspace");
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
                HCOMERR("Could not parse arguments, (scalars expected)");
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for logspace function");
        }
        mAnsType = Undefined;
        return;
    }
    //else if(desiredType != Scalar && (expr.startsWith("ones(") && expr.endsWith(")")))
    else if(desiredType != Scalar && isHcomFunctionCall("ones", expr))
    {
        QString argStr = expr.mid(5, expr.size()-6);

        bool parseOK;
        int nElem = int(argStr.toDouble(&parseOK)+0.5);
        // Ok this is not a number lets evaluate the expression instead
        if (!parseOK)
        {
            evaluateExpression(argStr, Scalar);
            if (mAnsType == Scalar)
            {
                nElem = int(mAnsScalar+0.5*(mAnsScalar/qAbs(mAnsScalar))); // Round to nearest int through truncation (also to avoid numerical problem like 2.9999999999 should be 3)
                parseOK=true;
            }
        }

        if (parseOK)
        {
            if (nElem > 0)
            {
                if (mpModel)
                {
                    QVector<double> data(nElem, 1.0);
                    mAnsVector = mpModel->getViewContainerObject()->getLogDataHandler()->createOrphanVariable("ones");
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
    //else if(desiredType != Scalar && (expr.startsWith("zeros(") && expr.endsWith(")")))
    else if(desiredType != Scalar && isHcomFunctionCall("zeros", expr))
    {
        QString argStr = expr.mid(6, expr.size()-7);
        bool parseOK;
        int nElem = int(argStr.toDouble(&parseOK)+0.5);
        // Ok this is not a number lets evaluate the expression instead
        if (!parseOK)
        {
            evaluateExpression(argStr, Scalar);
            if (mAnsType == Scalar)
            {
                nElem = int(mAnsScalar+0.5*(mAnsScalar/qAbs(mAnsScalar))); // Round to nearest int through truncation (also to avoid numerical problem like 2.9999999999 should be 3)
                parseOK=true;
            }
        }

        if (parseOK)
        {
            if (nElem > 0)
            {
                if (mpModel)
                {
                    QVector<double> data(nElem, 0.0);
                    mAnsVector = mpModel->getViewContainerObject()->getLogDataHandler()->createOrphanVariable("zeros");
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
    else if(desiredType != Scalar && isHcomFunctionCall("vector", expr))
    {
        QStringList args = extractFunctionCallExpressionArguments(expr);
        if (args.size() > 0)
        {
            if (mpModel)
            {
                QVector<double> data(args.size(), 0.0);
                for (int i=0; i<data.size(); ++i)
                {
                    bool isOK;
                    data[i] = args[i].toDouble(&isOK);
                    if (!isOK)
                    {
                        HCOMERR("Could not parse "+args[i]);
                    }
                }
                mAnsVector = mpModel->getViewContainerObject()->getLogDataHandler()->createOrphanVariable("vector");
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
            HCOMERR("Can not create empty vector");
        }
        mAnsType = Undefined;
        return;
    }
    //else if(desiredType != Scalar && expr.startsWith("td(") && expr.endsWith(")"))      //Function for converting variable to time-domain variable
    else if(desiredType != Scalar && isHcomFunctionCall("td", expr))
    {
        QString varName = expr.mid(3, expr.size()-4);

        evaluateExpression(varName, DataVector);
        SharedVectorVariableT pVar = mAnsVector;
        if (mAnsType == DataVector)
        {
            mAnsType = DataVector;
            SharedVectorVariableT pNewVar = pLogDataHandler->createOrphanVariable("TD", TimeDomainType);
            pNewVar->assignFrom(pLogDataHandler->getTimeVectorVariable(-1), pVar->getDataVectorCopy());
            mAnsVector = pNewVar;
            return;
        }
        else
        {
            HCOMERR(QString("Variable: %1 was not found!").arg(varName));
            mAnsType = Undefined;
            return;
        }
    }
    //else if(desiredType != Scalar && expr.startsWith("fd(") && expr.endsWith(")"))  //Function for converting variable to frequency domain variable
    else if(desiredType != Scalar && isHcomFunctionCall("fd", expr))
    {
        QString varName = expr.mid(3, expr.size()-4);

        evaluateExpression(varName, DataVector);
        SharedVectorVariableT pVar = mAnsVector;
        if (mAnsType == DataVector)
        {
            mAnsType = DataVector;
            SharedVectorVariableT pNewVar = pLogDataHandler->createOrphanVariable("TD", FrequencyDomainType);

            QVector<double> freqVec;
            QVector<double> timeVec = pLogDataHandler->getTimeVectorVariable(-1)->getDataVectorCopy();

            #ifndef Q_OS_OSX
                    const int n = pow(2, int(log2(timeVec.size()))); // This is odd.... /magse
            #else
                    const int n = (int)round(ldexp(2.0, int(log2(timeVec.size()))));
            #endif

            freqVec.reserve(n/2);
            const double maxt = timeVec.last();

            for(int i=1; i<=n/2; ++i)
            {
                freqVec.append(double(i)/maxt);
            }

            pNewVar->assignFrom(freqVec, pVar->getDataVectorCopy());
            mAnsVector = pNewVar;
            return;
        }
        else
        {
            HCOMERR(QString("Variable: %1 was not found!").arg(varName));
            mAnsType = Undefined;
            return;
        }
    }
    //else if(desiredType != Scalar && expr.startsWith("maxof(") && expr.endsWith(")"))
    else if(desiredType != Scalar && isHcomFunctionCall("maxof", expr))
    {
        QString args = expr.mid(6, expr.size()-7);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args, ',');
        if(splitArgs.size() == 2)
        {
            const QString arg1str = splitArgs.first().trimmed();
            const QString arg2str = splitArgs.last().trimmed();

            bool a1Scalar=false, a2Scalar=false, arg1Failed=false, arg2Failed=false;
            SharedVectorVariableT pArgVec1, pArgVec2;
            double arg1d, arg2d;

            // Evaluate argument1
            evaluateExpression(arg1str);
            if (mAnsType == Scalar)
            {
                a1Scalar=true;
                arg1d = mAnsScalar;
            }
            else if (mAnsType == DataVector)
            {
                pArgVec1 = mAnsVector;
            }
            else
            {
                arg1Failed=true;
            }

            // Evaluate argument2
            evaluateExpression(arg2str);
            if (mAnsType == Scalar)
            {
                a2Scalar=true;
                arg2d = mAnsScalar;
            }
            else if (mAnsType == DataVector)
            {
                pArgVec2 = mAnsVector;
            }
            else
            {
                arg2Failed=true;
            }

            if(!(arg1Failed && arg2Failed))
            {
                // Handle both scalar
                if (a1Scalar && a2Scalar)
                {
                    mAnsType = Scalar;
                    mAnsScalar = qMax(arg1d, arg2d);
                    return;
                }
                // Handle a2 scalar
                else if (pArgVec1 && a2Scalar)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->createOrphanVariable(QString("maxof%1%2").arg(pArgVec1->getSmartName()).arg(arg2d), pArgVec1->getVariableType());
                    QVector<double> v = pArgVec1->getDataVectorCopy();
                    QVector<double> r(v.size());
                    for (int i=0; i<v.size(); ++i)
                    {
                        r[i] = qMax(v[i],arg2d);
                    }
                    mAnsVector->assignFrom(pArgVec1->getSharedTimeOrFrequencyVector(), r);
                    return;
                }
                // Handle a1 scalar
                else if (a1Scalar && pArgVec2)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->createOrphanVariable(QString("maxof%1%2").arg(arg1d).arg(pArgVec2->getSmartName()), pArgVec2->getVariableType());
                    QVector<double> v = pArgVec2->getDataVectorCopy();
                    QVector<double> r(v.size());
                    for (int i=0; i<v.size(); ++i)
                    {
                        r[i] = qMax(arg1d, v[i]);
                    }
                    mAnsVector->assignFrom(pArgVec2->getSharedTimeOrFrequencyVector(), r);
                    return;
                }
                // Handle both vectors
                else if (pArgVec1 && pArgVec2)
                {
                    VariableTypeT type = pArgVec1->getVariableType();
                    if ( type != pArgVec2->getVariableType())
                    {
                        HCOMWARN(QString("Comparing different types %1!=%2 returning %1").arg(variableTypeAsString(type)).arg(variableTypeAsString(pArgVec2->getVariableType())));
                    }

                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->createOrphanVariable(QString("maxof%1%2").arg(pArgVec1->getSmartName()).arg(pArgVec2->getSmartName()), pArgVec1->getVariableType());
                    QVector<double> a = pArgVec1->getDataVectorCopy();
                    QVector<double> b = pArgVec2->getDataVectorCopy();
                    QVector<double> c(qMin(a.size(), b.size()));
                    for (int i=0; i<c.size(); ++i)
                    {
                        c[i] = qMax(a[i],b[i]);
                    }
                    mAnsVector->assignFrom(pArgVec1->getSharedTimeOrFrequencyVector(), c);
                    return;
                }
                else
                {
                    HCOMERR("Scalars or vectors could not be found");
                }
            }
            else
            {
                if (arg1Failed)
                {
                    HCOMERR(QString("Could not evaluate argument1: %1").arg(arg1str));
                }
                if (arg2Failed)
                {
                    HCOMERR(QString("Could not evaluate argument2: %1").arg(arg2str));
                }
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for maxof function.\n"+mLocalFunctionDescriptions.find("maxof").value().second);
        }
        mAnsType = Undefined;
        return;
    }
    //else if(desiredType != Scalar && expr.startsWith("minof(") && expr.endsWith(")"))
    else if(desiredType != Scalar && isHcomFunctionCall("minof", expr))
    {
        QString args = expr.mid(6, expr.size()-7);
        QStringList splitArgs = SymHop::Expression::splitWithRespectToParentheses(args, ',');
        if(splitArgs.size() == 2)
        {
            const QString arg1str = splitArgs.first().trimmed();
            const QString arg2str = splitArgs.last().trimmed();

            bool a1Scalar=false, a2Scalar=false, arg1Failed=false, arg2Failed=false;
            SharedVectorVariableT pArgVec1, pArgVec2;
            double arg1d, arg2d;

            // Evaluate argument1
            evaluateExpression(arg1str);
            if (mAnsType == Scalar)
            {
                a1Scalar=true;
                arg1d = mAnsScalar;
            }
            else if (mAnsType == DataVector)
            {
                pArgVec1 = mAnsVector;
            }
            else
            {
                arg1Failed=true;
            }

            // Evaluate argument2
            evaluateExpression(arg2str);
            if (mAnsType == Scalar)
            {
                a2Scalar=true;
                arg2d = mAnsScalar;
            }
            else if (mAnsType == DataVector)
            {
                pArgVec2 = mAnsVector;
            }
            else
            {
                arg2Failed=true;
            }

            if(!(arg1Failed && arg2Failed))
            {
                // Handle both scalar
                if (a1Scalar && a2Scalar)
                {
                    mAnsType = Scalar;
                    mAnsScalar = qMin(arg1d, arg2d);
                    return;
                }
                // Handle a2 scalar
                else if (pArgVec1 && a2Scalar)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->createOrphanVariable(QString("minof%1%2").arg(pArgVec1->getSmartName()).arg(arg2d), pArgVec1->getVariableType());
                    QVector<double> v = pArgVec1->getDataVectorCopy();
                    QVector<double> r(v.size());
                    for (int i=0; i<v.size(); ++i)
                    {
                        r[i] = qMin(v[i],arg2d);
                    }
                    mAnsVector->assignFrom(pArgVec1->getSharedTimeOrFrequencyVector(), r);
                    return;
                }
                // Handle a1 scalar
                else if (a1Scalar && pArgVec2)
                {
                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->createOrphanVariable(QString("minof%1%2").arg(arg1d).arg(pArgVec2->getSmartName()), pArgVec2->getVariableType());
                    QVector<double> v = pArgVec2->getDataVectorCopy();
                    QVector<double> r(v.size());
                    for (int i=0; i<v.size(); ++i)
                    {
                        r[i] = qMin(arg1d, v[i]);
                    }
                    mAnsVector->assignFrom(pArgVec2->getSharedTimeOrFrequencyVector(), r);
                    return;
                }
                // Handle both vectors
                else if (pArgVec1 && pArgVec2)
                {
                    VariableTypeT type = pArgVec1->getVariableType();
                    if ( type != pArgVec2->getVariableType())
                    {
                        HCOMWARN(QString("Comparing different types %1!=%2 returning %1").arg(variableTypeAsString(type)).arg(variableTypeAsString(pArgVec2->getVariableType())));
                    }

                    mAnsType = DataVector;
                    mAnsVector = pLogDataHandler->createOrphanVariable(QString("minof%1%2").arg(pArgVec1->getSmartName()).arg(pArgVec2->getSmartName()), pArgVec1->getVariableType());
                    QVector<double> a = pArgVec1->getDataVectorCopy();
                    QVector<double> b = pArgVec2->getDataVectorCopy();
                    QVector<double> c(qMin(a.size(), b.size()));
                    for (int i=0; i<c.size(); ++i)
                    {
                        c[i] = qMin(a[i],b[i]);
                    }
                    mAnsVector->assignFrom(pArgVec1->getSharedTimeOrFrequencyVector(), c);
                    return;
                }
                else
                {
                    HCOMERR("Scalars or vectors could not be found");
                }
            }
            else
            {
                if (arg1Failed)
                {
                    HCOMERR(QString("Could not evaluate argument1: %1").arg(arg1str));
                }
                if (arg2Failed)
                {
                    HCOMERR(QString("Could not evaluate argument2: %1").arg(arg2str));
                }
            }
        }
        else
        {
            HCOMERR("Wrong number of arguments provided for minof function.\n"+mLocalFunctionDescriptions.find("minof").value().second);
        }
        mAnsType = Undefined;
        return;
    }
    else if(desiredType != Scalar && expr.count("<")==1 /*&& getLogVariable(expr.section("<",0,0))*/)
    {
        QStringList args = expr.split('<');
        if (args.size() == 2)
        {
            //Reshape to look like a function call
            QString call = QString("lt(%1,%2)").arg(args[0]).arg(args[1]);
            executeLtBuiltInFunction(call);
            return;
        }
    }
    else if(desiredType != Scalar && expr.count(">")==1 /*&& getLogVariable(expr.section(">",0,0))*/)
    {
        QStringList args = expr.split('>');
        if (args.size() == 2)
        {
            //Reshape to look like a function call
            QString call = QString("gt(%1,%2)").arg(args[0]).arg(args[1]);
            executeGtBuiltInFunction(call);
            return;
        }
    }
    else if(desiredType != Scalar && desiredType != DataVector && isHcomFunctionCall("expr", expr)) {
       QStringList args = extractFunctionCallExpressionArguments(expr);
       if(args.size() != 1) {
           HCOMERR("Wrong number of arguments for expr function.\n"+mLocalFunctionDescriptions.find("expr").value().second);
           mAnsType = Undefined;
           return;
       }
       bool ok;
       mAnsExpression = SymHop::Expression(args[0], &ok, SymHop::Expression::NoSimplifications);
       if(!ok) {
           for(const auto &msg : mAnsExpression.readErrorMessages()) {
               HCOMERR(msg);
           }
           mAnsType = Undefined;
           return;
       }
       mAnsType = Expression;
       return;
    }
    else if(isHcomFunctionCall("eval", expr)) {
        QStringList args = extractFunctionCallExpressionArguments(expr);
        if(args.size() != 1) {
            HCOMERR("Wrong number of arguments for eval function.\n"+mLocalFunctionDescriptions.find("eval").value().second);
            mAnsType = Undefined;
            return;
        }
        if(mLocalExpressions.contains(args[0])) {
            bool ok;
            double value = mLocalExpressions[args[0]].evaluate(mLocalVars,&mLocalFunctionoidPtrs, &ok);
            if(ok) {
                mAnsScalar = value;
                mAnsType = Scalar;
                return;
            }
            else {
                HCOMERR("Failed to evaluate expression \""+args[0]+"\".");
                mAnsType = Undefined;
                return;
            }
        }
        else {
            HCOMERR("Local expression \""+args[0]+"\" not found.");
            mAnsType = Undefined;
            return;
        }
    }
    else if(isHcomFunctionCall("der", expr) || isHcomFunctionCall("factor", expr)) {
        QStringList args = extractFunctionCallExpressionArguments(expr);
        if(args.size() != 2) {
            HCOMERR("Wrong number of arguments for \""+getFunctionName(expr)+"\" function (should be 2)");
            mAnsType = Undefined;
            return;
        }
        SymHop::Expression expr1, expr2;
        evaluateExpression(args[0]);
        if(mAnsType != Expression) {
            HCOMERR("Failed to evaluate expression \""+args[0]+"\".");
            mAnsType = Undefined;
            return;
        }
        expr1 = mAnsExpression;
        evaluateExpression(args[1]);
        if(mAnsType != Expression) {
            HCOMERR("Failed to evaluate expression \""+args[1]+"\".");
            mAnsType = Undefined;
            return;
        }
        expr2 = mAnsExpression;
        if(getFunctionName(expr) == "der") {
            bool ok;
            mAnsExpression = expr1.derivative(expr2,ok);
            if(!ok) {
                HCOMERR("Failed to differentiate expression \""+args[0]+"\" with respect to \""+args[1]+"\"");
                mAnsType = Undefined;
                return;
            }
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "factor") {
            mAnsExpression = expr1;
            mAnsExpression.factor(expr2);
            mAnsType = Expression;
            return;
        }
    }
    else if(isHcomFunctionCall("simplify", expr) || isHcomFunctionCall("linearize", expr) ||
            isHcomFunctionCall("expand", expr) || isHcomFunctionCall("expandPowers", expr) ||
            isHcomFunctionCall("toLeftSided", expr) || isHcomFunctionCall("factorMostCommonFactor", expr) ||
            isHcomFunctionCall("removeDivisors", expr) || isHcomFunctionCall("left", expr) ||
            isHcomFunctionCall("right", expr) || isHcomFunctionCall("trapezoid", expr) ||
            isHcomFunctionCall("euler", expr) || isHcomFunctionCall("bdf1", expr) ||
            isHcomFunctionCall("bdf2", expr) || isHcomFunctionCall("latex", expr)) {

        QStringList args = extractFunctionCallExpressionArguments(expr);
        if(args.size() != 1) {
            HCOMERR("Wrong number of arguments for \""+getFunctionName(expr)+"\" function (should be 1)");
            mAnsType = Undefined;
            return;
        }
        evaluateExpression(args[0]);
        if(mAnsType != Expression) {
            HCOMERR("Failed to evaluate expression \""+args[0]+"\".");
            mAnsType = Undefined;
            return;
        }
        SymHop::Expression expr1 = mAnsExpression;
        if(getFunctionName(expr) == "simplify") {
            mAnsExpression = expr1;
            mAnsExpression._simplify(SymHop::Expression::FullSimplification, SymHop::Expression::Recursive);
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "linearize") {
            mAnsExpression = expr1;
            mAnsExpression.linearize();
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "expand") {
            mAnsExpression = expr1;
            mAnsExpression.expand();
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "expandPowers") {
            mAnsExpression = expr1;
            mAnsExpression.expandPowers();
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "toLeftSided") {
            mAnsExpression = expr1;
            mAnsExpression.toLeftSided();
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "factorMostCommonFactor") {
            mAnsExpression = expr1;
            mAnsExpression.factorMostCommonFactor();
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "removeDivisors") {
            mAnsExpression = expr1;
            mAnsExpression.removeDivisors();
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "left") {
            mAnsExpression = (*expr1.getLeft());
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "right") {
            mAnsExpression = (*expr1.getRight());
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "trapezoid") {
            mAnsExpression = expr1;
            bool ok;
            mAnsExpression.inlineTransform(SymHop::Expression::Trapezoid, ok);
            if(!ok) {
                HCOMERR("Failed to transform expression.");
                mAnsType = Undefined;
                return;
            }
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "euler") {
            bool ok;
            mAnsExpression = expr1.inlineTransform(SymHop::Expression::ExplicitEuler, ok);
            if(!ok) {
                HCOMERR("Failed to transform expression.");
                mAnsType = Undefined;
                return;
            }
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "bdf1") {
            bool ok;
            mAnsExpression = expr1.inlineTransform(SymHop::Expression::BDF1, ok);
            if(!ok) {
                HCOMERR("Failed to transform expression.");
                mAnsType = Undefined;
                return;
            }
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "bdf2") {
            bool ok;
            mAnsExpression = expr1.inlineTransform(SymHop::Expression::BDF2, ok);
            if(!ok) {
                HCOMERR("Failed to transform expression.");
                mAnsType = Undefined;
                return;
            }
            mAnsType = Expression;
            return;
        }
        else if(getFunctionName(expr) == "latex") {
            mAnsWildcard = expr1.toLaTeX();
            mAnsType = String;
            return;
        }
    }
    else if(desiredType != Scalar && isHcomFunctionCall("cut", expr))
    {
        executeCutBuiltInFunction(expr);
        return;
    }
    else if(desiredType != Scalar && isHcomFunctionCall("ssi", expr))
    {
        executeSsiBuiltInFunction(expr);
        return;
    }
    //timer.toc("Vector functions", 1);

    //Evaluate expression using SymHop
    bool ok;
    SymHop::Expression symHopExpr = SymHop::Expression(expr, &ok, SymHop::Expression::NoSimplifications);

    if(!ok || !symHopExpr.verifyExpression(mLocalFunctionoidPtrs.keys()+mLocalFunctionDescriptions.keys()))
    {
        HCOMERR("Could not evaluate expression: "+expr);
        mAnsType = Wildcard;
        mAnsWildcard = expr;
        return;
    }

    //Multiplication between data vector and scalar
    //timer.tic();
    //! @todo this code does pointer lookup, then does it again, and then get names to use string versions of logdatahandler functions, it could lookup once and then use the pointer versions instead
    //! @todo If SymHop simplifies expression to a constant, the code below won't find it
    if(desiredType != Scalar && symHopExpr.isMultiplyOrDivide() && (symHopExpr.getDivisors().isEmpty() || symHopExpr.getFactors().size() > 1) && pLogDataHandler)
    {
        SymHop::Expression f0 = symHopExpr.getFactors()[0];
        SymHop::Expression f1 = symHopExpr;
        f1.removeFactor(f0);

        VariableType varType0, varType1;
        double scalar0=0, scalar1=0;
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
            mAnsVector = pLogDataHandler->mulVariableWithScalar(vec1, scalar0);
            return;
        }
        else if(varType0 == DataVector && varType1 == Scalar)
        {
            mAnsType = DataVector;
            mAnsVector = pLogDataHandler->mulVariableWithScalar(vec0, scalar1);
            return;
        }
        else if(varType0 == DataVector && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogDataHandler->multVariables(vec0, vec1);
            return;
        }
    }
    if(desiredType != Scalar && pLogDataHandler && symHopExpr.isPower())
    {
        SymHop::Expression b = *symHopExpr.getBase();
        SymHop::Expression p = *symHopExpr.getPower();

        evaluateExpression(b.toString());

        //! @todo this toDouble == 2.0 comparison is not safe (but seems to work)
        if(mAnsType == DataVector && p.toDouble() >= 0.0/* && p.toDouble() == 2.0*/)
        {
            mAnsType = DataVector;
            //mAnsVector = pLogDataHandler->multVariables(mAnsVector, mAnsVector);
            mAnsVector = pLogDataHandler->elementWisePower(mAnsVector, p.toDouble());
            return;
        }
        //! @todo what if not square (power 2) Need elementWisePower function for that
    }
    if(desiredType != Scalar && pLogDataHandler && symHopExpr.isMultiplyOrDivide() && symHopExpr.getFactors().size() == 1)
    {
        SymHop::Expression f = symHopExpr.getFactors()[0];
        SymHop::Expression d = SymHop::Expression::fromFactorsDivisors(symHopExpr.getDivisors(), QList<SymHop::Expression>());

        VariableType f_varType, d_varType;
        double f_scalar, d_scalar;
        SharedVectorVariableT f_vec, d_vec;
        evaluateExpression(f.toString());
        if(mAnsType == DataVector || mAnsType == Scalar)
        {
            f_varType = mAnsType;
            if(f_varType == Scalar)
            {
                f_scalar = mAnsScalar;
            }
            else
            {
                f_vec = mAnsVector;
            }

            evaluateExpression(d.toString());
            if(mAnsType != DataVector && mAnsType != Scalar)
            {
                mAnsType = Undefined;
                return;
            }
            d_varType = mAnsType;
            if(d_varType == Scalar)
            {
                d_scalar = mAnsScalar;
            }
            else
            {
                d_vec = mAnsVector;
            }

            if(f_varType == DataVector && d_varType == Scalar)
            {
                mAnsType = DataVector;
                mAnsVector = pLogDataHandler->divVariableWithScalar(f_vec, d_scalar);
                return;
            }
            else if(f_varType == Scalar && d_varType == DataVector)
            {
                mAnsType = DataVector;
                SharedVectorVariableT ones = pLogDataHandler->createOrphanVariable("ones", d_vec->getVariableType());
                ones->assignFrom(QVector<double>(d_vec->getDataSize(), 1.0));
                auto div = pLogDataHandler->divVariables(ones, d_vec);
                mAnsVector = pLogDataHandler->mulVariableWithScalar(div, f_scalar);
                return;
            }
            else if(f_varType == DataVector && d_varType == DataVector)
            {
                mAnsType = DataVector;
                mAnsVector = pLogDataHandler->divVariables(f_vec, d_vec);
                return;
            }
            else if(f_varType == Scalar && d_varType == Scalar)
            {
                mAnsType = Scalar;
                mAnsScalar = f_scalar / d_scalar;
                return;
            }
        }
    }
    if(desiredType != Scalar && pLogDataHandler && symHopExpr.isAdd())
    {
        SymHop::Expression t0 = symHopExpr.getTerms()[0];
        SymHop::Expression t1 = symHopExpr;
        t1.subtractBy(t0);
        t0._simplify(SymHop::Expression::FullSimplification, SymHop::Expression::Recursive);
        t1._simplify(SymHop::Expression::FullSimplification, SymHop::Expression::Recursive);

        VariableType varType0, varType1;
        double scalar0=0, scalar1=0;
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
            mAnsVector = pLogDataHandler->addVariableWithScalar(vec0, scalar1);
            return;
        }
        else if(varType0 == Scalar && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogDataHandler->addVariableWithScalar(vec1, scalar0);
            return;
        }
        else if(varType0 == DataVector && varType1 == DataVector)
        {
            mAnsType = DataVector;
            mAnsVector = pLogDataHandler->addVariables(vec0, vec1);
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
        if(!symHopExpr.verifyExpression(mLocalFunctionoidPtrs.keys())) {
            mAnsType = Wildcard;
            HCOMERR("Illegal SymHop expression: "+symHopExpr.toString());
            mAnsWildcard = symHopExpr.toString();
        }

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

//! @brief Evaluate an expressions when the expected result is a scalar, the expression may in turn contain expressions
double HcomHandler::evaluateScalarExpression(QString expr, bool &rIsOK)
{
    // Note! we do not use 'Scalar' here since then the expression can not in it self be an expression
    evaluateExpression(expr, Undefined);
    if (mAnsType == Scalar)
    {
        rIsOK = true;
        return mAnsScalar;
    }
    else
    {
        rIsOK = false;
        return -1;
    }
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

void HcomHandler::updatePwd()
{
    if(mpModel && mpModel->isSaved() && gpConfig->getBoolSetting(cfg::setpwdtomwd))
    {
        mPwd = mpModel->getTopLevelSystemContainer()->getModelFileInfo().absoluteDir().path();
    }
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

        // Remove indentation and trailing spaces
        lines[l] = lines[l].trimmed();

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
            QString funcName = lines[l].section(" ",1).trimmed();
            QStringList funcCommands;
            while(!lines[l].trimmed().startsWith("enddefine"))
            {
                ++l;
                if(l>=lines.size())
                {
                    HCOMERR("Missing  enddefine  to end function definition.");
                    return QString();
                }
                funcCommands << lines[l];
            }
            funcCommands.removeLast();
            mFunctions.insert(funcName, funcCommands);
            HCOMPRINT("Defined function: "+funcName);
        }
        else if(lines[l].startsWith("goto"))
        {
            QString argument = lines[l].section(" ",1);
            return argument;
        }
        else if(lines[l].startsWith("while"))        //Handle while loops
        {
            QStringList args = extractFunctionCallExpressionArguments(lines[l]);
            if (args.size() != 1)
            {
                HCOMERR("While requires one condition expression");
                return "";
            }
            QString condition = args.front();
            QStringList loop;
            int nLoops=1;
            while(nLoops > 0)
            {
                ++l;
                if(l>=lines.size())
                {
                    HCOMERR("Missing  repeat  to end while loop.");
                    return QString();
                }
                lines[l] = lines[l].trimmed();

                if(lines[l].startsWith("while")) { ++nLoops; }
                if(lines[l].startsWith("repeat")) { --nLoops; }

                loop.append(lines[l]);
            }
            loop.removeLast();

            // Evaluate condition using SymHop
            SymHop::Expression symHopExpr = SymHop::Expression(condition);
            QMap<QString, double> localVars = mLocalVars;
            QStringList localPars;
            getParameters("*", localPars);
            for(int p=0; p<localPars.size(); ++p)
            {
                localVars.insert(localPars[p],getParameterValue(localPars[p]).toDouble());
            }

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

                // Update local variables for SymHop in case they have changed
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
            QStringList args = extractFunctionCallExpressionArguments(lines[l]);
            if (args.size() != 1)
            {
                HCOMERR("If requires one condition expression");
                return "";
            }
            QList<QString> conditions;
            conditions.append(args.front());
            QList<QStringList> ifCodes;
            ifCodes.append(QStringList());
            QStringList elseCode;
            bool inElse=false;
            int depth=1;
            while(true)
            {
                ++l;
                if(l>=lines.size())
                {
                    HCOMERR("Missing  endif  in if-statement.");
                    return QString();
                }
                lines[l] = lines[l].trimmed();
                if(lines[l].startsWith("if")) {
                    ++depth;
                }
                else if(lines[l] == "endif")
                {
                    if(depth > 1) {
                        --depth;
                    }
                    else {
                        break;
                    }
                }
                else if(lines[l].startsWith("elseif") && depth == 1) {
                    QStringList args = extractFunctionCallExpressionArguments(lines[l]);
                    if (args.size() != 1)
                    {
                        HCOMERR("If requires one condition expression");
                        return "";
                    }
                    conditions.append(args.front());
                    ifCodes.append(QStringList());
                    continue;
                }
                else if(lines[l] == "else" && depth == 1)
                {
                    inElse=true;
                    continue;
                }
                if(!inElse)
                {
                    ifCodes.last().append(lines[l]);
                }
                else
                {
                    elseCode.append(lines[l]);
                }
            }

            bool oneConditionWasTrue = false;
            for(int i=0; i<conditions.size(); ++i) {
                evaluateExpression(conditions[i], Scalar);
                if(mAnsType != Scalar)
                {
                    HCOMERR("Evaluation of if-statement argument failed.");
                    return QString();
                }
                if(mAnsScalar > 0)
                {
                    oneConditionWasTrue = true;
                    QString gotoLabel = runScriptCommands(ifCodes[i], pAbort);
                    if(*pAbort)
                    {
                        return "";
                    }
                    if(!gotoLabel.isEmpty())
                    {
                        return gotoLabel;
                    }
                    break;
                }
            }
            if(!oneConditionWasTrue)
            {
                if(!elseCode.isEmpty()) {
                    QString gotoLabel = runScriptCommands(elseCode, pAbort);
                    if(*pAbort)
                    {
                        return QString();
                    }
                    if(!gotoLabel.isEmpty())
                    {
                        return gotoLabel;
                    }
                }
                else {
                    continue;
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
            while(!lines[l].trimmed().startsWith("endforeach"))
            {
                ++l;
                if(l>=lines.size())
                {
                    HCOMERR("Missing  endforeach  in foreach-statement.");
                    return QString();
                }
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
void HcomHandler::getComponents(const QString &rStr, QList<ModelObject*> &rComponents, SystemObject *pSystem) const
{
    if(!mpModel) { return; }

    if(!pSystem)
        pSystem = mpModel->getViewContainerObject();

    if(!pSystem) { return; }

    if (rStr.contains("*"))
    {
        QString left = rStr.split("*").first();
        QString right = rStr.split("*").last();

        QStringList mo_names = pSystem->getModelObjectNames();
        for(int n=0; n<mo_names.size(); ++n)
        {
            if(mo_names[n].startsWith(left) && mo_names[n].endsWith(right))
            {
                rComponents.append(pSystem->getModelObject(mo_names[n]));
            }
        }
    }
    else
    {
        auto pModel = pSystem->getModelObject(rStr);
        if (pModel)
        {
            rComponents.append(pModel);
        }
    }
}


//! @brief Help function that returns a list of ports depending on input (with support for asterisks)
//! @param[in] rStr Port name to look for
//! @param[out] rPorts Reference to list of found components
void HcomHandler::getPorts(const QString &rStr, QList<Port*> &rPorts) const
{
    if(!mpModel) { return; }
    SystemObject *pCurrentSystem = mpModel->getViewContainerObject();
    if(!pCurrentSystem) { return; }

    if (rStr.contains("*"))
    {
        QStringList compNames = pCurrentSystem->getModelObjectNames();
        for(const QString &compName : compNames) {
            for(int p=0; p<pCurrentSystem->getModelObject(compName)->getPortListPtrs().size(); ++p) {
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
        if(pCurrentSystem->hasModelObject(compName))
        {
            if(pCurrentSystem->getModelObject(compName)->getPort(portName))
            {
                rPorts.append(pCurrentSystem->getModelObject(compName)->getPort(portName));
            }
        }

        QStringList compNames = pCurrentSystem->getModelObjectNames();
        for(const QString &compName : compNames) {
            ModelObject *pModel = pCurrentSystem->getModelObject(compName);
            QMapIterator<QString,QString> it(pModel->getVariableAliases());
            while(it.hasNext()) {
                it.next();
                if(it.value() == rStr) {
                    rPorts.append(pModel->getPort(it.key().section("#",0,0)));
                }
            }
        }
    }
}


QString HcomHandler::getfullNameFromAlias(const QString &rAlias) const
{
    if(mpModel)
    {
        SystemObject *pCurrentSystem = mpModel->getViewContainerObject();
        if(pCurrentSystem)
        {
            return pCurrentSystem->getFullNameFromAlias(rAlias);
        }
    }
    return "";
}


//! @brief Generates a list of parameters based on regular expression with wildcards support
//! @param[in] str String with (or without) wildcards
//! @param[out] rParameters Reference to list of parameters
void HcomHandler::getParameters(const QString str, QStringList &rParameters) const
{
    if(!mpModel) { return; }

    QStringList allParameters;
    SystemObject *pSystem = mpModel->getViewContainerObject();
    getParametersFromContainer(pSystem, allParameters);

    QStringList aliasNames = pSystem->getAliasNames();
    for(const QString &alias : aliasNames)
    {
        QString fullName = pSystem->getFullNameFromAlias(alias);
        fullName.replace("#",".");
        toShortDataNames(fullName);
        if(allParameters.contains(fullName))
        {
            allParameters.append(alias);
        }
    }

    // Use regexp pattern matching to filter out any name not matching pattern (or exact value)
    QRegExp rx(str);
    rx.setPatternSyntax(QRegExp::Wildcard);
    for (const QString &rParname : allParameters)
    {
        if(rx.exactMatch(rParname))
        {
            rParameters.append(rParname);
        }
    }
}

void HcomHandler::getParametersFromContainer(SystemObject *pSystem, QStringList &rParameters) const
{
    QStringList sysnames;
    if(pSystem != mpModel->getViewContainerObject())
    {
        sysnames.prepend(pSystem->getName());
        SystemObject *pParentSystem = pSystem->getParentSystemObject();
        while(pParentSystem != getModelPtr()->getViewContainerObject())
        {
            sysnames.prepend(pParentSystem->getName());
            pParentSystem = pParentSystem->getParentSystemObject();
        }
    }

    QList<ModelObject*> modelObjects = pSystem->getModelObjects();
    // Add quotation marks around component name if it contains spaces
    for(ModelObject *pMO : modelObjects)
    {
        // Recursively fetch paramters from subsystems
        if(pMO->getTypeName() == "Subsystem")
        {
            getParametersFromContainer(qobject_cast<SystemObject*>(pMO), rParameters);
        }
        else
        {
            QString componentName = pMO->getName();
            QStringList parameterNames = pMO->getParameterNames();

            // Add quotation marks around component name if it contains spaces
            //! @todo This should not be needed since spacec in names are no longer allowed
            if(componentName.contains(" "))
            {
                componentName.prepend("\"");
                componentName.append("\"");
            }

            // Build full short name and append to results
            for(QString &parName : parameterNames)
            {
                parName = makeFullParameterName(sysnames, componentName, parName);
                toShortDataNames(parName);
                rParameters.append(parName);
            }
        }
    }

    // Now append this systems own parameters
    QStringList systemParameters = pSystem->getParameterNames();
    for(auto &sparname : systemParameters)
    {
        if(sparname.contains(" "))
        {
            sparname.prepend("\"");
            sparname.append("\"");
        }
        sparname = makeFullParameterName(sysnames, "", sparname);
        toShortDataNames(sparname);
        rParameters.append(sparname);
        // This is quite a hack, but also add self.parName variant for own system parameter names
        if (sysnames.isEmpty()) {
            rParameters.append("self."+sparname);
        }
    }
}


//! @brief Returns the value of specified parameter
//! @param[in] parameterName The full parameter name (relative to the current view container)
QString HcomHandler::getParameterValue(QString parameterName, QString &rParameterType, bool searchFromTopLevel) const
{
    rParameterType.clear();
    if(mpModel)
    {
        // Convert to long name so that we can search in model
        toLongDataNames(parameterName);

        QStringList subsystems;
        QString compName, parName;
        splitFullParameterName(parameterName, subsystems, compName, parName);

        // Seek into the correct system
        SystemObject *pContainer;
        if(searchFromTopLevel) {
            pContainer = mpModel->getTopLevelSystemContainer();
        }
        else {
            pContainer = mpModel->getViewContainerObject();
        }
        for(const QString &subsystem: subsystems) {
            pContainer = qobject_cast<SystemObject*>(pContainer->getModelObject(subsystem));
        }

        if(!pContainer) {
            return "NaN";
        }

        ModelObject *pMO = nullptr;
        // Handle system parameter
        if (compName.isEmpty() || compName == "self") {
            pMO = pContainer;
        }
        // Handle ordinary component
        else {
            pMO = pContainer->getModelObject(compName);
        }

        // If we have component then try to find the actual parameter, by actual name
        if (findParameterEvenIfValueNotSpecified(pMO, parName))
        {
            CoreParameterData par;
            pMO->getParameter(parName, par);
            rParameterType = par.mType;
            if(par.mValue.startsWith("self."))
            {
                QString otherPar = par.mValue.remove(0,5);
                return getParameterValue(compName+"."+otherPar, rParameterType);
            }
            return par.mValue;
        }

        // If we get here we failed above, lets check if the parameter name was actually an alias
        QString fullNameFromAlias = pContainer->getFullNameFromAlias(parName);
        ModelObject *pCompFromAlias = pContainer->getModelObject(fullNameFromAlias.section("#",0,0));
        QString actualParName = fullNameFromAlias.section("#",1);
        if(pCompFromAlias && pCompFromAlias->hasParameter(actualParName))
        {
            CoreParameterData par;
            pCompFromAlias->getParameter(actualParName, par);
            rParameterType = par.mType;
            return par.mValue;
        }
    }
    return "NaN";
}

QString HcomHandler::getParameterValue(QString parameterName) const
{
    QString dummy;
    return getParameterValue(parameterName, dummy);
}


//! @brief Returns variable names matching a pattern by communicating directly with the model
//! @param pattern Pattern using wildcards
//! @param rVariables Reference list used for returning matching variables
void HcomHandler::getMatchingLogVariableNamesWithoutLogDataHandler(QString pattern, QStringList &rVariables) const
{
    QStringList unFilteredVariables;

    QStringList components = mpModel->getViewContainerObject()->getModelObjectNames();
    for(const QString &component : components)
    {
        ModelObject *pComponent = mpModel->getViewContainerObject()->getModelObject(component);
        QList<Port*> portPtrs = pComponent->getPortListPtrs();
        QStringList ports;
        for(const Port *pPort : portPtrs) {
            ports.append(pPort->getName());
        }
        for(const QString &port : ports) {
            Port *pPort = pComponent->getPort(port);
            NodeInfo nodeInfo(pPort->getNodeType());
            QStringList vars = nodeInfo.shortNames;

            for(const QString &var : vars) {
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
//! @param [in] addGenerationSpecifier Set wheter to add generation @gen specifier to end of found names
//! @param [in] generationOverride Lets you override the generation specifier (or decide generation if it is missing)  must be >= 0
//! @warning If you make changes to this function you MUST MAKE SURE that all other Hcom functions using this is are still working for all cases. Many depend on the behavior of this function.
void HcomHandler::getMatchingLogVariableNames(QString pattern, QStringList &rVariables, bool addGenerationSpecifier, const int generationOverride ) const
{
    //TicToc timer;
    rVariables.clear();

    // Abort if no model found
    if(!mpModel) { return; }

    // Get pointers to logdatahandler
    LogDataHandler2* pLogDataHandler = mpModel->getLogDataHandler().data();

    // Parse and chop the desired generation
    int desiredGen = -3;
    if (generationOverride < 0)
    {
        bool parseOK;
        desiredGen = parseAndChopGenerationSpecifier(pattern, parseOK);
        if (!parseOK)
        {
            HCOMERR("Failed to parse generation specifier");
            return;
        }
    }
    else
    {
        // Note! we use generation override instead of appending @c to name,
        // that would cause the plot generation to be come locked at the current in the plot
        desiredGen = generationOverride;
    }

    // Convert short hcom format to long format
    QString pattern_long = pattern;
    toLongDataNames(pattern_long);
    //! @todo what about subsystem $

    // If we know generation then search for it directly
    if (isSingleGenerationValue(desiredGen))
    {
        QRegExp regexp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard);
        QList<SharedVectorVariableT> alias_variables = pLogDataHandler->getMatchingVariablesAtGeneration(regexp, desiredGen, Alias);
        for (auto &var : alias_variables)
        {
            QString name = var->getAliasName();
            toShortDataNames(name);
            rVariables.append(name);
            if (addGenerationSpecifier)
            {
                rVariables.last().append(generationspecifier::separatorString+QString::number(toDisplayGeneration(var->getGeneration())));
            }
        }
        QList<SharedVectorVariableT> full_variables = pLogDataHandler->getMatchingVariablesAtGeneration(regexp, desiredGen, FullName);
        for (auto &var : full_variables)
        {
            QString name = var->getFullVariableName();
            toShortDataNames(name);
            rVariables.append(name);
            if (addGenerationSpecifier)
            {
                rVariables.last().append(generationspecifier::separatorString+QString::number(toDisplayGeneration(var->getGeneration())));
            }
        }
    }
    // Do more costly name lookup, generate a list of all variables and all generations
    else if (desiredGen == generationspecifier::allGenerations)
    {
        QRegExp regexp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard);
        QList<SharedVectorVariableT> alias_variables = pLogDataHandler->getMatchingVariablesFromAllGenerations(regexp, Alias);
        for (auto &var : alias_variables)
        {
            QString name = var->getAliasName();
            toShortDataNames(name);
            rVariables.append(name);
            if (addGenerationSpecifier)
            {
                rVariables.last().append(generationspecifier::separatorString+QString::number(toDisplayGeneration(var->getGeneration())));
            }
        }
        QList<SharedVectorVariableT> full_variables = pLogDataHandler->getMatchingVariablesFromAllGenerations(regexp, FullName);
        for (auto &var : full_variables)
        {
            QString name = var->getFullVariableName();
            toShortDataNames(name);
            rVariables.append(name);
            if (addGenerationSpecifier)
            {
                rVariables.last().append(generationspecifier::separatorString+QString::number(toDisplayGeneration(var->getGeneration())));
            }
        }
    }
    // Else generation number was not specified, lookup names at all generations
    else if (desiredGen == generationspecifier::noGenerationSpecified)
    {
        QRegExp regexp(pattern_long, Qt::CaseSensitive, QRegExp::Wildcard);
        QList<SharedVectorVariableT> alias_variables = pLogDataHandler->getMatchingVariablesAtRespectiveNewestGeneration(regexp, Alias);
        for (auto &var : alias_variables)
        {
            QString name = var->getAliasName();
            toShortDataNames(name);
            rVariables.append(name);
            if (addGenerationSpecifier)
            {
                rVariables.last().append(generationspecifier::separatorString+QString::number(toDisplayGeneration(var->getGeneration())));
            }
        }
        QList<SharedVectorVariableT> full_variables = pLogDataHandler->getMatchingVariablesAtRespectiveNewestGeneration(regexp, FullName);
        for (auto &var : full_variables)
        {
            QString name = var->getFullVariableName();
            toShortDataNames(name);
            rVariables.append(name);
            if (addGenerationSpecifier)
            {
                rVariables.last().append(generationspecifier::separatorString+QString::number(toDisplayGeneration(var->getGeneration())));
            }
        }
    }
    //timer.toc("getMatchingLogVariableNames("+pattern+")");
}

//! @brief Parses the generation specifier of a name and chops it from the name
//! @param[in,out] rStr A reference to the name string, the generation specifier will be chopped
//! @param[out] rOk A bool describing if parsing was successful
//! @returns The desired generation, or: -1 = Current generation, -2 = All generations, -3 = Not Specified, -4 on failure
int HcomHandler::parseAndChopGenerationSpecifier(QString &rStr, bool &rOk) const
{
    rOk = true;
    const int i = rStr.lastIndexOf(generationspecifier::separatorString);
    const bool generationStringFound = (i > -1);
    if (generationStringFound && mpModel)
    {
        QString genStr = rStr.right(rStr.size()-i-1);

        if( (genStr == "l") || (genStr == "L") )
        {
            rStr.chop(2);
            return mpModel->getLogDataHandler()->getLowestGenerationNumber();
        }
        else if( (genStr == "h") || (genStr == "H") )
        {
            rStr.chop(2);
            return mpModel->getLogDataHandler()->getHighestGenerationNumber();
        }
        else if( (genStr == "*") || (genStr == "a") || (genStr == "A") )
        {
            rStr.chop(2);
            return generationspecifier::allGenerations;
        }
        else if( (genStr == "c") || (genStr == "C") )
        {
            rStr.chop(2);
            return generationspecifier::currentGeneration;
        }
        else if(genStr.startsWith("(") && genStr.endsWith(")"))
        {
            //! @todo I removed l and h replacing here as itwas replacing every l/h in the string, need to solve this in some smarter way
            rStr.chop(genStr.size()+1);
            SymHop::Expression expr(genStr);
            double g = expr.evaluate(mLocalVars, &mLocalFunctionoidPtrs);
            return fromDisplayGeneration(int(g+0.5));
        }
        else
        {
            // Handle a specific generation number
            int g;
            if (toInt(genStr, g))
            {
                rStr.truncate(i);
                // Handle negative indexes ( -1 would mean the previous one: current-1 )
                if (g < 0) {
                    g = mpModel->getLogDataHandler()->getCurrentGenerationNumber() + g;
                }
                // Special handling of zero (treat it the same as generation one) but in the data this will still be generation zero
                else if (g == 0) {
                    // We cant use 0 here as that would subtract to -1 which would mean currentGeneration
                    g = fromDisplayGeneration(1);
                }
                // Handle a particular generation number
                else {
                    g = fromDisplayGeneration(g);
                }

                // Make sure we do not request a generation that does not exist
                if (g < mpModel->getLogDataHandler()->getLowestGenerationNumber()) {
                    return generationspecifier::invalidGeneration;
                }

                return qMax(g, 0);
            }
            else
            {
                rOk = false;
                return generationspecifier::invalidGeneration;
            }
        }
    }

    return generationspecifier::noGenerationSpecified;
}

//! @brief Help function that returns a list of variables according to input (with support for asterisks)
//! @param str String to look for
//! @param variables Reference to list of found variables
void HcomHandler::getLogVariablesThatStartsWithString(const QString str, QStringList &variables) const
{
    if(!mpModel) { return; }

    SystemObject *pSystem = mpModel->getViewContainerObject();
    QStringList names = mpModel->getLogDataHandler()->getVariableFullNames();
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

QString HcomHandler::getFunctionName(QString expression) const {
    QString funcName = expression.left(expression.indexOf("("));
    if(expression.contains("(") && isHcomFunctionCall(funcName, expression)) {
        return funcName;
    }
    return QString();
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
//! @returns True if it is a correct expression, otherwise false
bool HcomHandler::evaluateArithmeticExpression(QString cmd)
{
    //cmd.replace("**", "%%%%%");

    if(cmd.endsWith("*")) { return false; }

    SymHop::Expression expr = SymHop::Expression(cmd, 0, SymHop::Expression::NoSimplifications);

    //Assignment  (handle separately to update local variables not known to SymHop)
    if(expr.isAssignment())
    {
        QString left = expr.getLeft()->toString();

        QString right = expr.getRight()->toString();
        evaluateExpression(right);

        QStringList pars;
        if (mAnsType==Scalar || mAnsType==Wildcard)
        {
            getParameters(left, pars);
            SharedVectorVariableT data = getLogVariable(left);

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
            //! @todo this should be a help function
            if(!(left.at(i).isLetterOrNumber() || left.at(i) == '_' || left.at(i) == '.' || left.at(i) == ':' || left.at(i) == generationspecifier::separatorChar))
            {
                leftIsOk = false;
            }
        }

        if(!leftIsOk && (!mpModel || !getLogVariable(left)))
        {
            HCOMERR("Illegal variable name.");
            return false;
        }

        if(mAnsType == Wildcard && !pars.isEmpty())
        {
            executeCommand("chpa "+left+" "+mAnsWildcard);
            return true;
        }
        else if(mAnsType == Expression && pars.isEmpty()) {
            mLocalExpressions.insert(left, mAnsExpression);
            HCOMPRINT(mAnsExpression.toString());
            return true;
        }
        else if(mAnsType==Scalar)
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
            // Get log data
            SharedVectorVariableT leftData = getLogVariable(left);

            // If desired generation of desired variable exist then assign to it
            if (leftData && mAnsVector)
            {
                if (leftData->getVariableType() != mAnsVector->getVariableType())
                {
                    HCOMWARN(QString("Variable type missmatch: %1 != %2, you may have lost information!").arg(variableTypeAsString(leftData->getVariableType())).arg(variableTypeAsString(mAnsVector->getVariableType())));
                }
                if (leftData->getGeneration() != mAnsVector->getGeneration())
                {
                    HCOMWARN(QString("Variable generations missmatch %1 != %2 in assignment").arg(leftData->getGeneration()+1).arg(mAnsVector->getGeneration()+1));
                }
                leftData->assignFrom(mAnsVector);
                return true;
            }
            // Else lets create the variable or insert into it
            else if (mpModel && mAnsVector)
            {
                // Get rid of generation specifier from name
                bool parseOk;
                int gen = parseAndChopGenerationSpecifier(left, parseOk);
                if (parseOk)
                {
                    if (isSingleGenerationValue(gen) || (gen == generationspecifier::noGenerationSpecified))
                    {
                        if (gen == generationspecifier::noGenerationSpecified) {
                            gen = generationspecifier::currentGeneration;
                        }

                        // Value given but left does not exist (or at least generation does not exist), create/insert it
                        leftData = mpModel->getLogDataHandler()->insertNewVectorVariable(left, mAnsVector->getVariableType(), gen);
                        if (leftData)
                        {
                            if (leftData->getGeneration() != mAnsVector->getGeneration())
                            {
                                HCOMWARN(QString("Variable generations missmatch %1 != %2 in assignment").arg(leftData->getGeneration()+1).arg(mAnsVector->getGeneration()+1));
                            }

                            leftData->assignFrom(mAnsVector);
                            leftData->preventAutoRemoval();
                            mLocalVars.remove(left);    //Remove scalar variable with same name if it exists
                            return true;
                        }
                    }
                    else
                    {
                        HCOMERR(QString("Invalid generation value parsed: %1").arg(gen));
                    }
                }
                else
                {
                    HCOMERR(QString("Failed to parse generation specifier: %1").arg(left));
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
        //! @todo Should we allow pure expressions without assignment?
        //TicToc timer;
        evaluateExpression(cmd);
        //timer.toc("Evaluate expression "+cmd);
        if(mAnsType == Scalar)
        {
            HCOMPRINT(QString::number(mAnsScalar, 'g', 17));
            return true;
        }
        else if(mAnsType==DataVector)
        {
            HCOMPRINT(mAnsVector.data()->getFullVariableName());
            return true;
        }
        else if(mAnsType==Expression) {
            HCOMPRINT(mAnsExpression.toString());
            return true;
        }
        else if(mAnsType==String)
        {
            HCOMPRINT(mAnsWildcard);
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void HcomHandler::executeGtBuiltInFunction(QString functionCall)
{
    QStringList args = extractFunctionCallExpressionArguments(functionCall);
    if(args.size()==2)
    {
        const QString &arg1 = args[0];
        const QString &arg2 = args[1];

        bool arg1IsDouble=false, arg2IsDouble=false;
        double arg1AsDouble=0, arg2AsDouble=0;
        SharedVectorVariableT pVar1, pVar2;

        // Evaluate argument 1
        evaluateExpression(arg1);
        if (mAnsType == Scalar)
        {
            arg1AsDouble = mAnsScalar;
            arg1IsDouble = true;
        }
        else if (mAnsType == DataVector)
        {
            pVar1 = mAnsVector;
        }

        // Evaluate argument 2
        evaluateExpression(arg2);
        if (mAnsType == Scalar)
        {
            arg2AsDouble = mAnsScalar;
            arg2IsDouble = true;
        }
        else if (mAnsType == DataVector)
        {
            pVar2 = mAnsVector;
        }

        LogDataHandler2 *pLogDataHandler = 0;
        if(mpModel)
             pLogDataHandler = mpModel->getViewContainerObject()->getLogDataHandler().data();

        // Handle both scalars
        if (arg1IsDouble && arg2IsDouble)
        {
            mAnsType = Scalar;
            if (arg1AsDouble > arg2AsDouble)
            {
                mAnsScalar = 1;
            }
            else
            {
                mAnsScalar = 0;
            }
            return;
        }
        // Handle arg1 is double
        else if (arg1IsDouble && pVar2 && pLogDataHandler)
        {
            QVector<double> res;
            pVar2->elementWiseLt(res, arg1AsDouble);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar2->getSmartName()+QString("gt%1").arg(arg1AsDouble), pVar2->getVariableType());
            mAnsVector->assignFrom(pVar2->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        // Handle arg2 is double
        else if (arg2IsDouble && pVar1 && pLogDataHandler)
        {
            QVector<double> res;
            pVar1->elementWiseGt(res, arg2AsDouble);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar1->getSmartName()+QString("gt%1").arg(arg2AsDouble), pVar1->getVariableType());
            mAnsVector->assignFrom(pVar1->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        // Handle both vectors
        else if (pVar1 && pVar2 && pLogDataHandler)
        {
            //! @todo this assumes that both vectors have the same type
            if (pVar1->getDataSize() != pVar2->getDataSize())
            {
                HCOMERR("The vectors do not have the same length!");
                mAnsType = Undefined;
                return;
            }

            QVector<double> res;
            pVar1->elementWiseGt(res, pVar2);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar1->getSmartName()+"gt"+pVar2->getSmartName(), pVar1->getVariableType());
            mAnsVector->assignFrom(pVar1->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        else
        {
            HCOMERR("Could not find or evaluate one or both variable names");
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

void HcomHandler::executeLtBuiltInFunction(QString functionCall)
{
    QStringList args = extractFunctionCallExpressionArguments(functionCall);
    if(args.size()==2)
    {
        const QString &arg1 = args[0];
        const QString &arg2 = args[1];

        bool arg1IsDouble=false, arg2IsDouble=false;
        double arg1AsDouble=0, arg2AsDouble=0;
        SharedVectorVariableT pVar1, pVar2;

        // Evaluate argument 1
        evaluateExpression(arg1);
        if (mAnsType == Scalar)
        {
            arg1AsDouble = mAnsScalar;
            arg1IsDouble = true;
        }
        else if (mAnsType == DataVector)
        {
            pVar1 = mAnsVector;
        }

        // Evaluate argument 2
        evaluateExpression(arg2);
        if (mAnsType == Scalar)
        {
            arg2AsDouble = mAnsScalar;
            arg2IsDouble = true;
        }
        else if (mAnsType == DataVector)
        {
            pVar2 = mAnsVector;
        }

        LogDataHandler2 *pLogDataHandler = 0;
        if(mpModel)
            pLogDataHandler = mpModel->getViewContainerObject()->getLogDataHandler().data();

        // Handle both scalars
        if (arg1IsDouble && arg2IsDouble)
        {
            mAnsType = Scalar;
            if (arg1AsDouble < arg2AsDouble)
            {
                mAnsScalar = 1;
            }
            else
            {
                mAnsScalar = 0;
            }
            return;
        }
        // Handle arg1 is double
        else if (arg1IsDouble && pVar2 && pLogDataHandler)
        {
            QVector<double> res;
            pVar2->elementWiseGt(res, arg1AsDouble);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar2->getSmartName()+QString("lt%1").arg(arg1AsDouble), pVar2->getVariableType());
            mAnsVector->assignFrom(pVar2->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        // Handle arg2 is double
        else if (arg2IsDouble && pVar1 && pLogDataHandler)
        {
            QVector<double> res;
            pVar1->elementWiseLt(res, arg2AsDouble);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar1->getSmartName()+QString("lt%1").arg(arg2AsDouble), pVar1->getVariableType());
            mAnsVector->assignFrom(pVar1->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        // Handle both vectors
        else if (pVar1 && pVar2 && pLogDataHandler)
        {
            //! @todo this assumes that both vectors have the same type
            if (pVar1->getDataSize() != pVar2->getDataSize())
            {
                HCOMERR("The vectors do not have the same length!");
                mAnsType = Undefined;
                return;
            }

            QVector<double> res;
            pVar1->elementWiseLt(res, pVar2);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar1->getSmartName()+"lt"+pVar2->getSmartName(), pVar1->getVariableType());
            mAnsVector->assignFrom(pVar1->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        else
        {
            HCOMERR("Could not find or evaluate one or both variable names");
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

void HcomHandler::executeEqBuiltInFunction(QString functionCall)
{
    double eps = 1e-3;
    QStringList args = extractFunctionCallExpressionArguments(functionCall);

    if (args.size() == 3)
    {
        evaluateExpression(args[2]);
        if (mAnsType == Scalar)
        {
            eps = mAnsScalar;
        }
        else
        {
            HCOMERR("Could not parse eps argument");
            mAnsType = Undefined;
            return;
        }
    }

    if(args.size()>=2)
    {
        const QString &arg1 = args[0];
        const QString &arg2 = args[1];

        bool arg1IsDouble=false, arg2IsDouble=false;
        double arg1AsDouble=0, arg2AsDouble=0;
        SharedVectorVariableT pVar1, pVar2;

        // Evaluate argument 1
        evaluateExpression(arg1);
        if (mAnsType == Scalar)
        {
            arg1AsDouble = mAnsScalar;
            arg1IsDouble = true;
        }
        else if (mAnsType == DataVector)
        {
            pVar1 = mAnsVector;
        }

        // Evaluate argument 2
        evaluateExpression(arg2);
        if (mAnsType == Scalar)
        {
            arg2AsDouble = mAnsScalar;
            arg2IsDouble = true;
        }
        else if (mAnsType == DataVector)
        {
            pVar2 = mAnsVector;
        }

        LogDataHandler2 *pLogDataHandler = mpModel->getViewContainerObject()->getLogDataHandler().data();

        // Handle both scalars
        if (arg1IsDouble && arg2IsDouble)
        {
            mAnsType = Scalar;
            if (fuzzyEqual(arg1AsDouble, arg2AsDouble, eps))
            {
                mAnsScalar = 1;
            }
            else
            {
                mAnsScalar = 0;
            }
            return;
        }
        // Handle arg1 is double
        else if (arg1IsDouble && pVar2)
        {
            QVector<double> res;
            pVar2->elementWiseEq(res, arg1AsDouble, eps);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar2->getSmartName()+QString("eq%1").arg(arg1AsDouble), pVar2->getVariableType());
            mAnsVector->assignFrom(pVar2->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        // Handle arg2 is double
        else if (arg2IsDouble && pVar1)
        {
            QVector<double> res;
            pVar1->elementWiseEq(res, arg2AsDouble, eps);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar1->getSmartName()+QString("eq%1").arg(arg2AsDouble), pVar1->getVariableType());
            mAnsVector->assignFrom(pVar1->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        // Handle both vectors
        else if (pVar1 && pVar2)
        {
            //! @todo this assumes that both vectors have the same type
            if (pVar1->getDataSize() != pVar2->getDataSize())
            {
                HCOMERR("The vectors do not have the same length!");
                mAnsType = Undefined;
                return;
            }

            QVector<double> res;
            pVar1->elementWiseEq(res, pVar2, eps);
            mAnsVector = pLogDataHandler->createOrphanVariable(pVar1->getSmartName()+"eq"+pVar2->getSmartName(), pVar1->getVariableType());
            mAnsVector->assignFrom(pVar1->getSharedTimeOrFrequencyVector(), res);
            mAnsType = DataVector;
            return;
        }
        else
        {
            HCOMERR("Could not find or evaluate one or both variable names");
            mAnsType = Undefined;
            return;
        }
    }
    else
    {
        HCOMERR(QString("Wrong number of arguments provided for eq function.\n"+mLocalFunctionDescriptions.find("lt").value().second));
        mAnsType = Undefined;
        return;
    }
}

void HcomHandler::executeCutBuiltInFunction(QString functionCall)
{
    QStringList args = extractFunctionCallExpressionArguments(functionCall);
    if(args.size() != 2) {
        HCOMERR("Wrong number of arguments (should be 2)");
        mAnsType = Undefined;
        return;
    }

    //Find first vector
    evaluateExpression(args[0], DataVector);
    if (mAnsType != DataVector)
    {
        mAnsType = Undefined;
        HCOMERR(QString("Variable: %1 was not found!").arg(args[0]));
        return;
    }
    SharedVectorVariableT pVar1 = mAnsVector;

    //Find second vector
    evaluateExpression(args[1], DataVector);
    if (mAnsType != DataVector)
    {
        HCOMERR(QString("Variable: %1 was not found!").arg(args[1]));
        mAnsType = Undefined;
        return;
    }
    SharedVectorVariableT pVar2 = mAnsVector;

    if(pVar1->getDataSize() != pVar2->getDataSize()) {
        HCOMERR("Variables must be of same length.");
        mAnsType = Undefined;
        return;
    }

    VariableTypeT type1 = pVar1->getVariableType();
    VariableTypeT type2 = pVar2->getVariableType();
    if(type2 == ComplexType) {
        HCOMERR("Second variable must not be of complex type.");
        mAnsType = Undefined;
        return;
    }

    mAnsVector = mpModel->getLogDataHandler()->createOrphanVariable(pVar1->getFullVariableName()+"_cut", type1);

    QVector<double> data2 = pVar2->getDataVectorCopy();

    if(type1 == VectorType) {
        QVector<double> data1 = pVar1->getDataVectorCopy();
        for(int i=0; i<data1.size(); ++i) {
            if(data2[i] > 0.5) {
                mAnsVector->append(data1[i]);
            }
        }
    }
    else if(type1 == TimeDomainType || type1 == FrequencyDomainType) {
        QVector<double> data1 = pVar1->getDataVectorCopy();
        QVector<double> time = pVar1->getSharedTimeOrFrequencyVector()->getDataVectorCopy();
        for(int i=0; i<data1.size(); ++i) {
            if(data2[i] > 0.5) {
                mAnsVector->append(time[i], data1[i]);
            }
        }
    }
    else {
        HCOMERR("Unsupported data type for first variable.");
        mAnsType = Undefined;
        return;
    }

    mAnsType = DataVector;
    return;
}

void HcomHandler::executeSsiBuiltInFunction(QString functionCall)
{
    QStringList args = extractFunctionCallExpressionArguments(functionCall);

    //Find vector
    evaluateExpression(args[0], DataVector);
    if (mAnsType != DataVector)
    {
        mAnsType = Undefined;
        HCOMERR(QString("Variable: %1 was not found!").arg(args[0]));
        return;
    }

    bool ok;
    int methodArg = getNumber(args[1], &ok);
    if(!ok) {
        HCOMERR("Argument 2 should be a numerical value");
        mAnsType = Undefined;
        return;
    }

    double tol = getNumber(args[2], &ok);
    if(!ok) {
        HCOMERR("Argument 3 should be a numerical value");
        mAnsType = Undefined;
        return;
    }

    double win = 0;
    double stdev = 0;
    double l1 = 0;
    double l2 = 0;
    double l3 = 0;
    SteadyStateIdentificationMethodEnumT method;
    if(methodArg == 0) {
        if(args.size() != 4) {
            HCOMERR("Wrong number of arguments (should be 4)");
            mAnsType = Undefined;
            return;
        }
        method = RectangularWindowTest;
        win = getNumber(args[3], &ok);
        if(!ok) {
            HCOMERR("Argument 4 should be a numerical value");
            mAnsType = Undefined;
            return;
        }
    }
    else if(methodArg == 1) {
        if(args.size() != 5) {
            HCOMERR("Wrong number of arguments (should be 5)");
            mAnsType = Undefined;
            return;
        }
        method = VarianceRatioTest;
        win = getNumber(args[3], &ok);
        if(!ok) {
            HCOMERR("Argument 4 should be a numerical value");
            mAnsType = Undefined;
            return;
        }

        stdev = getNumber(args[4], &ok);
        if(!ok) {
            HCOMERR("Argument 5 should be a numerical value");
            mAnsType = Undefined;
            return;
        }
    }
    else if(methodArg == 2) {
        if(args.size() != 7) {
            HCOMERR("Wrong number of arguments (should be 7)");
            mAnsType = Undefined;
            return;
        }
        method = MovingAverageVarianceRatioTest;
        l1 = getNumber(args[3], &ok);
        if(!ok) {
            HCOMERR("Argument 4 should be a numerical value");
            mAnsType = Undefined;
            return;
        }

        l2 = getNumber(args[4], &ok);
        if(!ok) {
            HCOMERR("Argument 5 should be a numerical value");
            mAnsType = Undefined;
            return;
        }

        l3 = getNumber(args[5], &ok);
        if(!ok) {
            HCOMERR("Argument 6 should be a numerical value");
            mAnsType = Undefined;
            return;
        }

        stdev = getNumber(args[6], &ok);
        if(!ok) {
            HCOMERR("Argument 7 should be a numerical value");
            mAnsType = Undefined;
            return;
        }
    }

    SharedVectorVariableT pRetVar = mAnsVector->identifySteadyState(method, tol, win, stdev, l1, l2, l3);
    if(pRetVar == nullptr) {
        mAnsType = Undefined;
        return;
    }
    else {
        mAnsVector = pRetVar;
        mAnsType = DataVector;
        return;
    }
}



//! @brief Returns data pointers for the variable with given full short name format (may include generation)
//! @param fullShortName Full concatenated name of the variable, (short name format expected)
//! @returns Pointers to the data variable and container
SharedVectorVariableT HcomHandler::getLogVariable(QString fullShortName) const
{
    if(!mpModel)
    {
        return SharedVectorVariableT();
    }

    QString warningMessage;

    // If no generation is specified we want to use the current generation
    int generation = mpModel->getLogDataHandler()->getCurrentGenerationNumber();

    // Now try to parse and separate the generation  number from the name
    bool parseGenOK;
    int genRC = parseAndChopGenerationSpecifier(fullShortName, parseGenOK);
    // Handle parse failure
    if (!parseGenOK)
    {
        warningMessage = QString("Could not parse generation specifier in: %2, choosing current").arg(fullShortName);
    }
    // Handle ALL generations return code
    else if(genRC == generationspecifier::allGenerations)
    {
        warningMessage = "Could not parse a unique generation number, choosing current generation";
    }
    // Use given generation number
    else if (isSingleExplicitGenerationValue(genRC))
    {
        generation = genRC;
    }
    // Note! genRC = -1 (current) and -3 (not specified) will be handled as if current is chosen (without warning)

    // Convert to long name
    toLongDataNames(fullShortName);

    // Try to retrieve data
    SharedVectorVariableT data = mpModel->getLogDataHandler()->getVectorVariable(fullShortName, generation);

    // If data was found then also print any warnings from the generation parser, otherwise do not print anything and
    // let a higher level function deal with printing warnings or errors regarding data that was not found
    if (data && !warningMessage.isEmpty())
    {
        HCOMWARN(warningMessage);
    }

    // Now return the data
    return data;
}

QStringList HcomHandler::getAutoCompleteWords() const
{
    QStringList ret;

    //HCOM commands
    QStringList commands = getCommands();
    for(QString &command : commands)
    {
      command.append(" ");
    }
    ret << commands;

    //Parameters
    QStringList parameters;

    getParameters("*", parameters);
    ret << parameters;

    //Local variables
    ret << getLocalVariables().keys();

    //Local functions
    QStringList functions = mLocalFunctionDescriptions.keys();
    for(QString &function : functions)
    {
      function.append("()");
    }
    ret << functions;

    //Local functionoids
    QStringList functionoids = mLocalFunctionoidPtrs.keys();
    for(QString &functionoid : functionoids)
    {
      functionoid.append("()");
    }
    ret << functions;

    //Log variables
    QStringList variables;
    getMatchingLogVariableNames(QString("*%1H").arg(generationspecifier::separatorString), variables, false);
    ret <<  variables;


    //Clean up and sort alphabetically
    ret.removeDuplicates();
    ret.sort();

    return ret;
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


//! @brief Converts long data names to short data names (e.g. "Sysname$Component#Port#Pressure" -> "Sysname|Component.Port.p")
//! @param rName Reference to variable name string
void HcomHandler::toShortDataNames(QString &rName) const
{
    rName.replace("#",".");
    rName.replace("$","|");

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
}


//! @brief Converts short data names to long data names (e.g. "Sysname|Component.Port.p" -> "Sysname$Component#Port#Pressure")
void HcomHandler::toLongDataNames(QString &rName) const
{
    rName.replace(".", "#");
    rName.replace("|", "$");
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
}


//! @brief Converts a command to a directory path, or returns an empty string if command is invalid
QString HcomHandler::getDirectory(const QString &cmd) const
{
    if(QDir().exists(QDir::cleanPath(mPwd+"/"+cmd)))
    {
        return QDir::cleanPath(mPwd+"/"+cmd);
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


//! @brief Slot that aborts any HCOM script currently running
void HcomHandler::abortHCOM()
{
    mAborted = true;
    emit aborted();
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
    SharedVectorVariableT pData = mpHandler->getLogVariable(str);
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
    QStringList splitStr;
    splitRespectingQuotationsAndParanthesis(str, ',', splitStr);

    if(splitStr.size() != 2)
    {
        mpHandler->mpConsole->printErrorMessage(QString("Wrong number of arguments for peek function!"), "", false);
        ok = false;
        return 0;
    }

    SharedVectorVariableT pData = mpHandler->getLogVariable(splitStr[0]);

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

    QString idxStr = splitStr[1];

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
    SharedVectorVariableT pData = mpHandler->getLogVariable(str);

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


//! @brief Function operator for the "exists" functionoid
double HcomFunctionoidExists::operator()(QString &str, bool &ok)
{
    ok = true;
    auto modelPtr = mpHandler->getModelPtr();
    if(modelPtr) {
        auto system = modelPtr->getTopLevelSystemContainer();
        if(system && system->hasModelObject(str))
        {
            return 1;
        }
    }

    return 0;
}


//! @brief Function operator for the "count" functionoid
double HcomFunctionoidCount::operator()(QString &str, bool &ok)
{
    ok = true;
    int count = 0;
    auto modelPtr = mpHandler->getModelPtr();
    if(modelPtr) {
        auto system = modelPtr->getTopLevelSystemContainer();
        if(system) {
            for(const auto &comp : system->getModelObjects()) {
                if(comp->getTypeName() == str) {
                    ++count;
                }
            }
        }
    }
    return count;
}


//! @brief Function operator for the "obj" functionoid
//! @todo Should be renamed to "optObj"
double HcomFunctionoidObj::operator()(QString &str, bool &ok)
{
    int idx = str.toDouble();
    ok=true;
    return mpHandler->mpOptHandler->getObjectiveValue(idx);
}


//! @brief Function operator for the "min" functionoid
double HcomFunctionoidMin::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str);
    if(!pData)
    {
        QStringList args;
        splitRespectingQuotationsAndParanthesis(str, ',', args);
        double min=std::numeric_limits<double>::max();
        for(QString &arg : args)
        {
            mpHandler->evaluateExpression(arg);
            double new_val;
            if(mpHandler->mAnsType == HcomHandler::Scalar)
            {
                new_val = mpHandler->mAnsScalar;
            }
            else if (mpHandler->mAnsType == HcomHandler::DataVector)
            {
                new_val = mpHandler->mAnsVector->minOfData();
            }
            else
            {
                mpHandler->mpConsole->printErrorMessage(QString("%1 could not be evaluated").arg(arg), "", false);
                ok = false;
                return 0;
            }

            if(new_val < min)
            {
                min = new_val;
            }
        }
        ok=true;
        return min;
    }
    else
    {
        ok=true;
        return(pData->minOfData());
    }
}


//! @brief Function operator for the "max" functionoid
double HcomFunctionoidMax::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str);
    if(!pData)
    {
        QStringList args;
        splitRespectingQuotationsAndParanthesis(str, ',', args);
        double max=-std::numeric_limits<double>::max();
        for(QString &arg : args)
        {
            mpHandler->evaluateExpression(arg);
            double new_val;
            if(mpHandler->mAnsType == HcomHandler::Scalar)
            {
                new_val = mpHandler->mAnsScalar;
            }
            else if (mpHandler->mAnsType == HcomHandler::DataVector)
            {
                new_val = mpHandler->mAnsVector->maxOfData();
            }
            else
            {
                mpHandler->mpConsole->printErrorMessage(QString("%1 could not be evaluated").arg(arg), "", false);
                ok = false;
                return 0;
            }

            if(new_val > max)
            {
                max = new_val;
            }
        }
        ok=true;
        return max;
    }
    else
    {
        ok=true;
        return(pData->maxOfData());
    }
}


//! @brief Function operator for the "imin" functionoid
double HcomFunctionoidIMin::operator()(QString &str, bool &ok)
{
    SharedVectorVariableT pData = mpHandler->getLogVariable(str);

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
    SharedVectorVariableT pData = mpHandler->getLogVariable(str);

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
    if(mpHandler->mpOptHandler->isRunning())
    {
        return mpHandler->mpOptHandler->getCandidateParameter(pointIdx,parIdx);
    }
    else
    {
        return mpHandler->mpOptHandler->getParameter(pointIdx,parIdx);  //Hack for applying parameters
    }
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
    double scalar1=0, scalar2=0;
    SharedVectorVariableT pData1 = mpHandler->getLogVariable(args.first());
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


    SharedVectorVariableT pData2 = mpHandler->getLogVariable(args[1]);
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
        //! @todo maybe support this as a element wise comparison
        mpHandler->mpConsole->printWarningMessage("Comparing scalar with vector (will fail)", "", false);
    }

    // Else they are not the same
    return 0;
}



double HcomFunctionoidMaxPar::operator()(QString &str, bool &ok)
{
    QStringList args = str.split(',');
    if (args.size() != 2)
    {
        ok = false;
        mpHandler->mpConsole->printErrorMessage("Wrong number of arguments", "", false);
        return 0;
    }

    double ans=-1e100;

    QString typeName = args[0];
    QString par = args[1];
    mpHandler->toLongDataNames(par);

    ok=true;
    QList<ModelObject *> compPtrs = mpHandler->getModelPtr()->getViewContainerObject()->getModelObjects();
    for(ModelObject *comp : compPtrs)
    {
        if(comp->getTypeName() == typeName || comp->getAppearanceData()->getFullTypeName("") == typeName)
        {
            ok=true;
            QString strValue = comp->getParameterValue(par);
            bool ok2;
            double value = strValue.toDouble(&ok2);
            if(!ok2)
            {
                strValue = mpHandler->getModelPtr()->getViewContainerObject()->getParameterValue(strValue);
                value = strValue.toDouble();
            }
            if(value > ans)
            {
                ans = value;
            }
        }
    }

    return ans;
}

double HcomFunctionoidMinPar::operator()(QString &str, bool &ok)
{
    QStringList args = str.split(',');
    if (args.size() != 2)
    {
        ok = false;
        mpHandler->mpConsole->printErrorMessage("Wrong number of arguments", "", false);
        return 0;
    }

    double ans=1e100;

    QString typeName = args[0];
    QString par = args[1];
    mpHandler->toLongDataNames(par);

    ok=true;
    QList<ModelObject *> compPtrs = mpHandler->getModelPtr()->getViewContainerObject()->getModelObjects();
    for(ModelObject *comp : compPtrs)
    {
        if(comp->getTypeName() == typeName || comp->getAppearanceData()->getFullTypeName("") == typeName)
        {
            ok=true;
            QString strValue = comp->getParameterValue(par);
            bool ok2;
            double value = strValue.toDouble(&ok2);
            if(!ok2)
            {
                strValue = mpHandler->getModelPtr()->getViewContainerObject()->getParameterValue(strValue);
                value = strValue.toDouble();
            }
            if(value < ans)
            {
                ans = value;
            }
        }
    }

    return ans;
}


double HcomFunctionoidHg::operator()(QString &str, bool &ok)
{
    Q_UNUSED(str);

    if(!mpHandler->getModelPtr()) {
        ok = false;
        mpHandler->mpConsole->printErrorMessage("No model is open", "", false);
        return 0;
    }
    ok = true;
    return mpHandler->getModelPtr()->getLogDataHandler()->getHighestGenerationNumber()+1;
}

double HcomFunctionoidAns::operator()(QString &str, bool &ok)
{
    Q_UNUSED(str);
    if(mpHandler->mAnsType != HcomHandler::Scalar) {
        ok = false;
        mpHandler->mpConsole->printErrorMessage("ans() function only work with scalar computations.");
        return 0;
    }
    ok = true;
    return mpHandler->mAnsScalar;
}
