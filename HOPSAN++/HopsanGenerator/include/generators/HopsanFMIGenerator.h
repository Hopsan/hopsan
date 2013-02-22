#ifndef HOPSANFMIGENERAETOR_H
#define HOPSANFMIGENERAETOR_H

#include "HopsanGenerator.h"

class HopsanFMIGenerator : public HopsanGenerator
{
public:
    HopsanFMIGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void generateFromFmu(QString code);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem);

    bool readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType);
};

#endif // HOPSANFMIGENERAETOR_H
