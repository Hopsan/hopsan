/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   AppearanceData.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-04-22
//!
//! @brief Contains appearance data to be used by guiobjects and library widget
//!
//$Id$

#ifndef APPEARANCEDATA_H
#define APPEARANCEDATA_H

#include <QTextStream>
#include <QString>
#include <QPointF>
#include <QVector>
#include <QMap>
#include <QPen>
#include "common.h"

class GUIConnectorAppearance
{
public:
    //GUIConnectorAppearance();
    GUIConnectorAppearance(QString porttype, graphicsType gfxType);
    void setType(const QString porttype);
    void setIsoStyle(graphicsType gfxType);
    void setTypeAndIsoStyle(QString porttype, graphicsType gfxType);
    QPen getPen(QString situation, QString type, graphicsType gfxType);
    QPen getPen(QString situation);
    void adjustToZoom(qreal zoomFactor);

private:
    QPen mPrimaryPenPowerUser;
    QPen mActivePenPowerUser;
    QPen mHoverPenPowerUser;
    QPen mPrimaryPenSignalUser;
    QPen mActivePenSignalUser;
    QPen mHoverPenSignalUser;
    QPen mPrimaryPenPowerIso;
    QPen mActivePenPowerIso;
    QPen mHoverPenPowerIso;
    QPen mPrimaryPenSignalIso;
    QPen mActivePenSignalIso;
    QPen mHoverPenSignalIso;
    QPen mNonFinishedPen;

    QString mConnectorType;
    graphicsType mGfxType;

};

class PortAppearance
{
public:
    void selectPortIcon(QString cqstype, QString porttype, QString nodetype);

    qreal x;
    qreal y;
    qreal rot;
    QString iconPath;
    portDirection direction;
};

typedef QMap<QString, PortAppearance> PortAppearanceMapT;

class AppearanceData
{
public:
    AppearanceData();
    void setTypeName(QString name);
    void setName(QString name);
    void setBasePath(QString path);
    void setIconPathUser(QString path);
    void setIconPathISO(QString path);

    QString getTypeName();
    QString getName();
    QString getFullIconPath(graphicsType gfxType=USERGRAPHICS);
    QString getIconPathUser();
    QString getIconPathISO();
    QString getIconRotationBehaviour();
    QPointF getNameTextPos();
    size_t  getNumberOfPorts();
    PortAppearanceMapT &getPortAppearanceMap();

    bool haveIsoIcon();
    bool haveUserIcon();

    QString getBasePath();

    bool setAppearanceData(QTextStream &is);

    friend QTextStream& operator >>(QTextStream &is, AppearanceData &rData);
    friend QTextStream& operator <<(QTextStream &os, AppearanceData &rData);

    //This bool signals wheter there were errors when reading the data from input stream
    bool mIsOK;

private:
    QString mTypeName;
    QString mName;
    QString mIconPathUser;
    QString mIconPathISO;
    QString mIconRotationBehaviour;
    qreal mRotation;
    QPointF mNameTextPos;

    PortAppearanceMapT mPortAppearanceMap;

    //BaseDir for path strings, mayb should not be stored in here
    QString mBasePath;

};



#endif // APPEARANCEDATA_H
