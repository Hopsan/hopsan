#ifndef XMLUTILITIES_H
#define XMLUTILITIES_H

#include <QtXml>

void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument);

QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name);
void appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text);
void appendDomBooleanNode(QDomElement &rDomElement, const QString element_name, const bool value);

void appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val);
//! @todo maybe revert to using only appendDomTextNode names on all functions (then you need to think less)
void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b);
void appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c);
void appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<qreal> &rValues);

bool parseDomBooleanNode(QDomElement domElement);
qreal parseDomValueNode(QDomElement domElement);
void parseDomValueNode2(QDomElement domElement, double &rA, double &rB);
void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC);

//Save Load Definitions
#define HMF_ROOTTAG "hopsanmodelfile"
#define HMF_OBJECTTAG "object"              //Non core Gui Object
#define HMF_COMPONENTTAG "component"
#define HMF_SYSTEMTAG "system"
#define HMF_SYSTEMPORTTAG "systemport"
#define HMF_CONNECTORTAG "connect"
#define HMF_PARAMETERTAG "parameter"
#define HMF_GROUPTAG "group"
#define HMF_TEXTWIDGETTAG "textwidget"
#define HMF_BOXWIDGETTAG "boxwidget"

#define HMF_NAMETAG "name"
#define HMF_TYPETAG "typename"
#define HMF_CQSTYPETAG "cqs_type"
#define HMF_PORTTAG "port"

#define HMF_HOPSANGUITAG "hopsangui"
#define HMF_XYTAG "xy"                      //Containes an xy coordinate pair
#define HMF_EXTERNALPATHTAG "external_path" //Contains the path to an external subsystem
#define HMF_VALUETAG "value"
#define HMF_TURETAG "true"
#define HMF_FALSETAG "false"
#define HMF_POSETAG "pose"
#define HMF_NAMETEXTPOSTAG "nametextpos"
#define HMF_VISIBLETAG "nametextvisible"

#define HMF_CONNECTORSTARTCOMPONENTTAG "startcomponent"
#define HMF_CONNECTORSTARTPORTTAG "startport"
#define HMF_CONNECTORENDCOMPONENTTAG "endcomponent"
#define HMF_CONNECTORENDPORTTAG "endport"

#define HMF_SYSTEMAPPEARANCETAG "systemappearance"
#define CAF_ROOTTAG "componentappearancefile"
#define HMF_DISPLAYNAMETAG "displayname"
#define HMF_ISOICONTAG "isoicon"
#define HMF_USERICONTAG "usericon"
#define HMF_ICONROTATIONTAG "iconrotation"

#define HMF_VERSIONTAG "hmfversion"
#define HMF_HOPSANGUIVERSIONTAG "hopsanguiversion"
#define HMF_HOPSANCOREVERSIONTAG "hopsancoreversion"


#endif // XMLUTILITIES_H
