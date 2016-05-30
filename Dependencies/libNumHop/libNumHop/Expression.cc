#include "Expression.h"
#include "Helpfunctions.h"
#include <cstdlib>
#include <cmath>
#include <vector>

namespace numhop {

const std::string allOperators="=+-*/^<>&|";
const std::string operatorsNotAllowedAfterEqualSign="=*/^<>&|";
const std::string operatorsNotPME="*/^<>&|";

// Internal help functions
inline double boolify(const double v)
{
    if (v>0.5) {return 1.;} return 0.;
}

inline bool checkOperatorsNexttoEqualSign(size_t e, const std::string &expr)
{
    if ( (int(e) < int(expr.size())-1) && contains(operatorsNotAllowedAfterEqualSign, expr[e+1]) )
    {
        return false;
    }
    if ( e==0 || (e>0 && contains(allOperators, expr[e-1])) )
    {
        return false;
    }
    return true;
}

bool fixMultiOperators(size_t e, std::string &expr)
{
    char op = expr[e];
    const size_t s=e;

    // Now check + and minus signs
    if (op == '+' || op == '-')
    {
        bool isPositive=true;
        // Search forward and compress multiple signs into one
        for (; e<expr.size(); ++e)
        {
            if (expr[e] == '-')
            {
                isPositive = !isPositive;
            }
            else if (expr[e] != '+')
            {
                break;
            }
        }
        // Now replace the sequence of operators with one sign character
        if (isPositive)
        {
            expr.replace(s, e-s, 1, '+');
        }
        else
        {
            expr.replace(s, e-s, 1, '-');
        }
    }
    else if (contains(operatorsNotPME, op))
    {
        if (e<expr.size()-1 && contains(allOperators, expr[e+1]))
        {
             return false;
        }
    }
    return true;
}

//! @brief Check if a char is fisrt part of an exponential notation
//! @param[in] expr The expression string
//! @param[in] i The index of the first char efter the e or E
//! @returns True or False
bool isExpNot(const std::string &expr, const size_t i)
{
    // For i to be first part of exponential notation,
    // prev. char should be 'e' or 'E'
    // prev.prev. char should be a digit
    if (i>1)
    {
        //const char &c = expr[i];
        //! @todo maybe should check that c is +, - or digit
        const char &pc = expr[i-1];
        const char &ppc = expr[i-2];
        return ((pc == 'e') || (pc == 'E')) && isdigit(ppc);
    }
    return false;
}

//! @brief Find an operator and branch the expression tree at this point
//! @param[in] exprString The expression string to process
//! @param[in] evalOperators A string with the operators to search for
//! @param[out] rExprList The resulting expression list (Empty if nothing found)
//! @returns False if some error occurred else true
bool branchExpressionOnOperator(std::string exprString, const std::string &evalOperators, std::list<Expression> &rExprList)
{
    bool hadParanthesis;
    removeAllWhitespaces(exprString);
    stripLeadingTrailingParanthesis(exprString, hadParanthesis);

    size_t nOpenParanthesis=0, e=0;
    bool foundOperator=false;
    std::vector<size_t> breakpts;
    breakpts.push_back(e);
    for (e=0; e<exprString.size(); ++e)
    {
        const char &c = exprString[e];

        // Count parenthesis
        if (c == '(')
        {
            nOpenParanthesis++;
        }
        else if (c == ')')
        {
            nOpenParanthesis--;
        }

        if (nOpenParanthesis == 0)
        {
            //! @todo it might be a good idea to compare the char with a range of asci values, to quickly decide if a char is an operator, instead of comparing every char multiple times
            bool foundOperatorAtThisLocation=false;
            // Check for assignment, (can only have one assignment in expression)
            if ( (c == '=') && contains(evalOperators, '=') )
            {
                foundOperator=true;
                breakpts.push_back(e);
                if (!checkOperatorsNexttoEqualSign(e, exprString))
                {
                    return false;
                }
            }
            else if ( (c == '+') && !isExpNot(exprString, e) && contains(evalOperators, '+') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '-') && !isExpNot(exprString, e) && contains(evalOperators, '-') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '|') && contains(evalOperators, '|') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '*') && contains(evalOperators, '*') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '/') && contains(evalOperators, '/') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '&') && contains(evalOperators, '&') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '^') && contains(evalOperators, '^') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '<') && contains(evalOperators, '<') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '>') && contains(evalOperators, '>') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            // Make sure that next character is not also an operator (not allowed right now)
            if (foundOperatorAtThisLocation && !fixMultiOperators(e, exprString))
            {
                // Error in parsing
                return false;
            }
        }
    }
    breakpts.push_back(e);

    // Add expression
    if (foundOperator)
    {
        if (contains(evalOperators, "+-|"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                //char op = stripInitialSign(left);
                char op = left[0];
                if (op == '-')
                {
                    rExprList.push_back(Expression(left.substr(1), SubtractionT));
                }
                else if (op == '+')
                {
                    rExprList.push_back(Expression(left.substr(1), AdditionT));
                }
                else if (op == '|')
                {
                    rExprList.push_back(Expression(left.substr(1), OrT));
                }
                else
                {
                    rExprList.push_back(Expression(left, AdditionT));
                }
            }
        }
        else if (contains(evalOperators, "*/&"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                char op = left[0];
                if (op == '/')
                {
                    rExprList.push_back(Expression(left.substr(1), DivisionT));
                }
                else if (op == '*')
                {
                    rExprList.push_back(Expression(left.substr(1), MultiplicationT));
                }
                else if (op == '&')
                {
                    rExprList.push_back(Expression(left.substr(1), AndT));
                }
                else
                {
                    rExprList.push_back(Expression(left, AdditionT));
                }
            }
        }
        else if (contains(evalOperators, "=^<>"))
        {
            if (breakpts.size() == 3)
            {
                std::string left = exprString.substr(breakpts[0], breakpts[1]-breakpts[0]);
                std::string right = exprString.substr(breakpts[1], breakpts[2]-breakpts[1]);
                char op = right[0];
                if (op == '=')
                {
                    rExprList.push_back(Expression(left, right.substr(1), AssignmentT));
                }
                else if (op == '^')
                {
                    rExprList.push_back(Expression(left, right.substr(1), PowerT));
                }
                else if (op == '<')
                {
                    rExprList.push_back(Expression(left, right.substr(1), LessThenT));
                }
                else
                {
                    rExprList.push_back(Expression(left, right.substr(1), GreaterThenT));
                }
            }
            else
            {
                // Error in parsing
                return false;
            }
        }
    }

    // No error in parsing (even if we did not find anything)
    return true;
}

