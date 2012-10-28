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
#ifdef WIN32
#include <windows.h>
#endif

#include "GUIUtilities.h"
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include "common.h"
#include "Configuration.h"
#include "Widgets/LibraryWidget.h"

using namespace std;

const double DBLMAX = std::numeric_limits<double>::max();

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

//! @brief Strips leading and trailing spaces from a string
void stripLTSpaces(QString &rString)
{
    while (rString.startsWith(' '))
    {
        rString.remove(0,1);
    }
    while (rString.endsWith(' '))
    {
        rString.chop(1);
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
qreal deg2rad(const qreal deg)
{
    return deg*M_PI/180.0;
}

//! @brief Utility function to convert degrees to radians
qreal rad2deg(const qreal rad)
{
    return rad*180.0/M_PI;
}

//! @brief normalises degrees to range between -180 and 180 degrees
qreal normDeg180(const qreal deg)
{
    return rad2deg(normRad(deg2rad(deg)));
}

//! @brief normalises degrees to range between 0 and 359.999 degrees, 360.0 will be converted to 0
//! @todo potential danger comparing floats like this
qreal normDeg360(qreal deg)
{
    while (deg < 0.0)
    {
        deg += 360.0;
    }

    //If 360 (or very close) then set to zero
    while (deg >= 359.999)
    {
        deg -= 360.0;
    }

    return fabs(deg); //Make sure we return positive (close to 0 if we have rounding issus that takes us bellow zero)
}

//! @brief normalises radinas to range between -PI and PI degrees
qreal normRad(const qreal rad)
{
    return qAtan2(qCos(rad),qSin(rad));
}


//! @brief Calculates the distance between two points
double dist(const double x1, const double y1, const double x2, const double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}

//! @brief Compare two qreals with given tolerance
//! @return True if fabs(first-last) < eps
bool fuzzyEqual(const qreal first, const qreal second, const qreal eps)
{
    return (fabs(first-second) < eps);
}

//! @brief Check if first+eps is less then second, convenient to make usre float comparison works on all platforms
//! @return True if first+eps < second
bool fuzzyLT(const qreal first, const qreal second, const qreal eps)
{
    return (first+eps) < second;
}

//! @brief Calculates the 2NORM of one point, the absoulte distance from 0,0
qreal dist(const QPointF &rPoint)
{
    return sqrt( rPoint.x()*rPoint.x() + rPoint.y()*rPoint.y() );
}

//! @brief Calculates the distance between two points
qreal dist(const QPointF &rPoint1, const QPointF &rPoint2)
{
    qreal x = rPoint1.x() - rPoint2.x();
    qreal y = rPoint1.y() - rPoint2.y();
    return sqrt( x*x + y*y );
}


//! @brief Replaces a word in a string, but only if it is not part of a longer word.
//! @param string Reference to string that shall be modified
//! @param before The old word that will be replaced (if it exists)
//! @param after The new word that will replace the old one
void replaceWord(QString &string, QString before, QString after)
{
    if(string.contains(before))
    {
        if(!(string.indexOf(before) > 0 && string.at(string.indexOf(before)-1).isLetter()) &&
           !(string.indexOf(before) < string.size()-before.size() && string.at(string.indexOf(before)+before.size()).isLetter()))
        {
            string.replace(before, after);
        }
    }
}


//! @brief Parses special symbols, subscripts and superscript in parameter names
//! Names of greek letters will be transformed into greek letters.
//! All text after '^' will be superscript.
//! All text after '_' will be subscript.
//! Superscripts and subscripts always cancels each other.
//! @param input String with parameter name to parse
QString parseVariableDescription(QString input)
{
    QString retval;

    replaceWord(input,  "Gamma",        QObject::trUtf8("Γ"));
    replaceWord(input,  "Delta",        QObject::trUtf8("Δ"));
    replaceWord(input,  "Theta",        QObject::trUtf8("Θ"));
    replaceWord(input,  "Lambda",       QObject::trUtf8("Λ"));
    replaceWord(input,  "Xi",           QObject::trUtf8("Ξ"));
    replaceWord(input,  "Pi",           QObject::trUtf8("Π"));
    replaceWord(input,  "Sigma",        QObject::trUtf8("Σ"));
    replaceWord(input,  "Phi",          QObject::trUtf8("Φ"));
    replaceWord(input,  "Psi",          QObject::trUtf8("Ψ"));
    replaceWord(input,  "Omega",        QObject::trUtf8("Ω"));

    replaceWord(input,  "alpha",        QObject::trUtf8("α"));
    replaceWord(input,  "beta",         QObject::trUtf8("β"));
    replaceWord(input,  "gamma",        QObject::trUtf8("γ"));
    replaceWord(input,  "delta",        QObject::trUtf8("δ"));
    replaceWord(input,  "epsilon",      QObject::trUtf8("ε"));
    replaceWord(input,  "zeta",         QObject::trUtf8("ζ"));
    replaceWord(input,  "eta",          QObject::trUtf8("η"));
    replaceWord(input,  "theta",        QObject::trUtf8("θ"));
    replaceWord(input,  "lota",         QObject::trUtf8("ι"));
    replaceWord(input,  "kappa",        QObject::trUtf8("κ"));
    replaceWord(input,  "lambda",       QObject::trUtf8("λ"));
    replaceWord(input,  "mu",           QObject::trUtf8("μ"));
    replaceWord(input,  "nu",           QObject::trUtf8("ν"));
    replaceWord(input,  "xi",           QObject::trUtf8("ξ"));
    replaceWord(input,  "omicron",      QObject::trUtf8("ο"));
    replaceWord(input,  "pi",           QObject::trUtf8("π"));
    replaceWord(input,  "rho",          QObject::trUtf8("ρ"));
    replaceWord(input,  "finalsigma",   QObject::trUtf8("ς"));
    replaceWord(input,  "sigma",        QObject::trUtf8("σ"));
    replaceWord(input,  "tao",          QObject::trUtf8("τ"));
    replaceWord(input,  "upsilon",      QObject::trUtf8("υ"));
    replaceWord(input,  "phi",          QObject::trUtf8("φ"));
    replaceWord(input,  "chi",          QObject::trUtf8("χ"));
    replaceWord(input,  "psi",          QObject::trUtf8("ψ"));
    replaceWord(input,  "omega",        QObject::trUtf8("ω"));

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


//! @brief Parses a unit string with superscripts.
//! @param input String with unit name to parse
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
        idx=retval.indexOf("^", idx);       //Find next '^' symbol.
        if(idx==-1) break;                  //If no symbol, we are finished.
        retval.remove(idx,1);               //Remove the symbol
        retval.insert(idx, "<sup>");        //Begin superscript

        tempidx=std::numeric_limits<int>::max();        //Find next arithmetic symbol, and cancel subscript there, or at end if no symbols found.
        if(retval.contains("*"))
            tempidx=min(tempidx, retval.indexOf("*", idx));
        if(retval.contains("/"))
            tempidx=min(tempidx, retval.indexOf("/", idx));
        if(retval.contains("+"))
            tempidx=min(tempidx, retval.indexOf("+", idx));
        if(retval.contains("-"))
            tempidx=min(tempidx, retval.indexOf("-", idx));
        if(tempidx == -1 || tempidx > retval.size()) tempidx = retval.size();   // Will this ever happen?!
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
    unsigned long n = data.size();

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

    vector = tempVector;
}


void removeDir(QString path)
{
    QDir dir;
    dir.setPath(path);
    Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            removeDir(info.absoluteFilePath());
        }
        else
        {
#ifdef WIN32
            QStringList s;
            s << "del "+info.absoluteFilePath();
            QProcess browser;
            browser.start("cmd", s);
#else
            QFile::remove(info.absoluteFilePath());
#endif
        }
    }
    dir.rmdir(path);
}

