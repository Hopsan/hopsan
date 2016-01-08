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

        for(int i=0; i<mNumCandidates; ++i)
        {

            QVector<size_t> chosenIdx;
            QVector< QVector<double> > chosenPoints;
            chosenIdx.append(mBestId);
            chosenIdx.append(mWorstId);
            chosenPoints.append(mPoints[mBestId]);
            for(int j=0; j<mNumParameters-1; ++j)
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
            mCandidatePoints[i] = reflect(mPoints[mWorstId], mCentroidPoint, 1.0);
            emit candidateChanged(0);

            //Check if constraints are violated, if so, do new reflection
            //bool constraintsViolated=false;
            for(int p=0; p<mNumParameters; ++p)
            {
                if(mCandidatePoints[i][p] < mParameterMin[p] ||
                        mCandidatePoints[i][p] > mParameterMax[p])
                {
                    --i;
                }
            }
        }

        mpEvaluator->evaluateAllCandidates();
        double bestObj = 1e100;
        int bestId = -1;
        for(int i=0; i<mNumCandidates; ++i)
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
            emit pointChanged(mWorstId);
            emit objectiveChanged(mWorstId);
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
