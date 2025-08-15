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
//! @file   GUIUtilities.cpp
//! @author All <flumes@liu.se>
//! @date   2010-10-09
//!
//! @brief Contains a class for misc utilities
//!
//$Id$

#include "GUIUtilities.h"

#ifdef Q_OS_OSX
#include <utility>
#else
#include <algorithm>
#endif

#include <qmath.h>
#include <QPoint>
#include <QDir>
#include <QDebug>
#include <QStringList>
#include <limits>
#include <math.h>
#include <complex>
#ifdef _WIN32
#include <windows.h>
#endif

#include "../global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "common.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "CoreUtilities/HmfLoader.h"
#include "Widgets/LibraryWidget.h"
#include "MessageHandler.h"

#define UNDERSCORE 95
#define UPPERCASE_LOW 65
#define UPPERCASE_HIGH 90
#define LOWERCASE_LOW 97
#define LOWERCASE_HIGH 122
#define NUMBERS_LOW 48
#define NUMBERS_HIGH 57

using namespace std;
const double DoubleMax = std::numeric_limits<double>::max();

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
        return tempName.remove("\"").trimmed(); //Remove quotes and trim (just to be sure)
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


//! @brief Convenience function if you don't have a stream to read from
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


//! @brief This function may be used to add quotes around string, useful for saving names. Ex: string -> "string"
QString addQuotes(QString str)
{
    str.prepend("\"");
    str.append("\"");
    return str;
}

//! @brief This function is used to remove quotes around string Ex: "string" -> string
QString removeQuotes(QString str)
{
    if (str.startsWith("\"") && str.endsWith("\""))
    {
        return str.mid(1, str.size()-2);
    }
    return str;
}


//! @brief This function returns the relative absolute path
//! @param [in] pathtochange The absolute path that you want to change
//! @param [in] basedir The absolute directory path of the base directory
//! @returns The relative pathtochange, relative to basepath
QString relativePath(QFileInfo pathtochange, QDir basedir)
{
    if (!pathtochange.isAbsolute())
    {
        qDebug() << "pathtochange is not absolute in relativePath utility function, need to handle this nicer";
        return "";
    }
    return basedir.relativeFilePath(pathtochange.absoluteFilePath());
}


//! @brief normalises degrees to range between -180 and 180 degrees
double normDeg180(const double deg)
{
    return rad2deg(normRad(deg2rad(deg)));
}

//! @brief normalises degrees to range between 0 and 359.999 degrees, 360.0 will be converted to 0
//! @todo potential danger comparing floats like this
double normDeg360(double deg)
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

    return fabs(deg); //Make sure we return positive (close to 0 if we have rounding issue that takes us below zero)
}

//! @brief normalises radians to range between -PI and PI degrees
double normRad(const double rad)
{
    return qAtan2(qCos(rad),qSin(rad));
}


//! @brief Calculates the distance between two points
double dist(const double x1, const double y1, const double x2, const double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}



//! @brief Check if first+eps is less then second, convenient to make usre float comparison works on all platforms
//! @return True if first+eps < second
bool fuzzyLT(const double first, const double second, const double eps)
{
    return (first+eps) < second;
}

//! @brief Calculates the 2NORM of one point, the absolute distance from 0,0
double dist(const QPointF &rPoint)
{
    return sqrt( rPoint.x()*rPoint.x() + rPoint.y()*rPoint.y() );
}

