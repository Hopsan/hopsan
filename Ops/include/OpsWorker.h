/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

namespace Ops {

enum AlgorithmT {Undefined, NelderMead, ComplexRF, ComplexRFP,  ParticleSwarm, DifferentialEvolution, ParameterSweep, ControlledRandomSearch, ComplexBurmen};
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

class Worker
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
    void getParameterLimits(size_t idx, double &min, double &max);

    void setCandidateObjectiveValue(size_t idx, double value);
    double getObjectiveValue(size_t idx);
    std::vector<double> &getObjectiveValues();
    std::vector<std::vector<double> > &getPoints();
    std::vector<std::vector<double> > &getCandidatePoints();
    double getCandidateParameter(const size_t pointIdx, const size_t parIdx) const;
    double getParameter(const size_t pointIdx, const size_t parIdx) const;
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

    size_t getNumberOfCandidates();
    size_t getNumberOfPoints();
    size_t getNumberOfParameters();
    size_t getMaxNumberOfIterations();
    size_t getCurrentNumberOfIterations();

    double opsRand();

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
};

}

#endif // OPSWORKER_H
