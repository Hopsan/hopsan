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

#ifndef HOPSANEXEGENERATOR_H
#define HOPSANEXEGENERATOR_H

// Hopsan includes
#include "HopsanGeneratorBase.h"


class HopsanExeGenerator : public HopsanGeneratorBase
{
public:
    HopsanExeGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath="");
    bool generateToExe(QString savePath, hopsan::ComponentSystem *pSystem, const QStringList &externalLibraries, bool x64);

private:
    bool compileAndLinkExe(const QString &buildPath, const QString &modelName, bool x64) const;

    QStringList mExtraSourceFiles;
    QStringList mIncludePaths;
    QStringList mLinkPaths;
    QStringList mLinkLibraries;
};

#endif // HOPSANEXEGENERATOR_H
