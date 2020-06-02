#ifndef HOPSANC_H
#define HOPSANC_H

#ifdef _WIN32
#if defined HOPSANC_DLLEXPORT
#define HOPSANC_DLLAPI __declspec(dllexport)
#elif defined HOPSANC_DLLIMPORT
#define HOPSANC_DLLAPI __declspec(dllimport)
#else /* Static library */
#define HOPSANC_DLLAPI
#endif
#else
// Define empty on non WIN32 systems
#define HOPSANC_DLLAPI
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    HOPSANC_DLLAPI int printWaitingMessages();
    HOPSANC_DLLAPI int loadLibrary(const char* path);
    HOPSANC_DLLAPI int getMessage(char* buf, size_t bufSize);
    HOPSANC_DLLAPI int loadModel(const char* path);
    HOPSANC_DLLAPI int setParameter(const char* name, const char *value);
    HOPSANC_DLLAPI int setStartTime(double value);
    HOPSANC_DLLAPI int setTimeStep(double value);
    HOPSANC_DLLAPI int setStopTime(double value);
    HOPSANC_DLLAPI int setNumberOfLogSamples(size_t value);
    HOPSANC_DLLAPI int simulate();
    HOPSANC_DLLAPI int getTimeVector(double *data);
    HOPSANC_DLLAPI int getDataVector(const char *variable, double *data);
    HOPSANC_DLLAPI size_t getNumberOfLogSamples();

#ifdef __cplusplus
}
#endif


#endif // HOPSANC_H
