/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   CachableDataVector.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-02-12
//!
//! @brief Contains classes for caching data vectors to disk
//!
//$Id: LogVariable.cpp 4974 2013-02-01 08:47:48Z petno25 $

#include "CachableDataVector.h"

#include <QDebug>

MultiDataVectorCache::MultiDataVectorCache(const QString fileName)
{
    mIsMultiAppending = false;
    mIsMultiReadWriting = false;
    mIsMultiReading = false;
    mNumSubscribers = 0;
    mCacheFile.setFileName(fileName);
}

MultiDataVectorCache::~MultiDataVectorCache()
{
    removeCacheFile();
}

bool MultiDataVectorCache::addVector(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes)
{
    //! @todo maybe have register for how long added data at each start adress was, to prevent destorying data
    return appendToCache(rDataVector, rStartByte, rNumBytes);
}

bool MultiDataVectorCache::beginMultiAppend()
{
    mIsMultiAppending = smartOpenFile(QIODevice::WriteOnly | QIODevice::Append);
    return mIsMultiAppending;
}

void MultiDataVectorCache::endMultiAppend()
{
    mIsMultiAppending = false;
    smartCloseFile();
}

bool MultiDataVectorCache::copyDataTo(const quint64 startByte, const quint64 nBytes, QVector<double> &rData)
{
    return readToMem(startByte, nBytes, &rData);
}

bool MultiDataVectorCache::replaceData(const quint64 startByte, const QVector<double> &rNewData, quint64 &rNumBytes)
{
    //! @todo prevent destroying data if new data have new longer length
    return writeInCache(startByte, rNewData, rNumBytes);
}


bool MultiDataVectorCache::writeInCache(const quint64 startByte, const QVector<double> &rDataVector, quint64 &rBytesWriten)
{
    bool success = false;
    if (smartOpenFile(QIODevice::ReadWrite))
    {
        if (mCacheFile.seek(startByte))
        {
            rBytesWriten = mCacheFile.write((const char*)rDataVector.data(), sizeof(double)*rDataVector.size());
            if (rBytesWriten==rDataVector.size()*sizeof(double))
            {
                success = true;
            }
        }
    }

    if (!success)
    {
        mError = mCacheFile.errorString();
    }
    smartCloseFile();
    return success;
}

bool MultiDataVectorCache::appendToCache(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes)
{
    rNumBytes = 0;

    // Abort if data vector empty
    if (rDataVector.isEmpty())
    {
        return false;
    }

    bool success = false;
    if (smartOpenFile(QIODevice::WriteOnly | QIODevice::Append))
    {
        rStartByte = mCacheFile.pos();
        rNumBytes = mCacheFile.write((const char*)rDataVector.data(), sizeof(double)*rDataVector.size());
        if ( rNumBytes == sizeof(double)*rDataVector.size())
        {
            success = true;
        }
    }

    if (!success)
    {
        mError = mCacheFile.errorString();
    }
    smartCloseFile();
    return success;
}

bool MultiDataVectorCache::readToMem(const quint64 startByte, const quint64 nBytes, QVector<double> *pDataVector)
{
    bool success = false;
    if (smartOpenFile(QIODevice::ReadOnly))
    {
        if (mCacheFile.seek(startByte))
        {
            pDataVector->resize(nBytes/sizeof(double));
            qint64 n = mCacheFile.read((char*)pDataVector->data(), nBytes);
            if (quint64(n) == nBytes)
            {
                success = true;
            }
        }
    }

    if (!success)
    {
        mError = mCacheFile.errorString();
    }
    smartCloseFile();
    return success;
}

bool MultiDataVectorCache::smartOpenFile(QIODevice::OpenMode flags)
{
    if (mCacheFile.isOpen())
    {
        if (mCacheFile.openMode() == flags)
        {
            return true;
        }
        return false;
    }
    return mCacheFile.open(flags);
}

void MultiDataVectorCache::smartCloseFile()
{
    if (!(mIsMultiAppending || mIsMultiReadWriting || mIsMultiReading))
    {
        mCacheFile.close();
    }
}

