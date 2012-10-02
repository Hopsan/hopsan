/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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
#include <cmath>

#include "SymHop.h"

using namespace std;
using namespace SymHop;

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
//! "onNegative", "signedSquareL", "limit"


//! @brief Constructor for Expression class using QString
//! @param indata String containg a numerical expression
//! @param simplifications Specifies the degree of simplification
Expression::Expression(const QString indata, const ExpressionSimplificationT simplifications)
{
    commonConstructorCode(QStringList() << indata, simplifications);
}


//! @brief Constructor for Expression class using QStringList
//! @details This constructor is faster than the one using a string, because the string does not need to be parsed.
//! @param symbols String list containg numerical symbols
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree, this shall never be used when defining a new expression
Expression::Expression(QStringList symbols, const ExpressionSimplificationT simplifications, const QString parentSeparator)
{
    commonConstructorCode(symbols, simplifications, parentSeparator);
}


//! @brief Constructor for Exprsesion using left and right expressions and an operator
//! @details This is the fastest way to create a new expression of operator type, because no evaulations or parsing needs to be performed.
//! @param left Left side exprsesion
//! @param right Right side expression
//! @param mid Operator ("+", "*" or "/")
//! @param simplifications Specifies the degree of simplification
Expression::Expression(const Expression left, const QString mid, const Expression right, const ExpressionSimplificationT simplifications)
{
    if(mid == "+" || mid == "*" || mid == "/" || mid == "=")
    {
        mType = Expression::Operator;
        mString = mid;
        mChildren.append(left);
        mChildren.append(right);
        _simplify(simplifications);
    }
}


//! @brief Constructor for Exprsesion using a list of expressions joined by operaetors
//! @example Expression([A,B,C,D], "+") will result in A+B+C+D
//! @param children List of child expressions
//! @param separator Operator used to combine child expressions ("+", "*" or "/")
//! @param simplifications Specifies the degree of simplification
Expression::Expression(const QList<Expression> children, const QString separator, const ExpressionSimplificationT simplifications)
{
    if(children.size() == 1)
    {
        this->replaceBy(children[0]);
    }
    else if(children.size() == 2)
    {
        this->replaceBy(Expression(children[0], separator, children[1], simplifications));
    }
    else
    {
        if(separator == "+" || separator == "*" || separator == "/")
        {
            mType = Expression::Operator;
            mString = separator;
            mChildren.append(children.first());
            mChildren.append(Expression(children.mid(1, children.size()-1), separator));
            _simplify(simplifications);
        }
    }
}


//! @brief Constructor for creating an expression from a numerical value
//! @param value Value of new symbol
Expression::Expression(const double value)
{
    mType = Expression::Symbol;
    mString = QString::number(value);

    //Make sure numerical symbols have double precision
    bool isInt;
    mString.toInt(&isInt);
    if(isInt && !mString.contains("."))
    {
        mString.append(".0");
    }
}


//! @brief Common constructor code that constructs an Expression from a string list
//! @param symbols String list containg numerical symbols
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree
void Expression::commonConstructorCode(QStringList symbols, const ExpressionSimplificationT simplifications, const QString parentSeparator)
{
    //Only one symbol, so parse it as a string
    if(symbols.size() == 1)
    {
        QString str = symbols.first();
        symbols.clear();

        //Don't create empty expressions
        if(str.isEmpty())
        {
            return;
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
        }
        while(str.contains("++"))
        {
            str.replace("++", "+");
        }
        while(str.contains("+-+-"))
        {
            str.replace("+-+-","+-");
        }
        while(str.contains("-("))
        {
            str.replace("-(", "(-");
        }
        while(str.startsWith("+"))
        {
            str = str.right(str.size()-1);
        }
        while(str.contains("=+"))
        {
            str.replace("=+", "=");
        }

        //Remove all excessive parentheses
        while(str.startsWith("(") && str.endsWith(")"))
        {
            QString testString = str.mid(1, str.size()-2);
            if(verifyParantheses(testString))
            {
                str = testString;
            }
            else
            {
                break;
            }
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
            if(!var && parBal==0 && (str.at(i).isLetterOrNumber() || str.at(i) == '_' || str.at(i) == '-' || str.at(i) == '.')) //New variable or function string
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
            else if(var && !(str.at(i).isLetterOrNumber() || str.at(i) == '_' || str.at(i) == '.'))     //End of variable, append it to symbols
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
            {
                symbols.append(str.at(i));
            }
            else if(i == str.size()-1 && (var || parBal>0) && str.at(i) != ')')
            {
                symbols.append(str.mid(start, i-start+1));
            }
        }
    }


    //Store function derivatives in object
    //! @todo This shall probably not be stored in every expression
    mFunctionDerivatives.insert("sin", "cos");
    mFunctionDerivatives.insert("cos", "-sin");
    mFunctionDerivatives.insert("abs", "sign");
    mFunctionDerivatives.insert("onPositive", "dxOnPositive");
    mFunctionDerivatives.insert("onNegative", "dxOnNegative");
    mFunctionDerivatives.insert("signedSquareL", "dxSignedSquareL");
    mFunctionDerivatives.insert("limit", "dxLimit");

    //Store reserved symbols in object
    reservedSymbols << "mTime" << "Z";


    //Find top level symbol, set correct string and type, generate children
    if(splitAtFirstSeparator("=", symbols, simplifications))                        //Equality
    {
        mType = Expression::Equality;
    }
    else if(splitAtFirstSeparator("+", symbols, simplifications))                   //Addition
    {
        mType = Expression::Operator;
    }
    else if(splitAtFirstSeparator("*", symbols, simplifications))                   //Multiplication
    {
        mType = Expression::Operator;
    }
    else if(splitAtFirstSeparator("/", symbols, simplifications, parentSeparator))  //Division
    {
        mType = Expression::Operator;
    }
    else if(splitAtFirstSeparator("^", symbols, simplifications))                   //Power
    {
        mType = Expression::Operator;
    }
    else if(splitAtFirstSeparator("%", symbols, simplifications))                   //Modulus
    {
        mType = Expression::Operator;
    }
    else if(symbols.size() == 1 && symbols.first().contains("("))                   //Function
    {
        QString str = symbols.first();
        mType = Expression::Function;
        mString = str.left(str.indexOf("("));
        str = str.mid(str.indexOf("(")+1, str.size()-2-str.indexOf("("));

        QStringList args = splitWithRespectToParentheses(str, ',');
        //qDebug() << "Function: " << mString << ", arguments: " << args;
        for(int i=0; i<args.size(); ++i)
        {
            mChildren.append(Expression(args.at(i)));
        }
    }
    else                                                                            //Symbol
    {
        mType = Expression::Symbol;
        mString = symbols.first();

        //Make sure numerical symbols have double precision
        bool isInt;
        mString.toInt(&isInt);
        if(isInt && !mString.contains("."))
        {
            mString.append(".0");
        }
    }


    //Perform simplifications (but not for symbols, because that is pointless)
    if(mType != Expression::Symbol)
    {
        _simplify(simplifications);
    }
}


