/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   ComponentSystem.h
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains the subsystem component class and connection assistant help class
//!
//$Id$

#ifndef COMPONENTSYSTEM_H
#define COMPONENTSYSTEM_H

#if __cplusplus > 199711L
#include <mutex>
#include <chrono>
#include <ctime>
#endif

#include "Component.h"
#include "CoreUtilities/SimulationHandler.h"
#include "CoreUtilities/AliasHandler.h"

namespace hopsan {
    class NumHopHelper;

    class DLLIMPORTEXPORT ComponentSystem :public Component
    {
        friend class ConnectionAssistant;
        friend class AliasHandler;

    public:
        enum UniqeNameEnumT {UniqueComponentNameType, UniqueSysportNameTyp, UniqueSysparamNameType, UniqueAliasNameType, UniqueReservedNameType};
        typedef std::map<HString, std::pair<std::vector<HString>, std::vector<HString> > > SetParametersMapT;

        //==========Public functions==========
        virtual ~ComponentSystem();
        static Component* Creator(){ return new ComponentSystem(); }
        virtual void configure();

        // Subsystem CQS type methods
        bool isComponentSystem() const {return true;}
        CQSEnumT getTypeCQS() const;
        void setTypeCQS(CQSEnumT cqs_type, bool doOnlyLocalSet=false);
        bool changeSubComponentSystemTypeCQS(const HString &rName, const CQSEnumT newType);
        void determineCQSType();
        bool isTopLevelSystem() const;
        bool isExternalSystem() const;
        void setExternalModelFilePath(const HString &rPath);
        HString getExternalModelFilePath() const;

        // Adding removing and renaming components
        void addComponents(std::vector<Component*> &rComponents);
        void addComponent(Component *pComponent);
        void renameSubComponent(const HString &rOldName, const HString &rNewName);
        void removeSubComponent(const HString &rName, bool doDelete=false);
        void removeSubComponent(Component *pComponent, bool doDelete=false);
        HString reserveUniqueName(const HString &rDesiredName, const UniqeNameEnumT type=UniqueReservedNameType);
        void unReserveUniqueName(const HString &rName);

        // System Parameter functions
        bool renameParameter(const HString &rOldName, const HString &rNewName);
        virtual std::list<HString> getModelAssets() const;

        // Handle system ports
        Port* addSystemPort(HString portName, const HString &rDescription="");
        HString renameSystemPort(const HString &rOldname, const HString &rNewname);
        void deleteSystemPort(const HString &rName);

        // Getting added components and component names
        Component* getSubComponentOrThisIfSysPort(const HString &rName);
        Component* getSubComponent(const HString &rName) const;
        const std::vector<Component*> getSubComponents() const;
        ComponentSystem* getSubComponentSystem(const HString &rName) const;
        std::vector<HString> getSubComponentNames() const;
        bool haveSubComponent(const HString &rName) const;
        bool isEmpty() const;

        // Alias handler
        AliasHandler &getAliasHandler();

        // Connecting and disconnecting components
        bool connect(Port *pPort1, Port *pPort2);
        bool connect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2);
        bool disconnect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2);
        bool disconnect(Port *pPort1, Port *pPort2);

        // Convenience functions to enable and disable data logging
        void setAllNodesDoLogData(const bool logornot);

        // Start value loading
        bool keepsValuesAsStartValues();
        void setKeepValuesAsStartValues(bool load);
        void loadStartValues();
        void loadStartValuesFromSimulation();
        void evaluateParametersRecursively();

        // Parameter loading
        void loadParameters(const HString &rFilePath);
        void loadParameters(const SetParametersMapT &rParameterMap);

        // NumHop script
        bool evaluateNumHopScriptRecursively();
        bool runNumHopScript(const HString &rScript, bool printOutput, HString &rOutput);
        void setNumHopScript(const HString &rScript);

        // Initialize and simulate
        bool checkModelBeforeSimulation();
        virtual bool preInitialize();
        bool initialize(const double startT, const double stopT);
        void simulate(const double stopT);
        virtual void simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads = 0, const bool noChanges=false, ParallelAlgorithmT algorithm=OfflineSchedulingAlgorithm);
        void finalize();

        bool simulateAndMeasureTime(const size_t nSteps);
        double getTotalMeasuredTime();
        void sortComponentVectorsByMeasuredTime();
        void distributeCcomponents(std::vector< std::vector<Component*> > &rSplitCVector, size_t nThreads);
        void distributeQcomponents(std::vector< std::vector<Component*> > &rSplitQVector, size_t nThreads);
        void distributeSignalcomponents(std::vector< std::vector<Component*> > &rSplitSignalVector, size_t nThreads);
        void distributeNodePointers(std::vector< std::vector<Node*> > &rSplitNodeVector, size_t nThreads);
        void reschedule(size_t nThreads);

        // Set and get desired timestep
        void setDesiredTimestep(const double timestep);
        void setInheritTimestep(const bool inherit=true);
        bool doesInheritTimestep() const;
        double getDesiredTimeStep() const;

        // Log functions
        void logTimeAndNodes(const size_t simStep);
        void enableLog();
        void disableLog();
        std::vector<double>* getLogTimeVector();
        void setNumLogSamples(const size_t nLogSamples);
        double getLogStartTime() const;
        void setLogStartTime(const double logStartTime);
        size_t getNumLogSamples() const;
        size_t getNumActuallyLoggedSamples() const;

        // Stop a running initialization or simulation
        void stopSimulation(const HString &rReason);
        void stopSimulation();
        bool wasSimulationAborted();

        // System parameters
        bool setOrAddSystemParameter(const HString &rName, const HString &rValue, const HString &rType, const HString &rDescription="", const HString &rUnitOrQuantity="", const bool force=false);
        bool setSystemParameter(const HString &rName, const HString &rValue, const HString &rType, const HString &rDescription="", const HString &rUnitOrQuantity="", const bool force=false);
        void unRegisterParameter(const HString &name);
        void addSearchPath(const HString &searchPath);

        // Add and Remove sub-nodes
        void addSubNode(Node* pNode);
        void removeSubNode(Node* pNode);

    protected:
        // Constructor - Destructor- Creator
        ComponentSystem();

        // Internal Flags
        //! @brief This bool can be toggled off in programmed subsystems to avoid annoying warnings
        //! @ingroup ComponentPowerAuthorFunctions
        bool mWarnIfUnusedSystemParameters;

        // Log and timestep
        std::vector<size_t> mLogTheseTimeSteps;
        size_t mTotalTakenSimulationSteps;

        typedef std::map<HString, Component*> SubComponentMapT;
        SubComponentMapT mSubComponentMap;

        NumHopHelper *mpNumHopHelper;
        HString mNumHopScript;

    private:
        //==========Private functions==========
        // Time specific functions
        void setTimestep(const double timestep);
        void adjustTimestep(std::vector<Component*> componentPtrs);

        // log specific functions
        //! @todo restore these in some way
