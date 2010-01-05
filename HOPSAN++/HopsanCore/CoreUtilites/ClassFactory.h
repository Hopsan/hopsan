#ifndef CLASFACTORY_H_INCLUDED
#define CLASFACTORY_H_INCLUDED

#include <map>
#include <vector>
#include <iostream>

//This code has been "borrowed" from:
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
    typedef std::vector<FactoryPairT> FactoryVectorT;

    // called at the beginning of execution to register creation functions
    // used later to create class instances
    static _Key RegisterCreatorFunction(_Key idKey, CreatorFunctionT classCreator)
    {
        std::cout << "Registering: " << idKey << " key" << std::endl;
        std::cout << "BeforeInsert: Size: " << getFactoryMap()->size() << std::endl;
        getFactoryMap()->insert(FactoryPairT(idKey, classCreator));
        std::cout << "AfterInsert: Size: " << getFactoryMap()->size() << std::endl;
        return idKey;
    }

    // tries to create instance based on the key
    // using creator function (if provided)
    static _Base* CreateInstance(_Key idKey)
    {
        std::cout << "Create: Size: " << getFactoryMap()->size() << std::endl;
        typename FactoryMapT::iterator it = getFactoryMap()->find(idKey);
        if (it != getFactoryMap()->end())
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
    // to prevent inserting into map before initialisation takes place
    // place it into static function as static member,
    // so it will be initialised only once - at first call

    static FactoryMapT * getFactoryMap()
    {
        static FactoryMapT smFactoryMap;
        return &smFactoryMap;
    }
};

#endif // CLASFACTORY_H_INCLUDED
