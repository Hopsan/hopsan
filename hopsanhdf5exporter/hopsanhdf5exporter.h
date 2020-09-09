#ifndef HOPSANHDF5EXPORTER_H
#define HOPSANHDF5EXPORTER_H

#include "HopsanEssentials.h"

class HopsanHDF5Exporter
{

public:
    HopsanHDF5Exporter(const hopsan::HString &rFilePath, const hopsan::HString &rModelFileName, const hopsan::HString &rToolName);
    void addVariable(hopsan::HString &rSystemHierarchy, const hopsan::HString &rComponentName, const hopsan::HString &rPortName, const hopsan::HString &rVariableName, const hopsan::HString &rAliasName, const hopsan::HString &rUnit, const hopsan::HString &rQuantity, hopsan::HVector<double> &rDataVector);
    bool writeToFile();
    const hopsan::HString &getLastError();
private:
    hopsan::HString mLastError;
    hopsan::HString mFilePath, mModelFileName, mToolName;
    hopsan::HVector<hopsan::HString> mSystemHierarchies;
    hopsan::HVector<hopsan::HString> mComponentNames, mPortNames, mVariableNames, mAliasNames, mUnits, mQuantities;
    hopsan::HVector<hopsan::HVector<double> > mDataVectors;
};

#endif // HOPSANHDF5EXPORTER_H
