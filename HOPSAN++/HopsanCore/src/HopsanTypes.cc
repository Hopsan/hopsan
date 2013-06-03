#include "HopsanTypes.h"
#include <cstdlib>
#include <cstring>

using namespace hopsan;

HString::HString()
{
    mpDataBuffer=0;
    mSize=0;
}

HString::~HString()
{
    clear();
}

////! @todo, maybe remove this copyconstructor since it may be unsafe if std::string comes from an other DLL/SO
//HString::HString(const std::string &rStdString)
//{
//    mpDataBuffer=0;
//    mSize=0;
//    setString(rStdString.c_str());
//}

HString::HString(const char *str)
{
    mpDataBuffer=0;
    mSize=0;
    setString(str);
}

//! @brief Copy constructor
HString::HString(const HString &rOther)
{
    mpDataBuffer=0;
    mSize=0;
    setString(rOther.c_str());
}

HString::HString(const HString &rOther, const unsigned int pos, const unsigned int len)
{
    mpDataBuffer=0;
    mSize=0;

    // First caluclate end index and make sure it is within bounds
    unsigned int otherNumChar;
    if (len == npos)
    {
        otherNumChar = rOther.size()-pos;
    }
    else
    {
        // If given length is within bounds, use it
        if (pos+len <= rOther.size())
        {
            otherNumChar = pos+len;
        }
        // Else use the maximum allowed length
        else
        {
            otherNumChar = rOther.size()-pos;
        }
    }

    // Reallocate memmory and copy string
    mpDataBuffer = static_cast<char*>(realloc(mpDataBuffer,otherNumChar+1));
    mSize = otherNumChar;
    strncpy(mpDataBuffer, rOther.c_str()+pos, otherNumChar);
    mpDataBuffer[mSize] = '\0';
}

//! @brief Set the string by copying const char*
//! @param [in] str The string data to copy
void HString::setString(const char *str)
{
    const size_t s = strlen(str);
    if (s>0)
    {
        mpDataBuffer = static_cast<char*>(realloc(mpDataBuffer,s+1));
        strcpy(mpDataBuffer, str);
        mSize = s;
    }
    else
    {
        clear();
    }
}

HString &HString::append(const char *str)
{
    size_t s = strlen(str);
    if (s>0)
    {
        s += size();
        mpDataBuffer = static_cast<char*>(realloc(mpDataBuffer,s+1));
        strcpy(mpDataBuffer+size(), str);
        mSize = s;
    }
    return *this;
}

HString &HString::append(const char chr)
{
    mpDataBuffer = static_cast<char*>(realloc(mpDataBuffer,size()+2));
    mpDataBuffer[mSize] = chr;
    mSize = mSize+1;
    mpDataBuffer[mSize] = '\0';

    return *this;
}

HString &HString::append(const HString &str)
{
    this->append(str.c_str());
    return *this;
}

HString &HString::erase(unsigned int pos, unsigned int len)
{
    HString n1(*this, 0, pos);
    HString n2(*this, pos+len);
    setString((n1+n2).c_str());
    return *this;
}

//! @brief Returns a c_str pointer to internal data
//! @note The pointer is only valid, while the HString object is alive
const char *HString::c_str() const
{
    if (mpDataBuffer)
    {
        return mpDataBuffer;
    }
    else
    {
        //! @todo will this work when mpDataBuffer == 0
        return "";
    }

}

unsigned int HString::size() const
{
    return mSize;
}

//! @brief Check if string is empty
//! @returns True if empty, else False
bool HString::empty() const
{
    return (mSize==0);
}

//! @brief Compare string to const char*
//! @param [in] other The string to compare to
//! @returns True if same, else False
bool HString::compare(const char *other) const
{
    if (strlen(other) != size())
    {
        return false;
    }
    else
    {
        return (strcmp(mpDataBuffer,other) == 0);
    }
}

bool HString::compare(const HString &rOther) const
{
    if (mSize != rOther.size())
    {
        return false;
    }
    return (strcmp(mpDataBuffer, rOther.c_str()) == 0);
}

unsigned int HString::find_first_of(const char c, unsigned int pos) const
{
    return find(c,pos);
}

