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

//!
//! @file   SymHop.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-07-19
//!
//! @brief Contains the SymHop library for symbolic expressions
//!
//$Id$

#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>

#include "SymHop.h"

using namespace std;
using namespace SymHop;

// Local help functions
bool fuzzyEqual(const double &x, const double &y)
{
    if(y>0)
        return (x<=y*1.0000001 && x*1.0000001>=y);
    else
        return (x>=y*1.0000001 && x*1.0000001<=y);
}



void hAssert(const bool cond)
{
    if(!cond)
    {
        qDebug() << "hAssert FAILED!";
    }
}
//---------------------------------------------------

//! @class Expression
//! @brief The Expression class implement a class for symbolic expressions
//! @author Robert Braun <robert.braun@liu.se>
//!
//! Expressions are stored in a tree structures.
//!
//! Allowed operators are:
//! "*", "/", "+", "%"
//! (and "^", but it is to be replaced by pow() function)
//!
//! Allowed functions are:
//! "div", "rem", "mod", "tan", "cos", "sin", "atan", "acos", "asin", "atan2",
//! "sinh", "cosh", "tanh", "log", "exp", "sqrt", "sign", "abs", "der", "onPositive", "onNegative",
//! "signedSquareL", "limit", "integer", "floor", "ceil", "hopsanLimit", "hopsanDxLimit", "onPositive",
//! "onNegative", "signedSquareL", "limit", "nonZero"



//! @brief Copy constructor for non-const reference
//! @param other Expression to copy from
Expression::Expression(Expression &other)
{
    this->replaceBy(other);
}

//! @brief Copy constructor for const reference
//! @param other Expression to copy from
Expression::Expression(const Expression &other)
{
    this->replaceBy(other);
}


//! @brief Constructor for Expression class using QString
//! @param indata String containing a numerical expression
//! @param simplifications Specifies the degree of simplification
Expression::Expression(const QString indata, bool *ok, const ExpressionSimplificationT simplifications)
{
    bool isDouble;
    QString temp = QString::number(indata.toDouble(&isDouble), 'f', 20);

    if(!ok)
    {
        ok = new bool;
    }

    //If it is a number, make sure it has correct precision and remove extra zeros at end. Otherwise use original string.
    if(isDouble)
    {
        while(temp.endsWith("00")) { temp.chop(1); }
        commonConstructorCode(QStringList() << temp, *ok, simplifications);
    }
    else
    {
        commonConstructorCode(QStringList() << indata, *ok, simplifications);
    }
}


//! @brief Constructor for Expression class using QStringList
//! @details This constructor is faster than the one using a string, because the string does not need to be parsed.
//! @param symbols String list containing numerical symbols
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree, this shall never be used when defining a new expression
//FIXED
Expression::Expression(QStringList symbols, const ExpressionSimplificationT simplifications)
{
    bool dummy;
    commonConstructorCode(symbols, dummy, simplifications);
}


////! @brief Constructor for Expression using left and right expressions and an operator
////! @details This is the fastest way to create a new expression of operator type, because no evaluations or parsing needs to be performed.
////! @param left Left side Expression
////! @param right Right side expression
////! @param mid Operator ("+", "*" or "/")
////! @param simplifications Specifies the degree of simplification
//Expression::Expression(const Expression left, const QString mid, const Expression right, const ExpressionSimplificationT simplifications)
//{
//    if(mid == "+" || mid == "*" || mid == "/" || mid == "=")
//    {
//        mType = Expression::Operator;
//        mString = mid;
//        mChildren.append(left);
//        mChildren.append(right);
//        _simplify(simplifications);
//    }
//}


////! @brief Constructor for Expression using a list of expressions joined by operators
////! @example Expression([A,B,C,D], "+") will result in A+B+C+D
////! @param children List of child expressions
////! @param separator Operator used to combine child expressions ("+", "*" or "/")
////! @param simplifications Specifies the degree of simplification
////FIXED
//Expression::Expression(const QList<Expression> children, const QString separator)
//{
//    if(separator == "+")
//    {
//        mTerms = children;
//    }
//    else if(separator == "*")
//    {
//        mFactors = children;
//    }
//    else if(separator == "/")
//    {
//        mFactors.append(children[0]);
//        mDivisors = children.mid(1, children.size()-1);
//    }
//}


//! @brief Constructor for creating an expression from a numerical value
//! @param value Value of new symbol
//FIXED
Expression::Expression(const double value)
{
    mpLeft = nullptr;
    mpRight = nullptr;
    mpBase = nullptr;
    mpPower = nullptr;
    mpDividend = nullptr;

    if(value < 0) {
        this->replaceBy(Expression::fromTwoFactors(Expression("-1"), Expression(-1*value)));
        return;
    }
    mString = QString::number(value, 'f', 20);
    while(mString.endsWith("00")) { mString.chop(1); }
}


//! @brief Destructor
Expression::~Expression()
{
    this->cleanUp();
}


//! @brief Recursive cleanup function
void Expression::cleanUp()
{
    if(mpLeft)
    {
        mpLeft->cleanUp();
        delete mpLeft;
        mpLeft = nullptr;
    }
    if(mpRight)
    {
        mpRight->cleanUp();
        delete mpRight;
        mpRight = nullptr;
    }
    if(mpBase)
    {
        mpBase->cleanUp();
        delete mpBase;
        mpBase = nullptr;
    }
    if(mpPower)
    {
        mpPower->cleanUp();
        delete mpPower;
        mpPower = nullptr;
    }
    if(mpDividend)
    {
        mpDividend->cleanUp();
        delete mpDividend;
        mpDividend = nullptr;
    }
}

//! @brief Common constructor code that constructs an Expression from a string list
//! @param symbols String list containing numerical symbols
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree
//FIXED
void Expression::commonConstructorCode(QStringList symbols, bool &ok, const ExpressionSimplificationT simplifications)
{
    ok = true;

    mpLeft = nullptr;
    mpRight = nullptr;
    mpBase = nullptr;
    mpPower = nullptr;
    mpDividend = nullptr;

    //Only one symbol, so parse it as a string
    if(symbols.size() == 1)
    {
        QString str = symbols.first();
        symbols.clear();

        if(str.count("(") != str.count(")"))
        {
            ok = false;
            gSymHopMessages << "Unbalanced paretheses in expression: " << str;
            return;
        }

        //Don't create empty expressions
        if(str.isEmpty())
        {
            ok = false;
            return;
        }

        if(str.startsWith("-(") && str.endsWith(")"))
        {
            str.remove(0, 1);
            str.insert(0, "-1*");
        }

        //Replace power operators with pow() functions
        for(int i=0; i<str.size(); ++i)
        {
            if(str[i] == '^')
            {
                str[i] = ',';
                int j=i-1;
                int parBal = 0;
                while(j>=0 && (str[j].isLetterOrNumber() || str[j] == '.' || str[j] == '_' || str[j] == ')' || (str[j] == '(' && parBal != 0) || parBal > 0
                               || (j>=1 && str[j]=='-' && str[j-1] == 'e') || (j>=1 && str[j]=='+' && str[j-1] == 'e')))
                {
                    if(str[j] == ')') parBal++;
                    if(str[j] == '(') parBal--;
                    --j;
                }
                ++j;
                str.insert(j, "pow(");

                j = i+6;
                parBal = 0;
                while(j<str.size() && (str[j].isLetterOrNumber() || str[j] == '.' || str[j] == '_' || str[j] == '(' || (str[j] == ')' && parBal != 0) || parBal > 0))
                {
                    if(str[j] == ')') parBal--;
                    if(str[j] == '(') parBal++;
                    ++j;
                }
                str.insert(j, ")");
            }
        }

        //Trivial simplifications before parsing
        if(str.size() > 1)
        {
            str.replace("**", "^");
            str.remove(" ");
            str.replace("--", "+");
            str.replace("-", "+-");
            str.replace("--", "+");
            str.replace("/+", "/");
            str.replace("*+", "*");
            str.replace("^+", "^");
            str.replace("&&", "{");
            str.replace("||", "}");
            str.replace("!=", "~"); //! @todo Ugly solutions to rename like this
            str.replace("==", "!"); //! @todo Ugly solutions to rename like this
            str.replace(">=", "?");
            str.replace("<=", "¤");
        }
        while(str.contains("++")) { str.replace("++", "+"); }
        while(str.contains("+-+-")) { str.replace("+-+-","+-"); }
        //while(str.contains("-(")) { str.replace("-(", "(-"); }
        while(str.startsWith("+")) { str = str.right(str.size()-1); }
        while(str.contains("=+")) { str.replace("=+", "="); }
        while(str.contains("~+")) { str.replace("~+", "~"); }
        while(str.contains("!+")) { str.replace("!+", "!"); }
        while(str.contains("{+")) { str.replace("{+", "{"); }
        while(str.contains("}+")) { str.replace("}+", "}"); }

        //Remove all excessive parentheses
        while(str.startsWith("(") && str.endsWith(")"))
        {
            QString testString = str.mid(1, str.size()-2);
            if(verifyParantheses(testString)) { str = testString; }
            else { break; }
        }

        //Remove "+" sign at the beginning
        while(str.startsWith("+"))
        {
            str = str.right(str.size()-1);
        }

        //Generate a list of symbols from the string
        bool var = false;   //True if current symbol might be  a variable
        int parBal = 0;     //Parenthesis balance counter
        int start=0;        //Start index of each symbol
        for(int i=0; i<str.size(); ++i)
        {
            if(!var && parBal==0 && (str.at(i).isLetterOrNumber() || str.at(i) == '_' || str.at(i) == '-' || str.at(i) == '.' || str.at(i) == ':' || str.at(i) == '"')) //New variable or function string
            {
                var = true;
                start = i;
            }
            else if(var && str.at(i) == '(')    //String contains parentheses, so it is not a variable (but a function)
            {
                var = false;
                parBal++;
            }
            else if(!var && str.at(i) == '(')   //New string that begins with parenthesis
            {
                parBal++;
            }
            else if(str.at(i) == ')')      //End parenthesis, append string to symbols
            {
                parBal--;
                if(parBal == 0)
                {
                    var = false;
                    symbols.append(str.mid(start, i-start+1));
                }
            }
            else if(var && !(str.at(i).isLetterOrNumber() || str.at(i) == '|' || str.at(i) == '_' || str.at(i) == '.' || str.at(i) == ':'  || str.at(i) == '@' || str.at(i) == '"' ||
                             (i>1 && str.size() > i+2 && str.at(i-2).isNumber() && str.at(i-1) == 'e' && str.at(i) == '+' && str.at(i+1) == '-' && str.at(i+2).isNumber()) ||
                             (i>1 && str.size() > i+2 && str.at(i-2).isNumber() && str.at(i-1) == 'e' && str.at(i) == '-' && str.at(i+1) == '+' && str.at(i+2).isNumber()) ||
                             (i>1 && str.size() > i+1 && str.at(i-2).isNumber() && str.at(i-1) == 'e' && str.at(i) == '+' && str.at(i+1).isNumber()) ||
                             (i>2 && str.size() > i+1 && str.at(i-3).isNumber() && str.at(i-2) == "e" && str.at(i-1) == '+' && str.at(i) == '-' && str.at(i+1).isNumber()) ||
                             (i>2 && str.size() > i+1 && str.at(i-3).isNumber() && str.at(i-2) == "e" && str.at(i-1) == '-' && str.at(i) == '+' && str.at(i+1).isNumber()) ||
                             (i>1 && str.size() > i+2 && str.at(i-2) == '.' && str.at(i-1) == 'e' && str.at(i) == '+' && str.at(i+1) == '-' && str.at(i+2).isNumber()) ||
                             (i>1 && str.size() > i+2 && str.at(i-2) == '.' && str.at(i-1) == 'e' && str.at(i) == '-' && str.at(i+1) == '+' && str.at(i+2).isNumber()) ||
                             (i>1 && str.size() > i+1 && str.at(i-2) == '.' && str.at(i-1) == 'e' && str.at(i) == '+' && str.at(i+1).isNumber()) ||
                             (i>2 && str.size() > i+1 && str.at(i-3) == '.' && str.at(i-2) == "e" && str.at(i-1) == '+' && str.at(i) == '-' && str.at(i+1).isNumber()) ||
                             (i>2 && str.size() > i+1 && str.at(i-3) == '.' && str.at(i-2) == "e" && str.at(i-1) == '-' && str.at(i) == '+' && str.at(i+1).isNumber())))     //End of variable, append it to symbols (last two checks makes sure that Xe+Y and Xe-Y are treated as one symbol)
            {
                var = false;
                symbols.append(str.mid(start, i-start));
                --i;
            }
            else if(!var && parBal == 0)    //Something else than a variable and a function (i.e. an operator), append it to the list of symbols
            {
                symbols.append(str.at(i));
                start = i+1;
            }


            if(i == str.size()-1 && !var && parBal == 0 && str.at(i) != ')')    //Make sure to append last symbol
                symbols.append(str.at(i));
            else if(i == str.size()-1 && (var || parBal>0) && str.at(i) != ')')
                symbols.append(str.mid(start, i-start+1));
        }
    }

    //Store reserved symbols in object
    reservedSymbols << "mTimestep" << "mTime" << "Z";


    //Find top level symbol, set correct string and type, generate children
    if(splitAtSeparator("=", symbols, simplifications)) {}                        //Assignment
    else if(splitAtSeparator("!", symbols, simplifications)) {}                  //Logical equality (replace with function)
    else if(splitAtSeparator("~", symbols, simplifications)) {}                  //Logical inequality (replace with function)
    else if(splitAtSeparator("{", symbols, simplifications)) {}                  //Logical and (replace with function)
    else if(splitAtSeparator("}", symbols, simplifications)) {}                  //Logical or (replace with function)
    else if(splitAtSeparator(">", symbols, simplifications)) {}                  //Logical greater than (replace with function)
    else if(splitAtSeparator("?", symbols, simplifications)) {}                  //Logical greater than or equal (replace with function)
    else if(splitAtSeparator("<", symbols, simplifications)) {}                  //Logical smaller than (replace with function)
    else if(splitAtSeparator("¤", symbols, simplifications)) {}                  //Logical smaller than or equal (replace with function)
    else if(splitAtSeparator("+", symbols, simplifications)) {}                  //Addition
    else if(splitAtSeparator("*", symbols, simplifications)) {}                  //Multiplication/division
    else if(splitAtSeparator("^", symbols, simplifications)) {}                  //Power
    else if(splitAtSeparator("%", symbols, simplifications)) {}                  //Modulus
    else if(symbols.size() == 1 && symbols.first().contains("(") && symbols.first().endsWith(")"))                   //Function
    {
        QString str = symbols.first();
        mFunction = str.left(str.indexOf("("));
        str = str.mid(str.indexOf("(")+1, str.size()-2-str.indexOf("("));
        QStringList args = splitWithRespectToParentheses(str, ',');

        //Power function, special case
        if(mFunction == "pow" && args.size() == 2)
        {
            mFunction.clear();
            mpBase = new Expression();
            (*mpBase) = args[0];
            mpPower = new Expression();
            (*mpPower) = args[1];
        }
        else if(mFunction == "-pow" && args.size() == 2)
        {
            mFunction.clear();
            mpBase = new Expression();
            (*mpBase) = args[0];
            mpPower = new Expression();
            (*mpPower) = args[1];
            this->replaceBy(Expression::fromTwoFactors(Expression("-1"), *this));
        }
        else
        {
            for(int i=0; i<args.size(); ++i)
            {
                mArguments.append(Expression(args.at(i),&ok,simplifications));
            }
            if(mFunction.startsWith("-"))
            {
                mFunction.remove(0, 1);
                this->replaceBy(Expression::fromTwoFactors(Expression("-1"), *this));
            }
        }
    }
    else                                                                            //Symbol
    {
        //mType = Expression::Symbol;
        if(symbols.isEmpty())
        {
            mString = "0";
        }
        else
        {
            mString = symbols.first();
        }
        //Make sure numerical symbols have double precision
        bool isInt;
        mString.toInt(&isInt);
        if(isInt && !mString.contains("."))
            mString.append(".0");

        //Convert ".X" to "0.X"
        if(mString.startsWith("."))
        {
            mString.prepend("0");
        }
        else if(mString.startsWith("-."))
        {
            mString.insert(1, "0");
        }

        //Replace Xe+-Y with Xe-Y
        mString.replace("e+-", "e-");

        //Replace negative symbols with multiplication with -1
        if(mString.startsWith("-") && mString != "-1.0")
        {
            mString = mString.right(mString.size()-1);
            this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("-1") << (*this), QList<Expression>()));
        }
    }

    //Perform simplifications (but not for symbols, because that is pointless)
    if(!this->isSymbol())
        _simplify(simplifications);

    if(mFactors.size() == 1 && mDivisors.isEmpty())
    {
        this->replaceBy(mFactors[0]);
    }
    else if(mFactors.isEmpty() && !mDivisors.isEmpty())
    {
        this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("1"), mDivisors));
    }
    else if(mTerms.size() == 1)
    {
        this->replaceBy(mTerms[0]);
    }
}


