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

//! @brief Utility function to convert degrees to radians
qreal rad2deg(qreal rad)
{
    return rad*180.0/M_PI;
}

//! @brief normalises degrees to range between -180 and 180 degrees
qreal normDeg180(qreal deg)
{
    return rad2deg(normRad(deg2rad(deg)));
}

//! @brief normalises degrees to range between 0 and 360 degrees
qreal normDeg360(qreal deg)
{
    while (deg > 360.0 || deg < 0.0)
    {
        if (deg > 360.0)
        {
            deg -= 360.0;
        }
        else
        {
            deg += 360.0;
        }
    }
    return deg;
}

//! @brief normalises radinas to range between -PI and PI degrees
qreal normRad(qreal rad)
{
    return qAtan2(qCos(rad),qSin(rad));
}




QPointF getOffsetPointfromPort(GUIPort *pPort)
{
    QPointF point;

    if((pPort->getPortDirection() == LEFTRIGHT) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).x() > pPort->scenePos().x()))
    {
        point.setX(-20);
    }
    else if((pPort->getPortDirection() == LEFTRIGHT) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).x() < pPort->scenePos().x()))
    {
        point.setX(20);
    }
    else if((pPort->getPortDirection() == TOPBOTTOM) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).y() > pPort->scenePos().y()))
    {
        point.setY(-20);
    }
    else if((pPort->getPortDirection() == TOPBOTTOM) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).y() < pPort->scenePos().y()))
    {
        point.setY(20);
    }
    return point;
}

double dist(double x1,double y1, double x2, double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}
