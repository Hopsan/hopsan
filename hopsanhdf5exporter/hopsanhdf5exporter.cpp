#include "hopsanhdf5exporter.h"
#include "H5Cpp.h"

#include <ctime>

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
    HString error;
    const H5std_string filePath = mFilePath.c_str();
    try {
        // turn off auto printing of thrown exceptions so that they can be handled below
        H5::Exception::dontPrint();

        // Create and open a file
        H5::H5File file(filePath, H5F_ACC_TRUNC);

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
        // The map will sort the group names as unique values in the correct order
        std::map<HString, char> varhier;
        for(size_t i=0; i<mSystemHierarchies.size(); ++i) {

            //Split system hierarchy string to a vector of sub strings
            std::vector<HString> sysnames;
            HString tempHierarchy = mSystemHierarchies[i];
            while(!tempHierarchy.empty()) {
                sysnames.push_back(tempHierarchy.substr(0, tempHierarchy.find(".")));
                if(!tempHierarchy.containes(".")) {
                    tempHierarchy.clear();
                }
                else {
                    tempHierarchy.erase(0,tempHierarchy.find(".") + 1);
                }

            }

            HString c = mComponentNames[i];
            HString p = mPortNames[i];
            HString v = mVariableNames[i];

            HString hdf5name = "/results/";
            varhier.insert(std::pair<HString,char>(hdf5name, 0));
            for (const auto &sysname : sysnames) {
                hdf5name.append(sysname);
                varhier.insert(std::pair<HString,char>(hdf5name, 0));
                hdf5name.append("/");
            }
            if (!c.empty()) {
                hdf5name.append(c);
                varhier.insert(std::pair<HString,char>(hdf5name, 0));
                if (!p.empty()) {
                    hdf5name.append("/").append(p);
                    varhier.insert(std::pair<HString,char>(hdf5name, 0));
                }
            }
        }
        // Create all Groups
        for (const auto &var : varhier) {
            file.createGroup(var.first.c_str());
        }

        for(size_t i=0; i<mSystemHierarchies.size(); ++i) {
            // Create a dataspace for a vector of data
            hsize_t dims[1];
            dims[0] = size_t(mDataVectors[i].size());
            H5::DataSpace dataspace(1, dims);

            // Convert full or smart name to hdf5 path (groups) name
            //Split system hierarchy string to a vector of sub strings
            std::vector<HString> sysnames;
            while(!mSystemHierarchies[i].empty()) {
                sysnames.push_back(mSystemHierarchies[i].substr(0, mSystemHierarchies[i].find(".")));
                mSystemHierarchies[i].erase(0,mSystemHierarchies[i].find(".") + 1);
                if(!mSystemHierarchies[i].containes(".")) {
                    mSystemHierarchies[i].clear();
                }
                else {
                    mSystemHierarchies[i].erase(0,mSystemHierarchies[i].find(".") + 1);
                }
            }

            HString c = mComponentNames[i];
            HString p = mPortNames[i];
            HString v = mVariableNames[i];

            std::vector<HString> hdf5names;
            HString hdf5fullname = "/results/";
            for (const auto &sysname : sysnames) {
                hdf5fullname.append(sysname.c_str());
                hdf5fullname.append("/");
            }
            if (!c.empty()) {
                hdf5fullname.append(c.c_str());
                if (!p.empty()) {
                    hdf5fullname.append("/").append(p.c_str());
                }
            }
            // Append last part of the name
            hdf5fullname.append("/").append(v.c_str());
            hdf5names.push_back(hdf5fullname);

            // If we have an alias aswell then append it also
            //! @todo should alias be model global ?
            if (!mAliasNames[i].empty()) {
                HString aliasStr = mSystemHierarchies[i];
                aliasStr.replace(".", "/");
                aliasStr.append("/");
                aliasStr.append(mAliasNames[i]);
                hdf5names.push_back(aliasStr.c_str());
            }

            for (const auto &rHdf5Name : hdf5names) {
                // Create the data set, we hope that the code above has created the group already
                // if not then we will fail here and exit with an exception
                H5::DataSet dataset = file.createDataSet(rHdf5Name.c_str(), H5::PredType::NATIVE_DOUBLE, dataspace);

                // Write the data
                // Note! if name is already taken, then this will throw an exception
                dataset.write(mDataVectors[i].data(), H5::PredType::NATIVE_DOUBLE);

                // Add meta data attributes
                appendH5Attribute(dataset, "Unit", mUnits[i].c_str());
                appendH5Attribute(dataset, "Quantity", mUnits[i].c_str());
            }
        }

        file.close();
    }
    // catch failure caused by the H5File operations
    catch(H5::FileIException &e) {
        mLastError = HString(e.getCDetailMsg())+" in "+HString(e.getCFuncName());
        return false;
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException &e) {
        mLastError = HString(e.getCDetailMsg())+" in "+HString(e.getCFuncName());
        return false;
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException &e) {
        mLastError = HString(e.getCDetailMsg())+" in "+HString(e.getCFuncName());
        return false;
    }

    return true;
}

const hopsan::HString &HopsanHDF5Exporter::getLastError()
{
    return mLastError;
}


