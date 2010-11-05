#ifndef LOADOBJECTS_H
#define LOADOBJECTS_H

#include <QTextStream>
#include <QtXml>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include "AppearanceData.h"

//Forward Declarations
class GraphicsView;
class LibraryWidget;
class CoreSystemAccess;
class GUIObject;
class GUIModelObject;
class MessageWidget;
class GUISystem;

class HeaderLoadData
{
public:
    double startTime;
    double timeStep;
    double stopTime;

    QString hopsangui_version;
    QString hopsancore_version;
    QString hmf_version;
    QString caf_version;

    double viewport_x;
    double viewport_y;
    double viewport_zoomfactor;

    void read(QTextStream &rStream);
};

class ModelObjectLoadData
{
public:
    QString type;
    QString name;
    qreal posX, posY, rotation;
    int nameTextPos;
    int textVisible;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);

protected:
    void readGuiDataFromDomElement(QDomElement &rDomElement);

};

class SubsystemLoadData :public ModelObjectLoadData
{
public:
    QString loadtype;
    QString filepath;
    QString cqs_type;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);

};

//! @todo the class should be shared somehow with the other class that reads appearance data from files for components
class SystemAppearanceLoadData
{
public:
    QString usericon_path;
    QString isoicon_path;
    //! @todo code these four vectors in some other smarter way, compare with port apperance
    QVector<QString> portnames;
    QVector<qreal> port_xpos;
    QVector<qreal> port_ypos;
    QVector<qreal> port_angle;

    void read(QTextStream &rStream);
    //void readDomElement(QDomElement &rDomElement);

};

class ConnectorLoadData
{
public:
    QString startComponentName, endComponentName, startPortName, endPortName;
    QVector<QPointF> pointVector;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);
};

class ParameterLoadData
{
public:
    QString componentName, parameterName;
    qreal parameterValue;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);
};


GUIModelObject* loadGUIModelObject(const ModelObjectLoadData &rData, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings=UNDO);
GUIModelObject* loadGUIModelObject(QTextStream &rStream, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings=UNDO);
GUIModelObject* loadGUIModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings);

GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings);
GUIObject* loadSubsystemGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings);
GUIObject* loadSubsystemGUIObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings);

void loadConnector(const ConnectorLoadData &rData, GUISystem* pSystem, undoStatus undoSettings=UNDO);
void loadConnector(QTextStream &rStream, GUISystem* pSystem, undoStatus undoSettings=UNDO);
void loadConnector(QDomElement &rDomElement, GUISystem* pSystem, undoStatus undoSettings);

void loadParameterValues(const ParameterLoadData &rData, GUISystem* pSystem, undoStatus undoSettings=UNDO);
void loadParameterValues(QTextStream &rStream, GUISystem* pSystem, undoStatus undoSettings=UNDO);
void loadParameterValue(const ParameterLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings=UNDO);
void loadParameterValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings=UNDO);

void loadTextWidget(QDomElement &rDomElement, GUISystem *pSystem);
void loadBoxWidget(QDomElement &rDomElement, GUISystem *pSystem);

HeaderLoadData readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget);
void writeHeader(QTextStream &rStream);
void addHMFHeader(QDomElement &rDomElement);

void appendRootXMLProcessingInstruction(QDomDocument &rDomDocument);

QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name);
void appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text);
void appendDomBooleanNode(QDomElement &rDomElement, const QString element_name, const bool value);

void appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val);
//! @todo maybe revert to using only appendDomTextNode names on all functions (then you need to think less)
void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b);
void appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c);
void appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<qreal> &rValues);
//! @todo write one that takes a vector with data

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
#define HMF_POSETAG "pose"
#define HMF_NAMETEXTPOSTAG "nametextpos"
#define HMF_VISIBLETAG "nametextvisible"

#define HMF_CONNECTORSTARTCOMPONENTTAG "startcomponent"
#define HMF_CONNECTORSTARTPORTTAG "startport"
#define HMF_CONNECTORENDCOMPONENTTAG "endcomponent"
#define HMF_CONNECTORENDPORTTAG "endport"

#define CAF_ROOTTAG "componentappearancefile"
#define HMF_DISPLAYNAMETAG "displayname"
#define HMF_ISOICONTAG "isoicon"
#define HMF_USERICONTAG "usericon"
#define HMF_ICONROTATIONTAG "iconrotation"



#endif // LOADOBJECTS_H
