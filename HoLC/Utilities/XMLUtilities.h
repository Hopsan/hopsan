//!
//! @file   XMLUtilities.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-xx
//!
//! @brief Contains XML DOM help functions that are more or less Hopsan specific
//!
//$Id: XMLUtilities.h 6628 2014-02-24 13:47:38Z petno25 $

#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include <QtXml>

QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);

void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument);

QDomElement getOrAppendNewDomElement(QDomElement &rDomElement, const QString element_name);

//! @todo We could go back to using only appendDomNode and then overload manny different functions with same anme but different input arguments)
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
