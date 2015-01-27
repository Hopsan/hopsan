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

    int mNiter;
    int mNsearchp;
    double mRefcoeff;
    double mRandfac;
    double mForgfac;
    double mFunctol;
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
    void setModelFileInfo(QFile &rFile);
    void loadParameterFile(const QString &path="");

    void exportToLabView();
    void exportToFMUME32();
    void exportToFMUME64();
    void exportToFMUCS32();
    void exportToFMUCS64();
    void exportToFMU(QString savePath, bool me, CoreGeneratorAccess::TargetArchitectureT arch);
    void exportToSimulink();
    void exportToSimulinkCoSim();

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

    void openPropertiesDialog();

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