//! @brief Equality operator for expressions
bool Expression::operator==(const Expression &other) const
{
    if(mChildren.size() != other._getChildren().size())
    {
        return false;
    }

    if(mString != other._getString())
    {
        return false;
    }

    if(mString == "+")
    {
        QList<Expression> terms = getTerms();
        QList<Expression> otherTerms = other.getTerms();
        if(terms.size() != otherTerms.size())
        {
            return false;
        }
        for(int t=0; t<terms.size(); ++t)
        {
            otherTerms.removeOne(terms[t]);
        }

        return otherTerms.isEmpty();
    }
    else if(mString == "*" || mString == "/")
    {
        QList<Expression> factors = getFactors();
        QList<Expression> otherFactors = other.getFactors();
        QList<Expression> divisors = getDivisors();
        QList<Expression> otherDivisors = other.getDivisors();

        //Collect numerical factors in term 1
        double num=1;
        for(int f=0; f<factors.size(); ++f)
        {
            if(factors[f].isNumericalSymbol())
            {
                num *= factors[f].toDouble();
                factors.removeAt(f);
                --f;
            }
            else if(factors[f].getType() == Expression::Symbol || factors[f].getType() == Expression::Function)
            {
                if(factors[f].isNegative())
                {
                    factors[f] = factors[f].negative();
                    num *= -1;
                }
            }
        }
        factors.append(Expression(num));

        //Collect numerical factors in term 2
        num=1;
        for(int f=0; f<otherFactors.size(); ++f)
        {
            if(otherFactors[f].isNumericalSymbol())
            {
                num *= otherFactors[f].toDouble();
                otherFactors.removeAt(f);
                --f;
            }
            else if(otherFactors[f].getType() == Expression::Symbol || otherFactors[f].getType() == Expression::Function)
            {
                if(otherFactors[f].isNegative())
                {
                    otherFactors[f] = otherFactors[f].negative();
                    num *= -1;
                }
            }
        }
        otherFactors.append(Expression(num));


        if(factors.size() != otherFactors.size() || divisors.size() != otherDivisors.size())
        {
            return false;
        }

        int neg=1;
        for(int f=0; f<factors.size(); ++f)
        {
            if(!otherFactors.removeOne(factors[f]))
            {
                otherFactors.removeOne(factors[f].negative());
                neg *= -1;
            }
        }
        for(int d=0; d<divisors.size(); ++d)
        {
            if(!otherDivisors.removeOne(divisors[d]))
            {
                otherDivisors.removeOne(divisors[d].negative());
                neg *= -1;
            }
        }

        return (otherDivisors.isEmpty() && otherFactors.isEmpty() && neg == 1);
    }

    for(int i=0; i<mChildren.size(); ++i)
    {
        if(!(mChildren[i] == other.getChild(i)))
        {
            return false;
        }
    }

    return true;
}


//! @brief Returns the negative of the expression
Expression Expression::negative() const
{
    if(mType == Expression::Symbol || mType == Expression::Function)
    {
        QString ret = this->toString();
        if(ret.startsWith("-"))
        {
            ret = ret.right(ret.size()-1);
            return Expression(ret);
        }
        else
        {
            ret.prepend("-");
            return Expression(ret);
        }
    }
    return Expression(Expression(-1), "*", (*this));
}


//! @brief Counts how many times a sub exprsesion is used in the expression
//! @param var Expression to count
int Expression::count(const Expression &var) const
{
    if(*this == var)
    {
        return 1;
    }

    int retval=0;
    for(int c=0; c<mChildren.size(); ++c)
    {
        retval += mChildren[c].count(var);
    }
    return retval;
}


//! @brief Replaces this expression by another one
//! @param expr Expression to replace by
void Expression::replaceBy(const Expression expr)
{
    mType = expr.getType();
    mString = expr._getString();
    mChildren = expr._getChildren();
}


//! @brief Divides the expression by specified divisor
//! @param div Divisor expression
void Expression::divideBy(const Expression div)
{
    assert(!(div == Expression(0.0)));

    if(mString == "0.0")    //Dividing zero expression will give zero
    {
        return;
    }
    QList<Expression> terms = this->getTerms();
    if(terms.isEmpty())
    {
        this->replaceBy(Expression(*this, "/", div));
    }
    else
    {
        for(int t=0; t<terms.size(); ++t)
        {
            if(terms[t] == div)
            {
                terms[t] = Expression(1);
            }
            else
            {
                terms[t] = Expression(terms[t], "/", div, Expression::SimplifyWithoutMakingPowers);
            }
        }
        this->replaceBy(Expression(terms, "+", Expression::SimplifyWithoutMakingPowers));
    }
}


//! @brief Multiplies the expression by specified divisor
//! @param fac Factor expression
void Expression::multiplyBy(const Expression fac)
{
    if(mString == "0.0")    //Multiplying zero expression will give zero
    {
        return;
    }
    QList<Expression> terms = this->getTerms();
    if(terms.isEmpty())
    {
        this->replaceBy(Expression(*this, "*", fac));
    }
    else
    {
        for(int t=0; t<terms.size(); ++t)
        {
            terms[t] = Expression(terms[t], "*", fac, Expression::SimplifyWithoutMakingPowers);
        }
        this->replaceBy(Expression(terms, "+", Expression::SimplifyWithoutMakingPowers));
    }
}


//! @brief Adds the expression with specified tern
//! @param tern Term expression
void Expression::addBy(const Expression term)
{
    this->replaceBy(Expression(*this, "+", term, Expression::SimplifyWithoutMakingPowers));
}


//! @brief Subtracts the expression by specified term
//! @param tern Term expression
void Expression::subtractBy(const Expression term)
{
    this->replaceBy(Expression(*this, "+", term.negative(), Expression::SimplifyWithoutMakingPowers));
}


//! @brief Returns the type of the expression
Expression::ExpressionTypeT Expression::getType() const
{
    return mType;
}


//! @brief Returns the expression converted to a string
QString Expression::toString() const
{
    QString ret;

    if(mType == Expression::Symbol)
    {
        ret = mString;
    }
    else if(mType == Expression::Function)
    {
        ret = mString+"(";
        for(int i=0; i<mChildren.size(); ++i)
        {
            ret.append(mChildren[i].toString()+",");
        }
        ret.chop(1);
        ret.append(")");
    }
    else if((mType == Expression::Operator || mType == Expression::Equality) && mChildren.size() > 1)
    {
        QString leftStr = mChildren[0].toString();
        QString rightStr = mChildren[1].toString();

        if(this->isMultiplyOrDivide())
        {
            if(mChildren[0].isAdd())
            {
                leftStr.append(")");
                leftStr.prepend("(");
            }
            if(mChildren[1].isAdd())
            {
                rightStr.append(")");
                rightStr.prepend("(");
            }
            if(mString == "/" && mChildren[0]._getString() == "*")
            {
                leftStr.append(")");
                leftStr.prepend("(");
            }
            if(mString == "/" && mChildren[1].isMultiplyOrDivide())
            {
                rightStr.append(")");
                rightStr.prepend("(");
            }
        }
        else if(this->isPower())
        {
            if(mChildren[0].isAdd() || mChildren[0].isMultiplyOrDivide())
            {
                leftStr.append(")");
                leftStr.prepend("(");
            }
            if(mChildren[1].isAdd() || mChildren[1].isMultiplyOrDivide())
            {
                rightStr.append(")");
                rightStr.prepend("(");
            }
        }
        ret = leftStr + mString + rightStr;
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
    if(terms.isEmpty())
    {
        terms.append(*this);
    }

    //Cycle terms
    for(int t=0; t<terms.size(); ++t)
    {
        int nPos = terms[t].count(Expression("Z"));
        int nNeg = terms[t].count(Expression("-Z"));

        //Number of Z operators in term
        int idx = nPos+nNeg;

        //Remove all Z operators
        if(idx > 0)
        {
            terms[t].removeFactor(Expression("Z"));
        }

        while(termMap.size() < idx+1)
        {
            termMap.append(QList<Expression>());
        }
        //Store delay term
        termMap[idx].append(terms[t]);
    }

    //Replace delayed terms with delay function and store delay terms and delay steps in reference vectors
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

            delayTerm.factorMostCommonFactor();

            QString term = "mDelay"+QString::number(rDelayTerms.size())+".getIdx(1.0)";
            ret.append(term);
            ret.append("+");

            rDelayTerms.append(delayTerm);
            rDelaySteps.append(QString::number(i));
        }
    }
    for(int t=0; t<termMap[0].size(); ++t)
    {
        ret.append("("+termMap[0][t].toString()+")");
        ret.append("+");
    }
    ret.removeLast();

    //Replace this expression by the new one
    this->replaceBy(Expression(ret));

    //Simplify
    this->_simplify(Expression::FullSimplification, Recursive);
}


