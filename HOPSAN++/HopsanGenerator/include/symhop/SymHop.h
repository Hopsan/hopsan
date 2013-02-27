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
//$Id: SymHop.h 4881 2013-01-11 13:29:41Z robbr48 $

#ifndef SYMHOP_H
#define SYMHOP_H

//#include <QtGui>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>
#include "win32dll.h"

using namespace std;

namespace SymHop {

class DLLIMPORTEXPORT Expression
{
public:
    enum ExpressionTypeT {Null, Equality,Symbol,Operator,Function};
    enum ExpressionSimplificationT {FullSimplification, TrivialSimplifications, NoSimplifications};
    enum ExpressionRecursiveT {Recursive, NonRecursive};
    Expression(QString const indata=QString(), const ExpressionSimplificationT simplifications=FullSimplification);
    Expression(QStringList symbols, const ExpressionSimplificationT simplifications=FullSimplification, const QString parentSeparator=QString());
    //Expression(const Expression left, const QString mid, const Expression right, const ExpressionSimplificationT simplifications=FullSimplification);
    //Expression(const QList<Expression> children, const QString separator);
    Expression(const double value);

    void commonConstructorCode(QStringList symbols, const ExpressionSimplificationT simplifications=FullSimplification, const QString parentSeparator=QString());

    bool operator==(const Expression &other) const;
    void operator=( const Expression &other);

    static Expression fromTwoTerms(const Expression term1, const Expression term2);
    static Expression fromTerms(const QList<Expression> terms);
    static Expression fromTwoFactors(const Expression factor1, const Expression factor2);
    static Expression fromFactorDivisor(const Expression factor, const Expression divisor);
    static Expression fromFactorsDivisors(const QList<Expression> factors, const QList<Expression> divisors);
    static Expression fromBasePower(const Expression base, const Expression power);
    static Expression fromFunctionArguments(const QString function, const QList<Expression> arguments);
    static Expression fromEquation(const Expression left, const Expression right);

    double evaluate(const QMap<QString, double> variables) const;

    void replaceBy(Expression const expr);
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

    int countTerm(const Expression &expr) const;
    void removeTerm(const Expression &term);
    Expression removeNumericalFactors() const;
    double getNumericalFactor() const;

    //! @todo Must be public for the object-less constructor functions, solve later (AND DON'T USE THEM ANYWHERE ELSE!!!)
    //ExpressionTypeT mType;
    QString mString;                //Used for symbols
    QString mFunction;              //Used for functions
    QList<Expression> mArguments;   //Used for functions
    QList<Expression> mTerms;       //Used in terms
    QList<Expression> mFactors;     //Used in factor-divisor
    QList<Expression> mDivisors;    //Used in factor-divosor or modulo
    Expression *mpBase;        //Used in power
    Expression *mpPower;       //Used in power
    Expression *mpLeft;        //Used in equality
    Expression *mpRight;       //Used in equality
    Expression *mpDividend;   //Used in modulo

private:
    QStringList reservedSymbols;

    bool splitAtSeparator(const QString sep, const QStringList subSymbols, const ExpressionSimplificationT simplifications);
    bool verifyParantheses(const QString str) const;
    QStringList splitWithRespectToParentheses(const QString str, const QChar c);
};

QString getFunctionDerivative(const QString &key);
QStringList getSupportedFunctionsList();
QStringList getCustomFunctionList();

bool findPath(QList<int> &order, QList<QList<int> > dependencies, int level=0);
bool sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations);
void removeDuplicates(QList<Expression> &rSet);

bool isWhole(const double value);

void validateFunctions();
}

bool fuzzyEqual(const double &x, const double &y);

#endif // SYMHOP_H
