#include <QtCore/QCoreApplication>
#include <QTime>
#include <QDebug>
#include "math.h"

//Ok this is hack test to test how long time some functions take to run, its not sure that this code results in correct values thog, the
//compiler seems to do some unwanted magic

double myPow2(double a) __attribute__ ((noinline));
void myPow2(double a, double &res) __attribute__ ((noinline));
double mannyArgsCpy(double a, double b, double c, double d, double e, double f, double g) __attribute__ ((noinline));
double mannyArgsRef(double &a, double &b, double &c, double &d, double &e, double &f, double &g) __attribute__ ((noinline));
double mannyArgsPtr(double* a, double* b, double* c, double* d, double* e, double* f, double* g) __attribute__ ((noinline));

double myPow2(double a)
{
    double b,c,d,e;
    a = a*2;
    b = a+a;
    c = sqrt(b);
    d = c*a+b;
    e = d*sqrt(d)+b;

    return e;
}

void myPow2(double a, double &res)
{
    double b,c,d,e;
    a = a*2;
    b = a+a;
    c = sqrt(b);
    d = c*a+b;
    res = d*sqrt(d)+b;
}

double mannyArgsCpy(double a, double b, double c, double d, double e, double f, double g)
{
    double h,i,j,k;
    h = sqrt(a);
    i = h *99;
    j= i * sqrt(d);
    k = j *i /h;
    return a+b+c+d+e+f+g+k;
}

double mannyArgsRef(double &a, double &b, double &c, double &d, double &e, double &f, double &g)
{
    double h,i,j,k;
    h = sqrt(a);
    i = h *99;
    j= i * sqrt(d);
    k = j *i /h;
    return a+b+c+d+e+f+g+k;
}

double mannyArgsPtr(double* a, double* b, double* c, double* d, double* e, double* f, double* g)
{
    double h,i,j,k;
    h = sqrt(*a);
    i = h *99;
    j= i * sqrt(*d);
    k = j *i /h;

    return *a+*b+*c+*d+*e+*f+*g+k;
}

class mannyArgsClass
{
public:
    void reg(double* _a, double* _b, double* _c, double* _d, double* _e, double* _f, double* _g) __attribute__ ((noinline));
    double doIt() __attribute__ ((noinline));

private:
    double *a, *b, *c, *d, *e, *f, *g;
};

void mannyArgsClass::reg(double* _a, double* _b, double* _c, double* _d, double* _e, double* _f, double* _g)
{
    a = _a;
    b = _b;
    c = _c;
    d = _d;
    e = _e;
    f = _f;
    g = _g;
}

double mannyArgsClass::doIt()
{
    double h,i,j,k;
    h = sqrt(*a);
    i = h *99;
    j= i * sqrt(*d);
    k = j *i /h;

    return *a+*b+*c+*d+*e+*f+*g+k;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    //Test modulo
//    qDebug() << "Testing modulo";
//    size_t rIdx = 0;
//    size_t size = 10;
//    for (size_t i=0;i<101; ++i)
//    {
//        qDebug() << "i: " << i << " rIdx: " << rIdx;
//        rIdx = (rIdx + 1) % size;
//    }

    QTime timer;
    timer.start();

    //Test array vs Vector
    #include <vector>
    size_t max = 1000000;
    size_t ntries = 5000;
    double* pArray = new double[max];
    std::vector<double> mVector;
    mVector.resize(max);
    timer.restart();
    for (size_t j=0; j<ntries; ++j)
    {
        for (size_t i=0; i<max; ++i)
        {
            pArray[i] = i;
        }
    }
    qDebug() << "Array time: " << timer.elapsed() << " ms";

    timer.restart();
    for (size_t j=0; j<ntries; ++j)
    {
        for (size_t i=0; i<max; ++i)
        {
            mVector[i] = i;
        }
    }
    qDebug() << "Vector time: " << timer.elapsed() << " ms";


    unsigned int ctr;
    unsigned int maxctr = 10000000000;
    double answer;
    double answer2;


    //Check the time it takes to calc 5^2 (5*5) with a return value, not redeclaring double every time
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        answer = myPow2(5.0);
        answer2 += answer;
    }
    qDebug() << "myPow2 with return value answer declared outside loop: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    timer.restart();
    //Check the time it takes to calc 5^2 (5*5) with a return value, redeclaring double every time
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        double answer = myPow2(5.0);
        answer2 += answer;
    }
    qDebug() << "myPow2 with return value answer redeclared inside loop: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    timer.restart();
    //Check the time it takes to calc 5^2 (5*5) with a ref value, not redeclaring double every time
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        myPow2(5.0, answer);
        answer2 += answer;
    }
    qDebug() << "myPow2 without return value answer declared outside: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    double a=1,b=2,c=3,d=4,e=5,f=6,g=7;
    timer.restart();
    //Check the difference between copying doubles, using pointers, or references
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        answer = mannyArgsCpy(a,b,c,d,e,f,g);
        answer2 += answer;
    }
    qDebug() << "mannyArgs copy doubles: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    timer.restart();
    //Check the difference between copying doubles, using pointers, or references
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        answer = mannyArgsRef(a,b,c,d,e,f,g);
        answer2 += answer;
    }
    qDebug() << "mannyArgs reference doubles: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    double *pA = new double(1.0);
    double *pB = new double(2.0);;
    double *pC = new double(3.0);;
    double *pD = new double(4.0);;
    double *pE = new double(5.0);;
    double *pF = new double(6.0);;
    double *pG = new double(7.0);;
    timer.restart();
    //Check the difference between copying doubles, using pointers, or references
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        answer = mannyArgsPtr(pA,pB,pC,pD,pE,pF,pG);
        answer2 += answer;
    }
    qDebug() << "mannyArgs ptr to doubles: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    mannyArgsClass mannyArgsObj;
    timer.restart();
    mannyArgsObj.reg(&a,&b,&c,&d,&e,&f,&g);
    //Check the difference between copying doubles, using pointers, or references
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        answer = mannyArgsObj.doIt();
        answer2 += answer;
    }
    qDebug() << "mannyArgsClass reg and doit, we are not repeting function calls with arguments: " << timer.elapsed() << " ms";
    //answer2 = 0.0;

    mannyArgsClass* pMannyArgsObj = new mannyArgsClass();
    timer.restart();
    pMannyArgsObj->reg(&a,&b,&c,&d,&e,&f,&g);
    //Check the difference between copying doubles, using pointers, or references
    for (ctr=0; ctr<maxctr; ++ctr)
    {
        answer = pMannyArgsObj->doIt();
        answer2 += answer;
    }
    qDebug() << "mannyArgsClass reg and doit, we are not repeting function calls with arguments: " << timer.elapsed() << " ms";
    //answer2 = 0.0;


    qDebug() << "answer2: " << answer2;
    return app.exec();
}
