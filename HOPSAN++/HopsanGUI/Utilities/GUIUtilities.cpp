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
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include "common.h"
#include "Configuration.h"
#include "Widgets/LibraryWidget.h"

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
            QFile::remove(info.absoluteFilePath());
        }
    }
    dir.rmdir(path);
}


void copyIncludeFilesToDir(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkdir("include");
    QFile componentH(gExecPath + QString(INCLUDEPATH) + "Component.h");
    componentH.copy(saveDir.path() + "/include/Component.h");
    QFile componentSystemH(gExecPath + QString(INCLUDEPATH) + "ComponentSystem.h");
    componentSystemH.copy(saveDir.path() + "/include/ComponentSystem.h");
    QFile componentEssentialsH(gExecPath + QString(INCLUDEPATH) + "ComponentEssentials.h");
    componentEssentialsH.copy(saveDir.path() + "/include/ComponentEssentials.h");
    QFile componentUtilitiesH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities.h");
    componentUtilitiesH.copy(saveDir.path() + "/include/ComponentUtilities.h");
    QFile hopsanCoreH(gExecPath + QString(INCLUDEPATH) + "HopsanCore.h");
    hopsanCoreH.copy(saveDir.path() + "/include/HopsanCore.h");
    QFile hopsanEssentialsH(gExecPath + QString(INCLUDEPATH) + "HopsanEssentials.h");
    hopsanEssentialsH.copy(saveDir.path() + "/include/HopsanEssentials.h");
    QFile nodeH(gExecPath + QString(INCLUDEPATH) + "Node.h");
    nodeH.copy(saveDir.path() + "/include/Node.h");
    QFile portH(gExecPath + QString(INCLUDEPATH) + "Port.h");
    portH.copy(saveDir.path() + "/include/Port.h");
    QFile versionH(gExecPath + QString(INCLUDEPATH) + "version.h");
    versionH.copy(saveDir.path() + "/include/version.h");
    QFile win32dllH(gExecPath + QString(INCLUDEPATH) + "win32dll.h");
    win32dllH.copy(saveDir.path() + "/include/win32dll.h");

    QDir componentsDir;
    componentsDir.mkdir(saveDir.path() + "/include/Components");
    QFile componentsH(gExecPath + QString(INCLUDEPATH) + "Components/Components.h");
    componentsH.copy(saveDir.path() + "/include/Components/Components.h");

    QDir componentUtilitiesDir;
    componentUtilitiesDir.mkdir(saveDir.path() + "/include/ComponentUtilities");
    QFile auxiliarySimulationFunctionsH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/AuxiliarySimulationFunctions.h");
    auxiliarySimulationFunctionsH.copy(saveDir.path() + "/include/ComponentUtilities/AuxiliarySimulationFunctions.h");
    QFile csvParserH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/CSVParser.h");
    csvParserH.copy(saveDir.path() + "/include/ComponentUtilities/CSVParser.h");
    QFile delayH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/Delay.hpp");
    delayH.copy(saveDir.path() + "/include/ComponentUtilities/Delay.hpp");
    QFile doubleIntegratorWithDampingH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/DoubleIntegratorWithDamping.h");
    doubleIntegratorWithDampingH.copy(saveDir.path() + "/include/ComponentUtilities/DoubleIntegratorWithDamping.h");
    QFile doubleIntegratorWithDampingAndCoulumbFrictionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h");
    doubleIntegratorWithDampingAndCoulumbFrictionH.copy(saveDir.path() + "/include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h");
    QFile firstOrderTransferFunctionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/FirstOrderTransferFunction.h");
    firstOrderTransferFunctionH.copy(saveDir.path() + "/include/ComponentUtilities/FirstOrderTransferFunction.h");
    QFile integratorH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/Integrator.h");
    integratorH.copy(saveDir.path() + "/include/ComponentUtilities/Integrator.h");
    QFile integratorLimitedH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/IntegratorLimited.h");
    integratorLimitedH.copy(saveDir.path() + "/include/ComponentUtilities/IntegratorLimited.h");
    QFile ludcmpH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/ludcmp.h");
    ludcmpH.copy(saveDir.path() + "/include/ComponentUtilities/ludcmp.h");
    QFile matrixH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/matrix.h");
    matrixH.copy(saveDir.path() + "/include/ComponentUtilities/matrix.h");
    QFile readDataCurveH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/ReadDataCurve.h");
    readDataCurveH.copy(saveDir.path() + "/include/ComponentUtilities/ReadDataCurve.h");
    QFile secondOrderTransferFunctionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/SecondOrderTransferFunction.h");
    secondOrderTransferFunctionH.copy(saveDir.path() + "/include/ComponentUtilities/SecondOrderTransferFunction.h");
    QFile turbulentFlowFunctionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/TurbulentFlowFunction.h");
    turbulentFlowFunctionH.copy(saveDir.path() + "/include/ComponentUtilities/TurbulentFlowFunction.h");
    QFile valveHysteresisH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/ValveHysteresis.h");
    valveHysteresisH.copy(saveDir.path() + "/include/ComponentUtilities/ValveHysteresis.h");
    QFile whiteGaussianNoiseH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/WhiteGaussianNoise.h");
    whiteGaussianNoiseH.copy(saveDir.path() + "/include/ComponentUtilities/WhiteGaussianNoise.h");

    QDir coreUtilitiesDir;
    coreUtilitiesDir.mkdir(saveDir.path() + "/include/CoreUtilities");
    QFile hmfLoaderH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/HmfLoader.h");
    hmfLoaderH.copy(saveDir.path() + "/include/CoreUtilities/HmfLoader.h");
    QFile classFactoryHpp(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/ClassFactory.hpp");
    classFactoryHpp.copy(saveDir.path() + "/include/CoreUtilities/ClassFactory.hpp");
    QFile classFactoryStatusCheckHpp(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/ClassFactoryStatusCheck.hpp");
    classFactoryStatusCheckHpp.copy(saveDir.path() + "/include/CoreUtilities/ClassFactoryStatusCheck.hpp");
    QFile findUniqueNameH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/FindUniqueName.h");
    findUniqueNameH.copy(saveDir.path() + "/include/CoreUtilities/FindUniqueName.h");
    QFile hopsanCoreMessageHandlerH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/HopsanCoreMessageHandler.h");
    hopsanCoreMessageHandlerH.copy(saveDir.path() + "/include/CoreUtilities/HopsanCoreMessageHandler.h");
    QFile loadExternalH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/LoadExternal.h");
    loadExternalH.copy(saveDir.path() + "/include/CoreUtilities/LoadExternal.h");

    QDir nodesDir;
    nodesDir.mkdir(saveDir.path() + "/include/Nodes");
    QFile nodesH(gExecPath + QString(INCLUDEPATH) + "/Nodes/Nodes.h");
    nodesH.copy(saveDir.path() + "/include/Nodes/Nodes.h");

    QDir dependenciesDir;
    dependenciesDir.mkdir(saveDir.path() + "/include/Dependencies");

    QDir csvParserDir;
    csvParserDir.mkpath(saveDir.path() + "/include/Dependencies/libcsv_parser++-1.0.0/include/csv_parser");
    QFile csv_ParserH(gExecPath + QString(INCLUDEPATH) + "/Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp");
    csv_ParserH.copy(saveDir.path() + "/include/Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp");

    QDir rapidXmlDir;
    rapidXmlDir.mkdir(saveDir.path() + "/include/Dependencies/rapidxml-1.13");
    QFile rapidXmlH(gExecPath + QString(INCLUDEPATH) + "/Dependencies/rapidxml-1.13/rapidxml.hpp");
    rapidXmlH.copy(saveDir.path() + "/include/Dependencies/rapidxml-1.13/rapidxml.hpp");
    QFile rapidXmlUtilsH(gExecPath + QString(INCLUDEPATH) + "/Dependencies/rapidxml-1.13/rapidxml_utils.hpp");
    rapidXmlUtilsH.copy(saveDir.path() + "/include/Dependencies/rapidxml-1.13/rapidxml_utils.hpp");

    QDir includeDir;
    includeDir.mkdir(saveDir.path() + "/include/include");
    QFile svnRevNumH(gExecPath + QString(INCLUDEPATH) + "/include/svnrevnum.h");
    svnRevNumH.copy(saveDir.path() + "/include/include/svnrevnum.h");
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
//! Using the Box–Muller transform
//! @param average Average value of normal distribution
//! @param sigma Standard deviation of normal distribution
double normalDistribution(double average, double sigma)
{
    double U1 = (double)rand() / (double)RAND_MAX;
    double U2 = (double)rand() / (double)RAND_MAX;
    return average + sigma*sqrt(-2*log(U1))*cos(2*3.1415926*U2);
}



void generateComponentSourceCode(QString outputFile, QDomElement &rDomElement)
{
    //Initialize the file stream
    QFileInfo fileInfo;
    QFile file;
    fileInfo.setFile(outputFile);
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + outputFile);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    QString typeName = rDomElement.attribute("typename");
    QString displayName = rDomElement.attribute("displayname");
    QString cqsType = rDomElement.attribute("cqstype");
    if(cqsType == "S")
        cqsType = "Signal";

    fileStream << "#ifndef " << typeName.toUpper() << "_HPP_INCLUDED\n";
    fileStream << "#define " << typeName.toUpper() << "_HPP_INCLUDED\n\n";
    fileStream << "#include <math.h>\n";
    fileStream << "#include \"ComponentEssentials.h\"\n\n";
    fileStream << "namespace hopsan {\n\n";
    fileStream << "    class " << typeName << " : public Component" << cqsType << "\n";
    fileStream << "    {\n";
    fileStream << "    private:\n";
    QDomElement parameterElement = rDomElement.firstChildElement("parameter");
    while(!parameterElement.isNull())
    {
        fileStream << "        double " << parameterElement.attribute("name") << ";\n";
        parameterElement = parameterElement.nextSiblingElement("parameter");
    }

    QDomElement portElement = rDomElement.firstChildElement("port");
    fileStream << "        double ";
    int portId=1;
    QStringList allVarNames;
    while(!portElement.isNull())
    {
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");
        QString id = QString().setNum(portId);
        if(nodetype == "NodeSignal")
        {
            allVarNames << name;
        }
        else if(nodetype == "NodeHydraulic")
        {
            allVarNames << "p"+id << "q"+id << "c"+id << "Zc"+id;
        }
        ++portId;
        portElement = portElement.nextSiblingElement("port");
    }
    if(!allVarNames.isEmpty())
    {
        fileStream << "*mpND_" << allVarNames[0];
        for(int i=1; i<allVarNames.size(); ++i)
        {
            fileStream << ", *mpND_" << allVarNames[i];
        }
    }
    fileStream << ";\n";
    portElement = rDomElement.firstChildElement("port");
    fileStream << "        Port ";
    while(!portElement.isNull())
    {
        fileStream << "*mp" << portElement.attribute("name");
        portElement = portElement.nextSiblingElement("port");
        if(!portElement.isNull())
        {
            fileStream << ", ";
        }
    }
    fileStream << ";\n\n";
    fileStream << "    public:\n";
    fileStream << "        static Component *Creator()\n";
    fileStream << "        {\n";
    fileStream << "            return new " << typeName << "();\n";
    fileStream << "        }\n\n";
    fileStream << "        " << typeName << "() : Component" << cqsType << "()\n";
    fileStream << "        {\n";
    parameterElement = rDomElement.firstChildElement("parameter");
    while(!parameterElement.isNull())
    {
        fileStream << "            " << parameterElement.attribute("name") << " = " << parameterElement.attribute("init") << ";\n";
        parameterElement = parameterElement.nextSiblingElement("parameter");
    }
    fileStream << "\n";
    parameterElement = rDomElement.firstChildElement("parameter");
    while(!parameterElement.isNull())
    {
        QString name = parameterElement.attribute("name");
        QString desc = parameterElement.attribute("description");
        QString unit = parameterElement.attribute("unit");
        QString disp = parameterElement.attribute("displayname");
        fileStream << "            registerParameter(\"" << disp << "\", \"" << desc << "\", \"" << unit << "\", " << name << ");\n";
        parameterElement = parameterElement.nextSiblingElement("parameter");
    }
    fileStream << "\n";
    portElement = rDomElement.firstChildElement("port");
    while(!portElement.isNull())
    {
        QString type = portElement.attribute("type");
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");
        bool notreq = portElement.attribute("notrequired") == "True";
        fileStream << "            mp" << name << " = add" << type << "(\"" << name << "\", \"" << nodetype << "\"";
        if(notreq)
        {
            fileStream << ", Port::NOTREQUIRED);\n";
        }
        else
        {
            fileStream << ");\n";
        }
        portElement = portElement.nextSiblingElement("port");
    }
    fileStream << "        }\n\n";
    fileStream << "        void initialize()\n";
    fileStream << "        {\n";
    portElement = rDomElement.firstChildElement("port");
    portId=1;
    while(!portElement.isNull())
    {
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");
        QString def = portElement.attribute("default");
        bool notreq = portElement.attribute("notrequired") == "True";

        QStringList varNames;
        QStringList varLabels;
        if(nodetype == "NodeSignal")
        {
            varNames << name;
            varLabels << "VALUE";
        }
        else if(nodetype == "NodeHydraulic")
        {
            varNames << "p" << "q" << "c" << "Zc";
            varLabels << "PRESSURE" << "FLOW" << "WAVEVARIABLE" << "CHARIMP";
        }

        for(int i=0; i<varNames.size(); ++i)
        {
            QString varName;
            if(nodetype == "NodeSignal")
                varName = varNames[i];
            else
                varName = varNames[i]+QString().setNum(portId);
            fileStream << "            mpND_" << varName << " = getSafeNodeDataPtr(mp" << name << ", " << nodetype << "::" << varLabels[i];
            if(notreq)
            {
                fileStream << ", " << def;
            }
            fileStream << ");\n";
        }
        ++portId;
        portElement = portElement.nextSiblingElement("port");
    }
    fileStream << "        }\n\n";
    fileStream << "        void simulateOneTimestep()\n";
    fileStream << "        {\n";
    fileStream << "            double ";
    portElement = rDomElement.firstChildElement("port");
    portId=1;
    while(!portElement.isNull())
    {
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");

        QStringList varNames;
        if(nodetype == "NodeSignal")
        {
            varNames << name;
        }
        else if(nodetype == "NodeHydraulic")
        {
            varNames << "p" << "q" << "c" << "Zc";
        }

        for(int i=0; i<varNames.size()-1; ++i)
        {
            QString varName;
            if(nodetype == "NodeSignal")
                varName = varNames[i];
            else
                varName = varNames[i] + QString().setNum(portId);
            fileStream << varName << ", ";
        }
        QString varName;
        if(nodetype == "NodeSignal")
            varName = varNames.last();
        else
            varName = varNames.last() + QString().setNum(portId);
        fileStream << varName;
        ++portId;
        portElement = portElement.nextSiblingElement("port");
        if(!portElement.isNull())
        {
            fileStream << ", ";
        }
    }
    fileStream << ";\n";
    portElement = rDomElement.firstChildElement("port");
    portId=1;
    while(!portElement.isNull())
    {
        QString type = portElement.attribute("type");
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");

        QStringList varNames;
        if(nodetype == "NodeSignal" && type == "ReadPort")
        {
            varNames << name;
        }
        else if(nodetype == "NodeHydraulic" && cqsType == "Q")
        {
            varNames << "c" << "Zc";
        }
        else if(nodetype == "NodeHydraulic" && cqsType == "C")
        {
            varNames << "p" << "q";
        }
        else if(nodetype == "NodeHydraulic" && cqsType == "S")
        {
            varNames << "p" << "q" << "c" << "Zc";
        }

        for(int i=0; i<varNames.size(); ++i)
        {
            QString varName;
            if(nodetype == "NodeSignal")
                varName = varNames[i];
            else
                varName = varNames[i] + QString().setNum(portId);
            fileStream << "            " << varName << " = (*mpND_" << varName << ");\n";
        }
        ++portId;
        portElement = portElement.nextSiblingElement("port");
    }
    fileStream << "\n";
    QDomElement equationElement = rDomElement.firstChildElement("equation");
    while(!equationElement.isNull())
    {
        fileStream << "            " << equationElement.text() << ";\n";
        equationElement = equationElement.nextSiblingElement("equation");
    }
    fileStream << "\n";
    portElement = rDomElement.firstChildElement("port");
    portId=1;
    while(!portElement.isNull())
    {
        QString type = portElement.attribute("type");
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");

        QStringList varNames;
        if(nodetype == "NodeSignal" && type == "WritePort")
        {
            varNames << name;
        }
        else if(nodetype == "NodeHydraulic" && cqsType == "C")
        {
            varNames << "c" << "Zc";
        }
        else if(nodetype == "NodeHydraulic" && cqsType == "Q")
        {
            varNames << "p" << "q";
        }
        else if(nodetype == "NodeHydraulic" && cqsType == "S")
        {
            varNames << "p" << "q" << "c" << "Zc";
        }

        for(int i=0; i<varNames.size(); ++i)
        {
            QString varName;
            if(nodetype == "NodeSignal")
                varName = varNames[i];
            else
                varName = varNames[i] + QString().setNum(portId);
            fileStream << "            (*mpND_" << varName << ") = " << varName << ";\n";
        }
        ++portId;
        portElement = portElement.nextSiblingElement("port");
    }
    fileStream << "        }\n";
    fileStream << "    };\n";
    fileStream << "}\n\n";
    fileStream << "#endif // " << typeName.toUpper() << "_HPP_INCLUDED\n";
    file.close();

    QFile ccLibFile;
    ccLibFile.setFileName("tempLib.cc");
    if(!ccLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open tempLib.cc for writing.");
        return;
    }
    QTextStream ccLibStream(&ccLibFile);
    ccLibStream << "#include \"" << outputFile << "\"\n";
    ccLibStream << "#include \"ComponentEssentials.h\"\n\n";
    ccLibStream << "using namespace hopsan;\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* /*nfact_ptr*/)\n";
    ccLibStream << "{\n";
    ccLibStream << "    cfact_ptr->registerCreatorFunction(\"" << typeName<< "\", " << typeName << "::Creator);\n";
    ccLibStream << "}\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    ccLibStream << "{\n";
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    QFile clBatchFile;
    clBatchFile.setFileName("compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "g++.exe -shared tempLib.cc -o " << typeName << ".dll -I" << INCLUDEPATH << " -L./ -lHopsanCore\n";
    clBatchFile.close();

    QFile xmlFile;
    xmlFile.setFileName(typeName+".xml");
    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open " + typeName + ".xml  for writing.");
        return;
    }
    QTextStream xmlStream(&xmlFile);
    xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xmlStream << "<hopsanobjectappearance version=\"0.2\">\n";
    xmlStream << "  <modelobject typename=\"" << typeName << "\" displayname=\"" << displayName << "\">\n";
    xmlStream << "    <icons>\n";
    //! @todo Make it possible to choose icon files
    //! @todo In the meantime, use a default "generated component" icon
    xmlStream << "      <icon type=\"user\" path=\"generatedcomponenticon.svg\" iconrotation=\"ON\"/>\n";
    xmlStream << "    </icons>\n";
    xmlStream << "    <portpositions>\n";
    portElement = rDomElement.firstChildElement("port");
    QStringList leftPorts, rightPorts, topPorts;
    while(!portElement.isNull())
    {
        QString type = portElement.attribute("type");
        QString name = portElement.attribute("name");
        QString nodetype = portElement.attribute("nodetype");

        if(nodetype == "NodeSignal" && type == "ReadPort")
            leftPorts << name;
        else if(nodetype == "NodeSignal" && type == "WritePort")
            rightPorts << name;
        else
            topPorts << name;
        portElement = portElement.nextSiblingElement("port");
    }
    for(int i=0; i<leftPorts.size(); ++i)
    {
        xmlStream << "      <portpose name=\"" << leftPorts[i] << "\" x=\"0.0\" y=\"" << QString().setNum((double(i)+1)/(double(leftPorts.size())+1.0)) << "\" a=\"180\"/>\n";
    }
    for(int i=0; i<rightPorts.size(); ++i)
    {
        xmlStream << "      <portpose name=\"" << rightPorts[i] << "\" x=\"1.0\" y=\"" << QString().setNum((double(i)+1)/(double(rightPorts.size())+1.0)) << "\" a=\"0\"/>\n";
    }
    for(int i=0; i<topPorts.size(); ++i)
    {
        xmlStream << "      <portpose name=\"" << topPorts[i] << "\" x=\"" << QString().setNum((double(i)+1)/(double(topPorts.size())+1.0)) << "\" y=\"0.0\" a=\"270\"/>\n";
    }
    xmlStream << "    </portpositions>\n";
    xmlStream << "  </modelobject>\n";
    xmlStream << "</hopsanobjectappearance>\n";
    xmlFile.close();

    //Execute HopsanFMU compile script
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + gExecPath + " & compile.bat");
    p.waitForFinished();

    QDir componentsDir(QString(DOCSPATH));
    QDir generatedDir(QString(DOCSPATH) + "Generated Componentes/");
    if(!generatedDir.exists())
    {
        componentsDir.mkdir("Generated Componentes");
    }

    xmlFile.copy(generatedDir.path() + "/" + xmlFile.fileName());

    QFile dllFile(gExecPath+typeName+".dll");
    dllFile.copy(generatedDir.path() + "/" + typeName + ".dll");

    QFile svgFile(QString(OBJECTICONPATH)+"generatedcomponenticon.svg");
    svgFile.copy(generatedDir.path() + "/generatedcomponenticon.svg");

/*    file.remove();
    clBatchFile.remove();
    ccLibFile.remove();
    dllFile.remove();
    xmlFile.remove()*/;

    //Load the library
    gpMainWindow->mpLibrary->unloadExternalLibrary(generatedDir.path());
    gpMainWindow->mpLibrary->loadExternalLibrary(generatedDir.path());    //Load the library
}