//! @brief Calculates the distance between two points
double dist(const QPointF &rPoint1, const QPointF &rPoint2)
{
    double x = rPoint1.x() - rPoint2.x();
    double y = rPoint1.y() - rPoint2.y();
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
//! Names of Greek letters will be transformed into Greek letters.
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
QVector< complex<double> > realToComplex(const QVector<double> &rRealVector)
{
    QVector< complex<double> > complexVector;
    complexVector.reserve(rRealVector.size());
    for(int i=0; i<rRealVector.size(); ++i)
    {
        complexVector.append(std::complex<double>(rRealVector[i], 0));
    }
    return complexVector;
}


//! @brief Apply windowing function
//! Only Hann windows are supported for now
//! @param data Vector with data to be windowed
//! @param [out] Ca Gain compensation factor
//! @param [out] Cb Frequency compensation factor
void windowFunction(QVector<double> &data, WindowingFunctionEnumT function, double &rCa, double &rCb)
{
    if(data.size() <= 1) {
        rCa = 1;
        rCb = 1;
        gpMessageHandler->addErrorMessage("Windowing functions require data of size greater than 1.");
        return;
    }
    switch(function) {
        case RectangularWindow:
        {
            rCa = 1;
            rCb = 1;
            break;  //Rectangular shall do nothing
        }
        case HannWindow: {
            int N = data.size()-1;
            for(int n=0; n<=N && N>0; ++n) {
                data[n] *= 0.5*(1-cos(2*M_PI*n/N));
            }
            rCa = 0.5;
            rCb = 1.5;
            break;
        }
        case FlatTopWindow: {
            int N = data.size()-1;
            rCa = 0;
            //Coefficients for flat top window according to ISO 18431-2
            double a0 = 1.0;
            double a1 = -1.933;
            double a2 = 1.286;
            double a3 = -0.388;
            double a4 = 0.0322;
            for(int n=0; n<=N && N>0; ++n) {
                double w = a0 + a1*cos(2*M_PI*n/N) + a2*cos(4*M_PI*n/N) + a3*cos(6*M_PI*n/N) + a4*cos(8*M_PI*n/N);
                data[n] *= w;
                rCa += w;
                rCb += w*w;
            }
            rCa /= N;
            rCb /= N;
            rCb /= rCa*rCa;
            break;
        }
    }

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
#if __cplusplus >= 201103L // magse Värre än så. swap har flyttat från <algorithm> till <utility> i C++11
            swap(data[j-1],  data[i]);
#else
            swap(data[j-1].real(), data[i].real());     // Even numbers
            swap(data[j-1].imag(), data[i].imag());     // Odd numbers
#endif
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

#if __cplusplus >= 201103L
                data[j].real(data[i].real() - tempr);
                data[j].imag(data[i].imag() - tempi);
                data[i].real(data[i].real() + tempr);
                data[i].imag(data[i].imag() + tempi);
#else
                data[j].real() = data[i].real() - tempr;
                data[j].imag() = data[i].imag() - tempi;
                data[i].real() += tempr;
                data[i].imag() += tempi;
#endif
            }
            wtemp=wr;
            wr += wr*wpr - wi*wpi;
            wi += wi*wpr + wtemp*wpi;
        }
        mmax=istep;
    }
    qDebug () << "FFT successful!";
}


//! @brief Resample a data vector to specified size using linear interpolation
//! @param [in,out] vector Reference to vector that will be resampled
//! @param newSize New size of vector
void resampleVector(QVector<double> &vector, int newSize)
{
    int oldSize = vector.size();

    QVector<double> tempVector(newSize);

    for(int i=0; i<newSize; ++i) {
        double pos = double(oldSize-1)/double(newSize-1)*double(i);
        int prev = int(floor(pos)); //Previous index in original vector
        int next = min(oldSize-1, int(ceil(pos)));  //Next index in original vector
        double x = 0;
        if(next != prev) {
            x = (pos-prev)/(next-prev);  //Fraction
        }
        tempVector[i] = (1.0-x)*vector.at(prev) + x*vector.at(next);
    }

    vector = tempVector;
}