//! @brief Converts expression to double if possible
//! @param ok True if success, false if failed (= not a numerical symbol)
double Expression::toDouble(bool *ok) const
{
    if(mType == Expression::Symbol)
    {
        return mString.toDouble(ok);
    }
    else
    {
        *ok = false;
        return 0.0;
    }
}


//! @brief Tells whether or not this is a power operator
bool Expression::isPower() const
{
    return (mString =="^" || mString == "pow");
}


//! @brief Tells whether or not this is a multiplication or division
bool Expression::isMultiplyOrDivide() const
{
    return (mString == "*" || mString == "/");
}


//! @brief Tells whether or not this is an addition
bool Expression::isAdd() const
{
    return (mString == "+");
}


//! @brief Tells whether or not this is a numerical symbol
bool Expression::isNumericalSymbol() const
{
    if(mType == Expression::Symbol)
    {
        bool isNumber;
        mString.toDouble(&isNumber);
        return  isNumber;
    }
    else
    {
        return false;
    }
}


//! @brief Tells whether or not this is an assignment
bool Expression::isAssignment() const
{
    return (mType == Expression::Equality && mChildren[0].getType() == Expression::Symbol);
}


//! @brief Tells whether or not this is an equation
bool Expression::isEquation() const
{
    return (mType == Expression::Equality);
}


//! @brief Tells whether or not this is a negative symbol
bool Expression::isNegative() const
{
    return ((mType == Expression::Symbol || mType == Expression::Function) && mString.startsWith("-") && mString.size() > 1);
}


//! @brief Returns the derivative of the expression
//! @param x Expression to differentiate with
//! @param ok True if successful, otherwise false
Expression Expression::derivative(const Expression x, bool &ok) const
{
    ok = true;
    QString ret;

    //Equality, differentiate left and right expressions
    if(mType == Expression::Equality)
    {
        bool success;
        ret.append(mChildren[0].derivative(x, success).toString());
        if(!success) { ok = false; }
        ret.append("=");
        ret.append(mChildren[1].derivative(x, success).toString());
        if(!success) { ok = false; }
    }
    //Function
    else if(mType == Expression::Function)
    {
        QString f = this->toString();
        QString g;
        QString dg;
        if(!mChildren.isEmpty())
        {
            g = mChildren[0].toString();    //First argument
            bool success;
            dg = mChildren[0].derivative(x, success).toString();    //Derivative of first argument
            if(!success) { ok = false; }
        }

        QString func = mString;
        bool negative = false;
        if(func.startsWith('-'))        //Rememver that the function was negative
        {
            func = func.right(func.size()-1);
            negative = true;
        }

        //Custom functions
        if(func == "log")
        {
            ret = "("+dg+")/("+g+")";
        }
        else if(func == "exp")
        {
            ret = "("+dg+")*("+f+")";
        }
        else if(func == "tan")
        {
            ret = "2*("+dg+")/(cos(2*"+g+")+1)";
        }
        else if(func == "atan" || func == "atan2")
        {
            ret = "("+dg+")/(("+g+")^2+1)";
        }
        else if(func == "asin")
        {
            ret = "("+dg+")/sqrt(1-("+g+")^2)";
        }
        else if(func == "acos")
        {
            ret = "-("+dg+")/sqrt(1-("+g+")^2)";
        }
        else if(func == "mod")
        {
            ret = "0.0";
        }
        else if(func == "rem")
        {
            ret = "0.0";
        }
        else if(func == "sqrt")
        {
            ret = "("+dg+")/(2*sqrt("+g+"))";
        }
        else if(func == "sign")
        {
            ret = "0.0";
        }
        else if(func == "re")
        {
            ret = "0.0";
        }
        else if(func == "ceil")
        {
            ret = "0.0";
        }
        else if(func == "floor")
        {
            ret = "0.0";
        }
        else if(func == "int")
        {
            ret = "0.0";
        }
        else if(func == "dxLimit")
        {
            ret = "0.0";
        }
        else if(func == "mDelay")
        {
            ret = "0.0";
        }
        else if(func.startsWith("mDelay"))
        {
            ret = "0.0";
        }
        else if(func == "pow")
        {
            if(mChildren[0]._getString() == "Z" || mChildren[0]._getString() == "-Z")
            {
                ret = "0.0";
            }
            else
            {
                bool success;
                QString f = mChildren[0].toString();
                QString df = mChildren[0].derivative(x, success).toString();
                if(!success) { ok = false; }
                QString g = mChildren[1].toString();
                QString dg = mChildren[1].derivative(x, success).toString();
                if(!success) { ok = false; }

                ret = "pow("+f+","+g+"-1)*(("+g+")*("+df+")+("+f+")*log(("+f+"))*("+dg+"))";
            }
        }
        //No special function, so use chain rule
        else
        {
            if(!mFunctionDerivatives.contains(func))
            {
                //QMessageBox::critical(0, "SymHop", "Could not compute function derivative of \"" + func +"\": Not implemented.");
                ok = false;
            }
            else
            {
                ret = this->toString();
                ret = ret.mid(ret.indexOf("("), ret.size()-1);
                ret.prepend(mFunctionDerivatives.find(func).value());
                ret.append("*");
                bool success;
                ret.append(mChildren.first().derivative(x, success).toString());
                if(!success) { ok = false; }
            }
        }

        if(negative)
        {
            ret.prepend("-");
        }
    }
    //Multiplication, d/dx(f(x)*g(x)) = f'(x)*g(x) + f(x)*g'(x)
    else if(mType == Expression::Operator && mString == "*")
    {
        //Derivative of Z is zero
        if(mChildren[0]._getString() == "Z" || mChildren[0]._getString() == "-Z" || mChildren[1]._getString() == "Z" || mChildren[1]._getString() == "-Z")
        {
            ret = "0.0";
        }
        else
        {
            bool success;
            Expression exp1 = mChildren[0].derivative(x, success);
            if(!success) { ok = false; }
            Expression exp2 = mChildren[1].derivative(x, success);
            if(!success) { ok = false; }
            ret = "("+exp1.toString()+")*("+mChildren[1].toString()+")+("+mChildren[0].toString()+")*("+exp2.toString()+")";
        }
    }
    //Division, d/dx(f(x)/g(x)) = (g(x)*f'(x)-f(x)*g'(x))/g(x)^2
    else if(mType == Expression::Operator && mString == "/")
    {
        if(mChildren[0]._getString() == "Z" || mChildren[0]._getString() == "-Z")   //Derivative of Z is zero
        {
            ret = "0.0";
        }
        else
        {
            QString exp1 = mChildren[0].toString();
            QString exp2 = mChildren[1].toString();
            bool success;
            QString der1 = mChildren[0].derivative(x, success).toString();
            if(!success) { ok = false; }
            QString der2 = mChildren[1].derivative(x, success).toString();
            if(!success) { ok = false; }
            ret = "(("+exp2+")*("+der1+")-("+exp1+")*("+der2+"))/("+exp2+")^2";
        }
    }
    //Addition, derive left and right sides
    else if(mType == Expression::Operator && mString == "+")
    {
        bool success1, success2;
        QString leftDer, rightDer;
        if(mChildren[0]._getString() == "Z" || mChildren[0]._getString() == "-Z")   //Derivative of Z is zero
        {
            leftDer = "0.0";
        }
        else
        {
            leftDer = mChildren[0].derivative(x, success1).toString();
        }
        if(mChildren[1]._getString() == "Z" || mChildren[1]._getString() == "-Z")   //Derivative of Z is zero
        {
            rightDer = "0.0";
        }
        else
        {
            rightDer = mChildren[1].derivative(x, success2).toString();
        }

        ret = leftDer+mString+rightDer;
        if(!success1 || !success2) { ok = false; }
    }
    //Power, d/dx(f(x)^g(x)) = (g(x)*f'(x)+f(x)*log(f(x))*g'(x)) * f(x)^(g(x)-1)
    else if(mType == Expression::Operator && mString == "^")
    {
        bool success;
        QString f = mChildren[0].toString();
        QString df = mChildren[0].derivative(x, success).toString();
        if(!success) { ok = false; }
        QString g = mChildren[1].toString();
        QString dg = mChildren[1].derivative(x, success).toString();
        if(!success) { ok = false; }

        ret = "("+f+")^("+g+"-1)*(("+g+")*("+df+")+("+f+")*log(("+f+"))*("+dg+"))";
    }
    //Symbol
    else
    {
        if(*this == x)
        {
            ret = "1.0";
        }
        else
        {
            ret = "0.0";
        }
    }

    return Expression(ret);
}