//! @brief Equality operator for expressions
bool Expression::operator==(const Expression &other) const
{
    bool functionOk = false;
    if(this->isFunction() != other.isFunction())
    {
        functionOk = false;
    }
    else if(mFunction == other.getFunctionName() && mArguments == other.getArguments())
    {
        functionOk = true;
    }
    bool baseOk = (!mpBase && !other.getBase()) || (mpBase && other.getBase() && *mpBase == (*other.getBase()));
    bool powerOk = (!mpPower && !other.getPower()) || (mpPower && other.getPower() && *mpPower == (*other.getPower()));
    bool equationOk = false;
    if(this->isEquation() != other.isEquation())    //One is equation but other one is not
    {
        equationOk = false;
    }
    else if(!mpLeft && !other.getLeft() && !mpRight && !other.getRight())   //Neither are equations
    {
        equationOk = true;
    }
    else if((mpLeft && other.getLeft() && *mpLeft == (*other.getLeft())) &&
            (mpRight && other.getRight() && *mpRight == (*other.getRight())))
    {
        equationOk = true;
    }
    else if((mpLeft && other.getRight() && *mpLeft == (*other.getRight())) &&
            (mpRight && other.getLeft() && *mpRight == (*other.getLeft())))
    {
        equationOk = true;
    }
    bool dividendOk = (!mpDividend && !other.getDividends()) || (mpDividend && other.getDividends() && *mpDividend == (*other.getDividends()));
    bool termsOk=true;
    if(this->isAdd() && other.isAdd()) {
        for(const Expression &term : mTerms) {
            if(other.getTerms().count(term) != mTerms.count(term)) {
                termsOk=false;
            }
        }
    }

    bool stringOk = (mString == other.mString);

    bool factorOk = (mFactors.size() == other.mFactors.size());
    if(this->isMultiplyOrDivide() && other.isMultiplyOrDivide() && factorOk) {
        for(const Expression &factor : mFactors) {
            if(other.getFactors().count(factor) != mFactors.count(factor)) {
                factorOk=false;
            }
        }
    }

    bool divisorOk = true;
    if(this->isMultiplyOrDivide() && other.isMultiplyOrDivide()) {
        for(const Expression &divisor : mDivisors) {
            if(other.getDivisors().count(divisor) != mDivisors.count(divisor)) {
                divisorOk=false;
            }
        }
    }

    return (stringOk &&
            termsOk &&
            divisorOk &&
            factorOk &&
            functionOk &&
            baseOk && powerOk && equationOk && dividendOk);
}


//! @brief Equality operator for expressions
//FIXED
bool Expression::operator!=(const Expression &other) const
{
    return !(*this == other);
}


//! @brief Assignment operator for expressions
//! @param other Expression to assign from
void Expression::operator=(const Expression &other)
{
    if(this != &other)
    {
        this->replaceByCopy(other);
    }
}


//! @brief Assignment operator for expressions
//! @param other Expression to assign from
void Expression::operator=(Expression &other)
{
    if(this != &other)
    {
        this->replaceByCopy(other);
    }
}


//! @brief Combines to expressions to an addition expression
//! @param term1 First term
//! @param term2 Second term
Expression Expression::fromTwoTerms(const Expression term1, const Expression term2)
{
    return Expression::fromTerms(QList<Expression>() << term1 << term2);
}


//! @brief Combines a list of expressions to an addition expression
//! @param terms List with term expressions
Expression Expression::fromTerms(const QList<Expression> terms)
{
    Expression ret;
    if(terms.size() == 1)
    {
        ret = terms.first();
    }
    else
    {
        ret.mTerms = terms;
    }
    return ret;
}


//! @brief Combines to expressions to a multiplication expression
//! @param factor1 First factor
//! @param factor2 Second factor
Expression Expression::fromTwoFactors(const Expression factor1, const Expression factor2)
{
    return Expression::fromFactorsDivisors(QList<Expression>() << factor1 << factor2, QList<Expression>());
}


//! @brief Combines to expressions to a division expression
//! @param factor Factor expression
//! @param divisor Divisor expression
Expression Expression::fromFactorDivisor(const Expression factor, const Expression divisor)
{
    return Expression::fromFactorsDivisors(QList<Expression>() << factor, QList<Expression>() << divisor);
}


//! @brief Combines two list of expression to a multiplication/division expression
//! @param factors List with factors
//! @param divisors List with divisors
Expression Expression::fromFactorsDivisors(const QList<Expression> factors, const QList<Expression> divisors)
{
    if(factors.isEmpty()) {
        gSymHopMessages << "In Expression::fromFactorsDivisors(): At least one factor is required. Returning Expression(0).";
    }

    if(factors.size() == 1 && divisors.isEmpty())
    {
        Expression ret;
        ret.replaceBy(factors[0]);
        return ret;
    }

    Expression ret;
    ret.mFactors = factors;
    ret.mDivisors = divisors;
    return ret;
}


//! @brief Combines to expressions to a power expression
//! @param base Base expression
//! @param power Power expression
Expression Expression::fromBasePower(const Expression base, const Expression power)
{
    Expression ret = Expression(QString());
    ret.mpBase = new Expression();
    (*ret.mpBase) = base;
    ret.mpPower = new Expression();
    (*ret.mpPower) = power;
    return ret;
}

Expression Expression::fromFunctionArgument(const QString function, Expression argument)
{
    return Expression::fromFunctionArguments(function, QList<Expression>() << argument);
}



//FIXED
Expression Expression::fromFunctionArguments(const QString function, const QList<Expression> arguments)
{
    Expression ret;
    bool neg=false;
    if(function.startsWith("-"))
    {
        ret.mFunction = function.right(function.size()-1);
        neg = true;
    }
    else
    {
        ret.mFunction = function;
    }
    ret.mArguments = arguments;
    if(neg)
    {
        ret.changeSign();
    }
    return ret;
}

//FIXED
Expression Expression::fromEquation(const Expression left, const Expression right)
{
    Expression ret;
    ret.mpLeft = new Expression();
    ret.mpRight = new Expression();
    (*ret.mpLeft) = left;
    (*ret.mpRight) = right;
    return ret;
}


