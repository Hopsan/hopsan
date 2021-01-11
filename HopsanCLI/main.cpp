/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCLI/main.cpp
//! @brief The HopsanCLI main file
//!

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

#include "ModelUtilities.h"
#include "HopsanEssentials.h"
#include "HopsanCoreMacros.h"
#include "TicToc.hpp"
#include "version_cli.h"
#include "CoreUtilities/SaveRestoreSimulationPoint.h"

#include "CliUtilities.h"
#include "ModelValidation.h"
#include "BuildUtilities.h"

#ifdef USEOPS
#include "OpsWorker.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include "OpsWorkerNelderMead.h"
#include "OpsWorkerComplexRF.h"
#include "OpsWorkerComplexRFP.h"
#include "OpsWorkerComplexBurmen.h"
#include "OpsWorkerControlledRandomSearch.h"
#include "OpsWorkerDifferentialEvolution.h"
#include "OpsWorkerParticleSwarm.h"
#include "OpsWorkerParameterSweep.h"
#include "OpsWorkerGenetic.h"
#endif

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE SHAREDLIB_PREFIX "defaultcomponentlibrary" HOPSAN_DEBUG_POSTFIX "." SHAREDLIB_SUFFIX
const std::string default_library = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
#else
const std::string default_library = "";
#endif

using namespace std;
using namespace hopsan;

HopsanEssentials gHopsanCore;

#ifdef USEOPS
class OptimizationMessageHandler : public Ops::MessageHandler
{
public:
    OptimizationMessageHandler(size_t maxEvals, bool silent=false)
    {
        mMaxEvals = maxEvals;
        mSilent = silent;
    }

    void setWorker(Ops::Worker *pWorker)
    {
        mpWorker = pWorker;
    }
    void printMessage(const char *msg) { cout << msg << endl; }
    void objectiveChanged(size_t idx)
    {
        (void)idx;
       // cout << "Objective " << idx << " changed: " << mpWorker->getObjectiveValue(idx) << endl;
    }
    void stepCompleted(size_t step)
    {
        if(mSilent) return;
        double best = mpWorker->getObjectiveValue(mpWorker->getBestId());
        double worst = mpWorker->getObjectiveValue(mpWorker->getWorstId());
        cout << "Step: " << step << "/" << mMaxEvals << " Best: " << best << " Worst: " << worst << endl;
    }

private:
    Ops::Worker *mpWorker;
    size_t mMaxEvals;
    bool mSilent;
};

class OptimizationEvaluator : public Ops::Evaluator
{
public:
    OptimizationEvaluator(vector<ComponentSystem *> rootSystemPtrs,
                          vector<string> parNames,
                          vector<string> objComps,
                          vector<string> objPorts,
                          vector<double> objWeights,
                          vector<double> parMin,
                          vector<double> parMax,
                          double startTime,
                          double stopTime)
    {
        mRootSystemPtrs = rootSystemPtrs;
        mParNames = parNames;
        mObjComps = objComps;
        mObjPorts = objPorts;
        mObjWeights = objWeights;
        mParMin = parMin;
        mParMax = parMax;
        mEvaulationCounter = 0;
        mStartTime = startTime;
        mStopTime = stopTime;
    }

    //! @todo Make evaluateAll... work in parallel

    //void evaluateAllPoints();
    void evaluateCandidate(size_t idx)
    {
        for(size_t i=0; i<mpWorker->getNumberOfParameters(); ++i)
        {
            double par = mpWorker->getCandidateParameter(idx, i);
            if(!mRootSystemPtrs.at(0)->setParameterValue(HString(mParNames[i].c_str()), HString(std::to_string(par).c_str())))
            {
                cout << "Error: Parameter " << mParNames[i] << " not found in model." << endl;
            }
        }

        mRootSystemPtrs.at(0)->initialize(mStartTime,mStopTime);
        mRootSystemPtrs.at(0)->simulate(mStopTime);

        double obj = 0.0;
        for(size_t i=0; i<mObjComps.size(); ++i)
        {
            int portId = 0;
            Component *pComp = mRootSystemPtrs.at(0)->getSubComponent(mObjComps[i].c_str());
            Port *pPort = pComp->getPort(mObjPorts[i].c_str());
            double data = *pPort->getNodeDataPtr(portId);
            obj += mObjWeights[i]*data;
        }
        mpWorker->setCandidateObjectiveValue(idx, obj);

        ++mEvaulationCounter;
    }

