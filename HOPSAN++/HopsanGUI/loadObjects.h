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
//void addHMFHeader(QDomElement &rDomElement);

QDomElement appendHMFRootElement(QDomDocument &rDomDocument);



#endif // LOADOBJECTS_H
