/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   SymHop.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-07-19
//!
//! @brief Contains the SymHop library for symbolic expressions
//!
//$Id$

#ifndef SYMHOP_H
#define SYMHOP_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>
#include "win32dll.h"


//! @class SymHopFunctionoid
//! @brief Pure virtual class used for calling member functions from objects of classes unknown to SymHop
//! @author Robert Braun <robert.braun@liu.se>
//!
//! Function operator can be overloaded, and member variables can be added if necessary.
//!
class DLLIMPORTEXPORT SymHopFunctionoid
{
public:
    virtual double operator()(QString&str, bool &ok) = 0;
    virtual ~SymHopFunctionoid() = 0;
};

inline SymHopFunctionoid::~SymHopFunctionoid() { }

namespace SymHop {

typedef double (*FunctionPtr)(QString, bool&); // function pointer type

class DLLIMPORTEXPORT Expression
{
public:
    enum ExpressionTypeT {Null, Equality,Symbol,Operator,Function};
    enum ExpressionSimplificationT {FullSimplification, TrivialSimplifications, NoSimplifications};
    enum ExpressionRecursiveT {Recursive, NonRecursive};
    Expression(Expression &other);
    Expression(const Expression &other);
    Expression(QString const indata=QString(), bool *ok=0, const ExpressionSimplificationT simplifications=FullSimplification);
    Expression(QStringList symbols, const ExpressionSimplificationT simplifications=FullSimplification);
    //Expression(const Expression left, const QString mid, const Expression right, const ExpressionSimplificationT simplifications=FullSimplification);
    //Expression(const QList<Expression> children, const QString separator);
    Expression(const double value);
    ~Expression();
    void cleanUp();

    void commonConstructorCode(QStringList symbols, bool &ok, const ExpressionSimplificationT simplifications=FullSimplification);

    bool operator==(const Expression &other) const;
    void operator=(const Expression &other);
    void operator=(Expression &other);

    static Expression fromTwoTerms(const Expression term1, const Expression term2);
    static Expression fromTerms(const QList<Expression> terms);
    static Expression fromTwoFactors(const Expression factor1, const Expression factor2);
    static Expression fromFactorDivisor(const Expression factor, const Expression divisor);
    static Expression fromFactorsDivisors(const QList<Expression> factors, const QList<Expression> divisors);
    static Expression fromBasePower(const Expression base, const Expression power);
    static Expression fromFunctionArguments(const QString function, const QList<Expression> arguments);
    static Expression fromEquation(const Expression left, const Expression right);

    double evaluate(const QMap<QString, double> &variables, const QMap<QString, SymHopFunctionoid*> *functions=0 /* = QMap<QString, SymHop::Function>() */, bool *ok=0) const;
    // magse says: QMap is a derived type, you can't have reference this way.

    void replaceByCopy(const Expression expr);
    void replaceBy(const Expression &expr);
    void divideBy(Expression const div);
    void multiplyBy(Expression const fac);
    void addBy(Expression const term);
    void subtractBy(Expression const term);
    QString toString() const;
    void toDelayForm(QList<Expression> &rDelayTerms, QStringList &rDelaySteps);
    double toDouble(bool *ok=0) const;
    bool isPower() const;
    bool isMultiplyOrDivide() const;
    bool isAdd() const;
    bool isFunction() const;
    bool isSymbol() const;
    bool isNumericalSymbol() const;
    bool isVariable() const;
    bool isAssignment() const;
    bool isEquation() const;
    bool isNegative() const;

    void changeSign();

    Expression derivative(const Expression x, bool &ok) const;
    bool contains(const Expression expr) const;
    Expression bilinearTransform() const;

    QStringList getFunctions() const;
    QString getFunctionName() const;
    QString getSymbolName() const;

    QList<Expression> getVariables() const;

    QList<Expression> getArguments() const;
    QList<Expression> getTerms() const;
    QList<Expression> getFactors() const;
    QList<Expression> getDivisors() const;
    Expression *getBase() const;
    Expression *getPower() const;
    Expression *getLeft() const;
    Expression *getRight() const;
    Expression *getDividends() const;

    Expression getArgument(const int idx) const;

    void removeDivisors();
    void removeFactor(const Expression var);
    void replace(const Expression oldsymbol, const Expression newExpr);

    void expand(const ExpressionSimplificationT simplifications=FullSimplification);
    void linearize();

    void toLeftSided();
    void factor(const Expression var);
    void factorMostCommonFactor();

    bool verifyExpression();

    //Public functions that are not intended to be used externally
    bool _verifyFunctions() const;
    void _simplify(ExpressionSimplificationT type = Expression::FullSimplification, const ExpressionRecursiveT recursive=NonRecursive);

    double countTerm(const Expression &expr) const;
    void removeTerm(const Expression &term);
    Expression removeNumericalFactors() const;
    double getNumericalFactor() const;

    static QStringList splitWithRespectToParentheses(const QString str, const QChar c);
    static bool verifyParantheses(const QString str);

    //! @todo Must be public for the object-less constructor functions, solve later (AND DON'T USE THEM ANYWHERE ELSE!!!)
    //ExpressionTypeT mType;
    QString mString;                //Used for symbols
    QString mFunction;              //Used for functions
    QList<Expression> mArguments;   //Used for functions
    QList<Expression> mTerms;       //Used in terms
    QList<Expression> mFactors;     //Used in factor-divisor
    QList<Expression> mDivisors;    //Used in factor-divisor or modulo
    Expression *mpBase;        //Used in power
    Expression *mpPower;       //Used in power
    Expression *mpLeft;        //Used in equality
    Expression *mpRight;       //Used in equality
    Expression *mpDividend;   //Used in modulo

private:
    bool splitAtSeparator(const QString sep, const QStringList subSymbols, const ExpressionSimplificationT simplifications);
    QStringList reservedSymbols;
};

QString DLLIMPORTEXPORT getFunctionDerivative(const QString &key);
QStringList DLLIMPORTEXPORT getSupportedFunctionsList();
QStringList DLLIMPORTEXPORT getCustomFunctionList();

bool DLLIMPORTEXPORT findPath(QList<int> &order, QList<QList<int> > dependencies, int level=0, QList<int> preferredPath=QList<int>());
bool DLLIMPORTEXPORT sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations, QList<int> preferredOrder);
void DLLIMPORTEXPORT removeDuplicates(QList<Expression> &rSet);

bool DLLIMPORTEXPORT isWhole(const double value);
}

//bool fuzzyEqual(const double &x, const double &y);
//void hAssert(const bool cond);

#endif // SYMHOP_H
