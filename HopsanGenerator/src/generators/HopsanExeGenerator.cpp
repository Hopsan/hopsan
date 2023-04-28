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

#include "generators/HopsanExeGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include <cassert>
#include <QUuid>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QUrl>

#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace hopsan;

HopsanExeGenerator::HopsanExeGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath)
    : HopsanGeneratorBase(hopsanInstallPath, compilerPath, tempPath)
{
}



bool HopsanExeGenerator::generateToExe(QString savePath, ComponentSystem *pSystem, const QStringList &externalLibraries, bool x64)
{
#ifdef _WIN32
    if(mCompilerSelection.path.isEmpty())
    {
        printErrorMessage("Compiler path not specified.");
        return false;
    }
#endif
    // Set build directory
    const QString buildPath = QString("%1/exe-build").arg(savePath);
    // Clear old build artefacts
    removeDir(buildPath);

    // Create required directories
    QDir saveDir(savePath);
    saveDir.mkpath(buildPath);


    //------------------------------------------------------------------//
    //Obtain model name and version string
    //------------------------------------------------------------------//

    QString modelName = pSystem->getName().c_str();

    //------------------------------------------------------------------//
    //Copy HopsanCore files to export directory
    //------------------------------------------------------------------//

    if(!copyHopsanCoreSourceFilesToDir(buildPath)) {
        printErrorMessage("Failed to copy Hopsan source code files.");
        return false;
    }
    if(!copyDefaultComponentCodeToDir(buildPath)) {
        printErrorMessage("Failed to copy default component library files.");
        return false;
    }

    if(!copyExternalComponentCodeToDir(buildPath, externalLibraries, mExtraSourceFiles, mIncludePaths, mLinkPaths, mLinkLibraries)) {
        printErrorMessage("Failed to export required external component library files.");
        return false;
    }
    printMessage("Extra source files: "+mExtraSourceFiles.join(","));

    //------------------------------------------------------------------//
    //Copy source files from templates
    //------------------------------------------------------------------//

    bool c1 = copyFile(":/templates/exe_main.cpp", buildPath+"/exe_main.cpp");
    bool c2 = copyFile(":/templates/exe_utilities.cpp", buildPath+"/exe_utilities.cpp");
    bool c3 = copyFile(":/templates/exe_utilities.h", buildPath+"/exe_utilities.h");
    if (!(c1 && c2 && c3)) {
        printErrorMessage("Failed to copy template file(s)");
        return false;
    }

    //------------------------------------------------------------------//
    // Generate model file and export assets, replacing asset paths in model
    //------------------------------------------------------------------//

    QMap<QString, QString> assetsMap;
    copyModelAssetsToDir(savePath+"/"+modelName+"-resources", pSystem, assetsMap);

    bool genOK = generateModelFile(pSystem, buildPath, assetsMap);
    if (!genOK) {
        printErrorMessage("Failed to generate model file");
        return false;
    }

    //------------------------------------------------------------------//
    // Compiling and linking
    //------------------------------------------------------------------//

    if(!compileAndLinkExe(buildPath, modelName, x64))
    {
        return false;
    }

    printMessage("Finished.");
    return true;
}