//! @brief Process an expression string recursively to build an expression tree
//! @param[in] exprString The expression string to process
//! @param[out] rExprList A list of the resulting expression branches
bool interpretExpressionStringRecursive(std::string exprString, std::list<Expression> &rExprList)
{
    bool branchOK;
    branchOK = branchExpressionOnOperator(exprString, "=", rExprList);
    if (branchOK && rExprList.empty())
    {
        branchOK = branchExpressionOnOperator(exprString, "+-|", rExprList);
        if (branchOK && rExprList.empty())
        {
            branchOK = branchExpressionOnOperator(exprString, "*/&", rExprList);
            if (branchOK && rExprList.empty())
            {
                branchOK = branchExpressionOnOperator(exprString, "^<>", rExprList);
                if (branchOK && rExprList.empty())
                {
                    // This must be a value
                    rExprList.push_back(Expression(exprString, ValueT));
                }
            }
        }
    }
    return branchOK;
}

//! @brief Process an expression string recursively to build an expression tree
//! @param[in] exprString The expression string to process
//! @param[out] rExpr The resulting expression tree
bool interpretExpressionStringRecursive(std::string exprString, Expression &rExpr)
{
    rExpr = Expression(exprString, AdditionT);
    return rExpr.isValid();
}

//! @brief Default constructor
Expression::Expression()
{
    commonConstructorCode();
}

//! @brief Copy constructor
Expression::Expression(const Expression &other)
{
    copyFromOther(other);
}

//! @brief Constructor taking one expression string (rhs)
Expression::Expression(const std::string &exprString, ExpressionOperatorT op)
{
    commonConstructorCode();
    mRightExpressionString = exprString;
    removeAllWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator == ValueT)
    {
        if (!mRightExpressionString.empty())
        {
            mHasValue = true;
            mIsValid = true;
        }
    }
    else
    {
        mIsValid = interpretExpressionStringRecursive(mRightExpressionString, mRightChildExpressions);
        // If child expression is a value, then move the value into this expression
        if (mRightChildExpressions.size() == 1 && mRightChildExpressions.front().operatorType()==ValueT)
        {
            mRightExpressionString = mRightChildExpressions.front().rightExprString();
            mRightChildExpressions.clear();
            if (!mRightExpressionString.empty())
            {
                mHasValue = true;
                mIsValid = true;
            }
            else
            {
                mIsValid = false;
            }
        }
    }
}

