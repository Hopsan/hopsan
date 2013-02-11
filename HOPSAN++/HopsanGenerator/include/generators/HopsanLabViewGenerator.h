#ifndef HOPSANLABVIEWGENERATOR_H
#define HOPSANLABVIEWGENERATOR_H

#include "HopsanGenerator.h"

class HopsanLabViewGenerator : public HopsanGenerator
{
public:
    HopsanLabViewGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void generateToLabViewSIT(QString savePath, hopsan::ComponentSystem *pSystem);
};

#endif // HOPSANLABVIEWGENERATOR_H
