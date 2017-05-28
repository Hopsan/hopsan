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
//! @file   XMLUtilities.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-xx
//!
//! @brief Contains XML DOM help functions that are more or less Hopsan specific
//!
//$Id$

#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include <QtXml>

QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);

void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument);

QDomElement getOrAppendNewDomElement(QDomElement &rDomElement, const QString element_name);

//! @todo We could go back to using only appendDomNode and then overload many different functions with same name but different input arguments)
QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name);
void appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text);
void appendDomBooleanNode(QDomElement &rDomElement, const QString element_name, const bool value);

void appendDomIntegerNode(QDomElement &rDomElement, const QString element_name, const int val);

void appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val);
void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b);
void appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c);
void appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<qreal> &rValues);

int parseDomIntegerNode(QDomElement, const int defaultValue);
bool parseDomBooleanNode(QDomElement domElement, const bool defaultValue);
double parseDomValueNode(QDomElement domElement, const double defaultValue);
void parseDomValueNode2(QDomElement domElement, double &rA, double &rB);
void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC);

void setQrealAttribute(QDomElement domElement, const QString attrName, const qreal attrValue, const int precision=6,  const char format='f');

bool parseAttributeBool(const QDomElement domElement, const QString attributeName, const bool defaultValue);
int parseAttributeInt(const QDomElement domElement, const QString attributeName, const int defaultValue);
qreal parseAttributeQreal(const QDomElement domElement, const QString attributeName, const qreal defaultValue);

//Color help functions
QString makeRgbString(QColor color);
void parseRgbString(QString rgb, double &red, double &green, double &blue);

#endif // XMLUTILITIES_H