void MultiDataVectorCache::removeCacheFile()
{
    bool rc = mCacheFile.remove();
    qDebug() << "Removing file: " << mCacheFile.fileName() << " : " << rc;
}


bool MultiDataVectorCache::peek(const quint64 byte, double &rVal)
{
    bool success=false;
    if (smartOpenFile(QIODevice::ReadOnly))
    {
        if (mCacheFile.seek(byte))
        {
            if (mCacheFile.peek((char*)&rVal, sizeof(double)) == sizeof(double))
            {
                success = true;
            }
        }
    }

    if (!success)
    {
        mError = mCacheFile.errorString();
    }
    smartCloseFile();
    return success;
}

bool MultiDataVectorCache::poke(const quint64 byte, const double val)
{
    bool success=false;
    if (smartOpenFile(QIODevice::ReadWrite))
    {
        if (mCacheFile.seek(byte))
        {
            if (mCacheFile.write((const char*)&val, sizeof(double)) == sizeof(double))
            {
                success = true;
            }
        }
    }

    if (!success)
    {
        mError = mCacheFile.errorString();
    }
    smartCloseFile();
    return success;
}

bool MultiDataVectorCache::checkoutVector(const quint64 startByte, const quint64 nBytes, QVector<double> *&rpData)
{
    //! @todo how to prevent checkout of same data multiple times
    QVector<double> *pData = new QVector<double>();
    if (readToMem(startByte, nBytes, pData))
    {
        rpData = pData;
        mCheckoutMap.insert(pData, CheckoutInfoT(startByte,nBytes));
        return true;
    }
    delete pData;
    return false;
}

bool MultiDataVectorCache::returnVector(QVector<double> *&rpData)
{
    bool rc = false;
    if (mCheckoutMap.contains(rpData))
    {
        CheckoutInfoT info = mCheckoutMap.value(rpData, CheckoutInfoT(0,0));
        if (info.nBytes == rpData->size()*sizeof(double))
        {
            quint64 dummy;
            rc = writeInCache(info.startByte, *rpData, dummy);
            delete rpData;
            rpData = 0;
        }
        else
        {
            mError = "Trying to write new data that is to long or to short";
            //! @todo handle this instead, append maybe
        }
    }
    else
    {
        mError = "Pointer to data that has not been checked out";
    }
    return rc;
}


QString MultiDataVectorCache::getError() const
{
    return mError;
}

QFileInfo MultiDataVectorCache::getCacheFileInfo() const
{
    return QFileInfo(mCacheFile);
}

void MultiDataVectorCache::incrementSubscribers()
{
    ++mNumSubscribers;
}

//! @brief Decrement num subscribers, if no subscribers remain the cache file will be deleted (even if this object still remains).
void MultiDataVectorCache::decrementSubscribers()
{
    // If mNumSubs are 0or1 before this subtraction (it will be the last subscriber), 0 case should not happen unless someone has forgotten to increment
    --mNumSubscribers;
    if ( mNumSubscribers == 0)
    {
        removeCacheFile();
    }

    if (mNumSubscribers < 0)
    {
        qWarning() << "-- MultiDataVectorCache::decrementSubscribers: mNumSubscribers < 0" << mNumSubscribers << " This should not happen!";
    }
}

qint64 MultiDataVectorCache::getNumSubscribers() const
{
    return mNumSubscribers;
}


CachableDataVector::CachableDataVector(const QVector<double> &rDataVector, SharedMultiDataVectorCacheT pMultiCache, const bool cached)
{
    mCacheStartByte = 0;
    mCacheNumBytes = 0;
    // This bool is needed so that we can handled non-cached but empty data.
    // We can not rely on checking size of mDataVector (may be empty but non-cached) or the mCacheNumBytes (will still have a value after moving from cache to mem temporarly)
    mIsCached = false;

    if (pMultiCache)
    {
        mpMultiCache = pMultiCache;
        mpMultiCache->incrementSubscribers();
    }

    if (mpMultiCache && cached)
    {
        if (mpMultiCache->addVector(rDataVector,mCacheStartByte, mCacheNumBytes))
        {
            mIsCached = true;
        }
        else
        {
            mError = mpMultiCache->getError();
        }
    }
    else
    {
        mDataVector = rDataVector;
    }
}