unsigned int HString::rfind(const char c, unsigned int pos) const
{
    unsigned int i = pos;
    if (i > mSize)
    {
        i = mSize;
    }
    for ( ; i>0; --i)
    {
        if (mpDataBuffer[i-1] == c)
        {
            return i-1;
        }
    }
    return npos;
}

unsigned int HString::find(const char c, unsigned int pos) const
{
    for (unsigned int i=pos; i<mSize; ++i)
    {
        if (mpDataBuffer[i] == c)
        {
            return i;
        }
    }
    return npos;
}

unsigned int HString::find(const char *s, unsigned int pos) const
{
    const char* pFirst = strstr(mpDataBuffer+pos, s);
    if (pFirst)
    {
        return (pFirst - mpDataBuffer);
    }
    else
    {
        return npos;
    }
}

unsigned int HString::find(const HString &s, unsigned int pos) const
{
    return find(s.c_str(),pos);
}

bool HString::containes(const HString &rString) const
{
    return (find(rString) != npos);
}

bool HString::containes(const char *s) const
{
    return (find(s) != npos);
}

char HString::front() const
{
    return mpDataBuffer[0];
}

char &HString::front()
{
    return mpDataBuffer[0];
}

char HString::back() const
{
    return mpDataBuffer[mSize-1];
}

char &HString::back()
{
    return mpDataBuffer[mSize-1];
}

char HString::at(const unsigned int pos) const
{
    if (pos < mSize)
    {
        return mpDataBuffer[pos];
    }
    else
    {
        return '\0';
    }
}

bool HString::operator <(const HString &rhs) const
{
    if (empty())
    {
        return strcmp("", rhs.c_str()) < 0;
    }
    else
    {
        return strcmp(mpDataBuffer, rhs.c_str()) < 0;
    }
}

//! @todo these could be inlined
HString &HString::operator +=(const HString &rhs)
{
    return append(rhs.c_str());
}

HString &HString::operator +=(const char *rhs)
{
    return append(rhs);
}

HString &HString::operator +=(const char rhs)
{
    return append(rhs);
}

char &HString::operator [](const unsigned int idx)
{
    return mpDataBuffer[idx];
}

const char &HString::operator [](const unsigned int idx) const
{
    return mpDataBuffer[idx];
}

HString &HString::operator =(const char *rhs)
{
    setString(rhs);
    return *this;
}

HString &HString::operator =(const char rhs)
{
    setString(" ");
    mpDataBuffer[0]=rhs;
    return *this;
}

HString& HString::operator=(const HString &rhs)
{
    setString(rhs.c_str());
    return *this;
}

//! @brief Clear the string
void HString::clear()
{
    if (mpDataBuffer)
    {
        free(mpDataBuffer);
        mSize = 0;
        mpDataBuffer=0;
    }
}

void HString::replace(const unsigned int pos, const unsigned int len, const char *str)
{
    //! @todo do this properly without using local string
    std::string temp = mpDataBuffer;
    temp.replace(pos, len, str);
    this->setString(temp.c_str());
}

HString &HString::replace(const char *oldstr, const char *newstr)
{
    size_t pos = find(oldstr);
    while (pos!=HString::npos)
    {
        replace(pos, strlen(oldstr), newstr);
        pos = find(oldstr);
    }
    return *this;
}

HString &HString::replace(const HString &rOldstr, const HString &rNewstr)
{
    return replace(rOldstr.c_str(), rNewstr.c_str());
}

//! @brief Extract substring
//! @param [in] pos First index of substring
//! @param [in] len Num bytes in substring, or HString::npos for all
//! @returns A substring
HString HString::substr(const unsigned int pos, const unsigned int len) const
{
    HString sub(*this, pos, len);
    return sub;
}

//bool operator< (const HString& lhs, const HString& rhs)
//{
//    unsigned int size = lhs.size();
//    if (size > rhs.size())
//    {
//        size = rhs.size();
//    }

//    for (unsigned int i=0; i<size; ++i)
//    {
//        if (lhs[i]>rhs[i])
//        {
//            return false;
//        }
//    }
//    return true;
//}

//std::ostream& operator<<(std::ostream& os, const HString& obj)
//{
//  os << obj.c_str();
//  return os;
//}
