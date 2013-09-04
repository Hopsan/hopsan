#include <QtTest>
#include "SymHop.h"

using namespace SymHop;

Q_DECLARE_METATYPE(Expression*)
Q_DECLARE_METATYPE(Expression)
Q_DECLARE_METATYPE(int)
Q_DECLARE_METATYPE(QString)

class SymHopTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void SymHop_Not_Zero()
    {
        QFETCH(Expression*, expr1);
        QString failmsg("Failure! Expression equals zero.");
        QVERIFY2(expr1 != 0, failmsg.toStdString().c_str());
    }
    void SymHop_Not_Zero_data()
    {
        QTest::addColumn<Expression*>("expr1");
        QTest::newRow("0") << Expression("x=y").mpLeft;
        QTest::newRow("1") << Expression("x=y").mpRight;
        QTest::newRow("2") << Expression("x^y").mpBase;
        QTest::newRow("3") << Expression("x^y").mpPower;
        QTest::newRow("4") << Expression::fromBasePower(Expression("x"), Expression("y")).mpBase;
        QTest::newRow("5") << Expression::fromBasePower(Expression("x"), Expression("y")).mpPower;
        QTest::newRow("6") << Expression::fromEquation(Expression("x"), Expression("y")).mpLeft;
        QTest::newRow("7") << Expression::fromEquation(Expression("x"), Expression("y")).mpRight;
    }

    void SymHop_Num_Factors_Divisors_Terms()
    {
        QFETCH(Expression, expr);
        QFETCH(int, factors);
        QFETCH(int, divisors);
        QFETCH(int, terms);
        QString failmsg1("Failure! Wrong number of factors: "+expr.toString()+".mFactors.size() != "+QString::number(factors));
        QVERIFY2(expr.mFactors.size() == factors, failmsg1.toStdString().c_str());
        QString failmsg2("Failure! Wrong number of divisors: "+expr.toString()+".mDivisors.size() != "+QString::number(divisors));
        QVERIFY2(expr.mDivisors.size() == divisors, failmsg2.toStdString().c_str());
        QString failmsg3("Failure! Wrong number of terms: "+expr.toString()+".mTerms.size() != "+QString::number(terms));
        QVERIFY2(expr.mTerms.size() == terms, failmsg3.toStdString().c_str());
    }

    void SymHop_Num_Factors_Divisors_Terms_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<int>("factors");
        QTest::addColumn<int>("divisors");
        QTest::addColumn<int>("terms");
        QTest::newRow("0") << Expression("x*y/z") << 2 << 1 << 0;
        QTest::newRow("1") << Expression("x+y") << 0 << 0 << 2;
        QTest::newRow("2") << Expression(QStringList() << "x" << "*" << "y" << "/" << "z") << 2 << 1 << 0;
        QTest::newRow("3") << Expression::fromFactorDivisor(Expression("x"), Expression("y")) << 1 << 1 << 0;
        QTest::newRow("4") << Expression::fromFactorsDivisors(QList<Expression>() << Expression("x") << Expression("y"),
                                                              QList<Expression>() << Expression("z") << Expression("3")) << 2 << 2 << 0;
        QTest::newRow("5") << Expression::fromTerms(QList<Expression>() << Expression("x") << Expression("y")) << 0 << 0 << 2;
        QTest::newRow("6") << Expression::fromTwoFactors(Expression("x"), Expression("y")) << 2 << 0 << 0;
        QTest::newRow("7") << Expression::fromTwoTerms(Expression("x"), Expression("y")) << 0 << 0 << 2;
    }

    void SymHop_Func_And_Num_Arguments()
    {
        QFETCH(Expression, expr);
        QFETCH(QString, func);
        QFETCH(int, args);
        QString failmsg1("Failure! Wrong function name: "+expr.toString()+".mFunction != "+func.toStdString().c_str());
        QString failmsg2("Failure! Wrong number of arguments: "+expr.toString()+".mArguments.size() != "+QString::number(args));
        QVERIFY2(expr.mFunction == func, failmsg1.toStdString().c_str());
        QVERIFY2(expr.mArguments.size() == args, failmsg2.toStdString().c_str());
    }
    void SymHop_Func_And_Num_Arguments_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QString>("func");
        QTest::addColumn<int>("args");
        QTest::newRow("0") << Expression("sin(x)") << "sin" << 1;
        QTest::newRow("1") << Expression::fromFunctionArguments("foo", QList<Expression>() << Expression("x") << Expression("y")) << "foo" << 2;
    }

    void SymHop_To_Double()
    {
        QFETCH(Expression, expr);
        QFETCH(double, value);
        QString failmsg("Failure! Wrong value: "+expr.toString()+".toDouble() != "+QString::number(value));
        QVERIFY2(expr.toDouble() == value, failmsg.toStdString().c_str());
    }
    void SymHop_To_Double_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<double>("value");
        QTest::newRow("0") << Expression("5") << 5.0;
        QTest::newRow("1") << Expression(5) << 5.0;
        QTest::newRow("2") << Expression("2+3") << 5.0;
        QTest::newRow("3") << Expression("x") << 0.0;
    }

    void SymHop_Equality()
    {
        QFETCH(Expression, expr1);
        QFETCH(Expression, expr2);
        QString failmsg("Failure! "+expr1.toString()+" != "+expr2.toString());
        QVERIFY2(expr1 == expr2, failmsg.toStdString().c_str());
    }
    void SymHop_Equality_data()
    {
        QTest::addColumn<Expression>("expr1");
        QTest::addColumn<Expression>("expr2");
        QTest::newRow("0") << Expression("sin(2^z)+2*x/(5+y)") << Expression("1/(y+5)*x*2+sin(2.0^z)");

        //Validate replaceBy()
        QString exprStr = "x*5.0 + (y+5)/(3^z) = tan(sin(x))";
        Expression temp = Expression(exprStr);
        Expression expr = Expression::fromTwoTerms(Expression("x"), Expression("y"));
        expr.replaceBy(temp);
        QTest::newRow("1") << expr << temp;

        //Validate divideBy()
        temp = Expression("x/y+3");
        temp.divideBy(Expression("z"));
        QTest::newRow("2") << temp << Expression("(x/y+3)/z");

        //Validate multiplyBy()
        temp = Expression("y/z");
        temp.multiplyBy(Expression("x"));
        QTest::newRow("3") << temp << Expression("x*y/z");

        //Validate addBy()
        temp = Expression("x");
        temp.addBy(Expression("y"));
        QTest::newRow("4") << temp << Expression("x+y");

        //Validate subtractBy()
        temp = Expression("x");
        temp.subtractBy(Expression("y"));
        QTest::newRow("5") << temp << Expression("x-y");

        //Validate changeSign()
        temp = Expression("x*y");
        temp.changeSign();
        QTest::newRow("5") << temp << Expression("-x*y");
    }

    void SymHop_Is_Functions()
    {
        QFETCH(Expression, expr);
        QFETCH(bool, isPower);
        QFETCH(bool, isMulDiv);
        QFETCH(bool, isAdd);
        QFETCH(bool, isFunc);
        QFETCH(bool, isSymbol);
        QFETCH(bool, isNumSymbol);
        QFETCH(bool, isVariable);
        QFETCH(bool, isAssignment);
        QFETCH(bool, isEquation);
        QFETCH(bool, isNegative);

        QString failmsg1("Failure! "+expr.toString()+".isPower() != "+QString::number(isPower));
        QString failmsg2("Failure! "+expr.toString()+".isMultiplyOrDivide() != "+QString::number(isMulDiv));
        QString failmsg3("Failure! "+expr.toString()+".isAdd() != "+QString::number(isAdd));
        QString failmsg4("Failure! "+expr.toString()+".isFunction() != "+QString::number(isFunc));
        QString failmsg5("Failure! "+expr.toString()+".isSymbol() != "+QString::number(isSymbol));
        QString failmsg6("Failure! "+expr.toString()+".isNumericalSymbol() != "+QString::number(isNumSymbol));
        QString failmsg7("Failure! "+expr.toString()+".isVariable() != "+QString::number(isVariable));
        QString failmsg8("Failure! "+expr.toString()+".isAssignment() != "+QString::number(isAssignment));
        QString failmsg9("Failure! "+expr.toString()+".isEquation() != "+QString::number(isEquation));
        QString failmsg10("Failure! "+expr.toString()+".isNegative() != "+QString::number(isNegative));
        QVERIFY2(expr.isPower() == isPower,                 failmsg1.toStdString().c_str());
        QVERIFY2(expr.isMultiplyOrDivide() == isMulDiv,     failmsg2.toStdString().c_str());
        QVERIFY2(expr.isAdd() == isAdd,                     failmsg3.toStdString().c_str());
        QVERIFY2(expr.isFunction() == isFunc,               failmsg4.toStdString().c_str());
        QVERIFY2(expr.isSymbol() == isSymbol,               failmsg5.toStdString().c_str());
        QVERIFY2(expr.isNumericalSymbol() == isNumSymbol,   failmsg6.toStdString().c_str());
        QVERIFY2(expr.isVariable() == isVariable,           failmsg7.toStdString().c_str());
        QVERIFY2(expr.isAssignment() == isAssignment,       failmsg8.toStdString().c_str());
        QVERIFY2(expr.isEquation() == isEquation,           failmsg9.toStdString().c_str());
        QVERIFY2(expr.isNegative() == isNegative,           failmsg10.toStdString().c_str());
    }
    void SymHop_Is_Functions_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<bool>("isPower");
        QTest::addColumn<bool>("isMulDiv");
        QTest::addColumn<bool>("isAdd");
        QTest::addColumn<bool>("isFunc");
        QTest::addColumn<bool>("isSymbol");
        QTest::addColumn<bool>("isNumSymbol");
        QTest::addColumn<bool>("isVariable");
        QTest::addColumn<bool>("isAssignment");
        QTest::addColumn<bool>("isEquation");
        QTest::addColumn<bool>("isNegative");

        QTest::newRow("0") << Expression("x^y")         << true  << false << false << false << false << false << false << false << false << false;
        QTest::newRow("1") << Expression("x+y")         << false << false << true  << false << false << false << false << false << false << false;
        QTest::newRow("2") << Expression("x*y")         << false << true  << false << false << false << false << false << false << false << false;
        QTest::newRow("3") << Expression("x/y")         << false << true  << false << false << false << false << false << false << false << false;
        QTest::newRow("4") << Expression("x+y/z")       << false << false << true  << false << false << false << false << false << false << false;
        QTest::newRow("5") << Expression("sin(x*y)")    << false << false << false << true  << false << false << false << false << false << false;
        QTest::newRow("6") << Expression("x*sin(y)")    << false << true  << false << false << false << false << false << false << false << false;
        QTest::newRow("7") << Expression("x")           << false << false << false << false << true  << false << true  << false << false << false;
        QTest::newRow("8") << Expression("5.3")         << false << false << false << false << true  << true  << false << false << false << false;
        QTest::newRow("9") << Expression("x=y+z")       << false << false << false << false << false << false << false << true  << true  << false;
        QTest::newRow("10") << Expression("x+z=y")      << false << false << false << false << false << false << false << false << true  << false;
        QTest::newRow("11") << Expression("-x*y")       << false << true  << false << false << false << false << false << false << false << true;
        QTest::newRow("12") << Expression("-sin(x)")    << false << true  << false << false << false << false << false << false << false << true;
    }

    void SymHop_Contains()
    {
        QFETCH(Expression, expr1);
        QFETCH(Expression, expr2);
        QString failmsg("Failure! Wrong value: "+expr1.toString()+".contains("+expr2.toString()+") = false");
        QVERIFY2(expr1.contains(expr2), failmsg.toStdString().c_str());
    }
    void SymHop_Contains_data()
    {
        QTest::addColumn<Expression>("expr1");
        QTest::addColumn<Expression>("expr2");
        QTest::newRow("0") << Expression("(x+y)/(x+y^z)") << Expression("z");
        QTest::newRow("1") << Expression("sin(cos(x))") << Expression("x");
    }

    void SymHop_Derivative()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, der);
        bool ok;
        Expression temp = expr.derivative(Expression("x"), ok);
        QString failmsg("Failure! Expression: "+der.toString()+" != "+temp.toString());
        QVERIFY2(temp == der, failmsg.toStdString().c_str());
    }

    void SymHop_Derivative_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("der");
        QTest::newRow("0") << Expression(5) << Expression(0);
        QTest::newRow("1") << Expression("3*x") << Expression(3);
        QTest::newRow("2") << Expression("3*x*x") << Expression("6*x");
        QTest::newRow("3") << Expression("4*sin(2*x)") << Expression("8*cos(2*x)");
    }
};

QTEST_APPLESS_MAIN(SymHopTests)

#include "tst_symhoptest.moc"