//! @brief Limits Y and X vector to specified range in X vector
//! Both vectors must be of same size!
//! @param x X-vector
//! @param y Y-vector
//! @param min Minimum x value
//! @param max Maximum x value
void limitVectorToRange(QVector<double> &x, QVector<double> &y, double xmin, double xmax)
{
    if(x.isEmpty()) {
        return;
    }

    //Compute index of smallest element within range
    int imin = 0;
    while(x.at(imin) < xmin) {
        ++imin;
    }

    //Remove all elements below range
    x.remove(0,imin);
    y.remove(0,imin);

    if(x.isEmpty()) {
        return;
    }

    //Compute index of largest element within range
    int imax = x.size()-1;
    while(x.at(imax) > xmax) {
        --imax;
    }

    //Remove all elements above range
    x.remove(imax+1, x.size()-imax-1);
    y.remove(imax+1, y.size()-imax-1);

}


void removeDir(QString path, qint64 age_seconds)
{

    // Abort if dir is not old enough
    QFileInfo dirInfo(path);
    qint64 age = dirInfo.created().secsTo(QDateTime::currentDateTime());
    if (dirInfo.isDir() && (age < age_seconds))
    {
        return;
    }

    QDir dir(path);
    for(const QFileInfo &entryInfo : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (entryInfo.isDir()) {
            removeDir(entryInfo.absoluteFilePath(), age_seconds);
        }
        else {
            QFile::remove(entryInfo.absoluteFilePath());
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
    if (toPath.endsWith('/')) {
        toPath.chop(1);
    }

    QDir fromDir(fromPath);
    for(QFileInfo info : fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
        if (info.isDir()) {
            copyDir(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
        else {
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

    copyDir( gpDesktopHandler->getCoreIncludePath(), saveDir.path() );
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
//! Using the Box-Muller transform
//! @param average Average value of normal distribution
//! @param sigma Standard deviation of normal distribution
double normalDistribution(double average, double sigma)
{
    double U1 = 0;
    while(U1 == 0) {
        U1 = (double)rand() / (double)RAND_MAX;
    }
    double U2 = (double)rand() / (double)RAND_MAX;
    return average + sigma*sqrt(-2*log(U1))*cos(2*M_PI*U2);
}

//! @brief Generates random values evenly distributed between a minimum and a maximum value
//! @param min Minimum value
//! @param max Maximum value
double uniformDistribution(double min, double max)
{
    return min + (double)rand()/(double)RAND_MAX*(max-min);
}

// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}

//! @todo this should be handled by CORE
bool verifyParameterValue(QString &rValue, const QString type, const QStringList &rSelfParameterNames, const QStringList &rSysParNames, QString &rErrorString)
{
    //Strip trailing and leading spaces
    stripLTSpaces(rValue);

    // Doubles are evaluated numhop expressions
    if (type == "double")
    {
        // Replace incorrect decimal separator
        rValue.replace(",", ".");
        return true;
    }

    QString initialSign;
    // Strip initial +- sign to simplify further checks
    if ( (rValue[0] == '+') || (rValue[0] == '-') )
    {
        initialSign = rValue[0];
        rValue.remove(0,1);
    }

    if (rValue.startsWith("self.") && rSelfParameterNames.contains(rValue.mid(5))) {
        rValue.prepend(initialSign);
        return true;
    }
    else if(rSysParNames.contains(rValue)) {
        rValue.prepend(initialSign);
        return true;
    }

    if (type == "integer")
    {
        bool onlyNumbers=true;
        for(int i=0; i<rValue.size(); ++i)
        {
            if (!rValue[i].isNumber())
            {
                onlyNumbers = false;
                break;
            }
        }

        rValue.prepend(initialSign);
        if(!onlyNumbers)
        {
            rErrorString = QString("Invalid [integer] parameter value \"%1\". Only numbers, self.parameter_name or the name of a system parameter is allowed.").arg(rValue);
        }

        return onlyNumbers;
    }
    else if (type == "conditional")
    {
        bool onlyNumbers=true;
        for(int i=0; i<rValue.size(); ++i)
        {
            if (!rValue[i].isNumber())
            {
                onlyNumbers = false;
                break;
            }
        }

        rValue.prepend(initialSign);
        if(!onlyNumbers)
        {
            rErrorString = QString("Invalid [conditional] parameter value \"%1\". Only numbers are allowed.").arg(rValue);
        }

        return onlyNumbers;
    }
    else if (type == "bool")
    {
        if ((rValue != "true") && (rValue != "false"))
        {
            rErrorString = QString("Invalid [bool] parameter value \"%1\". Only \"true\" or \"false\", self.parameter_name or the name of a system parameter is allowed.").arg(rValue);
            return false;
        }
        return true;
    }
    else if (type == "string" || type == "textblock" || type == "filepath")
    {
        return true;
    }

    rErrorString = QString("Invalid parameter type \"%1\"").arg(type);
    return false;
}



double findSmallestValueGreaterThanZero(QVector<double> data)
{
    double retval = DoubleMax;

    for(int i=0; i<data.size(); ++i)
    {
        if(data[i] > 0 && data[i] < retval)
        {
            retval = data[i];
        }
    }

    return retval;
}



//! @brief Splits a string at specified character, but does not split inside quotations
//! @param str String to split
//! @param c Character to split at
//! @param split Reference to list with split strings
void splitWithRespectToQuotations(const QString &str, const QChar c, QStringList &split)
{
    bool withinQuotations=false;
    int start=0;
    int len=0;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '"')
        {
            withinQuotations=!withinQuotations;
        }
        else if(str[i] == c && !withinQuotations)
        {
            split.append(str.mid(start,len));
            start=start+len+1;
            len=-1;
        }
        ++len;
    }
    split.append(str.mid(start,len));
}


//! @brief Checks if a string contains a character, but ignores all characters within quotations
//! @param str String to search
//! @param c Character to search for
bool containsOutsideQuotes(const QString &str, const QChar c) {
    bool withinQuotations = false;
    for(int i=0; i<str.size(); ++i) {
        if (str[i] == '"') {
            withinQuotations = !withinQuotations;
        }
        else if(str[i] == c && !withinQuotations) {
            return true;
        }
    }
    return false;
}

//! @brief Splits a string at specified character, but does not split inside quotations and parenthesis
//! @param str String to split
//! @param c Character to split at
//! @param split Reference to list with split strings
void splitRespectingQuotationsAndParanthesis(const QString str, const QChar c, QStringList &rSplit)
{
    bool withinQuotations=false;
    int withinNumParanthesis=0;
    int start=0;
    int len=0;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '"')
        {
            withinQuotations=!withinQuotations;
        }
        // This code assumes that parenthesis are correctly ordered (some other code should check that)
        else if (str[i] == '(')
        {
            ++withinNumParanthesis;
        }
        else if (str[i] == ')')
        {
            --withinNumParanthesis;
        }
        else if(str[i] == c && !withinQuotations && (withinNumParanthesis==0))
        {
            rSplit.append(str.mid(start,len));
            start=start+len+1;
            len=-1;
        }
        ++len;
    }
    rSplit.append(str.mid(start,len));
}

//! @brief Reimplementation of the core function sanitize name
//! @todo this one may not be needed in the future when all loading of core data, and name checking, is moved to core
void santizeName(QString &rString)
{
    QString::iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))     ) )
        {
            // Replace invalid char with underscore
            *it = UNDERSCORE;
        }
    }
}

