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
//! @file   XMLUtilities.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-xx
//!
//! @brief Contains XML DOM help functions that are more or less Hopsan specific
//!
//$Id$

#include "XMLUtilities.h"
#include <QMessageBox>

//! @brief Function for loading an XML DOM Documunt from file
//! @param[in] rFile The file to load from
//! @param[in] rDomDocument The DOM Document to load into
//! @param[in] rootTagName The expected root tag name to extract from the Dom Document
//! @returns The extracted DOM root element from the loaded DOM document
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName)
{
    QString errorStr;
    int errorLine, errorColumn;
    if (!rDomDocument.setContent(&rFile, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(0, "Hopsan GUI",
                                 QString("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement xmlRoot = rDomDocument.documentElement();
        if (xmlRoot.tagName() != rootTagName)
        {
            QMessageBox::information(0, "Hopsan GUI",
                                     QString("The file is has the wrong Root Tag Name: ")
                                     + xmlRoot.tagName() + "!=" + rootTagName);
        }
        else
        {
            return xmlRoot;
        }
    }
    return QDomElement(); //NULL
}

//! @brief Appends xml processing instructions before the root tag in a DOM Document, run this ONCE before writing to file
//! @param[in] rDomDocument The DOM Document to append ProcessingInstructions to
void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument)
{
    QDomNode xmlProcessingInstruction = rDomDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    rDomDocument.insertBefore(xmlProcessingInstruction, rDomDocument.firstChild());
}

//! @brief This function adds the root alement including version info to a HMF xml document
//! @param[in] rDomDocument The XML DOM document to append to
//! @param[in] hmfVersion The version string of the hmf file
//! @param[in] hopsanGuiVersion The version string of the Hopsan GUI Application
//! @param[in] hopsanCoreVersion The version string of the Hopsan Core Library
//! @returns The created root DOM element
QDomElement appendHMFRootElement(QDomDocument &rDomDocument, QString hmfVersion, QString hopsanGuiVersion, QString hopsanCoreVersion)
{
    QDomElement hmfRoot = rDomDocument.createElement(HMF_ROOTTAG);
    rDomDocument.appendChild(hmfRoot);
    hmfRoot.setAttribute(HMF_HMFVERSIONTAG, hmfVersion);
    hmfRoot.setAttribute(HMF_HOPSANGUIVERSIONTAG, hopsanGuiVersion);
    hmfRoot.setAttribute(HMF_HOPSANCOREVERSIONTAG, hopsanCoreVersion);
    return hmfRoot;
}

//! @brief Function for adding one initially empty Dom Element to extingd Element
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] element_name The name of the new DOM element
//! @returns The new sub element dom node
QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name)
{
    QDomElement subDomElement = rDomElement.ownerDocument().createElement(element_name);
    rDomElement.appendChild(subDomElement);
    return subDomElement;
}

//! @brief Function for adding Dom elements containing one text node
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] text The text contents of the text node
void appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text)
{
    //Only write tag if both name and value is non empty
    if (!element_name.isEmpty() && !text.isEmpty())
    {
        QDomDocument ownerDomDocument = rDomElement.ownerDocument();
        QDomElement subDomElement = ownerDomDocument.createElement(element_name);
        subDomElement.appendChild(ownerDomDocument.createTextNode(text));
        rDomElement.appendChild(subDomElement);
    }
}

//! @brief Function for adding Dom elements containing one boolean text node
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] value The boolen value
void appendDomBooleanNode(QDomElement &rDomElement, const QString element_name, const bool value)
{
    if(value)
    {
        appendDomTextNode(rDomElement, element_name, HMF_TRUETAG);
    }
    else
    {
        appendDomTextNode(rDomElement, element_name, HMF_FALSETAG);
    }
}

//! @brief Function for adding Dom elements containing one text node (based on a double value)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] val The double value
void appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val)
{
    QString tmp_string;
    tmp_string.setNum(val);
    appendDomTextNode(rDomElement, element_name, tmp_string);
}

