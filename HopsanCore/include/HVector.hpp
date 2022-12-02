/*-----------------------------------------------------------------------------

 Copyright 2020 Hopsan Group

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

#ifndef HVECTOR_H
#define HVECTOR_H

#include <algorithm>
#include <vector>

namespace hopsan {

template<typename T>
class HVector
{
private:
    T *mpDataArray;
    size_t mSize;
    size_t mCapacity;

public:
    HVector()
    {
        mpDataArray = 0;
        mSize = 0;
        mCapacity = 0;
    }

    //! @brief copy constructor
    HVector(const HVector<T> &rOther)
    {
        mpDataArray = 0;
        mSize = 0;
        mCapacity = 0;
        *this = rOther;
    }

    HVector(const std::vector<T> &rOther)
    {
        mpDataArray = 0;
        mSize = 0;
        mCapacity = 0;
        this->reserve(rOther.size());
        this->resize(rOther.size());
        std::copy(rOther.begin(), rOther.end(), mpDataArray);
    }

    ~HVector()
    {
        clear();
    }

    //! @brief Clear the array
    void clear()
    {
        delete[] mpDataArray;
        mpDataArray = 0;
        mSize = 0;
        mCapacity = 0;
    }

    //! @brief Reserve capacity for the array
    //! @details If new size is smaller than old nothing is changed
    //! If new size is larger than old, the additional elements will be uninitialized
    //! @param [in] s New size
    void reserve(size_t s)
    {
        if (s > mCapacity) {
            T* pNewData = new T[s];
            if (pNewData) {
                std::copy(&mpDataArray[0], &mpDataArray[mSize], pNewData);
                delete[] mpDataArray;
                mpDataArray = pNewData;
                mCapacity = s;
            }
        }
    }

    //! @brief Resize the array, keeping old data if any.
    //! @details If new size is smaller than old, old data will intact up to the new size
    //! If new size is larger than old, the additional elements will be uninitialized
    //! @param [in] s New size
    void resize(size_t s)
    {
        if (s <= mCapacity) {
            mSize = s;
        }
        else {
            // Reserve twice the memory, to avoid reallocation on every resize in append case
            //! @todo Multiplying with 2 may be OK for small arrays, but for very large ones this could be a problem
            reserve(s*2);
            mSize = s;
        }
    }

    //! @brief Resize the array, initializing all values to defaultValue
    //! @param [in] s New size
    //! @param [in] defaultValue initialize value for all elements
    void resize(size_t s, const T &defaultValue)
    {
        resize(s);
        std::fill(&mpDataArray[0], &mpDataArray[mSize], defaultValue);
    }

    //! @brief Append data
    //! @note This function is slow, it will reallocate all array memory every time
    //! @param [in] data Data to append
    void append(const T &data)
    {
        size_t prevSize = mSize;
        resize(prevSize+1);
        mpDataArray[prevSize] = data;
    }

    //! @brief Assign from C-array
    //! param [in] pData A pointer to the array
    //! param [in] count The number of elements to copy from the array
    void assign_from(const T *pData, size_t count)
    {
        reserve(count);
        std::copy(&pData[0], &pData[count], mpDataArray);
        mSize = count;
    }

    const T& operator[] (size_t i) const
    {
        return mpDataArray[i];
    }

    T& operator[] (size_t i)
    {
        return mpDataArray[i];
    }

    const T& last() const
    {
        return  mpDataArray[mSize-1];
    }

    T& last()
    {
        return  mpDataArray[mSize-1];
    }

    const T& first() const
    {
        return  mpDataArray[0];
    }

    T& first()
    {
        return  mpDataArray[0];
    }

    //! @brief Returns the number of elements in the array
    //! @returns Number of elements in the array
    size_t size() const
    {
        return mSize;
    }

    //! @brief Returns the reserved number of element slots in the array
    //! @returns Number of reserved elements in the array
    size_t capacity() const
    {
        return mCapacity;
    }

    //! @brief Check if the array is empty
    //! @returns true if the array is empty
    bool empty() const
    {
        return (mSize==0);
    }

    bool contains(const T &data) const
    {
        for(int i=0; i<mSize; ++i) {
            if(mpDataArray[i] == data) {
                return true;
            }
        }
        return false;
    }

    HVector<T>& operator=(const HVector<T> &rhs)
    {
        resize(rhs.size());
        std::copy(&rhs.mpDataArray[0], &rhs.mpDataArray[rhs.size()], mpDataArray);
        return *this;
    }

    T* data()
    {
        return mpDataArray;
    }
};

}

#endif // HVECTOR_H