bool isNameValid(const QString &rString)
{
    QString::const_iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))   ||
                (*it == UNDERSCORE)                                   ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

//! @brief From a QString extract text between first and last
QString extractBetweenFromQString(const QString &rString, const QChar &rFirst, const QChar &rLast)
{
    int f = rString.indexOf(rFirst);
    int l = rString.lastIndexOf(rLast);
    if ((f > -1) && (l > -1))
    {
        if (l-(f+1) > 0)
        {
            return rString.mid(f+1,l-f-1);
        }
    }
    return "";
}


TicToc::TicToc(const TextOutput outType) : QTime(), mTextOutput(outType)
{
    tic();
}

void TicToc::tic()
{
    this->restart();
}

void TicToc::tic(const QString &text)
{
    print(text);
    tic();
}

int TicToc::toc()
{
    return this->elapsed();
}

int TicToc::toc(const QString &text, const int minMs)
{
    const int ms = toc();
    if (ms >= minMs)
    {
        if (ms == 0)
        {
            print(text+" took: less then 1 ms");
        }
        else
        {
            print(QString("%1 took: %2 ms").arg(text).arg(ms));
        }
    }
    return ms;
}

void TicToc::print(const QString &text)
{
    switch (mTextOutput) {
    case TextOutput::Qdebug:
        qDebug() << text;
        break;
    case TextOutput::DebugMessage:
        gpMessageHandler->addDebugMessage(text);
        break;
    case TextOutput::InfoMessage:
        gpMessageHandler->addInfoMessage(text);
        break;
    default:
        break;
    }

}

