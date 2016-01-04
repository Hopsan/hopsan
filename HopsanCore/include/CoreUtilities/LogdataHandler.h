#ifndef LOGDATAHANDLER_H
#define LOGDATAHANDLER_H

#include <vector>
#include <map>
#include <cstddef>

#include "HopsanTypes.h"

namespace hopsan
{

class LogDataRequests;
class ModelLogDataRequest;

class LogdataHandler
{
public:
    LogdataHandler();
    ~LogdataHandler();
    size_t registerLogRequest(size_t modelId, size_t nSlots, size_t dataLen, HShallowVectorD **ppLogArray, size_t id=0);
    void unregisterLogRequest(size_t id);
    inline size_t toatalRequestedElements() const
    {
        return mTotalNumRequestedElements;
    }
    inline size_t totalRequestedSizeInBytes() const
    {
        return mTotalNumRequestedElements*sizeof(double);
    }
    inline size_t totalNumAllocatedBytes() const
    {
        return mAllocatedSize;
    }
    bool allocateRequestedMemory(size_t modelId);
    //void clear();

protected:
    void deallocateRequest(LogDataRequests *pRequest);
    std::map<size_t, LogDataRequests*> mRequestMap;
    std::map<size_t, ModelLogDataRequest*> mModelRequestMap;
    //void *mpLogDataBuffer;
    size_t mTotalNumRequestedElements;
    size_t mAllocatedSize;
};

}

#endif // LOGDATAHANDLER_H
