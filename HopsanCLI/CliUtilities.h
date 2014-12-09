#ifndef CLIUTILITIES_H
#define CLIUTILITIES_H

#include <string>
#include <vector>
#include <sstream>

// ===== Help Functions for String Paths =====
void splitFilePath(const std::string fullPath, std::string &rBasePath, std::string &rFileName);
void splitFileName(const std::string fileName, std::string &rBaseName, std::string &rExt);
void splitStringOnDelimiter(const std::string &rString, const char delim, std::vector<std::string> &rSplitVector);
std::string relativePath(std::string basePath, std::string fullPath);

template <typename T>
std::string to_string(T val)
{
    std::stringstream ss;
    ss << val;
    return ss.str();
}

// ===== Print functions =====
enum ColorsEnumT {Red, Green, Blue, Yellow, White, Reset};
void printErrorMessage(const std::string &rError);
void printWarningMessage(const std::string &rWarning);
void printColorMessage(const ColorsEnumT color, const std::string &rMessage);
void setTerminalColor(const ColorsEnumT color);

// ===== Sys Functions =====
size_t getNumAvailibleCores();

// ===== Data Functions =====
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol);

// ===== Read File Functions =====
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames);

#endif // CLIUTILITIES_H
