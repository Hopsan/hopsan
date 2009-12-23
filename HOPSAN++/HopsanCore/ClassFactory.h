#ifndef CLASFACTORY_H_INCLUDED
#define CLASFACTORY_H_INCLUDED

#include <map>

template <typename _Key, typename _Base, typename _Predicator = std::less<_Key> >
class CClassFactory
{
public:
    CClassFactory() {};
    ~CClassFactory() {};

    typedef _Base* (*CreatorFunction) (void);
    typedef std::map<_Key, CreatorFunction, _Predicator> _mapFactory;

    // called at the beginning of execution to register creation functions
    // used later to create class instances
    static _Key RegisterCreatorFunction(_Key idKey, CreatorFunction classCreator)
    {
        get_mapFactory()->insert(std::pair<_Key, CreatorFunction>(idKey, classCreator));
        return idKey;
    }

    // tries to create instance based on the key
    // using creator function (if provided)
    static _Base* CreateInstance(_Key idKey)
    {
        typename _mapFactory::iterator it = get_mapFactory()->find(idKey);
        if (it != get_mapFactory()->end())
        {
            if (it->second)
            {
                return it->second();
            }
        }
        return NULL;
    }

protected:
    // map where the construction info is stored
    // to prevent inserting into map before initialisation takes place
    // place it into static function as static member,
    // so it will be initialised only once - at first call

    static _mapFactory * get_mapFactory()
    {
        static _mapFactory m_sMapFactory;
        return &m_sMapFactory;
    }
};

#endif // CLASFACTORY_H_INCLUDED
