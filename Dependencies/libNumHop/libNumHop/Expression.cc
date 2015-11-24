#include "Expression.h"
#include "Helpfunctions.h"
#include <cstdlib>
#include <cmath>
#include <vector>

namespace numhop {

std::string allOperators="=+-*/^";

// Internal help function
bool checkForDoubleOperators(size_t e, const std::string &expr)
{
    if (e<expr.size()-1 && contains(allOperators, expr[e+1]))
    {
         return true;
    }
    if (e>0 && contains(allOperators, expr[e-1]))
    {
        return true;
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
            bool foundOperatorAtThisLocation=false;
            // Check for assignment, (can only have one assignment in expression)
            if ( (c == '=') && contains(evalOperators, '=') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '+') && contains(evalOperators, '+') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '-') && contains(evalOperators, '-') )
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
            else if ( (c == '^') && contains(evalOperators, '^') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            // Make sure that next character is not also an operator (not allowed right now)
            if (foundOperatorAtThisLocation && checkForDoubleOperators(e, exprString))
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
        if (contains(evalOperators, "+-"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                char sign = stripInitialSign(left);
                if (sign == '-')
                {
                    rExprList.push_back(Expression(left, SubtractionT));
                }
                else if (sign == '+')
                {
                    rExprList.push_back(Expression(left, AdditionT));
                }
            }
        }
        else if (contains(evalOperators, "*/"))
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
                else
                {
                    rExprList.push_back(Expression(left, AdditionT));
                }
            }
        }
        else if (contains(evalOperators, "=^"))
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
                else
                {
                    rExprList.push_back(Expression(left, right.substr(1), PowerT));
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
        branchOK = branchExpressionOnOperator(exprString, "+-", rExprList);
        if (branchOK && rExprList.empty())
        {
            branchOK = branchExpressionOnOperator(exprString, "*/", rExprList);
            if (branchOK && rExprList.empty())
            {
                branchOK = branchExpressionOnOperator(exprString, "^", rExprList);
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
    return !rExpr.empty();
}

//! @brief Default constructor
Expression::Expression()
{
    mOperator = UndefinedT;
    mHadRightOuterParanthesis = false;
    mHadLeftOuterParanthesis = false;
}

//! @brief Copy constructor
Expression::Expression(const Expression &other)
{
    copyFromOther(other);
}

//! @brief Constructor taking one expression string (rhs)
Expression::Expression(const std::string &exprString, ExpressionOperatorT op)
{
    mRightExpressionString = exprString;
    removeAllWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator != ValueT)
    {
        interpretExpressionStringRecursive(mRightExpressionString, mRightChildExpressions);
    }
}

//! @brief Constructor taking two expression strings (lhs and rhs)
Expression::Expression(const std::string &leftExprString, const std::string &rightExprString, ExpressionOperatorT op)
{
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
    if (isValue())
    {
        return mRightExpressionString.empty();
    }
    else
    {
        return mRightChildExpressions.empty();
    }
}

//! @brief Check if this expression represents a value or variable
bool Expression::isValue() const
{
    return mOperator == ValueT;
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

    if (mOperator == ValueT)
    {
        lhsOK=true;
        char* pEnd;
        value = strtod(mRightExpressionString.c_str(), &pEnd);
        rhsOK = (pEnd != mRightExpressionString.c_str());
        if (!rhsOK)
        {
            value = rVariableStorage.value(mRightExpressionString, rhsOK);
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
            else if (optype == ValueT || optype == AssignmentT || optype == PowerT)
            {
                value = newValue;
            }
            // If evaluation error in child expression, abort and return false
            if (!rhsOK)
            {
                return false;
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
    else if (mOperator == ValueT)
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
}

}
