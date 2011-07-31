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
//! @file   GUIUtilities.cpp
//! @author All <flumes@liu.se>
//! @date   2010-10-09
//!
//! @brief Contains a class for misc utilities
//!
//$Id$

#include <qmath.h>
#include <QPoint>
#include <QDir>
#include <QDebug>
#include <QStringList>
#include <limits>
#include <math.h>
#include <complex>

#include "GUIUtilities.h"


using namespace std;

//! @brief This function extracts the name from a text stream
//! @return The extracted name without quotes, empty string if failed
//! It is assumed that the name was saved OK. but error indicated by empty string
QString readName(QTextStream &rTextStream)
{
    QString tempName;
    rTextStream >> tempName;
    if (tempName.startsWith("\""))
    {
        while (!tempName.endsWith("\""))
        {
            if (rTextStream.atEnd())
            {
                return QString(""); //Empty string (failed)
            }
            else
            {
                QString tmpstr;
                rTextStream >> tmpstr;
                tempName.append(" " + tmpstr);
            }
        }
        return tempName.remove("\"").trimmed(); //Remove quotes and trimm (just to be sure)
    }
    else
    {
        return QString(""); //Empty string (failed)
    }
}


//! @brief Convenience function if you dont have a stream to read from
//! @return The extracted name without quotes, empty string if failed
//! It is assumed that the name was saved OK. but error indicated by empty string
QString readName(QString namestring)
{
//    if (namestring.endsWith("\""))
//    {
//        namestring.chop(1);
//    }

//    if (namestring.startsWith("\""))
//    {
//        namestring.remove(1,0);
//    }

    QTextStream namestream(&namestring);
    return readName(namestream);
}


//! @brief This function may be used to add quotes around string, usefull for saving names. Ex: "string"
QString addQuotes(QString str)
{
    str.prepend("\"");
    str.append("\"");
    return str;
}

////! @brief This function returns the relative absolute path
////! @param [in] pathtochange The absolute path that you want to change
////! @param [in] basedir The absolute directory path of the base directory
////! @returns The realtive pathtochange, relative to basepath
//QString relativePath(QString pathtochange, QDir basedir)
//{
//    return basedir.relativeFilePath(pathtochange);
//}

//! @brief This function returns the relative absolute path
//! @param [in] pathtochange The absolute path that you want to change
//! @param [in] basedir The absolute directory path of the base directory
//! @returns The realtive pathtochange, relative to basepath
QString relativePath(QFileInfo pathtochange, QDir basedir)
{
    if (!pathtochange.isAbsolute())
    {
        qDebug() << "pathtochange is not absolute in relativePath utility function, need to handle this nicer";
        return "";
    }
    return basedir.relativeFilePath(pathtochange.absoluteFilePath());
}

//! @brief Utility function to convert degrees to radians
qreal deg2rad(qreal deg)
{
    return deg*M_PI/180.0;
}

//! @brief Utility function to convert degrees to radians
qreal rad2deg(qreal rad)
{
    return rad*180.0/M_PI;
}

//! @brief normalises degrees to range between -180 and 180 degrees
qreal normDeg180(qreal deg)
{
    return rad2deg(normRad(deg2rad(deg)));
}

//! @brief normalises degrees to range between 0 and 360 degrees
qreal normDeg360(qreal deg)
{
    while (deg > 360.0 || deg < 0.0)
    {
        if (deg > 360.0)
        {
            deg -= 360.0;
        }
        else
        {
            deg += 360.0;
        }
    }
    return deg;
}

//! @brief normalises radinas to range between -PI and PI degrees
qreal normRad(qreal rad)
{
    return qAtan2(qCos(rad),qSin(rad));
}


//! @brief Calculates the distance between two points
double dist(double x1,double y1, double x2, double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}

//! @brief Calculates the 2NORM of one point, the absoulte distance from 0,0
qreal dist(QPointF &rPoint)
{
    return sqrt( rPoint.x()*rPoint.x() + rPoint.y()*rPoint.y() );
}

//! @brief Calculates the distance between two points
qreal dist(QPointF &rPoint1, QPointF &rPoint2)
{
    qreal x = rPoint1.x() - rPoint2.x();
    qreal y = rPoint1.y() - rPoint2.y();
    return sqrt( x*x + y*y );
}



