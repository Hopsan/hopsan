#include "CoreUtilities/LogdataHandler.h"
#include <cstdlib>
#include <cassert>


using namespace hopsan;

namespace hopsan {
class LogDataRequests
{
public:
    LogDataRequests()
    {
        pStart=0;
        ppStart=0;
    }

    size_t modelID;
    size_t nSlots;
    size_t dataLen;
    size_t totalSize() const
    {
        return nSlots*dataLen;
    }
    HShallowVectorD *pStart;
    HShallowVectorD **ppStart;
};

class ModelLogDataRequest
{
public:
    ModelLogDataRequest()
    {
        mNumRequestedElements=0;
    }

    size_t mNumRequestedElements;
    HVectorD mLogData;
};
}

LogdataHandler::LogdataHandler()
{
    mAllocatedSize=0;
    mTotalNumRequestedElements=0;
    //mpLogDataBuffer=0;
}

LogdataHandler::~LogdataHandler()
{
    assert(mRequestMap.empty());
    //clear();
}

size_t LogdataHandler::registerLogRequest(size_t modelId, size_t nSlots, size_t dataLen, HShallowVectorD **ppLogArray, size_t id)
{
    ModelLogDataRequest *pModelRequest=0;
    std::map<size_t, ModelLogDataRequest*>::iterator it = mModelRequestMap.find(modelId);
    if (it!=mModelRequestMap.end())
    {
        pModelRequest = it->second;
    }
    else
    {
        pModelRequest = new ModelLogDataRequest();
        std::pair<size_t, ModelLogDataRequest*> element(modelId,pModelRequest);
        mModelRequestMap.insert(element);
    }

    // If id not 0 then we should reuse an existing request
    if (id!=0)
    {
        std::map<size_t, LogDataRequests*>::iterator it = mRequestMap.find(id);
        if (it != mRequestMap.end())
        {
            LogDataRequests *pLDR = it->second;
            // TODO clear the allocated data if new size of address  given
            if (nSlots*dataLen != pLDR->totalSize() || ppLogArray != pLDR->ppStart)
            {
                deallocateRequest(pLDR);
                pLDR->modelID = modelId;
                pLDR->nSlots = nSlots;
                pLDR->dataLen = dataLen;
                pLDR->ppStart = ppLogArray;
                mTotalNumRequestedElements+=pLDR->totalSize();
                pModelRequest->mNumRequestedElements += pLDR->totalSize();
            }
        }


    }
    // Else create a new one
    else
    {
        size_t lastID=0;
        if (!mRequestMap.empty())
        {
            std::map<size_t, LogDataRequests*>::iterator it = mRequestMap.end();
            it--;
            lastID = it->first;
        }
        id = lastID+1;

        LogDataRequests *pLDR = new LogDataRequests();
        pLDR->modelID = modelId;
        pLDR->nSlots = nSlots;
        pLDR->dataLen = dataLen;
        pLDR->ppStart = ppLogArray;
        std::pair<size_t, LogDataRequests*> element(id, pLDR);

        mTotalNumRequestedElements+=pLDR->totalSize();
        pModelRequest->mNumRequestedElements += pLDR->totalSize();

        mRequestMap.insert(element);
    }
    return id;

}

void LogdataHandler::unregisterLogRequest(size_t id)
{
    std::map<size_t, LogDataRequests*>::iterator it = mRequestMap.find(id);
    if (it != mRequestMap.end())
    {
        // Deallocate memory if allocated
        LogDataRequests *pLDR = it->second;
        deallocateRequest(pLDR);
        delete pLDR;
        mRequestMap.erase(it);
    }
    else
    {
        assert(false);
    }
}

bool LogdataHandler::allocateRequestedMemory(size_t modelId)
{
    // Try allocation
    std::map<size_t, ModelLogDataRequest*>::iterator it = mModelRequestMap.find(modelId);
    if (it!=mModelRequestMap.end())
    {
        ModelLogDataRequest *pModelRequest = it->second;
        pModelRequest->mLogData.resize(pModelRequest->mNumRequestedElements);

        //void *pMem = realloc(mpLogDataBuffer, totalRequestedSizeInBytes());
        //if (pMem)
        if (pModelRequest->mLogData.size() == pModelRequest->mNumRequestedElements)
        {
            //mpLogDataBuffer = pMem;
            //mAllocatedSize = totalRequestedSizeInBytes();

            double* pModelLogDataBuffer = pModelRequest->mLogData.data();

            size_t ctr=0;
            std::map<size_t, LogDataRequests*>::iterator it;
            for (it=mRequestMap.begin(); it!=mRequestMap.end(); ++it)
            {
                LogDataRequests *pLDR = it->second;
                //void *pNewData = realloc(pLDR->pStart, pLDR->totalSize()*sizeof(double));
                if (pLDR->pStart)
                {
                    //pLDR->pStart->resize(pLDR->totalSize());
                    size_t totSize = pLDR->dataLen*pLDR->nSlots;
                    (*pLDR->pStart) = HShallowVectorD(static_cast<double*>(pModelLogDataBuffer)+ctr, pLDR->dataLen*pLDR->nSlots);
                    ctr += totSize;
                }
                else
                {
    //                HVector<double> *pNewData = new HVector<double>();
    //                pNewData->resize(pLDR->totalSize());

    //                // Check if resize was OK
    //                if (pNewData->size() == pLDR->totalSize())
    //                {
    //                    pLDR->pStart = pNewData;
                    size_t totSize = pLDR->dataLen*pLDR->nSlots;
                    pLDR->pStart = new HShallowVectorD(static_cast<double*>(pModelLogDataBuffer)+ctr, pLDR->dataLen*pLDR->nSlots);
                    ctr += totSize;
                    if (pLDR->ppStart)
                    {
                        (*pLDR->ppStart) = pLDR->pStart;
                    }
    //                }
    //                else
    //                {
    //                    return false;
    //                }
                }
            }
            return true;
        }
    }

    return false;
}

void LogdataHandler::deallocateRequest(LogDataRequests *pRequest)
{
    if (pRequest->pStart)
    {
        // Deallocate
        delete pRequest->pStart;
        pRequest->pStart = 0;
        // Check if external data pointer was set, then unset it as memory has been deleted
        if (pRequest->ppStart)
        {
            // Assign 0 to external data pointer, we assume it has not yet been deleted
            (*pRequest->ppStart) = 0;
        }
    }

    std::map<size_t, ModelLogDataRequest*>::iterator it = mModelRequestMap.find(pRequest->modelID);
    if (it!=mModelRequestMap.end())
    {
        it->second->mNumRequestedElements -= pRequest->totalSize();
    }

    mTotalNumRequestedElements-=pRequest->totalSize();
}

//void LogdataHandler::clear()
//{

//}
