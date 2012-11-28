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

QDomElement appendHMFRootElement(QDomDocument &rDomDocument, QString hmfVersion, QString hopsanGuiVersion, QString hopsanCoreVersion);
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

bool parseDomBooleanNode(QDomElement domElement);
qreal parseDomValueNode(QDomElement domElement);
void parseDomValueNode2(QDomElement domElement, double &rA, double &rB);
void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC);

int parseDomIntegerNode(QDomElement);

void setQrealAttribute(QDomElement domElement, const QString attrName, const qreal attrValue, const int precision=6,  const char format='f');

//Attribute help functions
void appendPoseTag(QDomElement &rDomElement, const qreal x, const qreal y, const qreal th, const bool flipped, const int precision=6);
void appendCoordinateTag(QDomElement &rDomElement, const qreal x, const qreal y, const int precision=20);
void appendViewPortTag(QDomElement &rDomElement, const qreal x, const qreal y, const qreal zoom);
void appendSimulationTimeTag(QDomElement &rDomElement, const qreal start, const qreal step, const qreal stop, const bool inheritTs);

void parsePoseTag(QDomElement domElement, qreal &rX, qreal &rY, qreal &rTheta, bool &rFlipped);
void parseCoordinateTag(QDomElement domElement, qreal &rX, qreal &rY);
void parseViewPortTag(QDomElement domElement, qreal &rX, qreal &rY, qreal &rZoom);
void parseSimulationTimeTag(QDomElement domElement, QString &rStart, QString &rStep, QString &rStop, bool &rInheritTs);

bool parseAttributeBool(const QDomElement domElement, const QString attributeName, const bool defaultValue);
qreal parseAttributeQreal(const QDomElement domElement, const QString attributeName, const qreal defaultValue);

//Color help functions
QString makeRgbString(QColor color);
void parseRgbString(QString rgb, double &red, double &green, double &blue);

void verifyHmfSubComponentCompatibility(QDomElement &element, double hmfVersion);
void verifyConfigurationCompatibility(QDomElement &rConfigElement);

//Save Load Definitions
//! @todo clean up this list and give some smarter names, remove TAG from end, also make sure we use thses defines where appropriate instead of hardcoded strings
#define HMF_ROOTTAG "hopsanmodelfile"
#define HMF_OBJECTS "objects"
#define HMF_OBJECTTAG "object"              //Non core Gui Object
#define HMF_MODELOBJECT "modelobject"
#define HMF_COMPONENTTAG "component"
#define HMF_SYSTEMTAG "system"
#define HMF_SYSTEMPORTTAG "systemport"
#define HMF_CONNECTIONS "connections"
#define HMF_CONNECTORTAG "connect"
#define HMF_PARAMETERTAG "parameter"
#define HMF_PARAMETERS "parameters"
#define HMF_FAVORITEVARIABLES "favoritevariables"
#define HMF_FAVORITEVARIABLETAG "favoritevariable"
#define HMF_ALIASES "aliases"
#define HMF_ALIAS "alias"
#define HMF_STARTVALUES "startvalues"
#define HMF_STARTVALUE "startvalue"
#define HMF_GROUPTAG "group"
#define HMF_TEXTBOXWIDGETTAG "textboxwidget"
#define HMF_TEXTWIDGETTAG "textwidget"
#define HMF_BOXWIDGETTAG "boxwidget"
#define HMF_PORTSTAG "ports"
#define HMF_NAMESTAG "names"
#define HMF_LOGSAMPLES "logsamples"

#define HMF_NAMETAG "name"
#define HMF_TYPENAME "typename"
#define HMF_SUBTYPENAME "subtypename"
#define HMF_CQSTYPETAG "cqs_type"
#define HMF_TYPE "type"
#define HMF_CPPCODETAG "cppcode"
#define HMF_CPPINPUTS "inputs"
#define HMF_CPPOUTPUTS "outputs"

#define HMF_HOPSANGUITAG "hopsangui"
#define HMF_COORDINATES "coordinates"
#define HMF_COORDINATETAG "coordinate"
#define HMF_GEOMETRIES "geometries"
#define HMF_GEOMETRYTAG "geometry"
#define HMF_STYLETAG "style"
#define HMF_COLORTAG "color"
#define HMF_XYTAG "xy"                      //Containes an xy coordinate pair
#define HMF_EXTERNALPATHTAG "external_path" //Contains the path to an external subsystem
#define HMF_VALUETAG "value"
#define HMF_SYSTEMPARAMETERTAG "globalkey"
#define HMF_TRUETAG "true"
#define HMF_FALSETAG "false"

#define HMF_POSETAG "pose"
#define HMF_VIEWPORTTAG "viewport"
#define HMF_NAMETEXTTAG "nametext"
#define HMF_NAMETEXTPOSTAG "nametextpos"
#define HMF_VISIBLETAG "nametextvisible"

#define HMF_CONNECTORSTARTCOMPONENTTAG "startcomponent"
#define HMF_CONNECTORSTARTPORTTAG "startport"
#define HMF_CONNECTORENDCOMPONENTTAG "endcomponent"
#define HMF_CONNECTORENDPORTTAG "endport"
#define HMF_CONNECTORDASHEDTAG "dashed"

#define HMF_SYSTEMAPPEARANCETAG "systemappearance"

#define HMF_VERSIONTAG "hmfversion"
#define HMF_HOPSANGUIVERSIONTAG "hopsanguiversion"
#define HMF_HOPSANCOREVERSIONTAG "hopsancoreversion"
#define HMF_SIMULATIONTIMETAG "simulationtime"
#define HMF_SCRIPTFILETAG "scriptfile"

#define HMF_UNDO "hopsanundo"

#endif // XMLUTILITIES_H
