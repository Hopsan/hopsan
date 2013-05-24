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
    bool mlogPar;

    //Paramters
    QVector<OptParameter> mParamters;
    QVector<Objectives> mObjectives;
};


class SystemContainer : public ContainerObject
{
    Q_OBJECT
public:
    SystemContainer(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics);
    SystemContainer(ModelWidget *parentModelWidget, QGraphicsItem *pParent);
    void deleteInHopsanCore();

    double getTimeStep();
    bool doesInheritTimeStep();

    size_t getNumberOfLogSamples();
    void setNumberOfLogSamples(size_t nSamples);

    QString getTypeName() const;
    void setName(QString newName);
    QString getTypeCQS();

    void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents = FullModel);
    void loadFromDomElement(QDomElement &rDomElement);
    void setModelFileInfo(QFile &rFile);
    void loadParameterFile();

    void createLabviewSourceFiles();
    void exportToFMU();
    void exportToFMU(QString savePath);
    void exportToSimulink();
    void exportToSimulinkCoSim();

    // Parameter methods
    QStringList getParameterNames();
    void getParameters(QVector<CoreParameterData> &rParameterDataVec);
    void getParameter(const QString paramName, CoreParameterData &rData);
    QString getParameterValue(const QString paramName);

    CoreSystemAccess* getCoreSystemAccessPtr();
    ContainerObject *getParentContainerObject();

    OptimizationSettings getOptimizationSettings();
    void setOptimizationSettings(OptimizationSettings optSettings);

    enum { Type = SystemContainerType };
    int type() const;



public slots:
    void setTimeStep(const double timeStep);

signals:

protected:
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);

    void openPropertiesDialog();

    void loadOptSettingsFromDomElement(QDomElement &rDomElement);
    void saveOptSettingsToDomElement(QDomElement &rDomElement);

private:
    void commonConstructorCode();

    int mNumberOfLogSamples;

    QString mLoadType;
    CoreSystemAccess *mpCoreSystemAccess;

    QRadioButton *mpExportFmuGccRadioButton;
    QRadioButton *mpExportFmuMsvcRadioButton;

    OptimizationSettings mOptSettings;
};


#endif // GUISYSTEM_H