//! @brief Returns whether or not expression contains a sub expression
//! @param expr Expression to check for
bool Expression::contains(const Expression expr) const
{
    if(expr.toString() == this->toString())
    {
        return true;
    }

    for(int i=0; i<mChildren.size(); ++i)
    {
        if(mChildren[i].contains(expr))
        {
            return true;
        }
    }

    return false;
}


//! @brief Converts time derivatives (der) in the expression to Z operators with bilinar transform
Expression Expression::bilinearTransform() const
{
    Expression tempExpr;
    tempExpr.replaceBy(*this);
    QStringList res;

    tempExpr._getChildrenPtr()->clear();
    for(int i=0; i<mChildren.size(); ++i)
    {
        tempExpr._getChildrenPtr()->append(mChildren[i].bilinearTransform());
    }
    if(mString == "-der")
    {
        QString arg = tempExpr.getChild(0).toString();
        res << "-2" << "/" << "mTimestep" << "*" << "(1.0-Z)" << "/" << "(1.0+Z)" << "*" << "("+arg+")";
    }
    if(mString == "der")
    {
        QString arg = tempExpr.getChild(0).toString();
        res << "2.0" << "/" << "mTimestep" << "*" << "(1.0-Z)" << "/" << "(1.0+Z)" << "*" << "("+arg+")";
    }
    else
    {

    }

    if(res.isEmpty())
    {
        return tempExpr;
    }
    else
    {
        return Expression(res);
    }
}


//! @brief Returns a list with all contained symbols in the expression
QList<Expression> Expression::getSymbols() const
{
    QList<Expression> retval;
    for(int i=0; i<mChildren.size(); ++i)
    {
        retval.append(mChildren[i].getSymbols());
    }
    if(mType == Expression::Symbol && mString.startsWith("-") && mString.size() > 1 && mString[1].isLetter())
    {
        QString temp = mString;
        temp = temp.right(temp.size()-1);
        if(!reservedSymbols.contains(temp))
        {
            retval.append(Expression(temp));
        }
    }
    if(mType == Expression::Symbol && !reservedSymbols.contains(mString) && mString[0].isLetter())
    {
        retval.append(*this);
    }

    removeDuplicates(retval);

    return retval;
}


//! @brief Returns a list with all used functions in the expression
QStringList Expression::getFunctions() const
{
    QStringList retval;
    for(int i=0; i<mChildren.size(); ++i)
    {
        retval.append(mChildren[i].getFunctions());
    }
    if(mType == Expression::Function)
    {
        retval.append(mString);
        if(mString.startsWith('-'))
        {
            retval.last() = mString.right(mString.size()-1);
        }
    }

    retval.removeDuplicates();

    return retval;
}


//! @brief Returns the function name, or an empty string is this is not a function
QString Expression::getFunctionName() const
{
    if(mType == Expression::Function)
    {
        return mString;
    }
    else
    {
        return QString();
    }
}


//! @brief Returns the symbol name, or an empty string is this is not a symbol
QString Expression::getSymbolName() const
{
    if(mType == Expression::Symbol)
    {
        return mString;
    }
    else
    {
        return QString();
    }
}


//! @brief Returns the specified function argument, or an empty string if this is not a function or if it has too few arguments
//! @param idx Index of argument to return
Expression Expression::getArgument(const int idx) const
{
    if(mType == Expression::Function && mChildren.size() > idx)
    {
        return mChildren[idx];
    }
    return Expression();
}


//! @brief Returns a list of function arguments, or an empty list if this is not a function
QList<Expression> Expression::getArguments() const
{
    QList<Expression> retval;
    if(mType == Expression::Function)
    {
        for(int i=0; i<mChildren.size(); ++i)
        {
            retval.append(mChildren[i]);
        }
    }
    return retval;
}


//! @brief Returns child with specified index, or an empty expression if child does not exist
//! @param idx Index of child to return
Expression Expression::getChild(const int idx) const
{
    if(mChildren.size() > idx)
        return mChildren[idx];
    else
        return Expression();
}


//! @brief Returns a list of all term in the expression
QList<Expression> Expression::getTerms() const
{
    QList<Expression> retval;

    if(mString != "+")
    {
        return retval;      //Do nothing if node is not an addition
    }

    for(int i=0; i<mChildren.size(); ++i)
    {
        if(mChildren[i].isAdd())
        {
            retval.append(mChildren[i].getTerms());
        }
        else
        {
            retval.append(mChildren[i]);
        }
    }

    return retval;
}


//! @brief Returns a list of all factors in the expression
QList<Expression> Expression::getFactors() const
{
    QList<Expression> retval;

    if(mString != "*" && mString != "/")
    {
        return retval;      //Do nothing if node is not a multiplication or division
    }

    if(mString == "*")
    {
        for(int i=0; i<mChildren.size(); ++i)
        {
            if(mChildren[i].isMultiplyOrDivide())
            {
                retval.append(mChildren[i].getFactors());
            }
            else
            {
                retval.append(mChildren[i]);
            }
        }
    }
    else if(mString == "/")
    {
        if(mChildren[0].isMultiplyOrDivide())
        {
            retval.append(mChildren[0].getFactors());
        }
        else
        {
            retval.append(mChildren[0]);
        }
    }

    return retval;
}