CachableDataVector::~CachableDataVector()
{
    if (mpMultiCache)
    {
        mpMultiCache->decrementSubscribers();
    }
}

//! @brief Moves data to and from disk cache
bool CachableDataVector::setCached(const bool cached)
{
    bool rc;
    if (cached)
    {
        rc = moveToCache();
    }
    else
    {
        rc = copyToMem();
    }
    return rc;
}

bool CachableDataVector::isCached() const
{
    return mIsCached;

}

int CachableDataVector::size() const
{
    if (isCached())
    {
        return mCacheNumBytes/sizeof(double);
    }
    else
    {
        return mDataVector.size();
    }
}

bool CachableDataVector::isEmpty() const
{
    if (isCached())
    {
        return (mCacheNumBytes == 0);
    }
    else
    {
        return mDataVector.isEmpty();
    }
}

bool CachableDataVector::streamDataTo(QTextStream &rTextStream, const QString separator)
{
    if (isCached())
    {
        QVector<double> temp;
        bool isOk = copyDataTo(temp);
        int i=0;
        for (; i<temp.size()-1; ++i)
        {
            rTextStream << temp[i] << separator;
        }
        rTextStream << temp[i];
        return isOk;
    }
    else
    {
        int i=0;
        for (; i<mDataVector.size()-1; ++i)
        {
            rTextStream << mDataVector[i] << separator;
        }
        rTextStream << mDataVector[i];
        return true;
    }
}

bool CachableDataVector::copyDataTo(QVector<double> &rData)
{
    if (isCached())
    {
        if (!mpMultiCache->copyDataTo(mCacheStartByte, mCacheNumBytes, rData))
        {
            mError = mpMultiCache->getError();
            return false;
        }
    }
    else
    {
        rData = mDataVector;
    }
    return true;
}

bool CachableDataVector::replaceData(const QVector<double> &rNewData)
{
    if (isCached())
    {
        // If same length, then replace actual data
        if (rNewData.size()*sizeof(double) <= mCacheNumBytes)
        {
            return mpMultiCache->replaceData(mCacheStartByte, rNewData, mCacheNumBytes);
        }
        else
        {
            // Else append to the end of the file
            // This will leave the old data as junk in the file, but trying to delete it or move it would be to slow, and we live with the wasted space instead
            return mpMultiCache->addVector(rNewData, mCacheStartByte, mCacheNumBytes);
        }
    }
    else
    {
        mDataVector = rNewData;
        return true;
    }
}

bool CachableDataVector::peek(const int idx, double &rVal)
{
    if (isCached())
    {
        if (!mpMultiCache->peek(mCacheStartByte+idx*sizeof(double), rVal))
        {
            mError = mpMultiCache->getError();
            return false;
        }
    }
    else
    {
        if (idx >= mDataVector.size())
        {
            mError = "Index out of bounds";
            return false;
        }
        rVal = mDataVector[idx];
    }
    return true;
}

bool CachableDataVector::poke(const int idx, const double val)
{
    if (isCached())
    {
        if (!mpMultiCache->poke(mCacheStartByte+idx*sizeof(double),val))
        {
            mError = mpMultiCache->getError();
            return false;
        }
    }
    else
    {
        if (idx >= mDataVector.size())
        {
            mError = "Index out of bounds";
            return false;
        }
        mDataVector[idx] = val;
    }
    return true;
}

QVector<double> *CachableDataVector::beginFullVectorOperation()
{
    //! @todo what if trying to checkout vector that has not been created
    QVector<double> *pData = 0;
    if(isCached())
    {
        if (!mpMultiCache->checkoutVector(mCacheStartByte, mCacheNumBytes, pData))
        {
            mError = mpMultiCache->getError();
        }
    }
    else
    {
        pData = &mDataVector;
    }
    return pData;
}

