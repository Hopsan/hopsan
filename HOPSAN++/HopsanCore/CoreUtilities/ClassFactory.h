#ifndef CLASFACTORY_H_INCLUDED
#define CLASFACTORY_H_INCLUDED

#include <map>
#include <vector>
#include <iostream>

//This code is based on:
//http://www.codeproject.com/KB/architecture/SimpleDynCreate.aspx
template <typename _Key, typename _Base, typename _Predicator = std::less<_Key> >
class ClassFactory
{
public:
    ClassFactory() {};
    ~ClassFactory() {};

    typedef _Base* (*CreatorFunctionT) (void);
    typedef std::map<_Key, CreatorFunctionT, _Predicator> FactoryMapT;
    typedef std::pair<_Key, CreatorFunctionT> FactoryPairT;
    typedef std::vector<FactoryPairT> FactoryPairVectorT;

    // Used to register creator functions
    _Key RegisterCreatorFunction(_Key idKey, CreatorFunctionT classCreator)
    {
        std::cout << "Registering: " << idKey << " key" << std::endl;
        std::cout << "BeforeInsert: Size: " << mFactoryMap.size() << std::endl;
        mFactoryMap.insert(FactoryPairT(idKey, classCreator));
        std::cout << "AfterInsert: Size: " << mFactoryMap.size() << std::endl;
        return idKey;
    }

    // tries to create instance based on the key using creator function (if provided)
    _Base* CreateInstance(_Key idKey)
    {
        std::cout << "Create: Size: " << mFactoryMap.size() << std::endl;
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

protected:
    // map where the construction info is stored
    FactoryMapT mFactoryMap;

//    static FactoryMapT * getFactoryMap()
//    {
//        static FactoryMapT smFactoryMap;
//        return &smFactoryMap;
//    }
};

#endif // CLASFACTORY_H_INCLUDED
