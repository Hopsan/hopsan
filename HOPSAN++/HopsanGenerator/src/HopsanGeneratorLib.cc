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
//! @file   ComponentGeneratorLib.cc
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains the exported functions for component generator library
//!
//$Id$

#include "HopsanComponentGenerator.h"
#include "win32dll.h"

#include <QMessageBox>

using namespace std;


//! @brief Calls the Modelica generator
//! @param modelicaCode Modelica code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callModelicaGenerator(string modelicaCode, string coreIncludePath, string binPath, bool showDialog=false)
{
    qDebug() << "Called Modelica generator (in dll)!";

    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateFromModelica(QString(modelicaCode.c_str()));
    delete(pGenerator);
}


//! @brief Calls the C++ generator
//! @param cppCode C++ code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callCppGenerator(string cppCode, string coreIncludePath, string binPath, bool showDialog=false)
{
    qDebug() << "Called C++ generator (in dll)!";

    QString code = QString(cppCode.c_str());

    //Needed: typeName, displayName, cqsType
    QString typeName = code.section("class ", 1, 1).section(" ",0,0);
    QString displayName = typeName;
    QString cqsType = code.section("public ",1,1).section("\n",0,0);
    cqsType.remove("Component");
    cqsType = cqsType[0];

    qDebug() << "Type name: " << typeName;
    qDebug() << "CQS Type: " << cqsType;

    ComponentSpecification comp = ComponentSpecification(typeName, displayName, cqsType);

    QList<PortSpecification> ports;
    QStringList lines = code.split("\n");
    for(int l=0; l<lines.size(); ++l)
    {
        if(lines.at(l).contains("addPowerPort"))
        {
            QString portType = "PowerPort";
            QString portName = lines.at(l).section("\"",1,1);
            QString nodeType = lines.at(l).section("\"",3,3);
            ports.append(PortSpecification(portType, nodeType, portName, false, 0));
            comp.portNames.append(portName);
        }
        else if(lines.at(l).contains("addReadPort"))
        {
            QString portType = "ReadPort";
            QString portName = lines.at(l).section("\"",1,1);
            QString nodeType = lines.at(l).section("\"",3,3);
            ports.append(PortSpecification(portType, nodeType, portName, false, 0));
            comp.portNames.append(portName);
        }
        else if(lines.at(l).contains("addWritePort"))
        {
            QString portType = "WritePort";
            QString portName = lines.at(l).section("\"",1,1);
            QString nodeType = lines.at(l).section("\"",3,3);
            ports.append(PortSpecification(portType, nodeType, portName, false, 0));
            comp.portNames.append(portName);
        }
        else if(lines.at(l).contains("addPowerMultiPort"))
        {
            QString portType = "PowerMultiPort";
            QString portName = lines.at(l).section("\"",1,1);
            QString nodeType = lines.at(l).section("\"",3,3);
            ports.append(PortSpecification(portType, nodeType, portName, false, 0));
            comp.portNames.append(portName);
        }
        else if(lines.at(l).contains("addReadMultiPort"))
        {
            QString portType = "ReadMultiPort";
            QString portName = lines.at(l).section("\"",1,1);
            QString nodeType = lines.at(l).section("\"",3,3);
            ports.append(PortSpecification(portType, nodeType, portName, false, 0));
            comp.portNames.append(portName);
        }
    }

    //Add header guard
    code.prepend("#ifndef "+typeName.toUpper()+"_HPP_INCLUDED\n#define "+typeName.toUpper()+"_HPP_INCLUDED\n\n");
    code.append("\n#endif // "+typeName.toUpper()+"_HPP_INCLUDED\n");

    comp.plainCode = code;

    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->compileFromComponentObject(typeName+".hpp", comp, false);
    delete(pGenerator);
}


//! @brief Calls the functional mockup interface (FMU) import generator
//! @param path Path to the .fmu file
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callFmuImportGenerator(string path, string coreIncludePath, string binPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateFromFmu(QString(path.c_str()));
    delete(pGenerator);
}


//! @brief Calls the functional mockup interface (FMU) export generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callFmuExportGenerator(string path, hopsan::ComponentSystem *pSystem, string coreIncludePath, string binPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToFmu(QString(path.c_str()), pSystem);
    delete(pGenerator);
}


//! @brief Calls the Simulink S-function generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param compiler Compiler to use, 0 = MSVC2008 32-bit, 1 = MSVC2008 64-bit, 2 = MSVC2010 32-bit, 3 = MSVC2010 64-bit
//! @param disablePortLabels Tells whether or not port labels shall be disabled (for compatibility with older MATLAB versions)
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callSimulinkExportGenerator(string path, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, string coreIncludePath, string binPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToSimulink(QString(path.c_str()), pSystem, disablePortLabels, compiler);
    delete(pGenerator);
}


//! @brief Calls the Simulink S-function generator for co-simulations
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param compiler Compiler to use, 0 = MSVC2008 32-bit, 1 = MSVC2008 64-bit, 2 = MSVC2010 32-bit, 3 = MSVC2010 64-bit
//! @param disablePortLabels Tells whether or not port labels shall be disabled (for compatibility with older MATLAB versions)
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callSimulinkCoSimExportGenerator(string path, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, string coreIncludePath, string binPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToSimulinkCoSim(QString(path.c_str()), pSystem, disablePortLabels, compiler);
    delete(pGenerator);
}


//! @brief Calls the LabVIEW SIT generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callLabViewSITGenerator(string path, hopsan::ComponentSystem *pSystem, string coreIncludePath, string binPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToLabViewSIT(QString(path.c_str()), pSystem);
    delete(pGenerator);
}
