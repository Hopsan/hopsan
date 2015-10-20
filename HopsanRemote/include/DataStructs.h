#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include<string>

typedef struct
{
    std::string name;
    std::string alias;
    std::string quantity;
    std::string unit;
    std::vector<double> data;
}ResultVariableT;

#endif // DATASTRUCTS_H
