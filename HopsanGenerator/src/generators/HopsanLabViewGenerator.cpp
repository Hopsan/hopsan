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

#include "generators/HopsanLabViewGenerator.h"
#include "ComponentSystem.h"
#include <cassert>
#include <QFileInfo>
#include <QDir>

using namespace hopsan;

HopsanLabViewGenerator::HopsanLabViewGenerator(const QString &hopsanInstallPath)
    : HopsanGeneratorBase(hopsanInstallPath, {}, "")
{

}


bool HopsanLabViewGenerator::generateToLabViewSIT(QString savePath, hopsan::ComponentSystem *pSystem)
{
    printMessage("Initializing LabVIEW/SIT export");

    if(pSystem == 0)
    {
        printErrorMessage("System pointer is null. Aborting.");
        return false;
    }

    QFileInfo fileInfo;
    fileInfo.setFile(savePath);
    QString path = fileInfo.path();

    QFile modelHeaderFile;
    modelHeaderFile.setFileName(path + "/model.h");
    if(!modelHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open model.h for writing.");
        return false;
    }

    QFile codegenSourceFile;
    codegenSourceFile.setFileName(path + "/codegen.c");
    if(!codegenSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open codegen.c for writing.");
        return false;
    }

    QFile apiHeaderFile;
    apiHeaderFile.setFileName(path + "/SIT_API.h");
    if(!apiHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open SIT_API.h for writing.");
        return false;
    }

    QFile howToFile;
    howToFile.setFileName(path + "/HOW_TO_COMPILE.txt");
    if(!howToFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open SIT_API.h for writing.");
        return false;
    }

    printMessage("Writing model.h...");

    QFile modelHeaderTemplateFile(":templates/labviewModelTemplate.h");
    if(!modelHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open labviewModelTemplate.h for reading.");
        return false;
    }

    QString modelHeaderCode;
    QTextStream t1(&modelHeaderTemplateFile);
    modelHeaderCode = t1.readAll();
    modelHeaderTemplateFile.close();
    if(modelHeaderCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for model.h.");
        return false;
    }

    QTextStream modelHeaderStream(&modelHeaderFile);
    modelHeaderStream << modelHeaderCode;
    modelHeaderFile.close();


    printMessage("Writing codegen.c...");

    QFile codegenSourceTemplateFile(":templates/labviewCodegenTemplate.c");
    if(!codegenSourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open labviewCodegenTemplate.c for reading.");
        return false;
    }

    QString codegenSourceCode;
    QTextStream t2(&codegenSourceTemplateFile);
    codegenSourceCode = t2.readAll();
    codegenSourceTemplateFile.close();
    if(codegenSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for codegen.c.");
        return false;
    }

    QTextStream codegenSourceStream(&codegenSourceFile);
    codegenSourceStream << codegenSourceCode;
    codegenSourceFile.close();


    printMessage("Writing SIT_API.h...");

    QFile apiHeaderTemplateFile(":templates/labviewApiTemplate.h");
    if(!apiHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open labviewApiTemplate.h for reading.");
        return false;
    }

    QString apiHeaderCode;
    QTextStream t3(&apiHeaderTemplateFile);
    apiHeaderCode = t3.readAll();
    apiHeaderTemplateFile.close();
    if(apiHeaderCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for SIT_API.h.");
        return false;
    }

    QTextStream apiHeaderStream(&apiHeaderFile);
    apiHeaderStream << apiHeaderCode;
    apiHeaderFile.close();


    printMessage("Writing HOW_TO_COMPILE.txt...");

    QFile howToTemplateFile(":templates/labviewHowToTemplate.txt");
    if(!howToTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open labviewHowToTemplate.txt for reading.");
        return false;
    }

    QString howToCode;
    QTextStream t4(&howToTemplateFile);
    howToCode = t4.readAll();
    howToTemplateFile.close();
    if(howToCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HOW_TO_COMPILE.txt.");
        return false;
    }

    QTextStream howToStream(&howToFile);
    howToStream << howToCode;
    howToFile.close();


    printMessage("Creating "+fileInfo.fileName());

    QFile file;
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open file for writing: " + savePath);
        return false;
    }

    printMessage("Generating lists for input and output ports");

    //Create lists for input and output interface components
    QStringList inputs;
    QStringList outputs;
    QStringList mechCinterfaces;
    QStringList mechQinterfaces;
    QStringList hydCinterfaces;
    QStringList hydQinterfaces;

    std::vector<HString> compNames = pSystem->getSubComponentNames();
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        if(pComp->getTypeName() == "SignalInputInterface")
        {
            inputs.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "SignalOutputInterface")
        {
            outputs.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "MechanicInterfaceC")
        {
            mechCinterfaces.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "MechanicInterfaceQ")
        {
            mechQinterfaces.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "HydraulicInterfaceC")
        {
            hydCinterfaces.append(compNames[n].c_str());
        }
        else if(pComp->getTypeName() == "HydraulicInterfaceQ")
        {
            hydQinterfaces.append(compNames[n].c_str());
        }
    }

    printMessage("Writing " + fileInfo.fileName());

    QFile wrapperTemplateFile(":templates/labviewWrapperTemplate.cpp");
    assert(wrapperTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString wrapperCode;
    QTextStream t(&wrapperTemplateFile);
    wrapperCode = t.readAll();
    wrapperTemplateFile.close();
    assert(!wrapperCode.isEmpty());

        //Write initial comment

    QString replaceInports;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(inputs.at(i));
        replaceInports.append("    double "+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceInports.append("    double "+tempString+"C;\n");
        replaceInports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceInports.append("    double "+tempString+"F;\n");
        replaceInports.append("    double "+tempString+"X;\n");
        replaceInports.append("    double "+tempString+"V;\n");
        replaceInports.append("    double "+tempString+"M;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceInports.append("    double "+tempString+"C;\n");
        replaceInports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceInports.append("    double "+tempString+"P;\n");
        replaceInports.append("    double "+tempString+"Q;\n");
    }

    QString replaceOutports;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(outputs.at(i));
        replaceOutports.append("    double "+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"F;\n");
        replaceOutports.append("    double "+tempString+"X;\n");
        replaceOutports.append("    double "+tempString+"V;\n");
        replaceOutports.append("    double "+tempString+"M;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"C;\n");
        replaceOutports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"P;\n");
        replaceOutports.append("    double "+tempString+"Q;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"C;\n");
        replaceOutports.append("    double "+tempString+"Zc;\n");
    }

    QString replaceInportSize = QString::number(inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size());
    QString replaceOutportSize = QString::number(outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size());

    QString replaceInportAttribs;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(inputs.at(i));
        replaceInportAttribs.append("    { \""+tempString+"\", 1, 1},\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"F\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"X\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"V\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"M\", 1, 1},\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"P\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Q\", 1, 1},\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }

    QString replaceOutportAttribs;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(outputs.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"\", 1, 1},\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"F\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"X\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"V\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"M\", 1, 1},\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"P\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Q\", 1, 1},\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }

    QString replaceComponents;
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        replaceComponents.append("    addComponent(\"" + QString(compNames[n].c_str()) + QString("\", \"") + QString(pComp->getTypeName().c_str())+QString("\");\n"));
    }

    QString replaceConnections;
    QList<QPair<QPair<QString, QString>, QPair<QString, QString> > > connections;
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        for(size_t p=0; p<pComp->getPortNames().size(); ++p)
        {
            Port *pPort = pComp->getPortPtrVector()[p];
            if(pPort->isMultiPort())
            {
                continue;
            }
            for(size_t c=0; c<pPort->getConnectedPorts().size(); ++c)
            {
                Port *pPort2 = pPort->getConnectedPorts()[c];
                if(pPort2->getParentPort())
                {
                    pPort2 = pPort2->getParentPort();
                }
                QString comp1 = compNames[n].c_str();
                QString port1 = pPort->getName().c_str();
                QString comp2 = pPort2->getComponentName().c_str();
                QString port2 = pPort2->getName().c_str();

                QPair<QString, QString> pair1 = QPair<QString, QString>(comp1, port1);
                QPair<QString, QString> pair2 = QPair<QString, QString>(comp2, port2);

                if(!connections.contains(QPair<QPair<QString, QString>, QPair<QString, QString> >(pair1, pair2)) &&
                   !connections.contains(QPair<QPair<QString, QString>, QPair<QString, QString> >(pair2, pair1)))
                {
                    connections.append(QPair<QPair<QString, QString>, QPair<QString, QString> >(pair1, pair2));
                }
            }
        }
    }
    for(int i = 0; i != connections.size(); ++i)
    {
        replaceConnections.append("    connect(\"" +  connections[i].first.first + "\", \""+connections[i].first.second +
                      "\", \"" + connections[i].second.first + "\", \""+connections[i].second.second+"\");\n");
    }

    QString replaceParameters;
    for(size_t n=0; n<compNames.size(); ++n)
    {
        Component *pComp = pSystem->getSubComponent(compNames[n]);
        std::vector<HString> parNames;
        pComp->getParameterNames(parNames);
        for(size_t p=0; p<parNames.size(); ++p)
        {
            HString parValue;
            pComp->getParameterValue(parNames[p], parValue);
            replaceParameters.append("    setParameter(\"" + QString(compNames[n].c_str()) + "\", \"" + QString(parNames[p].c_str()) +  "\", " + QString(parValue.c_str()) + ");\n");
        }
    }

    QString replaceIndata;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(inputs.at(i));
        replaceIndata.append("        rtInport."+tempString+" = inData["+QString::number(i)+"];\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"C = inData["+QString::number(2*i+inputs.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Zc = inData["+QString::number(2*i+1+inputs.size())+"];\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"F = inData["+QString::number(4*i+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"X = inData["+QString::number(4*i+1+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"V = inData["+QString::number(4*i+2+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"M = inData["+QString::number(4*i+3+inputs.size()+2*mechCinterfaces.size())+"];\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"C = inData["+QString::number(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Zc = inData["+QString::number(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"P = inData["+QString::number(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Q = inData["+QString::number(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n");
    }

    QString replaceWriteNodeData;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(inputs.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+inputs.at(i)+"\", \"out\", 0, rtInport."+tempString+");\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n");
        replaceIndata.append("    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"F);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 2, rtInport."+tempString+"X);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"V);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 5, rtInport."+tempString+"M);\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"P);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"Q);\n");
    }

    QString replaceReadNodeData;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(outputs.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+" = readNodeData(\""+outputs.at(i)+"\", \"in\", 0);\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"F = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 1);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"X = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 2);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"V = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 0);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"M = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 5);\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"C = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 3);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Zc = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 4);\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"P = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 1);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Q = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 0);\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"C = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 3);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Zc = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 4);\n");
    }

    QString replaceOutData;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(outputs.at(i));
        replaceOutData.append("        outData["+QString::number(i)+"] = rtOutport."+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechCinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(4*i+outputs.size())+"] = rtOutport."+tempString+"F;\n");
        replaceOutData.append("        outData["+QString::number(4*i+1+outputs.size())+"] = rtOutport."+tempString+"X;\n");
        replaceOutData.append("        outData["+QString::number(4*i+2+outputs.size())+"] = rtOutport."+tempString+"V;\n");
        replaceOutData.append("        outData["+QString::number(4*i+3+outputs.size())+"] = rtOutport."+tempString+"M;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(mechQinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"C;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydCinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"P;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"Q;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toValidLabViewVarName(hydQinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size())+"] = rtOutport."+tempString+"C;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n");
    }

    wrapperCode.replace("<<<inports>>>", replaceInports);
    wrapperCode.replace("<<<outports>>>", replaceOutports);
    wrapperCode.replace("<<<inportsize>>>", replaceInportSize);
    wrapperCode.replace("<<<outportsize>>>", replaceOutportSize);
    wrapperCode.replace("<<<inportattribs>>>", replaceInportAttribs);
    wrapperCode.replace("<<<outportattribs>>>", replaceOutportAttribs);
    wrapperCode.replace("<<<components>>>", replaceComponents);
    wrapperCode.replace("<<<connections>>>", replaceConnections);
    wrapperCode.replace("<<<parameters>>>", replaceParameters);
    wrapperCode.replace("<<<indata>>>", replaceIndata);
    wrapperCode.replace("<<<writenodedata>>>", replaceWriteNodeData);
    wrapperCode.replace("<<<readnodedata>>>", replaceReadNodeData);
    wrapperCode.replace("<<<outdata>>>", replaceOutData);


    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    fileStream << wrapperCode;
    file.close();

    printMessage("Copying hopsanrt-wrapper.h");

    QFile wrapperTemplateHeaderFile(":templates/labviewWrapperTemplate.h");
    wrapperTemplateHeaderFile.copy(fileInfo.absoluteDir().path()+"/hopsanrt-wrapper.h");


    printMessage("Copying HopsanCore source code...");
    if(!this->copyHopsanCoreSourceFilesToDir(fileInfo.absoluteDir().path()))
        return false;

    printMessage("Copying component libraries source code...");
    if(!this->copyDefaultComponentCodeToDir(fileInfo.absoluteDir().path()))
        return false;

    //! @todo Check if success, otherwise tell user with error message
    printMessage("Finished!");
    return true;
}