//! @brief Constructor taking two expression strings (lhs and rhs)
Expression::Expression(const std::string &leftExprString, const std::string &rightExprString, ExpressionOperatorT op)
{
    commonConstructorCode();
    mLeftExpressionString = leftExprString;
    removeAllWhitespaces(mLeftExpressionString);
    stripLeadingTrailingParanthesis(mLeftExpressionString, mHadLeftOuterParanthesis);
    mRightExpressionString = rightExprString;
    removeAllWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator == AssignmentT)
    {
        mRightChildExpressions.push_back(Expression(mRightExpressionString, AdditionT));
    }
    else
    {
        mLeftChildExpressions.push_back(Expression(mLeftExpressionString, AdditionT));
        mRightChildExpressions.push_back(Expression(mRightExpressionString, AdditionT));
    }
    mIsValid = true;
}

//! @brief The assignment operator
Expression &Expression::operator=(const Expression &other)
{
    // Check for self-assignment
    if (this == &other)
    {
        return *this;
    }

    // Copy
    copyFromOther(other);

    // Return this
    return *this;
}

//! @brief Check if this expression is empty
bool Expression::empty() const
{
    if (hasValue())
    {
        return mRightExpressionString.empty();
    }
    else
    {
        return mRightChildExpressions.empty();
    }
}

//! @brief Check if this expression has a value or variable in its right expression string
bool Expression::hasValue() const
{
    return mHasValue;
}

//! @brief Check if this expression represents a constant value
//! @note Evaluate must have been called at least once before this function returns a relevant value
bool Expression::hasConstantValue() const
{
    return mHasConstantValue;
}

//! @brief Recursively check if an expression tree is valid after interpretation
bool Expression::isValid() const
{
    if (!mIsValid)
    {
        return false;
    }

    std::list<Expression>::const_iterator it;
    for (it=mLeftChildExpressions.begin(); it!=mLeftChildExpressions.end(); ++it)
    {
        if (!it->isValid())
        {
            return false;
        }
    }

    for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it)
    {
        if (!it->isValid())
        {
            return false;
        }
    }

    return true;
}

//! @brief Returns the (right hand side) expression string (without outer parenthesis)
const std::string &Expression::exprString() const
{
    return rightExprString();
}

//! @brief Returns the left hand side expression string (without outer parenthesis)
const std::string &Expression::leftExprString() const
{
    return mLeftExpressionString;
}

//! @brief Returns the right hand side expression string (without outer parenthesis)
const std::string &Expression::rightExprString() const
{
    return mRightExpressionString;
}

//! @brief Returns the operator type
ExpressionOperatorT Expression::operatorType() const
{
    return mOperator;
}

//! @brief Evaluate the expression
//! @param[in,out] rVariableStorage The variable storage to use for setting or getting variable values
//! @param[out] rEvalOK Indicates whether evaluation was successful or not
//! @return The value of the evaluated expression
double Expression::evaluate(VariableStorage &rVariableStorage, bool &rEvalOK)
{
    bool lhsOK=false,rhsOK=false;
    double value=0;

    // If this is a constant value, then return it
    if (mHasConstantValue)
    {
        // We take a shortcut here and return immediately
        rEvalOK = true;
        return mConstantValue;
    }
    // If this expression contains a value then evaluate it
    else if (mHasValue)
    {
        lhsOK=true;
        char* pEnd;
        value = strtod(mRightExpressionString.c_str(), &pEnd);
        rhsOK = (pEnd != mRightExpressionString.c_str()) &&
                (pEnd == mRightExpressionString.c_str()+mRightExpressionString.size());
        if (!rhsOK)
        {
            // The "value" seems to be a variable name
            // Lookup variable name in variable storage instead
            value = rVariableStorage.value(mRightExpressionString, rhsOK);
        }
        else
        {
            // If we could evaluate the string directly then this is a constant value
            // we can remember that so that the next evaluation is faster
            mHasConstantValue = true;
            mConstantValue = value;
        }
    }
    else if (mOperator == AssignmentT)
    {
        // Try to assign variable
        bool dummy;
        value = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        if (rhsOK)
        {
           lhsOK = rVariableStorage.setVariable(mLeftExpressionString, value, dummy);
        }
    }
    else if (mOperator == PowerT)
    {
        // Evaluate both sides
        double base = mLeftChildExpressions.front().evaluate(rVariableStorage, lhsOK);
        double exp = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        value = pow(base,exp);
    }
    else if (mOperator == LessThenT)
    {
        // Evaluate both sides
        double l = mLeftChildExpressions.front().evaluate(rVariableStorage, lhsOK);
        double r = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        value = double(l<r);
    }
    else if (mOperator == GreaterThenT)
    {
        // Evaluate both sides
        double l = mLeftChildExpressions.front().evaluate(rVariableStorage, lhsOK);
        double r = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        value = double(l>r);
    }
    else
    {
        lhsOK=true;
        std::list<Expression>::iterator it;
        for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it)
        {
            ExpressionOperatorT optype = it->operatorType();
            double newValue = it->evaluate(rVariableStorage, rhsOK);
            if (optype == AdditionT )
            {
                value += newValue;
            }
            else if (optype == SubtractionT)
            {
                value -= newValue;
            }
            else if (optype == MultiplicationT)
            {
                value *= newValue;
            }
            else if (optype == DivisionT)
            {
                value /= newValue;
            }
            else if (optype == OrT)
            {
                value = boolify(boolify(value)+boolify(newValue));
            }
            else if (optype == AndT)
            {
                value = boolify(value)*boolify(newValue);
            }
            else if (optype != UndefinedT)
            {
                value = newValue;
            }
            else
            {
                rEvalOK=false;
                return value;
            }
            // If evaluation error in child expression, abort and return false
            if (!rhsOK)
            {
                rEvalOK=false;
                return value;
            }
        }
    }

    rEvalOK = (lhsOK && rhsOK);
    return value;
}

