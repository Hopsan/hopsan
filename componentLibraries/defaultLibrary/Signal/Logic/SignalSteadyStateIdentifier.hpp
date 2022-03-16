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
//! @file   SignalSteadyStateIdentifier.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2022-03-10
//!
//! @brief Steady-state identifier component
//!
//$Id$

#ifndef SIGNALSTEADYSTATEIDENTIFIER_H
#define SIGNALSTEADYSTATEIDENTIFIER_H

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <limits>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSteadyStateIdentifier : public ComponentSignal
    {

    private:
        double *mpIn, *mpOut, *mpTol, *mpWl, *mpSd, *mpL1, *mpL2, *mpL3;
        int mMethod;
        std::vector<double> mWindow;
        size_t mWindowId;
        double mDelayedX;
        double mDelayedXf;
        double mDelayedDf;

    public:
        static Component *Creator()
        {
            return new SignalSteadyStateIdentifier();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            addOutputVariable("out", "1 if steadystate, else 0", "", &mpOut);

            std::vector<HString> methods;
            methods.push_back("1: Rectangular sliding window");
            methods.push_back("2: Variance ratio test (F-test)");
            methods.push_back("3: Exponentially weighted moving average (R-test)");

            addConditionalConstant("method", "Estimation method", methods, 0, mMethod);

            addInputVariable("tol", "Tolerance", "", 1.0, &mpTol);
            addInputVariable("wl", "Length of sliding window (for methods 1 and 2)", "", 0.1, &mpWl);
            addInputVariable("sd", "Standard deviation of white noise (for methods 2 and 3)", "", 0.01, &mpSd);
            addInputVariable("l1", "Filter factor 1 (for method 3)", "", 0.1, &mpL1);
            addInputVariable("l2", "Filter factor 2 (for method 3)", "", 0.1, &mpL2);
            addInputVariable("l3", "Filter factor 3 (for method 3)", "", 0.1, &mpL3);
        }


        void initialize()
        {
            mWindowId = 0;
            mWindow.resize(*mpWl/mTimestep, 0);

            mDelayedX = (*mpIn);
            mDelayedXf = (*mpIn);
            mDelayedDf = 0;

            if(mWindow.size() <= 2) {
                stopSimulation("Sliding window is too small compared to time step");
                return;
            }

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            if(mMethod == 0) {  //Rectangular sliding window
                mWindow[mWindowId] = *mpIn;
                mWindowId++;
                if(mWindowId > mWindow.size()-1) {
                    mWindowId = 0;
                }

                double xmax = mWindow[0];
                double xmin = mWindow[0];
                for(int i=0; i<mWindow.size(); ++i) {
                    if(mWindow[i] > xmax) {
                        xmax = mWindow[i];
                    }
                    else if(mWindow[i] < xmin) {
                        xmin = mWindow[i];
                    }
                }

                if(xmax-xmin < *mpTol) {
                    (*mpOut) = 1;
                }
                else {
                    (*mpOut) = 0;
                }
            }
            else if(mMethod == 1) { //Variance Ratio Test
                mWindow[mWindowId] = *mpIn;
                mWindowId++;
                if(mWindowId > mWindow.size()-1) {
                    mWindowId = 0;
                }

                //Randomize window
                std::vector<double> randWindow = mWindow;
                for(size_t i=0; i<randWindow.size(); ++i) {
                    randWindow[i] = randWindow[i] + (*mpSd)*WhiteGaussianNoise::getValue();
                }

                //Compute average of window
                double mean = 0;
                for(size_t i=0; i<randWindow.size(); ++i) {
                    mean += randWindow[i];
                }
                mean /= (double)randWindow.size();

                //Compute first variance
                double s1 = 0;
                for(size_t i=0; i<randWindow.size(); ++i) {
                    s1 += (randWindow[i]-mean)*(randWindow[i]-mean);
                }
                s1 /= ((double)randWindow.size()-1.0);

                //Compute second variance
                //This requires iterating in correct order
                double s2 = 0;
                size_t i = mWindowId;
                size_t j = (i == mWindow.size()-1) ? 0 : mWindowId+1;
                for(size_t k=0; k<mWindow.size()-1; ++k) {
                    s2 += (randWindow[j]-randWindow[i])*(randWindow[j]-randWindow[i]);
                    i = (i == mWindow.size()-1) ? 0 : i+1;
                    j = (i == mWindow.size()-1) ? 0 : i+1;
                }
                s2 /= ((double)randWindow.size()-1.0);

                //Avoid division by zero
                s1 = fmax(std::numeric_limits<double>::min(), fabs(s1));
                s2 = fmax(std::numeric_limits<double>::min(), fabs(s2));

                if(s1/s2 < (*mpTol)) {
                    (*mpOut) = 1;
                }
                else {
                    (*mpOut) = 0;
                }
            }
            else if(mMethod == 2) { // Moving Average Variance Ratio Test
                double l1 = (*mpL1);
                double l2 = (*mpL2);
                double l3 = (*mpL3);
                double x = (*mpIn);
                double xold = mDelayedX;
                double xf = mDelayedXf;
                double sd = (*mpSd);
                double dfold = mDelayedDf;

                x = x+sd*WhiteGaussianNoise::getValue();

                double vf = l2*(x-xf)*(x-xf);
                double s1 = (2.0-l1)/2.0*vf;

                double df = l3*(x-xold)*(x-xold) + (1-l3)*dfold;
                double s2 = df/2.0;

                //Avoid division by zero
                s1 = fmax(std::numeric_limits<double>::min(), fabs(s1));
                s2 = fmax(std::numeric_limits<double>::min(), fabs(s2));

                if(s1/s2 < (*mpTol)) {
                    (*mpOut) = 1;
                }
                else {
                    (*mpOut) = 0;
                }

                mDelayedX = x;
                mDelayedXf = l1*x+(1.0-l1)*xf;
                mDelayedDf = df;
            }
        }
    };
}

#endif // SIGNALSTEADYSTATEIDENTIFIER_H
