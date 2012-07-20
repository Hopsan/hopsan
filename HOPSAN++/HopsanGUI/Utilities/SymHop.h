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

#ifndef SYMHOP_H
#define SYMHOP_H

#include <QtGui>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>

using namespace std;

namespace SymHop {

class Expression
{
public:
    enum ExpressionTypeT {Equality,Symbol,Operator,Function};
    enum ExpressionSimplificationT {FullSimplification, SimplifyWithoutMakingPowers, TrivialSimplifications, NoSimplifications};
    enum ExpressionRecursiveT {Recursive, NonRecursive};
    Expression(QString const indata=QString(), const ExpressionSimplificationT simplifications=FullSimplification);
    Expression(QStringList symbols, const ExpressionSimplificationT simplifications=FullSimplification, const QString parentSeparator=QString());
    Expression(const Expression left, const QString mid, const Expression right, const ExpressionSimplificationT simplifications=FullSimplification);
    Expression(const QList<Expression> children, const QString separator, const ExpressionSimplificationT simplifications=FullSimplification);
    Expression(const double value);

    void commonConstructorCode(QStringList symbols, const ExpressionSimplificationT simplifications=FullSimplification, const QString parentSeparator=QString());

    bool operator==(const Expression &other) const;

    Expression negative() const;

    int count(const Expression &var) const;

    void replaceBy(Expression const expr);
    void divideBy(Expression const div);
    ExpressionTypeT getType() const;
    QString toString() const;
    void toDelayForm(QList<Expression> &rDelayTerms, QStringList &rDelaySteps);
    double toDouble(bool *ok=0) const;
    bool isPower() const;
    bool isMultiplyOrDivide() const;
    bool isAdd() const;
    bool isNumericalSymbol() const;
    bool isAssignment() const;
    bool isEquation() const;
    bool isNegative() const;

    Expression derivative(const Expression x, bool &ok) const;
    bool contains(const Expression expr) const;
    Expression bilinearTransform() const;

    QStringList getFunctions() const;
    QString getFunctionName() const;
    QString getSymbolName() const;

    QList<Expression> getSymbols() const;
    QList<Expression> getArguments() const;
    QList<Expression> getTerms() const;
    QList<Expression> getFactors() const;
    QList<Expression> getDivisors() const;

    Expression getArgument(const int idx) const;
    Expression getChild(const int idx) const;

    void removeDivisors();
    void removeFactor(const Expression var);
    void replace(const Expression oldsymbol, const Expression newExpr);

    void expand();
    void expandPowers();
    void expandParentheses(const ExpressionSimplificationT simplifications=FullSimplification);
    void linearize();

    void toLeftSided();
    void factor(const Expression var);
    void factorMostCommonFactor();

    bool verifyExpression();

    //Public functions that are not intended to be used externally
    QList<Expression> _getChildren() const;
    QList<Expression> *_getChildrenPtr();
    QString _getString() const;
    bool _verifyFunctions() const;
    void _simplify(ExpressionSimplificationT type = Expression::FullSimplification, const ExpressionRecursiveT recursive=NonRecursive);

private:
    ExpressionTypeT mType;
    QString mString;
    QList<Expression> mChildren;
    QMap<QString, QString> mFunctionDerivatives;
    QStringList reservedSymbols;

    bool splitAtFirstSeparator(const QString sep, const QStringList subSymbols, const ExpressionSimplificationT simplifications, const QString parentSeparator=QString());
    bool verifyParantheses(const QString str) const;
    QStringList splitWithRespectToParentheses(const QString str, const QChar c);
};


QStringList getSupportedFunctionsList();
QStringList getCustomFunctionList();

bool findPath(QList<int> &order, QList<QList<int> > dependencies, int level=0);
bool sortEquationSystem(QList<Expression> &equations, QList<QList<Expression> > &jacobian, QList<Expression> stateVars, QList<int> &limitedVariableEquations, QList<int> &limitedDerivativeEquations);
void removeDuplicates(QList<Expression> &rList);

}

#endif // SYMHOP_H
