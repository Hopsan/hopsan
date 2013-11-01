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
//! @file   CachableDataVector.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-02-12
//!
//! @brief Contains classes for caching data vectors to disk
//!
//$Id: LogVariable.cpp 4974 2013-02-01 08:47:48Z petno25 $

#ifndef CACHABLEDATAVECTOR_H
#define CACHABLEDATAVECTOR_H

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QSharedPointer>
#include <QVector>
#include <QMap>
#include <QTextStream>

////! @todo this could be a template
//class CachedSingleDataVector
//{
//public:
//    CachedSingleDataVector(const QVector<double> &rDataVector, const QString fileName=QString());
//    ~CachedSingleDataVector();

//    bool setCacheFile(const QString fileName);
//    bool setCached(const bool cached);
//    bool isCached() const;

//    int size() const;
//    bool isEmpty() const;

//    bool copyData(QVector<double> &rData);
//    bool replaceData(const QVector<double> &rNewData);
//    bool peek(const int idx, double &rVal);
//    bool poke(const int idx, const double val);

//    QVector<double> *beginFullVectorOperation();
//    bool endFullVectorOperation(QVector<double> *&rpData);

//    QString getError() const;

//private:
//    bool writeToCache(const QVector<double> &rDataVector);
//    bool moveToCache();
//    bool readToMem(QVector<double> &rDataVector);
//    bool moveToMem();

//    QString mError;
//    QFile mCacheFile;
//    QVector<double> mDataVector;
//    int mNumElements;
//};


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

    QString getError() const;
    QFileInfo getCacheFileInfo() const;

    void incrementSubscribers();
    void decrementSubscribers();
    qint64 getNumSubscribers() const;

protected:
    typedef struct CheckoutInfo{
        CheckoutInfo(quint64 sb, quint64 nb) {startByte = sb; nBytes=nb;}
        quint64 startByte;
        quint64 nBytes;
    }CheckoutInfoT;

    bool writeInCache(const quint64 startByte, const QVector<double> &rDataVector, quint64 &rBytesWriten);
    bool appendToCache(const QVector<double> &rDataVector, quint64 &rStartByte, quint64 &rNumBytes);
    bool readToMem(const quint64 startByte, const quint64 nBytes, QVector<double> *pDataVector);
    bool smartOpenFile(QIODevice::OpenMode flags);
    void smartCloseFile();
    void removeCacheFile();

    QMap<QVector<double> *, CheckoutInfoT> mCheckoutMap;
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

    QString getError() const;

private:
    bool moveToCache();
    bool copyToMem();

    QString mError;
    SharedMultiDataVectorCacheT mpMultiCache;
    QVector<double> mDataVector;
    quint64 mCacheStartByte;
    quint64 mCacheNumBytes;
    bool mIsCached;
};

#endif // CACHABLEDATAVECTOR_H
