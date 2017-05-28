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

//$Id$

#ifndef SOCKETIOTESTCOMPONENT_HPP_INCLUDED
#define SOCKETIOTESTCOMPONENT_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "socketutility.h"
#include <iostream>

#define ACK 0x01
#define NACK 0x02

namespace hopsan {

    class SocketIOTestComponent : public ComponentSignal
    {
    private:
        HString mOtherIp, mOtherPort, mThisPort;
        SocketUtility mSocketUtility;
        int mMasterSlave, mRequireACK, mInitTimout, mSimAckTimout, mSimReciveTimout, mSleepUS;
        double *mpInput, *mpOutput;
        Port *pInPort, *pOutPort;

        bool readACK(int timeout)
        {
            char ack;
            if (mSocketUtility.readSocket(ack, timeout) == 0)
            {
                addErrorMessage(HString("Failed to read ACK message before timeout! ")+mSocketUtility.getErrorString().c_str());
                return false;
            }
            if (ack != ACK)
            {
                addWarningMessage(HString("Did not receive ACK got: ")+to_hstring(ack));
                return false;
            }
            return true;
        }

        bool sendACKmsg(char msg)
        {
            if (mSocketUtility.writeSocket(msg) == 0)
            {
                addErrorMessage(mSocketUtility.getErrorString().c_str());
                return false;
            }
            return true;
        }

    public:
        static Component *Creator()
        {
            return new SocketIOTestComponent();
        }

        void configure()
        {
            addConstant("other_ip", "The ip address", "", "127.0.0.1", mOtherIp);
            addConstant("other_port", "Write port", "", "30000", mOtherPort);
            addConstant("this_port", "Read port", "", "30001", mThisPort);
            std::vector<HString> conds;
            conds.push_back("Master");
            conds.push_back("Slave");
            addConditionalConstant("masl", "Master or Slave", conds, mMasterSlave);
            conds.clear();
            conds.push_back("True");
            conds.push_back("False");
            addConditionalConstant("requireAck", "Require or Send ACK", conds, mRequireACK);
            addConstant("initTimeout", "The timeout during initialization", "ms", 5000, mInitTimout);
            addConstant("simAckTimeout", "The ack timeout during simulation", "ms", 1000, mSimAckTimout);
            addConstant("simRecTimeout", "The receive timeout during simulation", "ms", 1000, mSimReciveTimout);
            addConstant("timeoutSleep", "The timeout wait sleep", "us", -1, mSleepUS);

            pInPort = addInputVariable("in", "Value input", "", 0, &mpInput);
            pOutPort = addOutputVariable("out", "Value input", "", &mpOutput);

            addReadPort("sortIn", "NodeSignal", "Sorting port, value has no effect", Port::NotRequired);
            addWritePort("sortOut", "NodeSignal", "Sorting port, value has no effect", Port::NotRequired);
        }

        bool preInitialize()
        {
            mSocketUtility.setSleepUS(mSleepUS);
            if (!mSocketUtility.openSocket(mOtherIp.c_str(), mOtherPort.c_str(), mThisPort.c_str()))
            {
                addErrorMessage(mSocketUtility.getErrorString().c_str());
                return false;
            }
            return true;
        }

        void initialize()
        {
            // Master
            if (mMasterSlave == 0)
            {
                // Write init message
                if (mSocketUtility.writeSocket("Initialize") == 0)
                {
                    addErrorMessage(mSocketUtility.getErrorString().c_str());
                }

                // Wait for ACK or NACK
                if (mRequireACK == 0)
                {
                    if (!readACK(mInitTimout))
                    {
                        stopSimulation();
                        return;
                    }
                }
            }
            // Slave
            else if (mMasterSlave == 1)
            {
                // Wait for startup message
                std::string input;
                if (mSocketUtility.readSocket(input, 10, mInitTimout) == 0)
                {
                    addWarningMessage(HString("Failed to read startup message! "));
                }

                // Send ACK or NACK
                if (mRequireACK == 0)
                {
                    if (input == "Initialize")
                    {
                        if (!sendACKmsg(ACK))
                        {
                            stopSimulation();
                            return;
                        }
                    }
                    else
                    {
                        if (!sendACKmsg(NACK))
                        {
                            stopSimulation();
                            return;
                        }
                    }
                }
            }
        }


        void simulateOneTimestep()
        {
            // Master
            if (mMasterSlave == 0)
            {
                // Write input to socket
                if (mSocketUtility.writeSocket(*mpInput) == 0)
                {
                    addErrorMessage(mSocketUtility.getErrorString().c_str());
                }

                // Wait for ack
                if (mRequireACK == 0)
                {
                    if (!readACK(mSimAckTimout))
                    {
                        stopSimulation();
                        return;
                    }
                }

                // Wait for return message
                double tmp;
                if (mSocketUtility.readSocket(tmp, mSimReciveTimout))
                {
                    *mpOutput = tmp;
                }
            }
            // Slave
            else if (mMasterSlave == 1)
            {
                // Read input from socket
                if (mSocketUtility.readSocket(*mpOutput, mSimReciveTimout) == 0)
                {
                    addWarningMessage(HString("Failed to read double from socket before timeout!"));
                    stopSimulation();
                    return;
                }

                // Send ACK or NACK
                if (mRequireACK == 0)
                {
                    if (!sendACKmsg(ACK))
                    {
                        stopSimulation();
                        return;
                    }
                }

                // Write to socket
                if (mSocketUtility.writeSocket(*mpInput) == 0)
                {
                    addErrorMessage(mSocketUtility.getErrorString().c_str());
                }
            }
        }

        void finalize()
        {
            mSocketUtility.closeSocket();
        }

        bool isExperimental() const
        {
            return true;
        }
    };
}

#endif
