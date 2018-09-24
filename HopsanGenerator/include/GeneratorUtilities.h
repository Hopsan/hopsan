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
//! @file   ComponentGeneratorUtilities.h
//!
//! @brief Contains component generation utility functions
//!
//$Id$


#ifndef GENERATORUTILITIES_H
#define GENERATORUTILITIES_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDomElement>

class HopsanGeneratorBase;

namespace hopsan {
namespace os_strings {
    constexpr auto win = "win";
    constexpr auto win32 = "win32";
    constexpr auto win64 = "win64";
    constexpr auto apple = "apple";
    constexpr auto Linux = "linux";
}
}

bool matchOSString(const QString& os);

QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);

bool removeDir(QString path);
bool copyDir(const QString &fromPath, QString toPath, QString &rErrorMessage);
bool copyFile(const QString &source, const QString &target, QString &rErrorMessage);

bool compileComponentLibrary(QString path, HopsanGeneratorBase *pGenerator, QString extraCFlags="", QString extraLFlags="");
bool compile(QString wdPath, QString gccPath, QString o, QString srcFiles, QString inclPaths, QString cflags, QString lflags, QString &output);

int callProcess(const QString &name, const QStringList &args, const QString &workingDirectory, const int timeout_s, QString &rStdOut, QString &rStdErr);

QString toValidHopsanVarName(const QString &name);
QString toValidLabViewVarName(const QString &name);

QString extractTaggedSection(const QString &text, const QString &tagName);
void replaceTaggedSection(QString &text, const QString &tagName, const QString &replacement);
QString replaceTag(QString text, const QString &tagName, const QString &replacement);
QString replaceTags(QString text, const QStringList &tagNames, const QStringList &replacements);
bool replacePattern(const QString &rPattern, const QString &rReplacement, QString &rText);

void findAllFilesInFolderAndSubFolders(const QString &rootPath, const QString &suffix, QStringList &rFiles);
QStringList listHopsanCoreSourceFiles(const QString &hopsanInstallationPath);
QStringList listDefaultLibrarySourceFiles(const QString &hopsanInstallationPath);
QStringList listHopsanCoreIncludeFiles(const QString &hopsanInstallationPath);
QStringList getHopsanCoreIncludePaths();

//! @brief This utility class wraps a QTextStream and have stream operators to write whole lines. You do not need to add the newline char yourself.
class QTextLineStream
{
public:
    QTextLineStream(QTextStream &rTextStream)
    {
        mpQTextSream = &rTextStream;
    }
    friend QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input);
    friend QTextLineStream& operator <<(QTextLineStream &rLineStream, const QString &input);

private:
    QTextStream* mpQTextSream;
};

#endif // GENERATORUTILITIES_H
