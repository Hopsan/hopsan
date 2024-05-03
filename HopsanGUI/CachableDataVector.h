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
//! @file   CachableDataVector.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-02-12
//!
//! @brief Contains classes for caching data vectors to disk
//!
//$Id$

#ifndef CACHABLEDATAVECTOR_H
#define CACHABLEDATAVECTOR_H

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QSharedPointer>
#include <QVector>
#include <QMap>
#include <QTextStream>

//! @todo this could be a template
class MultiDataVectorCache
{
public:
    MultiDataVectorCache(const QString fileName);
    ~MultiDataVectorCache();
    bool addVector(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes);
    bool beginMultiAppend();
    void endMultiAppend();

    bool copyDataTo(const quint64 startByte, const quint64 nBytes, QVector<double> &rData);
    bool replaceData(const quint64 startByte, const QVector<double> &rNewData, quint64 &rNumBytes);
    bool peek(const quint64 byte, double &rVal);
    bool poke(const quint64 byte, const double val);

    bool checkoutVector(const quint64 startByte, const quint64 nBytes, QVector<double> *&rpData);
    bool returnVector(QVector<double> *&rpData);

    bool hasError() const;
    QString getError() const;
    QString getAndClearError();
    QFileInfo getCacheFileInfo() const;
    qint64 getCacheSize() const;

    void incrementSubscribers();
    void decrementSubscribers();
    qint64 getNumSubscribers() const;

protected:
    struct CheckoutInfo
    {
        CheckoutInfo(quint64 sb, quint64 nb) : startByte(sb), nBytes(nb) {}
        quint64 startByte;
        quint64 nBytes;
    };

    bool writeInCache(const quint64 startByte, const QVector<double> &rDataVector, quint64 &rBytesWriten);
    bool appendToCache(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes);
    bool readToMem(const quint64 startByte, const quint64 nBytes, QVector<double> *pDataVector);
    bool smartOpenFile(QIODevice::OpenMode flags);
    void smartCloseFile();
    void removeCacheFile();

    QMap<QVector<double> *, CheckoutInfo> mCheckoutMap;
    qint64 mNumSubscribers;
    QFile mCacheFile;
    QString mError;
    bool mIsMultiAppending;
    bool mIsMultiReadWriting;
    bool mIsMultiReading;
};
typedef QSharedPointer<MultiDataVectorCache> SharedMultiDataVectorCacheT;

class CachableDataVector
{
public:
    CachableDataVector(const QVector<double> &rDataVector, SharedMultiDataVectorCacheT pMultiCache, const bool cached=true);
    ~CachableDataVector();

    void switchCacheFile(SharedMultiDataVectorCacheT pMultiCache);

    bool setCached(const bool cached);
    bool isCached() const;

    int size() const;
    bool isEmpty() const;

    bool streamDataTo(QTextStream &rTextStream, const QString separator);
    bool copyDataTo(QVector<double> &rData);
    bool replaceData(const QVector<double> &rNewData);
    bool peek(const int idx, double &rVal);
    bool poke(const int idx, const double val);

    QVector<double> *beginFullVectorOperation();
    bool endFullVectorOperation(QVector<double> *&rpData);

    bool hasWarning() const;
    QString getWarning() const;
    QString getAndClearWarning();

    bool hasError() const;
    QString getError() const;
    QString getAndClearError();

private:
    bool moveToCache();
    bool copyToMem();

    QString mWarning;
    QString mError;
    SharedMultiDataVectorCacheT mpMultiCache;
    QVector<double> mDataVector;
    quint64 mCacheStartByte;
    quint64 mCacheNumBytes;
    bool mIsCached;
};

#endif // CACHABLEDATAVECTOR_H