    void evaluateAllCandidates()
    {
        for(size_t c=0; c<mpWorker->getNumberOfCandidates(); ++c)
        {
            for(size_t i=0; i<mpWorker->getNumberOfParameters(); ++i)
            {
                double par = mpWorker->getCandidateParameter(c, i);
                if(!mRootSystemPtrs.at(c)->setParameterValue(HString(mParNames[i].c_str()), HString(std::to_string(par).c_str())))
                {
                    cout << "Error: Parameter " << mParNames[i] << " not found in model." << endl;
                }
            }
        }

        int  threads = mpWorker->getNumberOfCandidates();
        if(threads == 1)
        {
            threads = -1;
        }

        gHopsanCore.getSimulationHandler()->initializeSystem(mStartTime,mStopTime,mRootSystemPtrs);
        gHopsanCore.getSimulationHandler()->simulateSystem(mStartTime,mStopTime,threads,mRootSystemPtrs);

        for(size_t c=0; c<mpWorker->getNumberOfCandidates(); ++c)
        {
            double obj = 0.0;
            for(size_t i=0; i<mObjComps.size(); ++i)
            {
                int portId = 0;
                Component *pComp = mRootSystemPtrs.at(c)->getSubComponent(mObjComps[i].c_str());
                Port *pPort = pComp->getPort(mObjPorts[i].c_str());
                double data = *pPort->getNodeDataPtr(portId);
                obj += mObjWeights[i]*data;
            }
            mpWorker->setCandidateObjectiveValue(c, obj);
            ++mEvaulationCounter;
        }
    }

    size_t getNumberOfEvaluations() { return mEvaulationCounter; }

    //void evaluateAllCandidates();
private:
    vector<ComponentSystem *> mRootSystemPtrs;
    vector<string> mParNames;
    vector<string> mObjComps;
    vector<string> mObjPorts;
    vector<double> mObjWeights;
    vector<double> mParMin;
    vector<double> mParMax;
    size_t mEvaulationCounter;
    double mStartTime;
    double mStopTime;
};
#endif

