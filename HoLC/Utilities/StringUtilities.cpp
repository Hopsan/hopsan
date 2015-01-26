#include "StringUtilities.h"
#include <QRegExp>

QString spaces(const int n)
{
    QString str(" ");
    return str.repeated(n);
}

void replacePatternLine(QString &rData, const QString &rPattern, const QString &rString)
{
    QRegExp rx("[^\\n]*"+rPattern+"[^\\n]*\\n");
    // The rx first matches anything but newline preceding the pattern (meaning it should stay on the pattern line)
    // The match also swallows any non-newline characters after the pattern until it encounters and swallows one newline character.
    // double escaping is needed since a QString is created first (due to "str"+pattern+"str")

    if (rString.isEmpty())
    {
        // Remove the pattern line
        rData.remove(rx);
    }
    else
    {
        // Replace the pattern with rString
        rData.replace(rx, rString);
    }
}


QString firstCapital(QString str)
{
    if (!str.isEmpty())
    {
        str[0] = str[0].toUpper();
    }
    return str;
}
