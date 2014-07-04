#include "socketutility.h"

#include <unistd.h>

//#include "boost/asio.hpp"

//using namespace boost::asio::ip;

//class SocketUtility_d
//{
//public:
//    boost::asio::io_service mIoService;
//    udp::endpoint mOtherEndpoint;
//    udp::socket *mpUdpSocket;
//};


//SocketUtility::SocketUtility()
//{
//    _d = new SocketUtility_d;
//}

//SocketUtility::~SocketUtility()
//{
//    delete _d;
//}

//bool SocketUtility::openSocket(const std::string &rUdpAdress, const std::string &rServicePort)
//{
//    try
//    {
//        udp::resolver resolver(_d->mIoService);
//        udp::resolver::query query(udp::v4(), rUdpAdress, rServicePort);
//        _d->mOtherEndpoint = *resolver.resolve(query);

//        _d->mpUdpSocket = new udp::socket(_d->mIoService);
//        _d->mpUdpSocket->open(udp::v4());
//        return true;
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return false;
//    }
//}

//bool SocketUtility::closeSocket()
//{
//    try
//    {
//        _d->mpUdpSocket->close();
//        delete _d->mpUdpSocket;
//        _d->mpUdpSocket = 0;
//        return true;
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return false;
//    }
//}

//size_t SocketUtility::writeSocket(const char value)
//{
//    try
//    {
//        return _d->mpUdpSocket->send_to(boost::asio::buffer((void*)&value, sizeof(char)), _d->mOtherEndpoint);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return 0;
//    }
//}

//size_t SocketUtility::writeSocket(const int value)
//{
//    try
//    {
//        return _d->mpUdpSocket->send_to(boost::asio::buffer((void*)&value, sizeof(int)), _d->mOtherEndpoint);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return 0;
//    }
//}

//size_t SocketUtility::writeSocket(const double value)
//{
//    try
//    {
//        double arr[1];
//        arr[0] = value;
//        return _d->mpUdpSocket->send_to(boost::asio::buffer(arr, sizeof(double)), _d->mOtherEndpoint);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return 0;
//    }
//}

//size_t SocketUtility::writeSocket(std::vector<double> &rData)
//{
//    try
//    {
//        return _d->mpUdpSocket->send_to(boost::asio::buffer(rData), _d->mOtherEndpoint);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return 0;
//    }
//}

//size_t SocketUtility::writeSocket(const std::string &rString)
//{
//    try
//    {
//        return _d->mpUdpSocket->send_to(boost::asio::buffer(rString), _d->mOtherEndpoint);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//        return 0;
//    }
//}

//size_t SocketUtility::readSocket(double &rValue)
//{
//    size_t len=0;
//    try
//    {
//        len = _d->mpUdpSocket->receive_from(boost::asio::buffer((void*)&rValue, sizeof(double)), _d->mOtherEndpoint);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//    }
//    return len;
//}

//size_t SocketUtility::readSocket(std::vector<double> &rData)
//{
//    size_t len=0;
//    try
//    {
//        len = _d->mpUdpSocket->receive_from(boost::asio::buffer(rData), _d->mOtherEndpoint);
//        len = len / sizeof(double);
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//    }
//    return len;
//}

//size_t SocketUtility::readSocket(std::vector<double> &rData, const size_t nElements)
//{
//    rData.resize(nElements);
//    return readSocket(rData);
//}

//size_t SocketUtility::readSocket(std::string &rString)
//{
//    size_t len=0;
//    try
//    {
//        char *pData = new char[rString.size()];
//        len = _d->mpUdpSocket->receive_from(boost::asio::buffer(pData, rString.size()*sizeof(char)), _d->mOtherEndpoint);
//        len = len / sizeof(char);
//        rString = std::string(pData, len);
//        delete pData;
//    }
//    catch( std::exception &e )
//    {
//        mErrorString = e.what();
//    }
//    return len;
//}

//size_t SocketUtility::readSocket(std::string &rString, const size_t nElements)
//{
//    rString.resize(nElements);
//    return readSocket(rString);
//}

//const std::string &SocketUtility::getErrorString()
//{
//    return mErrorString;
//}

#include "unistd.h"
#include <QtNetwork/QUdpSocket>
#include <QTime>

