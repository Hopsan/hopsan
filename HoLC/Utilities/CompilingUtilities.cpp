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

#include "CompilingUtilities.h"

#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>

QStringList compileComponentLibrary(const QString &compilerPath, const QString &path, const QString &target, const QStringList &sourceFiles, const QStringList &libs, const QStringList &includeDirs, bool &success)
{
    QStringList returnLog;
    returnLog.append("Compiling!");

    returnLog.append("Source files: ");
    QString c;
    Q_FOREACH(const QString &sourceFile, sourceFiles)
    {
        returnLog.append("  "+sourceFile);
        c.append(sourceFile+" ");
    }
    c.chop(1);

    returnLog.append("Include directories: ");
    QString i;
    Q_FOREACH(const QString &includeDir, includeDirs)
    {
        returnLog.append("  "+includeDir);
        i.append("-I\""+includeDir+"\" ");
    }
    i.chop(1);

    returnLog.append("Libraries: ");
    QString l;
    Q_FOREACH(const QString &lib, libs)
    {
        returnLog.append("  "+lib);
        QString baseName = QFileInfo(lib).baseName();
#ifdef __linux__
        baseName.remove(0,3);
#endif
        l.append("-L\""+QFileInfo(lib).absolutePath()+"\" -l"+baseName+" ");
    }
    l.chop(1);

    success = compile(compilerPath, path, target, c, i, l, "-Dhopsan=hopsan -fPIC -w -Wl,--rpath -Wl,\""+path+"\" -shared ", returnLog);

    return returnLog;
}


//! @brief Calls GCC or MinGW compiler with specified parameters
//! @param path Absolute path where compiler shall be run
//! @param o Objective file name (without file extension)
//! @param c List with source files, example: "file1.cpp file2.cc"
//! @param i Include command, example: "-Ipath1 -Ipath2"
//! @param l Link command, example: "-Lpath1 -lfile1 -lfile2"
//! @param flags Compiler flags
//! @param output Reference to string where output messages are stored
bool compile(const QString &compilerPath, const QString &path, const QString &o, const QString &c, const QString &i, const QString &l, const QString &flags, QStringList &output)
{
    //Create compilation script file
#ifdef _WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(path + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output.append("Could not open compile.bat for writing.");
        return false;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "call \""+compilerPath+"\" "+flags;
    clBatchStream << c+" -o "+o+".dll "+i+" "+l+"\n";
    clBatchFile.close();
#endif

    //Call compilation script file
#ifdef _WIN32
    QProcess gccProcess;
    gccProcess.setWorkingDirectory(path);

    //Add gcc bin directory to PATH variable in cmd process
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), "PATH=\\1;"+QFileInfo(compilerPath).absolutePath());
    gccProcess.setEnvironment(env);

    gccProcess.start("cmd.exe", QStringList() << "/c" << "compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAllStandardOutput();
    QByteArray gccError = gccProcess.readAllStandardError();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        output.append(gccResultList.at(i));
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
        output.append(gccErrorList.at(i));
        //output = output.remove(output.size()-1, 1);
    }
#elif defined __linux__
    QString gccCommand = "cd \""+path+"\" && "+compilerPath+" "+flags+" ";
    gccCommand.append(c+" -fpermissive -o "+o+".so "+i+" "+l);
    //qDebug() << "Command = " << gccCommand;
    gccCommand +=" 2>&1";
    FILE *fp;
    char line[130];
    output.append("Compiler command: \""+gccCommand+"\"\n");
    qDebug() << "Compiler command: \"" << gccCommand << "\"";
    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        output.append("Could not execute '" + gccCommand + "'! err=%d\n");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            output.append(QString::fromUtf8(line));
        }
    }
#endif

    QDir targetDir(path);
#ifdef _WIN32

    if(!targetDir.exists(o + ".dll") || !gccErrorList.isEmpty())
    {
        output.append("Compilation failed.");
        return false;
    }
#elif defined __linux__
    if(!targetDir.exists(o + ".so"))
    {
        qDebug() << targetDir.absolutePath();
        qDebug() << o + ".so";
        output.append("Compilation failed.");
        return false;
    }
#endif

    output.append("Compilation successful.");
    return true;
}

