/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

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

bool checkVariableOrClassName(QString str)
{
    QString ret;
    if (!str.isEmpty())
    {
        // Reserve memory for entire string (we will only append as many chars as we decide to keep)
        ret.reserve(str.size());
        // First ignore any non letter or number or underscore char in the beginning
        int c=0;
        if(str.size() == 0)
        {
            return false;
        }
        else if (!str[c].isLetter())
        {
            return false;
        }
        ++c;
        while ( c < str.size() )
        {
            if  ( !(str[c].isLetterOrNumber()) && !(str[c]=='_') )
            {
                return false;
            }
            ++c;
        }
    }
    return true;
}
