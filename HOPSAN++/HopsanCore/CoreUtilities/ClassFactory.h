//!
//! @file   ClassFactory.h
//! @author <peter.nordin@liu.se>
//! @date   2009-12-26
//!
//! @brief Contains template class ClassFactory for automatic object instantiation through key value
//!
//$Id$

#ifndef CLASFACTORY_H_INCLUDED
#define CLASFACTORY_H_INCLUDED

#include <map>
#include <vector>
#include <iostream>

namespace hopsan {

    //!
    //! @brief Template class for automatic object instantiation by key-value.
    //!
    //! This code is based on:
    //! http://www.codeproject.com/KB/architecture/SimpleDynCreate.aspx
    //!
    template <typename _Key, typename _Base, typename _Predicator = std::less<_Key> >
    class ClassFactory
    {
    public:
        ClassFactory() {}
        ~ClassFactory() {}

        typedef _Base* (*CreatorFunctionT) (void);
        typedef std::map<_Key, CreatorFunctionT, _Predicator> FactoryMapT;
        typedef std::pair<_Key, CreatorFunctionT> FactoryPairT;
        //typedef std::vector<FactoryPairT> FactoryPairVectorT;

        // Used to register creator functions
        _Key RegisterCreatorFunction(_Key idKey, CreatorFunctionT classCreator)
        {
            std::cout << "Registering: " << idKey << std::endl;
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

        //! @brief Tries to create instance based on the key using creator function (if provided)
        _Base* CreateInstance(_Key idKey)
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

        //! @brief Check if the factory have key registerd
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

        void UnRegisterCreatorFunction(_Key idKey)
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

        void ClearFactory()
        {
            mFactoryMap.clear();
        }

    protected:
        // map where the construction info is stored
        FactoryMapT mFactoryMap;

        //    static FactoryMapT * getFactoryMap()
        //    {
        //        static FactoryMapT smFactoryMap;
        //        return &smFactoryMap;
        //    }
    };
}

#endif // CLASFACTORY_H_INCLUDED
