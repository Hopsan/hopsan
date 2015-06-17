/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

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