//! @brief Evaluates the expression and returns the value
//!
//! Variables not included in the variables map will be treated as zero.
//! Unsupported functions will by definition return zero.
//! Derivatives will by definition return zero.
//! Equations cannot be evaluated and will by definition return zero.
//!
//! @param variables Map with variable names and values
//! @returns Evaluated value of expression
double Expression::evaluate(const QMap<QString, double> &variables, const QMap<QString, SymHopFunctionoid*> *functions, bool *ok) const
{
    if(!ok) {
        ok = new bool;
    }

    *ok=true;

    if(isAdd()) {
        double retval = 0;
        bool allOk = true;
        for(const Expression &term : mTerms) {
            bool thisOk;
            retval += term.evaluate(variables, functions, &thisOk);
            allOk *= thisOk;
        }
        if(allOk) {
            return retval;
        }
    }
    else if(isMultiplyOrDivide()) {
        double retval = 1;
        bool allOk = true;
        bool thisOk;
        for(const Expression &factor : mFactors) {
            retval *= factor.evaluate(variables, functions, &thisOk);
            allOk *= thisOk;
        }
        for(const Expression &divisor : mDivisors) {
            retval /= divisor.evaluate(variables, functions, &thisOk);
            allOk *= thisOk;
        }
        if(allOk) {
            return retval;
        }
    }
    else if(isPower())
    {
        bool ok1, ok2;
        double retval = pow(mpBase->evaluate(variables, functions, &ok1), mpPower->evaluate(variables, functions, &ok2));
        if(ok1 && ok2)
        {
           return retval;
        }
    }
    else if(isFunction())
    {
        double retval;
        if(functions && functions->contains(mFunction))
        {
            QString argString;
            for(int a=0; a<mArguments.size(); ++a)
            {
                argString.append(mArguments[a].toString()+",");
            }
            //qDebug() << "Evaluating: " << mFunction << " with arguments " << argString;
            argString.chop(1);
            bool ok;
            SymHopFunctionoid *pFunc = functions->find(mFunction).value();
            retval = (*pFunc)(argString, ok);
            if(ok)
            {
                //qDebug() << "Ok!";
                return retval;
            }
            //qDebug() << "Not ok!";
        }

        if(mFunction == "der") { return 0; }

        if(mArguments.size() == 0)
        {
            bool ok1=true;
            if(mFunction == "pi") {
                retval = M_PI;
                return retval;
            }
            else {
                ok1=false;
                return 0;
            }
        }
        else if(mArguments.size() == 1)
        {
            bool ok1=true;
            if(mFunction == "sin") { retval = sin(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "cos") { retval = cos(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "tan") { retval = tan(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "asin") { retval = asin(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "acos") { retval = acos(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "atan") { retval = atan(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "sinh") { retval = sinh(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "cosh") { retval = cosh(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "tanh") { retval = tanh(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "log") { retval = log(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "exp") { retval = exp(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "sqrt") { retval = sqrt(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "abs") { retval = fabs(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "integer") { retval = int(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "floor") { retval = floor(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "ceil") { retval = ceil(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "round") { retval = round(mArguments[0].evaluate(variables, functions, &ok1)); }
            else if(mFunction == "r2d") { retval = mArguments[0].evaluate(variables, functions, &ok1)*180.0/M_PI; }
            else if(mFunction == "d2r") { retval = mArguments[0].evaluate(variables, functions, &ok1)*M_PI/180.0; }
            else if(mFunction == "sign")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) >= 0.0)
                    retval = 1.0;
                else
                    retval = -1.0;
            }
            else if(variables.contains(this->toString()))
            {
                retval = variables.find(this->toString()).value();
            }
            else
            {
                ok1=false;
                return 0;
            }

            if(ok1)
            {
                return retval;
            }
        }
        else if(mArguments.size() == 2)
        {
            bool ok1=true;
            bool ok2=true;
            if(mFunction == "min") { retval = fmin(mArguments[0].evaluate(variables, functions, &ok1), mArguments[1].evaluate(variables, functions, &ok2)); }
            else if(mFunction == "max") { retval = fmax(mArguments[0].evaluate(variables, functions, &ok1), mArguments[1].evaluate(variables, functions, &ok2)); }
            else if(mFunction == "rem") { retval = fmod(mArguments[0].evaluate(variables, functions, &ok1), mArguments[1].evaluate(variables, functions, &ok2)); }
            else if(mFunction == "mod") { retval = fmod(mArguments[0].evaluate(variables, functions, &ok1), mArguments[1].evaluate(variables, functions, &ok2)); }
            else if(mFunction == "div") { retval = mArguments[0].evaluate(variables, functions, &ok1)/mArguments[1].evaluate(variables, functions, &ok2); }
            else if(mFunction == "atan2") { retval = atan2(mArguments[0].evaluate(variables, functions, &ok1), mArguments[1].evaluate(variables, functions, &ok2)); }
            else if(mFunction == "pow") { retval = pow(mArguments[0].evaluate(variables, functions, &ok1), mArguments[1].evaluate(variables, functions, &ok2)); }
            else if(mFunction == "equal" || mFunction == "eq")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) == mArguments[1].evaluate(variables, functions, &ok2))
                    retval=1;
                else
                    retval=0;
            }
            else if(mFunction == "notEqual")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) == mArguments[1].evaluate(variables, functions, &ok2))
                    retval=0;
                else
                    retval=1;
            }
            else if(mFunction == "logicalOr")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) != 0.0 || mArguments[1].evaluate(variables, functions, &ok2) != 0.0)
                    retval=1;
                else
                    retval=0;
            }
            else if(mFunction == "logicalAnd")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) != 0.0 && mArguments[1].evaluate(variables, functions, &ok2) != 0.0)
                    retval=1;
                else
                    retval=0;
            }
            else if(mFunction == "greaterThan")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) > mArguments[1].evaluate(variables, functions, &ok2))
                    retval=1;
                else
                    retval=0;
            }
            else if(mFunction == "greaterThanOrEqual")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) >= mArguments[1].evaluate(variables, functions, &ok2))
                    retval=1;
                else
                    retval=0;
            }
            else if(mFunction == "smallerThan")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) < mArguments[1].evaluate(variables, functions, &ok2))
                    retval=1;
                else
                    retval=0;
            }
            else if(mFunction == "smallerThanOrEqual")
            {
                if(mArguments[0].evaluate(variables, functions, &ok1) <= mArguments[1].evaluate(variables, functions, &ok2))
                    retval=1;
                else
                    retval=0;
            }
            else if(variables.contains(this->toString()))
            {
                retval = variables.find(this->toString()).value();
            }
            else
            {
                ok1=false;
                return 0;
            }

            if(ok1 && ok2)
            {
                return retval;
            }
        }
        else if(mArguments.size() == 3)
        {
            bool ok1=true;
            bool ok2=true;
            bool ok3=true;
            if(mFunction == "limit")
            {
                double val = mArguments[0].evaluate(variables, functions, &ok1);
                double min = mArguments[1].evaluate(variables, functions, &ok2);
                double max = mArguments[2].evaluate(variables, functions, &ok3);
                if(val > max) { retval = max; }
                else if(val < min) { retval = min; }
                else { retval = val; }
            }
            else if(variables.contains(this->toString()))
            {
                retval = variables.find(this->toString()).value();
            }
            else
            {
                ok1=false;
                return 0;
            }

            if(ok1 && ok2 && ok3)
            {
                return retval;
            }
        }
    }
    else if(isNumericalSymbol())
    {
        return this->toDouble();
    }
    else if(isVariable())
    {
        if(variables.contains(mString))
            return variables.find(mString).value();
    }
    else if(isEquation())
    {
        double retval;
        bool ok1, ok2;
        if(mpLeft->evaluate(variables, functions, &ok1) == mpRight->evaluate(variables, functions, &ok2)) {
            retval = 1;
        }
        else {
            retval = 0;
        }
        if(ok1 && ok2) {
            return retval;
        }
    }

    *ok=false;
    return 0;
}


//! @brief Replaces this expression by another one, but first makes a copy of the other expression
//! @note Use this version if replacing an expression with a sub-expression of itself
//! @param expr Expression to replace by
void Expression::replaceByCopy(const Expression expr)
{
    replaceBy(expr);
}


//! @brief Replaces this expression by another one
//! @param expr Expression to replace by
void Expression::replaceBy(const Expression &expr)
{
    mString = expr.mString;
    mFunction = expr.mFunction;

    QList<Expression> tempFactors;
    for(int f=0; f<expr.mFactors.size(); ++f)
    {
        tempFactors.append(expr.mFactors[f]);
    }
    mFactors.swap(tempFactors);

    QList<Expression> tempArguments;
    for(int a=0; a<expr.mArguments.size(); ++a)
    {
        tempArguments.append(expr.mArguments[a]);
    }
    mArguments.swap(tempArguments);

    QList<Expression> tempDivisors;
    for(int d=0; d<expr.mDivisors.size(); ++d)
    {
        tempDivisors.append(expr.mDivisors[d]);
    }
    mDivisors.swap(tempDivisors);

    QList<Expression> tempTerms;
    for(int t=0; t<expr.mTerms.size(); ++t)
    {
        tempTerms.append(expr.mTerms[t]);
    }
    mTerms.swap(tempTerms);

    mpBase = nullptr;
    mpPower = nullptr;
    mpLeft = nullptr;
    mpRight = nullptr;
    mpDividend = nullptr;
    if(expr.getBase())
    {
        //! @todo here (and below) we might have two problems,  first a potential memory leak, we just zero the pointers above
        //!       but if we delete the expressions here, what would happen when the "expr" argument happens to come from ourselves (crash)
        mpBase = new Expression();
        mpBase->replaceBy(*(expr.getBase()));
    }
    if(expr.getPower())
    {
        mpPower = new Expression();
        mpPower->replaceBy(*expr.getPower());
    }
    if(expr.getLeft())
    {
        mpLeft = new Expression();
        mpLeft->replaceBy(*expr.getLeft());
    }
    if(expr.getRight())
    {
        mpRight = new Expression();
        mpRight->replaceBy(*expr.getRight());
    }
    if(expr.getDividends())
    {
        mpDividend = new Expression();
        mpDividend->replaceBy(*expr.getDividends());
    }
}


//! @brief Divides the expression by specified divisor
//! @param div Divisor expression
//FIXED
void Expression::divideBy(const Expression div)
{
    assert(!(div == Expression(0.0)));
    this->replaceBy(Expression::fromFactorDivisor(*this, div));
    this->_simplify(TrivialSimplifications);
}


//! @brief Multiplies the expression by specified divisor
//! @param fac Factor expression
void Expression::multiplyBy(const Expression fac)
{
    this->replaceBy(Expression::fromTwoFactors(*this, fac));
    this->_simplify(TrivialSimplifications);
}


//! @brief Adds the expression with specified tern
//! @param tern Term expression
void Expression::addBy(const Expression term)
{
    this->replaceBy(Expression::fromTwoTerms(*this, term));
    this->_simplify(TrivialSimplifications);
}


//! @brief Subtracts the expression by specified term
//! @param tern Term expression
void Expression::subtractBy(const Expression term)
{
    Expression subTerm = term;
    subTerm.multiplyBy(Expression("-1"));
    this->replaceBy(Expression::fromTwoTerms(*this, subTerm));
    this->_simplify(TrivialSimplifications);
}


//! @brief Returns the expression converted to a string
QString Expression::toString() const
{
    QString ret;

    if(this->isSymbol()) {
        ret = mString;
    }
    else if(this->isFunction()) {
        ret = mFunction+"(";
        for(int i=0; i<mArguments.size(); ++i)
        {
            ret.append(mArguments[i].toString()+",");
        }
        if(!mArguments.isEmpty()) {
            ret.chop(1);
        }
        ret.append(")");
    }
    else if(this->isEquation()) {
        QString leftStr =mpLeft->toString();
        QString rightStr = mpRight->toString();
        ret = leftStr + "=" + rightStr;
    }
    else if(this->isAdd()) {
        for(const Expression &term : mTerms) {
            Expression tempTerm = term;
            QString termString;
            if(tempTerm.isNegative())
            {
                tempTerm.changeSign();
                if(ret.endsWith("+"))
                {
                    ret.chop(1);
                }
                ret.append("-");
            }
            termString = tempTerm.toString();
            ret.append(termString);
            ret.append("+");
        }
        ret.chop(1);
    }
    else if(this->isMultiplyOrDivide()) {
        int numMinus = mFactors.count(Expression("-1"))+mDivisors.count(Expression("-1"));
        bool isOdd = (numMinus%2 != 0);

        for(const Expression &factor : mFactors) {
            QString factString = factor.toString();
            if(factor.isAdd())
            {
                factString.prepend("(");
                factString.append(")");
            }
            ret.append(factString);
            ret.append("*");
        }
        if(mFactors.isEmpty()) {
            ret.append("1");
        }
        else {
            ret.chop(1);
        }
        if(!mDivisors.isEmpty()) { ret.append("/"); }
        if(mDivisors.size() > 1) { ret.append("("); }
        for(const Expression &divisor : mDivisors) {
            QString divString = divisor.toString();
            if(divisor.isAdd())
            {
                divString.prepend("(");
                divString.append(")");
            }
            ret.append(divString);
            ret.append("*");
        }
        if(mDivisors.size() > 0) { ret.chop(1); }
        if(mDivisors.size() > 1) { ret.append(")"); }
        ret.replace("*-1.0*","*");
        ret.remove("-1.0*");
        ret.remove("*-1.0");
        if(isOdd) {
            ret.prepend("-");
        }
    }
    else if(this->isPower())
    {
        QString baseStr = mpBase->toString();
        if(mpBase->isAdd() || mpBase->isMultiplyOrDivide())
        {
            baseStr.prepend("(");
            baseStr.append(")");
        }
        QString powerStr = mpPower->toString();
        if(mpPower->isAdd() || mpBase->isMultiplyOrDivide())
        {
            powerStr.prepend("(");
            powerStr.append(")");
        }
        ret = "pow("+baseStr+","+powerStr+")";
    }

    //Simplify output
    ret.replace("+-", "-");
    ret.replace("--", "+");

    return ret;
}