//! @brief Prints the expression (as it will be evaluated) to a string
std::string Expression::print()
{
    std::string fullexp;

    if (mOperator == AssignmentT)
    {
        std::string r = mRightChildExpressions.front().print();
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = mLeftExpressionString+"="+r;
    }
    else if (mOperator == PowerT)
    {
        std::string l = mLeftChildExpressions.front().print();
        std::string r = mRightChildExpressions.front().print();
        if (mHadLeftOuterParanthesis)
        {
            l="("+l+")";
        }
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = l+"^"+r;
    }
    else if (mOperator == LessThenT)
    {
        std::string l = mLeftChildExpressions.front().print();
        std::string r = mRightChildExpressions.front().print();
        if (mHadLeftOuterParanthesis)
        {
            l="("+l+")";
        }
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = l+"<"+r;
    }
    else if (mOperator == GreaterThenT)
    {
        std::string l = mLeftChildExpressions.front().print();
        std::string r = mRightChildExpressions.front().print();
        if (mHadLeftOuterParanthesis)
        {
            l="("+l+")";
        }
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = l+">"+r;
    }
    else if (mHasValue)
    {
        fullexp = mRightExpressionString;
        if (mHadRightOuterParanthesis)
        {
            fullexp = "("+fullexp+")";
        }
    }
    else
    {
        std::list<Expression>::iterator it;
        for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it)
        {
            ExpressionOperatorT optype = it->operatorType();
            if (optype == AdditionT)
            {
                fullexp += "+";
            }
            else if (optype == SubtractionT)
            {
                fullexp += "-";
            }
            else if (optype == MultiplicationT)
            {
                fullexp += "*";
            }
            else if (optype == DivisionT)
            {
                fullexp += "/";
            }
            else if (optype == OrT)
            {
                fullexp += "|";
            }
            else if (optype == AndT)
            {
                fullexp += "&";
            }
            fullexp += it->print();
        }
        stripInitialPlus(fullexp);
        if (mHadRightOuterParanthesis)
        {
            fullexp = "("+fullexp+")";
        }
    }
    return fullexp;
}

void Expression::commonConstructorCode()
{
    mOperator = UndefinedT;
    mHadRightOuterParanthesis = false;
    mHadLeftOuterParanthesis = false;
    mHasValue = false;
    mHasConstantValue = false;
    mIsValid = false;
    mConstantValue = 0;
}

//! @brief Copy from other expression (help function for assignment and copy constructor)
//! @param[in] other The expression to copy from
void Expression::copyFromOther(const Expression &other)
{
    mOperator = other.mOperator;
    mHadLeftOuterParanthesis = other.mHadLeftOuterParanthesis;
    mHadRightOuterParanthesis = other.mHadRightOuterParanthesis;
    mLeftChildExpressions = other.mLeftChildExpressions;
    mRightChildExpressions = other.mRightChildExpressions;
    mLeftExpressionString = other.mLeftExpressionString;
    mRightExpressionString = other.mRightExpressionString;
    mHasValue = other.mHasValue;
    mHasConstantValue = other.mHasConstantValue;
    mConstantValue = other.mConstantValue;
    mIsValid = other.mIsValid;
}

}
