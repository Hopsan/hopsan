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
    virtual void setup(QVector<ModelWidget*> models, bool runBenchmark=false);
    void reset();
    void clear();

    virtual bool simulateModels();

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
    virtual void setup(QVector<ModelWidget*> models, bool runBenchmark=false);
    virtual bool simulateModels();

protected:
    void determineBestSpeedup(int numParticles, int maxNumThreads, int numServers, int &rPM, int &rPA, double &rSU );
    virtual double SUa(int numParallellEvaluators, int numParticles);
    double SUm(int nThreads);
    QVector<double> mNumThreadsVsModelEvalTime;
};

class RemoteModelSimulationQueuer_CRFP1_HOMO_RESCHEDULE : public RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE
{
protected:
    virtual double SUa(int numParallellEvaluators, int numParticles);
};

class RemoteModelSimulationQueuer_CRFP2_HOMO_RESCHEDULE : public RemoteModelSimulationQueuer_PSO_HOMO_RESCHEDULE
{
protected:
    virtual double SUa(int numParallellEvaluators, int numParticles);
};


enum RemoteModelSimulationQueuerType {Basic, Pso_Homo_Reschedule, Crfp1_Homo_Reschedule, Crfp2_Homo_Reschedule};

void chooseRemoteModelSimulationQueuer(RemoteModelSimulationQueuerType type);
extern RemoteModelSimulationQueuer *gpRemoteModelSimulationQueuer;


#endif


#endif // REMOTESIMULATIONUTILS_H
