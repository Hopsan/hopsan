#ifndef VARIABLESTORAGE_H
#define VARIABLESTORAGE_H

#include <string>
#include <map>

namespace numhop {

class ExternalVariableStorage
{
public:
    virtual ~ExternalVariableStorage() {}

    // Overload to find external variables
    virtual double externalValue(std::string name, bool &rFound) const = 0;

    // Overload this to set your external value
    // return true if success, false if not (then variable should be set locally)
    virtual bool setExternalValue(std::string name, double value) = 0;
};

class VariableStorage
{
public:
    VariableStorage();
    bool setVariable(const std::string &name, double value, bool &rDidSetExternally);
    double value(const std::string &name, bool &rFound) const;

    bool isNameInternalValid(const std::string &name) const;
    void setDisallowedInternalNameCharacters(const std::string &disallowed);

    void setExternalStorage(ExternalVariableStorage *pExternalStorage);
    void setParentStorage(VariableStorage *pParentStorage);

    void clearInternalVariables();

private:
    ExternalVariableStorage *mpExternalStorage;
    VariableStorage *mpParentStorage;
    std::map<std::string, double> mVariableMap;
    std::string mDisallowedInternalNameChars;
};

}

#endif // VARIABLESTORAGE_H