//! @brief Function for adding Dom elements containing one text node (based on two double values)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] a The first double value
//! @param[in] b The second double value
void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b)
{
    QString num,str;
    num.setNum(a);
    str.append(num);
    str.append(" ");
    num.setNum(b);
    str.append(num);
    appendDomTextNode(rDomElement, element_name, str);
}

//! @brief Function for adding Dom elements containing one text node (based on three double values)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] a The first double value
//! @param[in] b The second double value
//! @param[in] c The theird double value
void appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c)
{
    QString num,str;
    num.setNum(a);
    str.append(num);
    str.append(" ");
    num.setNum(b);
    str.append(num);
    str.append(" ");
    num.setNum(c);
    str.append(num);
    appendDomTextNode(rDomElement, element_name, str);
}

//! @brief Function for adding Dom elements containing one text node (based on N double values)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] rValues A QVector containing all of the values to add
void appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<qreal> &rValues)
{
    QString num,str;
    for (int i=0; i<rValues.size(); ++i)
    {
        num.setNum(rValues[i]);
        str.append(num);
        str.append(" ");
    }
    str.chop(1); //Remove last space
    appendDomTextNode(rDomElement, element_name, str);
}


//! @brief Function that parses one DOM elements containing one text node (based on three double values)
//! @param[in] domElement The DOM Element to parse
//! @param[out] rA The first extracted double value
//! @param[out] rB The second extracted double value
//! @param[out] rC The theird extracted double value
void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC)
{
    QStringList poseList = domElement.text().split(" ");
    rA = poseList[0].toDouble();
    rB = poseList[1].toDouble();
    rC = poseList[2].toDouble();
}

//! @brief Function that parses one DOM elements containing one text node (based on two double values)
//! @param[in] domElement The DOM Element to parse
//! @param[out] rA The first extracted double value
//! @param[out] rB The second extracted double value
void parseDomValueNode2(QDomElement domElement, double &rA, double &rB)
{
    QStringList poseList = domElement.text().split(" ");
    rA = poseList[0].toDouble();
    rB = poseList[1].toDouble();
}

//! @brief Function that parses one DOM elements containing one text node (based on a double value)
//! @param[in] domElement The DOM Element to parse
//! @returns The extracted value
qreal parseDomValueNode(QDomElement domElement)
{
    return domElement.text().toDouble();
}

//! @brief Function that parses one DOM elements containing one text node (based on a boolean value)
//! @param[in] domElement The DOM Element to parse
//! @returns The extracted boolean value
bool parseDomBooleanNode(QDomElement domElement)
{
    return (domElement.text() == HMF_TRUETAG);
}

//! @brief Special purpose function for adding a Hopsan specific XML tag containing Object Pose information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
//! @param[in] th The orientaion (angle)
//! @param[in] flipped isFlipped status of the object
void appendPoseTag(QDomElement &rDomElement, qreal x, qreal y, qreal th, bool flipped)
{
    QDomElement pose = appendDomElement(rDomElement, HMF_POSETAG);
    pose.setAttribute("x",x);
    pose.setAttribute("y",y);
    pose.setAttribute("a",th);
    pose.setAttribute("flipped", flipped);
}

//! @brief Special purpose function for adding a Hopsan specific XML tag containing PortPose information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] name The port name
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
//! @param[in] th The orientaion (angle)
void appendPortPoseTag(QDomElement &rDomElement, QString name, qreal x, qreal y, qreal th)
{
    QDomElement pose = appendDomElement(rDomElement, HMF_PORTPOSETAG);
    pose.setAttribute("name",name);
    pose.setAttribute("x",x);
    pose.setAttribute("y",y);
    pose.setAttribute("a",th);
}

//! @brief Special purpose function for adding a Hopsan specific XML tag containing a coordinate
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
void appendCoordinateTag(QDomElement &rDomElement, qreal x, qreal y)
{
    QDomElement pose = appendDomElement(rDomElement, HMF_COORDINATETAG);
    pose.setAttribute("x",x);
    pose.setAttribute("y",y);
}

