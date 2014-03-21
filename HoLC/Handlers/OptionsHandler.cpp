#include "OptionsHandler.h"

OptionsHandler::OptionsHandler()
{
}

void OptionsHandler::setLibPath(const QString path)
{
    mLibPath = path;
}

void OptionsHandler::setIncludePath(const QString path)
{
    mIncludePath = path;
}

void OptionsHandler::setCompilerPath(const QString path)
{
    mCompilerPath = path;
}

QString OptionsHandler::getLibPath() const
{
    return mLibPath;
}

QString OptionsHandler::getIncludePath() const
{
   return mIncludePath;
}

QString OptionsHandler::getCompilerPath() const
{
    return mCompilerPath;
}
