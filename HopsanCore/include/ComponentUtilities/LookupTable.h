/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   LookupTable.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-04-12
//!
//! @brief Contains the utility Lookuptable class
//!
//$Id$

#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <vector>
#include <cstring>


template <typename T, size_t numDim>
class LookupTableND
{

public:
    enum IncreasingEnumT {StrictlyIncreasing, StrictlyDecreasing, NotStrictlyIncOrDec, Unknown};

    LookupTableND()
    {
        clear();
    }

    void clear()
    {
        mValueData.clear();
        for (size_t d=0; d<numDim; ++d)
        {
            mIndexData[d].clear();
            mIndexIncreasingOrDecreasing[d] = Unknown;
        }
        memset(&mNumSubDimDataElements, 0, sizeof(T)*numDim);
        resetFirstLast();
    }

    std::vector<double> &getIndexDataRef(size_t d)
    {
        return mIndexData[d];
    }

    std::vector<double> &getValueDataRef()
    {
        return mValueData;
    }

    bool isDataOK()
    {
        size_t num_index=0;
        for (size_t i=0; i<numDim; ++i)
        {
            const size_t sz = mIndexData[i].size();
            if (sz < 2)
            {
                resetFirstLast();
                return false;
            }
            num_index += sz;
            mIndexFirst[i] = mIndexData[i][0];
            mIndexLast[i] = mIndexData[i][sz-1];
        }

        if (num_index == mValueData.size())
        {
            // mNumSubDimDataElements = the number of elements belonging to this dimension and sub dimensions
            // Example: if dim = 0 (row) then numSubDimDataElements in a 3D case will be nCols*nPlanes
            // Example: if dim = 1 (col) then numSubDimDataElements in a 3D case will be nPlanes
            // Example: if dim = 2 (plane) then numSubDimDataElements in a 3D case will be 1
            for (size_t dim=0; dim<numDim; ++dim)
            {
                size_t mNumSubDimDataElements[dim] = 1;
                for (size_t sd=dim+1; sd<mIndexData.size(); ++sd )
                {
                    mNumSubDimDataElements[dim] *= mIndexData[sd].size();
                }
            }

            if (mIndexIncreasingOrDecreasing == Unknown)
            {
                calcIncreasingOrDecreasing();
            }
            return (mIndexIncreasingOrDecreasing == StrictlyIncreasing);
        }
        else
        {
            resetFirstLast();
            return false;
        }
    }

    IncreasingEnumT isIndexIncreasingOrDecresing(size_t d) const
    {
        return mIndexIncreasingOrDecreasing[d];
    }

    void calcIncreasingOrDecreasing(int d=-1)
    {
        size_t start, end;
        if (d<0)
        {
            start = 0;
            end = numDim;
        }
        else
        {
            start = size_t(d);
            end = start+1;
        }

        for (size_t i=start; i<end; ++i)
        {
            mIndexIncreasingOrDecreasing[i] = NotStrictlyIncOrDec;
            const std::vector<T> &indexdata = mIndexData[i]; //Create reference

            if(!mIndexData[i].empty())
            {
                bool increasing=true;
                bool decreasing=true;
                for(size_t row=1; row<indexdata.size(); ++row)
                {
                    if (indexdata[row] > indexdata[row-1])
                    {
                        increasing = increasing && true;
                        decreasing = false;
                    }

                    if (indexdata[row] < indexdata[row-1])
                    {
                        decreasing = decreasing && true;
                        increasing = false;
                    }
                }

                if(increasing)
                {
                    mIndexIncreasingOrDecreasing[i] = StrictlyIncreasing;
                }

                if(decreasing)
                {
                    mIndexIncreasingOrDecreasing[i] = StrictlyDecreasing;
                }
            }
        }
    }

    void sortIncreasing()
    {
        for (size_t i=0; i<numDim; ++i)
        {
            if (mIndexIncreasingOrDecreasing[i] == Unknown)
            {
                calcIncreasingOrDecreasing(i);
            }

            // If row is strictly decreasing the swap row order, else run quicksort and hope for the best
            if (mIndexIncreasingOrDecreasing == StrictlyDecreasing)
            {
                reverseAlongDim();
                isDataOK();
            }
            // Else if not already strictly increasing then sort it
            else if (mIndexIncreasingOrDecreasing == NotStrictlyIncOrDec)
            {
                quickSort(mIndexData, 0, mIndexData.size()-1);
                isDataOK();
            }
            // Else we are already OK
        }
    }

