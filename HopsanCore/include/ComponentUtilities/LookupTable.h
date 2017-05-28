/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

inline double interp1(const double x, const double i1, const double i2, const double v1, const double v2)
{
    return v1 + (x-i1)*(v2-v1)/(i2-i1);
}

class LookupTableNDBase
{

public:
    enum IncreasingEnumT {StrictlyIncreasing, StrictlyDecreasing, NotStrictlyIncOrDec, Unknown};

    LookupTableNDBase(const size_t nDims)
    {
        mNumDims = nDims;
        clear();
    }

    void clear()
    {
        mValueData.clear();
        mIndexData.clear(); mIndexData.resize(mNumDims);
        mNumSubDimDataElements.clear(); mNumSubDimDataElements.resize(mNumDims, 0);
        mIndexIncreasingOrDecreasing.clear(); mIndexIncreasingOrDecreasing.resize(mNumDims, Unknown);
        resetFirstLast();
    }

    bool isEmpty() const
    {
        return mValueData.empty();
    }

    std::vector<double> &getIndexDataRef(const size_t d)
    {
        return mIndexData[d];
    }

    std::vector<double> &getValueDataRef()
    {
        return mValueData;
    }

    bool isDataSizeOK()
    {
        size_t num_index=1;
        for (size_t d=0; d<mNumDims; ++d)
        {
            const size_t sz = mIndexData[d].size();
            if (sz < 2)
            {
                resetFirstLast();
                return false;
            }
            num_index *= sz;
            mIndexFirst[d] = mIndexData[d][0];
            mIndexLast[d] = mIndexData[d][sz-1];
        }

        return (num_index == mValueData.size());
    }

    bool isDataOK()
    {
        if (isDataSizeOK())
        {
            // mNumSubDimDataElements = the number of elements belonging to this dimension and sub dimensions
            // Example: if dim = 0 (row) then numSubDimDataElements in a 3D case will be nCols*nPlanes
            // Example: if dim = 1 (col) then numSubDimDataElements in a 3D case will be nPlanes
            // Example: if dim = 2 (plane) then numSubDimDataElements in a 3D case will be 1
            for (size_t dim=0; dim<mNumDims; ++dim)
            {
                mNumSubDimDataElements[dim] = 1;
                for (size_t sd=dim+1; sd<mNumDims; ++sd )
                {
                    mNumSubDimDataElements[dim] *= mIndexData[sd].size();
                }
            }

            bool isStrictlyInc=true;
            for (size_t d=0; d<mNumDims; ++d)
            {
                if (mIndexIncreasingOrDecreasing[d] == Unknown)
                {
                    calcIncreasingOrDecreasing(int(d));
                }

                isStrictlyInc = isStrictlyInc && (mIndexIncreasingOrDecreasing[d] == StrictlyIncreasing);
            }
            return isStrictlyInc;
        }
        else
        {
            resetFirstLast();
            return false;
        }
    }

    bool allIndexStrictlyIncreasing() const
    {
        for (size_t d=0; d<mNumDims; ++d)
        {
            if (mIndexIncreasingOrDecreasing[d] != StrictlyIncreasing)
            {
                return false;
            }
        }
        return true;
    }

    IncreasingEnumT isIndexIncreasingOrDecresing(size_t d) const
    {
        return mIndexIncreasingOrDecreasing[d];
    }

    void calcIncreasingOrDecreasing(int dim=-1)
    {
        size_t start, end;
        if (dim<0)
        {
            start = 0;
            end = mNumDims;
        }
        else
        {
            start = size_t(dim);
            end = start+1;
        }

        for (size_t d=start; d<end; ++d)
        {
            mIndexIncreasingOrDecreasing[d] = NotStrictlyIncOrDec;
            const std::vector<double> &rIndexData = mIndexData[d]; //Create reference

            if(!rIndexData.empty())
            {
                bool increasing=true;
                bool decreasing=true;
                for(size_t row=1; row<rIndexData.size(); ++row)
                {
                    if (rIndexData[row] > rIndexData[row-1])
                    {
                        increasing = increasing && true;
                        decreasing = false;
                    }

                    if (rIndexData[row] < rIndexData[row-1])
                    {
                        decreasing = decreasing && true;
                        increasing = false;
                    }

                    if (rIndexData[row] == rIndexData[row-1])
                    {
                        increasing = false;
                        decreasing = false;
                    }
                }

                if(increasing)
                {
                    mIndexIncreasingOrDecreasing[d] = StrictlyIncreasing;
                }
                else if(decreasing)
                {
                    mIndexIncreasingOrDecreasing[d] = StrictlyDecreasing;
                }
                else
                {
                    mIndexIncreasingOrDecreasing[d] = NotStrictlyIncOrDec;
                }
            }
        }
    }

