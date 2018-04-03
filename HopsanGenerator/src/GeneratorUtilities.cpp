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

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "GeneratorUtilities.h"
#include "generators/HopsanGenerator.h"
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
    QDir dir;
    dir.setPath(path);
    Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            removeDir(info.absoluteFilePath());
        }
        else
        {
            if(QFile::remove(info.absoluteFilePath()))
            {
                qDebug() << "Successfully removed " << info.absoluteFilePath();
            }
            else
            {
                qDebug() << "Failed to remove " << info.absoluteFilePath();
            }
        }
    }
    return dir.rmdir(path);     //If removing files fails, this will fail, so we only need to check this
}


//! @brief Copy a directory with contents
//! @param[in] fromPath The absolute path to the directory to copy
//! @param[in] toPath The absolute path to the destination (including destination dir name)
//! @param[out] rErrorMessage Error message if copy fail
//! @returns True if success else False
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
bool copyDir(const QString &fromPath, QString toPath, QString &rErrorMessage)
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
    foreach(QFileInfo info, fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            if(!copyDir(info.absoluteFilePath(), toPath+"/"+info.fileName(), rErrorMessage))
            {
                return false;
            }
        }
        else
        {
            if(!copyFile(info.absoluteFilePath(), toPath+"/"+info.fileName(), rErrorMessage))
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



bool compileComponentLibrary(QString path, HopsanGenerator *pGenerator, QString extraCFlags, QString extraLFlags)
{
    pGenerator->printMessage("Writing compilation script...");

    QStringList ccFiles;
    QString libFile,dbg_ext,libRootDir, cflags, lflags;
    if(QFileInfo(path).isFile())
    {
        QFile xmlFile(path);
        xmlFile.open(QFile::ReadOnly);

        QDomDocument doc;
        QDomElement rootElement = loadXMLDomDocument(xmlFile, doc, "hopsancomponentlibrary");

        // Load lib info
        QDomElement libElement = rootElement.firstChildElement("lib");

        dbg_ext = libElement.attribute("debug_ext");
        libFile = QString(LIBPREFIX)+libElement.text();

        QDomElement bfElement = rootElement.firstChildElement("buildflags").firstChildElement();
        while (!bfElement.isNull())
        {
            QString os = bfElement.attribute("os");
            if (bfElement.tagName() == "cflags")
            {
                // Only add flag if os attribute match current os
                if (os.isEmpty() || matchOSString(os))
                {
                    cflags.append(" "+bfElement.text());
                }
            }
            else if (bfElement.tagName() == "lflags")
            {
                // Only add flag if os attribute match current os
                if (os.isEmpty() || matchOSString(os))
                {
                    lflags.append(" "+bfElement.text());
                }
            }
            //! @todo handle other elements such as includepath libpath libflag defineflag and such
            bfElement = bfElement.nextSiblingElement();
        }

        QDomElement sourceElement = rootElement.firstChildElement("source");
        while (!sourceElement.isNull())
        {
            ccFiles.append(sourceElement.text());
            sourceElement = sourceElement.nextSiblingElement("source");
        }

        // If no cc files were specified then add at least the lib file
        if (ccFiles.isEmpty())
        {
            ccFiles.append(libElement.text()+".cpp");
        }

        xmlFile.close();
        libRootDir = QFileInfo(path).canonicalPath();
    }
    else if (QFileInfo(path).isDir())
    {
        libFile = QString(LIBPREFIX)+QDir(path).dirName();
        ccFiles = QDir(path).entryList(QStringList() << "*.cpp");
        libRootDir = path;
    }
    else
    {
        pGenerator->printErrorMessage(path+" does not exist, is not component library file or is not a directory");
        return false;
    }

    QString c;
    Q_FOREACH(const QString &file, ccFiles)
        c.append(file+" ");
    c.chop(1);

    QString hopsanBinDir = pGenerator->getBinPath();
    QString iflags = QString("-I\"%1\"").arg(pGenerator->getCoreIncludePath());
    lflags += QString(" -L\"%1\" -l%2").arg(hopsanBinDir).arg("hopsancore" TO_STR(DEBUG_EXT))+" "+extraLFlags;

    //! @todo setting rpath here is strange, as it will hardcode given path inte dll (so if you move it it wont work) /Peter
    cflags += QString(" -Dhopsan=hopsan -fPIC -w -Wl,--rpath,\"%1\" -shared ").arg(libRootDir);
    cflags += extraCFlags+" ";

    // Modify if debug
#ifdef DEBUGCOMPILING
    libFile+=dbg_ext;
    cflags.prepend("-g -DDEBUGCOMPILING ");
#else
    cflags.prepend("-DRELEASECOMPILING ");
#endif

    pGenerator->printMessage("\nCalling compiler utility:");
    pGenerator->printMessage("Path:     "+libRootDir);
    pGenerator->printMessage("Output:   "+libFile);
    pGenerator->printMessage("Sources:  "+c);
    pGenerator->printMessage("Cflags:   "+cflags);
    pGenerator->printMessage("Includes: "+iflags);
    pGenerator->printMessage("Lflags:   "+lflags+"\n");

    pGenerator->printMessage("Compiling please wait!");
    QString output;
    QString gccPath = pGenerator->getGccPath();
    bool success = compile(libRootDir, gccPath, libFile, c, iflags, cflags, lflags, output);
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
    o = o+TO_STR(DLL_EXT);

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
#elif __linux__
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
#ifdef _WIN32
    QProcess gccProcess;
    gccProcess.setWorkingDirectory(wdPath);
    gccProcess.start("cmd.exe", QStringList() << "/c" << "compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAllStandardOutput();
    QByteArray gccError = gccProcess.readAllStandardError();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        output = gccResultList.at(i);
        //output = output.remove(output.size()-1, 1);
    }
    QList<QByteArray> gccErrorList = gccError.split('\n');
    for(int i=0; i<gccErrorList.size(); ++i)
    {
        if(gccErrorList.at(i).trimmed() == "")
        {
            gccErrorList.removeAt(i);
            --i;
            continue;
        }
        output = output+ gccErrorList.at(i);
        //output = output.remove(output.size()-1, 1);
    }
#elif __linux__
    QString stdOut,stdErr;
    callProcess("/bin/sh", QStringList() << "compile.sh", wdPath, 60, stdOut, stdErr);
    output.append(stdOut);
    output.append("\n");
    output.append(stdErr);
    output.append("\n");
#endif

    QDir targetDir(wdPath);
#ifdef _WIN32

    if(!targetDir.exists(o) || !gccErrorList.isEmpty())
    {
        output.append("Compilation failed.");
        return false;
    }
#elif __linux__
    if(!targetDir.exists(o))
    {
        output.append("Compilation failed.");
        return false;
    }
#endif

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
                    "HopsanCore/dependencies/IndexingCSVParser" <<
                    "HopsanCore/dependencies/libNumHop/include";
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


int callProcess(const QString &name, const QStringList &args, const QString& workingDirectory, const int timeout, QString &rStdOut, QString &rStdErr)
{
    QProcess p;
    if(!workingDirectory.isEmpty())
    {
        p.setWorkingDirectory(workingDirectory);
    }
    p.start(name, args);
    p.waitForFinished(timeout*10000);

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
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/Dependencies", "cc", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/Dependencies", "cpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/libNumHop/src", "cpp", allFiles);
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/IndexingCSVParser/IndexingCSVParser.cpp";

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
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/Dependencies", "h", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/Dependencies", "hpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/rapidxml", "hpp", allFiles);
    findAllFilesInFolderAndSubFolders(hopsanInstallationPath+"/HopsanCore/dependencies/libNumHop/include", "h", allFiles);
    allFiles << hopsanInstallationPath+"/HopsanCore/dependencies/IndexingCSVParser/IndexingCSVParser.h" <<
                hopsanInstallationPath+"/HopsanCore/dependencies/IndexingCSVParser/IndexingCSVParserImpl.hpp";

    QDir rootDir(hopsanInstallationPath);

    // Make path relative to root dir
    for(int i=0; i<allFiles.size(); ++i)
    {
        allFiles[i] = rootDir.relativeFilePath(allFiles[i]);
    }

    return allFiles;
}

QStringList listDefaultLibrarySourceFiles(const QString &hopsanInstallationPath)
{
    QStringList allFiles;
    //! @todo handle external internal library
    // Now only internal
    allFiles << hopsanInstallationPath+"/componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cpp";

    QDir rootDir(hopsanInstallationPath);

    // Make path relative to root dir
    for(int i=0; i<allFiles.size(); ++i)
    {
        allFiles[i] = rootDir.relativeFilePath(allFiles[i]);
    }

    return allFiles;
}

bool matchOSString(QString os)
{
    if (os == "win32")
    {
#ifdef _WIN32
      return true;
#else
      return false;
#endif
    }
    else if (os == "win64")
    {
#ifdef _WIN64
      return true;
#else
      return false;
#endif
    }
    else if (os == "linux")
    {
#ifdef __linux__
      return true;
#else
      return false;
#endif
    }
    else if (os == "apple")
    {
#ifdef __APPLE__
      return true;
#else
      return false;
#endif
    }
    else
    {
        return false;
    }
}