bool HopsanExeGenerator::compileAndLinkExe(const QString &buildPath, const QString &modelName, bool x64) const
{
    printMessage("------------------------------------------------------------------------");
    printMessage("Compiling C++ files");
    printMessage("------------------------------------------------------------------------");

    bool cppCompileOK=false;
#ifdef _WIN32
    QFile compileCppBatchFile;
    compileCppBatchFile.setFileName(buildPath + "/compileCpp.bat");
    if(!compileCppBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compileCpp.bat for writing.");
        return false;
    }

    //Write the compilation script file
    QTextStream compileCppBatchStream(&compileCppBatchFile);
    compileCppBatchStream << "@echo off\n";
    compileCppBatchStream << "PATH=" << mCompilerSelection.path << ";%PATH%\n";
    compileCppBatchStream << "@echo on\n";
    compileCppBatchStream << "g++ -pipe -std=c++14 -c -DHOPSAN_INTERNALDEFAULTCOMPONENTS -DHOPSAN_INTERNAL_EXTRACOMPONENTS " << "exe_main.cpp exe_utilities.cpp " << mExtraSourceFiles.join(" ");
    QStringList srcFiles = listHopsanCoreSourceFiles(buildPath) + listInternalLibrarySourceFiles(buildPath);
    for(const QString &srcFile : srcFiles) {
        compileCppBatchStream << " " << srcFile;
    }
    // Add HopsanCore (and necessary dependency) include paths
    for(const QString includePath : getHopsanCoreIncludePaths())
    {
       compileCppBatchStream << QString(" -I\"%1\"").arg(includePath);
    }
    for(const QString includePath : mIncludePaths) {
        compileCppBatchStream << QString(" -I\"%1\"").arg(includePath);
    }
    compileCppBatchFile.close();

    cppCompileOK = callProcess("cmd.exe", QStringList() << "/c" << "cd /d " + buildPath + " & compileCpp.bat");
#else
    QFile compileCppBatchFile;
    compileCppBatchFile.setFileName(buildPath + "/compileCpp.sh");
    if(!compileCppBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compileCpp.sh for writing.");
        return false;
    }
    //Write the compilation script file
    QTextStream compileCppBatchStream(&compileCppBatchFile);
    compileCppBatchStream << mCompilerSelection.path+"g++ -pipe -std=c++14 -c -DHOPSAN_INTERNALDEFAULTCOMPONENTS -DHOPSAN_INTERNAL_EXTRACOMPONENTS " << "exe_main.cpp exe_utilities.cpp " << mExtraSourceFiles.join(" ");
    QStringList srcFiles = listHopsanCoreSourceFiles(buildPath) + listInternalLibrarySourceFiles(buildPath);
    for(const QString &srcFile : srcFiles) {
        compileCppBatchStream << " " << srcFile;
    }
    // Add HopsanCore (and necessary dependency) include paths
    for(const QString& includePath : getHopsanCoreIncludePaths()) {
        compileCppBatchStream << QString(" -I\"%1\"").arg(includePath);
    }
    for(const QString& includePath : mIncludePaths) {
        compileCppBatchStream << QString(" -I\"%1\"").arg(includePath);
    }
    compileCppBatchFile.close();

    cppCompileOK = callProcess("/bin/sh", QStringList() << "compileCpp.sh", buildPath);
#endif
    if (!cppCompileOK) {
        printErrorMessage("Failed to compile exported C++ Hopsan code for executable model.");
        return false;
    }

    QStringList objectFiles;
    objectFiles << "exe_main.o" << "exe_utilities.o";
    for(const QString& extraSrc : mExtraSourceFiles) {
        QFileInfo extraObjFile(extraSrc);
        objectFiles << extraObjFile.baseName()+".o";
    }
    for(const QString &srcFile : srcFiles) {
        QFileInfo fi(srcFile);
        objectFiles << fi.baseName()+".o";
    }

    if(!assertFilesExist(buildPath, objectFiles)) {
        return false;
    }

    printMessage("------------------------------------------------------------------------");
    printMessage("Linking");
    printMessage("------------------------------------------------------------------------");

    bool linkingOK=false;

    QString outputExecutableFile;
#ifdef _WIN32
    outputExecutableFile = QString("%1/%2.exe").arg(buildPath).arg(modelName);
#elif __linux__
    outputExecutableFile = QString("%1/%2").arg(buildPath).arg(modelName);
#endif
    //! @todo Add OSX support
    // Create output directory before linking
    QDir systemRootDir("");
    systemRootDir.mkpath(QFileInfo(outputExecutableFile).absolutePath());

#ifdef _WIN32
    QFile linkBatchFile;
    linkBatchFile.setFileName(buildPath + "/link.bat");
    if(!linkBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open link.bat for writing.");
        return false;
    }
    //Write the compilation script file
    QTextStream linkBatchStream(&linkBatchFile);
    linkBatchStream << "@echo off\n";
    linkBatchStream << "PATH=" << mCompilerSelection.path << ";%PATH%\n";
    linkBatchStream << "@echo on\n";
    linkBatchStream << "g++ -w -static -static-libgcc";
    for(const QString &objFile : objectFiles) {
        linkBatchStream << " " << objFile;
    }
    linkBatchStream << " -o \""+outputExecutableFile+"\"\n";
    for(const QString linkPath : mLinkPaths) {
        linkBatchStream << " -L\"" << linkPath << "\"";
    }
    for(const QString linkLibrary : mLinkLibraries) {
        linkBatchStream << " -l" << linkLibrary;
    }
    linkBatchFile.close();

    linkingOK = callProcess("cmd.exe", QStringList() << "/c" << "cd /d " + buildPath + " & link.bat");

#else
    QFile linkBatchFile;
    linkBatchFile.setFileName(buildPath + "/link.sh");
    if(!linkBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open link.sh for writing.");
        return false;
    }
    //Write the compilation script file
    QTextStream linkBatchStream(&linkBatchFile);
    linkBatchStream << mCompilerSelection.path+"g++ -pthread -w";
    for(const QString &objFile : objectFiles) {
        linkBatchStream << " " << objFile;
    }
    for(const QString& linkPaths : mLinkPaths) {
        linkBatchStream << " -L\"" << linkPaths << "\"";
    }
    for(const QString& linkLibrary : mLinkLibraries) {
        linkBatchStream << " -l" << linkLibrary;
    }
    linkBatchStream << " -ldl";
    linkBatchStream << " -o \""+outputExecutableFile << "\"\n";
    linkBatchFile.close();

    linkingOK = callProcess("/bin/sh", QStringList() << "link.sh", buildPath);

#endif

    if (!linkingOK) {
        printErrorMessage("Failed to link exported code for executable model.");
        return false;
    }

    QDir outputDir = QFileInfo(outputExecutableFile).absoluteDir();
    outputDir.cdUp();
#ifdef _WIN32
    QString targetFilePath = outputDir.absolutePath()+"/"+modelName+".exe";
#else
    QString targetFilePath = outputDir.absolutePath()+"/"+modelName;
#endif
    copyFile(outputExecutableFile, targetFilePath);
    setRWXRWXRW_FilePermissions(targetFilePath);

    return assertFilesExist("", QStringList() << outputExecutableFile);
}

