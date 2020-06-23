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

#ifndef HSTRING_H
#define HSTRING_H

#include "win32dll.h"
#include "HVector.hpp"
#include <cstddef>


namespace hopsan {

class HOPSANCORE_DLLAPI HString
{
private:
    char *mpDataBuffer;
    size_t mSize;

public:
    static const size_t npos;

    HString();
    ~HString();
    HString(const char* str);
    HString(const char* str, const size_t len);
    HString(char c);
    HString(const int value);
    HString(const HString &rOther);
    HString(const HString &rOther, size_t pos, size_t len=npos);
    void setString(const char* str);
    void setString(const char* str, const size_t len);
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
    bool startsWith(const HString& rOther) const;
    bool isNummeric() const;
    bool isBool() const;
    double toDouble(bool *isOK) const;
    long int toLongInt(bool *isOK) const;
    bool toBool(bool *isOK) const;

    HString substr(const size_t pos, const size_t len=npos) const;
    HVector<HString> split(const char delim) const;

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

}

#endif // HSTRING_H