class SocketUtility_d
{
public:
    QHostAddress mOtherAddress;
    quint16 mOtherListenPort;
    quint16 mThisListenPort;
    QUdpSocket mUdpSocket;
    QByteArray mReadBuffer;

    qint64 sendByteArry(const QByteArray &rArr)
    {
        return mUdpSocket.writeDatagram(rArr, mOtherAddress, mOtherListenPort);
    }

    qint64 sendBytes(const char *pData, qint64 nBytes)
    {
        return mUdpSocket.writeDatagram(pData, nBytes, mOtherAddress, mOtherListenPort);
    }

    void readSocketDatagram(QByteArray &rNewDatagram)
    {
        if (mUdpSocket.hasPendingDatagrams())
        {
            qint64 dataSize = mUdpSocket.pendingDatagramSize();
            rNewDatagram.resize(dataSize);
            QHostAddress sender;
            quint16 senderPort;
            mUdpSocket.readDatagram(rNewDatagram.data(), dataSize, &sender, &senderPort);
        }
    }
};

SocketUtility::SocketUtility()
{
    _d = new SocketUtility_d;
    mSleepUS = 100;
}

SocketUtility::~SocketUtility()
{
    delete _d;
}

bool SocketUtility::openSocket(const std::string &rOtherAddress, const std::string &rOtherPort, const std::string &rThisPort)
{
    _d->mOtherAddress.setAddress(QString::fromStdString(rOtherAddress));
    _d->mOtherListenPort = QString::fromStdString(rOtherPort).toUShort();
    _d->mThisListenPort = QString::fromStdString(rThisPort).toUShort();
    _d->mReadBuffer.reserve(10000);
    return _d->mUdpSocket.bind(QHostAddress::Any, _d->mThisListenPort);
}

void SocketUtility::closeSocket()
{
    _d->mUdpSocket.close();
    _d->mReadBuffer.clear();;
}

size_t SocketUtility::writeSocket(const char value)
{
    long int nSent = sendBytes((char *)&value, sizeof(char));
    if ( (nSent < 0) || (size_t(nSent) < sizeof(char)) )
    {
        mErrorString = "Error sending char: %1, %2";//.arg(value).arg(_d->mUdpSocket.errorString());
    }
    return nSent;
}

size_t SocketUtility::writeSocket(const int value)
{
    long int nSent = sendBytes((char *)&value, sizeof(int));
    if ( (nSent < 0) || (size_t(nSent) < sizeof(int)) )
    {
        mErrorString = "Error sending int: %1, %2";//.arg(value).arg(_d->mUdpSocket.errorString());
    }
    return nSent;
}

size_t SocketUtility::writeSocket(const double value)
{
    long int nSent = sendBytes((char *)&value, sizeof(double));
    if ( (nSent < 0) || (size_t(nSent) < sizeof(double)) )
    {
        mErrorString = "Error sending double: %1, %2";//.arg(value).arg(_d->mUdpSocket.errorString());
    }
    return nSent;
}

size_t SocketUtility::writeSocket(std::vector<double> &rData)
{
    long int nSent=0;
    foreach(double v, rData)
    {
        nSent += writeSocket(v);
    }

//    if (nSent != sizeof(double)*rData.size())
//    {

//    }

    return nSent;
}

size_t SocketUtility::writeSocket(const std::string &rString)
{
    long int nSent = sendBytes(rString.c_str(), sizeof(char)*rString.size());
    if ( (nSent < 0) || (size_t(nSent) < sizeof(char)*rString.size()) )
    {
        mErrorString = "Error sending char: %1, %2";//.arg(value).arg(_d->mUdpSocket.errorString());
    }
    return nSent;

//    size_t nSent=0;
//    for(size_t i=0; i<rString.size(); ++i)
//    {
//        nSent += writeSocket(rString[i]);
//    }
//    return nSent;
}

bool SocketUtility::readSocket(char &rValue)
{
    return readBytes(&rValue, sizeof(char)) == sizeof(char);
}

bool SocketUtility::readSocket(char &rValue, const int timeoutMS)
{
    QTime timer;
    timer.start();
    size_t nPending = numWaitingBytes();
    while ( (nPending < sizeof(char)) && (timer.elapsed() < timeoutMS))
    {
        if (mSleepUS > 0)
        {
            usleep(mSleepUS);
        }
        nPending = numWaitingBytes();
    }
    return readSocket(rValue);
}


