#ifndef LOADOBJECTS_H
#define LOADOBJECTS_H

#include <QTextStream>
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "Utilities/XMLUtilities.h"
#include <QFont>
#include <QColor>
#include <QPoint>


//Forward Declarations
class GraphicsView;
class LibraryWidget;
class CoreSystemAccess;
class GUIObject;
class GUIModelObject;
class MessageWidget;
class GUISystem;
class GUIContainerObject;

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
    bool isFlipped;
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
    QString externalfilepath;
    QString cqs_type;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);

};

//! @obsolete
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
    QStringList geometryList;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);
};

class ParameterLoadData
{
public:
    QString componentName, parameterName;
    qreal parameterValue;
    QString parameterGlobalKey;

    void read(QTextStream &rStream);
    void readDomElement(QDomElement &rDomElement);
};

class StartValueLoadData
{
public:
    QString portName, variable;
    qreal startValue;

    void readDomElement(QDomElement &rDomElement);
};

class TextWidgetLoadData
{
public:
    QString text;
    QFont font;
    QColor fontcolor;
    QPoint point;

    void readDomElement(QDomElement &rDomElement);
};

class BoxWidgetLoadData
{
public:
    QString linestyle;
    QColor linecolor;
    QPoint point;
    qreal width, height, linewidth;

    void readDomElement(QDomElement &rDomElement);
};

class GlobalParameterLoadData
{
public:
    QString name;
    double value;

    void readDomElement(QDomElement &rDomElement);
};

GUIModelObject* loadGUIModelObject(const ModelObjectLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
GUIModelObject* loadGUIModelObject(QTextStream &rStream, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
GUIModelObject* loadGUIModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);

GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings);
GUIObject* loadSubsystemGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings);
GUIObject* loadSubsystemGUIObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings);

void loadConnector(const ConnectorLoadData &rData, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
void loadConnector(QTextStream &rStream, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
void loadConnector(QDomElement &rDomElement, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);

void loadParameterValues(const ParameterLoadData &rData, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
void loadParameterValues(QTextStream &rStream, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
void loadParameterValue(const ParameterLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings=UNDO);
void loadParameterValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings=UNDO);

void loadStartValue(const StartValueLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings=UNDO);
void loadStartValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings=UNDO);

void loadGlobalParameter(const GlobalParameterLoadData &rData, GUIContainerObject* pSystem);
void loadGlobalParameter(QDomElement &rDomElement, GUIContainerObject* pSystem);

void loadTextWidget(QDomElement &rDomElement, GUIContainerObject *pSystem);
void loadBoxWidget(QDomElement &rDomElement, GUIContainerObject *pSystem);

HeaderLoadData readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget);
void writeHeader(QTextStream &rStream);
//void addHMFHeader(QDomElement &rDomElement);

QDomElement appendHMFRootElement(QDomDocument &rDomDocument);



#endif // LOADOBJECTS_H
