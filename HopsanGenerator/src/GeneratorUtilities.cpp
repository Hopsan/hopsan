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
//! @file   ComponentGeneratorUtilities.cpp
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utilities
//!
//$Id$

#include <QStringList>
#include <QProcess>
#include <QDomElement>
#include <QDirIterator>
#include <QDebug>

#include <cassert>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "GeneratorUtilities.h"
#include "generators/HopsanGeneratorBase.h"
#include "HopsanCoreVersion.h"

//! @brief Function for loading an XML DOM Document from file
//! @param[in] rFile The file to load from
//! @param[in,out] rDomDocument The DOM Document to load into
//! @param[in] rootTagName The expected root tag name to extract from the Dom Document
//! @returns The extracted DOM root element from the loaded DOM document
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName)
{
    QString errorStr;
    int errorLine, errorColumn;
    if (!rDomDocument.setContent(&rFile, false, &errorStr, &errorLine, &errorColumn))
    {
        //! @todo Error message somehow
    }
    else
    {
        QDomElement xmlRoot = rDomDocument.documentElement();
        if (xmlRoot.tagName() != rootTagName)
        {
            //! @todo Error message somehow
        }
        else
        {
            return xmlRoot;
        }
    }
    return QDomElement(); //NULL
}



bool removeDir(QString path)
{
    return QDir(path).removeRecursively();
}


//! @brief Copy a directory with contents
//! @param[in] fromPath The absolute path to the directory to copy
//! @param[in] toPath The absolute path to the destination (including destination dir name)
//! @param[in] excludeRegExps List of regexps for files to exclude
//! @param[out] rErrorMessage Error message if copy fail
//! @returns True if success else False
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
bool copyDir(const QString &fromPath, QString toPath, const QList<QRegExp>& excludeRegExps, QString &rErrorMessage)
{
    QDir toDir(toPath);
    if (!toDir.mkpath(toPath))
    {
        rErrorMessage = "Could not create directory: "+toPath;
        return false;
    }
    if (toPath.endsWith('/'))
    {
        toPath.chop(1);
    }

    QDir fromDir(fromPath);
    for(const QFileInfo& info : fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        // If symlink, check the destination name against exclusion list
        QString currentResolvedFileOrDirName = info.fileName();
        if (info.isSymLink()) {
            currentResolvedFileOrDirName = QFileInfo(info.symLinkTarget()).fileName();
        }
        // If this file or directory matches the exclude regexp, then do not copy it
        auto checkIfExclude = [&](const QRegExp& re) { return re.indexIn(currentResolvedFileOrDirName) > -1; };
        bool excludeThisFile = std::any_of(excludeRegExps.begin(), excludeRegExps.end(), checkIfExclude);
        if (excludeThisFile) {
            continue;
        }

        const QString srcPath = info.absoluteFilePath();
        const QString dstPath = toPath+"/"+info.fileName();

        if (info.isDir())
        {
            if(!copyDir(srcPath, dstPath, excludeRegExps, rErrorMessage))
            {
                return false;
            }
        }
        else
        {
            if(!copyFile(srcPath, dstPath, rErrorMessage))
            {
                return false;
            }
        }
    }
    return true;
}



// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}

QTextLineStream& operator <<(QTextLineStream &rLineStream, const QString &input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}