bool CachableDataVector::endFullVectorOperation(QVector<double> *&rpData)
{
    bool rc = true;
    if(isCached())
    {
        rc = mpMultiCache->returnVector(rpData);
        if (!rc)
        {
            mError = mpMultiCache->getError();
        }
    }
    rpData = 0;
    return rc;
}

QString CachableDataVector::getError() const
{
    return mError;
}

bool CachableDataVector::moveToCache()
{
    if (mpMultiCache)
    {
        // If not cached then add to cache
        if (mCacheNumBytes == 0)
        {
            if (mpMultiCache->addVector(mDataVector, mCacheStartByte, mCacheNumBytes))
            {
                mDataVector.clear();
                return true;
            }
        }
        // If data same length or shorter then replace data in cache
        else if (mDataVector.size()*sizeof(double) <= mCacheNumBytes)
        {
            if (mpMultiCache->replaceData(mCacheStartByte, mDataVector, mCacheNumBytes))
            {
                mDataVector.clear();
                return true;
            }
        }
        // Else new data is longer, add to cache and update adress and byte length
        else if (mpMultiCache->addVector(mDataVector, mCacheStartByte, mCacheNumBytes))
        {
            mDataVector.clear();
            return true;
        }
        // If we get here some error happened
        mError = mpMultiCache->getError();
        return false;
    }
    mError = "No MultiCache set";
    return false;
}

bool CachableDataVector::copyToMem()
{
    if (mpMultiCache && (mCacheNumBytes > 0))
    {
        if (mpMultiCache->copyDataTo(mCacheStartByte, mCacheNumBytes, mDataVector))
        {
            return true;
        }
        mError = mpMultiCache->getError();
        return false;
    }
    mError = "No cached data available";
    return false;
}

//CachedSingleDataVector::CachedSingleDataVector(const QVector<double> &rDataVector, const QString fileName)
//{
//    mNumElements = 0;

//    if (fileName.isEmpty())
//    {
//        mDataVector = rDataVector;
//        mNumElements = mDataVector.size();
//    }
//    else
//    {
//        if (setCacheFile(fileName))
//        {
//            writeToCache(rDataVector);
//        }
//    }
//}

//CachedSingleDataVector::~CachedSingleDataVector()
//{
//    mCacheFile.remove();
//}

//bool CachedSingleDataVector::setCacheFile(const QString fileName)
//{
//    // Prevent changing filename if file has already been created
//    if (!mCacheFile.exists())
//    {
//        mCacheFile.setFileName(fileName);
//        return true;
//    }
//    return false;
//}


//bool CachedSingleDataVector::isCached() const
//{
//    return mDataVector.isEmpty();
//}

//int CachedSingleDataVector::size() const
//{
//    return mNumElements;
//}

//bool CachedSingleDataVector::isEmpty() const
//{
//    return (mNumElements==0);
//}

//bool CachedSingleDataVector::copyData(QVector<double> &rData)
//{
//    if (isCached())
//    {
//        return readToMem(rData);
//    }
//    else
//    {
//        rData = mDataVector;
//        return true;
//    }
//}

//bool CachedSingleDataVector::replaceData(const QVector<double> &rNewData)
//{
//    if (isCached())
//    {
//        return writeToCache(rNewData);
//    }
//    else
//    {
//        mDataVector = rNewData;
//        mNumElements = mDataVector.size();
//        return true;
//    }
//}

//bool CachedSingleDataVector::peek(const int idx, double &rVal)
//{
//    if (isCached())
//    {
//        if (mCacheFile.open(QIODevice::ReadOnly))
//        {
//            bool rc = mCacheFile.seek(sizeof(double)*idx);
//            if (rc)
//            {
//                qint64 n = mCacheFile.peek((char*)&rVal, sizeof(double));
//                mCacheFile.close();
//                return true;
//            }
//            else
//            {
//                mCacheFile.close();
//                mError = mCacheFile.errorString();
//            }
//        }
//        else
//        {
//            mError = mCacheFile.errorString();
//        }
//        return false;
//    }
//    else
//    {
//        rVal = mDataVector[idx];
//        return true;
//    }
//}

