/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCLI/ModelValidation.cpp
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains model validation functions for CLI
//!
//$Id$

#include "ModelValidation.h"
#include "core_cli.h"
#include "CliUtilities.h"
#include "ModelUtilities.h"
#include "version_cli.h"

#include "HopsanEssentials.h"
#include "hopsan_rapidxml.hpp"
#include "ComponentUtilities/CSVParser.h"

#include <cstring>
#include <iomanip>
#include <ctime>

using namespace hopsan;
using namespace std;

class ValidatonVariable
{
public:
    std::string fullVarName;
    int column;
    int timecolumn;
    double tolerance;
};

bool exportHVCData(const string modelpath, const string baseFilePath, const std::vector<Port *> &rPorts, const std::vector<size_t> &rDataIds)
{
    std::string hvcFilePath = baseFilePath+".hvc";
    std::string csvFilePath = baseFilePath+".hvd";
    const double tol=0.01;

    std::map< vector<double>*, size_t> savedTimeVectors;

    try
    {
        std::ofstream hvcFile(hvcFilePath.c_str());
        std::ofstream csvFile(csvFilePath.c_str());

        if (!hvcFile.is_open())
        {
            printErrorMessage("Could not open file for writing: "+ hvcFilePath);
            return false;
        }

        if (!csvFile.is_open())
        {
            printErrorMessage("Could not open file for writing: "+ csvFilePath);
            return false;
        }

        rapidxml::xml_document<> doc;
        addXmlDeclaration(&doc);
        rapidxml::xml_node<> *pRootNode = appendEmptyNode(&doc, "hopsanvalidationconfiguration");
        writeStringAttribute(pRootNode, "hvcversion", "0.2");

        time_t now = time(0);
        tm *ltm = localtime(&now);
        stringstream dateSS, timeSS;
        dateSS << 1900+ltm->tm_year << std::setfill('0') << std::setw(2) << 1+ltm->tm_mon << ltm->tm_mday;
        timeSS << std::setfill('0') << std::setw(2) << 1+ltm->tm_hour << 1+ltm->tm_min << 1+ltm->tm_sec;

        rapidxml::xml_node<> *pValidationNode = appendEmptyNode(pRootNode, "validation");
        writeStringAttribute(pValidationNode, "hopsancliversion", HOPSANCLIVERSION);
        writeStringAttribute(pValidationNode, "time", timeSS.str());
        writeStringAttribute(pValidationNode, "date", dateSS.str());
        writeStringAttribute(pValidationNode, "hopsancoreversion", gHopsanCore.getCoreVersion());

        // Decide paths relativ to the hvc file basepath
        string basepath, dummy, relModelPath, relCsvPath;
        splitFilePath(hvcFilePath, basepath, dummy);
        relModelPath = relativePath(basepath, modelpath);
        relCsvPath = relativePath(basepath, csvFilePath);

        appendValueNode(pValidationNode, "modelfile", relModelPath);
        appendValueNode(pValidationNode, "hvdfile", relCsvPath);

        size_t csvRow = 0, csvTimeRow;
        for (size_t p=0; p<rPorts.size(); ++p)
        {
            // Write variable node to hvc
            rapidxml::xml_node<> *pVariableNode = appendEmptyNode(pValidationNode, "variable");
            std::string fullName = generateFullPortVariableName(rPorts[p], rDataIds[p]).c_str();
            writeStringAttribute(pVariableNode, "name", fullName );

            // Lookup if timevector has already been write to file, and at what row, if not then write and remember the row
            std::vector<double> *pLogTime = rPorts[p]->getLogTimeVectorPtr();
            if (savedTimeVectors.count(pLogTime) == 0)
            {
                // Write time vector
                for (size_t i=0; i<pLogTime->size()-1; ++i)
                {
                    csvFile << (*pLogTime)[i] << ", ";
                }
                csvFile << (*pLogTime)[pLogTime->size()-1] << std::endl;
                csvTimeRow = csvRow;
                savedTimeVectors.insert(std::pair<vector<double>*, size_t>(pLogTime, csvTimeRow));
                ++csvRow;
            }
            else
            {
                csvTimeRow = savedTimeVectors.at(pLogTime);
            }

            appendValueNode(pVariableNode, "timecolumn", to_string(csvTimeRow));
            appendValueNode(pVariableNode, "column", to_string(csvRow));
            appendValueNode(pVariableNode, "tolerance", to_string(tol));

            // Write data line to csv
            std::vector< std::vector<double> > *pLogData = rPorts[p]->getLogDataVectorPtr();
            if (pLogData &&  pLogData->size() > 0)
            {
                size_t nRows = pLogData->size();
                size_t nCols = pLogData->front().size();
                const size_t c = rDataIds[p];
                if (rDataIds[p] < nCols)
                {
                    for (size_t r=0; r<nRows-1; ++r)
                    {
                        csvFile << (*pLogData)[r][c] << ", ";
                    }
                    csvFile << (*pLogData)[nRows-1][c] << std::endl;
                    ++csvRow;
                }
            }
        }

        // Write and close hvc xml
        hvcFile << doc;
        hvcFile.close();
        doc.clear();

        // Write and close csv
        csvFile.close();
        transposeCSVresults(csvFilePath); // Turn it into a column file
        return true;
    }
    catch(std::exception &e)
    {
        printErrorMessage(e.what());
    }
    return false;
}