QString Expression::toLaTeX() const
{
    QString ret;

    if(this->isSymbol())
    {
        QStringList greekSymbols = {"alpha","beta","gamma","Gamma","delta","Delta","epsilon","varepsilon","zeta","eta","theta","vartheta","Theta","iota","kappa","lambda","Lambda","mu","nu","xi","Xi","pi","Pi","rho","varrho","sigma","Sigma","tau","upsilon","Upsilon","phi","varphi","Phi","chi","psi","Psi","omega","Omega"};

        if(greekSymbols.contains(mString)) {
            ret = "\\"+mString;
        }
        else if(this->isNumericalSymbol()) {
            ret = QString::number(mString.toDouble());
        }
        else {
            ret = mString;
        }
    }
    else if(this->isFunction())
    {
        QStringList builtinFunctions = {"sin","cos","tan","arcsin","arccos","arctan","cot","sec","csc","sinh","cosh","tanh","lg","ln","log","min","max"     };
        if(mFunction == "pi" && mArguments.size() == 0) {
            ret = "\\pi";
        }
        else if(mFunction == "sqrt") {
            ret = "\\sqrt{"+mArguments[0].toLaTeX()+"}";
        }
        else if(builtinFunctions.contains(mFunction)) {
            ret = "\\"+mFunction+"\\left("+mArguments[0].toLaTeX()+"\\right)";
        }
        else if(mFunction == "asin") {
            ret = "\\arcsin\\left("+mArguments[0].toLaTeX()+"\\right)";
        }
        else if(mFunction == "acos") {
            ret = "\\arccos\\left("+mArguments[0].toLaTeX()+"\\right)";
        }
        else if(mFunction == "atan") {
            ret = "\\arctan\\left("+mArguments[0].toLaTeX()+"\\right)";
        }
        else if(mFunction == "exp") {
            ret = "e^{"+mArguments[0].toLaTeX()+"}";
        }
        else if(mFunction == "abs") {
            ret = "\\left\\|"+mArguments[0].toLaTeX()+"\\right\\|";
        }
        else if(mFunction == "cos") {
            ret = "\\sin\\left("+mArguments[0].toLaTeX()+"\\right)";
        }
        else if(mFunction == "der") {
            if(mArguments[0].getFunctionName() == "der") {
                if(mArguments[0].getArgument(0).isSymbol()) {
                    ret = "\\dfrac{d^2 "+mArguments[0].getArgument(0).toLaTeX()+"}{dt^2}";
                }
                else {
                    ret = "\\dfrac{d^2}{dt^2}\\left("+mArguments[0].getArgument(0).toLaTeX()+"\\right)";
                }
            }
            else {
                if(mArguments[0].isSymbol()) {
                    ret = "\\dfrac{d "+mArguments[0].toLaTeX()+"}{dt}";
                }
                else {
                    ret = "\\dfrac{d}{dt}\\left("+mArguments[0].toLaTeX()+"\\right)";
                }
            }
        }
        else if(mFunction == "dder") {
            if(mArguments[0].isSymbol()) {
                ret = "\\dfrac{d^2 "+mArguments[0].toLaTeX()+"}{dt^2}";
            }
            else {
                ret = "\\dfrac{d^2}{dt^2}\\left("+mArguments[0].toLaTeX()+"\\right)";
            }
        }
        else {
            ret = "\\mathrm{"+mFunction+"}\\left(";
            for(int i=0; i<mArguments.size(); ++i)
            {
                ret.append(mArguments[i].toLaTeX()+",");
            }
            if(!mArguments.isEmpty()) {
                ret.chop(1);
            }
            ret.append("\\right)");
        }
    }
    else if(this->isEquation())
    {
        QString leftStr =mpLeft->toLaTeX();
        QString rightStr = mpRight->toLaTeX();
        ret = leftStr + "=" + rightStr;
    }
    else if(this->isAdd())
    {
        Q_FOREACH(const Expression &term, mTerms)
        {
            Expression tempTerm = term;
            QString termString;
            if(tempTerm.isNegative())
            {
                tempTerm.changeSign();
                if(ret.endsWith("+"))
                {
                    ret.chop(1);
                }
                ret.append("-");
            }
            termString = tempTerm.toLaTeX();
            ret.append(termString);
            ret.append("+");
        }
        ret.chop(1);
    }
    else if(this->isMultiplyOrDivide())
    {
        double num = this->getNumericalFactor();
        int numMinus = mFactors.count(Expression("-1"))+mDivisors.count(Expression("-1"));
        bool isOdd = (numMinus%2 != 0);

        if(QString::number(num) != "1") {
            ret.append(QString::number(num)+" ");
        }

        for(const Expression &factor : mFactors)
        {
            if(factor.isNumericalSymbol()) {
                continue;
            }
            QString factString = factor.toLaTeX();
            if(factor.isAdd() && mFactors.size() > 1)
            {
                factString.prepend("(");
                factString.append(")");
            }
            ret.append(factString);
            ret.append(" ");
        }
        if(mFactors.isEmpty())
        {
            ret.append("1");
        }
        else
        {
            ret.chop(1);
        }
        if(!mDivisors.isEmpty()) {
            ret.prepend("\\dfrac{");
            ret.append("}{"); }
        for(const Expression &divisor : mDivisors)
        {
            QString divString = divisor.toLaTeX();
            if(divisor.isAdd() && mDivisors.size() > 1)
            {
                divString.prepend("(");
                divString.append(")");
            }
            ret.append(divString);
            ret.append(" ");
        }
        if(mDivisors.size() > 0) { ret.chop(1); }
        if(mDivisors.size() > 0) { ret.append("}"); }
        ret.replace("*-1.0*","*");
        ret.remove("-1.0*");
        ret.remove("*-1.0");
        if(isOdd) {
            ret.prepend("-");
        }
    }
    else if(this->isPower())
    {
        QString baseStr = mpBase->toLaTeX();
        if(mpBase->isAdd() || mpBase->isMultiplyOrDivide())
        {
            baseStr.prepend("(");
            baseStr.append(")");
        }
        QString powerStr = mpPower->toLaTeX();
        if(mpPower->isAdd() || mpBase->isMultiplyOrDivide())
        {
            powerStr.prepend("(");
            powerStr.append(")");
        }
        if(powerStr.size() > 1) {
            powerStr.prepend("{");
            powerStr.append("}");
        }
        ret = baseStr+"^"+powerStr;
    }

    //Simplify output
    ret.replace("+-", "-");
    ret.replace("--", "");

    return ret;
}


//! @brief Converts all delay operators ("Z") to delay functions ("mDelay") and extracts the delay terms
//! @details This function assumes that the function is linearized, so that there are no Z operators in divisors.
//! @param delayTerms Reference to list of delayed terms; new terms are appended
//! @param delaySteps Reference to list of delay steps for each term; new values are appended
void Expression::toDelayForm(QList<Expression> &rDelayTerms, QStringList &rDelaySteps)
{
    //Generate list of terms
    QList<QList<Expression> > termMap;
    QList<Expression> terms = getTerms();

    //Cycle terms
    for (auto &term : terms)
    {
        int idx = 0;
        for(const auto &factor : term.getFactors())
        {
            if(factor == Expression("Z"))
            {
                idx = 1;
            }
            else if(factor.isPower() && (*factor.getBase()) == Expression("Z"))
            {
                idx = int(factor.getPower()->toDouble());
            }
        }

        //Remove all Z operators
        if(idx > 0)
        {
            term.replace(Expression("Z"), Expression(1.0));
            term._simplify(TrivialSimplifications, Recursive);
        }

        while(termMap.size() < idx+1)
        {
            termMap.append(QList<Expression>());
        }

        //Store delay term
        termMap[idx].append(term);
    }

    //Replace delayed terms with delay function and store delay terms and delay steps in reference vectors
    Expression retExpr = Expression(0);
    QStringList ret;
    for(int i=termMap.size()-1; i>0; --i)
    {
        if(!termMap[i].isEmpty())
        {
            QStringList delayTermSymbols;
            for(int t=0; t<termMap[i].size(); ++t)
            {
                delayTermSymbols << "("+termMap[i][t].toString()+")" << "+";
            }
            delayTermSymbols.removeLast();
            Expression delayTerm = Expression(delayTermSymbols);
            double num = delayTerm.getNumericalFactor();
            if(delayTerm.isNumericalSymbol()) {
                retExpr.addBy(Expression(num));
                continue;       //Don't store purely numerical delay factors (they will never change anyway)
            }
            delayTerm = delayTerm.removeNumericalFactors();

            delayTerm.factorMostCommonFactor();

            //Check if delay term already exists, if so re-use it
            int idx = rDelayTerms.size();
            for(int d=0; d<rDelayTerms.size(); ++d) {
                if(rDelayTerms[d] == delayTerm) {
                    if(int(rDelaySteps[d].toDouble()) == i) {
                        idx = d;
                    }
                }
            }

            retExpr.addBy(fromTwoFactors(Expression(num),fromFunctionArguments("mDelay"+QString::number(idx)+".getOldest", QList<Expression>())));

            if(idx == rDelayTerms.size()) {
                QString term = "mDelay"+QString::number(rDelayTerms.size(), 'f', 20)+".getOldest()";
                rDelayTerms.append(delayTerm);
                rDelaySteps.append(QString::number(i, 'f', 20));
                while(rDelaySteps.last().endsWith("00")) { rDelaySteps.last().chop(1); }
            }
        }
    }
    for(int t=0; t<termMap[0].size(); ++t)
    {
        retExpr.addBy(termMap[0][t]);
    }

    //Replace this expression by the new one
    //this->replaceBy(Expression(ret));
    this->replaceBy(retExpr);

    //Simplify
    this->_simplify(Expression::FullSimplification, Recursive);

    if(mpLeft) { mpLeft->toDelayForm(rDelayTerms, rDelaySteps); }
    if(mpRight) { mpRight->toDelayForm(rDelayTerms, rDelaySteps); }
    if(mpBase) { mpBase->toDelayForm(rDelayTerms, rDelaySteps); }
    if(mpPower) { mpPower->toDelayForm(rDelayTerms, rDelaySteps); }
    if(mpDividend) { mpDividend->toDelayForm(rDelayTerms, rDelaySteps); }

    for(int i=0; i<mFactors.size(); ++i) { mFactors[i].toDelayForm(rDelayTerms, rDelaySteps); }
    for(int i=0; i<mDivisors.size(); ++i) { mDivisors[i].toDelayForm(rDelayTerms, rDelaySteps); }
    for(int i=0; i<mArguments.size(); ++i) { mArguments[i].toDelayForm(rDelayTerms, rDelaySteps); }
    for(int i=0; i<mDivisors.size(); ++i) { mDivisors[i].toDelayForm(rDelayTerms, rDelaySteps); }
    for(int i=0; i<mTerms.size(); ++i) { mTerms[i].toDelayForm(rDelayTerms, rDelaySteps); }
}


//! @brief Converts expression to double if possible
//! @param ok True if success, false if failed (= not a numerical symbol)
double Expression::toDouble(bool *ok) const
{
    if(this->isSymbol()) {
        return mString.toDouble(ok);
    }
    else if(this->isMultiplyOrDivide() && mFactors.size() == 2 && mFactors.contains(Expression("-1"))) {
        double retval = 1;
        if(ok) { *ok = true; }
        for(const Expression &factor : mFactors) {
            bool ok2;
            retval *= factor.toDouble(&ok2);
            if(!ok2)
            {
                *ok = false;
            }
        }
        return retval;
    }
    else {
        if(ok) {
            *ok = false;
        }
        return 0.0;
    }
}


//! @brief Tells whether or not this is a power operator
bool Expression::isPower() const
{
    return !(mpPower == nullptr);
}


//! @brief Tells whether or not this is a multiplication or division
bool Expression::isMultiplyOrDivide() const
{
    return !mFactors.isEmpty();
}


//! @brief Tells whether or not this is an addition
bool Expression::isAdd() const
{
    return !mTerms.isEmpty();
}


//! @brief Tells wheter or not this is a function
bool Expression::isFunction() const
{
    return !mFunction.isEmpty();
}


//! @brief Tells whether or not this is a symbol
bool Expression::isSymbol() const
{
    return (!mString.isEmpty());
}


//! @brief Tells whether or not this is a numerical symbol
bool Expression::isNumericalSymbol() const
{
    return (!mString.isEmpty() && (mString[0].isNumber() || mString == "-1.0"));
}

bool Expression::isInteger() const
{
    return (this->isNumericalSymbol()) && (trunc(this->toDouble()) == this->toDouble());
}

//! @brief Tells whether or not this is a variable
bool Expression::isVariable() const
{
    return (!mString.isEmpty() && mString[0].isLetter());
}


//! @brief Tells whether or not this is an assignment
bool Expression::isAssignment() const
{
    return (this->isEquation() && (mpLeft->isSymbol()));
}


//! @brief Tells whether or not this is an equation
bool Expression::isEquation() const
{
    return (mpLeft != nullptr);
}


//! @brief Tells whether or not this is a negative symbol
bool Expression::isNegative() const
{
    if(this->isSymbol() && mString == "-1")
    {
        return true;
    }
    else if(mFactors.contains(Expression("-1")))
    {
        return true;
    }
    return false;
}


void Expression::changeSign()
{
    if(this->isNegative())
    {
        mFactors.removeOne(Expression("-1"));
        if(mFactors.size() == 1 && mDivisors.isEmpty())
        {
            this->replaceByCopy(mFactors.first());
        }
        else if(mFactors.isEmpty())
        {
            mFactors.append(Expression("1"));
        }
    }
    else
    {
        if(isMultiplyOrDivide())
        {
            mFactors << Expression("-1");
        }
        else
        {
            this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("-1") << (*this), QList<Expression>()));
        }
    }
}


