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
//! @file   GUISystem.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI System class, representing system components
//!
//$Id$

#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#include <QGraphicsWidget>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QPoint>
#include <QFileInfo>
#include <QRadioButton>

#include "GUIContainerObject.h"
#include "common.h"

//Forward Declaration
class ModelWidget;


class OptParameter
{
public:
    QString mComponentName, mParameterName;
    double mMin, mMax;
};


class Objectives
{
public:
    QString mFunctionName;
    QStringList mData;
    QList<QStringList> mVariableInfo;
    double mWeight, mNorm, mExp;
};


class OptimizationSettings
{
public:
    OptimizationSettings();

    QString mScriptFile;
    int mNiter;
    int mNsearchp;
    double mRefcoeff;
    double mRandfac;
    double mForgfac;
    double mPartol;
    bool mPlot;
    bool mSavecsv;
    bool mFinalEval;
    bool mlogPar;

    //Parameters
    QVector<OptParameter> mParamters;
    QVector<Objectives> mObjectives;
};


class SensitivityAnalysisParameter
{
public:
    QString compName, parName;
    double min, max;      //Used for square distribution
    double aver, sigma;   //Used for normal distribution
};


class SensitivityAnalysisVariable
{
public:
    QString compName, portName, varName;
};


class SensitivityAnalysisSettings
{
public:
    SensitivityAnalysisSettings();

    enum DistributionEnumT { UniformDistribution, NormalDistribution };

    int nIter;     //Number of iterations
    QVector<SensitivityAnalysisParameter> parameters;
    QVector<SensitivityAnalysisVariable> variables;
    DistributionEnumT distribution;
};


class SystemContainer : public ContainerObject
{
    Q_OBJECT
public:
    SystemContainer(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);
    SystemContainer(ModelWidget *parentModelWidget, QGraphicsItem *pParent);
    void deleteInHopsanCore();

    double getTimeStep();
    bool doesInheritTimeStep();

    size_t getNumberOfLogSamples();
    void setNumberOfLogSamples(size_t nSamples);
    double getLogStartTime() const;
    void setLogStartTime(const double logStartT);

    QString getTypeName() const;
    void setName(QString newName);
    QString getTypeCQS();

    void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents = FullModel);
    void loadFromDomElement(QDomElement domElement);
    void setModelFileInfo(QFile &rFile, const QString relModelPath="");
    void loadParameterFile(const QString &path="");

    void exportToLabView();
    void exportToFMU1_32();
    void exportToFMU1_64();
    void exportToFMU2_32();
    void exportToFMU2_64();
    void exportToFMU(QString savePath, int version, ArchitectureEnumT arch);
    void exportToSimulink();

    // Parameter methods
    QStringList getParameterNames();
    void getParameters(QVector<CoreParameterData> &rParameterDataVec);
    void getParameter(const QString paramName, CoreParameterData &rData);
    QString getParameterValue(const QString paramName);
    bool hasParameter(const QString &rParamName);

    CoreSystemAccess* getCoreSystemAccessPtr();
    ContainerObject *getParentContainerObject();


    void getSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings);
    void setSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings);
    void getOptimizationSettings(OptimizationSettings &optSettings);
    void setOptimizationSettings(OptimizationSettings &optSettings);

    // Type info
    enum { Type = SystemContainerType };
    int type() const;
    virtual QString getHmfTagName() const;


public slots:
    void setTimeStep(const double timeStep);
    void setVisibleIfSignal(bool visible);

signals:

protected:
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);

    //Protected overloaded Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void loadSensitivityAnalysisSettingsFromDomElement(QDomElement &rDomElement);
    void saveSensitivityAnalysisSettingsToDomElement(QDomElement &rDomElement);
    void loadOptimizationSettingsFromDomElement(QDomElement &rDomElement);
    void saveOptimizationSettingsToDomElement(QDomElement &rDomElement);

private:
    void commonConstructorCode();

    int mNumberOfLogSamples;
    double mLogStartTime;

    QString mLoadType;
    CoreSystemAccess *mpCoreSystemAccess;

    QRadioButton *mpExportFmuGccRadioButton;
    QRadioButton *mpExportFmuMsvcRadioButton;

    OptimizationSettings mOptSettings;
    SensitivityAnalysisSettings mSensSettings;
};

#endif // GUISYSTEM_H
