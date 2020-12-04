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
//! @file   Ops.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2015-08-31
//!
//! @brief Contains the optimization worker base class
//!
//$Id$

#ifndef OPSWORKER_H
#define OPSWORKER_H

#include <vector>
#include <string>

#include "OpsWin32DLL.h"

namespace Ops {

enum AlgorithmT {Undefined, NelderMead, ComplexRF, ComplexRFP,  ParticleSwarm, DifferentialEvolution, Genetic, ParameterSweep, ControlledRandomSearch, ComplexBurmen};
enum SamplingT {SamplingRandom, SamplingLatinHypercube};

class Evaluator;
class MessageHandler;

template<typename T>
bool inVector(const std::vector<T> &rVector, const T &value)
{
    typename std::vector<T>::const_iterator it = rVector.begin();
    for(; it!=rVector.end(); ++it)
    {
        if((*it) == value)
            return true;
    }
    return false;
}

template<typename T>
void removeFromVector(std::vector<T> &rVector, const size_t idx)
{
    rVector.erase(rVector.begin() + idx);
}

class OPS_DLLAPI Worker
{
    friend class Evaluator;
public:
    Worker(Evaluator *pEvaluator, MessageHandler *pMessageHandler);
    virtual ~Worker();

    virtual AlgorithmT getAlgorithm();

    virtual void initialize();
    virtual void run();
    virtual void finalize();

    void distrubteCandidatePoints();
    void distributePoints();
    virtual void distributePoints(std::vector<std::vector<double> > *pVector);

    virtual bool checkForConvergence();
    void calculateBestAndWorstId();
    std::vector<size_t> getIdsSortedFromWorstToBest();
    size_t getBestId();
    size_t getWorstId();
    size_t getLastWorstId();



    void setParameterLimits(size_t idx, double min, double max);
    void getParameterLimits(size_t idx, double &min, double &max) const;

    void setCandidateObjectiveValue(size_t idx, double value);
    double getObjectiveValue(size_t idx) const;
    std::vector<double> &getObjectiveValues();
    std::vector<std::vector<double> > &getPoints();
    std::vector<std::vector<double> > &getCandidatePoints();
    double getCandidateParameter(const size_t pointIdx, const size_t parIdx) const;
    double getParameter(const size_t pointIdx, const size_t parIdx) const;
    void setParameter(const size_t pointIdx, const size_t parIdx, const double value);
    void setIgnoreParameterWhenSampling(const size_t pointIdx, const size_t parIdx);
    double getMaxPercentalParameterDiff();
    double getMaxPercentalParameterDiff(std::vector<std::vector<double> > &points);

    std::vector<const char *> getParameterNamesPtr();

    virtual void setNumberOfPoints(size_t value);
    void setNumberOfCandidates(size_t value);
    virtual void setNumberOfParameters(size_t value);
    void setParameterNames(std::vector<const char *> names);
    void setConvergenceTolerance(double value);
    void setMaxNumberOfIterations(size_t value);
    void setTolerance(double value);
    void setSamplingMethod(SamplingT dist);
    void setUseSurrogateModel(size_t interval);

    size_t getNumberOfCandidates();
    size_t getNumberOfPoints();
    size_t getNumberOfParameters();
    size_t getMaxNumberOfIterations();
    size_t getCurrentNumberOfIterations();

    double opsRand();

    bool aborted();

protected:
    size_t mIterationCounter;
    size_t mNumCandidates;
    size_t mNumPoints;
    size_t mNumParameters;
    std::vector< std::string > mParameterNames;
    std::vector<double> mParameterMin, mParameterMax;
    std::vector< std::vector<double> > mPoints;
    std::vector< std::vector<double> > mCandidatePoints;
    std::vector<double> mCandidateObjectives;
    std::vector<double> mObjectives;
    double mnMaxIterations;
    size_t mWorstId, mBestId, mLastWorstId, mSecondBestId;
    double mTolerance;
    SamplingT mDistribution;
    Evaluator *mpEvaluator;
    MessageHandler *mpMessageHandler;
    std::vector<std::pair<size_t,size_t>> mIgnoredWhenSampling;
    bool mUseSurrogateModel;
    size_t mNumSurrogateModelUpdateInterval;
};

}

#endif // OPSWORKER_H
