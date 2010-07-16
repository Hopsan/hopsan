#ifndef LOADOBJECTS_H
#define LOADOBJECTS_H

#include <QTextStream>
#include "AppearanceData.h"

//Forward Declarations
class GraphicsView;
class LibraryWidget;
class GUIRootSystem;
class GUIObject;

class ObjectLoadData
{
public:
    QString type;
    QString name;
    qreal posX, posY, rotation;
    int nameTextPos;

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


GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GraphicsView* pGraphicsView);
void loadConnector(QTextStream &rStream, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem);
void loadParameterValues(QTextStream &rStream, GraphicsView* pGraphicsView);

GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView);
void loadConnector(const ConnectorLoadData &rData, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem);
void loadParameterValues(const ParameterLoadData &rData, GraphicsView* pGraphicsView);

#endif // LOADOBJECTS_H
