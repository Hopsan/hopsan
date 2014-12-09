#ifndef MODELVALIDATION_H
#define MODELVALIDATION_H

#include <string>

bool performModelTest(const std::string hvcFilePath);
bool createModelTestDataSet(const std::string modelPath, const std::string hvcFilePath);

#endif // MODELVALIDATION_H