    void sortIncreasing()
    {
        for (size_t d=0; d<mNumDims; ++d)
        {
            if (mIndexIncreasingOrDecreasing[d] == Unknown)
            {
                calcIncreasingOrDecreasing(int(d));
            }

            // If row is strictly decreasing the swap row order, else run quicksort and hope for the best
            if (mIndexIncreasingOrDecreasing[d] == StrictlyDecreasing)
            {
                reverseAlongDim(d);
                mIndexIncreasingOrDecreasing[d] = Unknown;
                isDataOK();
            }
            // Else if not already strictly increasing then sort it
            else if (mIndexIncreasingOrDecreasing[d] == NotStrictlyIncOrDec)
            {
                quickSort(d, mIndexData[d], 0, mIndexData[d].size()-1);
                mIndexIncreasingOrDecreasing[d] = Unknown;
                isDataOK();
            }
            // Else we are already OK
        }
    }

    //! @brief Get a "slice" of data at idx at given dimension
    void getDimDataAt(const size_t dim, const size_t idx, std::vector<double> &rData)
    {
        // mNumSubDimDataElements = the number of elements belonging to this dimension and sub dimensions

        // sizeOfOneSlice = The total number of elements in the slize to extract
        // Example: if dim = 0 (row)   then in a 3D case sizeOfOneSlice = numSubDimDataElements (nCols*nPlanes)
        // Example: if dim = 1 (col)   then in a 3D case sizeOfOneSlice = numSubDimDataElements*nRows (nPlanes*nRows)
        // Example: if dim = 3 (plane) then in a 3D case sizeOfOneSlice = numSubDimDataElements*nCols*nRows (1*nCols*nRows)
        size_t sizeOfOneSlice = mNumSubDimDataElements[dim];
        for (int d=int(dim)-1; d>=0; --d)
        {
            sizeOfOneSlice *= mIndexData[d].size();
        }
        rData.resize(sizeOfOneSlice);

        // stepBetweenSlicePartsStarts = The step size between the start of each "part" of a slice
        // Example: if dim = 0 (row)   then in a 3d case step =  numSubDimDataElements * nRows (nCols*nPlanes * nRows) (but not relevant, since will go out of range)
        // Example: if dim = 1 (col)   then in a 3d case step =  numSubDimDataElements * nCols (nPlanes * nCols)
        // Example: if dim = 3 (plane) then in a 3d case step =  numSubDimDataElements * nPlanes (1 * nPlanes)
        size_t stepBetweenSliceParts = mNumSubDimDataElements[dim] * mIndexData[dim].size();

        // Calculate the start index
        size_t part_start_idx = mNumSubDimDataElements[dim]*idx;

        // Copy data
        size_t ctr=0;
        while (ctr<sizeOfOneSlice)
        {
            // Copy each sub dimension element
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

    void insertDimDataAt(const size_t dim, const size_t idx, const std::vector<double> &rData)
    {
        // mNumSubDimDataElements = the number of elements belonging to this dimension and sub dimensions

        // sizeOfOneSlice = The total number of elements in the slice to extract
        // Example: if dim = 0 (row)   then in a 3D case sizeOfOneSlice = numSubDimDataElements (nCols*nPlanes)
        // Example: if dim = 1 (col)   then in a 3D case sizeOfOneSlice = numSubDimDataElements*nRows (nPlanes*nRows)
        // Example: if dim = 3 (plane) then in a 3D case sizeOfOneSlice = numSubDimDataElements*nCols*nRows (1*nCols*nRows)
        size_t sizeOfOneSlice = mNumSubDimDataElements[dim];
        for (int d=int(dim)-1; d>=0; --d)
        {
            sizeOfOneSlice *= mIndexData[d].size();
        }

        // stepBetweenSlicePartsStarts = The step size between the start of each "part" of a slice
        // Example: if dim = 0 (row)   then in a 3d case step =  numSubDimDataElements * nRows (nCols*nPlanes * nRows) (but not relevant, since will go out of range)
        // Example: if dim = 1 (col)   then in a 3d case step =  numSubDimDataElements * nCols (nPlanes * nCols)
        // Example: if dim = 3 (plane) then in a 3d case step =  numSubDimDataElements * nPlanes (1 * nPlanes)
        size_t stepBetweenSliceParts = mNumSubDimDataElements[dim] * mIndexData[dim].size();

        // Calculate the start index
        size_t part_start_idx = mNumSubDimDataElements[dim]*idx;

        // Insert data
        size_t ctr=0;
        while (ctr<sizeOfOneSlice)
        {
            // Copy each sub dimension element
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

    size_t getDimSize(const size_t dim) const
    {
        return mIndexData[dim].size();
    }

    //! @note Assumes that x is within index range
    size_t findIndexAlongDim(const size_t dim, const double x) const
    {
        return intervalHalfSubDiv(x, 0, mIndexData[dim].size()-1, dim);
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

    size_t quickSortPartition( const size_t dim, const std::vector<double> &rIndexArray, const size_t left, const size_t right, const size_t pivotIndex)
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

    void quickSort(const size_t dim, const std::vector<double> &rIndexArray, const size_t left, const size_t right)
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
            // but not if it happened to be zero (would lead to underflow in size_t)
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
        std::vector<double> &rIndexdata = mIndexData[dim]; // Get reference to desired index vector

        // Swap index
        double tmp = rIndexdata[r1];
        rIndexdata[r1] = rIndexdata[r2];
        rIndexdata[r2] = tmp;

        // Swap data
        if (mNumDims == 1)
        {
            tmp = mValueData[r1];
            mValueData[r1] = mValueData[r2];
            mValueData[r2] = tmp;
        }
        else
        {
            std::vector<double> slice1, slice2;
            getDimDataAt(dim, r1, slice1);
            getDimDataAt(dim, r2, slice2);

            insertDimDataAt(dim, r1, slice2);
            insertDimDataAt(dim, r2, slice1);
        }

    }

    void reverseAlongDim(size_t d)
    {
        //! @todo does not yet work for dim >1
        if (mNumDims < 2)
        {
            std::vector<double>::reverse_iterator rit;
            std::vector<double> tempData;

            std::vector<double> &rIndexdata = mIndexData[d]; // Get reference to desired index vector

            // Reverse the index data
            tempData.reserve(rIndexdata.size());
            for (rit=rIndexdata.rbegin(); rit!=rIndexdata.rend(); ++rit)
            {
                tempData.push_back(*rit);
            }
            rIndexdata.swap(tempData);

            // Reverse the value data
            tempData.clear();
            tempData.reserve(mValueData.size());
            for (rit=mValueData.rbegin(); rit!=mValueData.rend(); ++rit)
            {
                tempData.push_back(*rit);
            }
            mValueData.swap(tempData);
        }
        else
        {
            quickSort(d, mIndexData[d], 0, mIndexData[d].size()-1);
        }
    }

    void resetFirstLast()
    {
        mIndexFirst.clear(); mIndexFirst.resize(mNumDims, 0);
        mIndexLast.clear(); mIndexLast.resize(mNumDims, 1);
    }

    inline double limitToRange(const size_t dim, const double val) const
    {
        if (val < mIndexFirst[dim])
        {
            return mIndexFirst[dim];
        }
        else if(val > mIndexLast[dim])
        {
            return mIndexLast[dim];
        }
        return val;
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

    size_t mNumDims;

    std::vector<size_t> mNumSubDimDataElements;
    std::vector<double> mIndexFirst;
    std::vector<double> mIndexLast;
    std::vector<IncreasingEnumT> mIndexIncreasingOrDecreasing;

    std::vector< std::vector<double> > mIndexData;
    std::vector<double> mValueData;
};

class LookupTable1D : public LookupTableNDBase
{
public:
    LookupTable1D() : LookupTableNDBase(1) {}

    inline size_t calcDataIndex(const size_t r) const
    {
        return r;
    }

    std::vector<double> &getIndexDataRef()
    {
        return mIndexData[0];
    }

    double interpolate(const double x) const
    {
        // Handle outside minimum index range
        if( x<mIndexFirst[0] )
        {
            return mValueData[0];
        }
        // Handle outside maximum index range
        else if( x>=mIndexLast[0] )
        {
            return mValueData[mValueData.size()-1];
        }
        // Handle in range
        {
            const std::vector<double> &rIndexData = mIndexData[0];
            const size_t idx = findIndexAlongDim(0, x);

            // Note, assumes that index data is strictly increasing (two values can not be the same). That will lead to division by zero here
            return mValueData[idx] + (x - rIndexData[idx])*(mValueData[idx+1] -  mValueData[idx])/(rIndexData[idx+1] -  rIndexData[idx]);
        }
    }
};


class LookupTable2D : public LookupTableNDBase
{
public:
    LookupTable2D() : LookupTableNDBase(2) {}

    inline size_t calcDataIndex(const size_t r, const size_t c) const
    {
        return r*mNumSubDimDataElements[0] + c;
    }

    double interpolate(double r, double c) const
    {
        // Handle outside index range
        r = limitToRange(0, r);
        c = limitToRange(1, c);

        const size_t tl_r = findIndexAlongDim(0, r);
        const size_t tr_r = tl_r;
        const size_t tl_c = findIndexAlongDim(1, c);
        const size_t bl_c = tl_c;

        const size_t tr_c = tl_c+1;
        const size_t br_c = tr_c;
        const size_t bl_r = tl_r+1;
        const size_t br_r = bl_r;

        const double tl_v = mValueData[calcDataIndex(tl_r, tl_c)];
        const double tr_v = mValueData[calcDataIndex(tr_r, tr_c)];
        const double bl_v = mValueData[calcDataIndex(bl_r, bl_c)];
        const double br_v = mValueData[calcDataIndex(br_r, br_c)];

        // Note, interp1 assumes that index data is strictly increasing (two values can not be the same). That will lead to division by zero here
        const double val_l = interp1(r, mIndexData[0][tl_r], mIndexData[0][bl_r], tl_v, bl_v);
        const double val_r = interp1(r, mIndexData[0][tr_r], mIndexData[0][br_r], tr_v, br_v);

        return interp1(c, mIndexData[1][tl_c], mIndexData[1][tr_c], val_l, val_r);
    }
};


class LookupTable3D : public LookupTableNDBase
{
public:
    LookupTable3D() : LookupTableNDBase(3) {}

    inline size_t calcDataIndex(const size_t r, const size_t c, const size_t p) const
    {
        return r*mNumSubDimDataElements[0] + c*mNumSubDimDataElements[1] + p;
    }

    double interpolate(double r, double c, double p) const
    {
        // Handle outside index range
        r = limitToRange(0, r);
        c = limitToRange(1, c);
        p = limitToRange(2, p);

        // Find planes, lower an higher
        const size_t pl = findIndexAlongDim(2, p);

        // Now do 2d interpolation in each plane
        const size_t tl_r = findIndexAlongDim(0, r);
        const size_t tl_c = findIndexAlongDim(1, c);
        const double vpl = interp2d(tl_r, tl_c, pl, r, c);
        const double vph = interp2d(tl_r, tl_c, pl+1, r, c);

        // Return the 1d interpolation between the planes
        return interp1(p, mIndexData[2][pl], mIndexData[2][pl+1], vpl, vph);
    }

private:
    double interp2d(const size_t tl_r, const size_t tl_c, const size_t plane, const double r, const double c) const
    {
        const size_t tr_r = tl_r;
        const size_t bl_c = tl_c;
        const size_t tr_c = tl_c+1;
        const size_t br_c = tr_c;
        const size_t bl_r = tl_r+1;
        const size_t br_r = bl_r;

        const double tl_v = mValueData[calcDataIndex(tl_r, tl_c, plane)];
        const double tr_v = mValueData[calcDataIndex(tr_r, tr_c, plane)];
        const double bl_v = mValueData[calcDataIndex(bl_r, bl_c, plane)];
        const double br_v = mValueData[calcDataIndex(br_r, br_c, plane)];

        // Note, interp1 assumes that index data is strictly increasing (two values can not be the same). That will lead to division by zero here
        const double val_l = interp1(r, mIndexData[0][tl_r], mIndexData[0][bl_r], tl_v, bl_v);
        const double val_r = interp1(r, mIndexData[0][tr_r], mIndexData[0][br_r], tr_v, br_v);

        return interp1(c, mIndexData[1][tl_c], mIndexData[1][tr_c], val_l, val_r);
    }
};

#endif // LOOKUPTABLE_H
