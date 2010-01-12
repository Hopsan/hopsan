#ifndef TRANSFERFUNCTION_H_INCLUDED
#define TRANSFERFUNCTION_H_INCLUDED

#include <deque>
#include "win32dll.h"

class DLLIMPORTEXPORT TransferFunction
{
public:
    TransferFunction();
    void Initialize();
    double Filter(double signal, const double num [3], const double den [3], double timestep);
private:
    double u [3];
	double y [3];
	double a [3];
	double b [3];
};

#endif // TRANSFERFUNCTION_H_INCLUDED