    //! @brief Get a "slice" of data at idx at given dimension
    void getDimDataAt(const size_t dim, const size_t idx, std::vector<T> &rData)
    {
        // mNumSubDimDataElements = the number of elements belonging to this dimension and sub dimensions

        // sizeOfOneSlice = The total number of elements in the slize to extract
        // Example: if dim = 0 (row)   then in a 3D case sizeOfOneSlice = numSubDimDataElements (nCols*nPlanes)
        // Example: if dim = 1 (col)   then in a 3D case sizeOfOneSlice = numSubDimDataElements*nRows (nPlanes*nRows)
        // Example: if dim = 3 (plane) then in a 3D case sizeOfOneSlice = numSubDimDataElements*nCols*nRows (1*nCols*nRows)
        size_t sizeOfOneSlice = mNumSubDimDataElements[dim];
        for (size_t d=dim-1; d>=0; --d)
        {
            sizeOfOneSlice *= mIndexData[d].size();
        }
        rData.resize(sizeOfOneSlice);

        // stepBetweenSlicePartsStarts = The step size between the start of each "part" of a slice
        // Example: if dim = 0 (row)   then in a 3d case step =  numSubDimDataElements * nRows (nCols*nPlanes * nRows) (but not relevant, since will go out of range)
        // Example: if dim = 1 (col)   then in a 3d case step =  numSubDimDataElements * nCols (nPlanes * nCols)
        // Example: if dim = 3 (plane) then in a 3d case step =  numSubDimDataElements * nPlanes (1 * nPlanes)
        size_t stepBetweenSliceParts = mNumSubDimDataElements[dim] * mIndexData[dim].size();

        // Caclulate the start index
        size_t part_start_idx = mNumSubDimDataElements[dim]*idx;

        // Copy data
        size_t ctr=0;
        while (ctr<sizeOfOneSlice)
        {
            // Copy each sub dimension elment
            for (size_t i=0; i<mNumSubDimDataElements[dim]; ++i)
            {
                rData[ctr] = mValueData[part_start_idx+i];
                // Increment counter of how many elements we have copied
                ++ctr;
            }
            // Increment to next part of the slice
            part_start_idx += stepBetweenSliceParts;
        }
    }

    void insertDimDataAt(const size_t dim, const size_t idx, const std::vector<T> &rData)
    {
        // mNumSubDimDataElements = the number of elements belonging to this dimension and sub dimensions

        // sizeOfOneSlice = The total number of elements in the slize to extract
        // Example: if dim = 0 (row)   then in a 3D case sizeOfOneSlice = numSubDimDataElements (nCols*nPlanes)
        // Example: if dim = 1 (col)   then in a 3D case sizeOfOneSlice = numSubDimDataElements*nRows (nPlanes*nRows)
        // Example: if dim = 3 (plane) then in a 3D case sizeOfOneSlice = numSubDimDataElements*nCols*nRows (1*nCols*nRows)
        size_t sizeOfOneSlice = mNumSubDimDataElements[dim];
        for (size_t d=dim-1; d>=0; --d)
        {
            sizeOfOneSlice *= mIndexData[d].size();
        }
        rData.resize(sizeOfOneSlice);

        // stepBetweenSlicePartsStarts = The step size between the start of each "part" of a slice
        // Example: if dim = 0 (row)   then in a 3d case step =  numSubDimDataElements * nRows (nCols*nPlanes * nRows) (but not relevant, since will go out of range)
        // Example: if dim = 1 (col)   then in a 3d case step =  numSubDimDataElements * nCols (nPlanes * nCols)
        // Example: if dim = 3 (plane) then in a 3d case step =  numSubDimDataElements * nPlanes (1 * nPlanes)
        size_t stepBetweenSliceParts = mNumSubDimDataElements[dim] * mIndexData[dim].size();

        // Caclulate the start index
        size_t part_start_idx = mNumSubDimDataElements[dim]*idx;

        // Insert data
        size_t ctr=0;
        while (ctr<sizeOfOneSlice)
        {
            // Copy each sub dimension elment
            for (size_t i=0; i<mNumSubDimDataElements[dim]; ++i)
            {
                mValueData[part_start_idx+i] = rData[ctr];
                // Increment counter of how many elements we have copied
                ++ctr;
            }
            // Increment to next part of the slice
            part_start_idx += stepBetweenSliceParts;
        }
    }

