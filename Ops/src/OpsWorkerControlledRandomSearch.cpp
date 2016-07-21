#include "OpsWorkerControlledRandomSearch.h"
#include "OpsWorkerSimplex.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>

using namespace Ops;

WorkerControlledRandomSearch::WorkerControlledRandomSearch(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : WorkerSimplex(pEvaluator, pMessageHandler)
{
}

AlgorithmT WorkerControlledRandomSearch::getAlgorithm()
{
    return ControlledRandomSearch;
}

void WorkerControlledRandomSearch::initialize()
{
    WorkerSimplex::initialize();
}

void WorkerControlledRandomSearch::run()
{
    mpMessageHandler->printMessage("Running optimization with controlled random search (CRS2) algorithm.");

    distributePoints();

    mpEvaluator->evaluateAllPoints();
    mpMessageHandler->objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    size_t iterationsSinceLastChange=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        ++iterationsSinceLastChange;

        //Check convergence
        if(checkForConvergence()) break;

        //Calculate best and worst point
        calculateBestAndWorstId();

        if(iterationsSinceLastChange > std::max(size_t(1),mNumPoints/mNumCandidates/5))
        {
            //distrubteCandidatePoints();
            bool feasible;
            mCandidatePoints[0] = reflect(mPoints[mWorstId], mPoints[mBestId], -0.5, feasible);
            mpEvaluator->evaluateCandidate(0);
            mPoints[mWorstId] = mCandidatePoints[0];
            mObjectives[mWorstId] = mCandidateObjectives[0];
            iterationsSinceLastChange=0;
            mpMessageHandler->candidateChanged(0);
            mpMessageHandler->pointChanged(mWorstId);
            mpMessageHandler->objectiveChanged(mWorstId);
        }
        else
        {
            size_t nFailed=0;
            for(size_t i=0; i<mNumCandidates; ++i)
            {
                std::vector<size_t> chosenIdx;
                std::vector< std::vector<double> > chosenPoints;
                chosenIdx.push_back(mBestId);
                //chosenIdx.push_back(mWorstId);
                chosenPoints.push_back(mPoints[mBestId]);
                for(size_t j=0; j<mNumParameters; ++j)
                {
                    size_t idx = opsRand();
                    while(inVector(chosenIdx,idx))
                    {
                        idx = round(opsRand()*(mNumPoints-1));
                    }
                    chosenIdx.push_back(idx);
                    chosenPoints.push_back(mPoints[idx]);
                }

                //Find geometrical center
                findCentroidPoint(chosenPoints);

                //Reflect worst point
                bool feasible;
                if(nFailed < 100)
                {
                    mCandidatePoints[i] = reflect(mPoints[mWorstId], mCentroidPoint, 1.0, feasible);
                }
                else
                {
                    std::vector<std::vector<double> > *pVector = new std::vector<std::vector<double> >();
                    pVector->push_back(mCandidatePoints[i]);
                    distributePoints(pVector);
                    delete pVector;
                }
                mpMessageHandler->candidateChanged(0);

                //Check if constraints are violated, if so, do new reflection
                //bool constraintsViolated=false;
                //for(size_t p=0; p<mNumParameters; ++p)
                //{
                //    if(mCandidatePoints[i][p] < mParameterMin[p] ||
                //            mCandidatePoints[i][p] > mParameterMax[p])
                //    {
                //        --i;
                //    }
                //}
                if(!feasible)
                {
                    ++nFailed;
                    --i;
                }
            }


          mpEvaluator->evaluateAllCandidates();
          double bestObj = 1e100;
          int bestId = -1;
          for(size_t i=0; i<mNumCandidates; ++i)
          {
              if(mCandidateObjectives[i] < bestObj)
              {
                  bestObj = mCandidateObjectives[i];
                  bestId = i;
              }
          }

          //Check if new point is better; if so, keep it
          if(mCandidateObjectives[bestId] < mObjectives[mWorstId])
          {
              mPoints[mWorstId] = mCandidatePoints[bestId];
              mObjectives[mWorstId] = mCandidateObjectives[bestId];
              mpMessageHandler->pointChanged(mWorstId);
              mpMessageHandler->objectiveChanged(mWorstId);
              iterationsSinceLastChange=0;
          }
       }

        mpMessageHandler->stepCompleted(mIterationCounter);
    }


    if(mpMessageHandler->aborted())
    {
        mpMessageHandler->printMessage("Optimization was aborted after "+std::to_string(mIterationCounter)+" iterations.");
    }
    else if(mIterationCounter == mnMaxIterations)
    {
        mpMessageHandler->printMessage("Optimization failed to converge after "+std::to_string(mIterationCounter)+" iterations");
    }
    else
    {
        mpMessageHandler->printMessage("Optimization converged in parameter values after "+std::to_string(mIterationCounter)+" iterations.");
    }

    // Clean up
    finalize();

    return;
}