int main(int argc, char *argv[])
{
    bool returnSuccess=false;
    try {
        TCLAP::CmdLine cmd("HopsanCLI", ' ', HOPSANCLIVERSION);

        // Define a value argument and add it to the command line.
        //TCLAP::ValueArg<std::string> saveNodesPathsOption("n", "savenodes", "A file containing lines with the ComponentName;PortName to save node data from", false, "", "FilePath string", cmd);
        TCLAP::SwitchArg testInstanciateComponentsOption("", "testInstanciateComponents", "Create an instance of each registered component to look for errors.", cmd);
        TCLAP::SwitchArg endPauseOption("", "endPause", "Pauses the CLI at the end to let you see its output", cmd);
        TCLAP::SwitchArg printDebugOption("", "printDebug", "Show debug messages in the output", cmd);
        TCLAP::SwitchArg silentOption("", "silent", "Disable all output messages", cmd);
        TCLAP::SwitchArg createHvcTestOption("", "createValidationData","Create a model validation data set based on the variables connected to scopes in the model given by option -m", cmd);
        TCLAP::SwitchArg prefixRootLevelName("", "prefixRootSystemName", "Prefix the root-level system name to exported results and parameters", cmd);

        TCLAP::ValueArg<std::string> coreLogFileOption("", "log.corelogfile", "The simulation core log file destination", false, "", "Filepath", cmd);
        TCLAP::ValueArg<std::string> buildCompLibOption("", "buildComponentLibrary", "Build the specified component library (point to the library xml)", false, "", "string", cmd);
        TCLAP::ValueArg<std::string> destinationOption("d","destination","Destination for resulting files",false,"","Path to directory", cmd);
        TCLAP::ValueArg<std::string> saveSimulationStateOption("", "saveSimState", "Export the simulation state to this file", false, "Path to file", "string", cmd);
        TCLAP::ValueArg<std::string> loadSimulationStateOption("", "loadSimState", "Load the simulation state (with time offset) from this file", false, "Path to file", "string", cmd);
        TCLAP::ValueArg<std::string> loadSimulationSVOption("", "loadSimStartValues", "Load the start values (simulation state without time offset) from this file", false, "Path to file", "string", cmd);
        TCLAP::ValueArg<std::string> resultsCSVSortOption("", "resultsCSVSort", "Export results in columns or in rows: [rows, cols]", false, "rows", "string", cmd);
        TCLAP::ValueArg<std::string> resultsFinalCSVOption("", "resultsFinalCSV", "Export the results (only final values)", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> resultsFullCSVOption("", "resultsFullCSV", "Export the results (all logged data) to CSV", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> resultsFinalHDF5Option("", "resultsFinalHDF5", "Exeport the results (only final values) to HDF5", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> resultsFullHDF5Option("", "resultsFullHDF5", "Exeport the results (all logged data) to HDF5", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> parameterExportOption("", "parameterExport", "CSV file with exported parameter values", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> parameterImportOption("", "parameterImport", "CSV file with parameter values to import", false, "", "Path to file", cmd);
        TCLAP::ValueArg<std::string> hvcTestOption("t","validate","Perform model validation based on HopsanValidationConfiguration",false,"","Path to .hvc file", cmd);
        TCLAP::ValueArg<std::string> nLogSamplesOption("l","numLogSamples","Set the number of log samples to store for the top-level system, (default: Use number in .hmf)",false,"","integer", cmd);
        TCLAP::ValueArg<std::string> logonlyOption("","logonly","If specified, log only given ports or variables. Can be a file (one full port/variable name per line) or coma separated list.",false,"","string", cmd);
        TCLAP::ValueArg<std::string> simulateOption("s","simulate","Specify simulation time as: [hmf] or [start,ts,stop] or [ts,stop] or [stop]",false,"","Comma separated string", cmd);
        TCLAP::ValueArg<std::string> parallelOption("p","parallel","Enable parallel simulation with specified number of threads. 0 threads  means auto-detect number of procssors.",false,"0","integer", cmd);
        TCLAP::ValueArg<std::string> extLibsFileOption("","externalLibsFile","A text file containing the external libs to load",false,"","Path to file", cmd);
        TCLAP::MultiArg<std::string> extLibPathsOption("e","externalLib","Path to a .dll/.so/.dylib externalComponentLib. Can be given multiple times",false,"Path to file", cmd);
        TCLAP::MultiArg<std::string> optimizationOption("o","optScript","Optimization scripts",false,"Path to files", cmd);
        TCLAP::MultiArg<std::string> optimizationSettings("","optSettings","Optimization settings",false,"Settings", cmd);
        TCLAP::ValueArg<std::string> hmfPathOption("m","hmf","The Hopsan model file to load",false,"","Path to file", cmd);

        // Parse the argv array.
        cmd.parse( argc, argv );

        std::string destinationPath = destinationOption.getValue();
        if (!destinationPath.empty())
        {
            if ( destinationPath[destinationPath.size()-1] != '/')
            {
                destinationPath.push_back('/');
            }
        }

        if (buildCompLibOption.isSet())
        {
            string output;
            cout << "Building component library: " << buildCompLibOption.getValue() << endl;
            returnSuccess = buildComponentLibrary(buildCompLibOption.getValue(), output);
            if (!returnSuccess)
            {
                printErrorMessage("Failed to build component library: "+buildCompLibOption.getValue(), silentOption.getValue());
            }
        }

        if (coreLogFileOption.isSet()) {
            gHopsanCore.openCoreLogFile(coreLogFileOption.getValue().c_str());
        } else {
            gHopsanCore.openCoreLogFile("hopsan_logfile.txt");
        }

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
        // Load default Hopsan component lib
        string libpath = getCurrentExecPath()+"/"+default_library;
        gHopsanCore.loadExternalComponentLib(libpath.c_str());
#endif
        // Print initial core messages
        printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());

        // Load external libs
        vector<string> externalComponentLibraries;
        // Check extLibs file options
        if (extLibsFileOption.isSet())
        {
            readExternalLibsFromTxtFile(extLibsFileOption.getValue(), externalComponentLibraries);
        }

        // Check the multiarg option
        for (size_t i=0; i<extLibPathsOption.getValue().size(); ++i)
        {
            externalComponentLibraries.push_back(extLibPathsOption.getValue()[i]);
        }

        // Load the actual external lib .dll/.so/.dylib files
        for (size_t i=0; i<externalComponentLibraries.size(); ++i)
        {
            bool rc = gHopsanCore.loadExternalComponentLib(externalComponentLibraries[i].c_str());
            printWaitingMessages(printDebugOption.getValue(), silentOption.getValue()); // Print after loading
            if (rc)
            {
                printColorMessage(Green, "Success loading External library: " + externalComponentLibraries[i], silentOption.getValue());
            }
            else
            {
                printErrorMessage("Failed to load External library: " + externalComponentLibraries[i], silentOption.getValue());
            }
        }

        if (testInstanciateComponentsOption.isSet())
        {
            cout <<  "Testing to instantiate each registered component. Any Error or Warning messages will be shown below:" << endl;
            //! @todo write as function
            vector<HString> types =  gHopsanCore.getRegisteredComponentTypes();
            size_t nErrors=0;
            for (size_t i=0; i<types.size(); ++i)
            {
                Component *pComp = gHopsanCore.createComponent(types[i].c_str());
                nErrors += gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages();// + gHopsanCore.getNumWarningMessages();
                printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());
                gHopsanCore.removeComponent(pComp);
            }
            if (nErrors==0)
            {
                returnSuccess=true;
            }
        }

        if (hmfPathOption.isSet() && createHvcTestOption.getValue())
        {
            string model = hmfPathOption.getValue();
            string dst = destinationPath;
            string basePath, baseName, filename, ext;
            splitFilePath(model, basePath, filename);
            splitFileName(filename, baseName, ext);
            cout <<  "Creating HVC Validation Data Set from Model: " << model << endl << "Saving data to: " << dst+baseName << ".*" << endl;
            returnSuccess =  createModelTestDataSet(model, dst+baseName);
        }

#ifdef USEOPS
        if(optimizationOption.isSet() && hmfPathOption.isSet())
        {
            string script;
            bool scriptFilesOk = true;

            vector<string> scripts = optimizationOption.getValue();
            for(size_t i=0; i<scripts.size(); ++i)
            {
                string line;
                ifstream file;
                file.open(scripts[i].c_str());
                if(!file.is_open())
                {
                    printErrorMessage("Unable to open file: "+scripts[i]);
                    scriptFilesOk = false;
                }
                else
                {
                    while ( file.good() )
                    {
                        std::vector<string> words;
                        getline(file, line);
                        // If this is run on linux, and a windows EOL style file is read, then the CR will remain,
                        // lets remove it in such a case
                        if (!line.empty() && (line[line.size()-1] == '\r'))
                        {
                            line.erase(line.size()-1);
                        }

                        if(line != "")
                        {
                            script += line+"\n";
                        }
                    }
                }
            }

            vector<string> settings = optimizationSettings.getValue();
            for(size_t i=0; i<settings.size(); ++i)
            {
                script += settings[i];
                if(i < settings.size()-1)
                {
                    script += "\n";
                }
            }

            cout << "SCRIPT: " << script << endl;

            string algorithm;
            vector<vector<double> > points;
            vector<double> objectives;
            vector<string> parNames;
            vector<string> objComps;
            vector<string> objPorts;
            vector<double> objWeights;
            vector<double> parMin;
            vector<double> parMax;
            size_t nPoints = 0;
            size_t nParams = 0;
            size_t maxEvals = 0;
            size_t nModels = 1;
            double tolerance = 1e-3;
            double alpha = 1.3;
            double beta = 0.3;
            double gamma = 2.0;
            double rho = 0.5;
            double sigma = -0.5;
            double omega1 = 1.0;
            double omega2 = 0.5;
            double C1 = 2;
            double C2 = 2;
            double vmax = 2;
            double CP = 0.2;
            double MP = 0.1;
            size_t elites = 0;
            std::string parallelMethod = "multidistance";
            double alphaMin = 0.0;
            double alphaMax = 2.0;
            size_t nPredictions = 1;
            size_t nRetractions = 1;
            double F = 1.0;
            double CR = 0.5;
            bool printDebugFile = false;
            bool silent = false;

            string line;
            stringstream scriptStream(script);

            while(getline(scriptStream,line,'\n'))
            {
                vector<string> words;
                // If this is run on linux, and a windows EOL style file is read, then the CR will remain,
                // lets remove it in such a case
                if (!line.empty() && (line[line.size()-1] == '\r'))
                {
                    line.erase(line.size()-1);
                }
                //cout << "LINE: " << line << endl;
                string word;
                stringstream lineStream(line);
                while(getline(lineStream, word, ' '))
                {
                    words.push_back(word);
                    //cout << "WORD: " << word << endl;
                }
                if( line.empty() || (words.size() > 0 && words[0][0] == '#') )
                {
                    continue;
                }
                if(words.size() == 2 && words[0] == "algorithm")
                {
                    algorithm = words[1];
                }
                else if(words.size() == 2 && words[0] == "maxevals")
                {
                    maxEvals = std::stoul(words[1]);
                }
                else if(words.size() == 1 && words[0] == "printdebugfile")
                {
                    printDebugFile = true;
                }
                else if(words.size() == 1 && words[0] == "silent")
                {
                    silent = true;
                }
                else if(words.size() == 2 && words[0] == "npoints")
                {
                    nPoints = std::stoul(words[1]);
                }
                else if(words.size() == 2 && words[0] == "tolerance")
                {
                    tolerance = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "nmodels")
                {
                    nModels = std::stoul(words[1]);
                }
                else if(words.size() == 4 && words[0] == "objective")
                {
                    objComps.push_back(words[1]);
                    objPorts.push_back(words[2]);
                    objWeights.push_back(stod(words[3]));
                }
                else if(words.size() == 4 && words[0] == "parameter")
                {
                    parNames.push_back(words[1]);
                    parMin.push_back(stod(words[2]));
                    parMax.push_back(stod(words[3]));
                    ++nParams;
                }
                else if(words.size() == 2 && words[0] == "alpha")
                {
                    alpha = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "beta")
                {
                    beta = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "gamma")
                {
                    gamma = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "rho")
                {
                    rho = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "omega1")
                {
                    omega1 = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "omega2")
                {
                    omega2 = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "C1")
                {
                    C1 = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "C2")
                {
                    C2 = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "vmax")
                {
                    vmax = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "CP")
                {
                    CP = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "MP")
                {
                    MP = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "elites")
                {
                    elites = std::stoul(words[1]);
                }
                else if(words.size() == 2 && words[0] == "sigma")
                {
                    sigma = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "parallelmethod")
                {
                    parallelMethod = words[1];
                }
                else if(words.size() == 2 && words[0] == "alphamin")
                {
                    alphaMin = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "alphamax")
                {
                    alphaMax = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "npredictions")
                {
                    nPredictions = std::stoul(words[1]);
                }
                else if(words.size() == 2 && words[0] == "nretractions")
                {
                    nRetractions = std::stoul(words[1]);
                }
                else if(words.size() == 2 && words[0] == "F")
                {
                    F = std::stod(words[1]);
                }
                else if(words.size() == 2 && words[0] == "CR")
                {
                    CR = std::stod(words[1]);
                }
                else
                {
                    printWarningMessage("Unhandled line in opt script: "+line);
                }
            }
            objectives.resize(nPoints);
            points.resize(nPoints);
            for(vector<double> &point : points)
            {
                point.resize(nParams);
            }

            //! @todo Make sure vectors are correct size!

            if(scriptFilesOk)
            {
                cout << "Loading Hopsan Model File: " << hmfPathOption.getValue() << endl;
                double startTime=0, stopTime=2;
                bool modelFileOk=true;
                std::vector<ComponentSystem*> rootSystemPtrs;
                for(size_t m=0; m<nModels; ++m)
                {
                    rootSystemPtrs.push_back(gHopsanCore.loadHMFModelFile(hmfPathOption.getValue().c_str(), startTime, stopTime));
                    if(rootSystemPtrs.at(m))
                    {
                        if (parameterImportOption.isSet())
                        {
                            cout << "Importing parameter values from file: " << parameterImportOption.getValue() << endl;
                            importParameterValuesFromCSV(parameterImportOption.getValue(), rootSystemPtrs.at(m));
                        }
                        rootSystemPtrs.at(m)->disableLog();
                    }
                    else
                    {
                        printErrorMessage("Could not load model file: " + hmfPathOption.getValue());
                        modelFileOk=false;
                        returnSuccess=false;
                        break;
                    }
                }
                size_t nErrors = gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages();
                printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());
                if (nErrors < 1 && modelFileOk)
                {
                    OptimizationEvaluator *pEvaluator = new OptimizationEvaluator(rootSystemPtrs, parNames, objComps, objPorts,
                                                                                  objWeights, parMin, parMax, startTime, stopTime);

                    //Initialize base worker
                    Ops::Worker *pBaseWorker;
                    Ops::MessageHandler *pOpsMessages;
                    if(silent)
                    {
                        pOpsMessages = new Ops::MessageHandler();
                    }
                    else
                    {
                        pOpsMessages = new OptimizationMessageHandler(maxEvals, silentOption.getValue());
                    }
                    if(algorithm == "neldermead")
                        pBaseWorker = new Ops::WorkerNelderMead(pEvaluator, pOpsMessages);
                    else if(algorithm == "complexrf")
                        pBaseWorker = new Ops::WorkerComplexRF(pEvaluator, pOpsMessages);
                    else if(algorithm == "complexrfp")
                        pBaseWorker = new Ops::WorkerComplexRFP(pEvaluator, pOpsMessages);
                    else if(algorithm == "complexburmen")
                        pBaseWorker = new Ops::WorkerComplexBurmen(pEvaluator, pOpsMessages);
                    else if(algorithm == "crs")
                        pBaseWorker = new Ops::WorkerControlledRandomSearch(pEvaluator, pOpsMessages);
                    else if(algorithm == "de")
                        pBaseWorker = new Ops::WorkerDifferentialEvolution(pEvaluator, pOpsMessages);
                    else if(algorithm == "pso")
                        pBaseWorker = new Ops::WorkerParticleSwarm(pEvaluator, pOpsMessages);
                    else if(algorithm == "parametersweep")
                        pBaseWorker = new Ops::WorkerParameterSweep(pEvaluator, pOpsMessages);
                    else if(algorithm == "genetic")
                        pBaseWorker = new Ops::WorkerGenetic(pEvaluator, pOpsMessages);
                    else
                        pBaseWorker = new Ops::Worker(pEvaluator, pOpsMessages);

                    //Set common parameters
                    pEvaluator->setWorker(pBaseWorker);
                    if(!silent)
                    {
                        dynamic_cast<OptimizationMessageHandler*>(pOpsMessages)->setWorker(pBaseWorker);
                    }
                    pBaseWorker->setMaxNumberOfIterations(maxEvals);
                    pBaseWorker->setNumberOfCandidates(nModels);
                    pBaseWorker->setNumberOfPoints(nPoints);
                    pBaseWorker->setNumberOfParameters(nParams);
                    for(size_t p=0; p<parNames.size(); ++p)
                    {
                        pBaseWorker->setParameterLimits(p,parMin[p],parMax[p]);
                    }
                    pBaseWorker->setTolerance(tolerance);
                    pBaseWorker->setSamplingMethod(Ops::SamplingLatinHypercube);

                    //Set algorithm-specific parameters
                    if(algorithm == "neldermead")
                    {
                        Ops::WorkerNelderMead *pWorker = dynamic_cast<Ops::WorkerNelderMead*>(pBaseWorker);
                        pWorker->setReflectionFactor(alpha);
                        pWorker->setExpansionFactor(gamma);
                        pWorker->setContractionFactor(rho);
                        pWorker->setReductionFactor(sigma);
                    }
                    else if(algorithm == "complexrf")
                    {
                        Ops::WorkerComplexRF *pWorker = dynamic_cast<Ops::WorkerComplexRF*>(pBaseWorker);
                        pWorker->setReflectionFactor(alpha);
                        pWorker->setForgettingFactor(gamma);
                        pWorker->setRandomFactor(beta);
                    }
                    else if(algorithm == "complexrfp")
                    {
                        Ops::WorkerComplexRFP *pWorker = dynamic_cast<Ops::WorkerComplexRFP*>(pBaseWorker);
                        pWorker->setReflectionFactor(alpha);
                        pWorker->setForgettingFactor(gamma);
                        pWorker->setRandomFactor(beta);
                        if(parallelMethod == "taskprediction")
                        {
                            pWorker->setParallelMethod(Ops::TaskPrediction);
                        }
                        else if(parallelMethod == "multidirection")
                        {
                            pWorker->setParallelMethod(Ops::MultiDirection);
                        }
                        else if(parallelMethod == "multidistance")
                        {
                            pWorker->setParallelMethod(Ops::MultiDistance);
                        }
                        pWorker->setMinimumReflectionFactor(alphaMin);
                        pWorker->setMaximumReflectionFactor(alphaMax);
                        pWorker->setNumberOfPredictions(nPredictions);
                        pWorker->setNumberOfRetractions(nRetractions);
                    }
                    else if(algorithm == "complexburmen")
                    {
                        Ops::WorkerComplexBurmen *pWorker = dynamic_cast<Ops::WorkerComplexBurmen*>(pBaseWorker);
                        pWorker->setReflectionFactor(alpha);
                        pWorker->setRandomFactor(beta);
                        pWorker->setForgettingFactor(gamma);
                    }
                    else if(algorithm == "de")
                    {
                        Ops::WorkerDifferentialEvolution *pWorker = dynamic_cast<Ops::WorkerDifferentialEvolution*>(pBaseWorker);
                        pWorker->setDifferentialWeight(F);
                        pWorker->setCrossoverProbability(CR);
                    }
                    else if(algorithm == "pso")
                    {
                        Ops::WorkerParticleSwarm *pWorker = dynamic_cast<Ops::WorkerParticleSwarm*>(pBaseWorker);
                        pWorker->setOmega1(omega1);
                        pWorker->setOmega2(omega2);
                        pWorker->setC1(C1);
                        pWorker->setC2(C2);
                        pWorker->setVmax(vmax);
                    }
                    else if(algorithm == "genetic")
                    {
                        Ops::WorkerGenetic *pWorker = dynamic_cast<Ops::WorkerGenetic*>(pBaseWorker);
                        pWorker->setCrossoverProbability(CP);
                        pWorker->setMutationProbability(MP);
                        pWorker->setNumberOfElites(elites);
                    }

                    //Error checking
                    //! @todo Also check that optimization parameters and objective ports exist in model
                    for(size_t i=0; i<objComps.size(); ++i) //Check that objective function components exists
                    {
                        if(!rootSystemPtrs[0]->haveSubComponent(objComps[i].c_str()))
                        {
                            printErrorMessage("Error: Objective function component not found. Aborting.", silentOption.getValue());
                            return -1;
                        }
                    }

                    //Execute optimization
                    pBaseWorker->initialize();
                    pBaseWorker->run();

                    //Print results
                    if(printDebugFile)
                    {
                        std::ofstream file;
                        file.open("HopsanCLI_debug.csv", std::ios_base::app);
                        if (!file.good())
                        {
                            printErrorMessage("Could not open HopsanCLI_debug.csv for writing!", silentOption.getValue());
                        }
                        else
                        {
                            file << pBaseWorker->getAlgorithm() << ",";
                            file << pBaseWorker->getNumberOfCandidates() << ",";
                            file << pBaseWorker->getCurrentNumberOfIterations() << ",";
                            file << pEvaluator->getNumberOfEvaluations() << ",";
                            file << "0,"; //Surrogate models, no longer used
                            file << pBaseWorker->getObjectiveValue(pBaseWorker->getBestId());
                            for(size_t i=0; i<pBaseWorker->getNumberOfParameters(); ++i)
                            {
                                file << "," << pBaseWorker->getParameter(pBaseWorker->getBestId(),i);
                            }
                            file << "," <<pBaseWorker->getObjectiveValue(pBaseWorker->getWorstId());
                            for(size_t i=0; i<pBaseWorker->getNumberOfParameters(); ++i)
                            {
                                file << "," << pBaseWorker->getParameter(pBaseWorker->getWorstId(),i);
                            }
                            file << endl;
                            file.close();
                        }
                    }

                    returnSuccess=true;
                }
            }
        }
#endif

        if(hmfPathOption.isSet() && !createHvcTestOption.getValue() && !optimizationOption.isSet())
        {
            returnSuccess=false;
            printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());

            if (hvcTestOption.isSet())
            {
                printWarningMessage("Do not specify a hmf file in combination with the -t (--validate) option. Model should be loaded from the .hvc file", silentOption.getValue());
            }

            cout << "Loading Hopsan Model File: " << hmfPathOption.getValue() << endl;
            double startTime=0, stopTime=2;
            ComponentSystem* pRootSystem = gHopsanCore.loadHMFModelFile(hmfPathOption.getValue().c_str(), startTime, stopTime);
            size_t nErrors = gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages();
            printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());
            if (nErrors < 1)
            {
                if (parameterImportOption.isSet())
                {
                    cout << "Importing parameter values from file: " << parameterImportOption.getValue() << endl;
                    importParameterValuesFromCSV(parameterImportOption.getValue(), pRootSystem);
                }

                if (parameterExportOption.isSet())
                {
                    cout << "Exporting parameter values to file: " << destinationPath+parameterExportOption.getValue() << endl;
                    string prefix;
                    if (prefixRootLevelName.getValue())
                    {
                        prefix = pRootSystem->getName().c_str()+string("$");
                    }
                    exportParameterValuesToCSV(destinationPath+parameterExportOption.getValue(), pRootSystem, prefix);
                }

                cout << endl << "Model Hierarchy:" << endl;
                printComponentHierarchy(pRootSystem, "", true, true);
                cout << endl;

                std::vector<std::string> logOnlyPortsOrVariables;
                if (pRootSystem && simulateOption.isSet())
                {
                    bool doSimulate=true;

                    // Abort if root model is empty (probably load hmf failed somehow)
                    if (pRootSystem->isEmpty())
                    {
                        printErrorMessage("The root system seems to be empty, aborting!", silentOption.getValue());
                        doSimulate = false;
                    }

                    double stepTime = pRootSystem->getTimestep();
                    // Parse simulation options, and replace hmf values if needed
                    vector<string> simTime;
                    if (simulateOption.getValue() != "hmf")
                    {
                        splitStringOnDelimiter(simulateOption.getValue(),',',simTime);
                        if (simTime.size() == 3)
                        {
                            startTime = atof(simTime[0].c_str());
                            stepTime = atof(simTime[1].c_str());
                            stopTime = atof(simTime[2].c_str());
                        }
                        else if (simTime.size() == 2)
                        {
                            stepTime = atof(simTime[0].c_str());
                            stopTime = atof(simTime[1].c_str());
                        }
                        else if (simTime.size() == 1)
                        {
                            stopTime = atof(simTime[0].c_str());
                        }
                    }
                    pRootSystem->setDesiredTimestep(stepTime);

                    if (nLogSamplesOption.isSet())
                    {
                        size_t nSamp = atoi(nLogSamplesOption.getValue().c_str());
                        cout << "Setting nLogSamples to: " << nSamp << endl;
                        pRootSystem->setNumLogSamples(nSamp);
                    }

                    if (logonlyOption.isSet())
                    {
                        auto file_or_list = logonlyOption.getValue();
                        // Check if argument is a file, if so, read from it line-by-line
                        std::ifstream port_name_file(file_or_list);
                        if (port_name_file.is_open())
                        {
                            std::string line;
                            while(std::getline(port_name_file, line))
                            {
                                if (!line.empty())
                                {
                                    logOnlyPortsOrVariables.push_back(line);
                                }
                            }
                            port_name_file.close();
                        }
                        // Else read , separated string
                        else
                        {
                            splitStringOnDelimiter(file_or_list,',',logOnlyPortsOrVariables);
                        }

                        // Now disable all nodes and then enable the requested ones
                        forEachPort(pRootSystem, [](hopsan::Port& port){port.setEnableLogging(false);});
                        for (const auto& port_name : logOnlyPortsOrVariables)
                        {
                            hopsan::Port* pPort = getPortWithFullName(pRootSystem, port_name);
                            if (pPort)
                            {
                                pPort->setEnableLogging(true);
                            }
                            else
                            {
                                printWarningMessage("Could not find port: '"+port_name+"' when processing logonly input");
                            }
                        }
                    }

                    // Apply loaded simulation states or only load start values
                    if (loadSimulationStateOption.isSet())
                    {
                        double timeOffset;
                        restoreSimulationPoint(loadSimulationStateOption.getValue().c_str(), pRootSystem, timeOffset);
                        startTime+=timeOffset;
                        stopTime+=timeOffset;
                        pRootSystem->setKeepValuesAsStartValues(true);
                    }
                    else if (loadSimulationSVOption.isSet())
                    {
                        double dummy;
                        restoreSimulationPoint(loadSimulationSVOption.getValue().c_str(), pRootSystem, dummy);
                        pRootSystem->setKeepValuesAsStartValues(true);
                    }

                    //! @todo maybe use simulation handler object instead
                    TicToc isoktimer("IsOkTime");
                    doSimulate = doSimulate && pRootSystem->checkModelBeforeSimulation();
                    isoktimer.TocPrint();
                    if (doSimulate)
                    {
                        TicToc initTimer("InitializeTime");
                        doSimulate = doSimulate && pRootSystem->initialize(startTime, stopTime);
                        initTimer.TocPrint();
                    }
                    else
                    {
                        printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());
                        printErrorMessage("Initialize failed, Simulation aborted!", silentOption.getValue());
                    }

                    if (doSimulate)
                    {
                        cout << "Simulating: " << startTime << " to " << stopTime << " with Ts: " << stepTime << "     Please Wait!" << endl;
                        TicToc simuTimer("SimulationTime");
                        if(parallelOption.isSet()) {
                            int nThreads = atoi(parallelOption.getValue().c_str());
                            if(nThreads < 0) {
                                printErrorMessage("Number of threads cannot be negative.");
                                return -1;
                            }
                            pRootSystem->simulateMultiThreaded(startTime, stopTime, nThreads);
                        }
                        else {
                            pRootSystem->simulate(stopTime);
                        }

                        simuTimer.TocPrint();
                    }
                    if (pRootSystem->wasSimulationAborted())
                    {
                        printErrorMessage("Simulation was aborted!", silentOption.getValue());
                    }
                    else
                    {
                        returnSuccess = true;
                    }

                    pRootSystem->finalize();
                }

                printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());

                // Check in what formats to export
                if (resultsFinalCSVOption.isSet())
                {
                    cout << "Saving Final results to file: " << destinationPath+resultsFinalCSVOption.getValue() << endl;
                    string prefix;
                    if (prefixRootLevelName.getValue())
                    {
                        prefix = pRootSystem->getName().c_str()+string("$");
                    }
                    saveResultsToCSV(pRootSystem, destinationPath+resultsFinalCSVOption.getValue(), Final, logOnlyPortsOrVariables);
                    // Should we transpose the result
                    if (resultsCSVSortOption.getValue() == "cols")
                    {
                        cout << "Transposing CSV file" << endl;
                        transposeCSVresults(destinationPath+resultsFinalCSVOption.getValue());
                    }
                    else if (resultsCSVSortOption.getValue() != "rows")
                    {
                        printErrorMessage("Unknown CSV sorting format: " + resultsCSVSortOption.getValue(), silentOption.getValue());
                    }
                }

                if (resultsFullCSVOption.isSet())
                {
                    cout << "Saving Full results to file: " << destinationPath+resultsFullCSVOption.getValue() << endl;
                    string prefix;
                    if (prefixRootLevelName.getValue())
                    {
                        prefix = pRootSystem->getName().c_str()+string("$");
                    }
                    saveResultsToCSV(pRootSystem, destinationPath+resultsFullCSVOption.getValue(), Full, logOnlyPortsOrVariables);
                    // Should we transpose the result
                    if (resultsCSVSortOption.getValue() == "cols")
                    {
                        cout << "Transposing CSV file" << endl;
                        transposeCSVresults(destinationPath+resultsFullCSVOption.getValue());
                    }
                    else if (resultsCSVSortOption.getValue() != "rows")
                    {
                        printErrorMessage("Unknown CSV sorting format: " + resultsCSVSortOption.getValue(), silentOption.getValue());
                    }
                }


                if(resultsFullHDF5Option.isSet()) {
                    cout << "Saving full results to file: " << destinationPath+resultsFullHDF5Option.getValue() << endl;
                    saveResultsToHDF5(pRootSystem, destinationPath+resultsFullHDF5Option.getValue(), logOnlyPortsOrVariables, Full);
                }

                if(resultsFinalHDF5Option.isSet()) {
                    cout << "Saving final results to file: " << destinationPath+resultsFinalHDF5Option.getValue() << endl;
                    saveResultsToHDF5(pRootSystem, destinationPath+resultsFinalHDF5Option.getValue(), logOnlyPortsOrVariables, Final);
                }

                // Save simulation state
                if (saveSimulationStateOption.isSet())
                {
                    saveSimulationPoint(saveSimulationStateOption.getValue().c_str(), pRootSystem);
                }

                // Now remove the rootsystem
                delete pRootSystem;
                pRootSystem = 0;
            }
            else
            {
                printErrorMessage("There were errors while loading the model", silentOption.getValue());
            }
        }

        // Perform a unit test
        if(hvcTestOption.isSet())
        {
            returnSuccess = performModelTest(hvcTestOption.getValue());
        }
        else
        {
            printWaitingMessages(printDebugOption.getValue(), silentOption.getValue());
            cout << endl << "HopsanCLI Done!" << endl;
        }

        setTerminalColor(Reset); //Reset terminal color
        if (endPauseOption.getValue())
        {
            cout << "Press ENTER to continue ...";
            cin.get();
        }

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cout << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }

    if (returnSuccess)
    {
        return 0;
    }
    // If not success return 1
    return 1;
}