bool compileComponentLibrary(QString path, HopsanGeneratorBase *pGenerator, QString extraCFlags, QString extraLFlags)
{
    pGenerator->printMessage("Writing compilation script...");

    ComponentLibrary cl;
    QString libRootDir;
    if(QFileInfo(path).isFile())
    {
        cl.loadFromXML(path);
        libRootDir = QFileInfo(path).canonicalPath();
    }
    else if (QFileInfo(path).isDir())
    {
        cl.mSourceFiles = QDir(path).entryList(QStringList() << "*.cpp");
        cl.mSharedLibraryName = QDir(path).dirName();
        cl.mSharedLibraryDebugExtension = "_d";
        libRootDir = path;
    }
    else
    {
        pGenerator->printErrorMessage(path+" does not exist! It is not component library file or a directory containing one");
        return false;
    }

    using Compiler = CompilerHandler::Compiler;

    CompilerHandler ch(CompilerHandler::Language::Cpp);
    ch.addIncludePath(pGenerator->getHopsanCoreIncludePath());
    for(QString includePath : cl.mIncludePaths) {
        ch.addIncludePath(includePath);
    }
    ch.addCompilerFlag("-std=c++14 -fPIC -w", {Compiler::GCC, Compiler::Clang});
    //! @todo setting rpath here is strange, as it will hard-code given path into dll (so if you move it it wont work) /Peter
    ch.addCompilerFlag(QString(R"(-Wl,--rpath,"%1")").arg(libRootDir), Compiler::GCC);

    ch.addLibraryPath(pGenerator->getHopsanBinPath());
    ch.addLibraryPath(pGenerator->getHopsanLibPath());
    for(QString linkPath : cl.mLinkPaths) {
        ch.addLibraryPath(linkPath);
    }
    CompilerHandler::BuildType buildType = CompilerHandler::BuildType::Release;
#if defined(HOPSAN_BUILD_TYPE_DEBUG)
    buildType = CompilerHandler::BuildType::Debug;
    cl.mSharedLibraryName += cl.mSharedLibraryDebugExtension;
    ch.addDefinition("HOPSAN_BUILD_TYPE_DEBUG");
    ch.addLinkLibrary("hopsancore_d");
#else
    ch.addDefinition("HOPSAN_BUILD_TYPE_RELEASE");
    ch.addLinkLibrary("hopsancore");
#endif
#ifdef _WIN32
    ch.addDefinition("HOPSANCORE_DLLIMPORT", "");
#endif
    ch.addLinkLibrary("c++", {Compiler::Clang});
    for(QString linkLibPath : cl.mLinkLibraries) {
        ch.addLinkLibrary(linkLibPath);
    }

    ch.addBuildFlags(cl.mBuildFlags);
    ch.addCompilerFlag(extraCFlags);
    ch.addLinkerFlag(extraLFlags);

    ch.setSourceFiles(cl.mSourceFiles+cl.mExtraSourceFiles);
    ch.setSharedLibraryOutputFile(LIBPREFIX+cl.mSharedLibraryName, buildType);

    const auto& compilerSelection = pGenerator->getCompilerSelection();

    pGenerator->printMessage("\n");
    pGenerator->printMessage("Calling compiler utility:");
    pGenerator->printMessage("Work Directory: "+libRootDir);
    pGenerator->printMessage("Output file:    "+ch.outputFile());
    pGenerator->printMessage("Source files:   "+ch.sourceFiles().join(" "));
    pGenerator->printMessage("Compiler flags: "+ch.compilerFlags(compilerSelection.compiler).join(" "));
    pGenerator->printMessage("Linker flags:   "+ch.linkerFlags(compilerSelection.compiler).join(" "));
    pGenerator->printMessage("\n");

    pGenerator->printMessage("Compiling please wait!");
    QString output;
    bool success = compile(libRootDir, compilerSelection.path, ch, compilerSelection.compiler, output);
    pGenerator->printMessage(output);
    return success;
}


//! @brief Calls GCC or MinGW compiler with specified parameters
//! @param wdPath Absolute path where compiler shall be run
//! @param o Objective file name (without file extension)
//! @param srcFiles List with source files, example: "file1.cpp file2.cpp"
//! @param inclPaths Include paths, example: "-Ipath1 -Ipath2"
//! @param cflags Compiler flags
//! @param lflags Link paths and libs, example: "-Lpath1 -lfile1 -lfile2"
//! @param output Reference to string where output messages are stored
bool compile(QString wdPath, QString gccPath, QString o, QString srcFiles, QString inclPaths, QString cflags, QString lflags, QString &output)
{
    // Append dll extension for this platform
    o = o+SHAREDLIB_SUFFIX;

    // Create compilation script file
    QFile compileScript;
#ifdef _WIN32
    compileScript.setFileName(wdPath + "/compile.bat");
    if(!compileScript.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = "Could not open compile.bat for writing.";
        return false;
    }
    QTextStream clBatchStream(&compileScript);
    clBatchStream << "@echo off\n";
    clBatchStream << "set PATH=" << gccPath << ";%PATH%\n";
    clBatchStream << "@echo on\n";
    clBatchStream << "if exist " << o << " (" << "\n";
    clBatchStream << "  del " << o << "\n";
    clBatchStream << ")" << "\n";
    clBatchStream << "g++.exe " << cflags << " " << srcFiles << " " << inclPaths;
    clBatchStream << " -o " << o << " " << lflags <<"\n";
    compileScript.close();
#else
    Q_UNUSED(gccPath);
    compileScript.setFileName(wdPath + "/compile.sh");
    if(!compileScript.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = "Could not open compile.sh for writing.";
        return false;
    }
    QTextStream compileStream(&compileScript);
    compileStream << "#!/bin/sh\n";
    compileStream << "rm -f " << o << "\n";
    compileStream << "gcc " << cflags << " " << srcFiles;
    compileStream << " -fpermissive -o " << o << " ";
    compileStream << inclPaths << " " << lflags;
    compileScript.close();
#endif

    //Call compilation script file
    QString stdOut,stdErr;
    bool compiledOK = false;
#ifdef _WIN32
    compiledOK = (callProcess("cmd.exe", QStringList() << "/c" << "compile.bat", wdPath, 600, stdOut, stdErr) == 0);
#else
    compiledOK = (callProcess("/bin/sh", QStringList() << "compile.sh", wdPath, 600, stdOut, stdErr) == 0);
#endif
    output.append(stdOut);
    output.append("\n");
    output.append(stdErr);
    output.append("\n");

    if (!compiledOK) {
        output.append("Compilation failed.");
        return false;
    }

    QDir targetDir(wdPath);
    if(!targetDir.exists(o)) {
        output.append("Compilation failed.");
        return false;
    }

    output.append("Compilation successful.");
    return true;
}