//! @brief Returns a list of all divisors in the expression
QList<Expression> Expression::getDivisors() const
{
    QList<Expression> divisors;


    if(mString == "/")
    {
        if(mChildren[1]._getString() == "*")
        {
            divisors.append(mChildren[1].getFactors());
        }
        else
        {
            divisors.append(mChildren[1]);
        }
        divisors.append(mChildren[0].getDivisors());    //Don't search right child for divisors in case right side IS the divisor!
    }
    else if(mString == "*")
    {
        for(int i=0; i<mChildren.size(); ++i)
        {
            if(mChildren[i].isMultiplyOrDivide())
            {
                divisors.append(mChildren[i].getDivisors());
            }
        }
    }

    return divisors;
}


//! @brief Removes all divisors in the expression
void Expression::removeDivisors()
{
    if(!isMultiplyOrDivide())
    {
        return;
    }
    QList<Expression> factors = getFactors();
//    QStringList ret;
//    for(int f=0; f<factors.size(); ++f)
//    {
//        ret << factors[f].toString() << "*";
//    }
//    ret.removeLast();
    this->replaceBy(Expression(factors, "*", NoSimplifications));
}


//! @brief Removes specified factor in the expression
//! @param var Factor to remove
void Expression::removeFactor(const Expression var)
{
    for(int i=0; i<mChildren.size(); ++i)
    {
        mChildren[i].removeFactor(var);
    }

    if(*this == var)
    {
        this->replaceBy(Expression(1));
        return;
    }

    QList<Expression> factors = this->getFactors();
    QList<Expression> divisors = this->getDivisors();
    if(this->isMultiplyOrDivide() && !factors.isEmpty())
    {
        factors.removeAll(var);
        while(factors.contains(var.negative()))
        {
            factors.removeOne(var.negative());
            factors.append(Expression(-1.0));
        }
        Expression tempExpr1;
        Expression tempExpr2;
        if(!divisors.isEmpty() && !factors.isEmpty())
        {
            tempExpr1 = Expression(factors, "*", TrivialSimplifications);
            tempExpr2 = Expression(divisors, "*", TrivialSimplifications);
            this->replaceBy(Expression(tempExpr1, "/", tempExpr2, TrivialSimplifications));
        }
        else if(!factors.isEmpty())
        {
            tempExpr1 = Expression(factors, "*", TrivialSimplifications);
            this->replaceBy(tempExpr1);
        }
        else if (!divisors.isEmpty())
        {
            tempExpr2 = Expression(divisors, "*", TrivialSimplifications);
            this->replaceBy(Expression(Expression(1.0), "/", tempExpr2, TrivialSimplifications));
        }
        else
        {
            this->replaceBy(Expression(1.0));
        }
    }
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
        for(int c=0; c<mChildren.size(); ++c)
        {
            mChildren[c].replace(oldExpr, newExpr);
        }
    }
    return;
}


//! @brief Expands all parentheses and powers in the expression
void Expression::expand()
{
    expandPowers();
    expandParentheses(TrivialSimplifications);
    _simplify(SimplifyWithoutMakingPowers, Recursive);
}


//! @brief Expands all powers in the expression
void Expression::expandPowers()
{
    for(int c=0; c<mChildren.size(); ++c)
    {
        mChildren[c].expandPowers();
    }

    QList<Expression> terms = getTerms();
    if(terms.isEmpty())
    {
        if(mType != Expression::Equality)
        {
            terms.append(*this);
        }
        else
        {
            return;
        }
    }

    //Expand all power functions (if they have integer exponents)
    for(int t=0; t<terms.size(); ++t)
    {
        QList<Expression> factors = terms[t].getFactors();
        QList<Expression> divisors = terms[t].getDivisors();

        if(factors.isEmpty())
        {
            factors.append(terms[t]);
        }
        for(int f=0; f<factors.size(); ++f)
        {
            if(factors[f].isPower())
            {
                //qDebug() << "factors[f]: " << factors[f].toString();
                Expression base = factors[f].getChild(0);
                double exp = factors[f].getChild(1).toDouble();
                if(exp-floor(exp) == 0.0)
                {
                    factors.removeAt(f);
                    for(int i=0; i<int(exp); ++i)
                    {
                        factors.append(base);
                    }
                }
            }
        }
        QStringList strExpr;
        for(int f2=0; f2<factors.size(); ++f2)
        {
            strExpr << "("+factors[f2].toString()+")" << "*";
        }
        if(!strExpr.isEmpty())
        {
            strExpr.removeLast();
        }
        if(strExpr.isEmpty())
        {
            strExpr.append("1.0");
        }
        for(int d=0; d<divisors.size(); ++d)
        {
            strExpr << "/" << divisors[d].toString();
        }
        terms[t] = Expression(strExpr, TrivialSimplifications);
    }


    QStringList res;
    for(int t=0; t<terms.size(); ++t)
    {
        res << "("+terms[t].toString()+")" << "+";
    }
    if(!res.isEmpty())
    {
        res.removeLast();
    }
    this->replaceBy(Expression(res, TrivialSimplifications));
}


//! @brief Expands all parentheses in the expression
//! @param simplifications Specifies the degree of simplification
void Expression::expandParentheses(const ExpressionSimplificationT simplifications)
{
    if(mType == Expression::Symbol || mType == Expression::Function) { return; }

    for(int c=0; c<mChildren.size(); ++c)
    {
        mChildren[c].expandParentheses(simplifications);
    }

    QString start = this->toString();

    while(true)
    {
         QList<Expression> terms = getTerms();
        if(terms.isEmpty())
        {
            if(mType != Expression::Equality)
            {
                terms.append(*this);
            }
            else
            {
                return;
            }
        }

        bool foundTermWithFactorWithSubTerms = false;
        for(int t=0; t<terms.size(); ++t)
        {
            QList<Expression> factors = terms[t].getFactors();
            QList<Expression> divisors = terms[t].getDivisors();

            //Cancel out same factors and divisors
            if(!divisors.isEmpty())
            {
                for(int f=0; f<factors.size(); ++f)
                {
                    if(f>=0 && divisors.contains(factors[f]))
                    {
                        divisors.removeOne(factors[f]);
                        factors.removeAt(f);
                        f=0;
                        continue;
                    }
                    else if(f>=0 && divisors.contains(factors[f].negative()))
                    {
                        divisors.removeOne(factors[f].negative());
                        factors.removeAt(f);
                        factors.append(Expression(-1.0));
                        f=0;
                        continue;
                    }
                }
            }

            for(int f=0; f<factors.size(); ++f)
            {
                QList<Expression> subTerms = factors[f].getTerms();
                if(subTerms.size() > 1)
                {
                    foundTermWithFactorWithSubTerms = true;
                    terms.removeAt(t);
                    factors.removeAt(f);
                    QStringList resTerm;
                    for(int f2=0; f2<factors.size(); ++f2)
                    {
                        resTerm << factors[f2].toString() << "*";
                    }
                    if(!resTerm.isEmpty())
                    {
                        resTerm.removeLast();
                    }
                    if(resTerm.isEmpty())
                    {
                        resTerm << "1.0";
                    }
                    for(int d=0; d<divisors.size(); ++d)
                    {
                        resTerm << "/" << divisors[d].toString();
                    }
                    if(resTerm.size() > 1 && resTerm[0] == "1.0" && resTerm[1] == "/")
                    {
                        resTerm.removeAt(0);
                    }
                    if(!resTerm.isEmpty() && !resTerm.startsWith("/"))
                    {
                        resTerm.prepend("*");
                    }
                    for(int s=0; s<subTerms.size(); ++s)
                    {
                        resTerm.prepend("("+subTerms[s].toString()+")");
                        terms.append(Expression(resTerm, TrivialSimplifications));
                        resTerm.removeAt(0);
                    }
                    t=0;
                    f=0;
                }
            }
        }
        if(!foundTermWithFactorWithSubTerms)
        {
            break;
        }

        this->replaceBy(Expression(terms,"+"));

//        QStringList res;
//        for(int t=0; t<terms.size(); ++t)
//        {
//            res << "("+terms[t].toString()+")" << "+";
//        }
//        if(!res.isEmpty())
//        {
//            res.removeLast();
//        }
//        this->replaceBy(Expression(res, simplifications));
    }

    return;
}