//! @brief Copy a directory with contents
//! @param [in] fromPath The absolute path to the directory to copy
//! @param [in] toPath The absolute path to the destination (including resulting dir name)
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
void copyDir(const QString fromPath, QString toPath)
{
    QDir toDir(toPath);
    toDir.mkpath(toPath);
    if (toPath.endsWith('/'))
    {
        toPath.chop(1);
    }

    QDir fromDir(fromPath);
    foreach(QFileInfo info, fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            copyDir(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
        else
        {
            QFile::copy(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
    }
}

//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
void copyIncludeFilesToDir(QString path)
{    
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include");
    saveDir.cd("include");

    copyDir( QString(COREINCLUDEPATH), saveDir.path() );
}


//Optimization


//! @brief Reflects the worst point through the centroid of the remaining points
//! @param vector Vector with points
//! @param worst Index of worst vector
//! @param alpha Reflection coefficient
void reflectWorst(QVector< QVector<double> > &vector, int worst, double alpha)
{
    int n = vector.size();              //Number of points
    int k = vector.first().size();      //Number of parameters
    QVector<double> x_w = vector.at(worst);
    QVector<double> x_c;

    for(int i=0; i<k; ++i)
    {
        x_c.append(1.0/(n-1)*(sum(vector,i)-x_w.at(i)));
    }

    QVector<double> x_new;
    for(int i=0; i<k; ++i)
    {
        x_new.append(x_c.at(i)+alpha*(x_c.at(i)-x_w.at(i)));
    }
    vector.replace(worst, x_new);
}


//! @brief Sums the number with index i in each vector (in a vector of vectors)
//! @param vector Vector to sum elements from
//! @param i Index to sum
double sum(QVector< QVector<double> > vector, int i)
{
    double retval = 0;
    for(int j=0; j<vector.size(); ++j)
    {
        retval += vector.at(j).at(i);
    }
    return retval;
}


//! @brief Returns the first time for which the data vector has reached specified value
//! @param vData Data vector
//! @param vTime Time vector
//! @param value Value to look for
double firstTimeAt(QVector<double> vData, QVector<double> vTime, double value)
{
    return vTime.at(vData.indexOf(value));
}


//! @brief Generates a normal distributed random value
//! Using the BoxâMuller transform
//! @param average Average value of normal distribution
//! @param sigma Standard deviation of normal distribution
double normalDistribution(double average, double sigma)
{
    double U1 = (double)rand() / (double)RAND_MAX;
    double U2 = (double)rand() / (double)RAND_MAX;
    return average + sigma*sqrt(-2*log(U1))*cos(2*3.1415926*U2);
}

// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}

bool verifyParameterValue(QString &rValue, const QString type, const QStringList &rSysParNames, QString &rErrorString)
{
    //Strip trailing and leading spaces
    stripLTSpaces(rValue);

    //! @todo what if empty
    QString initialSign="";
    // Strip initial +- sign to simplify further checks
    if ( (rValue[0] == '+') || (rValue[0] == '-') )
    {
        initialSign = rValue[0];
        rValue.remove(0,1);
    }

    if(rSysParNames.contains(rValue))
    {
        rValue.prepend(initialSign);
        return true;
    }

    if (type == "double")
    {
        bool onlyNumbers=true;
        if ( rValue[0].isNumber() || rValue[0] == '.')
        {
            // Replace incorrect decimal separator
            rValue.replace(",", ".");

            if( rValue.count("e") > 1 || rValue.count(".") > 1 || rValue.count("E") > 1 )
            {
                onlyNumbers=false;
            }
            else
            {
                for(int i=1; i<rValue.size(); ++i)
                {
                    if(!rValue[i].isDigit() && (rValue[i] != 'e') && (rValue[i] != 'E') && (rValue[i] != '+') && (rValue[i] != '-') && (rValue[i] != '.'))
                    {
                        onlyNumbers=false;
                        break;
                    }
                    else if( (rValue[i] == '+' || rValue[i] == '-') && !((rValue[i-1] == 'e') || (rValue[i-1] == 'E')) )
                    {
                        onlyNumbers=false;
                        break;
                    }
                }
            }
        }
        else
        {
            onlyNumbers = false;
        }

        if(!onlyNumbers)
        {
            rErrorString = QString("Invalid [double] parameter value \"%1\". Only numbers are allowed. Nummeric strings like 1[eE][+-]5 will work.").arg(rValue);
        }

        rValue.prepend(initialSign);
        return onlyNumbers;
    }
    else if (type == "integer")
    {
        bool onlyNumbers=true;
        for(int i=1; i<rValue.size(); ++i)
        {
            if (!rValue[i].isNumber())
            {
                onlyNumbers = false;
                break;
            }
        }

        if(!onlyNumbers)
        {
            rErrorString = QString("Invalid [integer] parameter value \"%1\". Only numbers are allowed.").arg(rValue);
        }

        rValue.prepend(initialSign);
        return onlyNumbers;
    }
    else if (type == "bool")
    {
        if ((rValue != "true") && (rValue != "false"))
        {
            rErrorString = QString("Invalid [bool] parameter value \"%1\". Only \"true\" or \"false\" are allowed.").arg(rValue);
            return false;
        }
        return true;
    }
    else if (type == "string")
    {
        return true;
    }

    rErrorString = QString("Invalid parameter type \"%1\"").arg(type);
    return false;
}



double findSmallestValueGreaterThanZero(QVector<double> data)
{
    double retval = DBLMAX;

    for(int i=0; i<data.size(); ++i)
    {
        if(data[i] > 0 && data[i] < retval)
        {
            retval = data[i];
        }
    }

    return retval;
}
