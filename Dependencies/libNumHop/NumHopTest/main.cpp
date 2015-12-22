#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>


#include "numhop.h"

using namespace std;
using namespace numhop;

class MyVariableStorage : public ExternalVariableStorage
{
public:
    MyVariableStorage(map<string, double> *pExtVars)
    {
        mpVars = pExtVars;
    }

    double externalValue(string name, bool &rFound) const
    {
        if (mpVars->count(name) > 0)
        {
            rFound = true;
            return mpVars->at(name);
        }
        rFound = false;
        return 0;
    }

    bool setExternalValue(string name, double value)
    {
        map<string, double>::iterator it = mpVars->find(name);
        if (it != mpVars->end())
        {
            //cout << "external found" << endl;
            it->second = value;
            return true;
        }
        return false;
    }

private:
    map<string, double> *mpVars;
};


int main()
{
    map<string, double> externalVars;
    MyVariableStorage extVariableStorage(&externalVars);
    VariableStorage variableStorage;

    variableStorage.setExternalStorage(&extVariableStorage);
    cout << "libNumHop example executable!" << endl;

    externalVars.insert(pair<string,double>("dog", 55));
    externalVars.insert(pair<string,double>("cat", 66));
    std::vector<std::string> expr;
    expr.push_back("a=5;a=8;a;");
    expr.push_back("a=5;\n a=8\n a;");
    expr.push_back("a=1;b=2;c=3;d=a+b*c;d");
    expr.push_back("a=1;b=2;2^2;b^2;b^b;2^(1+1);b^(a+a)");
    expr.push_back("a=1;b=2;c=3;d=4;a=(3+b)+4^2*c^(2+d)-7*(d-1)");
    expr.push_back("a=1;b=2;c=3;d=4;a=(3+b)+4^2*c^(2+d)-7/6/5*(d-1)");
    expr.push_back("7/3/4/5");
    expr.push_back("7/(3/(4/5))");
    expr.push_back("(4/3*14*7/3/4/5*5/(4*3/2))");
    expr.push_back(" \t #   \n    a=5;\n #   a=8\n a+1; \r\n a+2 \r a+3 \r\n #Some comment ");
    expr.push_back("a=1; b=2; a-b; a-b+a");
    expr.push_back("a=1; b=2; -a+b; b-a; (-a)+b; b+(-a); ; b+(+a); b+a; +a+b");
    expr.push_back("a=1;b=2;c=3;d=4; a-b+c-d+a; a-b-c-d+a");
    expr.push_back("1-2*3-3*4-4*5;");
    expr.push_back("1-(-2-3-(-4-5))");
    expr.push_back("dog=4; 1-(-2-3-(-dog-5))");
    expr.push_back("-dog");
    expr.push_back("a=1;b=2;-(a-b)");
    expr.push_back("2e-2-1E-2; 1e+2+3E+2; 1e2+1E2");

    expr.push_back("cat \n dog \r dog=5;cat=2;a=3;b=dog*cat*a;b");
    expr.push_back("-1-2-3*4-4-3");
    expr.push_back("-1-(2-3)*4-4-3");
    expr.push_back("-(((-2-2)-3)*4)");

    expr.push_back("2--3; 1+-3; 1-+3; 1++3; 1---3");

    expr.push_back("#The following will not work!\n 2*-2; a += 5; 1+1-; = 5;");

    for (size_t i=0; i<expr.size(); ++i)
    {
        cout << endl;
        cout << expr[i] << endl;
        cout << "----------------------------------------------------------------" << endl;

        list<string> exprlist;
        extractExpressionRows(expr[i], '#', exprlist);

        for (list<string>::iterator it = exprlist.begin(); it!=exprlist.end(); ++it)
        {
            Expression e;
            bool interpretOK = interpretExpressionStringRecursive(*it, e);
            if (interpretOK)
            {
                bool evalOK;
                double value = e.evaluate(variableStorage, evalOK);
                double value2 = e.evaluate(variableStorage, evalOK); // evaluate again, should give same result
                cout << "Evaluating: ";
                if (evalOK)
                {
                    cout << "OK    : ";
                }
                else
                {
                    cout << "FAILED: ";
                }
                cout << e.print() << "\t\t Value: " << value << " " << value2 << endl;
            }
            else
            {
                cout << "Interpreting FAILED: " << *it << endl;
            }
        }
    }

    return 0;
}

