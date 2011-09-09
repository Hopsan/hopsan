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

#include "GUIContainerObject.h"
#include "../common.h"

//Forward Declaration
class ProjectTab;


class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem( QPointF position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *pParentContainer, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS);
    GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *pParent);
    ~GUISystem();

    double getStartTime();
    double getTimeStep();
    double getStopTime();

    size_t getNumberOfLogSamples();
    void setNumberOfLogSamples(size_t nSamples);

    QString getTypeName();
    void setName(QString newName);
    //void setTypeCQS(QString typestring);
    QString getTypeCQS();

    void saveToDomElement(QDomElement &rDomElement);
    //void loadFromHMF(QString modelFilePath=QString());
    void loadFromDomElement(QDomElement &rDomElement);
    void setModelFileInfo(QFile &rFile);

    void saveToWrappedCode();
    void createFMUSourceFiles();
    void createSimulinkSourceFiles();

    QVector<QString> getParameterNames();

    CoreSystemAccess* getCoreSystemAccessPtr();
    GUIContainerObject *getParentContainerObject();

    enum { Type = GUISYSTEM };
    int type() const;

public slots:
    void updateStartTime();
    void updateTimeStep();
    void updateStopTime();
    void updateSimulationParametersInToolBar();

signals:

protected:
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    void saveCoreDataToDomElement(QDomElement &rDomElement);

    void openPropertiesDialog();

private:
    void commonConstructorCode();

    double mStartTime;
    double mStopTime;
    double mTimeStep;
    size_t mNumberOfLogSamples;

    QString mLoadType;
    CoreSystemAccess *mpCoreSystemAccess;
};

#endif // GUISYSTEM_H
