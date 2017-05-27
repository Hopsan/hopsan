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

#ifndef OPTIMIZATIONTESTFUNCTION2D_HPP_INCLUDED
#define OPTIMIZATIONTESTFUNCTION2D_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities/AuxiliarySimulationFunctions.h"
#include <math.h>

namespace hopsan {

class OptimizationTestFunction2D : public ComponentSignal
{
private:
    double *mpX, *mpY, *mpOut;
    int mFunc;

public:
    static Component *Creator()
    {
        return new OptimizationTestFunction2D();
    }

    void configure()
    {
        // Add ports to the component
        addInputVariable("x", "", "", 0.0, &mpX);
        addInputVariable("y", "", "", 0.0, &mpY);
        addOutputVariable("out", "", "", 0.0, &mpOut);

        std::vector<HString> functions;
        functions.push_back("Ackley's' Function");
        functions.push_back("Beale's' Function");
        functions.push_back("Booth's' Function");
        functions.push_back("Bukin Function N.6");
        functions.push_back("Cross-In-Tray Function");
        functions.push_back("Easom Function");
        functions.push_back("Eggholder Function");
        functions.push_back("Goldstein-Price Function");
        functions.push_back("Holder Table Function");
        functions.push_back("Levi Function");
        functions.push_back("Matyas Function");
        functions.push_back("McCormick Function");
        functions.push_back("Peaks Function");
        functions.push_back("Rosenbrock Function");
        functions.push_back("Schaffer Function N.2");
        functions.push_back("Schaffer Function N.4");
        functions.push_back("Sphere Function");
        functions.push_back("Styblinski-Tang Function");
        functions.push_back("Three-Hump Camel Function");
        addConditionalConstant("function", "Test Function", functions, mFunc);
    }

    void initialize()
    {
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        double x = (*mpX);
        double y = (*mpY);

        switch(mFunc)
        {
        case 0:
            (*mpOut) = ackley(x,y);
            break;
        case 1:
            (*mpOut) = beale(x,y);
            break;
        case 2:
            (*mpOut) = booth(x,y);
            break;
        case 3:
            (*mpOut) = bukin6(x,y);
            break;
        case 4:
            (*mpOut) = crossInTray(x,y);
            break;
        case 5:
            (*mpOut) = easom(x,y);
            break;
        case 6:
            (*mpOut) = eggholder(x,y);
            break;
        case 7:
            (*mpOut) = goldsteinPrice(x,y);
            break;
        case 8:
            (*mpOut) = holderTable(x,y);
            break;
        case 9:
            (*mpOut) = levi(x,y);
            break;
        case 10:
            (*mpOut) = matyas(x,y);
            break;
        case 11:
            (*mpOut) = mcCormick(x,y);
            break;
        case 12:
            (*mpOut) = peaks(x,y);
            break;
        case 13:
            (*mpOut) = rosenbrock(x,y);
            break;
        case 14:
            (*mpOut) = schaffer2(x,y);
            break;
        case 15:
            (*mpOut) = schaffer4(x,y);
            break;
        case 16:
            (*mpOut) = sphere(x,y);
            break;
        case 17:
            (*mpOut) = styblinskiTang(x,y);
            break;
        case 18:
            (*mpOut) = threeHumpCamel(x,y);
            break;
        default:
            (*mpOut) = 0;
            break;
        }
    }



    inline double ackley(double x, double y) const
    {
        return -20.0*exp(-0.2*sqrt(0.5*(x*x+y*y))) - exp(0.5*(cos(2.0*pi*x)+cos(2.0*pi*y))) + 20 + 2.71828;
    }

    inline double beale(double x, double y) const
    {
        return (15.0-x+x*y)*(15.0-x+x*y) + (2.25-x+x*y*y)*(2.25-x+x*y*y)+(2.625-x+x*y*y*y)*(2.625-x+x*y*y*y);
    }

    inline double booth(double x, double y) const
    {
        return (x+2.0*y-7.0)*(x+2.0*y-7.0) + (2.0*x+y-5.0)*(2.0*x+y-5.0);
    }

    inline double bukin6(double x, double y) const
    {
        return 100.0*sqrt(fabs(y-0.01*x*x)) + 0.01*fabs(x+10.0);
    }

    inline double crossInTray(double x, double y) const
    {
        return -0.0001*pow( fabs(sin(x)*sin(y)*exp(fabs(100.0-sqrt(x*x+y*y)/pi))) + 1.0, 0.1);
    }

    inline double easom(double x, double y) const
    {
        return -cos(x)*cos(y)*exp( -( (x-pi)*(x-pi) + (y-pi)*(y-pi) ) );
    }

    inline double eggholder(double x, double y) const
    {
        return -(y+47.0)*sin(sqrt(fabs(y+x/2.0+47.0)))-x*sin(fabs(x-(y+47.0)));
    }

    inline double goldsteinPrice(double x, double y) const
    {
        return (1.0 + (x+y+1.0)*(x+y+1.0) * (19.0-14.0*x+3.0*x*x-14.0*y+6.0*x*y+3.0*y*y)) * (30.0+(2.0*x-3.0*y)*(2.0*x-3.0*y)*(18.0-32.0*x+12.0*x*x+48.0*y-36.0*x*y+27.0*y*y));
    }

    inline double holderTable(double x, double y) const
    {
        return -fabs(sin(x)*cos(y)*exp(fabs(1-sqrt(x*x+y*y)/pi)));
    }

    inline double levi(double x, double y) const
    {
        return sin(3.0*pi*x)*sin(3.0*pi*x) + (x-1.0)*(x-1.0)*(1.0+sin(3.0*pi*y)*sin(3.0*pi*y)) + (y-1.0)*(y-1.0)*(1.0+sin(2.0*pi*y)*sin(2.0*pi*y));
    }

    inline double matyas(double x, double y) const
    {
        return 0.26*(x*x+y*y) - 0.48*x*y;
    }

    inline double mcCormick(double x, double y) const
    {
        return sin(x+y)+(x-y)*(x-y)-1.5*x+2.5*y+1;
    }

    inline double peaks(double x, double y) const
    {

        return 3.0*(1.0-x)*(1.0-x)*exp(-x*x - (y+1.0)*(y+1.0)) - 10.0*(x/5.0 - x*x*x - y*y*y*y*y)*exp(-x*x-y*y) - 1.0/3.0*exp(-(x+1.0)*(x+1.0) - y*y);
    }

    inline double rosenbrock(double x, double y) const
    {
        return pow(1.0-x,2.0)+100.0*pow(y-pow(x,2.0),2.0);
    }

    inline double schaffer2(double x, double y) const
    {
        return 0.5 + (sin(x*x-y*y)*sin(x*x-y*y)-0.5) / ( (1.0+0.001*(x*x+y*y)) * (1.0+0.001*(x*x+y*y)) );
    }

    inline double schaffer4(double x, double y) const
    {
        return 0.5 + (cos(sin(fabs(x*x-y*y)))-0.5) / ( (1.0+0.001*(x*x+y*y)) * (1.0+0.001*(x*x+y*y)) );
    }

    inline double sphere(double x, double y) const
    {
        return x*x+y*y;
    }

    inline double styblinskiTang(double x, double y) const
    {
        return (x*x*x*x-16.0*x*x+5.0*x + y*y*y*y-16.0*y*y+5.0*y)/2.0;
    }

    inline double threeHumpCamel(double x, double y) const
    {
        return 2.0*x*x - 1.05*x*x*x*x + x*x*x*x*x*x/6.0 + x*y + y*y;
    }

};

}
#endif //OPTIMIZATIONTESTFUNCTION2D_HPP_INCLUDED
