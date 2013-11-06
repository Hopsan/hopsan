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

#include "generators/HopsanGenerator.h"
#include "generators/HopsanModelicaGenerator.h"
#include "generators/HopsanSimulinkGenerator.h"
#include "generators/HopsanLabViewGenerator.h"
#include "generators/HopsanFMIGenerator.h"
#include "HopsanTypes.h"
#include "win32dll.h"

#include <QMessageBox>

using namespace std;


//! @brief Calls the Modelica generator
//! @param modelicaCode Modelica code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callModelicaGenerator(hopsan::HString path, bool showDialog=false, int solver=0)
{
    HopsanModelicaGenerator *pGenerator = new HopsanModelicaGenerator("", "", showDialog);
    pGenerator->generateFromModelica(QString(path.c_str()), HopsanGenerator::SolverT(solver));
    delete(pGenerator);
}


//! @brief Calls the Modelica generator
//! @param modelicaCode Modelica code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callLibraryGenerator(hopsan::HString path, vector<hopsan::HString> hppFiles, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator("", "", showDialog);
    QStringList tempList;
    for(size_t i=0; i<hppFiles.size(); ++i)
    {
        tempList.append(QString(hppFiles[i].c_str()));
    }
    pGenerator->generateNewLibrary(QString(path.c_str()), tempList);
    delete(pGenerator);
}


//! @brief Calls the C++ generator
//! @param cppCode C++ code
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callCppGenerator(hopsan::HString hppPath)
{
    qDebug() << "Called C++ generator (in dll)!";

    QFile hppFile(QString(hppPath.c_str()));
    hppFile.open(QFile::ReadOnly);
    QString code = hppFile.readAll();
    hppFile.close();

    //Needed: typeName, displayName, cqsType
    QString typeName = code.section("class ", 1, 1).section(" ",0,0);
    QString displayName = typeName;

    QStringList portNames;
    QStringList lines = code.split("\n");
    for(int l=0; l<lines.size(); ++l)
    {
        if(lines.at(l).contains("addPowerPort") || lines.at(l).contains("addReadPort") || lines.at(l).contains("addWritePort") ||
           lines.at(l).contains("addPowerMultiPort") || lines.at(l).contains("addReadMultiPort"))
        {
            portNames.append(lines.at(l).section("\"",1,1));
        }
    }

    QFile xmlFile;
    xmlFile.setFileName(QFileInfo(hppFile).filePath()+typeName+".xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
        xmlStream << "  <modelobject typename=\"" << typeName << "\" displayname=\"" << displayName << "\">\n";
        xmlStream << "    <icons/>\n";
        xmlStream << "    <ports>\n";
        double xDelay = 1.0/(portNames.size()+1.0);
        double xPos = xDelay;
        double yPos = 0;
        for(int i=0; i<portNames.size(); ++i)
        {
            xmlStream << "      <port name=\"" << portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
            xPos += xDelay;
        }
        xmlStream << "    </ports>\n";
        xmlStream << "  </modelobject>\n";
        xmlStream << "</hopsanobjectappearance>\n";
        xmlFile.close();
    }
}


//! @brief Calls the functional mockup interface (FMU) import generator
//! @param path Path to the .fmu file
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callFmuImportGenerator(hopsan::HString path, hopsan::HString targetPath, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanFMIGenerator *pGenerator = new HopsanFMIGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateFromFmu(QString(path.c_str()), QString(targetPath.c_str()));
    //Find
    QString typeName = QFileInfo(QString(path.c_str())).baseName();
    pGenerator->generateNewLibrary(QString(targetPath.c_str())+"/"+typeName+"/", QStringList() << QString(targetPath.c_str())+"/component_code/"+typeName+".hpp");
    delete(pGenerator);
}


//! @brief Calls the functional mockup interface (FMU) export generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callFmuExportGenerator(hopsan::HString path, hopsan::ComponentSystem *pSystem, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanFMIGenerator *pGenerator = new HopsanFMIGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
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
extern "C" DLLIMPORTEXPORT void callSimulinkExportGenerator(const hopsan::HString path, const hopsan::HString modelFile, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanSimulinkGenerator *pGenerator = new HopsanSimulinkGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToSimulink(QString(path.c_str()), QString(modelFile.c_str()), pSystem, disablePortLabels, compiler);
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
extern "C" DLLIMPORTEXPORT void callSimulinkCoSimExportGenerator(hopsan::HString path, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanSimulinkGenerator *pGenerator = new HopsanSimulinkGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToSimulinkCoSim(QString(path.c_str()), pSystem, disablePortLabels, compiler);
    delete(pGenerator);
}


//! @brief Calls the LabVIEW SIT generator
//! @param path Path to export to
//! @param pSystem Pointer to system that shall be exported
//! @param coreIncludePath Path to HopsanCore include files
//! @param binPath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window

extern "C" DLLIMPORTEXPORT void callLabViewSITGenerator(hopsan::HString path, hopsan::ComponentSystem *pSystem, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanLabViewGenerator *pGenerator = new HopsanLabViewGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateToLabViewSIT(QString(path.c_str()), pSystem);
    delete(pGenerator);
}


//! @brief Calls the component library compile utility
//! @param path Path to library
//! @param name Name of output dll
//! @param extraLinks Additional linker commands
//! @param coreIncludePath Path to HopsanCore include files
//! @param binpath Path to HopsanCore binary files
//! @param showDialog True if generator output shall be displayed in a dialog window
extern "C" DLLIMPORTEXPORT void callComponentLibraryCompiler(hopsan::HString path, hopsan::HString extraLinks, hopsan::HString coreIncludePath, hopsan::HString binPath, bool showDialog=false)
{
    HopsanGenerator *pGenerator = new HopsanGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    compileComponentLibrary(QString(path.c_str()), pGenerator, QString(extraLinks.c_str()));
    delete(pGenerator);
}

