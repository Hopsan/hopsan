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
//$Id: XMLUtilities.cpp 6763 2014-03-20 16:26:00Z petno25 $

#include "XMLUtilities.h"
#include <QMessageBox>
#include <QLocale>

//! @brief Help function to get "correct" string representation of bool
QString bool2str(const bool in)
{
    if (in)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

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
                                 QString(rFile.fileName() + ": Parse error at line %1, column %2:\n%3")
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
                                     QString("The file has the wrong Root Tag Name: ")
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

//! @brief Function to get a sub dom ellement, if it does not exist it is first added
QDomElement getOrAppendNewDomElement(QDomElement &rDomElement, const QString element_name)
{
    QDomElement elem = rDomElement.firstChildElement(element_name);
    if (elem.isNull())
    {
        return appendDomElement(rDomElement, element_name);
    }
    else
    {
        return elem;
    }
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
        appendDomTextNode(rDomElement, element_name, "true");
    }
    else
    {
        appendDomTextNode(rDomElement, element_name, "false");
    }
}

//! @brief Function for adding Dom elements containing one text node (based on an integer value)
//! @param[in] rDomElement The DOM Element to add to
//! @param[in] element_name The name of the new DOM element
//! @param[in] val The double value
void appendDomIntegerNode(QDomElement &rDomElement, const QString element_name, const int val)
{
    QString tmp_string;
    tmp_string.setNum(val);
    appendDomTextNode(rDomElement, element_name, tmp_string);
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
void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const qreal a, const qreal b)
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

//! @brief Helpfunction for adding a qreal (float or double) attribute to a Dom element while making sure that decimal point is . and not ,
//! @param domElement The DOM element to add the attribute to
//! @param[in] attrName The name of the attribute
//! @param[in] attrValue The value of the attribute
void setQrealAttribute(QDomElement domElement, const QString attrName, const qreal attrValue, const int precision, const char format)
{
    QString str;
    domElement.setAttribute(attrName, str.setNum(attrValue, format, precision));
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
double parseDomValueNode(QDomElement domElement, const double defaultValue)
{
    if ( !domElement.isNull() )
    {
        bool isOk;
        double val = domElement.text().toDouble(&isOk);
        if (isOk)
        {
            return val;
        }
    }
    // If dom was null or if parsing failed we return the default value
    return defaultValue;
}

//! @brief Function that parses one DOM elements containing one text node (based on an integer value)
//! @param[in] domElement The DOM Element to parse
//! @param[in] defaultVal The default value to use if DOM element is null or if parsing fails
//! @returns The extracted value
int parseDomIntegerNode(QDomElement domElement, const int defaultValue)
{
    if ( !domElement.isNull() )
    {
        bool isOk;
        int val = domElement.text().toInt(&isOk);
        if (isOk)
        {
            return val;
        }
    }
    // If dom was null or if parsing failed we return the default value
    return defaultValue;
}


//! @brief Function that parses one DOM elements containing one text node (based on a boolean value)
//! @param[in] domElement The DOM Element to parse
//! @param[in] defaultVal The default value to use if DOM element is null
//! @returns The extracted boolean value
bool parseDomBooleanNode(QDomElement domElement, const bool defaultValue)
{
    // If dom element is null then return default value
    if ( domElement.isNull() )
    {
        return defaultValue;
    }
    // else check if it is true or not (something else = false)
    return (domElement.text() == "true");
}




qreal parseAttributeQreal(const QDomElement domElement, const QString attributeName, const qreal defaultValue)
{
    if (domElement.hasAttribute(attributeName))
    {
        bool isOK=false;
        const qreal val = domElement.attribute(attributeName).toDouble(&isOK);
        if (isOK)
        {
            return val;
        }
    }
    return defaultValue;
}

bool parseAttributeBool(const QDomElement domElement, const QString attributeName, const bool defaultValue)
{
    QString attr = domElement.attribute(attributeName, bool2str(defaultValue));
    if ( (attr=="true") || (attr=="True") || (attr=="1"))
    {
        return true;
    }

    return false;
}

//! @brief Converts a color to a string of syntax "R,G,B".
QString makeRgbString(QColor color)
{
    QString red, green,blue;
    red.setNum(color.red());
    green.setNum(color.green());
    blue.setNum(color.blue());
    return QString(red+","+green+","+blue);
}

//! @brief Converts a string of syntax "R,G,B" into three numbers for red, green and blue.

void parseRgbString(QString rgb, double &red, double &green, double &blue)
{
    QStringList split = rgb.split(",");
    red = split[0].toDouble();
    green = split[1].toDouble();
    blue = split[2].toDouble();
}


int parseAttributeInt(const QDomElement domElement, const QString attributeName, const int defaultValue)
{
    if (domElement.hasAttribute(attributeName))
    {
        bool isOK=false;
        const int val = domElement.attribute(attributeName).toInt(&isOK);
        if (isOK)
        {
            return val;
        }
    }
    return defaultValue;
}
