/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#ifndef INDEXINTERVALCOLLECTION_H
#define INDEXINTERVALCOLLECTION_H

#include <QList>

class IndexIntervalCollection
{
public:
    class MinMaxT
    {
    public:
        MinMaxT(int min, int max);
        int mMin, mMax;
    };

    void addValue(const int val);
    void removeValue(const int val);
    int min() const;
    int max() const;
    bool isContinuos() const;
    bool isEmpty() const;
    bool contains(const int val) const;
    void clear();

    QList<MinMaxT> getList() const;
    QList<int> getCompleteList() const;
    int getNumIIC() const;
    int getNumI() const;

    void testMe();

private:
    void mergeIntervals(int first, int second);
    QList<MinMaxT> mIntervalList;
};

#endif // INDEXINTERVALCOLLECTION_H
