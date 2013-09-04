#include <QtTest>
#include "SymHop.h"

using namespace SymHop;

Q_DECLARE_METATYPE(Expression*)
Q_DECLARE_METATYPE(Expression)
Q_DECLARE_METATYPE(int)
Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE(QStringList)
Q_DECLARE_METATYPE(QList<Expression>)
typedef QMap<QString, double> stringDoubleMap;
Q_DECLARE_METATYPE(stringDoubleMap)

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

    void SymHop_Get_Functions()
    {
        QFETCH(Expression, expr);
        QFETCH(QStringList, funcs);

        QString failmsg("Failure! getFunctions() returned incorrect values.");
        QStringList testFuncs = expr.getFunctions();
        qSort(funcs);
        qSort(testFuncs);
        QVERIFY2(funcs == testFuncs, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Functions_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QStringList>("funcs");
        QStringList functions = QStringList() << "foo" << "bar" << "sin" << "cos";
        QTest::newRow("0") << Expression("foo(x)+1.0/bar(x)+sin(x^cos(y))") << functions;
    }

    void SymHop_Get_Function_Name()
    {
        QFETCH(Expression, expr);
        QFETCH(QString, name);

        QString failmsg("Failure! getFunctionName() returned incorrect value.");
        QVERIFY2(expr.getFunctionName() == name, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Function_Name_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QString>("name");
        QTest::newRow("0") << Expression("foo(x)") << "foo";
    }

    void SymHop_Get_Symbol_Name()
    {
        QFETCH(Expression, expr);
        QFETCH(QString, name);

        QString failmsg("Failure! getSymbolName() returned incorrect value.");
        QVERIFY2(expr.getSymbolName() == name, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Symbol_Name_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QString>("name");
        QTest::newRow("0") << Expression("x") << "x";
        QTest::newRow("1") << Expression(3) << "3.0";
    }

    void SymHop_Get_Variables()
    {
        QFETCH(Expression, expr);
        QFETCH(QList<Expression>, vars);

        QString failmsg("Failure! getVariables() returned incorrect values.");
        QList<Expression> testVars = expr.getVariables();
        QVERIFY2(vars == testVars, failmsg.toStdString().c_str());      //! @todo This test requires same sorting, should not be required
    }

    void SymHop_Get_Variables_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QList<Expression> >("vars");
        QList<Expression> variables = QList<Expression>() << Expression("x") << Expression("y") << Expression("z") << Expression("A") << Expression("B");
        QTest::newRow("0") << Expression("x*5.0 + (y+5)/(3^z) = tan(sin(A/B))") << variables;
    }


    void SymHop_Get_Arguments()
    {
        QFETCH(Expression, expr);
        QFETCH(QList<Expression>, args);

        QString failmsg("Failure! getArguments() returned incorrect values.");
        QList<Expression> testArgs = expr.getArguments();
        QVERIFY2(args == testArgs, failmsg.toStdString().c_str());      //! @todo This test requires same sorting, should not be required
    }

    void SymHop_Get_Arguments_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QList<Expression> >("args");
        QList<Expression> arguments = QList<Expression>() << Expression("x") << Expression("y") << Expression("3.0");
        QTest::newRow("0") << Expression("foo(x,y,3)") << arguments;
    }

    void SymHop_Get_Terms()
    {
        QFETCH(Expression, expr);
        QFETCH(QList<Expression>, terms);

        QString failmsg("Failure! getTerms() returned incorrect values.");
        QList<Expression> testTerms = expr.getTerms();
        QVERIFY2(terms == testTerms, failmsg.toStdString().c_str());      //! @todo This test requires same sorting, should not be required
    }

    void SymHop_Get_Terms_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QList<Expression> >("terms");
        QList<Expression> terms1 = QList<Expression>() << Expression("x") << Expression("y/z") << Expression("-2^x");
        QTest::newRow("0") << Expression("x+y/z-2^x") << terms1;
    }

    void SymHop_Get_Factors()
    {
        QFETCH(Expression, expr);
        QFETCH(QList<Expression>, factors);

        QString failmsg("Failure! getFactors() returned incorrect values.");
        QList<Expression> testFactors = expr.getFactors();
        QVERIFY2(factors == testFactors, failmsg.toStdString().c_str());      //! @todo This test requires same sorting, should not be required
    }

    void SymHop_Get_Factors_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QList<Expression> >("factors");
        QList<Expression> factors1 = QList<Expression>() << Expression("x") << Expression("y");
        QTest::newRow("0") << Expression("x*y/A") << factors1;
    }

    void SymHop_Get_Divisors()
    {
        QFETCH(Expression, expr);
        QFETCH(QList<Expression>, divisors);

        QString failmsg("Failure! getDivisors() returned incorrect values.");
        QList<Expression> testDivisors = expr.getDivisors();
        QVERIFY2(divisors == testDivisors, failmsg.toStdString().c_str());      //! @todo This test requires same sorting, should not be required
    }

    void SymHop_Get_Divisors_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QList<Expression> >("divisors");
        QList<Expression> divisors1 = QList<Expression>() << Expression("z") << Expression("sin(A)");
        QTest::newRow("0") << Expression("(x+3)*y/z/sin(A)") << divisors1;
    }

    void SymHop_Get_Base()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, base);

        QString failmsg("Failure! getBase() returned incorrect value.");
        QVERIFY2(*expr.getBase() == base, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Base_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("base");
        QTest::newRow("0") << Expression("x^y") << Expression("x");
    }

    void SymHop_Get_Power()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, power);

        QString failmsg("Failure! getPower() returned incorrect value.");
        QVERIFY2(*expr.getPower() == power, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Power_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("power");
        QTest::newRow("0") << Expression("x^y") << Expression("y");
    }

    void SymHop_Get_Left()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, left);

        QString failmsg("Failure! getLeft() returned incorrect value.");
        QVERIFY2(*expr.getLeft() == left, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Left_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("left");
        QTest::newRow("0") << Expression("x+y=2+z") << Expression("x+y");
    }

    void SymHop_Get_Right()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, right);

        QString failmsg("Failure! getRight() returned incorrect value.");
        QVERIFY2(*expr.getRight() == right, failmsg.toStdString().c_str());
    }

    void SymHop_Get_Right_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("right");
        QTest::newRow("0") << Expression("x+y=2+z") << Expression("2+z");
    }

    //! @todo Validate getDividends (not implemented yet)

    void SymHop_Remove_Divisors()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! removeDivisors() returned incorrect value.");
        expr.removeDivisors();
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Remove_Divisors_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("a*b/c/(d+e)") << Expression("a*b");
    }

    void SymHop_Remove_Factor()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, factor);
        QFETCH(Expression, result);

        QString failmsg("Failure! removeFactor() returned incorrect value.");
        expr.removeFactor(factor);
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Remove_Factor_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("factor");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x*y/z") << Expression("x") << Expression("y/z");
    }

    void SymHop_Remove_Replace()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, oldExpr);
        QFETCH(Expression, newExpr);
        QFETCH(Expression, result);

        QString failmsg("Failure! replace() returned incorrect value.");
        expr.replace(oldExpr,newExpr);
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Remove_Replace_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("oldExpr");
        QTest::addColumn<Expression>("newExpr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x*y+2/x+z^x") << Expression("x") << Expression("A") << Expression("A*y+2/A+z^A");
    }

    void SymHop_Expand()
    {
        QFETCH(Expression, expr);
        QFETCH(QString, str);

        QString failmsg("Failure! expand() returned incorrect value.");
        expr.expand();
        QVERIFY2(expr.toString() == str, failmsg.toStdString().c_str());
    }

    void SymHop_Expand_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QString>("str");
        QTest::newRow("0") << Expression("x*(y+z*(A+B))") << "x*y+x*z*A+x*z*B";
    }

    void SymHop_Linearize()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! linearize() returned incorrect value.");
        expr.linearize();
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Linearize_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x/y+3/z=a/b+5") << Expression("x*z*b+3*y*b=a*y*z+5*y*z*b");
    }

    void SymHop_To_Left_Sided()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! toLeftSided() returned incorrect value.");
        expr.toLeftSided();
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_To_Left_Sided_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x=3") << Expression("x-3=0");
        QTest::newRow("1") << Expression("x/y+3/z=a/b+5") << Expression("x/y+3/z-a/b-5=0");
    }

    void SymHop_Factor()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, factor);
        QFETCH(Expression, result);

        QString failmsg("Failure! factor() returned incorrect value.");
        expr.factor(factor);
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Factor_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("factor");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x*y+3*x+z") << Expression("x") << Expression("x*(y+3)+z");
    }

    void SymHop_Factor_Most_Common_Factor()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! factorMostCommonFactor() returned incorrect value.");
        expr.factorMostCommonFactor();
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Factor_Most_Common_Factor_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x*y+x*z+3*x+A*y") << Expression("x*(y+z+3)+A*y");
    }

    void SymHop_Simplify()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! Simplification did something wrong.");
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Simplify_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("1*x*y/x+0") << Expression("1*x*y/x+0");
        QTest::newRow("1") << Expression("a*(b*(c*d))") << Expression("a*b*c*d");
        QTest::newRow("2") << Expression("-1*-1*-x") << Expression("-x");
        QTest::newRow("3") << Expression("a+(b+(c+d))") << Expression("a+b+c+d");
        QTest::newRow("4") << Expression("2*x+x+x+y") << Expression("4*x+y");
        QTest::newRow("5") << Expression("x^1+y^-2+2^3") << Expression("x+1/y^2+8");
        QTest::newRow("6") << Expression("2+3+x") << Expression("5+x");
        QTest::newRow("7") << Expression("(x+2)*(y+3)+2*4") << Expression("x*y+3*x+2*y+14");
        QTest::newRow("8") << Expression("x*x^4/x^2") << Expression("x^3");
        QTest::newRow("9") << Expression("0+0") << Expression("0");
        QTest::newRow("10") << Expression("1*1") << Expression("1");
    }

    void SymHop_Count_Term()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, term);
        QFETCH(int, count);

        QString failmsg("Failure! countTerm() did something wrong.");
        QVERIFY2(expr.countTerm(term) == count, failmsg.toStdString().c_str());
    }

    void SymHop_Count_Term_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("term");
        QTest::addColumn<int>("count");
        QTest::newRow("0") << Expression("4*sin(x)+3*cos(x)-2*sin(x)+sin(x)") << Expression("sin(x)") << 3;
    }

    void SymHop_Remove_Term()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, term);
        QFETCH(Expression, result);

        QString failmsg("Failure! removeTerm() did something wrong.");
        expr.removeTerm(term);
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Remove_Term_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("term");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("4*sin(x)+3*cos(x)-2*sin(x)+sin(x)") << Expression("5*sin(x)") << Expression("3*cos(x)");
    }

    void SymHop_Remove_Numerical_Factors()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! removeNumericalFactors() did something wrong.");
        QVERIFY2(expr.removeNumericalFactors() == result, failmsg.toStdString().c_str());
    }

    void SymHop_Remove_Numerical_Factors_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("2*x/(3*y)") << Expression("x/y");
    }

    void SymHop_Get_Numerical_Factor()
    {
        QFETCH(Expression, expr);
        QFETCH(double, factor);

        QString failmsg("Failure! getNumericalFactor() did something wrong.");
        QVERIFY2(fuzzyEqual(expr.getNumericalFactor(), factor), failmsg.toStdString().c_str());
    }

    void SymHop_Get_Numerical_Factor_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<double>("factor");
        QTest::newRow("0") << Expression("2*x/(3*y)") << 2.0/3.0;
    }


    void SymHop_To_Delay_Form()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! toDelayForm() did something wrong.");
        QList<Expression> delayTerms;
        QStringList delaySteps;
        expr.toDelayForm(delayTerms, delaySteps);
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_To_Delay_Form_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("x*Z+y*Z^2") << Expression("mDelay0.getIdx(1)+mDelay1.getIdx(1)");
    }

    void SymHop_Bilinear_Transform()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, result);

        QString failmsg("Failure! bilinearTransform() did something wrong.");
        expr = expr.bilinearTransform();
        QVERIFY2(expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Bilinear_Transform_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression("der(x)+3") << Expression("x*2/mTimestep*(1-Z)/(1+Z)+3");
    }

    void SymHop_To_String()
    {
        QFETCH(Expression, expr);
        QFETCH(QString, str);

        QString failmsg("Failure! toString() did something wrong.");
        QVERIFY2(expr.toString() == str, failmsg.toStdString().c_str());
    }

    void SymHop_To_String_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<QString>("str");
        QTest::newRow("0") << Expression("x-2*y+3-z") << "x-y*2.0-z+3.0";
        QTest::newRow("1") << Expression("x*5.0 + (y+5)/(3^z) = tan(sin(x))") << "x*5.0+(y+5.0)/pow(3.0,z)=tan(sin(x))";
    }

    void SymHop_Derivative()
    {
        QFETCH(Expression, expr);
        QFETCH(Expression, der);
        QFETCH(Expression, result);

        QString failmsg("Failure! derivative() did something wrong.");
        bool ok;
        expr = expr.derivative(der,ok);
        QVERIFY2(ok && expr == result, failmsg.toStdString().c_str());
    }

    void SymHop_Derivative_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<Expression>("der");
        QTest::addColumn<Expression>("result");
        QTest::newRow("0") << Expression(5) << Expression("x") << Expression(0);
        QTest::newRow("1") << Expression("3*x") << Expression("x") << Expression(3);
        QTest::newRow("2") << Expression("3*x*x") << Expression("x") << Expression("6*x");
        QTest::newRow("3") << Expression("4*sin(2*x)") << Expression("x") << Expression("8*cos(2*x)");
        QTest::newRow("4") << Expression("2*x^5+3*x") << Expression("x") << Expression("10.0*x^4.0+3.0");
        QTest::newRow("5") << Expression("cos(2*x^2)") << Expression("x") << Expression("-sin(2.0*pow(x,2.0))*x*4.0");
    }

    void SymHop_Evaluate()
    {
        QFETCH(Expression, expr);
        QFETCH(stringDoubleMap, vars);
        QFETCH(double, value);
        QString failmsg("Failure! evaluate() did something wrong.");
        QVERIFY2(fuzzyEqual(expr.evaluate(vars), value), failmsg.toStdString().c_str());
    }

    void SymHop_Evaluate_data()
    {
        QTest::addColumn<Expression>("expr");
        QTest::addColumn<stringDoubleMap>("vars");
        QTest::addColumn<double>("value");
        stringDoubleMap variables;
        double x = 5.1;
        double y = 32.12;
        double z = 12.13;
        variables.insert("x", x);
        variables.insert("y", y);
        variables.insert("z", z);
        QTest::newRow("0") << Expression("sin(x)") << variables << sin(x);
        QTest::newRow("1") << Expression("floor(x)") << variables << floor(x);
        QTest::newRow("2") << Expression("limit(x,0,4)") << variables << 4.0;
        QTest::newRow("3") << Expression("x*y/z") << variables << x*y/z;
        QTest::newRow("4") << Expression("x^y") << variables << pow(x,y);
        QTest::newRow("5") << Expression("x*y+z") << variables << x*y+z;
        QTest::newRow("6") << Expression("x=y") << variables << 0.0;
        QTest::newRow("7") << Expression("der(x)+y") << variables << y;
        QTest::newRow("8") << Expression("undefinedfunction(x)+y") << variables << y;
        QTest::newRow("9") << Expression("x+A") << variables << x;
        QTest::newRow("10") << Expression("2e5+2") << variables << 2e5+2;
        QTest::newRow("11") << Expression("2e-5+2") << variables << 2e-5+2;
    }
};

QTEST_APPLESS_MAIN(SymHopTests)

#include "tst_symhoptest.moc"
