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
//! @file   HopsanEssentials.h
//! @author <peter.nordin@liu.se>
//! @date   2010-02-19
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#ifndef HopsanEssentials_H
#define HopsanEssentials_H

#include "Node.h"
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/LoadExternal.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include <iostream>
#include <fstream>

namespace hopsan {
extern "C" {
    //! @brief This class gives access to HopsanCore Initialization, externalLib loading and ComponentCreation.
    //!
    //! This is a singleton class and as such it can only be used in one instance in any main program
    //! further it is not capable of deleting it self. You have to do that manually by using "delete some_ptr"
    //! or you can let the operating system clean it up when the program exits (If you can live with that).
    //! DO NOT create an instance of this object within HopsanCore this instance will not be the same as the one in the main program.
    //! If you need to use it, the main program should pass in a pointer.
    //!
    class DLLIMPORTEXPORT HopsanEssentials
    {
    private:
        static bool mHasInstance;
        static HopsanEssentials* mpInstance;
        NodeFactory* mpNodeFactory;
        ComponentFactory* mpComponentFactory;
        HopsanCoreMessageHandler* mpMessageHandler;
        LoadExternal mExternalLoader;

        void Initialize();
        HopsanEssentials();

    public:
        static HopsanEssentials* getInstance();
        ~HopsanEssentials();

        std::string getCoreVersion();

        Component* CreateComponent(const std::string &rString);
        bool hasComponent(const std::string type);
        ComponentSystem* CreateComponentSystem();

        Node* createNode(const NodeTypeT &rNodeType);

        void getMessage(std::string &rMessage, std::string &rType, std::string &rTag);
        size_t checkMessage();

        bool loadExternalComponentLib(const std::string path);
        bool unLoadExternalComponentLib(const std::string path);
    };
}

void addLogMess(std::string log);

static std::ofstream hopsanLogFile;
}
#endif // HopsanEssentials_H
