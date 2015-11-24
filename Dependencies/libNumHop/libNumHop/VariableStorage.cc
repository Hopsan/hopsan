#include "VariableStorage.h"
#include "Helpfunctions.h"

namespace numhop {

//! @brief Default constructor
VariableStorage::VariableStorage()
{
    mpExternalStorage = 0;
    mpParentStorage = 0;
}

//! @brief Set a variable value
//! @param[in] name The name of the variable
//! @param[in] value The value
//! @param[out] rDidSetExternally Indicates if the variable was an external variable
//! @returns True if the variable was set, false otherwise
bool VariableStorage::setVariable(const std::string &name, double value, bool &rDidSetExternally)
{
    // First try to set it externally
    rDidSetExternally = false;
    if (mpExternalStorage)
    {
        rDidSetExternally = mpExternalStorage->setExternalValue(name, value);
    }
    // If we could not set externally, then set it internally
    if (isNameInternalValid(name))
    {
        std::map<std::string,double>::iterator it = mVariableMap.find(name);
        if (it == mVariableMap.end())
        {
            mVariableMap.insert(std::pair<std::string,double>(name, value));
            return true;
        }
        else
        {
            it->second = value;
            return true;
        }
    }
    return rDidSetExternally;
}

//! @brief Check if a given name is a valid internal storage name, based on given disallowed characters
//! @param[in] name The name to check
//! @returns True if the name is valid, else false
bool VariableStorage::isNameInternalValid(const std::string &name) const
{
    for (size_t i=0; i<mDisallowedInternalNameChars.size(); ++i)
    {
        if (contains(name, mDisallowedInternalNameChars[i]))
        {
            return false;
        }
    }
    return true;
}

//! @brief Set disallowed characters in internal names
//! @param[in] disallowed A string containing disallowed characters
void VariableStorage::setDisallowedInternalNameCharacters(const std::string &disallowed)
{
    mDisallowedInternalNameChars = disallowed;
}

//! @brief Get the value of a variable
//! @param[in] name The name of the variable
//! @param[out] rFound Indicates if the variable was found
//! @returns The value of the variable (if it was found, else a dummy value)
double VariableStorage::value(const std::string &name, bool &rFound) const
{
    // First try to find variable internally
    rFound=false;
    std::map<std::string, double>::const_iterator it = mVariableMap.find(name);
    if (it != mVariableMap.end())
    {
        rFound=true;
        return it->second;
    }
    // Else try to find it externally
    if (mpExternalStorage)
    {
        double value = mpExternalStorage->externalValue(name, rFound);
        if (rFound)
        {
            return value;
        }
    }
    return 0;
}

//! @brief Set the external storage
//! @param[in] pExternalStorage A pointer to the external storage to use in variable lookup
void VariableStorage::setExternalStorage(ExternalVariableStorage *pExternalStorage)
{
    mpExternalStorage = pExternalStorage;
}

//! @brief Set the parent storage (not used yet)
//! @param[in] pParentStorage A pointer to the parent storage to use in variable lookup
//! @warning Not yet implemented
void VariableStorage::setParentStorage(VariableStorage *pParentStorage)
{
    mpParentStorage = pParentStorage;
}

}
