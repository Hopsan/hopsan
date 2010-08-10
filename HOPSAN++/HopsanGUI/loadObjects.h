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
void loadConnector(QTextStream &rStream, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, bool noUnDo=false);
void loadParameterValues(QTextStream &rStream, GraphicsView* pGraphicsView, bool noUnDo=false);

GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo=false);
void loadConnector(const ConnectorLoadData &rData, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, bool noUnDo=false);
void loadParameterValues(const ParameterLoadData &rData, GraphicsView* pGraphicsView, bool noUnDo=false);

void readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget);
void writeHeader(QTextStream &rStream);

#endif // LOADOBJECTS_H
