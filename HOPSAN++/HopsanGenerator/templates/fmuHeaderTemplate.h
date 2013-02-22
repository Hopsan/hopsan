#ifndef HOPSANFMU_H
#define HOPSANFMU_H

#ifdef __cplusplus
    extern "C" {
#endif

void initializeHopsanWrapper(char* filename);
void initializeHopsanWrapperFromBuiltInModel();
void simulateOneStep();
double getVariable(size_t ref, size_t idx);
double getTimeStep();
double getFmuTime();
void setVariable(size_t ref, size_t idx, double value);
void setParameter(char* name, double value);

#ifdef __cplusplus
}
#endif

#endif // HOPSANFMU_H