//! @brief Linearizes the expression by multiplying with all divisors until no divisors remains
//! @note Should only be used on equations (obviously)
void Expression::linearize()
{
    this->expand();

    while(true)
    {
        QList<Expression> terms = getTerms();
        if(terms.isEmpty())
        {
            return;
        }

        QList<Expression> divisors;
        QList<QList<int> > divisorToTermMap;
        for(int t=0; t<terms.size(); ++t)
        {
            divisorToTermMap.append(QList<int>());
            QList<Expression> subDivisors = terms[t].getDivisors();

            //Merge duplicate divisors (otherwise they will cause trouble below with the divisorToTermMap)
            for(int s=0; s<subDivisors.size(); ++s)
            {
                int n = subDivisors.count(subDivisors[s]);
                if(n > 1)
                {
//                    QStringList resDiv;
//                    for(int i=0; i<n; ++i)
//                    {
//                        resDiv << subDivisors[s].toString() << "*";
//                    }
//                    resDiv.removeLast();
                    QList<Expression> resDiv;
                    for(int i=0; i<n; ++i)
                    {
                        resDiv.append(subDivisors[s]);
                    }
                    subDivisors.removeAll(subDivisors[s]);
                    subDivisors.append(Expression(resDiv, "*", TrivialSimplifications));
                }
            }

            for(int s=0; s<subDivisors.size(); ++s)
            {
                if(!divisors.contains(subDivisors[s]))
                {
                    divisors.append(subDivisors[s]);
                }
                divisorToTermMap[t].append(divisors.indexOf(subDivisors[s]));
                terms[t].removeDivisors();
            }
        }
        if(divisors.isEmpty())
        {
            break;
        }

        QStringList divisorList;
        for(int d=0; d<divisors.size(); ++d)
        {
            divisorList << divisors[d].toString();
        }

        QStringList res;
        for(int t=0; t<terms.size(); ++t)
        {
            for(int d=0; d<divisors.size(); ++d)
            {
                if(!divisorToTermMap[t].contains(d))
                {
                    res << "("+divisors[d].toString()+")"<<"*";
                }
            }
            res << terms[t].toString() << "+";
        }
        res.removeLast();

        Expression tempExpr = Expression(res, TrivialSimplifications);

        this->replaceBy(tempExpr);

        expandParentheses(TrivialSimplifications);

        _simplify(Expression::SimplifyWithoutMakingPowers, NonRecursive);
    }


    //Divide by common factors (that exist in all terms)
    while(true)
    {
        expandPowers();
        QList<Expression> terms = getTerms();
        QList<Expression> commonFactors;
        QList<Expression> termFactors = terms[0].getFactors();
        for(int f=0; f<termFactors.size(); ++f)
        {
            if(!(termFactors[f] == Expression(1.0)) && !(termFactors[f] == Expression(-1.0)))
            {
                if(!commonFactors.contains(termFactors[f]) && !commonFactors.contains(termFactors[f].negative()))
                {
                    commonFactors.append(termFactors[f]);
                }
            }
        }
        for(int t=1; t<terms.size() && !commonFactors.isEmpty(); ++t)
        {
            termFactors = terms[t].getFactors();
            for(int c=0; c<commonFactors.size(); ++c)
            {
                if(!termFactors.contains(commonFactors[c]) && !termFactors.contains(commonFactors[c].negative()))
                {
                    commonFactors.removeAt(c);
                    --c;
                }
            }
        }
        if(commonFactors.size() > 0)
        {
            QStringList div;
            for(int c=0; c<commonFactors.size(); ++c)
            {
                div << commonFactors[c].toString() << "/";
            }
            div.removeLast();
            for(int t=0; t<terms.size(); ++t)
            {
                terms[t] = Expression(terms[t], "/", div);
            }
            this->replaceBy(Expression(terms, "+"));
        }
        else
        {
            break;
        }
    }

    _simplify(FullSimplification, Recursive);

    return;
}


//! @brief Moves all right side expressions to the left side, if this is an equation
void Expression::toLeftSided()
{
    if(this->isEquation())
    {
        mChildren[0] = Expression(mChildren[0], "+", mChildren[1].negative());
        mChildren[1] = Expression(0.0);
    }
}


//! @brief Factors specified expression
//! @param var Expression to factor
//! @example Factorizing "a*b*c + a*c*d + b*c*d" for "b" => "b*(a*c + c*d) + a*c*d"
void Expression::factor(const Expression var)
{
    QStringList termsWithVar;
    QList<Expression> terms = getTerms();
    for(int t=0; t<terms.size(); ++t)
    {
        if(terms[t] == var)
        {
            termsWithVar << "1.0" << "+";
            terms.removeAt(t);
            --t;
        }
        else if(terms[t] == var.negative())
        {
            termsWithVar << "-1.0" << "+";
            terms.removeAt(t);
            --t;
        }
        else
        {
            QList<Expression> factors = terms[t].getFactors();
            if(factors.count(var)+factors.count(var.negative()) == 1)
            {
                Expression tempExpr = var.negative();
                terms[t] = Expression(terms[t], "/", var, FullSimplification);
                termsWithVar << terms[t].toString() << "+";
                terms.removeAt(t);
                --t;
            }
        }
    }

    if(!termsWithVar.isEmpty())
    {
        termsWithVar.removeLast();
        QStringList ret;
        ret << var.toString() << "*" << Expression(termsWithVar).toString();
        for(int t=0; t<terms.size(); ++t)
        {
            ret << "+" << terms[t].toString();
        }

        this->replaceBy(Expression(ret, NoSimplifications));
        this->_simplify(Expression::FullSimplification, Recursive);
    }
}


//! @brief Factors the most common factor in the expression
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


//! @brief Internal function, returns a list with all children
QList<Expression> Expression::_getChildren() const
{
    return mChildren;
}


//! @brief Internal function, returns a pointer to the list of children
QList<Expression> *Expression::_getChildrenPtr()
{
    return &(mChildren);
}


//! @brief Internal function, returns the contained string
QString Expression::_getString() const
{
    return mString;
}


