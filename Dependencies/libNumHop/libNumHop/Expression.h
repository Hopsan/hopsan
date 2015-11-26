#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <list>
#include "VariableStorage.h"

namespace numhop {

enum ExpressionOperatorT {AssignmentT, AdditionT, SubtractionT, MultiplicationT, DivisionT, PowerT, ValueT, UndefinedT};

class Expression
{
public:
    Expression();
    Expression(const Expression &other);
    Expression(const std::string &exprString, ExpressionOperatorT op);
    Expression(const std::string &leftExprString, const std::string &rightExprString, ExpressionOperatorT op);

    Expression& operator= (const Expression &other);

    bool empty() const;
    bool hasValue() const;
    bool hasConstantValue() const;

    const std::string &exprString() const;
    const std::string &leftExprString() const;
    const std::string &rightExprString() const;
    ExpressionOperatorT operatorType() const;

    double evaluate(VariableStorage &rVariableStorage, bool &rEvalOK);

    std::string print();

protected:
    void commonConstructorCode();
    void copyFromOther(const Expression &other);

    std::string mLeftExpressionString, mRightExpressionString;
    bool mHadLeftOuterParanthesis, mHadRightOuterParanthesis;
    std::list<Expression> mLeftChildExpressions, mRightChildExpressions;
    bool mHasValue, mHasConstantValue;
    double mConstantValue;
    ExpressionOperatorT mOperator;
};

bool interpretExpressionStringRecursive(std::string exprString, std::list<Expression> &rExprList);
bool interpretExpressionStringRecursive(std::string exprString, Expression &rExpr);

}

#endif // EXPRESSION_H
