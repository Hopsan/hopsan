#ifndef NAMESANITISER_H
#define NAMESANITISER_H

#include <string>

void santizeName(std::string &rString);
std::string santizeName(const std::string &rString);

bool isNameValid(const std::string &rString);
bool isNameValid(const std::string &rString, const std::string &rExceptions);

std::string &replace(std::string &rString, const std::string &rOld, const std::string &rNew);

inline bool contains(const std::string &rString, const std::string &rPattern)
{
    return rString.find(rPattern) != std::string::npos;
}

#endif // NAMESANITISER_H
