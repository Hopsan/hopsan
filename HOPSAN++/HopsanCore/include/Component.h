/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   Component.h
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Component base classes
//!
//$Id$

#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include "Node.h"
#include "Port.h"
#include "Parameters.h"
#include "win32dll.h"
#include <string>
#include <map>

namespace hopsan {

#pragma pack(push, 1)
typedef struct HopsanExternalLibInfo
{
    char* hopsanCoreVersion;
    char* libCompiledDebugRelease;
    char* libName;
}HopsanExternalLibInfoT;
#pragma pack(pop)

//Forward declaration
class ComponentSystem;
class HopsanEssentials;
class HopsanCoreMessageHandler;

//! @brief Enum type for parameters, describing if they are to be dynamic or constant
enum ParamDynConstT {Dynamic, Constant};

class DLLIMPORTEXPORT Component
{
    friend class ComponentSystem;
    friend class HopsanEssentials; //Need to be able to set typename

public:
    //! @brief Enum type for all CQS types
    enum CQSEnumT {C, Q, S, UndefinedCQSType};

    //==========Public functions==========
    // Configureation and simulation functions
    virtual void configure();
    virtual void deconfigure();
    virtual bool initialize(const double startT, const double stopT);
    virtual void simulate(const double startT, const double Ts);
    virtual void finalize(const double startT, const double Ts);
    virtual void setDesiredTimestep(const double timestep);
    virtual void setInheritTimestep(const bool inherit=true);
    virtual bool doesInheritTimestep() const;
    virtual bool checkModelBeforeSimulation();

    // Name and type
    void setName(std::string name, bool doOnlyLocalRename=false);
    const std::string getName() const;
    const std::string getTypeName() const;
    const std::string getSubTypeName() const;
    void setSubTypeName(const std::string subTypeName);

    // Component type identification
    virtual CQSEnumT getTypeCQS() const;
    std::string getTypeCQSString() const;
    virtual bool isComponentC() const;
    virtual bool isComponentQ() const;
    virtual bool isComponentSystem() const;
    virtual bool isComponentSignal() const;

    // Parameters
    //void registerDynamicParameter(const std::string name, const std::string description, const std::string unit, double &rValue);
    void initializeDynamicParameters();
    void updateDynamicParameterValues();

    void registerParameter(const std::string name, const std::string description, const std::string unit, double &rValue, const ParamDynConstT dynconst=Dynamic);
    void registerParameter(const std::string name, const std::string description, const std::string unit, int &rValue);
    void registerParameter(const std::string name, const std::string description, const std::string unit, std::string &rValue);
    void registerParameter(const std::string name, const std::string description, const std::string unit, bool &rValue);
    virtual void unRegisterParameter(const std::string name);

    bool hasParameter(const std::string name) const;
    const std::vector<Parameter*> *getParametersVectorPtr() const;
    void getParameterNames(std::vector<std::string> &rParameterNames);
    const Parameter *getParameter(const std::string name);
    void getParameterValue(const std::string name, std::string &rValue);
    void* getParameterDataPtr(const std::string name);
    bool setParameterValue(const std::string name, const std::string value, bool force=false);
    void updateParameters();
    bool checkParameters(std::string &errParName);

    // Start values
    double getStartValue(Port* pPort, const size_t idx, const size_t portIdx=0);
    void setStartValue(Port* pPort, const size_t idx, const double value);
    void disableStartValue(Port* pPort, const size_t idx);
    virtual void loadStartValues();
    virtual void loadStartValuesFromSimulation();

    // Ports
    std::vector<Port*> getPortPtrVector();
    Port *getPort(const std::string portname);
    std::vector<std::string> getPortNames();

    // System parent
    ComponentSystem *getSystemParent();
    size_t getModelHierarchyDepth();

    //! @todo Should it be possible to set timestep of a component? Should only be possible for a Systemcomponent
    //void setTimestep(const double timestep);
    double getTimestep() const;
    double *getTimePtr();

