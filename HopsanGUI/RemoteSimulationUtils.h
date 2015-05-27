#ifndef REMOTESIMULATIONUTILS_H
#define REMOTESIMULATIONUTILS_H

#include <QVector>
#include <QQueue>
#include <QStringList>

#ifdef USEZMQ
class ModelWidget;

#include "RemoteCoreAccess.h"

class RemoteModelSimulationQueuer
{
public:
    virtual ~RemoteModelSimulationQueuer();
    virtual void setup(QVector<ModelWidget*> models);
    void reset();
    void clear();

    virtual void simulateModels();

    bool hasServers() const;

    int mNumThreadsPerModel = 1;

protected:
    QVector<ModelWidget*> mAllModels;
    QVector<QQueue<ModelWidget*>> mModelQueues;
    QVector<SharedRemoteCoreSimulationHandlerT> mRemoteCoreSimulationHandlers;
    SharedRemoteCoreAddressHandlerT mpRemoteCoreAddressHandler;
    QStringList mServerBlacklist;

    bool connectToAddressServer();
    void accuireSlotsAndEnqueModels(QVector<ModelWidget*> models, int numThreads);
};

class RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE : public RemoteModelSimulationQueuer
{
public:
    virtual void setup(QVector<ModelWidget*> models);
    virtual void simulateModels();

protected:
    void determineBestSpeedup(int numParticles, int maxNumThreads, int numServers, int &rPM, int &rPA, double &rSU );
    double SUm(int nThreads);
    QVector<double> mNumThreadsVsModelSpeed;
};


enum RemoteModelSimulationQueuerType {Basic, Pso_Homo_Reschedule};

void chooseRemoteModelSimulationQueuer(RemoteModelSimulationQueuerType type);
extern RemoteModelSimulationQueuer *gpRemoteModelSimulationQueuer;


#endif


#endif // REMOTESIMULATIONUTILS_H
