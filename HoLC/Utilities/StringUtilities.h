#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include <QString>

QString spaces(const int n);
void replacePatternLine(QString &rData, const QString &rPattern, const QString &rString);
QString firstCapital(QString str);

#endif // STRINGUTILITIES_H