//! @brief Returns the derivative of the expression
//! @param x Expression to differentiate with
//! @param ok True if successful, otherwise false
Expression Expression::derivative(const Expression x, bool &ok) const
{
    ok = true;
    Expression ret;

    //Derivative of self, return 1
    if(*this == x)
    {
        ret = Expression(1);
    }
    //Equality, differentiate left and right expressions
    else if(this->isEquation())
    {
        bool success;
        Expression left = mpLeft->derivative(x, success);
        if(!success) { ok = false; }
        Expression right = mpRight->derivative(x, success);
        if(!success) { ok = false; }
        ret = Expression::fromEquation(left, right);
    }
    //Function
    else if(this->isFunction())
    {
        Expression f = *this;
        Expression g;
        Expression dg;
        QString func = mFunction;
        if(!mArguments.isEmpty())
        {
            g = mArguments[0];    //First argument
            bool success;
            dg = mArguments[0].derivative(x, success);    //Derivative of first argument
            if(!success) { ok = false; }
        }

        bool negative = false;
        if(func.startsWith('-'))        //Remember that the function was negative
        {
            func = func.right(func.size()-1);
            negative = true;
        }

        //Custom functions
        if(func == "greaterThan" || func == "smallerThan" || func == "greaterThanOrEqual" || func == "smallerThanOrEqual" || func == "notEqual" || func == "equal") {
            ret = Expression(0);
        }
        else if(func == "log")
        {
            ret = Expression::fromFactorDivisor(dg, g);
        }
        else if(func == "exp")
        {
            ret = Expression::fromTwoFactors(dg, f);
        }
        else if(func == "tan")
        {
            Expression factor = fromTwoFactors(Expression(2), dg);
            Expression funcExpr = fromFunctionArguments("cos", QList<Expression>() << fromTwoFactors(Expression(2), g));
            Expression divisor = fromTwoTerms(funcExpr, Expression(1));
            ret = fromFactorDivisor(factor, divisor);
        }
        else if(func == "atan")
        {
            Expression divisor = fromTwoTerms(fromTwoFactors(g, g), Expression(1));
            ret = fromFactorDivisor(dg, divisor);
        }
        else if(func == "atan2")
        {
            Expression g = fromFactorDivisor(mArguments[0],mArguments[1]);
            bool success;
            dg = g.derivative(x, success);
            if(!success) { return false; }
            Expression divisor = fromTwoTerms(fromTwoFactors(g, g), Expression(1));
            ret = fromFactorDivisor(dg, divisor);
        }
        else if(func == "asin")
        {
            Expression term1 = Expression(1);
            Expression term2 = fromFactorsDivisors(QList<Expression>() << g << g << Expression(-1), QList<Expression>());
            Expression arg = fromTwoTerms(term1, term2);
            Expression root = fromFunctionArguments("sqrt", QList<Expression>() << arg);
            ret = fromFactorDivisor(dg, root);
        }
        else if(func == "acos")
        {
            Expression gNeg = g;
            gNeg.changeSign();
            Expression dgNeg = dg;
            dgNeg.changeSign();
            Expression arg = fromTwoTerms(Expression(1), fromTwoFactors(g, gNeg));
            Expression root = fromFunctionArguments("sqrt", QList<Expression>() << arg);
            ret = fromFactorDivisor(dgNeg, root);
        }
        else if(func == "mod")
        {
            ret = Expression(0);
        }
        else if(func == "rem")
        {
            ret = Expression(0);
        }
        else if(func == "sqrt")
        {
            Expression funcExpr = fromFunctionArguments("sqrt", QList<Expression>() << g);
            Expression divisor = fromTwoTerms(Expression(2), funcExpr);
            ret = fromFactorDivisor(dg, divisor);
        }
        else if(func == "sign")
        {
            ret = Expression(0);
        }
        else if(func == "re")
        {
            ret = Expression(0);
        }
        else if(func == "ceil")
        {
            ret = Expression(0);
        }
        else if(func == "floor")
        {
            ret = Expression(0);
        }
        else if(func == "int")
        {
            ret = Expression(0);
        }
        else if(func == "dxLimit")
        {
            ret = Expression(0);
        }
        else if(func == "dxLimit3")
        {
            ret = Expression(0);
        }
        else if(func == "mDelay")
        {
            ret = Expression(0);
        }
        else if(func.startsWith("mDelay"))
        {
            ret = Expression(0);
        }
        else if(func.startsWith("delay_") && func.contains(".getIdx"))
        {
            ret = Expression(0);
        }
        else if(func.startsWith("nonZero"))
        {
            ret = dg;
        }
        else if(func == "limit") {
                ret = dg;
        }
        else if(func == "pow")
        {
            if(g == Expression("Z") || g == Expression("-Z"))
            {
                ret = Expression(0);
            }
            else
            {
                bool success;
                Expression f = mArguments[0];
                Expression df = mArguments[0].derivative(x, success);
                if(!success) { ok = false; }
                Expression g = mArguments[1];
                Expression dg = mArguments[1].derivative(x, success);
                if(!success) { ok = false; }


                Expression factor1 = fromBasePower(f, fromTwoTerms(g, Expression("-1")));
                Expression term1 = fromTwoFactors(df, f);
                Expression funcExpr = fromFunctionArguments("log", QList<Expression>() << f);
                Expression term2 = fromFactorsDivisors(QList<Expression>() << f << funcExpr << dg, QList<Expression>());
                Expression factor2 = fromTwoTerms(term1, term2);

                ret = fromTwoFactors(factor1, factor2);
            }
        }
        else if(func == "max") {
            Expression g2,dg2;
            if(mArguments.size() < 2) {
                ok = false;
            }
            else {
                g2 = mArguments[1];
                g2.changeSign();
                bool success;
                dg2 = mArguments[1].derivative(x, success);    //Derivative of first argument
                if(!success) {
                    ok = false;
                }
            }
            ret = Expression::fromFunctionArguments("ifPositive",QList<Expression>() << Expression::fromTwoTerms(g,g2) << dg << dg2);
        }
        else if(func == "ifElse") {
            if(mArguments.size() == 2) {
                ret = Expression::fromFunctionArguments("ifElse", QList<Expression>() << g << dg);
            }
            else if(mArguments.size() == 3) {
                bool success;
                Expression dg2 = mArguments[1].derivative(x, success);
                if(!success) {
                    ok = false;
                }
                ret = Expression::fromFunctionArguments("ifElse", QList<Expression>() << g << dg << dg2);
            }
        }
        //No special function, so use chain rule
        else
        {
            if(getFunctionDerivative(func) == "")
            {
                gSymHopMessages << "In Expression::derivative(): Could not compute function derivative of \"" << func << "\": Not implemented.";
                ok = false;
            }
            else
            {
                ret = fromFunctionArguments(getFunctionDerivative(func), mArguments);
                bool success;
                ret = fromTwoFactors(ret, mArguments.first().derivative(x, success));
                if(!success) {ok = false; }
            }
        }

        if(negative)
        {
            ret.changeSign();
        }
    }

    //Multiplication, d/dx(f(x)*g(x)) = f'(x)*g(x) + f(x)*g'(x)
    else if(isMultiplyOrDivide())
    {
        //Derivative of Z is zero
        if(mFactors.contains(Expression("Z")))
        {
            ret = Expression(0);
        }
        else if(!mDivisors.isEmpty())
        {
            Expression exp1 = Expression::fromFactorsDivisors(QList<Expression>() << mFactors, QList<Expression>());
            Expression exp2 = Expression::fromFactorsDivisors(QList<Expression>() << mDivisors, QList<Expression>());

            bool success;
            Expression der1 = exp1.derivative(x, success);
            if(!success) { ok = false; }
            Expression der2 = exp2.derivative(x, success);
            if(!success) { ok = false; }

            Expression term1 = fromTwoFactors(exp2, der1);
            Expression term2 = fromTwoFactors(exp1, der2);
            term2.changeSign();
            Expression factor = fromTwoTerms(term1, term2);
            Expression divisor = fromTwoFactors(exp2, exp2);
            ret = fromFactorDivisor(factor, divisor);
        }
        else
        {
            bool success;
            Expression exp1;
            exp1.replaceBy(*mFactors.begin());
            Expression der1;
            der1.replaceBy(exp1.derivative(x, success));
            if(!success) { ok = false; }
            QList<Expression> rest = mFactors;
            rest.removeFirst();
            Expression exp2;
            if(rest.size() == 1)
            {
                exp2.replaceBy(rest.first());
            }
            else
            {
                exp2 = Expression::fromFactorsDivisors(rest, QList<Expression>());
            }
            Expression der2 = exp2.derivative(x, success);
            if(!success) { ok = false; }

            Expression firstTerm = Expression::fromTwoFactors(der1, exp2);
            Expression secondTerm = Expression::fromTwoFactors(exp1, der2);
            ret = fromTwoTerms(firstTerm, secondTerm);
        }
    }
    //Addition
    else if(isAdd())
    {
        QList<Expression> derSet;
        for(const Expression &term : mTerms) {
            derSet << term.derivative(x, ok);
        }
        ret = Expression::fromTerms(derSet);
    }

    //Power, d/dx(f(x)^g(x)) = (g(x)*f'(x)+f(x)*log(f(x))*g'(x)) * f(x)^(g(x)-1)
    else if(this->isPower())
    {
        bool success;
        QString f = mpBase->toString();
        QString df = mpBase->derivative(x, success).toString();
        if(!success) { ok = false; }
        QString g = mpPower->toString();
        QString dg = mpPower->derivative(x, success).toString();
        if(!success) { ok = false; }

        //! @todo Make this without using string
        ret = Expression("("+f+")^("+g+"-1)*(("+g+")*("+df+")+("+f+")*log(("+f+"))*("+dg+"))");
    }

    //Symbol
    else
    {
        if(*this == x)
        {
            ret = Expression(1);
        }
        else
        {
            ret = Expression(0);
        }
    }

    Expression retExp = Expression(ret);
    retExp._simplify(FullSimplification, Recursive);

    if(retExp.toString() == "") {
        ok = false;
        gSymHopMessages << "In Expression::derivative(): Empty return string encountered.";
    }

    return retExp;
}


Expression *Expression::findFunction(const QString funcName) {
    if(this->getFunctionName() == funcName) {
        return this;
    }

    Expression *pExpr = nullptr;
    for(Expression &term : mTerms) {
        pExpr = term.findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }
    }
    for(Expression &argument : mArguments) {
        pExpr = argument.findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }
    }
    for(Expression &factor : mFactors) {
        pExpr = factor.findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }
    }
    for(Expression &divisor : mDivisors)
    {
        pExpr = divisor.findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }
    }
    if(mpBase) {
        pExpr = mpBase->findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }
    }
    if(mpPower) {
        pExpr = mpPower->findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }

    }
    if(mpLeft) {
        pExpr = mpLeft->findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }

    }
    if(mpRight) {
        pExpr = mpRight->findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }

    }
    if(mpDividend) {
        pExpr = mpDividend->findFunction(funcName);
        if(pExpr != nullptr) {
            return pExpr;
        }

    }

    return nullptr;
}

QStringList Expression::readErrorMessages()
{
    QStringList ret = gSymHopMessages;
    gSymHopMessages.clear();
    return ret;
}


//! @brief Returns whether or not expression contains a sub expression
//! @param expr Expression to check for
bool Expression::contains(const Expression expr) const
{
    if(*this == expr) {
        return true;
    }

    for(const Expression &term : mTerms) {
        if(term.contains(expr)) {
            return true;
        }
    }
    for(int i=0; i<mArguments.size(); ++i) {
        if(mArguments[i].contains(expr)) {
            return true;
        }
    }
    for(const Expression &factor : mFactors) {
        if(factor.contains(expr)) {
            return true;
        }
    }
    for(const Expression &divisor : mDivisors) {
        if(divisor.contains(expr)) {
            return true;
        }
    }
    if(mpBase && mpBase->contains(expr)) {
        return true;
    }
    if(mpPower && mpPower->contains(expr)) {
        return true;
    }
    if(mpLeft && mpLeft->contains(expr)) {
        return true;
    }
    if(mpRight && mpRight->contains(expr)) {
        return true;
    }
    if(mpDividend && mpDividend->contains(expr)) {
        return true;
    }

    return false;
}


//! @brief Converts time derivatives (der) in the expression to Z operators with bilinear transform
Expression Expression::bilinearTransform() const
{
    Expression retExpr;
    retExpr.replaceBy(*this);
    QStringList res;

    if(this->isAdd())
    {
        QList<Expression> newTerms = mTerms;
        QList<Expression>::iterator it;
        for(it=newTerms.begin(); it!=newTerms.end(); ++it)
        {
            (*it) = (*it).bilinearTransform();
        }
        retExpr = Expression::fromTerms(newTerms);
    }
    else if(this->isEquation())
    {
        Expression newLeft = *mpLeft;
        Expression newRight = *mpRight;
        newLeft = newLeft.bilinearTransform();
        newRight = newRight.bilinearTransform();
        retExpr = Expression::fromEquation(newLeft, newRight);
    }
    else if(this->isMultiplyOrDivide())
    {
        QList<Expression> newFactors = mFactors;
        QList<Expression>::iterator it;
        for(it=newFactors.begin(); it!=newFactors.end(); ++it)
        {
            (*it) = (*it).bilinearTransform();
        }
        QList<Expression> newDivisors = mDivisors;
        for(it=newDivisors.begin(); it!=newDivisors.end(); ++it)
        {
            (*it) = (*it).bilinearTransform();
        }
        retExpr = Expression::fromFactorsDivisors(newFactors, newDivisors);
    }
    else if(this->isFunction())
    {
        if(mFunction == "der")
        {
            QString arg = retExpr.getArgument(0).toString();
            res << "2.0" << "/" << "mTimestep" << "*" << "(1.0-Z)" << "/" << "(1.0+Z)" << "*" << "("+arg+")";
            retExpr = Expression(res);
        }
    }

    return retExpr;
}


Expression Expression::inlineTransform(const InlineTransformT transform, bool &ok) const
{
    QString transformStr;
    if(transform == Trapezoid || transform == AdamsMoulton2) {
        transformStr = "2.0/mTimestep*(1.0-Z)/(1.0+Z)*(%1)";
    }
    else if(transform == ExplicitEuler) {
        transformStr = "(1.0 - Z)/Z/mTimestep*(%1)";
    }
    else if(transform == ImplicitEuler || transform == BDF1 || transform == AdamsMoulton1) {
        transformStr = "(1.0 - Z)/mTimestep*(%1)";
    }
    else if(transform == BDF2) {
        transformStr = "(1.5 - 2*Z + 0.5*Z*Z)/mTimestep*(%1)";
    }
    else if(transform == BDF3) {
        transformStr = "(11.0 - 18.0*Z + 9.0*Z*Z - 2.0*Z*Z*Z)/6.0/mTimestep*(%1)";
    }
    else if(transform == BDF4) {
        transformStr = "(25.0 - 48.0*Z + 36.0*Z*Z-16.0*Z*Z*Z+3.0*Z*Z*Z*Z)/12.0/mTimestep*(%1)";
    }
    else if(transform == BDF5) {
        transformStr = "(137.0 - 300.0*Z + 300.0*Z*Z - 200.0*Z*Z*Z + 75.0*Z*Z*Z*Z - 12.0*Z*Z*Z*Z*Z)/60.0/mTimestep*(%1)";
    }
    else if(transform == AdamsMoulton3) {
        transformStr = "(1.0-Z)/(5.0/12.0 + 2.0/3.0*Z - 1.0/12.0*Z*Z)/mTimestep*(%1)";
    }
    else if(transform == AdamsMoulton4) {
        transformStr = "(1.0-Z)/(9.0/24.0 + 19.0/24.0*Z - 5.0/24.0*Z*Z + 1.0/24.0*Z*Z*Z)/mTimestep*(%1)";
    }
    else {
        gSymHopMessages << "In Expression::inlineTransform(): Undefined inline transform";
        ok = false;
        return (*this);
    }

    Expression retExpr;
    retExpr.replaceBy(*this);
    QStringList res;

    if(this->isAdd()) {
        QList<Expression> newTerms = mTerms;
        QList<Expression>::iterator it;
        for(it=newTerms.begin(); it!=newTerms.end(); ++it) {
            (*it) = (*it).inlineTransform(transform, ok);
            if(!ok) {
                return (*this);
            }
        }
        retExpr = Expression::fromTerms(newTerms);
    }
    else if(this->isEquation()) {
        Expression newLeft = *mpLeft;
        Expression newRight = *mpRight;
        newLeft = newLeft.inlineTransform(transform, ok);
        if(!ok) {
            return (*this);
        }
        newRight = newRight.inlineTransform(transform, ok);
        if(!ok) {
            return (*this);
        }

        retExpr = Expression::fromEquation(newLeft, newRight);
    }
    else if(this->isMultiplyOrDivide()) {
        QList<Expression> newFactors = mFactors;
        QList<Expression>::iterator it;
        for(it=newFactors.begin(); it!=newFactors.end(); ++it) {
            (*it) = (*it).inlineTransform(transform, ok);
            if(!ok) {
                return (*this);
            }
        }
        QList<Expression> newDivisors = mDivisors;
        for(it=newDivisors.begin(); it!=newDivisors.end(); ++it) {
            (*it) = (*it).inlineTransform(transform, ok);
            if(!ok) {
                return (*this);
            }
        }
        retExpr = Expression::fromFactorsDivisors(newFactors, newDivisors);
    }
    else if(this->isFunction()) {
        if(mFunction == "delay" && mArguments.size() == 2) {
            QString funcArg = retExpr.getArgument(0).toString();
            int steps = int(retExpr.getArgument(1).toDouble());
            retExpr = funcArg;
            for(int i=0; i<steps; ++i) {
                retExpr.multiplyBy(Expression("Z"));
            }
            retExpr = retExpr.inlineTransform(transform, ok);
        }
        if(mFunction == "der") {
            QString funcArg = retExpr.getArgument(0).toString();
            retExpr = Expression(transformStr.arg(funcArg));
            retExpr = retExpr.inlineTransform(transform, ok);
        }
    }
    else if(this->isSymbol()) {
        if(mString == "s") {
            retExpr = Expression(transformStr.arg("1"));
        }
    }

    ok = true;
    return retExpr;
}


