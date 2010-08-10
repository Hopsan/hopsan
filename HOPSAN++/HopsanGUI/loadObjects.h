#ifndef LOADOBJECTS_H
#define LOADOBJECTS_H

#include <QTextStream>
#include "AppearanceData.h"

//Forward Declarations
class GraphicsView;
class LibraryWidget;
class GUIRootSystem;
class GUIObject;
class MessageWidget;

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


GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo=false);
GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo);
void loadConnector(QTextStream &rStream, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, bool noUnDo=false);
void loadParameterValues(QTextStream &rStream, GraphicsView* pGraphicsView, bool noUnDo=false);

GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo=false);
void loadConnector(const ConnectorLoadData &rData, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, bool noUnDo=false);
void loadParameterValues(const ParameterLoadData &rData, GraphicsView* pGraphicsView, bool noUnDo=false);

HeaderLoadData readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget);
void writeHeader(QTextStream &rStream);

#endif // LOADOBJECTS_H
