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

#ifndef STATUSINFOSTRUCTS_H
#define STATUSINFOSTRUCTS_H

#include<string>

class ServerStatusT
{
public:
    std::string services;
    std::string users;
    int numFreeSlots;
    int numTotalSlots;
    std::string startTime;
    std::string stopTime;
    bool isReady;
};

class WorkerStatusT
{
public:
    bool model_loaded=false;
    bool simulation_inprogress=false;
    bool simualtion_success=false;
    bool simulation_finished=false;
    double current_simulation_time=-1;
    double simulation_progress=-1;
    double estimated_simulation_time_remaining=-1;
    bool shell_inprogress=false;
    bool shell_exitok=false;
};

class ServerMachineInfoT
{
public:
    std::string address;
    std::string relayaddress;
    std::string description;
    int numslots;
    double evalTime;
};

#endif // STATUSINFOSTRUCTS_H
