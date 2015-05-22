#ifndef REMOTESIMULATIONUTILS_H
#define REMOTESIMULATIONUTILS_H

#include <QVector>
#include <QQueue>

#ifdef USEZMQ
class ModelWidget;

#include "RemoteCoreAccess.h"

class RemoteModelSimulationQueuer
{
public:
    void setupSimulationHandlers(QVector<ModelWidget*> models);
    void clear();

    void simulateModels();

    bool hasServers() const;

    int mnThreadsPerModel = 1;
    QVector<ModelWidget*> mAllModels;
    QVector<QQueue<ModelWidget*>> mModelQueues;
    QVector<SharedRemoteCoreSimulationHandlerT> mRemoteCoreSimulationHandlers;
    SharedRemoteCoreAddressHandlerT mpRemoteCoreAddressHandler;
};

extern RemoteModelSimulationQueuer gRemoteModelSimulationQueuer;

#endif


#endif // REMOTESIMULATIONUTILS_H
