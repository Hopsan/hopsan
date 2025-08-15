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
//! @file   GUIUtilities.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-10-09
//!
//! @brief Contains a class for misc utilities
//!
//$Id$


#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QPointF>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDomElement>
#include <QTime>
#include <complex>
#include <functional>

#include "common.h"

class GUIMessageHandler;
class SystemObject;

QString readName(QTextStream &rTextStream);
QString readName(QString namestring);
void stripLTSpaces(QString &rString);
QString removeAllSpaces(const QString &org);
QString addQuotes(QString str);
QString removeQuotes(QString str);
QString relativePath(QFileInfo pathtochange, QDir basedir);
double normDeg180(const double deg);
double normDeg360(const double deg);
double normRad(const double rad);
double dist(const QPointF &rPoint);
double dist(const QPointF &rPoint1, const QPointF &rPoint2);
double dist(double x1,double y1, double x2, double y2);
bool fuzzyLT(const double first, const double second, const double eps);
void replaceWord(QString &string, QString before, QString after);
QString parseVariableDescription(QString input);
QString parseVariableUnit(QString input);
QVector< std::complex<double> > realToComplex(const QVector<double> &rRealVector);
void windowFunction(QVector<double> &data, WindowingFunctionEnumT function, double &Ca, double &Cb);
void FFT(QVector< std::complex<double> > &data);
void resampleVector(QVector<double> &vector, int newSize);
void limitVectorToRange(QVector<double> &x, QVector<double> &y, double min, double max);
void removeDir(QString path, qint64 age_seconds=-1);
void copyDir(const QString fromPath, QString toPath);
void copyIncludeFilesToDir(QString path);
double normalDistribution(double average, double sigma);
double uniformDistribution(double min, double max);

bool verifyParameterValue(QString &rValue, const QString type, const QStringList &rSelfParameterNames, const QStringList &rSysParNames, QString &rErrorString);
QStringList getAllAccessibleSystemParameterNames(SystemObject* pSystem);

double findSmallestValueGreaterThanZero(QVector<double> data);
void splitWithRespectToQuotations(const QString &str, const QChar c, QStringList &split);
bool containsOutsideQuotes(const QString &str, const QChar c);
void splitRespectingQuotationsAndParanthesis(const QString str, const QChar c, QStringList &rSplit);
void extractSectionsRespectingQuotationsAndParanthesis(const QString str, const QChar c, QStringList &rSplit, QList<int> &rSectionIndexes);
void extractSections(const QString str, const QChar c, QStringList &rSplit, QList<int> &rSectionIndexes);
void splitOnAny(const QString &str, const QStringList &delims, QStringList &rSplitList);
void santizeName(QString &rString);
bool isNameValid(const QString &rString);
QString extractBetweenFromQString(const QString &rString, const QChar &rFirst, const QChar &rLast);
QVector<int> linspace(const int start, const int stop, const int step=1);
QString extractFilenameExtension(const QString &rFilename);
void splitFilepath(const QString &rFilepath, QString &rDirPath, QString &rFilename, QString &rFilesuffix);
bool isVersionGreaterThan(QString v1, QString v2);
bool replacePattern(const QString &rPattern, const QString &rReplacement, QString &rText);

// In-line utility functions

//! @brief Convenient function to check if a string is numeric
//! @param [in] str The string to check
//! @return true if string can be converted into a double (it  is a number) else returns false
inline bool isNumber(const QString &str)
{
    bool isok;
    str.toDouble(&isok);
    return isok;
}

//! @brief Compare two doubles with given tolerance
//! @return True if qAbs(first-last) < eps
inline bool fuzzyEqual(const double first, const double second, const double eps)
{
    return (qAbs(first-second) < eps);
}

//! @brief Utility function to convert degrees to radians
inline constexpr double deg2rad(const double deg)
{
    return deg*M_PI/180.0;
}

//! @brief Utility function to convert degrees to radians
inline constexpr double rad2deg(const double rad)
{
    return rad*180.0/M_PI;
}

//! @brief Converts a string to Integer returning true if success
//! @param[in] rString The string to convert
//! @param[out] rIntValue The value of the converted int
//! @returns True if conversion was successful
inline bool toInt(const QString &rString, int &rIntValue)
{
    bool isOK=false;
    rIntValue = rString.toInt(&isOK);
    return isOK;
}

//Optimization
void reflectWorst(QVector< QVector<double> > &vector, int worst, double alpha=1.3);
double sum(QVector< QVector<double> > vector, int i);

//! @brief This utility class wraps a QTextStream and have stream operators to write whole lines. You do not need to add the newline char yourself.
class QTextLineStream
{
public:
    QTextLineStream(QTextStream &rTextStream)
    {
        mpQTextSream = &rTextStream;
    }
    friend QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input);

private:
    QTextStream* mpQTextSream;
};

class TicToc : public QTime
{
public:
    enum class TextOutput {Quiet, Qdebug, DebugMessage, InfoMessage};
    explicit TicToc(const TextOutput outType=TextOutput::Qdebug);
    void tic();
    void tic(const QString &text);
    int toc();
    int toc(const QString &text, const int minMs=0);
private:
    void print(const QString &text);
    TextOutput mTextOutput;
};

bool saveXmlFile(QString xmlFilePath, GUIMessageHandler* pMessageHandler, std::function<QDomDocument()> saveFunction, int indentation=XMLINDENTATION);

#endif // GUIUTILITIES_H
