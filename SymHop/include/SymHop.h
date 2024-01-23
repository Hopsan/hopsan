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

#ifndef SYMHOP_H
#define SYMHOP_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>
#include "symhop_win32dll.h"


//! @class SymHopFunctionoid
//! @brief Pure virtual class used for calling member functions from objects of classes unknown to SymHop
//! @author Robert Braun <robert.braun@liu.se>
//!
//! Function operator can be overloaded, and member variables can be added if necessary.
//!
class SYMHOP_DLLAPI SymHopFunctionoid
{
public:
    virtual double operator()(QString&str, bool &ok) = 0;
    virtual ~SymHopFunctionoid() = 0;
};

inline SymHopFunctionoid::~SymHopFunctionoid() { }

namespace SymHop {

typedef double (*FunctionPtr)(QString, bool&); // function pointer type
static QStringList gSymHopMessages;

class SYMHOP_DLLAPI Expression
{
public:
    enum ExpressionTypeT {Null, Equality,Symbol,Operator,Function};
    enum ExpressionSimplificationT {FullSimplification, TrivialSimplifications, NoSimplifications};
    enum ExpressionRecursiveT {Recursive, NonRecursive};
    enum InlineTransformT {UndefinedTransform, Trapezoid, ExplicitEuler, ImplicitEuler, BDF1, BDF2, BDF3, BDF4, BDF5, AdamsMoulton1, AdamsMoulton2, AdamsMoulton3, AdamsMoulton4};
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
    bool operator!=(const Expression &other) const;
    void operator=(const Expression &other);
    void operator=(Expression &other);

    static Expression fromTwoTerms(const Expression term1, const Expression term2);
    static Expression fromTerms(const QList<Expression> terms);
    static Expression fromTwoFactors(const Expression factor1, const Expression factor2);
    static Expression fromFactorDivisor(const Expression factor, const Expression divisor);
    static Expression fromFactorsDivisors(const QList<Expression> factors, const QList<Expression> divisors);
    static Expression fromBasePower(const Expression base, const Expression power);
    static Expression fromFunctionArgument(const QString function, Expression argument);
    static Expression fromFunctionArguments(const QString function, const QList<Expression> arguments);
    static Expression fromEquation(const Expression left, const Expression right);

    double  evaluate(const QMap<QString, double> &variables, const QMap<QString, SymHopFunctionoid*> *functions=0 /* = QMap<QString, SymHop::Function>() */, bool *ok=0) const;
    // magse says: QMap is a derived type, you can't have reference this way.

    void replaceByCopy(const Expression expr);
    void replaceBy(const Expression &expr);
    void divideBy(Expression const div);
    void multiplyBy(Expression const fac);
    void addBy(Expression const term);
    void subtractBy(Expression const term);
    QString toString() const;
    QString toLaTeX() const;
    void toDelayForm(QList<Expression> &rDelayTerms, QStringList &rDelaySteps);
    double toDouble(bool *ok=0) const;
    bool isPower() const;
    bool isMultiplyOrDivide() const;
    bool isAdd() const;
    bool isFunction() const;
    bool isSymbol() const;
    bool isNumericalSymbol() const;
    bool isInteger() const;
    bool isVariable() const;
    bool isAssignment() const;
    bool isEquation() const;
    bool isNegative() const;

    void changeSign();

    Expression derivative(const Expression x, bool &ok) const;
    bool contains(const Expression expr) const;
    Expression bilinearTransform() const;
    Expression inlineTransform(const InlineTransformT transform, bool &ok) const;

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
    void expandPowers();
    void linearize();

    void toLeftSided();
    void factor(const Expression var);
    void factorMostCommonFactor();

    bool verifyExpression(const QStringList &userFunctions = QStringList()) const;

    //Public functions that are not intended to be used externally
    bool _verifyFunctions(const QStringList &userFunctions) const;
    void _simplify(ExpressionSimplificationT type = Expression::FullSimplification, const ExpressionRecursiveT recursive=NonRecursive);

    double countTerm(const Expression &expr) const;
    void removeTerm(const Expression &term);
    Expression removeNumericalFactors() const;
    double getNumericalFactor() const;

    static QStringList splitWithRespectToParentheses(const QString str, const QChar c);
    static bool verifyParantheses(const QString str);

    Expression *findFunction(const QString funcName);

    QStringList readErrorMessages();

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

QString SYMHOP_DLLAPI getFunctionDerivative(const QString &key);
QStringList SYMHOP_DLLAPI getSupportedFunctionsList();
QStringList SYMHOP_DLLAPI getCustomFunctionList();

bool SYMHOP_DLLAPI findPath(QList<int> &order, QList<QList<int> > dependencies, int level=0, QList<int> preferredPath=QList<int>());
bool SYMHOP_DLLAPI sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations, QList<int> preferredOrder);
void SYMHOP_DLLAPI removeDuplicates(QList<Expression> &rSet);

bool SYMHOP_DLLAPI isWhole(const double value);

SymHop::Expression::InlineTransformT SYMHOP_DLLAPI strToTransform(const QString &str);
}

//bool fuzzyEqual(const double &x, const double &y);
//void hAssert(const bool cond);

#endif // SYMHOP_H