QString parseVariableDescription(QString input)
{
    QString retval;
    input.replace("alpha",      QObject::trUtf8("α"), Qt::CaseInsensitive);
    input.replace("beta",       QObject::trUtf8("β"), Qt::CaseInsensitive);
    input.replace("delta",      QObject::trUtf8("δ"), Qt::CaseInsensitive);
    input.replace("epsilon",    QObject::trUtf8("ε"), Qt::CaseInsensitive);
    input.replace("kappa",      QObject::trUtf8("κ"), Qt::CaseInsensitive);
    input.replace("tao",        QObject::trUtf8("τ"), Qt::CaseInsensitive);
    input.replace("omega",      QObject::trUtf8("ω"), Qt::CaseInsensitive);
    input.replace("eta",        QObject::trUtf8("η"), Qt::CaseInsensitive);
    input.replace("rho",        QObject::trUtf8("ρ"), Qt::CaseInsensitive);
    input.replace("signa",      QObject::trUtf8("σ"), Qt::CaseInsensitive);

    if(input.count("_") == 1 && !input.contains("^"))
    {
        retval.append(input.section('_',0,0));
        retval.append("<sub>");
        retval.append(input.section('_',1,1));
        retval.append("</sub>");
    }
    else if(input.count("^") == 1 && !input.contains("_"))
    {
        retval.append(input.section('^',0,0));
        retval.append("<sup>");
        retval.append(input.section('^',1,1));
        retval.append("</sup>");
    }
    else
    {
        retval = input;
    }

    return retval;
}


QString parseVariableUnit(QString input)
{
    if(!input.startsWith("[") || (!input.endsWith("]")))        //If no square brackets, we don't know what to do, so do nothing
    {
        return input;
    }
    input = input.mid(1, input.size()-2);              //Remove brackets from input
    QString retval = input;
    int idx = 0;
    int tempidx;

    while(true)
    {
        idx=retval.indexOf("^", idx);
        if(idx==-1) break;
        retval.remove(idx,1);
        retval.insert(idx, "<sup>");

        tempidx=std::numeric_limits<int>::max();
        if(retval.contains("*"))
            tempidx=min(tempidx, retval.indexOf("*", idx));
        if(retval.contains("/"))
            tempidx=min(tempidx, retval.indexOf("/", idx));
        if(retval.contains("+"))
            tempidx=min(tempidx, retval.indexOf("+", idx));
        if(retval.contains("-"))
            tempidx=min(tempidx, retval.indexOf("-", idx));
        if(tempidx == -1 || tempidx > retval.size()) tempidx = retval.size();
        idx=tempidx;

        retval.insert(idx, "</sup>");
    }

    retval.prepend("[");        //Add brackets to output
    retval.append("]");

    return retval;
}


//! @brief Converts a vector of real numbers to a vector of complex numbers with i=0
//! @param realVector Vector with real numbers
//! @returns Vector with complex numbers
QVector< complex<double> > realToComplex(QVector<double> realVector)
{
    QVector< complex<double> > complexVector;
    for(int i=0; i<realVector.size(); ++i)
    {
        complexVector.append(std::complex<double>(realVector[i], 0));
    }
    return complexVector;
}



//! @brief Forward fast fourier transform
//! Transforms given vector into its fourier transform.
//! Even elements in the vector represents real numbers, and odd imaginary numbers.
//! Original source:
//! V. Myrnyy, A Simple and Efficient FFT Implementation in C++, Dr.Dobbs, 2007
//! http://drdobbs.com/cpp/199500857
//! @param data Vector with data
void FFT(QVector< complex<double> > &data)
{
    unsigned long n = data.size();       // n = data.size()

    qDebug() << "FFT of vector with size " << n;

    unsigned long mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    // Reverse-binary reindexing
    j=1;
    for (i=0; i<n; ++i)
    {
        if (j>i)
        {
            swap(data[j-1].real(), data[i].real());     //Even numbers
            swap(data[j-1].imag(), data[i].imag());         //Odd numbers
        }
        m = n>>1;
        while (m>=2 && j>m)
        {
            j -= m;
            m >>= 1;    //m = m/2
        }
        j += m;
    };

    // Here begins the Danielson-Lanczos section
    mmax=1;
    while (n>mmax)
    {
        istep = mmax<<1;
        theta = -(M_PI/mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m=0; m < mmax; m++)
        {
            for (i=m; i < n; i += istep)
            {
                j=i+mmax;
                tempr = wr*data[j].real() - wi*data[j].imag();
                tempi = wr*data[j].imag() + wi*data[j].real();

                data[j].real() = data[i].real() - tempr;
                data[j].imag() = data[i].imag() - tempi;
                data[i].real() += tempr;
                data[i].imag() += tempi;
            }
            wtemp=wr;
            wr += wr*wpr - wi*wpi;
            wi += wi*wpr + wtemp*wpi;
        }
        mmax=istep;
    }
    qDebug () << "FFT successful!";
}


//! @brief Reduces number of log samples of a data vector to specified value
//! @param vector Reference to vector that will be reduced
//! @param newSize New size of vector
void reduceVectorSize(QVector<double> &vector, int newSize)
{
    int oldSize = vector.size();

    QVector<double> tempVector;

    for(int i=0; i<newSize; ++i)
    {
        tempVector.append(vector.at(oldSize/newSize*i));
    }
}
