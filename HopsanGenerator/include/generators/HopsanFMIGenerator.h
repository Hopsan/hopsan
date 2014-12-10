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
    HopsanFMIGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    bool generateFromFmu(QString &rPath, QString &rTargetPath, QString &rTypeName, QString &rHppPath);
    void generateFromFmu1(QString code, QString targetPath);
    bool generateFromFmu2(QString &rPath, QString &RargetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, bool me=false);

    bool readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType);
    void getInterfaceInfo(QString typeName, QString compName,
                          QStringList &inVars, QStringList &inComps, QStringList &inPorts, QList<int> &inDatatypes,
                          QStringList &outVars, QStringList &outComps, QStringList &outPorts, QList<int> &outDatatypes,
                          QList<QStringList> &tlmPorts);
};

#endif // HOPSANFMIGENERAETOR_H
