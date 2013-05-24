#include "generators/HopsanLabViewGenerator.h"
#include "ComponentSystem.h"
#include <cassert>

using namespace hopsan;

HopsanLabViewGenerator::HopsanLabViewGenerator(QString coreIncludePath, QString binPath, bool showDialog)
    : HopsanGenerator(coreIncludePath, binPath, showDialog)
{

}


void HopsanLabViewGenerator::generateToLabViewSIT(QString savePath, hopsan::ComponentSystem *pSystem)
{
    printMessage("Initializing LabVIEW/SIT export");


    QFileInfo fileInfo;
    fileInfo.setFile(savePath);


    printMessage("Creating "+fileInfo.fileName());


    QFile file;
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open file for writing: " + savePath);
        return;
    }

    printMessage("Generating lists for input and output ports");

    //Create lists for input and output interface components
    QStringList inputs;
    QStringList outputs;
    QStringList mechCinterfaces;
    QStringList mechQinterfaces;
    QStringList hydCinterfaces;
    QStringList hydQinterfaces;

    std::vector<std::string> compNames = pSystem->getSubComponentNames();
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
        QString tempString = toVarName(inputs.at(i));
        replaceInports.append("    double "+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceInports.append("    double "+tempString+"C;\n");
        replaceInports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceInports.append("    double "+tempString+"F;\n");
        replaceInports.append("    double "+tempString+"X;\n");
        replaceInports.append("    double "+tempString+"V;\n");
        replaceInports.append("    double "+tempString+"M;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceInports.append("    double "+tempString+"C;\n");
        replaceInports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceInports.append("    double "+tempString+"P;\n");
        replaceInports.append("    double "+tempString+"Q;\n");
    }

    QString replaceOutports;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceOutports.append("    double "+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"F;\n");
        replaceOutports.append("    double "+tempString+"X;\n");
        replaceOutports.append("    double "+tempString+"V;\n");
        replaceOutports.append("    double "+tempString+"M;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"C;\n");
        replaceOutports.append("    double "+tempString+"Zc;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"P;\n");
        replaceOutports.append("    double "+tempString+"Q;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceOutports.append("    double "+tempString+"C;\n");
        replaceOutports.append("    double "+tempString+"Zc;\n");
    }

    QString replaceInportSize = QString::number(inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size());
    QString replaceOutportSize = QString::number(outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size());

    QString replaceInportAttribs;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceInportAttribs.append("    { \""+tempString+"\", 1, 1},\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"F\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"X\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"V\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"M\", 1, 1},\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"P\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Q\", 1, 1},\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceInportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }

    QString replaceOutportAttribs;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"\", 1, 1},\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceInportAttribs.append("    { \""+tempString+"F\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"X\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"V\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"M\", 1, 1},\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"C\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Zc\", 1, 1},\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceOutportAttribs.append("    { \""+tempString+"P\", 1, 1},\n");
        replaceOutportAttribs.append("    { \""+tempString+"Q\", 1, 1},\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
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
        std::vector<std::string> parNames;
        pComp->getParameterNames(parNames);
        for(size_t p=0; p<parNames.size(); ++p)
        {
            char* parValue;
            pComp->getParameterValue(parNames[p], &parValue);
            replaceParameters.append("    setParameter(\"" + QString(compNames[n].c_str()) + "\", \"" + QString(parNames[p].c_str()) +  "\", " + QString(parValue) + ");\n");
        }
    }

    QString replaceIndata;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceIndata.append("        rtInport."+tempString+" = inData["+QString::number(i)+"];\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"C = inData["+QString::number(2*i+inputs.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Zc = inData["+QString::number(2*i+1+inputs.size())+"];\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"F = inData["+QString::number(4*i+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"X = inData["+QString::number(4*i+1+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"V = inData["+QString::number(4*i+2+inputs.size()+2*mechCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"M = inData["+QString::number(4*i+3+inputs.size()+2*mechCinterfaces.size())+"];\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"C = inData["+QString::number(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Zc = inData["+QString::number(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceIndata.append("        rtInport."+tempString+"P = inData["+QString::number(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n");
        replaceIndata.append("        rtInport."+tempString+"Q = inData["+QString::number(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n");
    }

    QString replaceWriteNodeData;
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = toVarName(inputs.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+inputs.at(i)+"\", \"out\", 0, rtInport."+tempString+");\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n");
        replaceIndata.append("    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"F);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 2, rtInport."+tempString+"X);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"V);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 5, rtInport."+tempString+"M);\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceWriteNodeData.append("    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"P);\n");
        replaceWriteNodeData.append("    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"Q);\n");
    }

    QString replaceReadNodeData;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+" = readNodeData(\""+outputs.at(i)+"\", \"in\", 0);\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"F = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 1);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"X = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 2);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"V = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 0);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"M = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 5);\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"C = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 3);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Zc = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 4);\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"P = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 1);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Q = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 0);\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
        replaceReadNodeData.append("    rtOutport."+tempString+"C = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 3);\n");
        replaceReadNodeData.append("    rtOutport."+tempString+"Zc = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 4);\n");
    }

    QString replaceOutData;
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = toVarName(outputs.at(i));
        replaceOutData.append("        outData["+QString::number(i)+"] = rtOutport."+tempString+";\n");
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechCinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(4*i+outputs.size())+"] = rtOutport."+tempString+"F;\n");
        replaceOutData.append("        outData["+QString::number(4*i+1+outputs.size())+"] = rtOutport."+tempString+"X;\n");
        replaceOutData.append("        outData["+QString::number(4*i+2+outputs.size())+"] = rtOutport."+tempString+"V;\n");
        replaceOutData.append("        outData["+QString::number(4*i+3+outputs.size())+"] = rtOutport."+tempString+"M;\n");
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(mechQinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"C;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n");
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydCinterfaces.at(i));
        replaceOutData.append("        outData["+QString::number(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"P;\n");
        replaceOutData.append("        outData["+QString::number(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"Q;\n");
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = toVarName(hydQinterfaces.at(i));
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

    //! @todo Check if success, otherwise tell user with error message
    printMessage("Finished!");
}
