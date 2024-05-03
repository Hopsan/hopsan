/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   CachableDataVector.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-02-12
//!
//! @brief Contains classes for caching data vectors to disk
//!
//$Id$

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
    //! @todo maybe have register for how long added data at each start address was, to prevent destroying data
    return appendToCache(rDataVector, rStartByte, rNumBytes);
}

bool MultiDataVectorCache::beginMultiAppend()
{
    // We need to open in read write mode to make it possible to read earlier data from file while new data is appended
    mIsMultiAppending = smartOpenFile(QIODevice::ReadWrite | QIODevice::Append);
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
        // In case someone has moved the pointer by reading, lets us seek to 'end of file' instead
        mCacheFile.seek( mCacheFile.size() );
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
        // If the file was already open, then we must check if the desired flags are already set
        if ( (mCacheFile.openMode() & flags) != 0 )
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
        mCheckoutMap.insert(pData, CheckoutInfo(startByte,nBytes));
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
        CheckoutInfo info = mCheckoutMap.value(rpData, CheckoutInfo(0,0));
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

bool MultiDataVectorCache::hasError() const
{
    return !mError.isEmpty();
}


QString MultiDataVectorCache::getError() const
{
    return mError+"; File: "+getCacheFileInfo().absoluteFilePath();
}

QString MultiDataVectorCache::getAndClearError()
{
    QString error = getError();
    mError.clear();
    return error;
}

QFileInfo MultiDataVectorCache::getCacheFileInfo() const
{
    return QFileInfo(mCacheFile);
}

qint64 MultiDataVectorCache::getCacheSize() const
{
    return mCacheFile.size();
}

void MultiDataVectorCache::incrementSubscribers()
{
    ++mNumSubscribers;
}

//! @brief Decrement number of subscribers, if no subscribers remain the cache file will be deleted (even if this object still remains).
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
    // We can not rely on checking size of mDataVector (may be empty but non-cached) or the mCacheNumBytes (will still have a value after moving from cache to memory temporarily)
    mIsCached = false;

    if (pMultiCache)
    {
        mpMultiCache = pMultiCache;
        mpMultiCache->incrementSubscribers();
    }

    if (mpMultiCache && cached)
    {
        // We add directly to cache here instead of first copying to memory and then moving to cache (saves some time)
        if (mpMultiCache->addVector(rDataVector,mCacheStartByte, mCacheNumBytes))
        {
            mIsCached = true;
        }
        else
        {
            mWarning = "Failed to cache log data on disk, falling back to RAM storage: "+mpMultiCache->getError();
            mDataVector = rDataVector;
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

void CachableDataVector::switchCacheFile(SharedMultiDataVectorCacheT pMultiCache)
{
    if (mpMultiCache != pMultiCache)
    {
        bool wasCached = isCached();
        if (wasCached)
        {
            copyToMem();
        }

        mCacheNumBytes = 0;
        mCacheStartByte = 0;
        mIsCached = false;

        // Decrement old cache subscribers
        if (mpMultiCache)
        {
            mpMultiCache->decrementSubscribers();
        }

        // Set new cache file, and increment subscribers if pointer is not NULL
        mpMultiCache = pMultiCache;
        if (mpMultiCache)
        {
            mpMultiCache->incrementSubscribers();

            // Move to cache if we were previously cached
            //! @todo maybe we should always move to cache
            if (wasCached)
            {
                moveToCache();
            }
        }
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

bool CachableDataVector::hasWarning() const
{
    return !mWarning.isEmpty();
}

QString CachableDataVector::getWarning() const
{
    return mWarning;
}

QString CachableDataVector::getAndClearWarning()
{
    QString error = mWarning;
    mWarning.clear();
    return error;
}

bool CachableDataVector::hasError() const
{
    return !mError.isEmpty();
}

QString CachableDataVector::getError() const
{
    return mError;
}

QString CachableDataVector::getAndClearError()
{
    QString error = mError;
    mError.clear();
    return error;
}

bool CachableDataVector::moveToCache()
{
    if (mpMultiCache)
    {
        // If not cached then add to cache
        if (mCacheNumBytes == 0)
        {
            // If we do not have data, then just tag, that we want to sue cache
            if (mDataVector.isEmpty())
            {
                mIsCached = true;
                return true;
            }
            // If we have data, then try to add it to the cache
            else if (mpMultiCache->addVector(mDataVector, mCacheStartByte, mCacheNumBytes))
            {
                mDataVector.clear();
                mIsCached = true;
                return true;
            }
        }
        // If data same length or shorter then replace data in cache
        else if (mDataVector.size()*sizeof(double) <= mCacheNumBytes)
        {
            if (mpMultiCache->replaceData(mCacheStartByte, mDataVector, mCacheNumBytes))
            {
                mDataVector.clear();
                mIsCached = true;
                return true;
            }
        }
        // Else new data is longer, add to cache and update address and byte length
        else if (mpMultiCache->addVector(mDataVector, mCacheStartByte, mCacheNumBytes))
        {
            mDataVector.clear();
            mIsCached = true;
            return true;
        }
        // If we get here some error happened
        mError = mpMultiCache->getError();
        mIsCached = false;
        return false;
    }
    mError = "No MultiCache set";
    mIsCached = false;
    return false;
}

bool CachableDataVector::copyToMem()
{
    if (mpMultiCache && (mCacheNumBytes > 0))
    {
        if (mpMultiCache->copyDataTo(mCacheStartByte, mCacheNumBytes, mDataVector))
        {
            mIsCached = false;
            return true;
        }
        mError = mpMultiCache->getError();
        return false;
    }
    mError = "No cached data available";
    return false;
}
