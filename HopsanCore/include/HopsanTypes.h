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


//$Id$

#ifndef HOPSANTYPES_H
#define HOPSANTYPES_H

#include "win32dll.h"
#include <cstddef>
#include <climits>
#include <cstring>
#include <algorithm>

namespace hopsan {

class DLLIMPORTEXPORT HString
{
private:
    char *mpDataBuffer;
    size_t mSize;

public:
    static const size_t npos;

    HString();
    ~HString();
    HString(const char* str);
    HString(const HString &rOther);
    HString(const HString &rOther, size_t pos, size_t len=npos);
    void setString(const char* str);
    HString &append(const char* str);
    HString &append(const char chr);
    HString &append(const HString &str);
    HString &erase (size_t pos = 0, size_t len = npos);
    void clear();

    void replace(const size_t pos, const size_t len, const char* str);
    HString &replace(const char* oldstr, const char* newstr);
    HString &replace(const HString &rOldstr, const HString &rNewstr);

    const char *c_str() const;
    size_t size() const;
    bool empty() const;
    bool compare(const char* other) const;
    bool compare(const HString &rOther) const;
    bool isNummeric() const;
    bool isBool() const;
    double toDouble(bool *isOK) const;
    long int toLongInt(bool *isOK) const;

    HString substr(const size_t pos, const size_t len=npos) const;

    size_t find_first_of(const char c, size_t pos = 0) const;
    size_t rfind(const char c, size_t pos = npos) const;
    size_t find(const char c, size_t pos = 0) const;
    size_t find(const char *s, size_t pos = 0) const;
    size_t find(const HString &s, size_t pos = 0) const;
    bool containes(const HString &rString) const;
    bool containes(const char c) const;
    bool containes(const char *s) const;

    char front() const;
    char &front();
    char back() const;
    char &back();
    char at(const size_t pos) const;
    char& operator[](const size_t idx);
    const char& operator[](const size_t idx) const;

    bool operator<(const HString &rhs) const;

    HString& operator+=(const HString& rhs);
    HString& operator+=(const char *rhs);
    HString& operator+=(const char rhs);

    HString& operator=(const char* rhs);
    HString& operator=(const char rhs);
    HString& operator=(const HString &rhs);
};

inline bool operator==(const HString& lhs, const HString& rhs){return lhs.compare(rhs);}
inline bool operator!=(const HString& lhs, const HString& rhs){return !operator==(lhs,rhs);}
inline bool operator> (const HString& lhs, const HString& rhs){return rhs<lhs;}
inline bool operator<=(const HString& lhs, const HString& rhs){return !operator> (lhs,rhs);}
inline bool operator>=(const HString& lhs, const HString& rhs){return !(lhs<rhs);}

inline HString operator+(HString lhs, const HString& rhs)
{
  lhs += rhs;
  return lhs;
}

inline HString operator+(HString lhs, const char rhs)
{
  lhs += rhs;
  return lhs;
}

template<typename T>
class HShallowVector
{
protected:
    T *mpDataArray;
    size_t mSize;

public:
    HShallowVector()
    {
        mpDataArray = 0;
        mSize = 0;
    }

    HShallowVector(T* pArray, const size_t size)
    {
        mpDataArray = pArray;
        mSize = size;
    }

    //! @brief copy constructor
    HShallowVector(const HShallowVector<T> &rOther)
    {
        mpDataArray = rOther.mpDataArray;
        mSize = rOther.mSize;
    }

    //! @brief Assignment operator
    HShallowVector& operator=(const HShallowVector &rhs)
    {
        mpDataArray = rhs.mpDataArray;
        mSize = rhs.mSize;
        return *this;
    }

    ~HShallowVector()
    {
        clear();
    }

    //! @brief Clear the array
    void clear()
    {
        mpDataArray = 0;
        mSize = 0;
    }

    //! @brief Copy values into a section of the underlying array
    //! @param[in] pSource A pointer to the array with source data
    //! @param[in] num The number of elements to copy
    //! @param[in] start The first element in the section to copy into
    //! It is assumed that both the underlying array and source array are long enough, no bounds checking is done
    inline
    void copyFrom(const T *pSource, const size_t num, const size_t start=0)
    {
        memcpy(static_cast<void*>(&mpDataArray[start]), static_cast<const void*>(pSource), num*sizeof(T));
    }

    inline
    const T& operator[] (const size_t i) const
    {
        return mpDataArray[i];
    }

    inline
    T& operator[] (const size_t i)
    {
        return mpDataArray[i];
    }

    //! @brief Returns the number of elements in the array
    //! @returns Number of elements in the array
    inline
    size_t size() const
    {
        return mSize;
    }

    //! @brief Check if the array is empty
    //! @returns true if the array is empty
    inline
    bool empty() const
    {
        return (mSize==0);
    }

    //! @brief Returns a modifiable copy of the data pointer, use with care
    inline
    T* data()
    {
        return mpDataArray;
    }
};


template<typename T>
class HVector : public HShallowVector<T>
{
protected:
    using HShallowVector<T>::mpDataArray;
    using HShallowVector<T>::mSize;
public:
    HVector()
    {
        mpDataArray = 0;
        mSize = 0;
    }

    //! @brief copy constructor
    HVector(const HVector<T> &rOther)
    {
        mpDataArray = new T[rOther.size()];
        mSize = rOther.size();
        for (size_t i=0; i<rOther.size(); ++i)
        {
            mpDataArray[i] = rOther[i];
        }
    }

    ~HVector()
    {
        clear();
    }

