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

#include "IndexIntervalCollection.h"

#include <QDebug>

void IndexIntervalCollection::addValue(const int val)
{
    if (mIntervalList.isEmpty())
    {
        mIntervalList.append(MinMaxT(val,val));
    }
    else
    {
        for(int i=0; i<mIntervalList.size(); ++i)
        {
            if (val < mIntervalList[i].mMin)
            {
                // Check if we should extend downwards
                if (val ==  mIntervalList[i].mMin-1)
                {
                    mIntervalList[i].mMin = val;
                }
                // If not then we should add a new one
                // We do not need to check if we should merge here as that should have been
                // handled while processing the previous intervals
                else
                {
                    mIntervalList.insert(i, MinMaxT(val,val));
                }
                break;
            }
            else if (val > mIntervalList[i].mMax)
            {
                // Check if we should extend upwards
                if (val ==  mIntervalList[i].mMax+1)
                {
                    mIntervalList[i].mMax = val;
                    //! @todo check merge FIXA /Peter
                    // If we have an other interval after ours, then lets check if we should merge them
                    if (i+1<mIntervalList.size())
                    {
                        // Check if we should merge
                        if (mIntervalList[i+1].mMin == val+1)
                        {
                            mergeIntervals(i,i+1);
                        }
                    }
                    break;
                }
                // Add new, if this is the last one
                // else this is handled by the less then check above
                else if (i+1 >= mIntervalList.size())
                {
                    mIntervalList.insert(i+1, MinMaxT(val,val));
                    break;
                }

                // Else keep checking the next iic
            }
            //! @todo need to check and merge intervals after adding
            // If non of the above were triggered then the value was within an already existing interval
        }
    }
}

void IndexIntervalCollection::removeValue(const int val)
{
    for(int i=0; i<mIntervalList.size(); ++i)
    {
        if ((val >= mIntervalList[i].mMin) && (val <= mIntervalList[i].mMax))
        {
            // If interval is singular then remove it
            if (mIntervalList[i].mMin == mIntervalList[i].mMax)
            {
                mIntervalList.removeAt(i);
            }
            // If val = min then shrink it
            else if (mIntervalList[i].mMin == val)
            {
                mIntervalList[i].mMin += 1;
            }
            // If val = max than shrink it
            else if (mIntervalList[i].mMax == val)
            {
                mIntervalList[i].mMax -= 1;
            }
            // Else split the interval
            else
            {
                MinMaxT newInterv(val+1, mIntervalList[i].mMax);
                mIntervalList[i].mMax = val-1;
                mIntervalList.insert(i+1,newInterv);
            }
            // Since we took care of the value we can break the loop
            break;
        }
    }
}

int IndexIntervalCollection::min() const
{
    if (mIntervalList.isEmpty())
    {
        return -1;
    }
    return mIntervalList.first().mMin;
}

int IndexIntervalCollection::max() const
{
    if (mIntervalList.isEmpty())
    {
        return -1;
    }
    return mIntervalList.last().mMax;
}

QList<IndexIntervalCollection::MinMaxT> IndexIntervalCollection::getList() const
{
    return mIntervalList;
}

QList<int> IndexIntervalCollection::getCompleteList() const
{
    QList<int> complete;
    for (int i=0; i<mIntervalList.size(); ++i)
    {
        for (int j=mIntervalList[i].mMin; j<=mIntervalList[i].mMax; ++j)
        {
            complete.append(j);
        }
    }
    return complete;
}

int IndexIntervalCollection::getNumIIC() const
{
    return mIntervalList.size();
}

int IndexIntervalCollection::getNumI() const
{
    int ctr=0;
    for (int ic=0; ic<mIntervalList.size(); ++ic)
    {
        ctr += (mIntervalList[ic].mMax - mIntervalList[ic].mMin) + 1;
    }
    return ctr;
}

void IndexIntervalCollection::testMe()
{
    addValue(1); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", " << getCompleteList();
    addValue(3); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(0); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(-2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(4); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(6); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(10); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(15); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(12); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    removeValue(4); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    removeValue(2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

    addValue(4); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();
    addValue(2); qDebug() << "IIC: " << getNumIIC() << "nI: " << getNumI() << ", "  << getCompleteList();

}

void IndexIntervalCollection::mergeIntervals(int first, int second)
{
    // We assume that the intervals to merge are actually continuous neighbors
    mIntervalList[first].mMax = mIntervalList[second].mMax;
    mIntervalList.removeAt(second);
}

bool IndexIntervalCollection::isContinuos() const
{
    return (mIntervalList.size() == 1);
}

bool IndexIntervalCollection::isEmpty() const
{
    return mIntervalList.isEmpty();
}

bool IndexIntervalCollection::contains(const int val) const
{
    for(int i=0; i<mIntervalList.size(); ++i)
    {
        if ((val >= mIntervalList[i].mMin) && (val <= mIntervalList[i].mMax))
        {
            return true;
        }
    }
    return false;
}

void IndexIntervalCollection::clear()
{
    mIntervalList.clear();
}


IndexIntervalCollection::MinMaxT::MinMaxT(int min, int max)
{
    mMin = min;
    mMax = max;
}

