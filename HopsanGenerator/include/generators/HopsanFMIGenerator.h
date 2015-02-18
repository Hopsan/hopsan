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
    bool generateFromFmu(QString &rPath, QString &rTargetPath, QString &rTypeName, QString &rHppPath);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, int version=2, bool x64=true);

private:
    bool generateFromFmu1(QString &rPath, QString &targetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context);
    bool generateFromFmu2(QString &rPath, QString &targetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context);
    //void generateToFmu1(QString savePath, hopsan::ComponentSystem *pSystem, bool me, bool x64);
    //void generateToFmu2(QString savePath, hopsan::ComponentSystem *pSystem, bool me, bool x64);

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
    void compileAndLinkFMU(const QString &savePath, const QString &modelName, int version) const;
    void sortFiles(const QString &savePath, const QString &modelName, bool x64) const;
    void compressFiles(const QString &savePath, const QString &modelName) const;

};

#endif // HOPSANFMIGENERAETOR_H
