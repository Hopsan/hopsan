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
//! @file   GUIUtilities.cpp
//! @author All <flumes@liu.se>
//! @date   2010-10-09
//!
//! @brief Contains a class for misc utilities
//!
//$Id$

#include <cmath>
#include <QPoint>
#include <QDir>

#include "GUIUtilities.h"
#include "GUIPort.h"
#include "GUIObject.h"

using namespace std;

//! @brief This function extracts the name from a text stream
//! @return The extracted name without quotes, empty string if failed
//! It is assumed that the name was saved OK. but error indicated by empty string
QString readName(QTextStream &rTextStream)
{
    QString tempName;
    rTextStream >> tempName;
    if (tempName.startsWith("\""))
    {
        while (!tempName.endsWith("\""))
        {
            if (rTextStream.atEnd())
            {
                return QString(""); //Empty string (failed)
            }
            else
            {
                QString tmpstr;
                rTextStream >> tmpstr;
                tempName.append(" " + tmpstr);
            }
        }
        return tempName.remove("\"").trimmed(); //Remove quotes and trimm (just to be sure)
    }
    else
    {
        return QString(""); //Empty string (failed)
    }
}


//! @brief Convenience function if you dont have a stream to read from
//! @return The extracted name without quotes, empty string if failed
//! It is assumed that the name was saved OK. but error indicated by empty string
QString readName(QString namestring)
{
    QTextStream namestream(&namestring);
    return readName(namestream);
}


//! @brief This function may be used to add quotes around string, usefull for saving names. Ex: "string"
QString addQuotes(QString str)
{
    str.prepend("\"");
    str.append("\"");
    return str;
}

//! @brief This function returns the relative absolute path
//! @param [in] pathtochange The absolute path theat you want to change
//! @param [in] basedirpath The absolute directory path of the base directory
//! @returns The realtive pathtochange, relative to basepath
QString relativePath(QString pathtochange, QString basedirpath)
{
    QDir basedir(basedirpath);
    return basedir.relativeFilePath(pathtochange);
}

//! @brief Utility function to convert degrees to radians
qreal deg2rad(qreal deg)
{
    return deg*M_PI/180.0;
}




QPointF getOffsetPointfromPort(GUIPort *pPort)
{
    QPointF point;

    if((pPort->getPortDirection() == LEFTRIGHT) && (pPort->getGuiObject()->mapToScene(pPort->getGuiObject()->boundingRect().center()).x() > pPort->scenePos().x()))
    {
        point.setX(-20);
    }
    else if((pPort->getPortDirection() == LEFTRIGHT) && (pPort->getGuiObject()->mapToScene(pPort->getGuiObject()->boundingRect().center()).x() < pPort->scenePos().x()))
    {
        point.setX(20);
    }
    else if((pPort->getPortDirection() == TOPBOTTOM) && (pPort->getGuiObject()->mapToScene(pPort->getGuiObject()->boundingRect().center()).y() > pPort->scenePos().y()))
    {
        point.setY(-20);
    }
    else if((pPort->getPortDirection() == TOPBOTTOM) && (pPort->getGuiObject()->mapToScene(pPort->getGuiObject()->boundingRect().center()).y() < pPort->scenePos().y()))
    {
        point.setY(20);
    }
    return point;
}