//! @brief Creates a linear space between start and stop with step size step
//! @param [in] start The start value
//! @param [in] stop The stop value, must be > start
//! @param [in] step The step size, must be > 0
//! @returns A linear space vector
QVector<int> linspace(const int start, const int stop, const int step)
{
    QVector<int> vec;
    vec.reserve((stop-start)/step+1);
    for (int i=start; i<=stop; i=i+step)
    {
        vec.push_back(i);
    }
    return vec;
}


QString extractFilenameExtension(const QString &rFilename)
{
    int i = rFilename.lastIndexOf('.');
    if (i < 0)
    {
        return "";
    }
    else
    {
        return rFilename.right(rFilename.size()-1-i);
    }
}


bool isVersionGreaterThan(QString v1, QString v2)
{
    return hopsan::isVersionAGreaterThanB(v1.toStdString().c_str(), v2.toStdString().c_str());
}

//! @brief Extracts the dirPath, fileName (including suffix), and the suffix from a file path
void splitFilepath(const QString &rFilepath, QString &rDirPath, QString &rFilename, QString &rFilesuffix)
{
    rDirPath = rFilepath.left(rFilepath.lastIndexOf("/"));
    rFilename = rFilepath.right(rFilepath.size()-rFilepath.lastIndexOf("/"));
    rFilesuffix = extractFilenameExtension(rFilename);
}

//! @brief Replaces a pattern preserving pattern indentation, adding indentation to each line of text
//! @param[in] rPattern The pattern to replace
//! @param[in] rReplacement The replacement text
//! @param[in] rText The text to replace in
//! @returns True if pattern was found (and replaced), else False
bool replacePattern(const QString &rPattern, const QString &rReplacement, QString &rText)
{
    bool didReplace=false;
    while (true)
    {
        // First find pattern start in text
        int b = rText.indexOf(rPattern);
        if (b > -1)
        {
            // From beginning search backwards to count number of white spaces
            int nIndent = 0;
            QString indentString;
            --b;
            while (rText[b].isSpace() && (rText[b] != '\n'))
            {
                ++nIndent;
                indentString.append(" ");
                --b;
            }

            //Add indentation to each line in replacement text
            QString newrepl, repl=rReplacement;
            QTextStream ts(&repl);
            while (!ts.atEnd())
            {
                newrepl += indentString+ts.readLine()+"\n";
            }
            // If original replacement string lacks newline at end, the last newline should be removed
            if (!rReplacement.endsWith("\n"))
            {
                //! @todo will this work with crlf
                newrepl.chop(1);
            }

            // Now replace pattern
            rText.replace(indentString+rPattern, newrepl);

            didReplace =  true;
        }
        else
        {
            break;
        }
    }
    return didReplace;
}

//! @brief Remove all spaces from a string
QString removeAllSpaces(const QString &org)
{
    QString res;
    res.reserve(org.size());
    for (int i=0; i<org.size(); ++i)
    {
        if (!org[i].isSpace())
        {
            res.append(org[i]);
        }
    }
    return res;
}

