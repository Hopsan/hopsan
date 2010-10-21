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

class ObjectLoadData
{
public:
    QString type;
    QString name;
    qreal posX, posY, rotation;
    int nameTextPos;
    int textVisible;

    void read(QTextStream &rStream);
};

class SubsystemLoadData :public ObjectLoadData
{
public:
    QString loadtype;
    QString filepath;
    QString cqs_type;

    void read(QTextStream &rStream);
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

};

class ConnectorLoadData
{
public:
    QString startComponentName, endComponentName, startPortName, endPortName;
    QVector<QPointF> pointVector;

    void read(QTextStream &rStream);
};

class ParameterLoadData
{
public:
    QString componentName, parameterName;
    qreal parameterValue;

    void read(QTextStream &rStream);
};


GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings=UNDO);
GUIObject* loadSubsystemGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings);
void loadConnector(QTextStream &rStream, GUISystem* pSystem, undoStatus undoSettings=UNDO);
void loadParameterValues(QTextStream &rStream, GUISystem* pSystem, undoStatus undoSettings=UNDO);

GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings=UNDO);
GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings);
void loadConnector(const ConnectorLoadData &rData, GUISystem* pSystem, undoStatus undoSettings=UNDO);
void loadParameterValues(const ParameterLoadData &rData, GUISystem* pSystem, undoStatus undoSettings=UNDO);

HeaderLoadData readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget);
void writeHeader(QTextStream &rStream);
void addHMFHeader(QDomElement &rDomElement);

QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name);
void appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text);
void appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val);
//! @todo maybe revert to using only appendDomTextNode names on all functions (then you need to think less)
void appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c);
void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b);
//! @todo write one that takes a vector with data

#endif // LOADOBJECTS_H