//        void setLogSettingsSampleTime(double log_dt, double start, double stop, double sampletime);
//        void setLogSettingsSkipFactor(double factor, double start, double stop, double sampletime);
        void setupLogSlotsAndTs(const double simStartT, const double simStopT, const double simTs);
        void preAllocateLogSpace();

        // Add and Remove subcomponent ptrs from storage vectors
        void addSubComponentPtrToStorage(Component* pComponent);
        void removeSubComponentPtrFromStorage(Component* pComponent);

        // Clear all contents of the system (use in destructor)
        void clear();

        bool sortComponentVector(std::vector<Component*> &rOldSignalVector);

        // UniqueName specific functions
        HString determineUniquePortName(const HString &rPortname);
        HString determineUniqueComponentName(const HString &rName) const;
        bool hasReservedUniqueName(const HString &rName) const;

        //==========Private member variables==========
        CQSEnumT mTypeCQS;
        HString mExternalModelFilePath;

        std::vector<Component*> mComponentSignalptrs;
        std::vector<Component*> mComponentQptrs;
        std::vector<Component*> mComponentCptrs;
        std::vector<Component*> mComponentUndefinedptrs;
        std::vector<Node*> mSubNodePtrs;

        typedef std::map<HString, UniqeNameEnumT> TakenNamesMapT;
        TakenNamesMapT mTakenNames;


        bool volatile mStopSimulation;

        // This block of variables are only used with multi-threading but they must be included always else
        // components inheriting ComponentSystem will not know that they exist resulting in overwriting memory
        //! @todo we could hide them in a private struct and put a forward declared pointer here instead
#if __cplusplus > 199711L
        std::mutex *mpStopMutex;
#endif
        std::vector<double *> mvTimePtrs;
        std::vector< std::vector<Component*> > mSplitCVector;
        std::vector< std::vector<Component*> > mSplitQVector;
        std::vector< std::vector<Component*> > mSplitSignalVector;
        std::vector< std::vector<Node*> > mSplitNodeVector;
        //------------------------------------------------------------------

        bool mKeepValuesAsStartValues;

        AliasHandler mAliasHandler;

        // Log related variables
        size_t mRequestedNumLogSamples, mnLogSlots, mLogCtr;
        double mRequestedLogStartTime, mLogTimeDt;
        bool mEnableLogData;
        std::vector<double> mTimeStorage;
    };


    class ConditionalComponentSystem : public ComponentSystem
    {
    public:
        static Component* Creator(){ return new ConditionalComponentSystem(); }
        void configure();
        void simulate(const double stopT);
        void simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges, ParallelAlgorithmT algorithm);
    private:
        double *mpCondition;
        bool mAsleep;
    };
}



#if __cplusplus > 199711L
#ifdef _WIN32
//! @todo Move to utilities?
struct HighResClock
{
    typedef long long                               rep;
    typedef std::nano                               period;
    typedef std::chrono::duration<rep, period>      duration;
    typedef std::chrono::time_point<HighResClock>   time_point;
    static const bool is_steady = true;

    static time_point now();
};
#endif //_WIN32
#endif //C++11


#endif // COMPONENTSYSTEM_H
