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
//! @file   GUIComponent.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI class representing Components
//!
//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include "GUIModelObject.h"
#include "../common.h"
#include "../Utilities/XMLUtilities.h"
#include <assert.h>

//Forward declarations
class GUIConnector;
class GUIPort;
class GUIContainerObject;

class GUIComponent : public GUIModelObject
{
    Q_OBJECT

public:
    GUIComponent(QPointF position, qreal rotation, GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *pParentContainer, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS);
    ~GUIComponent();

    void getParameters(QVector<QString> &qParameterNames, QVector<QString> &qParameterValues, QVector<QString> &qDescriptions, QVector<QString> &qUnits, QVector<QString> &qTypes);
    bool hasPowerPorts();
    QVector<QString> getParameterNames();
    QString getParameterUnit(QString name);
    QString getParameterDescription(QString name);
    QString getParameterValue(QString name);
//    QString getParameterValueTxt(QString name);
    bool setParameterValue(QString name, QString sysParName, bool force=0);
    QString getStartValueTxt(QString portName, QString variable);
    bool setStartValue(QString portName, QString variable, QString sysParName);

    //void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    QString getTypeName();
    QString getTypeCQS();

    enum { Type = GUICOMPONENT };
    int type() const;

private slots:
    virtual void setVisible(bool visible);

protected:
    void saveCoreDataToDomElement(QDomElement &rDomElement);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openPropertiesDialog();
    void createPorts();
};

#endif // GUICOMPONENT_H