//! @brief Special purpose help function for adding a Hopsan specific XML tag containing viewport information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] x The x coordinate
//! @param[in] y The y coordinate
//! @param[in] zoom The zoom factor
void appendViewPortTag(QDomElement &rDomElement, qreal x, qreal y, qreal zoom)
{
    QDomElement pose = appendDomElement(rDomElement, HMF_VIEWPORTTAG);
    pose.setAttribute("x",x);
    pose.setAttribute("y",y);
    pose.setAttribute("zoom",zoom);
}

//! @brief Special purpose help function for adding a Hopsan specific XML tag containing simulationtime information
//! @param[in] rDomElement The DOM Element to append to
//! @param[in] start The starttime
//! @param[in] step The timestep size
//! @param[in] stop The stoptime
void appendSimulationTimeTag(QDomElement &rDomElement, qreal start, qreal step, qreal stop)
{
    QDomElement simu = appendDomElement(rDomElement, HMF_SIMULATIONTIMETAG);
    simu.setAttribute("start", start);
    simu.setAttribute("timestep", step);
    simu.setAttribute("stop", stop);
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing Object Pose information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rTheta The orientaion (angle)
//! @param[out] rFlipped isFlipped status of the object
void parsePoseTag(QDomElement domElement, qreal &rX, qreal &rY, qreal &rTheta, bool &rFlipped)
{
    rX = domElement.attribute("x").toDouble();
    rY = domElement.attribute("y").toDouble();
    rTheta = domElement.attribute("a").toDouble();
    rFlipped = (domElement.attribute("flipped") == "1");
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing PortPose information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rName The name of the port
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rTheta The orientaion (angle)
void parsePortPoseTag(QDomElement domElement, QString &rName, qreal &rX, qreal &rY, qreal &rTheta)
{
    rName = domElement.attribute("name");
    bool dummy;
    parsePoseTag(domElement, rX, rY, rTheta, dummy);
}

//! @brief Special purpose function for parsing a Hopsan specific XML tag containing a coordinate
//! @param[in] domElement The DOM Element to parse
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
void parseCoordinateTag(QDomElement domElement, qreal &rX, qreal &rY)
{
    rX = domElement.attribute("x").toDouble();
    rY = domElement.attribute("y").toDouble();
}

//! @brief Special purpose help function for parsing a Hopsan specific XML tag containing viewport information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rX The x coordinate
//! @param[out] rY The y coordinate
//! @param[out] rZoom The zoom factor
void parseViewPortTag(QDomElement domElement, qreal &rX, qreal &rY, qreal &rZoom)
{
    rX = domElement.attribute("x").toDouble();
    rY = domElement.attribute("y").toDouble();
    rZoom = domElement.attribute("zoom").toDouble();
}

//! @brief Special purpose help function for parsing a Hopsan specific XML tag containing simulationtime information
//! @param[in] domElement The DOM Element to parse
//! @param[out] rStart The starttime
//! @param[out] rStep The timestep size
//! @param[out] rStop The stoptime
void parseSimulationTimeTag(QDomElement domElement, qreal &rStart, qreal &rStep, qreal &rStop)
{
    rStart = domElement.attribute("start").toDouble();
    rStep = domElement.attribute("timestep").toDouble();
    rStop = domElement.attribute("stop").toDouble();
}


QString makeRgbString(QColor color)
{
    QString red, green,blue;
    red.setNum(color.red());
    green.setNum(color.green());
    blue.setNum(color.blue());
    return QString(red+","+green+","+blue);
}


void parseRgbString(QString rgb, double &red, double &green, double &blue)
{
    QStringList split = rgb.split(",");
    red = split[0].toDouble();
    green = split[1].toDouble();
    blue = split[2].toDouble();
}


//! @todo Add check for separate orifice areas in 4/3 valve
void verifyHmfSubComponentCompatibility(QDomElement &element, double hmfVersion)
{
    if(hmfVersion <= 0.2)
    {
        if(element.attribute("typename") == "HydraulicPressureSource")
        {
            element.setAttribute("typename", "HydraulicPressureSourceC");
        }
    }
}
