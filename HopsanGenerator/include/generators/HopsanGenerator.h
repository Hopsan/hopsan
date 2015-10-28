/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HopsanGenerator.h
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains the Hopsan generator class
//!
//$Id$


#ifndef HOPSANGENERAETOR_H
#define HOPSANGENERAETOR_H

#ifdef _WIN32
#define LIBEXT ".dll"
#define LIBPREFIX ""
#else
#define LIBEXT ".so"
#define LIBPREFIX "lib"
#endif

#include <QPointF>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDomElement>

#include "win32dll.h"
#include "SymHop.h"
#include "GeneratorUtilities.h"

//class ModelObjectAppearance;

// Forward declarations
#ifdef USEQTGUI
class QTextEdit;
class QVBoxLayout;
class QDialog;
class QPushButton;
#endif

namespace hopsan {
class ComponentSystem;
}


class DLLIMPORTEXPORT HopsanGenerator
{
public:
    enum SolverT {NumericalIntegration, BilinearTransform};

    HopsanGenerator(const QString coreIncludePath, const QString binPath, const QString gccPath, const bool showDialog=false);
    void setOutputPath(const QString path);
    void setTarget(const QString fileName);
    QString getCoreIncludePath() const;
    QString getBinPath() const;
    QString getHopsanRootPath() const;
    QString getGccPath() const;
    void printMessage(const QString &msg, const QString &color="Black") const;
    void printWarningMessage(const QString &msg) const;
    void printErrorMessage(const QString &msg) const;
    void compileFromComponentObject(const QString &outputFile, const ComponentSpecification &comp, const bool overwriteStartValues=false, const QString customSourceFile="");
    void generateNewLibrary(QString dstPath, QStringList hppFiles, QStringList cflags=QStringList(), QStringList lflags=QStringList());
    bool generateCafFile(QString &rPath, ComponentAppearanceSpecification &rCafSpec);

protected:

    QString generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues=false) const;
    void generateOrUpdateComponentAppearanceFile(QString path, ComponentSpecification comp, QString sourceFile=QString());
    bool assertFilesExist(const QString &path, const QStringList &files) const;
    void callProcess(const QString &name, const QStringList &args, const QString workingDirectory=QString()) const;
    bool runUnixCommand(QString cmd) const;
    bool replaceInFile(const QString &fileName, const QStringList &before, const QStringList &after) const;
    bool copyHopsanCoreSourceFilesToDir(QString tgtPath) const;
    bool copyDefaultComponentCodeToDir(const QString &path) const;
    bool copyBoostIncludeFilesToDir(const QString &path) const;
    bool copyFile(const QString &source, const QString &target) const;
    bool copyDir(const QString fromPath, const QString toPath) const;
    void cleanUp(const QString &path, const QStringList &files, const QStringList &subDirs) const;
    void getNodeAndCqTypeFromInterfaceComponent(const QString &compType, QString &nodeType, QString &cqType);


    QString mOutputPath;
    QString mTarget;        //Name of HMF file, empty by default, only used if not empty
    QString mTempPath;
    QString mCoreIncludePath;
    QString mBinPath;
    QString mHopsanRootPath;
    QString mGccPath;

#ifdef USEQTGUI
    QTextEdit *mpTextEdit;
    QVBoxLayout *mpLayout;
    QPushButton *mpDoneButton;
    QWidget *mpDialog;
    QDialog *mpPortsDialog;
#endif

    bool mShowDialog;
};


#endif // HOPSANGENERAETOR_H