//! @brief Generate script and calls compiler with specified parameters
//! @param[in] wdPath Absolute path to compilation work directory
//! @param[in] compilerPath Path to the directory containing the compiler (Example: C:\mingw64\bin)
//! @param[in] ch Compiler handler containing compiler and linker commands
//! @param[in] compiler The compiler to use
//! @param[out] output Reference to string where output messages are stored
bool compile(QString wdPath, QString compilerPath, CompilerHandler& ch, CompilerHandler::Compiler compiler, QString &output)
{
    // Create compilation script file
    QString batchScript = QString(
R"(@echo off
set PATH=%1;%PATH%
@echo on
if exist %2 (
  del %2
)
%3
)").arg(compilerPath).arg(ch.outputFile()).arg(ch.compileCommand(compiler));

    QString shellScript = QString(
R"(#!/bin/sh
PATH=%1:${PATH}
rm -f  %2
%3
)").arg(compilerPath).arg(ch.outputFile()).arg(ch.compileCommand(compiler));

    QFile compileScript;
#ifdef _WIN32
    compileScript.setFileName(wdPath + "/compile.bat");
    QString& rScript = batchScript;
#else
    compileScript.setFileName(wdPath + "/compile.sh");
    QString& rScript = shellScript;
#endif
    if(!compileScript.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = QString("Could not open %1 for writing.").arg(compileScript.fileName());
        return false;
    }
    QTextStream scriptStream(&compileScript);
    scriptStream << rScript;
    compileScript.close();

    // Call compilation script file
    QString stdOut,stdErr;
    bool compiledOK = false;
#ifdef _WIN32
    compiledOK = (callProcess("cmd.exe", QStringList() << "/c" << "compile.bat", wdPath, 600, stdOut, stdErr) == 0);
#else
    compiledOK = (callProcess("/bin/sh", QStringList() << "compile.sh", wdPath, 600, stdOut, stdErr) == 0);
#endif
    output.append(stdOut);
    output.append("\n");
    output.append(stdErr);
    output.append("\n");

    if (!compiledOK) {
        output.append("Compilation failed.");
        return false;
    }

    QDir targetDir(wdPath);
    if(!targetDir.exists(ch.outputFile())) {
        output.append("Compilation failed.");
        return false;
    }

    output.append("Compilation successful.");
    return true;
}



//! @brief Removes all illegal characters from the string, so that it can be used as a variable name.
//! @param name Original string
//! @returns String without illegal characters
QString toValidHopsanVarName(const QString &name)
{
    QString ret;
    if (!name.isEmpty())
    {
        // Reserve memory for entire string (we will only append as many chars as we decide to keep)
        ret.reserve(name.size());
        // First ignore any non letter or number or underscore char in the beginning
        int c=0;
        while ( (c<name.size()) && !(name[c].isLetterOrNumber() || name[c]=='_') )
        {
            ++c;
        }
        // Now ignore any non letter or number or underscore
        while ( c < name.size() )
        {
            if  ( name[c].isLetterOrNumber() || name[c]=='_' )
            {
                ret.append(name[c]);
            }
            ++c;
        }
    }
    return ret;
}

