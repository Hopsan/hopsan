#ifndef HOPSANC_H
#define HOPSANC_H

#include "hopsanc_win32dll.h"

extern "C" {
    HOPSANC_DLLAPI int loadLibrary(const char* path);
    HOPSANC_DLLAPI int loadModel(const char* path);
    HOPSANC_DLLAPI int setParameter(const char* name, const char *value);
    HOPSANC_DLLAPI void setStartTime(double value);
    HOPSANC_DLLAPI void setTimeStep(double value);
    HOPSANC_DLLAPI void setStopTime(double value);
    HOPSANC_DLLAPI int initialize();
    HOPSANC_DLLAPI int simulate();
    HOPSANC_DLLAPI double *getTimeVector(int &value);
    HOPSANC_DLLAPI double *getDataVector(const char *variable, int &size);
}


#endif // HOPSANC_H
