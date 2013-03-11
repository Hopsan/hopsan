#ifndef NAMESANITISER_H
#define NAMESANITISER_H

#include <string>

void santizeName(std::string &rString);
std::string santizeName(const std::string &rString);

bool isNameValid(const std::string &rString);
bool isNameValid(const std::string &rString, const std::string &rExceptions);

#endif // NAMESANITISER_H