bool runTheActualTest(ComponentSystem *pRootSystem, double startTime, double stopTime, size_t nCores)
{
        if (gHopsanCore.getNumWarningMessages() > 0)
        {
            printWaitingMessages(false);
        }

        int nThreads = -1; //Single threaded
        if (nCores > 1)
        {
            nThreads = int(nCores);
        }

        SimulationHandler simuhandler;
        bool isOK = simuhandler.initializeSystem(startTime, stopTime, pRootSystem);
        if (isOK)
        {
            isOK = simuhandler.simulateSystem(startTime, stopTime, nThreads, pRootSystem);
            if (!isOK)
            {
                printWaitingMessages(false);
                printErrorMessage("Simulation failed, Simulation aborted!");
            }
        }
        else
        {
            printWaitingMessages(false);
            printErrorMessage("Initialize failed, Simulation aborted!");
        }
        simuhandler.finalizeSystem(pRootSystem);

        return isOK;
}



bool extractTestData(ComponentSystem *pRootSystem, string fullVarName, std::vector<double> &rvTime, std::vector<double> &rvSim)
{
    // Figure out system hierarcy and names (split name attribute)
    std::vector<std::string> sysfields;
    std::vector<std::string> namefields;
    splitStringOnDelimiter(fullVarName, '$', sysfields);
    if (sysfields.size() < 2)
    {
        printErrorMessage("To few system fields in: "+ fullVarName);
        return false;
    }

    // Travle through the model to find the system to extract data from
    ComponentSystem *pSystemToExtractFrom = pRootSystem;
    if (sysfields.size() > 2)
    {
        for (size_t s=1; s<sysfields.size(); ++s)
        {
            ComponentSystem *pParentSystem = pSystemToExtractFrom;
            pSystemToExtractFrom = pSystemToExtractFrom->getSubComponentSystem(sysfields[s].c_str());
            if (!pSystemToExtractFrom)
            {
                printErrorMessage("Could not find system: "+sysfields[s]+" in parent system: "+pParentSystem->getName().c_str());
                return false;
            }
        }
    }

    // If we only have one sysfiled then components lie in the top-level system
    string name = sysfields.back();
    // Now extract comp port and var names
    splitStringOnDelimiter(name, '#', namefields);
    if (namefields.size() != 3)
    {
        printErrorMessage("Wrong number of name fields in: "+name);
        return false;
    }
    string compName, portName, varName;
    compName = namefields[0];
    portName = namefields[1];
    varName  = namefields[2];



    // Find the Component and Port containing the variable
    Component* pComp = pSystemToExtractFrom->getSubComponent(compName.c_str());
    if (!pComp)
    {
        printErrorMessage("No such component name: " + compName);
        return false;
    }
    Port *pPort = pComp->getPort(portName.c_str());
    if (!pPort)
    {
        printErrorMessage("No such port name: " + portName + " in component: " + compName);
        return false;
    }

    // Copy the time and data vectors that we want to compare
    //! @todo with better access function in core we could avoid copying data and work directly with the data stored
    rvTime = *pPort->getLogTimeVectorPtr();
    int dataId = pPort->getNodeDataIdFromName(varName.c_str());
    if (dataId < 0)
    {
        printErrorMessage("No such varaiable name: " + varName + " in: " + pPort->getNodeType().c_str());
        return false;
    }
    vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();
    rvSim.reserve(rvTime.size());
    for(size_t i=0; i<rvTime.size(); ++i)
    {
        rvSim.push_back(pLogData->at(i).at(dataId));
    }
    return true;
}

