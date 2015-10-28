/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef HOPSANFMIGENERAETOR_H
#define HOPSANFMIGENERAETOR_H

//Hopsan includes
#include "HopsanGenerator.h"

//FMILibrary includes
#include "FMI/fmi_import_context.h"
#include <FMI1/fmi1_import.h>
#include <FMI2/fmi2_import.h>
#include <JM/jm_portability.h>

class HopsanFMIGenerator : public HopsanGenerator
{
public:
    HopsanFMIGenerator(QString coreIncludePath, QString binPath, QString gccPath, bool showDialog=false);
    bool generateFromFmu(const QString &rFmuPath, QString targetPath, QString &rTypeName, QString &rHppPath);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, int version=2, bool x64=true);

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

    void generateModelDescriptionXmlFile(hopsan::ComponentSystem *pSystem, QString savePath, QString guid, int version, size_t &nReals, size_t &nInputs, size_t &nOutputs);
    void generateModelFile(const hopsan::ComponentSystem *pSystem, const QString &savePath) const;
    void replaceNameSpace(const QString &savePath) const;
    bool compileAndLinkFMU(const QString &savePath, const QString &modelName, int version) const;
    void sortFiles(const QString &savePath, const QString &modelName, bool x64) const;
    void compressFiles(const QString &savePath, const QString &modelName) const;
    //bool replaceFMIVariablesWithTLMPort(QStringList &rPortVarNames, QStringList &rPortVarVars, QStringList &rPortVarRefs, QList<size_t> &rPortVarDataIds,
    //                                    QStringList &rActualNames, QStringList &rActualVars, QStringList &rActualRefs,
    //                                    const QStringList &rTags, const QList<size_t> &rDataIds, const QString &rPortName, const QDomElement portElement);

};

#endif // HOPSANFMIGENERAETOR_H
