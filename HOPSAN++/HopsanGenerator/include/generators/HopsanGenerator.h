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
//! @file   ComponentGeneratorUtilities.h
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utiluties
//!
//$Id$


#ifndef COMPONENTGENERATORUTILITIES_H
#define COMPONENTGENERATORUTILITIES_H

#include <QPointF>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QDomElement>
#include "win32dll.h"

#include "SymHop.h"
#include "GeneratorUtilities.h"

//class ModelObjectAppearance;

namespace hopsan {
class ComponentSystem;
}


class DLLIMPORTEXPORT HopsanGenerator
{
public:
    enum SolverT {BilinearTransform, NumericalIntegration};

    HopsanGenerator(const QString coreIncludePath, const QString binPath, const bool showDialog=false);
    void setOutputPath(const QString path);
    void setTarget(const QString fileName);
    QString getCoreIncludePath() const;
    QString getBinPath() const;
    void printMessage(const QString &msg) const;
    void printErrorMessage(const QString &msg) const;
    void compileFromComponentObject(const QString &outputFile, const ComponentSpecification &comp, const bool overwriteStartValues=false, const QString customSourceFile="");

protected:

    QString generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues=false) const;
    bool assertFilesExist(const QString &path, const QStringList &files) const;
    void callProcess(const QString &name, const QStringList &args, const QString workingDirectory=QString()) const;
    bool runUnixCommand(QString cmd) const;
    bool replaceInFile(const QString &fileName, const QStringList &before, const QStringList &after) const;
    bool copyIncludeFilesToDir(QString path, bool skipDependencies=false) const;
    bool copySourceFilesToDir(QString path) const;
    bool copyDefaultComponentCodeToDir(const QString &path) const;
    bool copyBoostIncludeFilesToDir(const QString &path) const;
    bool copyFile(const QString &source, const QString &target) const;
    void cleanUp(const QString &path, const QStringList &files, const QStringList &subDirs) const;
    void getNodeAndCqTypeFromInterfaceComponent(const QString &compType, QString &nodeType, QString &cqType);


    QString mOutputPath;
    QString mTarget;        //Name of HMF file, empty by default, only used if not empty
    QString mTempPath;
    QString mCoreIncludePath;
    QString mBinPath;

    QTextEdit *mpTextEdit;
    QVBoxLayout *mpLayout;
    QPushButton *mpDoneButton;
    QWidget *mpDialog;

    bool mShowDialog;
};


#endif // COMPONENTGENERATORUTILITIES_H
