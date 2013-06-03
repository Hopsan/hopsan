#ifndef HOPSANTYPES_H
#define HOPSANTYPES_H

#include <string>
#include <ostream>
#include "win32dll.h"


namespace hopsan {

class DLLIMPORTEXPORT HString
{
private:
    char *mpDataBuffer;
    unsigned int mSize;

public:
    static const unsigned int npos = -1;

    HString();
    ~HString();
    //HString(const std::string &rStdString);
    HString(const char* str);
    HString(const HString &rOther);
    HString(const HString &rOther, const unsigned int pos, const unsigned int len=npos);
    void setString(const char* str);
    HString &append(const char* str);
    HString &append(const char chr);
    HString &append(const HString &str);
    HString &erase (unsigned int pos = 0, unsigned int len = npos);
    void clear();

    void replace(const unsigned int pos, const unsigned int len, const char* str);
    HString &replace(const char* oldstr, const char* newstr);
    HString &replace(const HString &rOldstr, const HString &rNewstr);

    const char *c_str() const;
    unsigned int size() const;
    bool empty() const;
    bool compare(const char* other) const;
    bool compare(const HString &rOther) const;

    HString substr(const unsigned int pos, const unsigned int len=npos) const;

    unsigned int find_first_of(const char c, unsigned int pos = 0) const;
    unsigned int rfind(const char c, unsigned int pos = npos) const;
    unsigned int find(const char c, unsigned int pos = 0) const;
    unsigned int find(const char *s, unsigned int pos = 0) const;
    unsigned int find(const HString &s, unsigned int pos = 0) const;
    bool containes(const HString &rString) const;
    bool containes(const char *s) const;

    char front() const;
    char &front();
    char back() const;
    char &back();
    char at(const unsigned int pos) const;
    char& operator[](const unsigned int idx);
    const char& operator[](const unsigned int idx) const;

    bool operator<(const HString &rhs) const;

    HString& operator+=(const HString& rhs);
    HString& operator+=(const char *rhs);
    HString& operator+=(const char rhs);

    HString& operator=(const char* rhs);
    HString& operator=(const char rhs);
    HString& operator=(const HString &rhs);

    //friend std::ostream& operator<<(std::ostream& os, const HString& obj);
};

inline bool operator==(const HString& lhs, const HString& rhs){return lhs.compare(rhs);}
//bool DLLIMPORTEXPORT operator< (const HString& lhs, const HString& rhs);
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

//std::ostream& operator<<(std::ostream& os, const HString& obj);

////! @todo this might not be needed later when/if we use hstrings everywhere
//inline std::string toStdString(const hopsan::HString &rString)
//{
//    return std::string(rString.c_str());
//}

template<typename T>
class HVector
{
private:
    T *mpDataArray;
    unsigned int mSize;

public:
    HVector()
    {
        mpDataArray = 0;
        mSize = 0;
    }

    //! @brief copy constructor
    HVector(const HVector<T> &rOther)
    {
        clear();
        mpDataArray = new T[rOther.size()];
        mSize = rOther.size();
        for (unsigned int i=0; i<rOther.size(); ++i)
        {
            (*mpDataArray)[i] = rOther[i];
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
    //! @detailed If new size is smaller than old, old data will be truncated
    //! If new size is larger than old, the additional elements will be uninitialized
    //! @param [in] s New size
    void resize(const unsigned int s)
    {
        // Create new dummy array
        T* pNewArray = new T[s];

        // Check how many elements to copy
        unsigned int n = s;
        if (size() < s)
        {
            n = size();
        }

        // Copy old data to new array
        for (unsigned int i=0; i<n; ++i)
        {
            pNewArray[i] = mpDataArray[i];
        }

        // Clear old data
        clear();

        // Set new data
        mpDataArray = pNewArray;
        mSize = n;
    }

    //! @brief Resize the array, initializing all values to defaultValue
    //! @param [in] s New size
    //! @param [in] s defaultValue initialize value for all elements
    void resize(const unsigned int s, const T defaultValue)
    {
        clear();
        mpDataArray = new T[s](defaultValue);
        mSize = s;
    }

    //! @brief Append data
    //! @note This function is slow, it will reallocate all array memmory every time
    //! @param [in] rData Data to append
    void append(const T &rData)
    {
        resize(size()+1);
        mpDataArray[size()-1] = rData;
    }

    const T& operator[] (const unsigned int i) const
    {
        return mpDataArray[i];
    }

    T& operator[] (const unsigned int i)
    {
        return mpDataArray[i];
    }

    //! @brief Returns the number of elements in the array
    //! @returns Number of elements in the array
    unsigned int size() const
    {
        return mSize;
    }

    //! @brief Ceck if the array is empty
    //! @returns true if the array is empty
    bool empty() const
    {
        return (mSize==0);
    }
};

}

#endif // HOPSANTYPES_H
