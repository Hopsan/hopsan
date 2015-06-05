#ifndef OPTIMIZATIONWORKERSIMPLEX_H
#define OPTIMIZATIONWORKERSIMPLEX_H

#include "OptimizationWorkerComplex.h"

class OptimizationWorkerSimplex : public OptimizationWorkerComplex
{
public:
    OptimizationWorkerSimplex(OptimizationHandler *pHandler);

    void init(const ModelWidget *pModel, const QString &modelPath);
    void run();
    void finalize();
};

#endif // OPTIMIZATIONWORKERSIMPLEX_H