//! @brief Removes all illegal characters from the string, so that it can be used as a variable name.
//! @param name Original string
//! @returns String without illegal characters
QString toValidLabViewVarName(const QString &name)
{
    QString ret;
    if (!name.isEmpty())
    {
        // Reserve memory for entire string (we will only append as many chars as we decide to keep)
        ret.reserve(name.size());
        // First ignore any non letter char in the beginning
        int c=0;
        while ( (c<name.size()) && !name[c].isLetter() )
        {
            ++c;
        }
        // Now ignore any non letter or number
        while ( c < name.size() )
        {
            if ( name[c].isLetterOrNumber() )
            {
                ret.append(name[c]);
            }
            ++c;
        }
    }
    return ret;
}


QString extractTaggedSection(const QString &text, const QString &tagName)
{
    QString startStr = ">>>"+tagName+">>>";
    QString endStr = "<<<"+tagName+"<<<";
    if(text.contains(startStr) && text.contains(endStr))
    {
        int i = text.indexOf(startStr)+startStr.size();
        int n = text.indexOf(endStr)-i;
        return text.mid(i, n);
    }
    return QString();
}


void replaceTaggedSection(QString &text, const QString &tagName, const QString &replacement)
{
    QString taggedSection = ">>>"+tagName+">>>"+extractTaggedSection(text, tagName)+"<<<"+tagName+"<<<";
    text.replace(taggedSection, replacement);
}


QString replaceTag(QString text, const QString &tagName, const QString &replacement)
{
    text.replace("<<<"+tagName+">>>", replacement);
    return text;
}


QString replaceTags(QString text, const QStringList &tagNames, const QStringList &replacements)
{
    Q_ASSERT(tagNames.size() == replacements.size());
    for(int i=0; i<tagNames.size(); ++i)
    {
        text.replace("<<<"+tagNames[i]+">>>", replacements[i]);
    }
    return text;
}


//! @brief Find all files with given file extension recursively
//! @param [in] rootPath The root directory path for the search
//! @param [in] suffix The file extension to look for
//! @param [in,out] rFiles A list of filenames to which the found results will be appended
void findAllFilesInFolderAndSubFolders(const QString &rootPath, const QString &suffix, QStringList &rFiles)
{
    QDir dir(rootPath);
    QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        iterator.next();
        if (!iterator.fileInfo().isDir())
        {
            QString fileName = iterator.filePath();
            if (fileName.endsWith("."+suffix)) {
                rFiles.append(fileName);
            }
        }
    }
}


QStringList getHopsanCoreIncludePaths()
{
    QStringList includePaths;
    includePaths << "HopsanCore/include" <<
                    "componentLibraries/defaultLibrary";
    includePaths << "HopsanCore/dependencies/rapidxml" <<
                    "HopsanCore/dependencies/indexingcsvparser/include" <<
                    "HopsanCore/dependencies/libnumhop/include" <<
                    "HopsanCore/dependencies/sundials-extra/include" <<
                    "HopsanCore/dependencies/sundials/include";
    return includePaths;
}




//! @brief Replaces a pattern preserving pattern indentation, adding indentation to each line of text
//! @param[in] rPattern The pattern to replace
//! @param[in] rReplacement The replacement text
//! @param[in] rText The text to replace in
//! @returns True if pattern was found (and replaced), else False
bool replacePattern(const QString &rPattern, const QString &rReplacement, QString &rText)
{
    bool didReplace=false;
    while (true)
    {
        // First find pattern start in text
        int b = rText.indexOf(rPattern);
        if (b > -1)
        {
            // From beginning search backwards to count number of white spaces
            int nIndent = 0;
            QString indentString;
            --b;
            while (rText[b].isSpace() && (rText[b] != '\n'))
            {
                ++nIndent;
                indentString.append(" ");
                --b;
            }

            //Add indentation to each line in replacement text
            QString newrepl, repl=rReplacement;
            QTextStream ts(&repl);
            while (!ts.atEnd())
            {
                newrepl += indentString+ts.readLine()+"\n";
            }
            // If original replacement string lacks newline at end, the last newline should be removed
            if (!rReplacement.endsWith("\n"))
            {
                //! @todo will this work with crlf
                newrepl.chop(1);
            }

            // Now replace pattern
            rText.replace(indentString+rPattern, newrepl);

            didReplace =  true;
        }
        else
        {
            break;
        }
    }
    return didReplace;
}


