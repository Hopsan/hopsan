//$Id$

#ifndef GLOBAL_H
#define GLOBAL_H

class ServerConfig
{
public:
    int mControlPort = 23300;
    int mClientPortStart = 23301;
    int mClientPortEnd = 23310;
    int mMaxClients = 20;
    int mMaxThreadsPerClient = 2;
};

extern ServerConfig gServerConfig;

#endif // GLOBAL_H