//! @brief Returns a list with all contained symbols in the expression
//FIXED
QList<Expression> Expression::getVariables() const
{
    QList<Expression> retval;

    if(this->isAdd()) {
        for(const Expression &term : mTerms) {
            retval.append(term.getVariables());
        }
    }
    else if(this->isEquation()) {
        retval.append(mpLeft->getVariables());
        retval.append(mpRight->getVariables());
    }
    else if(this->isMultiplyOrDivide()) {
        for(const Expression &factor : mFactors) {
            retval.append(factor.getVariables());
        }
        for(const Expression &divisor : mDivisors) {
            retval.append(divisor.getVariables());
        }
    }
    else if(this->isFunction()) {
        for(const Expression &argument : mArguments) {
            retval.append(argument.getVariables());
        }
    }
    else if(this->isPower()) {
        retval.append(mpBase->getVariables());
        retval.append(mpPower->getVariables());
    }
    else if(this->isVariable() && !reservedSymbols.contains(mString)) {
        retval.append(*this);
    }

    removeDuplicates(retval);

    return retval;
}


//! @brief Returns a list with all used functions in the expression
QStringList Expression::getFunctions() const
{
    QStringList retval;

    if(this->isAdd()) {
        QList<Expression>::iterator it;
        for(const Expression &term : mTerms) {
            retval.append(term.getFunctions());
        }
    }
    else if(this->isEquation()) {
        retval.append(mpLeft->getFunctions());
        retval.append(mpRight->getFunctions());
    }
    else if(this->isMultiplyOrDivide()) {
        for(const Expression &factor : mFactors) {
            retval.append(factor.getFunctions());
        }
        for(const Expression &divisor : mDivisors) {
            retval.append(divisor.getFunctions());
        }
    }
    else if(this->isPower()) {
        retval.append(mpBase->getFunctions());
        retval.append(mpPower->getFunctions());
    }
    else if(this->isFunction()) {
        retval.append(mFunction);
        for(const Expression &argument : mArguments) {
            retval.append(argument.getFunctions());
        }
    }

    retval.removeDuplicates();

    return retval;
}


//! @brief Returns the function name, or an empty string is this is not a function
QString Expression::getFunctionName() const
{
    return mFunction;
}


//! @brief Returns the symbol name, or an empty string is this is not a symbol
QString Expression::getSymbolName() const
{
     return mString;
}


//! @brief Returns the specified function argument, or an empty string if this is not a function or if it has too few arguments
//! @param idx Index of argument to return
Expression Expression::getArgument(const int idx) const
{
    if(mArguments.size() > idx)
    {
        return mArguments[idx];
    }
    return Expression();
}


//! @brief Returns a list of function arguments, or an empty list if this is not a function
QList<Expression> Expression::getArguments() const
{
    return mArguments;
}


//! @brief Internal function, returns a list with all terms
QList<Expression> Expression::getTerms() const
{
    if(this->isAdd())
    {
        return mTerms;
    }
    else
    {
        return QList<Expression>() << *this;
    }
}

//! @brief Internal function, returns a list with all divisors
QList<Expression> Expression::getDivisors() const
{
    return mDivisors;
}

//! @brief Internal function, returns a list with all factors
QList<Expression> Expression::getFactors() const
{
    if(this->isMultiplyOrDivide())
    {
        return mFactors;
    }
    else
    {
        return QList<Expression>() << *this;
    }
}

//! @brief Internal function, returns a list with all exponential bases
Expression *Expression::getBase() const
{
    return mpBase;
}

//! @brief Internal function, returns a list with all powers
Expression *Expression::getPower() const
{
    return mpPower;
}

//! @brief Internal function, returns a list with all left sides
Expression *Expression::getLeft() const
{
    return mpLeft;
}

//! @brief Internal function, returns a list with all right sides
Expression *Expression::getRight() const
{
    return mpRight;
}

//! @brief Internal function, returns a list with all dividends
Expression *Expression::getDividends() const
{
    return mpDividend;
}




//! @brief Removes all divisors in the expression
void Expression::removeDivisors()
{
    mDivisors.clear();
}


//! @brief Removes specified factor in the expression
//! @param var Factor to remove
void Expression::removeFactor(const Expression var)
{
    if(*this == var)
    {
        this->replaceBy(Expression(1));
        return;
    }

    mFactors.removeOne(var);
    if(mFactors.size() == 1 && mDivisors.empty()) {
        this->replaceBy(Expression(mFactors.first()));
    }
    for(int d=0; d<mDivisors.size(); ++d)
    {
        if(mDivisors[d].isMultiplyOrDivide())
        {
            mFactors.append(mDivisors[d].mDivisors);
            mDivisors.append(mDivisors[d].mFactors);
            mDivisors.removeAt(d);
            --d;
        }
    }
    return;
}


//! @brief Replaces all expressions specified with a new expression
//! @param oldExpr Old expression to replace
//! @param newExpr New expression
void Expression::replace(const Expression oldExpr, const Expression newExpr)
{
    if(*this == oldExpr)
    {
        this->replaceBy(newExpr);
    }
    else
    {
        if(this->isAdd())
        {
            for(int i=0; i<mTerms.size(); ++i)
            {
                mTerms[i].replace(oldExpr, newExpr);
            }
        }
        else if(this->isEquation())
        {
            mpLeft->replace(oldExpr, newExpr);
            mpRight->replace(oldExpr, newExpr);
        }
        else if(this->isMultiplyOrDivide())
        {
            QList<Expression>::iterator it;
            for(it=mFactors.begin(); it!=mFactors.end(); ++it)
            {
                (*it).replace(oldExpr, newExpr);
            }
            for(it=mDivisors.begin(); it!=mDivisors.end(); ++it)
            {
                (*it).replace(oldExpr, newExpr);
            }
        }
        else if(this->isPower())
        {
            mpBase->replace(oldExpr, newExpr);
            mpPower->replace(oldExpr, newExpr);
        }
        else if(this->isFunction())
        {
            for(int a=0; a<mArguments.size(); ++a)
            {
                mArguments[a].replace(oldExpr, newExpr);
            }
        }

    }
    return;
}


//! @brief Expands all parentheses in the expression
//! @param simplifications Specifies the degree of simplification
void Expression::expand(const ExpressionSimplificationT simplifications)
{
    if(!this->isMultiplyOrDivide()) { return; }

    bool hasParentheses = false;
    for(const Expression &factor : mFactors) {
        if(factor.isAdd()) {
            hasParentheses=true;
        }
    }
    if(!hasParentheses) {
        return;
    }

    QList<Expression>::iterator it;
    while(mFactors.size() > 1)
    {
        QList<Expression> multipliedTerms;
        Expression factor1, factor2;

        //Extract the first two factors in the factor set
        it = mFactors.begin();
        factor1 = (*it);
        ++it;
        factor2 = (*it);

        //Subtract the extracted elements from the original set
        mFactors.removeOne(factor1);
        mFactors.removeOne(factor2);

        //Generate a set of terms for each extracted factor
        QList<Expression> terms1, terms2;
        terms1 = factor1.getTerms();
        terms2 = factor2.getTerms();

        //Multiply each term in the first set with each term in the second set
        for(const Expression &exp1 : terms1) {
            for(const Expression &exp2 : terms2) {
                Expression newTerm = Expression::fromFactorsDivisors(QList<Expression>() << exp1 << exp2, QList<Expression>());    //Multiplication!
                newTerm.expand();       //Recurse sub terms
                multipliedTerms << newTerm;
            }
        }

        mFactors.append(Expression::fromTerms(multipliedTerms));
    }
    if(mFactors.size() == 1 && mDivisors.isEmpty())
    {
        replaceByCopy(mFactors.first());
    }

    _simplify(simplifications);
}


//Expand power functions from e.g. "pow(x,3)" to "x*x*x". This will improve performance in generated code.
void Expression::expandPowers()
{
    if(this->isPower() && this->getPower()->isInteger()) {
        int nFactors = int(mpPower->toDouble()+0.5);
        QList<Expression> factors;
        for(int i=0; i<nFactors; ++i) {
            factors << (*mpBase);
        }
        Expression replacement = Expression::fromFactorsDivisors(factors, QList<Expression>());
        this->replaceBy(replacement);
        return;
    }

    if(this->isEquation()) {
        mpLeft->expandPowers();
        mpRight->expandPowers();
    }

    if(this->isMultiplyOrDivide()) {
        for(auto &factor : mFactors) {
            factor.expandPowers();
        }
        for(auto &divisor : mDivisors) {
            divisor.expandPowers();
        }
    }

    if(this->isAdd()) {
        for(auto &term : mTerms) {
            term.expandPowers();
        }
    }

    if(this->isFunction()) {
        for(auto &arg : mArguments) {
            arg.expandPowers();
        }
    }
}


//! @brief Linearizes the expression by multiplying with all divisors until no divisors remains
//! @note Should only be used on equations (obviously)
void Expression::linearize()
{
    if(!this->isEquation()) {
        gSymHopMessages << "In Expression::linearize(): Only equation expressions can be linearized.";
        return;
    }


    this->expand();

    //Generate list with terms from both left and right side
    QList<Expression> allTerms;
    allTerms << mpLeft->getTerms() << mpRight->getTerms();

    //Generate list with all divisors from all terms
    QList<Expression> allDivisors;
    for(const Expression &term : allTerms) {
        QList<Expression> divisors = term.getDivisors();
        for(const Expression &exp : divisors) {
            if(!allDivisors.contains(exp)) {
                allDivisors << exp;
            }
            else {
                while(allDivisors.count(exp) < divisors.count(exp)) {    //Make sure the all divisors vector contains at least the same amount of each divisor as the term
                    allDivisors << exp;
                }
            }
        }
    }

    //Multiply each term with each divisor (on both sides)
    QList<Expression> leftTerms = mpLeft->getTerms();
    for(int i=0; i<leftTerms.size(); ++i)
    {
        QList<Expression> tempDivisors = allDivisors;
        for(int d=0; d<leftTerms[i].getDivisors().size(); ++d)
        {
            tempDivisors.removeOne(leftTerms[i].getDivisors()[d]);
        }
        leftTerms[i].replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << leftTerms[i].getFactors() << tempDivisors, QList<Expression>()));
    }
    mpLeft->replaceBy(Expression::fromTerms(leftTerms));
    QList<Expression> rightTerms = mpRight->getTerms();
    for(int i=0; i<rightTerms.size(); ++i)
    {
        QList<Expression> tempDivisors = allDivisors;
        for(int d=0; d<rightTerms[i].getDivisors().size(); ++d)
        {
            tempDivisors.removeOne(rightTerms[i].getDivisors()[d]);
        }
        rightTerms[i].replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << rightTerms[i].getFactors() << tempDivisors, rightTerms[i].getDivisors()));
    }
    mpRight->replaceBy(Expression::fromTerms(rightTerms));

    _simplify(FullSimplification, Recursive);

    return;
}


//! @brief Moves all right side expressions to the left side, if this is an equation
void Expression::toLeftSided()
{
    if(!this->isEquation()) {
        gSymHopMessages << "In Expression::toLeftSided(): Only equations can be converted to left-sided.";
        return;
    }
    if(mpRight->isAdd())
    {
        for(int i=0; i<mpRight->mTerms.size(); ++i)
        {
            mpRight->mTerms[i].changeSign();
        }
    }
    else
    {
        mpRight->changeSign();
    }
    mpLeft->replaceBy(Expression::fromTerms(QList<Expression>() << mpLeft->getTerms() << mpRight->getTerms()));
    (*mpRight) = Expression("0");
}