//! @brief Split a string into parts based on any of the delimiters in delims
//! @param[in] str The string to split
//! @param[in] delims A list of delimiters (single character strings)
//! @param[out] rSplitList A list of the split string, delimiters are not included
void splitOnAny(const QString &str, const QStringList &delims, QStringList &rSplitList)
{
    int b=0; int i;
    for (i=0; i<str.size(); ++i)
    {
        if (delims.contains(str[i]))
        {
            int len = i-b;
            if (len > 0)
            {
                rSplitList.append(QStringRef(&str,b,len).toString());
            }
            b=i+1;
        }
    }
    int len = i-b;
    if (len > 0)
    {
        rSplitList.append(QStringRef(&str,b,len).toString());
    }
}


void extractSectionsRespectingQuotationsAndParanthesis(const QString str, const QChar c, QStringList &rSplit, QList<int> &rSectionIndexes)
{
    bool withinQuotations=false;
    int withinNumParanthesis=0;
    int start=0;
    int len=0;
    bool withinSection=false;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == '"')
        {
            withinQuotations=!withinQuotations;
        }
        // This code assumes that parenthesis are correctly ordered (some other code should check that)
        else if (str[i] == '(')
        {
            ++withinNumParanthesis;
        }
        else if (str[i] == ')')
        {
            --withinNumParanthesis;
        }
        else if(str[i] == c && !withinQuotations && (withinNumParanthesis==0))
        {
            rSplit.append(str.mid(start,len));
            start=start+len+1;
            len=-1;

            if (withinSection)
            {
                rSectionIndexes.append(rSplit.size()-1);
            }

            // Toggle bool
            withinSection = !withinSection;
        }
        ++len;
    }
    if (len > 0)
    {
        rSplit.append(str.mid(start,len));
    }
}

void extractSections(const QString str, const QChar c, QStringList &rSplit, QList<int> &rSectionIndexes)
{
    int start=0;
    int len=0;
    bool withinSection=false;
    for(int i=0; i<str.size(); ++i)
    {
        if(str[i] == c)
        {
            rSplit.append(str.mid(start,len));
            start=start+len+1;
            len=-1;

            if (withinSection)
            {
                rSectionIndexes.append(rSplit.size()-1);
            }

            // Toggle bool
            withinSection = !withinSection;
        }
        ++len;
    }
    rSplit.append(str.mid(start,len));
}

bool saveXmlFile(QString xmlFilePath, GUIMessageHandler *pMessageHandler, std::function<QDomDocument()> saveFunction, int indentation)
{
    TicToc tt;
    QFile xmlFile(xmlFilePath);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        pMessageHandler->addErrorMessage("Could not open the file: "+xmlFile.fileName()+" for writing.");
        return false;
    }
    const int open_ms = tt.toc();

    tt.tic();
    // Saving directly using a QTextStream is very slow on a Windows network drive, so instead stream to a byte array and tehn save that
    QByteArray temp_data;
    QTextStream temp_data_stream(&temp_data);
    QDomDocument doc = saveFunction();
    doc.save(temp_data_stream, indentation);
    xmlFile.write(temp_data);
    const double save_ms = tt.toc();

    tt.tic();
    xmlFile.close();
    const int close_ms = tt.toc();

    const double total_s = (open_ms+save_ms+close_ms)*1.0e-3;
    const double save_s = save_ms*1.0e-3;
    const double size_mb = xmlFile.size() * 1.0e-6;
    pMessageHandler->addDebugMessage(QString("Saving file: %1 took %2 s at %3 MB/s. Opening: %4 ms, Closing: %5 ms")
                                     .arg(xmlFilePath).arg(total_s).arg(size_mb/save_s).arg(open_ms).arg(close_ms));
    return true;
}

//! @brief Get all system parameter names for provided system its parent and grand parents
QStringList getAllAccessibleSystemParameterNames(SystemObject *pSystem) {
    QStringList parameterNames = pSystem->getParameterNames();
    if ( !pSystem->isTopLevelContainer() ) {
        parameterNames += getAllAccessibleSystemParameterNames(pSystem->getParentSystemObject());
    }
    return parameterNames;
}