bool SocketUtility::readSocket(double &rValue)
{
    return readBytes((char*)&rValue, sizeof(double)) == sizeof(double);
}

bool SocketUtility::readSocket(double &rValue, const int timeoutMS)
{
    QTime timer;
    timer.start();
    size_t nPending = numWaitingBytes();
    while ( (nPending < sizeof(double)) && (timer.elapsed() < timeoutMS))
    {
        if (mSleepUS > 0)
        {
            usleep(mSleepUS);
        }
        nPending = numWaitingBytes();
    }
    return readSocket(rValue);
}


bool SocketUtility::readSocket(std::vector<double> &rData, const size_t nElements)
{
    rData.clear();
    rData.reserve(nElements);
    while (rData.size() < nElements)
    {
        double val;
        bool isOK = readSocket(val, 100);
        if (isOK)
        {
            rData.push_back(val);
        }
        else
        {
            break;
        }
    }
    return rData.size();
}

bool SocketUtility::readSocket(std::string &rString, const size_t nElements, const int timeoutMS)
{
    QTime timer;
    timer.start();
    size_t nPending = numWaitingBytes();
    while ( (nPending < nElements) && (timer.elapsed() < timeoutMS))
    {
        if (mSleepUS > 0)
        {
            usleep(mSleepUS);
        }
        poll(); // We need to poll here to make sure new datagrams are processed (since we don read in a separate thread)
        nPending = numWaitingBytes();
    }
    char *pBuffer = new char(nElements);
    size_t nBytes = readBytes(pBuffer, nElements);
    rString = std::string(pBuffer, nBytes);
    delete pBuffer;
    return nElements == rString.size();
}

size_t SocketUtility::poll()
{
    QByteArray newDatagram;
    _d->readSocketDatagram(newDatagram);
    _d->mReadBuffer.append(newDatagram);
    return size_t(newDatagram.size());
}

long int SocketUtility::sendBytes(const char *pData, size_t nBytes)
{
    return _d->mUdpSocket.writeDatagram(pData, nBytes, _d->mOtherAddress, _d->mOtherListenPort);
}

const std::string &SocketUtility::getErrorString()
{
    return mErrorString;
}


size_t SocketUtility::numWaitingBytes() const
{
    int num = _d->mReadBuffer.size();
    if (_d->mUdpSocket.hasPendingDatagrams())
    {
        num += _d->mUdpSocket.pendingDatagramSize();
    }
    return  size_t(num);
}

void SocketUtility::setSleepUS(int us)
{
    mSleepUS = us;
}

size_t SocketUtility::readBytes(char *pTargetBuffer, size_t nBytes)
{
    // Check if new datagram waiting in socket, if so, read it
    if (_d->mUdpSocket.hasPendingDatagrams() )
    {
        QByteArray newDatagram;
        _d->readSocketDatagram(newDatagram);

        // If read buffer is empty and new datagram is equal size as the requested data, then copy directly without pushing to read buffer
        if (_d->mReadBuffer.isEmpty() && newDatagram.size() == int(nBytes))
        {
            memcpy(pTargetBuffer, newDatagram.constData(), size_t(nBytes));
            return nBytes;
        }
        // If read buffer is empty and new datagram is larger then requested data, then copy directly and push the rest to the read buffer
        else if (_d->mReadBuffer.isEmpty() && newDatagram.size() > int(nBytes))
        {
            memcpy(pTargetBuffer, newDatagram.constData(), size_t(nBytes));
            newDatagram.remove(0, nBytes);
            _d->mReadBuffer.append(newDatagram);
            return nBytes;
        }
        // Else push datagram to read buffer, for later processing
        else
        {
            _d->mReadBuffer.append(newDatagram);
        }
    }

    // Read from read buffer if it contains exactly the data
    if (_d->mReadBuffer.size() == int(nBytes))
    {
        memcpy(pTargetBuffer, _d->mReadBuffer.constData(), nBytes);
        _d->mReadBuffer.clear();
        return nBytes;
    }
    // Read from read buffer if it contains more than the data
    else if (_d->mReadBuffer.size() > int(nBytes))
    {
        memcpy(pTargetBuffer, _d->mReadBuffer.constData(), nBytes);
        _d->mReadBuffer.remove(0, nBytes);
        return nBytes;
    }

    // If not enough bytes are waiting then we do not read anything
    return 0;
}

//#include "socketutility.moc"