//! @brief Performs a unit test on a model
//! @param hvcFilePath Name of test model
bool performModelTest(const std::string hvcFilePath)
{
    // Figure out basepath and basename
    string basepath, basename, filename, ext;
    splitFilePath(hvcFilePath, basepath, filename);
    splitFileName(filename, basename, ext);
    //cout << basepath << " :: " << filename << " :: " << basename << " :: " << ext << endl;

    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> hmfFile(hvcFilePath.c_str());
        doc.parse<0>(hmfFile.data());
        rapidxml::xml_node<> *pRootNode = doc.first_node();

        //Check for correct root node name
        //! @todo should check version also
        if (strcmp(pRootNode->name(), "hopsanvalidationconfiguration")!=0)
        {
            printErrorMessage(hvcFilePath  + " Has wrong root tag name: " + string(pRootNode->name()));
            return false;
        }

        std::string hvcVersion = readStringAttribute(pRootNode, "hvcversion", "0");

        rapidxml::xml_node<> *pValidationNode = pRootNode->first_node("validation");
        if (!pValidationNode)
        {
            printErrorMessage("No validation node found in xml");
            return false;
        }
        while (pValidationNode != 0)
        {
            // Run each validation
            string modelfile = readStringNodeValue(pValidationNode->first_node("modelfile"), "");
            string parameterset = readStringNodeValue(pValidationNode->first_node("parameterset"), "");

            // If no modelfile was given use one with the same basename
            if (modelfile.empty())
            {
                modelfile = basepath + basename + ".hmf";
            }
            else
            {
                // Assumes that modelfile path was relaitve in xml
                modelfile = basepath + modelfile;
            }

            if (hvcVersion == "0.1")
            {
                printWarningMessage("hvcVersion is old 0.1, you should update to a newer format!");
                rapidxml::xml_node<> *pComponentNode, *pPortNode, *pVariableNode;
                pComponentNode = pValidationNode->first_node("component");
                if (!pComponentNode)
                {
                    printErrorMessage("No component node found in xml");
                    return false;
                }
                while (pComponentNode != 0)
                {
                    string compName = readStringAttribute(pComponentNode, "name", "_noname_");
                    pPortNode = pComponentNode->first_node("port");
                    if (!pPortNode)
                    {
                        printErrorMessage("No port node found in xml");
                        return false;
                    }
                    while (pPortNode != 0)
                    {
                        string csvfile = readStringNodeValue(pValidationNode->first_node("csvfile"), "");
                        string portName = readStringAttribute(pPortNode, "name", "_noname_");
                        csvfile = readStringNodeValue(pPortNode->first_node("csvfile"), "");
                        pVariableNode = pPortNode->first_node("variable");
                        if (!pVariableNode)
                        {
                            printErrorMessage("No variable node found in xml");
                            return false;
                        }
                        while (pVariableNode != 0)
                        {
                            string varname = readStringAttribute(pVariableNode, "name", "_noname_");
                            csvfile = readStringNodeValue(pVariableNode->first_node("csvfile"), csvfile); //Do we have variable specific csv override
                            if (csvfile.empty())
                            {
                                // If no csvfile was given use one with the same basename
                                csvfile = basepath + basename + ".csv";
                            }
                            else
                            {
                                // Assumes that csvfile path was relaitve in xml
                                csvfile = basepath + csvfile;
                            }

                            const int column = readIntNodeValue(pVariableNode->first_node("column"), 1);
                            const double tolerance = readDoubleNodeValue(pVariableNode->first_node("tolerance"), 0.01);

                            // ----- Perform test -----
                            vector<double> vRef, vSim1, vSim2, vTime;

                            // Load reference data curve
                            //! @todo should not reload if same as already laoded
                            bool success=false;
                            CSVParser refData(success, csvfile.c_str(), '\n', '"');
                            if(!success)
                            {
                                printErrorMessage("Unable to initialize CSV file: " + csvfile + " : " + refData.getErrorString().c_str());
                                return false;
                            }

                            double startTime=0, stopTime=1;
                            ComponentSystem* pRootSystem = gHopsanCore.loadHMFModelFile(modelfile.c_str(), startTime, stopTime);

                            if ( pRootSystem && ((gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages()) < 1) )
                            {
                                if (gHopsanCore.getNumWarningMessages() > 0)
                                {
                                    printWaitingMessages(false);
                                }

                                //! @todo maybe use simulation handler object
                                //First simulation
                                if (!pRootSystem->checkModelBeforeSimulation())
                                {
                                    printWaitingMessages(false);
                                    printErrorMessage("checkModelBeforeSimulation() failed, Simulation aborted!");
                                    return false;
                                }

                                if (pRootSystem->initialize(startTime, stopTime))
                                {
                                    pRootSystem->simulate(stopTime);
                                }
                                else
                                {
                                    printWaitingMessages(false);
                                    printErrorMessage("Initialize failed, Simulation aborted!");
                                    return false;
                                }
                                pRootSystem->finalize();

                                //copy the data
                                Component* pComp = pRootSystem->getSubComponent(compName.c_str());
                                if (!pComp)
                                {
                                    printErrorMessage("No such component name: " + compName);
                                    return false;
                                }
                                Port *pPort = pComp->getPort(portName.c_str());
                                if (!pPort)
                                {
                                    printErrorMessage("No such port name: " + portName + " in component: " + compName);
                                    return false;
                                }

                                //! @todo with better access function in core we could avoid copying data and work directly with the data stored
                                vTime = *pPort->getLogTimeVectorPtr();
                                int dataId = pPort->getNodeDataIdFromName(varname.c_str());
                                if (dataId < 0)
                                {
                                    printErrorMessage("No such varaiable name: " + varname + " in: " + pPort->getNodeType().c_str());
                                    return false;
                                }

                                for(size_t i=0; i<vTime.size(); ++i)
                                {
                                    vSim1.push_back(pRootSystem->getSubComponent(compName.c_str())->getPort(portName.c_str())->getLogDataVectorPtr()->at(i).at(dataId));
                                }

                                //Second simulation
                                if (pRootSystem->initialize(startTime, stopTime))
                                {
                                    pRootSystem->simulate(stopTime);
                                }
                                else
                                {
                                    printWaitingMessages(false);
                                    printErrorMessage("Initialize failed, Simulation aborted!");
                                    return false;
                                }
                                pRootSystem->finalize();

                                for(size_t i=0; i<vTime.size(); ++i)
                                {
                                    vSim2.push_back(pRootSystem->getSubComponent(compName.c_str())->getPort(portName.c_str())->getLogDataVectorPtr()->at(i).at(dataId));
                                }

                                // Print the messages if there were any errors or warnings
                                if ( (gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages() + gHopsanCore.getNumWarningMessages()) != 0)
                                {
                                    printWaitingMessages(false);
                                }
                            }
                            else
                            {
                                printWaitingMessages(false);
                                printErrorMessage("Could not load modelfile without errors: " + modelfile);
                                return false;
                            }


                            for(size_t i=0; i<vTime.size(); ++i)
                            {
                                vRef.push_back(refData.interpolate(vTime[i], column));
                            }

                            //std::cout.rdbuf(cout_sbuf); // restore the original stream buffer

                            if(!compareVectors(vSim1, vRef, tolerance))
                            {
                                printColorMessage(Red, string("Validation data test failed: ") + pRootSystem->getName().c_str() + ":" + compName + ":" + portName + ":" + varname);
                                return false;
                            }

                            if(!compareVectors(vSim1, vSim2, tolerance))
                            {
                                printColorMessage(Red, string("Consistency test failed (two consecutive simulations gave different results): ") + pRootSystem->getName().c_str() + ":" + compName + ":" + portName + ":" + varname);
                                return false;
                            }

                            printColorMessage(Green, string("Test successful: ") + pRootSystem->getName().c_str() + ":" + compName + ":" + portName + ":" + varname);

                            pVariableNode = pVariableNode->next_sibling("variable");
                        }
                        pPortNode = pPortNode->next_sibling("port");
                    }
                    pComponentNode = pComponentNode->next_sibling("component");
                }
            }
            else if (hvcVersion == "0.2")
            {
                bool hvdsuccess=false;
                string hvdfile = readStringNodeValue(pValidationNode->first_node("hvdfile"), "");
                if (hvdfile.empty())
                {
                    // If no csvfile was given use one with the same basename
                    hvdfile = basepath + basename + ".hvd";
                }
                else
                {
                    // Assumes that csvfile path was relaitve in xml
                    hvdfile = basepath + hvdfile;
                }
                // Load reference data curves
                CSVParser refData(hvdsuccess, hvdfile.c_str(), '\n', '"');
                if(!hvdsuccess)
                {
                    printErrorMessage("Unable to initialize HVD file: " + hvdfile + " : " + refData.getErrorString().c_str());
                    return false;
                }

                // Read all variable to test
                vector<ValidatonVariable> validationVariables;
                rapidxml::xml_node<> *pVariableNode = pValidationNode->first_node("variable");
                while (pVariableNode != 0)
                {
                    ValidatonVariable var;
                    var.fullVarName = readStringAttribute(pVariableNode, "name", "_noname_");
                    var.column = readIntNodeValue(pVariableNode->first_node("column"), -1);
                    var.timecolumn = readIntNodeValue(pVariableNode->first_node("timecolumn"), -1);
                    var.tolerance = readDoubleNodeValue(pVariableNode->first_node("tolerance"), 0.01);
                    validationVariables.push_back(var);
                    pVariableNode = pVariableNode->next_sibling("variable");
                }

                // ==================
                // Now begin testing
                // ==================
                size_t nCores = getNumAvailibleCores();
                vector< vector< vector<double> > > vvvSimulationTime1, vvvSimulationData1;

                // Resize vectors (to avoid data copying later)
                vvvSimulationTime1.resize(nCores);
                vvvSimulationData1.resize(nCores);
                for (size_t i=0; i<nCores; ++i)
                {
                    vvvSimulationTime1[i].resize(validationVariables.size());
                    vvvSimulationData1[i].resize(validationVariables.size());
                }

                //  Load the system
                double startTime=0, stopTime=-1;
                ComponentSystem* pRootSystem = gHopsanCore.loadHMFModelFile(modelfile.c_str(), startTime, stopTime);
                if ( pRootSystem && ((gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages()) < 1) )
                {
                    if (gHopsanCore.getNumWarningMessages() > 0)
                    {
                        printWaitingMessages(false);
                    }

                    // Run simulations for each core setup
                    for (size_t c=0; c<nCores; ++c)
                    {
                        // Run first simulation, Exit if failure
                        bool simOK = runTheActualTest(pRootSystem, startTime, stopTime, c+1);
                        if (!simOK)
                        {
                            return false;
                        }

                        // Now extract data for each variable, Exit if failure
                        for (size_t v=0; v<validationVariables.size(); ++v)
                        {
                            ValidatonVariable &var = validationVariables[v];
                            bool extractOK = extractTestData(pRootSystem, var.fullVarName, vvvSimulationTime1[c][v], vvvSimulationData1[c][v]);
                            if (!extractOK)
                            {
                                return false;
                            }
                        }

                        // Run second simulation, Exit if failure
                        simOK = runTheActualTest(pRootSystem, startTime, stopTime, c+1);
                        if (!simOK)
                        {
                            return false;
                        }

                        // Now extract data for each variable and run consistency tests, Exit if failure
                        for (size_t v=0; v<validationVariables.size(); ++v)
                        {
                            vector<double> vSimulationTime2, vSimulationData2;

                            ValidatonVariable &var = validationVariables[v];
                            bool extractOK = extractTestData(pRootSystem, var.fullVarName, vSimulationTime2, vSimulationData2);
                            if (!extractOK)
                            {
                                return false;
                            }

                            // Run consistency tests
                            if(!compareVectors(vvvSimulationTime1[c][v], vSimulationTime2, 0.001))
                            {
                                printErrorMessage("The two time vectors from each simulation was not the same, aborting!");
                                return false;
                            }

                            if(!compareVectors(vvvSimulationData1[c][v], vSimulationData2, var.tolerance))
                            {
                                printErrorMessage(string("Consistency test failed (two consecutive simulations gave different results): ") + var.fullVarName);
                                return false;
                            }
                        }

                        // Print the messages if there were any errors or warnings (but still success)
                        if ( (gHopsanCore.getNumErrorMessages() + gHopsanCore.getNumFatalMessages() + gHopsanCore.getNumWarningMessages()) != 0)
                        {
                            printWaitingMessages(false);
                        }
                    }

                    // =================================
                    // Now lets compare all the vectors
                    // =================================

                    vector<double> &vTime = vvvSimulationTime1[0][0];
                    //! @todo compare the time vectors somehow

                    for (size_t v=0; v<validationVariables.size(); ++v)
                    {
                        vector<double> vReferenceData;
                        ValidatonVariable &rVar = validationVariables[v];

                        // Build the reference vector for each variable, by interpolating from reference data file
                        // Interpolation prevents failure if nLogSamples would change (provided sample frequency is not decreased to much)
                        vReferenceData.reserve(vTime.size());
                        for(size_t i=0; i<vTime.size(); ++i)
                        {
                            vReferenceData.push_back(refData.interpolate(vTime[i], rVar.timecolumn, rVar.column));
                        }

                        for (size_t c=0; c<vvvSimulationData1.size(); ++c)
                        {
                            if(!compareVectors(vvvSimulationData1[c][v], vReferenceData, rVar.tolerance))
                            {
                                printColorMessage(Red, string("Validation data test failed: ") + rVar.fullVarName + " " + to_string(c+1) + " Cores");
                                return false;
                            }
                            printColorMessage(Green, string("Test successful: ") + rVar.fullVarName + " " + to_string(c+1) + " Cores");
                        }
                    }
                }
                else
                {
                    printWaitingMessages(false);
                    printErrorMessage("Could not load modelfile without errors: " + modelfile);
                    return false;
                }
            }
            else
            {
                printErrorMessage("Unsupported hvc version: "+ hvcVersion);
            }
            pValidationNode = pValidationNode->next_sibling("validation");
        }
    }
    catch(std::exception &e)
    {
        printErrorMessage(string("Could not open or read file: ") + hvcFilePath + string(" ") + e.what());
        return false;
    }

    // Test was apparently succesfull or else we would not have reached this point
    return true;
}

