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

#ifndef HOPSANFMIGENERAETOR_H
#define HOPSANFMIGENERAETOR_H

// Hopsan includes
#include "HopsanGeneratorBase.h"

// FMILibrary includes and forward declaration
#include "FMI/fmi_import_context.h"
struct jm_callbacks;

class HopsanFMIGenerator : public HopsanGeneratorBase
{
public:
    HopsanFMIGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath="");
    bool generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, const QStringList& externalLibraries, int version=2, bool x64=true);

private:

    //Utility functions
    bool readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType);

    bool generateModelDescriptionXmlFile(hopsan::ComponentSystem *pSystem, QString savePath, QString guid, int version, size_t &nReals, size_t &nInputs, size_t &nOutputs);
    void replaceNameSpace(const QString &savePath, int version) const;
    bool compileAndLinkFMU(const QString &fmuBuildPath, const QString &fmuStagePath, const QString &modelName, const int version, bool x64) const;
    bool compressFiles(const QString &fmuStagePath, const QString &modelName) const;

    QStringList mExtraSourceFiles;
    QStringList mIncludePaths;
    QStringList mLinkPaths;
    QStringList mLinkLibraries;
};

#endif // HOPSANFMIGENERAETOR_H