    double interpolate1d(const double x, const size_t dim) const
    {
        std::vector<T> &rIndexData = mIndexData[dim];
        size_t idx = findIndexAlongDim(x, dim);
        if(rIndexData[idx+1] ==  rIndexData[idx])       //Check for division by zero (this means that if several X values have the same value, we will always pick the first one since we cannot interpolate between them)
        {
            return mValueData[idx];
        }
        else
        {
            return mValueData[idx] + (x - mIndexData[idx])*(mValueData[idx+1] -  mValueData[idx])/(rIndexData[idx+1] -  rIndexData[idx]);
        }
    }

    size_t getDimSize(const size_t dim) const
    {
        return mIndexData[dim].size();
    }

    size_t findIndexAlongDim(const T x, const size_t dim)
    {
        // Handle outside index range
        if( x<mIndexFirst[dim] )
        {
            return 0;
        }
        // Handle outside index range
        else if( x>=mIndexLast[dim] )
        {
            return mIndexData[dim].size()-1;
        }
        // Handle within range
        else
        {
            return intervalHalfSubDiv(x, 0, mIndexData[dim].size()-1, dim);
        }
    }

protected:
    size_t intervalHalfSubDiv(const double x, const size_t i1, const size_t iend, const size_t dim) const
    {
        if (iend-i1 <= 1)
        {
            // When the two indexes are next to each other lets return the smallest one as the start row for interpolation
            return i1;
        }
        else
        {
            //Calc split index
            size_t splitIdx = i1 + (iend - i1)/2; //Allow truncation

            if (x <= mIndexData[dim][splitIdx])
            {
                // Use lower half
                return intervalHalfSubDiv(x, i1, splitIdx, dim);
            }
            else
            {
                // Use higher half
                return intervalHalfSubDiv(x, splitIdx, iend, dim);
            }
        }
    }

    size_t quickSortPartition( const size_t dim, const std::vector<T> &rIndexArray, const size_t left, const size_t right, const size_t pivotIndex)
    {
        // left is the index of the leftmost element of the array
        // right is the index of the rightmost element of the array (inclusive)
        // number of elements in subarray = right-left+1

        double pivotValue = rIndexArray[pivotIndex];
        swapDataSliceInDim(pivotIndex, right, dim);  // Move pivot to end
        size_t storeIndex = left;
        for (size_t i=left; i<right; ++i)  // left ≤ i < right
        {
            if (rIndexArray[i] < pivotValue)
            {
                swapDataSliceInDim(i, storeIndex, dim);
                storeIndex = storeIndex + 1;
            }
        }
        swapDataSliceInDim(storeIndex, right, dim); // Move pivot to its final place
        return storeIndex;
    }

    void quickSort(const size_t dim,  const std::vector<T> &rIndexArray, const size_t left, const size_t right)
    {
        // If the list has 2 or more items
        if (left < right)
        {
            // See "Choice of pivot" section below for possible choices
            //choose any 'pivotIndex' such that 'left' ≤ 'pivotIndex' ≤ 'right'
            size_t pivotIndex = left+(right-left)/2;

            // Get lists of bigger and smaller items and final position of pivot
            size_t pivotNewIndex = quickSortPartition(dim, rIndexArray, left, right, pivotIndex);

            // Recursively sort elements smaller than the pivot
            // but not if it happend to be zero (would lead to underflow in size_t)
            if (pivotNewIndex>0)
            {
                quickSort(dim, rIndexArray, left, pivotNewIndex-1);
            }

            // Recursively sort elements at least as big as the pivot
            quickSort(dim, rIndexArray, pivotNewIndex+1, right);
        }
    }

