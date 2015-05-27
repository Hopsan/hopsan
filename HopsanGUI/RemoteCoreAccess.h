//!
//! @file   RemoteCoreAccess.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-05-11
//!
//! @brief Contains the HopsanCore Qt API classes for communication with the Remote HopsanCore
//!
//$Id$

#ifndef REMOTECOREACCESS_H
#define REMOTECOREACCESS_H

#include <QString>
#include <QMultiMap>
#include <QSharedPointer>
#include <QStringList>

#ifdef USEZMQ
class RemoteHopsanClient;

class RemoteCoreAddressHandler
{
private:
    typedef struct
    {
        QString addr;
        double speed=-1;
        int nSlots=0;
        int nOpenSlots=0;
        bool mResponding=false;
    }ServerInfoT;

    QString mHopsanAddressServerIP, mHopsanAddressServerPort;
    RemoteHopsanClient *mpRemoteHopsanClient=0;

    //! @todo shared pointer to info maybe
    QMap<QString, ServerInfoT> mAvailableServers;
    QMultiMap<double, QString> mServerSpeedMap;
    QList<QString> mNotRespondingServers;

    void requestServerInfo(QString address);
    void removeNotRespondingServers();

public:
    RemoteCoreAddressHandler();
    ~RemoteCoreAddressHandler();
    void setHopsanAddressServer( QString ip, QString port );
    void setHopsanAddressServer( QString ip_port );
    QString getAddressAndPort() const;

    bool isConnected();

    bool connect();
    void disconnect();

    QList<QString> requestAvailableServers();
    //QList<QString> requestAvailableServers(int nOpenSlots);

    QString getBestAvailableServer(int nRequiredSlots, const QStringList &rExcludeList=QStringList());
    QList<QString> getMatchingAvailableServers(double requiredSpeed, int nRequiredSlots, const QStringList &rExcludeList=QStringList());
    void getMaxNumSlots(int &rMaxNumSlots, int &rNumServers);
};

typedef QSharedPointer<RemoteCoreAddressHandler> SharedRemoteCoreAddressHandlerT;
SharedRemoteCoreAddressHandlerT getSharedRemoteCoreAddressHandler();

class RemoteCoreSimulationHandler
{
private:
    QString mRemoteServerAddress,   mRemoteServerPort;
    RemoteHopsanClient *mpRemoteHopsanClient=0;
    int mNumThreads=1;

public:
    RemoteCoreSimulationHandler();
    ~RemoteCoreSimulationHandler();

    void setHopsanServer(QString ip, QString port );
    void setHopsanServer(QString ip_port );
    QString getHopsanServerAddress() const;
    void setNumThreads(int nThreads);
    int numThreads() const;

    bool connect();
    void disconnect();

    bool loadModel(QString hmfModelFile);
    bool loadModelStr(QString hmfStr);
    bool simulateModel_blocking(double *pProgress);
    bool simulateModel_nonblocking();

    bool benchmarkModel_blocking(const QString &rModel, const int nThreads, double &rSimTime);


    bool requestSimulationProgress(double *pProgress);

    bool getCoreMessages(QVector<QString> &rTypes, QVector<QString> &rTags, QVector<QString> &rMessages, bool includeDebug=true);

    bool getLogData(std::vector<std::string> *pNames, std::vector<double> *pData);

    QString getLastError() const;
};

typedef QSharedPointer<RemoteCoreSimulationHandler> SharedRemoteCoreSimulationHandlerT;
#endif

#endif // REMOTECOREACCESS_H