//! @brief Factors specified expression
//! @param var Expression to factor
//! @example Factorizing "a*b*c + a*c*d + b*c*d" for "b" => "b*(a*c + c*d) + a*c*d"
//FIXED
void Expression::factor(const Expression var)
{
    QList<Expression> terms = getTerms();
    QList<Expression> termsWithVar, termsWithoutVar;

    for(int i=0; i<terms.size(); ++i)
    {
        if(terms[i] == var || terms[i].getFactors().contains(var))
        {
            terms[i].removeFactor(var);
            termsWithVar << terms[i];
        }
        else
        {
            termsWithoutVar << terms[i];
        }
    }

    if(!termsWithVar.isEmpty())
    {
        Expression termsWithVarExpr = Expression::fromTerms(termsWithVar);
        Expression factoredTerm = Expression::fromFactorsDivisors(QList<Expression>() << var << termsWithVarExpr, QList<Expression>());

        this->replaceBy(Expression::fromTerms(QList<Expression>() << factoredTerm << termsWithoutVar));
    }
}


//! @brief Factors the most common factor in the expression
//FIXED
void Expression::factorMostCommonFactor()
{
    if(!isAdd())
    {
        return;
    }

    QList<Expression> symbols;
    QList<int> counter;
    QList<Expression> terms = this->getTerms();
    for(int t=0; t<terms.size(); ++t)
    {
        QList<Expression> factors = terms[t].getFactors();
        for(int f=0; f<factors.size(); ++f)
        {
            if(factors[f] == Expression(-1.0) || factors[f] == Expression(1.0))       //Ignore "-1.0" and "1.0"
            {
                continue;
            }
            if(symbols.contains(factors[f]))
            {
                int idx = symbols.indexOf(factors[f]);
                counter[idx] = counter[idx]+1;
            }
            else
            {
                symbols.append(factors[f]);
                counter.append(1);
            }
        }
    }

    int max=0;
    Expression mostCommon;
    for(int s=0; s<symbols.size(); ++s)
    {
        if(counter[s] > max)
        {
            max = counter[s];
            mostCommon = symbols[s];
        }
    }

    if(max>1)
    {
        factor(mostCommon);
    }

    return;
}






//! @brief Verifies that the expression is correct
//FIXED
bool Expression::verifyExpression(const QStringList &userFunctions) const
{
    //Verify all functions
    if(!_verifyFunctions(userFunctions))
    {
        return false;
    }

    return true;
}


//! @brief Verifies that all functions are supported
//FIXED
bool Expression::_verifyFunctions(const QStringList &userFunctions) const
{
    bool success = true;

    QStringList functions = this->getFunctions();
    for(int i=0; i<functions.size(); ++i)
    {
        if(!getSupportedFunctionsList().contains(functions[i]) && !getCustomFunctionList().contains(functions[i]) && !userFunctions.contains(functions[i]))
        {
            //QMessageBox::critical(0, "SymHop", "Function \""+functions[i]+"\" is not supported by component generator.");
            success = false;
        }
    }

    return success;
}


//! @brief Simplifies the expression
//! @param type Tells the degree of simplification to perform
//! @param recursive Tells whether or not children are to be recursively simplified
//! @note Recursion is not needed when creating new expressions, since the creator recurses all children anyway.
//FIXED
void Expression::_simplify(ExpressionSimplificationT type, const ExpressionRecursiveT recursive)
{
    if(this->isSymbol() || this->isVariable())
    {
        return;
    }

    if(type == NoSimplifications)
    {
        return;
    }

    if(recursive == Recursive)
    {
        for(int i=0; i<mArguments.size(); ++i)
        {
            mArguments[i]._simplify(type, recursive);
        }
        for(int i=0; i<mTerms.size(); ++i)
        {
            mTerms[i]._simplify(type, recursive);
        }
        for(int i=0; i<mFactors.size(); ++i)
        {
            mFactors[i]._simplify(type, recursive);
        }
        for(int i=0; i<mDivisors.size(); ++i)
        {
            mDivisors[i]._simplify(type, recursive);
        }
        if(mpBase) { mpBase->_simplify(type, recursive); }
        if(mpPower) { mpPower->_simplify(type, recursive); }
        if(mpLeft) { mpLeft->_simplify(type, recursive); }
        if(mpRight) { mpRight->_simplify(type, recursive); }
    }

    //restartFull:

    //Trivial simplifications
    if(this->isMultiplyOrDivide())
    {
        //Merge factors and divisors together
        for(int f=0; f<mFactors.size(); ++f)
        {
            if(mFactors[f].isMultiplyOrDivide())
            {
                mFactors.append(mFactors[f].mFactors);
                mDivisors.append(mFactors[f].mDivisors);
                mFactors.removeAt(f);
                --f;
            }
        }
        for(int d=0; d<mDivisors.size(); ++d)
        {
            if(mDivisors[d].isMultiplyOrDivide())
            {
                mFactors.append(mDivisors[d].mDivisors);
                mDivisors.append(mDivisors[d].mFactors);
                mDivisors.removeAt(d);
                --d;
            }
        }

        if(mFactors.size() > 1) { mFactors.removeAll(Expression(1.0)); }    //Replace 1*x with x
        if(mFactors.isEmpty()) { replaceBy(Expression(1)); }                //Make sure 1*1 = 1 (don't remove all 1s)

        if(mFactors.size() == 1 && mDivisors.isEmpty()) { replaceBy(mFactors.first()); }

        if(mFactors.contains(Expression(0))) { replaceBy(Expression(0)); }    //Replace "0*x" and "x*0" with "0.0"

        int nNeg = mFactors.count(Expression("-1"))+mDivisors.count(Expression("-1"));        //Remove unnecessary negatives
        if(nNeg > 1)
        {
            mFactors.removeAll(Expression("-1"));
            mDivisors.removeAll(Expression("-1"));
            if(nNeg % 2 != 0)
            {
                mFactors << Expression("-1");
            }
            else if(mFactors.isEmpty()) {
                mFactors << Expression("1");
            }
        }
    }
    else if(this->isAdd())
    {
        for(int t=0; t<mTerms.size(); ++t)
        {
            if(mTerms[t].isAdd())
            {
                mTerms.append(mTerms[t].mTerms);
                mTerms.removeAt(t);
                --t;
            }
        }


        mTerms.removeAll(Expression(0));
        if(mTerms.isEmpty())
        {
            replaceBy(Expression(0));
        }

        //Join all similar terms together (i.e. replace "2*x+x+x+y" with "4*x+y")
        for(int t=0; t<mTerms.size(); ++t)
        {
            double cnt = this->countTerm(mTerms[t]);
            double num = mTerms[t].getNumericalFactor();
            if(cnt != num)
            {
                Expression tempTerm = mTerms[t].removeNumericalFactors();
                tempTerm.multiplyBy(Expression(this->countTerm(mTerms[t])));
                this->removeTerm(mTerms[t]);
                if(!this->isSymbol() && !this->isAdd() && !this->isMultiplyOrDivide() && !this->isFunction() && !this->isPower()) //Nothing left at all, replace with sum of terms
                {
                    this->replaceBy(tempTerm);
                }
                else if(!this->isAdd()) //Only one term was left, so it was converted to symbol - convert back
                {
                    this->replaceBy(fromTwoTerms(*this, tempTerm));
                }
                else
                {
                    mTerms.append(tempTerm);
                }
                --t;
            }
        }
        if(mTerms.size() == 1)
        {
            replaceBy(mTerms.first());
        }
    }
    else if(this->isPower())
    {
        if(mpPower->toDouble(new bool) == 1.0)
        {
            replaceBy(*mpBase);
        }
        else if(mpPower->isNegative())
        {
            mpPower->changeSign();
            this->replaceBy(Expression::fromFactorsDivisors(QList<Expression>() << Expression("1"), QList<Expression>() << (*this)));
        }
        else        //Replace with number if both base and exponent are numericals
        {
            if(mpBase->isNumericalSymbol() && mpPower->isNumericalSymbol())
            {
                double value = pow(mpBase->toDouble(),mpPower->toDouble());
                this->replaceBy(Expression(value));
            }
        }
    }

    if(type == TrivialSimplifications)      //End of trivial simplifications, return if this was specified
    {
        return;
    }


    if(isAdd())
    {
        //Sum up all numericals to one term
        double value=0;
        bool foundOne=false;
        for(int i=0; i<mTerms.size(); ++i)
        {
            if(mTerms[i].isNumericalSymbol())
            {
                value += mTerms[i].toDouble();
                mTerms.removeAt(i);
                --i;
                foundOne = true;
            }
        }
        if(foundOne)
        {
            mTerms << Expression(value);
        }

        mTerms.removeAll(Expression("0.0"));

        if(mTerms.size() == 1)
        {
            this->replaceBy(mTerms.first());
        }

        return;
    }

    if(this->isMultiplyOrDivide())
    {
        if(mFactors.size() > 1)
        {
            expand(NoSimplifications);
            if(isAdd() || mFactors.size() == 1)
            {
                this->_simplify(FullSimplification, Recursive);
            }
        }

        //Multiply all numericals together (except for -1)
        double value = 1;
        bool foundOne=false;
        for(int i=0; i<mFactors.size(); ++i)
        {
            if(mFactors[i].isNumericalSymbol() && !(mFactors[i] == Expression("-1")))
            {
                value *= mFactors[i].toDouble();
                mFactors.removeAt(i);
                --i;
                foundOne=true;
            }
        }
        for(int i=0; i<mDivisors.size(); ++i)
        {
            if(mDivisors[i].isNumericalSymbol() && !(mDivisors[i] == Expression("-1")))
            {
                value /= mDivisors[i].toDouble();
                mDivisors.removeAt(i);
                --i;
                foundOne = true;
            }
        }
        if(foundOne)
        {
            mFactors << Expression(value);
            this->replaceBy(Expression::fromFactorsDivisors(mFactors, mDivisors));
        }

        //Expand power functions in factors if power is int
        for(int i=0; i<mFactors.size(); ++i)
        {
            if(mFactors[i].isPower() && mFactors[i].getPower()->isNumericalSymbol() && isWhole(mFactors[i].getPower()->toDouble()))
            {
                int n = int(mFactors[i].getPower()->toDouble());
                for(int j=0; j<n; ++j)
                {
                    mFactors << (*mFactors[i].getBase());
                }
                mFactors.removeAt(i);
                --i;
            }
        }

        //Expand power functions in divisors if power is int
        for(int i=0; i<mDivisors.size(); ++i)
        {
            if(mDivisors[i].isPower() && mDivisors[i].getPower()->isNumericalSymbol() && isWhole(mDivisors[i].getPower()->toDouble()))
            {
                int n = int(mDivisors[i].getPower()->toDouble());
                for(int j=0; j<n; ++j)
                {
                    mDivisors << (*mDivisors[i].getBase());
                }
                mDivisors.removeAt(i);
                --i;
            }
        }

        bool removedFactorOrDivisor=false;

        //Cancel out same factors and divisors
        bool didSomething = true;
        while(didSomething) {
            didSomething = false;
            for(const Expression &factor : mFactors)
            {
                if(mDivisors.contains(factor))
                {
                    mDivisors.removeOne(factor);
                    mFactors.removeOne(factor);
                    removedFactorOrDivisor = true;
                    didSomething = true;
                    break;
                }
            }
        }

        if(removedFactorOrDivisor && mFactors.isEmpty() && mDivisors.isEmpty()) { replaceBy(Expression(1)); }
        else if(removedFactorOrDivisor && mFactors.isEmpty()) { mFactors.append(Expression(1)); }

        //Join multiple factors to powers
        didSomething = true;
        while(didSomething) {
            didSomething = false;
            for(const Expression &factor : mFactors) {
                int n = mFactors.count(factor);
                if(n > 1) {
                    mFactors.append(Expression::fromBasePower(factor, Expression(n)));
                    mFactors.removeAll(factor);
                    didSomething = true;
                    break;
                }
            }
        }

        if(mFactors.size() == 1 && mDivisors.isEmpty()) { replaceByCopy(mFactors.first()); }

        //Join multiple divisors to powers
        didSomething = true;
        while(didSomething) {
            didSomething = false;
            for(const Expression &divisor : mDivisors) {
                int n = mDivisors.count(divisor);
                if(n > 1) {
                    mDivisors.append(Expression::fromBasePower(divisor, Expression(n)));
                    mDivisors.removeAll(divisor);
                    didSomething = true;
                    break;
                }
            }
        }
    }

    if(this->isFunction()) {
        if(mFunction == "ifElse" && mArguments.size() == 3) {
            if(mArguments.at(1) == mArguments.at(2)) {
                this->replaceBy(mArguments.at(1));
            }
        }
    }

    if(mFactors.isEmpty() && !mDivisors.isEmpty())
    {
        mFactors.append(Expression(1));
    }
}


