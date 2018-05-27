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

// FMILibrary includes
#include "FMI/fmi_import_context.h"
#include <FMI1/fmi1_import.h>
#include <FMI2/fmi2_import.h>
#include <JM/jm_portability.h>

void hopsanLogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message);

class HopsanFMIGenerator : public HopsanGeneratorBase
{
public:
    HopsanFMIGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath="");
    bool generateFromFmu(const QString &rFmuPath, QString targetPath, QString &rTypeName, QString &rHppPath);
    bool generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, int version=2, bool x64=true);

private:
    bool generateFromFmu1(const QString &rFmuPath, const QString &targetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context);
    bool generateFromFmu2(const QString &rFmuPath, const QString &rTargetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context);

    //Utility functions
    void getInterfaceInfo(QString typeName, QString compName,
                          QStringList &inVars, QStringList &inComps, QStringList &inPorts, QList<int> &inDatatypes,
                          QStringList &outVars, QStringList &outComps, QStringList &outPorts, QList<int> &outDatatypes,
                          QList<QStringList> &tlmPorts);

    bool readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType);

    bool generateModelDescriptionXmlFile(hopsan::ComponentSystem *pSystem, QString savePath, QString guid, int version, size_t &nReals, size_t &nInputs, size_t &nOutputs);
    bool generateModelFile(const hopsan::ComponentSystem *pSystem, const QString &savePath) const;
    void replaceNameSpace(const QString &savePath) const;
    bool compileAndLinkFMU(const QString &savePath, const QString &modelName, int version) const;
    void sortFiles(const QString &savePath, const QString &modelName, bool x64) const;
    bool compressFiles(const QString &savePath, const QString &modelName) const;
};

#endif // HOPSANFMIGENERAETOR_H
