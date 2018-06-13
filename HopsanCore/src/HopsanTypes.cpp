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


//$Id$

#include "HopsanTypes.h"
#include <cstdlib>
#include <cstring>
#include <string>

using namespace hopsan;

const size_t HString::npos = -1;

HString::HString()
{
    mpDataBuffer=0;
    mSize=0;
}

HString::~HString()
{
    clear();
}

HString::HString(const char *str)
{
    mpDataBuffer=0;
    mSize=0;
    setString(str);
}

HString::HString(const char *str, const size_t len)
{
    mpDataBuffer=0;
    mSize=0;
    setString(str, len);
}

//! @brief This constructor is used to catch mistakes made by Hospan component developers
HString::HString(const int value)
{
    (void)value;
    mpDataBuffer=0;
    mSize=0;
    setString("Error: Creating HString from int (value)");
    //! @todo maybe support constructing HString directly from int, long int / double and such types
}

//! @brief Copy constructor
HString::HString(const HString &rOther)
{
    mpDataBuffer=0;
    mSize=0;
    setString(rOther.c_str());
}

HString::HString(const HString &rOther, size_t pos, size_t len)
{
    mpDataBuffer=0;
    mSize=0;

    // Make sure initial pos is not to high
    if (pos > rOther.size())
    {
        pos = rOther.size();
    }

    // Auto calculate end index and make sure it is within bounds
    if (len == npos)
    {
        len = rOther.size()-pos;
    }

    // If given length to long then set maximum allowed length
    if (pos+len > rOther.size())
    {
        len = rOther.size()-pos;
    }

    // Reallocate memory and copy string
    mpDataBuffer = static_cast<char*>(realloc(mpDataBuffer,len+1));
    mSize = len;
    strncpy(mpDataBuffer, rOther.c_str()+pos, len);
    mpDataBuffer[mSize] = '\0';
}

//! @brief Set the string by copying const char*
//! @param [in] str The string data to copy
void HString::setString(const char *str)
{
    setString(str, strlen(str));
}

//! @brief Set the string by copying a specific number of chars from const char*
//! @param [in] str The string data to copy
//! @param [in] len The number of bytes to copy
void HString::setString(const char *str, const size_t len)
{
    if (len>0)
    {
        mpDataBuffer = static_cast<char*>(realloc(mpDataBuffer,len+1));
        strcpy(mpDataBuffer, str);
        mSize = len;
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

HString &HString::erase(size_t pos, size_t len)
{
    if (pos > mSize)
    {
        pos = mSize;
    }
    HString n1(*this, 0, pos);
    if (len == npos)
    {
        len = mSize-pos;
    }
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

size_t HString::size() const
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
    if (mSize == strlen(other))
    {
        if (mSize == 0) {
            return true;
        }
        else {
            return (strcmp(mpDataBuffer,other) == 0);
        }
    }
    return false;
}

bool HString::compare(const HString &rOther) const
{
    if (mSize == rOther.size()) {
        if (mSize == 0) {
            return true;
        }
        else {
            return (strcmp(mpDataBuffer, rOther.c_str()) == 0);
        }
    }
    return false;
}

bool HString::startsWith(const HString &rOther) const
{
    if (mSize < rOther.size()) {
        return false;
    }
    else if (empty() && rOther.empty()) {
        return true;
    }
    else {
        return (strncmp(mpDataBuffer, rOther.c_str(), rOther.size()) == 0);
    }
}

//! @brief Check if the string can be interpreted as a number
bool HString::isNummeric() const
{
    if (empty() || isspace(*mpDataBuffer))
    {
        return false;
    }
    else
    {
        char *pEnd;
        strtod(mpDataBuffer, &pEnd);
        return *pEnd == '\0';
    }
}

//! @brief Check if the string can be interpreted as bool (true, false, 1, 0)
bool HString::isBool() const
{
    return (compare("true") || compare("false") || compare("1") || compare("0"));
}

double HString::toDouble(bool *isOK) const
{
    if (mSize > 0)
    {
        char *pEnd;
        double d = strtod(mpDataBuffer, &pEnd);
        *isOK = (*pEnd == '\0');
        return d;
    }
    else
    {
        *isOK = false;
        return 0;
    }
}

long int HString::toLongInt(bool *isOK) const
{
    if (mSize > 0)
    {
        char *pEnd;
        //! @todo maybe support other bases then 10, see strtol documentation
        long int i = strtol(mpDataBuffer, &pEnd, 10);
        *isOK = (*pEnd == '\0');
        return i;
    }
    else
    {
        *isOK = false;
        return 0;
    }
}

bool HString::toBool(bool *isOK) const
{
    *isOK = isBool();
    if (compare("true") || compare("1")) {
        return true;
    }
    return false;
}

size_t HString::find_first_of(const char c, size_t pos) const
{
    return find(c,pos);
}

size_t HString::rfind(const char c, size_t pos) const
{
    size_t i = pos;
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

size_t HString::find(const char c, size_t pos) const
{
    for (size_t i=pos; i<mSize; ++i)
    {
        if (mpDataBuffer[i] == c)
        {
            return i;
        }
    }
    return npos;
}

size_t HString::find(const char *s, size_t pos) const
{
    // Make sure s or local string is not null string
    if ( (strlen(s) > 0) && !empty() && pos<mSize)
    {
        // Try to find substring pointer
        const char* pFirst = strstr(mpDataBuffer+pos, s);
        if (pFirst)
        {
            // Calculate address offset from pointers
            return (pFirst - mpDataBuffer);
        }
    }

    // Return npos if not found
    return npos;
}

size_t HString::find(const HString &s, size_t pos) const
{
    return find(s.c_str(),pos);
}

bool HString::containes(const HString &rString) const
{
    return (find(rString) != npos);
}

bool HString::containes(const char c) const
{
    return (find(c) != npos);
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

char HString::at(const size_t pos) const
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

char &HString::operator [](const size_t idx)
{
    return mpDataBuffer[idx];
}

const char &HString::operator [](const size_t idx) const
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

void HString::replace(const size_t pos, const size_t len, const char *str)
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
        pos = find(oldstr, pos+strlen(newstr));
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
HString HString::substr(const size_t pos, const size_t len) const
{
    HString sub(*this, pos, len);
    return sub;
}

HVector<HString> HString::split(const char delim) const
{
  HVector<HString> parts;
  size_t b = 0;
  while (true)
  {
    size_t e = find(delim, b);
    parts.append(substr(b,e-b));
    if (e == npos)
    {
      break;
    }
    b=e+1;
  }
  return parts;
}