//! @brief Verifies that the expression is correct
bool Expression::verifyExpression()
{

    //Verify all functions
    if(!_verifyFunctions())
    {
        return false;
    }

    return true;
}


//! @brief Verifies that all functions are supported
bool Expression::_verifyFunctions() const
{
    bool success = true;

    QStringList functions = this->getFunctions();
    for(int i=0; i<functions.size(); ++i)
    {
        if(!getSupportedFunctionsList().contains(functions[i]) && !getCustomFunctionList().contains(functions[i]))
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
void Expression::_simplify(ExpressionSimplificationT type, const ExpressionRecursiveT recursive)
{
    if(type == NoSimplifications)
    {
        return;
    }


    if(recursive == Recursive)
    {
        for(int i=0; i<mChildren.size(); ++i)
        {
            mChildren[i]._simplify(type, recursive);
        }
    }

    //Trivial simplifications
    if(mString == "*")
    {
        if(mChildren[0]._getString() == "0.0" || mChildren[1]._getString() == "0.0")
        {
            replaceBy(Expression(0.0));     //Replace "0*x" and "x*0" with "0.0"
        }
        else if(mChildren[0]._getString() == "-0.0" || mChildren[1]._getString() == "-0.0")
        {
            replaceBy(Expression(0.0));     //Replace "-0*x" and "x*-0" with "0.0"
        }
        else if(mChildren[0].isNumericalSymbol() && mChildren[1].isNumericalSymbol())
        {
            //Replace "3*2" with "6" (for example)
            double replacement = mChildren[0].toDouble()*mChildren[1].toDouble();
            replaceBy(Expression(replacement));
        }
        else if(mChildren[0]._getString() == "1.0")
        {
            replaceBy(mChildren[1]);        //Replace "x*1" with "x"
        }
        else if(mChildren[1]._getString() == "1.0")
        {
            replaceBy(mChildren[0]);        //Replace "1*x" with "x"
        }
        else if(mChildren[0]._getString() == "-1.0" && mChildren[1].getType() == Expression::Symbol)
        {
            replaceBy(mChildren[1].negative());
        }
        else if(mChildren[1]._getString() == "-1.0" && mChildren[0].getType() == Expression::Symbol)
        {
            replaceBy(mChildren[0].negative());
        }
        else if(mChildren[0].isNegative() && mChildren[1].isNegative())
        {
            mChildren[0] = mChildren[0].negative();
            mChildren[1] = mChildren[1].negative();
        }
    }
    else if(mString == "/")
    {
        if(mChildren[0]._getString() == "0.0")
        {
            replaceBy(Expression(0.0));
        }
        else if(mChildren[0]._getString() == "-0.0")
        {
            replaceBy(Expression(0.0));
        }
        else if(mChildren[1]._getString() == "1.0")
        {
            replaceBy(mChildren[0]);
        }
        else if(mChildren[1]._getString() == "-1.0")
        {
            replaceBy(mChildren[0].negative());
        }
    }
    else if(mString == "+")
    {
        if(mChildren[0].isNumericalSymbol() && mChildren[1].isNumericalSymbol())
        {
            double replacement = mChildren[0].toDouble()+mChildren[1].toDouble();
            replaceBy(Expression(QString::number(replacement), NoSimplifications));
        }
        else if(mChildren[0]._getString() == "0.0")
        {
            replaceBy(mChildren[1]);
        }
        else if(mChildren[1]._getString() == "0.0")
        {
            replaceBy(mChildren[0]);
        }
    }
    else if(mString == "^" || mString == "pow")
    {
        if(mChildren[1].toDouble() == 1.0)
        {
            replaceBy(mChildren[0]);
        }
        else if(mChildren[1].toString().startsWith("-"))
        {
            QString pos = mChildren[1].toString();
            pos.remove(0,1);
            replaceBy(Expression(QStringList() << "1.0" << "/" << "("+mChildren[0].toString()+"^"+pos+")"));
        }
        else        //Replace with number if both base and exponent are numericals
        {
            bool ok1, ok2;
            double x1 = mChildren[0].toDouble(&ok1);
            double x2 = mChildren[1].toDouble(&ok2);
            if(x1 && x2)
            {
                mString = QString::number(pow(x1,x2));
                mType = Expression::Symbol;
                mChildren.clear();
            }
        }
    }
    else if(mType == Expression::Function && mString == "")     //"Empty" function, expression is surrounded by parentheses (should normally never happen)
    {
        QString temp = this->toString();
        temp = temp.mid(1, temp.size()-2);
        replaceBy(Expression(temp));
    }

    if(mString == "^")  //Should power symbols even be allowed?
    {
        mString = "pow";
        mType = Expression::Function;
    }



    if(type == TrivialSimplifications)
    {
        return;
    }



    bool didSomething = false;

    //Simplify terms (if this is an addition)
    QList<Expression> newTerms;
    QList<double> termNums;
    QList<Expression> terms = this->getTerms();

    if(terms.size() > 0)
    {
        //Remove terms that are pureley numerical, and sum up their value
        double numericals = 0;
        for(int t=0; t<terms.size(); ++t)
        {
            bool ok=false;
            numericals += terms[t].toDouble(&ok);
            if(ok)
            {
                terms.removeAt(t);
                --t;
                didSomething = true;
            }
        }

        for(int t=0; t<terms.size(); ++t)
        {
            QList<Expression> factors = terms[t].getFactors();
            QList<Expression> divisors = terms[t].getDivisors();
            if(factors.isEmpty())
            {
                factors.append(terms[t]);
            }

            //Remove numerical factors from the term and remember their value
            double num=1;
            bool changed=false;
            for(int f=0; f<factors.size(); ++f)
            {
                bool ok;
                double tempNum = factors[f].toDouble(&ok);
                if(ok)
                {
                    num *= tempNum;
                    factors.removeAt(f);
                    changed=true;
                    --f;
                }
            }

            //Recreate the term if a factor was removed
            if(changed)
            {
                if(factors.isEmpty())
                {
                    terms[t] = Expression(0);
                }
                else
                {
                    if(divisors.isEmpty())
                    {
                        terms[t] = Expression(factors, "*", TrivialSimplifications);
                    }
                    else
                    {
                        Expression num = Expression(factors, "*", TrivialSimplifications);
                        Expression den = Expression(divisors, "*", TrivialSimplifications);
                        terms[t] = Expression(num, "/", den, TrivialSimplifications);
                    }
                }
            }

            if(newTerms.contains(terms[t]))
            {
                termNums[newTerms.indexOf(terms[t])] += num;
                didSomething=true;
            }
            else if(newTerms.contains(terms[t].negative()))
            {
                termNums[newTerms.indexOf(terms[t].negative())] -= num;
                didSomething=true;
            }
            else
            {
                newTerms.append(terms[t]);
                termNums.append(num);
            }
        }
        if(numericals != 0.0)
        {
            newTerms.append(Expression(QString::number(numericals), NoSimplifications));
            termNums.append(1);
        }
        if(didSomething)
        {
            for(int t=0; t<newTerms.size(); ++t)
            {
                newTerms[t] = Expression(newTerms[t], "*", Expression(termNums[t]));
            }

            this->replaceBy(Expression(newTerms, "+", TrivialSimplifications));

            return;
        }
    }



    //Simplify factors and divisors (if this is multiply or divide)
    QList<Expression> factors = this->getFactors();
    QList<Expression> divisors = this->getDivisors();
    if(type != Expression::SimplifyWithoutMakingPowers)
    {
        for(int f=0; f<factors.size(); ++f)     //Expand power functions (i.e. pow(x,2.3) => x*x*pow(0.3) )
        {
            if(factors[f].isPower() && factors[f].getChild(1).toDouble() > 1)
            {
                Expression expr = factors[f].getChild(0);
                double p=factors[f].getChild(1).toDouble();
                int flr = floor(p);
                double rem = p-flr;
                if(rem != 0.0)
                {
                    factors[f] = Expression("pow(("+expr.toString()+"),"+QString::number(rem)+")");
                }
                else
                {
                    factors.removeAt(f);
                    --f;
                }
                for(int i=0; i<flr; ++i)
                {
                    factors.append(expr);
                }
            }
        }

        for(int d=0; d<divisors.size(); ++d)     //Expand power functions (i.e. pow(x,2.3) => x*x*pow(0.3) )
        {
            if(divisors[d].isPower() && divisors[d].getChild(1).toDouble() > 1)
            {
                Expression expr = divisors[d].getChild(0);
                double p=divisors[d].getChild(1).toDouble();
                int flr = floor(p);
                double rem = p-flr;
                if(rem != 0.0)
                {
                    divisors[d] = Expression("pow(("+expr.toString()+"),"+QString::number(rem)+")", TrivialSimplifications);
                }
                else
                {
                    divisors.removeAt(d);
                    --d;
                }
                for(int i=0; i<flr; ++i)
                {
                    divisors.append(expr);
                }
            }
        }
    }

    //Cancel out same factors and divisors
    if(!divisors.isEmpty())
    {
        for(int f=0; f<factors.size(); ++f)
        {
            while(f>=0 && divisors.contains(factors[f]))
            {
                divisors.removeOne(factors[f]);
                factors.removeAt(f);
                --f;
                didSomething = true;
            }
            while(f>=0 && divisors.contains(factors[f].negative()))
            {
                divisors.removeOne(factors[f].negative());
                factors.removeAt(f);
                factors.append(Expression(-1.0));
                --f;
                didSomething = true;
            }
        }
    }

    //Identify multiple factors and convert them to powers
    QList<QPair<Expression, int> > powers;
    if(type != Expression::SimplifyWithoutMakingPowers)
    {
        for(int f=0; f<factors.size(); ++f)
        {
            if(factors.count(factors[f]) > 1 || divisors.count(factors[f]) > 1)
            {
                powers.append(QPair<Expression,int>(factors[f], factors.count(factors[f])-divisors.count(factors[f])));
                if(!divisors.isEmpty())
                {
                    divisors.removeAll(factors[f]);
                }
                factors.removeAll(factors[f]);
                f=0;
                didSomething = true;
            }
        }
    }


    if(didSomething)
    {
        QStringList exprStr;
        for(int p=0; p<powers.size(); ++p)
        {
            exprStr << "pow("+powers[p].first.toString()+","+QString::number(powers[p].second)+")" << "*";
        }
        for(int f=0; f<factors.size(); ++f)
        {
            exprStr << "("+factors[f].toString()+")" << "*";
        }
        if(!exprStr.isEmpty())
        {
            exprStr.removeLast();
        }

        if(powers.isEmpty() && factors.isEmpty())
        {
            exprStr << "1.0";        //If all factors are cancelled out, the numinator must be 1
        }

        if(!divisors.isEmpty())
        {
            exprStr << "/" << "(";
            for(int t=0; t<divisors.size(); ++t)
            {
                exprStr.last().append("("+divisors[t].toString()+")*");
            }
            exprStr.last().chop(1);
            exprStr.last().append(")");
        }
        Expression tempExpr = Expression(exprStr, TrivialSimplifications);
        //qDebug() << "SIMPLIFY\n  Before: " << toString() << "\n  After:  " << tempExpr.toString();
        this->replaceBy(tempExpr);
    }
}


//! @brief Splits the expression at first specified separator in specified list of symbols
//! @details Left and right side sybmols will be used to create left and right side child expressions.
//! @param sep Separator to split at
//! @param subSymbols List of symbols to split
//! @param simplifications Specifies the degree of simplification
//! @param parentSeparator Used when recursively creating the tree, this shall never be used when defining a new expression
bool Expression::splitAtFirstSeparator(const QString sep, const QStringList subSymbols, const ExpressionSimplificationT simplifications, const QString parentSeparator)
{
    if(subSymbols.contains(sep))
    {
        if(parentSeparator == "/" && sep == "*")
        {
            mString = "/";
        }
        else if(parentSeparator == "/" && sep == "/")
        {
            mString = "*";
        }
        else
        {
            mString = sep;
        }
        int split = subSymbols.indexOf(sep);
        QStringList right, left;
        for(int i=0; i<split; ++i)
        {
            left.append(subSymbols.at(i));
        }
        for(int i=split+1; i<subSymbols.size(); ++i)
        {
            right.append(subSymbols.at(i));
        }
        if(!left.isEmpty())
        {
            mChildren.append(Expression(left, simplifications));
        }
        mChildren.append(Expression(right, simplifications, sep));        //Right should always have a value
        return true;
    }
    return false;
}


//! @brief Returns a list with supported functions for equation-based model genereation
QStringList SymHop::getSupportedFunctionsList()
{
    return QStringList() << "div" << "rem" << "mod" << "tan" << "cos" << "sin" << "atan" << "acos" << "asin" << "atan2" << "sinh" << "cosh" << "tanh" << "log" << "exp" << "sqrt" << "sign" << "abs" << "der" << "onPositive" << "onNegative" << "signedSquareL" << "limit" << "integer" << "floor" << "ceil" << "pow";

}


//! @brief Returns a list of custom Hopsan functions that need to be allowed in the symbolic library
QStringList SymHop::getCustomFunctionList()
{
    return QStringList() << "hopsanLimit" << "hopsanDxLimit" << "onPositive" << "onNegative" << "signedSquareL" << "limit";
}


//! @brief Finds the first path through a matrix of dependencies, used to sort jacobian matrixes
bool SymHop::findPath(QList<int> &order, QList<QList<int> > dependencies, int level)
{
    if(level > dependencies.size()-1)
    {
        return true;
    }

    for(int i=0; i<dependencies.at(level).size(); ++i)
    {
        if(!order.contains(dependencies.at(level).at(i)))
        {
            order.append(dependencies.at(level).at(i));
            if(findPath(order, dependencies, level+1))
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
bool SymHop::sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations)
{
    qDebug() << "Jacobian:";
    for(int i=0; i<jacobian.size(); ++i)
    {
        QString line;
        for(int j=0; j<jacobian.size(); ++j)
        {
            line.append(jacobian[i][j].toString());
            line.append("  ");
        }
        qDebug() << line;
    }

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
    if(!findPath(order, dependencies))
    {
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
void SymHop::removeDuplicates(QList<Expression> &rList)
{
    QStringList temp;
    for(int i=0; i<rList.size(); ++i)
    {
        temp.append(rList[i].toString());
    }
    temp.removeDuplicates();
    rList.clear();
    for(int i=0; i<temp.size(); ++i)
    {
        rList.append(Expression(temp[i]));
    }
}


//! @brief Checks whether or no the parentheses are correct in a string
//! @param str String to verify
bool Expression::verifyParantheses(const QString str) const
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

