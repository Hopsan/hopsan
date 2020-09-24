#include "hopsanhdf5exporter.h"
#include "H5Cpp.h"

#include <ctime>
#include <set>

using namespace hopsan;

//! @brief Help function to append string attribute to HDF5 object
void appendH5Attribute(H5::H5Object &rObject, const H5std_string &attrName, const H5std_string &attrValue)
{
    H5::DataSpace attr_dataspace = H5::DataSpace( H5S_SCALAR );
    H5::StrType attr_strtype( H5::PredType::C_S1, std::max(attrValue.size(), size_t(1)) );
    H5::Attribute attribute = rObject.createAttribute( attrName, attr_strtype, attr_dataspace );
    attribute.write( attr_strtype, attrValue );
}

HopsanHDF5Exporter::HopsanHDF5Exporter(const hopsan::HString &rFilePath, const hopsan::HString &rModelFileName, const hopsan::HString &rToolName) :
    mFilePath(rFilePath),
    mModelFileName(rModelFileName),
    mToolName(rToolName) {}

void HopsanHDF5Exporter::addVariable(hopsan::HString &rSystemHierarchy, const hopsan::HString &rComponentName, const hopsan::HString &rPortName, const hopsan::HString &rVariableName, const hopsan::HString &rAliasName, const hopsan::HString &rUnit, const hopsan::HString &rQuantity, hopsan::HVector<double> &rDataVector)
{
    mSystemHierarchies.append(rSystemHierarchy);
    mComponentNames.append(rComponentName);
    mPortNames.append(rPortName);
    mVariableNames.append(rVariableName);
    mAliasNames.append(rAliasName);
    mUnits.append(rUnit);
    mQuantities.append(rQuantity);
    mDataVectors.append(rDataVector);
}

bool HopsanHDF5Exporter::writeToFile()
{
    try {
        // turn off auto printing of thrown exceptions so that they can be handled below
        H5::Exception::dontPrint();

        // Create and open a file
        H5::H5File file(mFilePath.c_str(), H5F_ACC_TRUNC);

        //Generate date and time string
        time_t rawtime;
        struct tm * timeinfo;
        char timestr[100];
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        std::strftime(timestr,sizeof(timestr),"%a %b %d %H:%M:%S %Y",timeinfo);
        HString dateTime = HString(timestr);

        H5::Group root = file.openGroup("/");
        appendH5Attribute(root, "date", dateTime.c_str());
        appendH5Attribute(root, "model", mModelFileName.c_str());
        appendH5Attribute(root, "tool", mToolName.c_str());

        // Build directory/group hierarchy
        // We need this to avoid massive exception casting when creating directories as
        // group names will be repeated, also we need to create one group depth at a time
        // The set will sort the group names as unique values in the correct order
        std::set<HString> uniqueGroupPaths;
        for(size_t i=0; i<mSystemHierarchies.size(); ++i) {

            std::vector<HString> sysnames;
            if (!mSystemHierarchies[i].empty()) {
                //Split system hierarchy string to a vector of sub strings
                //! @todo tmp would not be needed if HVector has iterators implemented
                auto tmp = mSystemHierarchies[i].split('.');
                sysnames = std::vector<HString>(tmp.data(), tmp.data()+tmp.size());
            }

            const HString& componentName = mComponentNames[i];
            const HString& portName = mPortNames[i];

            HString fullGroupPath = "/results/";
            uniqueGroupPaths.insert(fullGroupPath);
            for (const auto &sysname : sysnames) {
                fullGroupPath.append(sysname);
                uniqueGroupPaths.insert(fullGroupPath);
                fullGroupPath.append("/");
            }
            if (!componentName.empty()) {
                fullGroupPath.append(componentName);
                uniqueGroupPaths.insert(fullGroupPath);
                if (!portName.empty()) {
                    fullGroupPath.append("/").append(portName);
                    uniqueGroupPaths.insert(fullGroupPath);
                }
            }
        }

        // Create all Groups
        for (const auto &groupPath : uniqueGroupPaths) {
            file.createGroup(groupPath.c_str());
        }

        HVector<HString> errors;
        for(size_t i=0; i<mSystemHierarchies.size(); ++i) {
            // Create a dataspace for a vector of data
            hsize_t dims[1];
            dims[0] = size_t(mDataVectors[i].size());
            H5::DataSpace dataspace(1, dims);

            HString systemNames = mSystemHierarchies[i];
            systemNames.replace('.', '/');
            if (!systemNames.empty()) {
                systemNames.append('/');
            }

            const HString& componentName = mComponentNames[i];
            const HString& portName = mPortNames[i];
            const HString& variableName = mVariableNames[i];

            std::vector<HString> hdf5NamesForThisVariable;

            HString hdf5FullVariableName = "/results/" + systemNames;
            if (!componentName.empty()) {
                hdf5FullVariableName.append(componentName).append('/');
                if (!portName.empty()) {
                    hdf5FullVariableName.append(portName).append('/');
                }
            }
            // Append last part of the name, the variable name
            hdf5FullVariableName.append(variableName);

            hdf5NamesForThisVariable.push_back(hdf5FullVariableName);

            // If variable has an alias then  create an additional hdf5 variable with the alias name
            //! @todo should alias be model global ?
            //! @todo investigate if links can be be used instead of duplicating data
            if (!mAliasNames[i].empty()) {
                HString hdf5FullVariableAliasName = "/results/"+systemNames+mAliasNames[i];
                hdf5NamesForThisVariable.push_back(hdf5FullVariableAliasName);
            }

            for (const auto &hdf5Name : hdf5NamesForThisVariable) {
                // Create the data set, we hope that the code above has created the group already
                // if not then we will fail here and exit with an exception
                // Exception will also occure if name is already taken
                try {
                    H5::DataSet dataset = file.createDataSet(hdf5Name.c_str(), H5::PredType::NATIVE_DOUBLE, dataspace);

                    // Write the data
                    dataset.write(mDataVectors[i].data(), H5::PredType::NATIVE_DOUBLE);

                    // Add meta data attributes
                    appendH5Attribute(dataset, "Unit", mUnits[i].c_str());
                    appendH5Attribute(dataset, "Quantity", mQuantities[i].c_str());
                }
                catch(H5::Exception &e) {
                    errors.append(HString(e.getCDetailMsg())+" in "+HString(e.getCFuncName()) + " for dataset " + hdf5Name);
                    // Log this error but continue to the next variable
                }
            }
        }

        file.close();

        if (errors.size() > 0) {
            mLastError = errors[0];
            for (size_t i=1; i<errors.size(); ++i) {
                mLastError += "; " + errors[i];
            }
            return false;
        }
    }
    // Catch any other H5 exceptions
    catch(H5::Exception &e) {
        mLastError = HString(e.getCDetailMsg())+" in "+HString(e.getCFuncName());
        return false;
    }

    return true;
}

const hopsan::HString &HopsanHDF5Exporter::getLastError()
{
    return mLastError;
}


