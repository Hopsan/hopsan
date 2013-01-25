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
    HopsanGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void printMessage(QString msg);
    void printErrorMessage(QString msg);
    void generateFromModelica(QString code);
    void generateFromFmu(QString code);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem);
    void generateToSimulink(QString savePath, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler);
    void generateToSimulinkCoSim(QString savePath, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler);
    void generateToLabViewSIT(QString savePath, hopsan::ComponentSystem *pSystem);
    void compileFromComponentObject(QString outputFile, ComponentSpecification comp, bool overwriteStartValues=false);

private:
    void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList);
    void generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList);
    QString generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues=false);

    bool verifyParameteres(QList<ParameterSpecification> parameters);
    bool verifyPorts(QList<PortSpecification> ports);
    bool verifyUtilities(QList<UtilitySpecification> utilities);
    bool verifyStaticVariables(QList<StaticVariableSpecification> variables);
    bool verifyEquationSystem(QList<SymHop::Expression> equations, QList<SymHop::Expression> stateVars);
    QStringList getQVariables(QString nodeType);
    QStringList getCVariables(QString nodeType);
    QStringList getVariableLabels(QString nodeType);
    inline QString toVarName(const QString org);
    QString extractTaggedSection(QString str, QString tag);
    void replaceTaggedSection(QString &str, QString tag, QString replacement);
    QString replaceTag(QString str, QString tag, QString replacement);
    QString replaceTags(QString str, QStringList tags, QStringList replacements);

    QString mOutputPath;
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
