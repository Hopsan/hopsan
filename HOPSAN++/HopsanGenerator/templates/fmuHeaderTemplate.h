#ifndef HOPSANFMU_H
#define HOPSANFMU_H

#ifdef WRAPPERCOMPILATION
    extern "C" {
#else
    #define DLLEXPORT
#endif

DLLEXPORT void initializeHopsanWrapper(char* filename);
DLLEXPORT void simulateOneStep();
DLLEXPORT double getVariable(char* component, char* port, size_t idx);

DLLEXPORT void setVariable(char* component, char* port, size_t idx, double value);
DLLEXPORT void setParameter(char* name, double value);

#ifdef WRAPPERCOMPILATION
}
#endif
#endif // HOPSANFMU_H
