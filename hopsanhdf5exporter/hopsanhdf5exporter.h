#ifndef HOPSANHDF5EXPORTER_H
#define HOPSANHDF5EXPORTER_H

#include "HopsanEssentials.h"

using namespace hopsan;

class HopsanHDF5Exporter
{

public:
    HopsanHDF5Exporter(const hopsan::HString &rFilePath, const hopsan::HString &rModelFileName, const hopsan::HString &rToolName);
    void addVariable(hopsan::HString &rSystemHierarchy, const hopsan::HString &rComponentName, const hopsan::HString &rPortName, const hopsan::HString &rVariableName, const hopsan::HString &rAliasName, const hopsan::HString &rUnit, const hopsan::HString &rQuantity, hopsan::HVector<double> &rDataVector);
    bool writeToFile();
    const HString &getLastError();
private:
    HString mLastError;
    HString mFilePath, mModelFileName, mToolName;
    HVector<HString> mSystemHierarchies;
    HVector<HString> mComponentNames, mPortNames, mVariableNames, mAliasNames, mUnits, mQuantities;
    HVector<HVector<double> > mDataVectors;
};

#endif // HOPSANHDF5EXPORTER_H