//bool CachedSingleDataVector::poke(const int idx, const double val)
//{
//    if (isCached())
//    {
//        if (mCacheFile.open(QIODevice::ReadWrite))
//        {
//            bool rc = mCacheFile.seek(sizeof(double)*idx);
//            if (rc)
//            {
//                qint64 n = mCacheFile.write((const char*)&val, sizeof(double));
//                //! @todo handle writing out of bounds
//                mCacheFile.close();
//                return true;
//            }
//            else
//            {
//                mCacheFile.close();
//                mError = mCacheFile.errorString();
//            }
//        }
//        else
//        {
//            mError = mCacheFile.errorString();
//        }
//        return false;
//    }
//    else
//    {
//        mDataVector[idx] = val;
//        return true;
//    }
//}

//QVector<double> *CachedSingleDataVector::beginFullVectorOperation()
//{
//    QVector<double> *pData;
//    if(isCached())
//    {
//        pData = new QVector<double>();
//        readToMem(*pData);
//    }
//    else
//    {
//        pData = &mDataVector;
//    }
//    return pData;
//}

////! @todo for now it is dangerous (very bad) to make operations between begin end (that do not operate only on the pointer), should this be blocked or even allowed through a theird state
//bool CachedSingleDataVector::endFullVectorOperation(QVector<double> *&rpData)
//{
//    bool rc = true;
//    if(isCached())
//    {
//        rc = writeToCache(*rpData);
//        delete rpData;
//    }
//    rpData = 0;
//    return rc;
//}

//QString CachedSingleDataVector::getError() const
//{
//    return mError;
//}

//bool CachedSingleDataVector::moveToCache()
//{
//    bool rc = true;
//    if (!isCached())
//    {
//        rc = writeToCache(mDataVector);
//        mDataVector.clear();
//    }
//    return rc;
//}

//bool CachedSingleDataVector::readToMem(QVector<double> &rDataVector)
//{
//    if (mCacheFile.open(QIODevice::ReadOnly))
//    {
//        rDataVector.resize(mNumElements);
//        qint64 n = mCacheFile.read((char*)rDataVector.data(), sizeof(double)*mNumElements);
//        if (n != sizeof(double)*mNumElements)
//        {
//            mError = mCacheFile.errorString();//"Failed to read all expected data from file";
//            mCacheFile.close();
//            return false;
//        }
//        mCacheFile.close();
//        return true;
//    }
//    else
//    {
//        mError = mCacheFile.errorString();
//    }
//    return false;
//}

//bool CachedSingleDataVector::moveToMem()
//{
//    bool rc = true;
//    if (isCached())
//    {
//        rc = readToMem(mDataVector);
//        if (!rc)
//        {
//            mDataVector.clear();
//        }
//        //! @todo should we remove file ? on succfull move
//    }
//    return rc;
//}

//bool CachedSingleDataVector::writeToCache(const QVector<double> &rDataVector)
//{
//    if (mCacheFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
//    {
//        qint64 n = mCacheFile.write((const char*)rDataVector.data(), sizeof(double)*rDataVector.size());
//        if (n != sizeof(double)*rDataVector.size())
//        {
//            mError = mCacheFile.errorString();//"Failed to write data to file";
//            mCacheFile.close();
//            return false;
//        }
//        mNumElements = rDataVector.size();
//        mCacheFile.close();
//        return true;
//    }
//    else
//    {
//        mError = mCacheFile.errorString();
//    }
//    return false;
//}


//bool CachedSingleDataVector::setCached(const bool cached)
//{
//    bool rc;
//    if (cached)
//    {
//        rc = moveToCache();
//    }
//    else
//    {
//        rc = moveToMem();
//    }
//    return rc;
//}
