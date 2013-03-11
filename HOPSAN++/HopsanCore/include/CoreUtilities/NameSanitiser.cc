#include "NameSanitiser.h"

#define UNDERSCORE 95
#define UPPERCASE_LOW 65
#define UPPERCASE_HIGH 90
#define LOWERCASE_LOW 97
#define LOWERCASE_HIGH 122
#define NUMBERS_LOW 48
#define NUMBERS_HIGH 57

void santizeName(std::string &rString)
{
    std::string::iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))     ) )
        {
            // Replace invalid char with underscore
            *it = UNDERSCORE;
        }
    }
}

bool isNameValid(const std::string &rString)
{
    std::string::const_iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))   ||
                (*it == UNDERSCORE)                                   ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

bool isNameValid(const std::string &rString, const std::string &rExceptions)
{
    std::string::const_iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))   ||
                (*it == UNDERSCORE) || (rExceptions.find(*it)!=std::string::npos) ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

std::string santizeName(const std::string &rString)
{
    std::string newString = rString;
    santizeName(newString);
    return newString;
}
