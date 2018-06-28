/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#ifndef RELAYHANDLER_H
#define RELAYHANDLER_H

#include <thread>
#include <mutex>
#include <queue>

#include "zmq.hpp"
#include "hopsanremotecommon/MessageUtilities.h"

typedef std::lock_guard<std::mutex> scope_lock;

extern zmq::context_t gContext;

class Relay
{
public:
    Relay(const std::string &id, const std::string &endpoint ) : mId(id), mEndpoint(endpoint)
    {
        mHaveFrontMessage.lock();
    }

    bool connectToEndpoint()
    {
        int linger_ms = 1000;
        try
        {
            mpBackendSocket = new zmq::socket_t(gContext, ZMQ_REQ);
            mpBackendSocket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
            mpBackendSocket->connect(mEndpoint.c_str());
            return true;
        }
        catch(zmq::error_t e)
        {
            std::cout << "Error: Relay could not create backend socket" << std::endl;
            return false;
        }
    }

    void disconnectFromEndpoint()
    {
        mpBackendSocket->disconnect(mEndpoint.c_str());
        delete mpBackendSocket;
        mpBackendSocket=nullptr;
    }

    void relayThread()
    {
        while (true)
        {
            zmq::message_t *pFrontendMessage=nullptr;

            // Wait here until we have a message to relay through backend
            mHaveFrontMessage.lock();

            // If we ahve one then fetch it from queue
            mMutex.lock();
            mHaveFrontMessage.unlock();
            if (!mFrontendQueue.empty())
            {
                pFrontendMessage = mFrontendQueue.front();
                mFrontendQueue.pop();
            }
            // If we have no more messages then lock the mutex so taht we will wait next time
            if (mFrontendQueue.empty())
            {
                mHaveFrontMessage.lock();
            }
            mMutex.unlock();

            // If we have a frontend message and the backend socket exists, tehn relay teh message
            if (pFrontendMessage && mpBackendSocket)
            {
                // Send message
                mpBackendSocket->send(*pFrontendMessage);

                // Wait for response
                zmq::message_t backend_return_msg;
                bool rc = receiveWithTimeout(*mpBackendSocket, 10000, backend_return_msg); //!< @todo timeout size
                if (!rc)
                {
                    std::cout << "Error: Timeout in Realy receive" << std::endl;
                    backend_return_msg = createZmqMessage(NotAck, "Receive timeout in Relay");
                }

                //            size_t offset;
                //            bool unpackok;
                //            size_t id = getMessageId(back_msg, offset, unpackok);
                //            std::cout << "back_msg id:" << id << std::endl;

                // Now it should be safe to delete the front message (as we have gotten a reply)
                delete pFrontendMessage;

                // Push the return message onto return queue
                mMutex.lock();
                zmq::message_t *pNewMsg = new zmq::message_t();
                pNewMsg->move(&backend_return_msg);
                mBackendQueue.push(pNewMsg);
                mHaveResponse = true; // Raise flag
                mMutex.unlock();
            }

            if (mStopThread)
            {
                break;
            }
        }
        mThreadRunning = false;
    }

    void startRelaying()
    {
        if (!mThreadRunning)
        {
            mStopThread= false;
            mThreadRunning = true;
            mpThread = new std::thread(&Relay::relayThread, this);
        }
    }

    void stopRelaying()
    {
        mStopThread = true;
        mHaveFrontMessage.unlock();
        mpThread->join();
        delete mpThread;
        mpThread = nullptr;
    }

    std::string id() const
    {
        return mId;
    }

    bool haveResponse() const
    {
        return mHaveResponse;
    }

    bool threadRunning() const
    {
        return mThreadRunning;
    }

    void popResponse(zmq::message_t &rResponse)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        rResponse.move(mBackendQueue.front());
        delete mBackendQueue.front();
        mBackendQueue.pop();
        // IF we poped the last message then lower the flag
        if (mBackendQueue.empty())
        {
            mHaveResponse = false;
        }
    }

    void pushMessage(zmq::message_t &msg)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        zmq::message_t *pNewMsg = new zmq::message_t();
        pNewMsg->move(&msg);
        mFrontendQueue.push(pNewMsg);
        // When a new message is pushed, we unlock this mutex as asignal for the thread to move on and process it
        mHaveFrontMessage.unlock();
    }

    bool isFinished() const
    {
        return !mThreadRunning && !mHaveResponse;
    }

private:
    std::string mId;
    std::string mEndpoint;
    std::queue<zmq::message_t*> mFrontendQueue;
    std::queue<zmq::message_t*> mBackendQueue;
    zmq::socket_t *mpBackendSocket;
    std::thread *mpThread=nullptr;
    std::mutex mMutex;
    std::mutex mHaveFrontMessage;
    bool mStopThread = true;
    bool mHaveResponse = false;
    bool mThreadRunning = false;
};

class RelayHandler
{
public:
    ~RelayHandler()
    {
        // Remove and close all relays
        for (auto it=mRelays.begin(); it!=mRelays.end(); ++it)
        {
            removeRelay(it->first);
            it = mRelays.begin();
        }
        purgeRemoved();
    }

    Relay* addRelay(std::string id, std::string endpoint)
    {
        scope_lock lock(mMutex);
        Relay *pRelay = new Relay(id, endpoint);
        mRelays.insert(std::pair<std::string, Relay*>(id, pRelay));
        return pRelay;
    }

    void removeRelay(const std::string &id)
    {
        scope_lock lock(mMutex);
        auto it = mRelays.find(id);
        if (it != mRelays.end())
        {
            mRemoved.push_back(it->second);
            mRelays.erase(it);
            mRemoved.back()->stopRelaying();
        }
    }

    void purgeRemoved()
    {
        scope_lock lock(mMutex);
        for (auto it=mRemoved.begin(); it!=mRemoved.end(); ++it)
        {
            if ((*it)->isFinished())
            {
                (*it)->disconnectFromEndpoint();
                delete *it;
                mRemoved.erase(it);
                it=mRemoved.begin();
            }
        }
    }

    std::list<Relay*> getRelays()
    {
        scope_lock lock(mMutex);
        std::list<Relay*> out;
        for (auto it=mRelays.begin(); it!=mRelays.end(); ++it)
        {
            out.push_back(it->second);
        }
        return out;
    }

    Relay* getRelay(const std::string &rRelayId)
    {
        scope_lock lock(mMutex);
        auto it = mRelays.find(rRelayId);
        if (it != mRelays.end())
        {
            return it->second;
        }
        return 0;
    }

private:
    std::mutex mMutex;
    std::map<std::string, Relay*> mRelays;
    std::list<Relay*> mRemoved;
};

extern RelayHandler gRelayHandler;

#endif // RELAYHANDLER_H