    void swapDataSliceInDim(const size_t r1, const size_t r2, const size_t dim)
    {
        std::vector<T> &rIndexdata = mIndexData[dim]; // Get reference to desired index vector

        // Swap index
        T tmp = rIndexdata[r1];
        rIndexdata[r1] = rIndexdata[r2];
        rIndexdata[r2] = tmp;

        // Swap data
        if (numDim == 1)
        {
            tmp = mValueData[r1];
            mValueData[r1] = mValueData[r2];
            mValueData[r2] = tmp;
        }
        else
        {
            std::vector<T> slice1, slice2;
            getDimDataAt(dim, r1, slice1);
            getDimDataAt(dim, r2, slice2);

            insertDimDataAt(dim, r1, slice2);
            insertDimDataAt(dim, r2, slice1);
        }

    }

    void reverseAlongDim(size_t d)
    {
        //! @todo does not yet work for dim >1
        if (numDim < 2)
        {
            typename std::vector<T>::reverse_iterator rit;
            std::vector<T> tempData;

            std::vector<T> &rIndexdata = mIndexData[d]; // Get reference to desired index vector

            // Reverse the index data
            tempData.reserve(rIndexdata.size());
            for (rit=rIndexdata.rbegin(); rit!=rIndexdata.rend(); ++rit)
            {
                tempData.push_back(*rit);
            }
            rIndexdata.swap(tempData);

            // Reverse the value data
            //! @todo does not yet work for dim >=3
            tempData.clear();
            tempData.reserve(mValueData.size());
            if (numDim == 1)
            {
                for (rit=mValueData.rbegin(); rit!=mValueData.rend(); ++rit)
                {
                    tempData.push_back(*rit);
                }
                mValueData.swap(tempData);
            }
            else
            {

            }
        }
    }

    void resetFirstLast()
    {
        memset(&mIndexFirst, 0, sizeof(T)*numDim);
        memset(&mIndexLast, 1, sizeof(T)*numDim);
    }

    inline size_t calcDataIndex(const std::vector<size_t> coordinates)
    {
        size_t ind=0;
        for (size_t i=0; i<coordinates.size(); ++i)
        {
            ind += coordinates[i]*mNumSubDimDataElements[i];
        }
        return ind;
    }

    T mNumSubDimDataElements[numDim];
    T mIndexFirst[numDim];
    T mIndexLast[numDim];
    IncreasingEnumT mIndexIncreasingOrDecreasing[numDim];

    std::vector<T> mIndexData[numDim];
    std::vector<T> mValueData;

};

template<typename T>
class LookupTableND<T,1>
{
public:
    inline size_t calcDataIndex(const size_t r) const
    {
        return r;
    }
};


template<typename T>
class LookupTableND<T,2>
{
public:
    inline size_t calcDataIndex(const size_t r, const size_t c) const
    {
        return r*LookupTableND<T,2>::mNumSubDimDataElements[0] + c;
    }
};

template<typename T>
class LookupTableND<T,3>
{
public:
    inline size_t calcDataIndex(const size_t r, const size_t c, const size_t p) const
    {
        return r*LookupTableND<T,3>::mNumSubDimDataElements[0] + c*LookupTableND<T,3>::mNumSubDimDataElements[1] + p;
    }
};


class LookupTable1DNonTemplate
{
public:
    enum IncreasingEnumT {StrictlyIncreasing, StrictlyDecreasing, NotStrictlyIncOrDec, Unknown};

    LookupTable1DNonTemplate();
    void clear();

    std::vector<double> &getIndexDataRef();
    std::vector<double> &getValueDataRef();

    bool isDataOK();

    IncreasingEnumT isIndexIncreasingOrDecresing() const;
    IncreasingEnumT calcIncreasingOrDecreasing();
    void sortIncreasing();

    double interpolate(const double x) const;

protected:
    size_t intervalHalfSubDiv(const double x, const size_t i1, const size_t iend) const;
    size_t quickSortPartition(const std::vector<double> &rIndexArray, const size_t left, const size_t right, const size_t pivotIndex);
    void quickSort(const std::vector<double> &rIndexArray, const size_t left, const size_t right);
    void swapRows(const size_t r1, const size_t r2);
    void reverseRows();

    std::vector<double> mIndexData;
    std::vector<double> mValueData;
    double mFirstIndex, mFirstValue, mLastIndex, mLastValue;
    IncreasingEnumT mIndexIncreasingOrDecreasing;
};

class LookupTable2DNonTemplate : public LookupTable1DNonTemplate
{
public:
    LookupTable2DNonTemplate();

    double interpolate2D(const double x1, const double x2) const;

};





#endif // LOOKUPTABLE_H