int callProcess(const QString &name, const QStringList &args, const QString& workingDirectory, const int timeout_s, QString &rStdOut, QString &rStdErr)
{
    QProcess p;
    if(!workingDirectory.isEmpty())
    {
        p.setWorkingDirectory(workingDirectory);
    }
    p.start(name, args);
    p.waitForFinished(timeout_s*1000);

    rStdOut = p.readAllStandardOutput();
    rStdErr = p.readAllStandardError();

    if (p.exitStatus() == QProcess::ExitStatus::NormalExit)
    {
        return p.exitCode();
    }
    else
    {
        return -1;
    }
}

//! @brief Copies a file to a target and informs user of the outcome
//! @param[in] source Source file path
//! @param[in] target Full target file path
//! @param[out] rErrorMessage If copy fail, this contains an error message
//! @todo Copy symlinks as symlinks
//! @returns True if copy successful, otherwise false
bool copyFile(const QString &source, const QString &target, QString &rErrorMessage)
{
    QFile sourceFile;
    sourceFile.setFileName(source);
    // Remove target file if it already exists
    if(QFile::exists(target))
    {
        if (!QFile::remove(target))
        {
            rErrorMessage = "The file already exists, and it could not be overwritten: "+target;
            return false;
        }
    }
    // Create directory if it does not exist before copying
    QDir tgtDir = QFileInfo(target).dir();
    if (!tgtDir.exists())
    {
        tgtDir.mkpath(".");
    }
    // Now copy the files
    if(!sourceFile.copy(target))
    {
        rErrorMessage = "Unable to copy file: " +sourceFile.fileName() + " to " + target;
        return false;
    }
    QFile::setPermissions(target, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
    return true;
}

QStringList listHopsanCoreSourceFiles(const QString &hopsanInstallationPath)
{
    QStringList allFiles;
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/src", "cpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/dependencies", "cc", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/dependencies", "cpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/libnumhop/src", "cpp", allFiles);

    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/kinsol/kinsol_spils.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/kinsol/kinsol_ls.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/kinsol/kinsol_io.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/kinsol/kinsol_direct.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/kinsol/kinsol_bbdpre.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/kinsol/kinsol.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sunmatrix/dense/fsunmatrix_dense.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sunmatrix/dense/sunmatrix_dense.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/nvector/serial/fnvector_serial.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/nvector/serial/nvector_serial.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_futils.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_nvector_senswrapper.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_nonlinearsolver.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_version.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_iterative.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_band.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_direct.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_dense.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_nvector.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_linearsolver.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_matrix.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sundials/sundials_math.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sunmatrix/band/sunmatrix_band.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sunlinsol/band/sunlinsol_band.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/sundials/src/sunlinsol/dense/sunlinsol_dense.c";
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/indexingcsvparser/src/indexingcsvparser.cpp";

    QDir rootDir(hopsanInstallationPath);

    // Make path relative to root dir
    for(int i=0; i<allFiles.size(); ++i)
    {
        allFiles[i] = rootDir.relativeFilePath(allFiles[i]);
    }

    return allFiles;
}

QStringList listHopsanCoreIncludeFiles(const QString &hopsanInstallationPath)
{
    QStringList allFiles;
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/include", "h", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/include", "hpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/dependencies", "h", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/dependencies", "hpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/rapidxml", "hpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/libnumhop/include", "h", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/sundials/include", "h", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/sundials-extra/include", "h", allFiles);
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/indexingcsvparser/include/indexingcsvparser/indexingcsvparser.h";

    QDir rootDir(hopsanInstallationPath);

    // Make path relative to root dir
    for(int i=0; i<allFiles.size(); ++i)
    {
        allFiles[i] = rootDir.relativeFilePath(allFiles[i]);
    }

    return allFiles;
}

QStringList listInternalLibrarySourceFiles(const QString &hopsanInstallationPath)
{
    QStringList allFiles;

    // Default component library
    allFiles << hopsanInstallationPath+"/componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cpp";
    // Extra libraries (if they exist)
    const QString extraLibrary = hopsanInstallationPath+"/componentLibraries/extra-components.cpp";
    if (QFile::exists(extraLibrary)) {
        allFiles << extraLibrary;
    }

    // Make path relative to root dir
    QDir rootDir(hopsanInstallationPath);
    for(int i=0; i<allFiles.size(); ++i)
    {
        allFiles[i] = rootDir.relativeFilePath(allFiles[i]);
    }

    return allFiles;
}

