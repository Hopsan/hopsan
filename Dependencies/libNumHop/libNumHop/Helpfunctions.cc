#include "Helpfunctions.h"

using namespace std;
namespace numhop {

//! @brief Split a string into multiple rows based on ; or \n characters
//! @param[in] expr The expression as a string
//! @param[in] comment The comment character (ignore those lines)
//! @param[out] rExpressions The list of expression lines
void extractExpressionRows(const string &expr, const char &comment, list<string> &rExpressions)
{
    size_t s=0, e;
    for (e=0; e<expr.size(); ++e)
    {
        const char &c = expr[e];
        bool foundSeparator = (c == '\n' || c == '\r' || c == ';' || c == comment);
        bool foundEOS = (e+1 == expr.size());
        if (foundSeparator || foundEOS)
        {
            // Handle end of string, we need to read the last char
            if (foundEOS && !foundSeparator)
            {
                ++e;
            }

            if (s<expr.size() && (e-s)>0)
            {
                string part = expr.substr(s,e-s);
                stripLeadingTrailingWhitespaces(part);
                if (!part.empty())
                {
                    rExpressions.push_back(part);
                }
            }

            // If comment skip to after next newline
            if (c == comment)
            {
                for (; e<expr.size(); ++e)
                {
                    const char &cc = expr[e];
                    if (cc == '\n' || cc == '\r')
                    {
                        // Advance (start) one step from the found \n character
                        s=e+1;
                        break;
                    }
                }
            }
            else
            {
                // Advance (start) one step from the found ; or \n character
                s = e+1;
            }
        }
    }
}

//! @brief Strip leading and trailing spaces from a string
//! @param[in,out] rString The string to process
void stripLeadingTrailingWhitespaces(string &rString)
{
    while (!rString.empty() && (rString[0] == ' ' || rString[0] == '\t'))
    {
        rString.erase(0,1);
    }
    while (!rString.empty() && (rString[rString.size()-1] == ' ' || rString[rString.size()-1] == '\t'))
    {
        rString.erase(rString.size()-1);
    }
}

//! @brief Remove all white spaces (space, tab) from a string
//! @param[in,out] rString The string to process
void removeAllWhitespaces(string &rString)
{
    for (size_t i=0; i<rString.size(); ++i)
    {
        if (rString[i] == ' ' || rString[i] == '\t')
        {
            rString.erase(i,1);
        }
    }
}

//! @brief Strip leading and trailing parenthesis ( ) from a string
//! @param[in,out] rString The string to process
//! @param[out] rDidStrip Indicates  whether parenthesis were removed or not
//! @returns false if there is an error in the number of parenthesis, else true
bool stripLeadingTrailingParanthesis(string &rString, bool &rDidStrip)
{
    rDidStrip = false;
    if (!rString.empty() && rString[0] == '(' && rString[rString.size()-1] == ')')
    {
        // Need to count parenthesis so that we only clear if the closing one closes the one first opened
        size_t numOpen=1;
        bool doClear=false;
        for (size_t i=1; i<rString.size(); ++i)
        {
            char &c = rString[i];
            if (c == '(')
            {
                numOpen++;
            }
            else if (c == ')')
            {
                numOpen--;
            }

            // Break if we finally close the first parenthesis
            if (numOpen == 0)
            {
                if (i == rString.size()-1)
                {
                    doClear=true;
                }
                break;
            }
        }

        if (doClear)
        {
            rString.erase(0,1);
            rString.erase(rString.size()-1, 1);
        }
        rDidStrip = doClear;
    }

    return true;
}


//! @brief Strip the initial sign character (if it exists)
//! @param[in,out] rString The string to process
//! @returns + or - char, representing the sign of string (+ if no sign found)
char stripInitialSign(string &rString)
{
    if (!rString.empty())
    {
        if (rString[0] == '-')
        {
            rString.erase(0,1);
            return '-';
        }
        else if (rString[0] == '+')
        {
            rString.erase(0,1);
        }
    }
    return '+';
}

//! @brief Strip the initial + sign character (if it exists)
//! @param[in,out] rString The string to process
void stripInitialPlus(string &rString)
{
    if (!rString.empty() && rString[0]=='+')
    {
        rString.erase(0,1);
    }
}

}