    //! @brief Clear the array
    void clear()
    {
        if (mpDataArray)
        {
            delete[] mpDataArray;
            mSize = 0;
            mpDataArray=0;
        }
    }

    //! @brief Resize the array, keeping old data if any.
    //! @details If new size is smaller than old, old data will be truncated
    //! If new size is larger than old, the additional elements will be default initialized
    //! @param [in] s New size
    void resize(const size_t s)
    {
        if (s != mSize)
        {
            mpDataArray = static_cast<T*>(realloc(static_cast<void*>(mpDataArray), s*sizeof(T)));
            //! @todo what if failure
            if (mpDataArray)
            {
                mSize = s;
            }
            else
            {
                mSize = 0;
            }
        }
//        // Create new dummy array
//        T* pNewArray = new T[s];

//        // Check how many elements to copy
//        size_t nCopy = s;
//        if (mSize < s)
//        {
//            nCopy = mSize;
//        }

//        // Copy old data to new array
//        for (size_t i=0; i<nCopy; ++i)
//        {
//            pNewArray[i] = mpDataArray[i];
//        }

//        // Clear old data
//        clear();

//        // Set new data
//        mpDataArray = pNewArray;
//        mSize = s;
    }

    //! @brief Resize the array, initializing ALL values to defaultValue
    //! @param [in] s New size
    //! @param [in] rDefaultValue initialize value for all elements
    void resize(const size_t s, const T &rDefaultValue)
    {
//        clear();
//        mpDataArray = new T[s];
//        mSize = s;
        resize(s);
        for (size_t i=0; i<mSize; ++i)
        {
            mpDataArray[i] = rDefaultValue;
        }
    }

    //! @brief Append data
    //! @note This function is slow, it will reallocate all array memory every time
    //! @param[in] rData Data to append
    inline
    void append(const T &rData)
    {
        resize(mSize+1);
        mpDataArray[mSize-1] = rData;
    }
};

//! @brief Shallow Hopsan matrix representing a plain data array of type T stored in row major order
template<typename T>
class HShallowMatrix
{
private:
    T *mpDataArray;
    size_t mRows, mCols;

public:
    HShallowMatrix()
    {
        mpDataArray = 0;
        mRows = 0;
        mCols = 0;
    }

    HShallowMatrix(T *pData, size_t rows, size_t cols)
    {
        mpDataArray = pData;
        mRows = rows;
        mCols = cols;
    }

    //! @brief Assignment operator
    HShallowMatrix& operator=(const HShallowMatrix &rhs)
    {
        mpDataArray = rhs.mpDataArray;
        mRows = rhs.mRows;
        mCols = rhs.mCols;
        return *this;
    }

    //! @brief Shallow copy constructor
    HShallowMatrix(const HShallowMatrix<T> &rOther)
    {
        *this = rOther;
    }

    inline size_t rows() const
    {
        return mRows;
    }

    inline size_t columns() const
    {
        return mCols;
    }

    //! @brief Extract a row from the matrix
    //! @param[in] row The row index
    //! @param[out] rDest The destination (std::vector compatible container, resize() and [] needed)
    //! @param[in] maxNumCols The maximum number of columns to include from the row
    template <typename T2>
    void getRow(const size_t row, T2 &rDest, size_t maxNumCols=INT_MAX) const
    {
        if (row < mRows)
        {
            maxNumCols = std::min(mCols, maxNumCols);
            rDest.resize(maxNumCols);
            size_t start = row*mCols;
            for (size_t c=0; c<maxNumCols; ++c)
            {
                rDest[c] = mpDataArray[start+c];
            }
        }
    }

    //! @brief Extract a row from the matrix
    //! @param[in] row The row index
    //! @param[in] maxNumCols The maximum number of columns to include from the row
    //! @returns The extracted row (std::vector compatible container, resize() and [] needed)
    template <typename T2>
    T2 getRow(const size_t row, size_t maxNumCols=INT_MAX) const
    {
        T2 vec;
        getRow(row,vec,maxNumCols);
        return vec;
    }

    //! @brief Extract a column from the matrix
    //! @param[in] col The column index
    //! @param[out] rDest The destination (std::vector compatible container, resize() and [] needed)
    //! @param[in] maxNumRows The maximum number of rows to include from the column
    template <typename T2>
    void getColumn(const size_t col, T2 &rDest, size_t maxNumRows=INT_MAX) const
    {
        if (col < mCols)
        {
            maxNumRows = std::min(mRows, maxNumRows);
            rDest.resize(maxNumRows);
            size_t step = mCols;
            for (size_t r=0; r<maxNumRows; ++r)
            {
                rDest[r] = mpDataArray[r*step+col];
            }
        }
    }

    //! @brief Extract a column from the matrix
    //! @param[in] col The column index
    //! @param[in] maxNumRows The maximum number of rows to include from the column
    //! @returns The extracted column (std::vector compatible container, resize() and [] needed)
    template <typename T2>
    T2 getColumn(const size_t col, size_t maxNumRows=INT_MAX) const
    {
        T2 vec;
        getColumn(col,vec,maxNumRows);
        return vec;
    }

    //! @todo check bounds error
    T at(size_t row, size_t column) const
    {
        return mpDataArray[row*mCols+column];
    }

    bool empty() const
    {
        return (mRows==0 || mCols==0);
    }

};

typedef HShallowMatrix<double> HShallowMatrixD;
typedef HShallowVector<double> HShallowVectorD;
typedef HVector<double> HVectorD;

}

#endif // HOPSANTYPES_H