bool matchOSString(const QString &os)
{
#if defined (_WIN64)
    return (os == hopsan::os_strings::win64) || (os == hopsan::os_strings::win);
#elif defined(_WIN32)
    return (os == hopsan::os_strings::win32) || (os == hopsan::os_strings::win);
#elif defined(__linux__)
    return (os == hopsan::os_strings::Linux);
#elif defined(__APPLE__)
    return (os == hopsan::os_strings::apple);
#else
    return false;
#endif
}

QString sharedLibrarySuffix(const BuildFlags::Platform platform)
{
    switch (platform) {
    case BuildFlags::Platform::Linux:
        return hopsan::sharedlibrary_suffixes::so;
    case BuildFlags::Platform::apple:
        return hopsan::sharedlibrary_suffixes::dylib;
    default:
        return hopsan::sharedlibrary_suffixes::dll;
    }
}

BuildFlags::Platform currentPlatform()
{
#if defined (_WIN64)
    return BuildFlags::Platform::win64;
#elif defined(_WIN32)
    return BuildFlags::Platform::win32;
#elif defined(__linux__)
    return BuildFlags::Platform::Linux;
#elif defined(__APPLE__)
    return BuildFlags::Platform::apple;
#else
    return BuildFlags::Platform::notset;
#endif

}

BuildFlags::Compiler defaultCompiler(const BuildFlags::Platform platform)
{
    switch (platform) {
    case BuildFlags::Platform::Linux:
        return BuildFlags::Compiler::GCC;
    case BuildFlags::Platform::apple:
        return BuildFlags::Compiler::Clang;
    default:
        return BuildFlags::Compiler::GCC;
    }
}

CompilerHandler::CompilerHandler(const CompilerHandler::Language language)
{
    setLanguage(language);
}

void CompilerHandler::addCompilerFlag(QString cflag, const Compiler compiler)
{
    if (mBuildFlags.empty() || (mBuildFlags.back().mCompiler != compiler)) {
        mBuildFlags.push_back(BuildFlags(compiler, QStringList(cflag), QStringList()));
    } else {
        mBuildFlags.back().mCompilerFlags.append(cflag);
    }
}

void CompilerHandler::addCompilerFlag(QString cflag, const Compilers compilers)
{
    for (auto compiler : compilers) {
        addCompilerFlag(cflag, compiler);
    }
}

void CompilerHandler::addLinkerFlag(QString lflag, const Compiler compiler)
{
    if (mBuildFlags.empty() || (mBuildFlags.back().mCompiler != compiler)) {
        mBuildFlags.push_back(BuildFlags(compiler, QStringList(), QStringList(lflag)));
    } else {
        mBuildFlags.back().mLinkerFlags.append(lflag);
    }

}

void CompilerHandler::addLinkerFlag(QString lflag, const Compilers compilers)
{
    for (auto compiler : compilers) {
        addLinkerFlag(lflag, compiler);
    }
}

void CompilerHandler::addBuildFlags(const QVector<BuildFlags> &flags)
{
    mBuildFlags += flags;
}

void CompilerHandler::addIncludePath(QString ipath, const Compilers compilers)
{
    addCompilerFlag(QString(R"(-I"%1")").arg(ipath), compilers);
}

void CompilerHandler::addLibraryPath(QString lpath, const Compilers compilers)
{
    for (auto compiler : compilers) {
        QString lflag;
        if (compiler == Compiler::MSVC) {
            lflag = QString(R"(-LIBPATH:"%1")").arg(lpath);
        } else {
            lflag = QString(R"(-L"%1")").arg(lpath);
        }
        addLinkerFlag(lflag, compiler);
    }
}

void CompilerHandler::addLinkLibrary(QString lib, const Compilers compilers)
{
    for (auto compiler : compilers) {
        QString lflag;
        if (compiler == Compiler::MSVC) {
            lflag = lib;
        } else {
            lflag = QString("-l%1").arg(lib);
        }
        addLinkerFlag(lflag, compiler);
    }
}