bool Expression::splitAtSeparator(const QString sep, const QStringList subSymbols, const ExpressionSimplificationT simplifications)
{
    if(subSymbols.contains(sep) || (sep == "*" && subSymbols.contains("/")))
    {
        if(sep == "=")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "=")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(!onRight)
            {
                return false;
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            else
            {
                if(mpLeft)
                {
                    mpLeft->cleanUp();
                    delete mpLeft;
                }
                if(mpRight)
                {
                    mpRight->cleanUp();
                    delete mpRight;
                }
                mpLeft = new Expression(left);
                mpRight = new Expression(right);
            }
        }
        else if(sep == "+")
        {
            QStringList term;
            bool negative=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "+")
                {
                    if(negative) { term << "*" << "-1"; }
                    mTerms.append(Expression(term, simplifications));
                    term.clear();
                    negative = false;
                }
                else if(subSymbols[i] == "-")
                {
                    if(negative) { term << "*" << "-1"; }
                    mTerms.append(Expression(term, simplifications));
                    term.clear();
                    negative = true;
                }
                else
                {
                    term.append(subSymbols[i]);
                }
            }
            mTerms.append(Expression(term, simplifications));
        }
        else if(sep == "*")
        {
            QStringList factorOrDiv;
            bool div = false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "*")
                {
                    if(factorOrDiv.isEmpty())
                    {
                        return false;
                    }
                    if(div)
                    {
                        mDivisors.append(Expression(factorOrDiv, simplifications));
                    }
                    else
                    {
                        mFactors.append(Expression(factorOrDiv, simplifications));
                    }
                    factorOrDiv.clear();
                    div = false;
                }
                else if(subSymbols[i] == "/")
                {
                    if(factorOrDiv.isEmpty())
                    {
                        return false;
                    }
                    if(div)
                    {
                        mDivisors.append(Expression(factorOrDiv, simplifications));
                    }
                    else
                    {
                        mFactors.append(Expression(factorOrDiv, simplifications));
                    }
                    factorOrDiv.clear();
                    div = true;
                }
                else
                {
                    factorOrDiv << subSymbols[i];
                }
            }
            if(div)
            {
                mDivisors.append(Expression(factorOrDiv, simplifications));
            }
            else
            {
                mFactors.append(Expression(factorOrDiv, simplifications));
            }
        }
        else if(sep == "^")
        {
            bool inPower=false;
            QStringList base;
            QStringList power;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "^")
                {
                    inPower = true;
                }
                else if(!inPower)
                {
                    base.append(subSymbols[i]);
                }
                else
                {
                    power.append(subSymbols[i]);
                }
            }
            if(base.isEmpty() || power.isEmpty())
            {
                return false;
            }
            if(mpBase)
            {
                mpBase->cleanUp();
                delete mpBase;
            }
            if(mpPower)
            {
                mpPower->cleanUp();
                delete mpPower;
            }
            mpBase = new Expression(base);
            mpPower = new Expression(power);
        }
        else if(sep == "%")
        {
            bool inDivisor=false;
            QStringList dividend;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "%")
                {
                    inDivisor = true;
                }
                else if(!inDivisor)
                {
                    dividend.append(subSymbols[i]);
                }
                else
                {
                    mDivisors.append(subSymbols[i]);
                }
            }
            if(mpDividend)
            {
                mpDividend->cleanUp();
                delete mpDividend;
            }
            mpDividend = new Expression(dividend);
        }
        else if(sep == "!")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "!")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "equal";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == "~")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "~")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "notEqual";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == "{")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "{")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "logicalAnd";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == "}")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "}")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "logicalOr";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == ">")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == ">")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "greaterThan";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == "?")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "?")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "greaterThanOrEqual";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == "<")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "<")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "smallerThan";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        else if(sep == "¤")
        {
            QStringList left, right;
            bool onRight=false;
            for(int i=0; i<subSymbols.size(); ++i)
            {
                if(subSymbols[i] == "¤")
                {
                    onRight = true;
                }
                else if(!onRight)
                {
                    left.append(subSymbols[i]);
                }
                else
                {
                    right.append(subSymbols[i]);
                }
            }
            if(left.isEmpty() || right.isEmpty())
            {
                return false;
            }
            mFunction = "smallerThanOrEqual";
            mArguments.append(Expression(left));
            mArguments.append(Expression(right));
        }
        return true;
    }

    return false;
}


double Expression::countTerm(const Expression &expr) const
{
    Expression pureTerm = expr.removeNumericalFactors();
    double ret=0;
    for(const Expression &term : mTerms) {
        if(term.removeNumericalFactors() == pureTerm) {
            ret += term.getNumericalFactor();
        }
    }

    return ret;
}


//! @brief Removes all copies of specified term, without regards to numerical factors
void Expression::removeTerm(const Expression &term)
{
    Expression pureTerm = term.removeNumericalFactors();
    for(int t=0; t<mTerms.size(); ++t)
    {
        if(mTerms[t].removeNumericalFactors() == pureTerm)
        {
            mTerms.removeAt(t);
            --t;
        }
    }

    if(mTerms.size() == 1)
    {
        replaceBy(mTerms.first());
    }
}


Expression Expression::removeNumericalFactors() const
{
    if(!isMultiplyOrDivide()) {
        return *this;
    }
    Expression ret;

    for(const Expression &factor : mFactors) {
        if(!factor.isNumericalSymbol()) {
            ret.mFactors.append(factor);
        }
    }
    ret.mDivisors.append(mDivisors);

    if(ret.mFactors.size() == 1 && ret.mDivisors.isEmpty())
    {
        //Expression expr = Expression(ret.mFactors.first());
        ret.replaceByCopy(ret.mFactors.first());
    }
    else if(ret.mFactors.isEmpty() && !ret.mDivisors.isEmpty())
    {
        ret.mFactors.append(Expression("1"));
    }
    else if(ret.mFactors.isEmpty() && ret.mDivisors.isEmpty())
    {
        ret.replaceBy(Expression("1"));
    }

    return ret;
}


double Expression::getNumericalFactor() const
{
    if(!isMultiplyOrDivide()) {
        return 1;
    }

    double ret=1;

    for(const Expression &factor : mFactors) {
        if(factor.isNumericalSymbol()) {
            ret *= factor.toDouble();
        }
    }

    return ret;
}


//! @brief Returns derivative to specified function, or an empty string if function is not supported
QString SymHop::getFunctionDerivative(const QString &key)
{
    if(key == "sin") { return "cos"; }
    if(key == "cos") { return "-sin"; }
    if(key == "abs") { return "sign"; }
    if(key == "onPositive") { return "dxOnPositive"; }
    if(key == "onNegative") { return "dxOnNegative"; }
    if(key == "signedSquareL") { return "dxSignedSquareL"; }
    if(key == "der") { return "dder"; }     //For use with Modelica parser

    //For use with Modelica parser
    if(key.startsWith("STATEVAR"))
    {
        QString tempStr = key;
        bool ok;
        int number = tempStr.remove(QString("STATEVAR")).toInt(&ok);
        if(ok)
        {
            return "DSTATEVAR"+QString::number(number);
        }
    }

    return "";
}


//! @brief Returns a list with supported functions for equation-based model generation
QStringList SymHop::getSupportedFunctionsList()
{
    return QStringList() << "div" << "rem" << "mod" << "tan" << "cos" << "sin" << "atan" << "acos" << "asin" << "atan2" << "sinh" << "cosh" << "tanh" << "log" << "exp" << "sqrt" << "sign" << "abs" << "der" << "onPositive" << "onNegative" << "signedSquareL" << "limit" << "integer" << "floor" << "ceil" << "pow" << "min" << "max" << "nonZero" << "turbulentFlow" << "delay" << "equal" << "notEqual" << "greaterThan" << "smallerThan" << "greaterThanOrEqual" << "smallerThanOrEqual" << "r2d" << "d2r" << "pi";
}


//! @brief Returns a list of custom Hopsan functions that need to be allowed in the symbolic library
QStringList SymHop::getCustomFunctionList()
{
    return QStringList() << "hopsanLimit" << "hopsanDxLimit" << "onPositive" << "onNegative" << "signedSquareL" << "limit" << "nonZero" << "turbulentFlow" << "ifElse";
}


//! @brief Finds the first path through a matrix of dependencies, used to sort jacobian matrices
bool SymHop::findPath(QList<int> &order, QList<QList<int> > dependencies, int level, QList<int> preferredPath)
{
    if(level > dependencies.size()-1)
    {
        return true;
    }

    if(dependencies.at(level).isEmpty())        //Only used to generate preferred path, for the "real" order the dependencies will never have empty rows
    {
        order.append(-1);
        ++level;
    }

    if(level > dependencies.size()-1)
    {
        return true;
    }

    if(preferredPath.size() > level+1 && dependencies.at(level).contains(preferredPath.at(level)) && !order.contains(preferredPath.at(level)))
    {
        order.append(preferredPath.at(level));
        if(findPath(order, dependencies, level+1, preferredPath))
        {
            return true;
        }
        order.removeLast();
    }

    for(int i=0; i<dependencies.at(level).size(); ++i)
    {
        if(!order.contains(dependencies.at(level).at(i)))
        {
            order.append(dependencies.at(level).at(i));
            if(findPath(order, dependencies, level+1, preferredPath))
            {
                return true;
            }
            order.removeLast();
        }
    }
    return false;
}


//! @brief Sorts an equation system so that all diagonal elements in the jacobian matrix are non-zero
//! @param equation List of system equations
//! @param symbols List of all variables
//! @param stateVars List of state variables
//! @param limitedVariableEquations Reference to list with indexes for limitation functions
//! @param limitedDerivativeEquations Reference to list with indexes for derivative limitation functions
bool SymHop::sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations, QList<int> preferredOrder)
{
//    qDebug() << "Jacobian:";
//    for(int i=0; i<jacobian.size(); ++i)
//    {
//        QString line;
//        for(int j=0; j<jacobian.size(); ++j)
//        {
//            line.append(jacobian[i][j].toString());
//            line.append("  ");
//        }
//        qDebug() << line;
//    }

    //Generate a dependency tree between equations and variables
    Expression zero = Expression(0.0);
    QList<QList<int> > dependencies;
    for(int v=0; v<stateVars.size(); ++v)
    {
        dependencies.append(QList<int>());
        for(int e=0; e<stateVars.size(); ++e)
        {
            if(!(jacobian[e][v] == zero))
            {
                dependencies[v].append(e);
            }
        }
    }

    //Recurse dependency tree to find a good sorting order
    QList<int> order;
    if(!findPath(order, dependencies, 0, preferredOrder))
    {
        gSymHopMessages << "In sortEquationSystem(): Failed to find sorting path.";
        return false;
    }

    //Sort equations to new order
    QList<Expression> sortedEquations;
    QList<QList<Expression> > sortedJacobian;
    for(int i=0; i<order.size(); ++i)
    {
        sortedEquations.append(equations.at(order[i]));
        sortedJacobian.append(jacobian.at(order[i]));
        for(int j=0; j<limitedVariableEquations.size(); ++j)    //Sort limited variable equation numbers
        {
            if(limitedVariableEquations[j] == i)
            {
                limitedVariableEquations[j] = order[i];
            }
        }
        for(int j=0; j<limitedDerivativeEquations.size(); ++j)  //Sort limited derivative equation numbers
        {
            if(limitedDerivativeEquations[j] == i)
            {
                limitedDerivativeEquations[j] = order[i];
            }
        }
    }

    jacobian = sortedJacobian;
    equations = sortedEquations;

    return true;
}


//! @brief Removes all duplicates in a list of expressions
//! @param rList Reference to the list
void SymHop::removeDuplicates(QList<Expression> &rSet)
{
    QList<Expression> tempSet;
    for(const Expression &item : rSet) {
        if(!tempSet.contains(item)) {
            tempSet.append(item);
        }
    }

    rSet = tempSet;
}


bool SymHop::isWhole(const double value)
{
    return (static_cast<int>(value) == value);
}


//! @brief Checks whether or no the parentheses are correct in a string
//! @param str String to verify
bool Expression::verifyParantheses(const QString str)
{
    int balance = 0;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '(')
        {
            ++balance;
        }
        else if(str[i] == ')')
        {
            --balance;
        }
        if(balance<0) { return false; }
    }
    return (balance==0);
}


//! @brief Splits a string at specified character, but does not split inside parentheses
//! @param str String to split
//! @param c Character to split at
QStringList Expression::splitWithRespectToParentheses(const QString str, const QChar c)
{
    if(str.isEmpty())
    {
        return QStringList();
    }

    QStringList ret;
    int parBal=0;
    int start=0;
    int len=0;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '(')
        {
            ++parBal;
        }
        else if(str[i] == ')')
        {
            --parBal;
        }
        else if(str[i] == c && parBal == 0)
        {
            ret.append(str.mid(start,len));
            start=start+len+1;
            len=-1;
        }
        ++len;
    }
    ret.append(str.mid(start,len));
    return ret;
}

Expression::InlineTransformT SymHop::strToTransform(const QString &str)
{
    if(str == "trapezoid") {
        return Expression::Trapezoid;
    }
    else if(str == "expliciteuler") {
        return Expression::ExplicitEuler;
    }
    else if(str == "impliciteuler") {
        return Expression::ImplicitEuler;
    }
    else if(str == "bdf1") {
        return Expression::BDF1;
    }
    else if(str == "bdf2") {
        return Expression::BDF2;
    }
    else if(str == "bdf3") {
        return Expression::BDF3;
    }
    else if(str == "bdf4") {
        return Expression::BDF4;
    }
    else if(str == "bdf5") {
        return Expression::BDF5;
    }
    else if(str == "adamsmoulton1") {
        return Expression::AdamsMoulton1;
    }
    else if(str == "adamsmoulton2") {
        return Expression::AdamsMoulton2;
    }
    else if(str == "adamsmoulton3") {
        return Expression::AdamsMoulton3;
    }
    else if(str == "adamsmoulton4") {
        return Expression::AdamsMoulton4;
    }
    else {
        return Expression::UndefinedTransform;
    }
}
