#ifndef HOPSANFMU_H
#define HOPSANFMU_H

#ifdef __cplusplus
    extern "C" {
#endif

void initializeHopsanWrapper(char* filename);
void initializeHopsanWrapperFromBuiltInModel();
void simulateOneStep();
double getVariable(char* component, char* port, size_t idx);
double getTimeStep();
void setVariable(char* component, char* port, size_t idx, double value);
void setParameter(char* name, double value);

#ifdef __cplusplus
}
#endif

#endif // HOPSANFMU_H