void CompilerHandler::addDefinition(QString macroname, QString value, const Compilers compilers)
{
    for (auto compiler : compilers) {
        QString cflag;
        if (value.isEmpty()) {
            cflag = QString("-D%1").arg(macroname);
        } else {
            cflag = QString("-D%1=%2").arg(macroname).arg(value);
        }
        addCompilerFlag(cflag, compiler);
    }
}

void CompilerHandler::addDefinition(QString macroname, const Compilers compilers)
{
    addDefinition(macroname, {}, compilers);
}

void CompilerHandler::setLanguage(const CompilerHandler::Language language)
{
    mLanguage = language;
}

QString CompilerHandler::compileCommand(const Compiler compiler)
{
    const QStringList cflags = compilerFlags(compiler);
    const QStringList lflags = linkerFlags(compiler);

    const QString compilerString = BuildFlags::compilerString(compiler, mLanguage);
    if (compiler == Compiler::MSVC) {
        return QString("%1 %2 %3 %4 -Fe%5").arg(compilerString)
                                           .arg(cflags.join(" "))
                                           .arg(mSourceFiles.join(" "))
                                           .arg(lflags.join(" "))
                                           .arg(mOutputFile);
    } else {
        return QString("%1 %2 %3 %4 -o %5").arg(compilerString)
                                           .arg(cflags.join(" "))
                                           .arg(mSourceFiles.join(" "))
                                           .arg(lflags.join(" "))
                                           .arg(mOutputFile);
    }
}

QStringList CompilerHandler::compilerFlags(const CompilerHandler::Compiler compiler) const
{
    QStringList cflags;
    for (const auto& bf : mBuildFlags) {
        if ( (bf.mCompiler == Compiler::Any) || (bf.mCompiler == compiler)  ) {
            cflags.append(bf.mCompilerFlags);
        }
    }
    return cflags;
}

QStringList CompilerHandler::linkerFlags(const CompilerHandler::Compiler compiler) const
{
    QStringList lflags;
    for (const auto& bf : mBuildFlags) {
        if ( (bf.mCompiler == Compiler::Any) || (bf.mCompiler == compiler)  ) {
            lflags.append(bf.mLinkerFlags);
        }
    }
    return lflags;
}

void CompilerHandler::setOutputFile(QString outputFile, const OutputType outputType)
{
    mOutputFile = outputFile;
    mOutputType = outputType;
}

void CompilerHandler::setSharedLibraryOutputFile(QString outputLibraryFileName, const BuildType buildType)
{
    const QString suffix = sharedLibrarySuffix(currentPlatform());
    const QString extension = "."+suffix;
    if (!outputLibraryFileName.endsWith(extension)) {
        outputLibraryFileName.append(extension);
    }
    addLinkerFlag("-shared", {Compiler::GCC, Compiler::Clang});
    if (buildType == BuildType::Release) {
        addLinkerFlag("-MD", Compiler::MSVC);
        addLinkerFlag("-LD", Compiler::MSVC);
    } else {
        addCompilerFlag("-g", {Compiler::GCC, Compiler::Clang});
        addLinkerFlag("-MDd", Compiler::MSVC);
        addLinkerFlag("-LDd", Compiler::MSVC);
    }

    setOutputFile(outputLibraryFileName, OutputType::SharedLibrary);
}

void CompilerHandler::setSourceFiles(const QStringList &sourceFiles)
{
    mSourceFiles = sourceFiles;
}

QString CompilerHandler::outputFile() const
{
    return mOutputFile;
}

QStringList CompilerHandler::sourceFiles() const
{
    return mSourceFiles;
}

void setRW_RW_RW_FilePermissions(const QString &filePath) {
#if defined(_WIN32)
    QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser | QFile::ReadOther | QFile::WriteOther);
#else
    QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther | QFile::WriteOther);
#endif
}

void setRW_RW_RW_FilePermissions(const QStringList &filePaths)
{
    for (const QString& filePath : filePaths) {
        setRW_RW_RW_FilePermissions(filePath);
    }
}

void setRWXRWXRW_FilePermissions(const QString &filePath)
{
#if defined(_WIN32)
    QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                          QFile::ReadOther | QFile::WriteOther | QFile::ExeOther |
                          QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
#else
    QFile::setPermissions(filePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                          QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                          QFile::ReadOther | QFile::ExeOther);
#endif
}
