#ifndef HOPSANFMIGENERAETOR_H
#define HOPSANFMIGENERAETOR_H

#include "HopsanGenerator.h"

class HopsanFMIGenerator : public HopsanGenerator
{
public:
    HopsanFMIGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void generateFromFmu(QString code);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem);
    void generateToFmuOld(QString savePath, hopsan::ComponentSystem *pSystem);
};

#endif // HOPSANFMIGENERAETOR_H
