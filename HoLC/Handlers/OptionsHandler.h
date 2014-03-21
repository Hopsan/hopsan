#ifndef OPTIONSHANDLER_H
#define OPTIONSHANDLER_H

#include <QString>

class OptionsHandler
{
public:
    OptionsHandler();
    void setLibPath(const QString path);
    void setIncludePath(const QString path);
    void setCompilerPath(const QString path);

    QString getLibPath() const;
    QString getIncludePath() const;
    QString getCompilerPath() const;

private:
    QString mLibPath, mIncludePath, mCompilerPath;
};

#endif // OPTIONSHANDLER_H
