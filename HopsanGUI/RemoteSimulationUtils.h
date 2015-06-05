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
    enum ReschedulingMethodEnumT {None, InternalLoadBalance, ExternalRescheduling};

    RemoteModelSimulationQueuer() : mReschedulingMethod(None) {}
    virtual ~RemoteModelSimulationQueuer();
    void reset();
    void clear();

    virtual void benchmarkModel(ModelWidget* pModel);
    virtual void determineBestSpeedup(int maxNumThreads, int maxPA, int &rPM, int &rPA, double &rSU );
    virtual void setupModelQueues(QVector<ModelWidget*> models, int numThreads);

    virtual bool simulateModels(bool &rExternalReschedulingNeeded);
    virtual bool simulateModels();

    bool hasServers() const;

    int mNumThreadsPerModel = -1;

protected:
    QVector<ModelWidget*> mAllModels;
    QVector<QQueue<ModelWidget*>> mModelQueues;
    QVector<SharedRemoteCoreSimulationHandlerT> mRemoteCoreSimulationHandlers;
    SharedRemoteCoreAddressHandlerT mpRemoteCoreAddressHandler;
    QStringList mServerBlacklist;
    QVector<double> mNumThreadsVsModelEvalTime;
    ReschedulingMethodEnumT mReschedulingMethod;
    int mAlgorithmMaxPa = 1;

    bool connectToAddressServer();

    virtual double SUa(int numParallellEvaluators, int numParticles);
    virtual double SUq(int Pa, int Pm, int np, int nc);
    double SUm(int nThreads);
};


class RemoteModelSimulationQueuerLB : public RemoteModelSimulationQueuer
{
public:
    virtual bool simulateModels(bool &rExternalReschedulingNeeded);
    virtual bool simulateModels();
};

class RemoteModelSimulationQueuerCRFP : public RemoteModelSimulationQueuerLB
{
public:
    virtual bool simulateModels();
};


class RemoteModelSimulationQueuer_Homo_Re_PSO : public RemoteModelSimulationQueuerLB
{
public:
    RemoteModelSimulationQueuer_Homo_Re_PSO()
    {
        mReschedulingMethod=InternalLoadBalance;
        mAlgorithmMaxPa = 1024;
    }
protected:
    virtual double SUa(int numParallellEvaluators, int numParticles);
};

class RemoteModelSimulationQueuer_Homo_Re_CRFP0 : public RemoteModelSimulationQueuerCRFP
{
public:
    RemoteModelSimulationQueuer_Homo_Re_CRFP0()
    {
        mReschedulingMethod=ExternalRescheduling;
        mAlgorithmMaxPa = 8;
    }
protected:
    double SUa(int numParallellEvaluators, int numParticles);
};

class RemoteModelSimulationQueuer_Homo_Re_CRFP1 : public RemoteModelSimulationQueuerCRFP
{
public:
    RemoteModelSimulationQueuer_Homo_Re_CRFP1()
    {
        mReschedulingMethod=ExternalRescheduling;
        mAlgorithmMaxPa = 8;
    }
protected:
    double SUa(int numParallellEvaluators, int numParticles);
    double SUq(int Pa, int Pm, int np, int nc);
};



enum RemoteModelSimulationQueuerType {Basic, Pso_Homo_Reschedule, Crfp0_Homo_Reschedule, Crfp1_Homo_Reschedule};
void chooseRemoteModelSimulationQueuer(RemoteModelSimulationQueuerType type);
extern RemoteModelSimulationQueuer *gpRemoteModelSimulationQueuer;


#endif


#endif // REMOTESIMULATIONUTILS_H