bool createModelTestDataSet(const string modelPath, const string hvcFilePath)
{
    double startT, stopT;
    ComponentSystem *pSystem = gHopsanCore.loadHMFModelFile(modelPath.c_str(), startT, stopT);
    if (!pSystem)
    {
        printWaitingMessages(false);
        return false;
    }

    SimulationHandler simhandler;

    bool iOk=false, sOk=false, fOk=false;
    // Initialize
    iOk = simhandler.initializeSystem(startT, stopT, pSystem);
    if (iOk)
    {
        // Simulate
        sOk = simhandler.simulateSystem(startT, stopT, -1, pSystem);
        if (!sOk)
        {
           printErrorMessage("Simulate Failed!");
        }
    }
    else
    {
        printErrorMessage("Initialize Failed! Aborting");
    }
    // Finalize
    fOk = simhandler.simulateSystem(startT, stopT, -1, pSystem);
    if (!fOk)
    {
       printErrorMessage("Finalize Failed!");
    }

    std::vector<Port*> data_ports;
    std::vector<size_t> data_ids;

    // Now find all scopes and save connected data as validation data
    const std::vector<Component*> components =  pSystem->getSubComponents();
    for (size_t c=0; c<components.size(); ++c)
    {
        const Component *pComponent = components[c];

        //cout << pComponent->getTypeName().c_str() << endl;
        if (pComponent->getTypeName() == "SignalSink")
        {
            //cout << " found SignalSink" << endl;
            const std::vector<Port*> scope_ports = pComponent->getPortPtrVector();
            for (size_t sp=0; sp<scope_ports.size(); ++sp)
            {
                std::vector<Port*> ports = scope_ports[sp]->getConnectedPorts();
                data_ports.insert(data_ports.end(), ports.begin(), ports.end());
                data_ids.push_back(0); //!< @todo here we assume that we only save signals
            }
        }
        else if (pComponent->isComponentSystem())
        {
            printErrorMessage("Handeling subsystems is not yet supported!");
        }
    }

    exportHVCData(modelPath, hvcFilePath, data_ports, data_ids);

    return false;
}


