#ifndef COMPILINGUTILITIES_H
#define COMPILINGUTILITIES_H

#include <QStringList>
#include <QFileInfo>

QStringList compileComponentLibrary(const QString &compilerPath, const QString &path, const QString &target, const QStringList &sourceFiles, const QStringList &libs, const QStringList &includeDirs, bool &success);

//! @brief Calls GCC or MinGW compiler with specified parameters
//! @param path Absolute path where compiler shall be run
//! @param o Objective file name (without file extension)
//! @param c List with source files, example: "file1.cpp file2.cc"
//! @param i Include command, example: "-Ipath1 -Ipath2"
//! @param l Link command, example: "-Lpath1 -lfile1 -lfile2"
//! @param flags Compiler flags
//! @param output Reference to string where output messages are stored
bool compile(const QString &compilerPath, const QString &path, const QString &o, const QString &c, const QString &i, const QString &l, const QString &flags, QStringList &output);

#endif // COMPILINGUTILITIES_H
