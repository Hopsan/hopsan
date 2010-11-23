//!
//! @file   ClassFactory.hpp
//! @author <peter.nordin@liu.se>
//! @date   2009-12-26
//!
//! @brief Contains template class ClassFactory for automatic object instantiation through key value
//!
//$Id$

#ifndef CLASFACTORY_HPP_INCLUDED
#define CLASFACTORY_HPP_INCLUDED

#include <map>
#include <iostream>

namespace hopsan {

    //! @brief Template class for automatic object instantiation by key-value.
    //!
    //! This code is based on:
    //! http://www.codeproject.com/KB/architecture/SimpleDynCreate.aspx
    template <typename _Key, typename _Base, typename _Predicator = std::less<_Key> >
    class ClassFactory
    {
    protected:
        typedef _Base* (*CreatorFunctionT) (void);
        typedef std::map<_Key, CreatorFunctionT, _Predicator> FactoryMapT;
        typedef std::pair<_Key, CreatorFunctionT> FactoryPairT;

        //Map where the construction info is stored
        FactoryMapT mFactoryMap;

    public:
        ClassFactory() {}

        //! @brief Used to register creator functions
        _Key registerCreatorFunction(_Key idKey, CreatorFunctionT classCreator)
        {
            //std::cout << "Registering: " << idKey << std::endl;
            //std::cout << "BeforeInsert: Size: " << mFactoryMap.size() << std::endl;
            std::pair<typename FactoryMapT::iterator, bool> rc;
            rc = mFactoryMap.insert(FactoryPairT(idKey, classCreator));
            if (!rc.second)
            {
                std::cout << "Warning! You are trying to register a Key value that already exist. This registration will be ignored, Key: " << idKey << std::endl;
            }
            //std::cout << "AfterInsert: Size: " << mFactoryMap.size() << std::endl;
            return idKey;
        }

        //! @brief Creates an instance based on the key using creator function (if registered)
        _Base* createInstance(_Key idKey)
        {
            //std::cout << "Create: Size: " << mFactoryMap.size() << std::endl;
            typename FactoryMapT::iterator it = mFactoryMap.find(idKey);
            if (it != mFactoryMap.end())
            {
                if (it->second)
                {
                    return it->second();
                }
            }
            std::cout << "Warning key: " << idKey << " not found!" << std::endl;
            return NULL;
        }

        //! @brief Check if the factory has key registerd
        bool hasKey(_Key idKey)
        {
            if (mFactoryMap.count(idKey) > 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        //! @brief Unregister creator functions for given key
        void unRegisterCreatorFunction(_Key idKey)
        {
            size_t rc;
            rc = mFactoryMap.erase(idKey);
            if (rc > 0)
            {
                std::cout << "Sucessfully unregistered: " << idKey << std::endl;
            }
            else
            {
                std::cout << "Failed to unregister: " << idKey << std::endl;
            }
        }

        //! @brief Clear the entire factory map (unregister everything)
        void clearFactory()
        {
            mFactoryMap.clear();
        }
    };
}

#endif // CLASFACTORY_HPP_INCLUDED
