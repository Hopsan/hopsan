#include "OpsWorkerControlledRandomSearch.h"
#include "OpsWorkerSimplex.h"
#include "OpsEvaluator.h"
#include <math.h>

using namespace Ops;

WorkerControlledRandomSearch::WorkerControlledRandomSearch(Evaluator *pEvaluator)
    : WorkerSimplex(pEvaluator)
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
    emit message("Running optimization with controlled random search (CRS2) algorithm.");

    distributePoints();

    mpEvaluator->evaluateAllPoints();
    emit objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mIsAborted; ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        //Calculate best and worst point
        calculateBestAndWorstId();

        while(true)
        {
            QVector<size_t> chosenIdx;
            QVector< QVector<double> > chosenPoints;
            chosenIdx.append(mBestId);
            chosenIdx.append(mWorstId);
            chosenPoints.append(mPoints[mBestId]);
            for(int i=0; i<mNumParameters-1; ++i)
            {
                int idx = opsRand();
                while(chosenIdx.contains(idx))
                {
                    idx = round(opsRand()*(mNumPoints-1));
                }
                chosenIdx.append(idx);
                chosenPoints.append(mPoints[idx]);
            }

            //Find geometrical center
            findCentroidPoint(chosenPoints);

            //Reflect worst point
            mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, 1.0);
            emit candidateChanged(0);

            //Check if constraints are evaluated, if so, do new reflection
            bool constraintsViolated=false;
            for(int p=0; p<mNumParameters; ++p)
            {
                if(mCandidatePoints[0][p] < mParameterMin[p] ||
                   mCandidatePoints[0][p] > mParameterMax[p])
                {
                    constraintsViolated = true;
                }
            }
            if(!constraintsViolated)
            {
                //Evaluate new point
                mpEvaluator->evaluateCandidate(0);

                //Check if new point is better; if so, keep it
                if(mCandidateObjectives[0] < mObjectives[mWorstId])
                {
                    mPoints[mWorstId] = mCandidatePoints[0];
                    mObjectives[mWorstId] = mCandidateObjectives[0];
                    emit pointChanged(mWorstId);
                    emit objectiveChanged(mWorstId);
                    break;
                }
            }
        }

        emit stepCompleted(mIterationCounter);
    }


    if(mIsAborted)
    {
        emit message("Optimization was aborted after "+QString::number(mIterationCounter)+" iterations.");
    }
    else if(mIterationCounter == mnMaxIterations)
    {
        emit message("Optimization failed to converge after "+QString::number(mIterationCounter)+" iterations");
    }
    else
    {
        emit message("Optimization converged in parameter values after "+QString::number(mIterationCounter)+" iterations.");
    }

    // Clean up
    finalize();

    return;
}
