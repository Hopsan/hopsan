#ifndef SERVERSTATUSMESSAGE_H
#define SERVERSTATUSMESSAGE_H

#include<string>

typedef struct
{
    int numFreeSlots;
    int numTotalSlots;
    std::string startTime;
    std::string stopTime;
    bool isReady;
}ServerStatusT;

typedef struct
{
    bool model_loaded=false;
    bool simulation_inprogress=false;
    bool simualtion_success=false;
    bool simulation_finished=false;
    double current_simulation_time=-1;
    double simulation_progress=-1;
    double estimated_simulation_time_remaining=-1;
}WorkerStatusT;

#endif // SERVERSTATUSMESSAGE_H