    void setMeasuredTime(const double time);
    double getMeasuredTime() const;

    void addDebugMessage(const std::string message, const std::string tag="");
    void addWarningMessage(const std::string message, const std::string tag="");
    void addErrorMessage(const std::string message, const std::string tag="");
    void addInfoMessage(const std::string message, const std::string tag="");

    // Stop a running simulation
    void stopSimulation();

    // HopsanEssentials
    HopsanEssentials *getHopsanEssentials();

    //Searchpath
    std::string findFilePath(const std::string fileName);

protected:
    //==========Protected member functions==========
    // Constructor - Destructor
    Component();
    virtual ~Component();

    // Virtual functions
    virtual void initialize(); //!< @todo We should really be able to return sucess true or false from components
    virtual void simulateOneTimestep();
    virtual void finalize();
    virtual void setTimestep(const double timestep);
    inline virtual size_t calcNumSimSteps(const double startT, const double stopT)
    {
        // Round to nearest, we may not get exactly the stop time that we want
        return size_t((stopT-startT)/mTimestep+0.5);
    }

    // Port functions
    Port* addPort(const std::string portName, const PortTypesEnumT portType, const NodeTypeT nodeType, const Port::ReqConnEnumT reqConnection);
    Port* addPowerPort(const std::string portName, const std::string nodeType, const Port::ReqConnEnumT reqConnect=Port::REQUIRED);
    Port* addReadPort(const std::string portName, const std::string nodeType, const Port::ReqConnEnumT reqConnect=Port::REQUIRED);
    Port* addWritePort(const std::string portName, const std::string nodeType, const Port::ReqConnEnumT reqConnect=Port::REQUIRED);
    Port* addPowerMultiPort(const std::string portName, const std::string nodeType, const Port::ReqConnEnumT reqConnect=Port::REQUIRED);
    Port* addReadMultiPort(const std::string portname, const std::string nodetype, const Port::ReqConnEnumT reqConnect=Port::REQUIRED);

    bool getPort(const std::string portname, Port* &rpPort);
    std::string renamePort(const std::string oldname, const std::string newname);
    void deletePort(const std::string name);

    // NodeData ptr function
    double *getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue=0);
    double *getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId, const double defaultValue=0);

    // Unique name functions
    virtual std::string determineUniquePortName(std::string portname);

    //==========Protected member variables==========
    bool mInheritTimestep;
    double mTimestep, mDesiredTimestep;
    double mTime;

    size_t mModelHierarchyDepth; //!< This variable containes the depth of the system in the model hierarchy, (used by connect to figure out where to store nodes)
    std::vector< std::pair<double*, double*> > mDynamicParameterDataPtrs;

    ComponentSystem* mpSystemParent;

    std::vector<std::string> mSearchPaths;

private:
    typedef std::map<std::string, Port*> PortPtrMapT;
    typedef std::pair<std::string, Port*> PortPtrPairT;

    // Private member functions
    void setSystemParent(ComponentSystem *pComponentSystem);
    void setTypeName(const std::string typeName);

    // Private member variables
    std::string mName;
    std::string mTypeName;
    std::string mSubTypeName;
    Parameters *mpParameters;
    PortPtrMapT mPortPtrMap;
    double mMeasuredTime;
    HopsanEssentials *mpHopsanEssentials;
    HopsanCoreMessageHandler *mpMessageHandler;
};



class DLLIMPORTEXPORT ComponentSignal : public Component
{
public:
    CQSEnumT getTypeCQS() const {return S;}
    bool isComponentSignal() const {return true;}
};


class DLLIMPORTEXPORT ComponentC : public Component
{
    CQSEnumT getTypeCQS() const {return C;}
    bool isComponentC() const {return true;}
};


class DLLIMPORTEXPORT ComponentQ : public Component
{
    CQSEnumT getTypeCQS() const {return Q;}
    bool isComponentQ() const {return true;}
};

typedef ClassFactory<std::string, Component> ComponentFactory;
}

#endif // COMPONENT_H_INCLUDED
