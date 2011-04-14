//!
//! @file   loadObjects.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains classes and functions used to recreate models from load data
//!
//$Id$

#ifndef LOADOBJECTS_H
#define LOADOBJECTS_H

#include "GUIObjects/GUIModelObjectAppearance.h"
#include "Utilities/XMLUtilities.h"
#include <QFont>
#include <QColor>
#include <QPoint>


//Forward Declarations
class GraphicsView;
class LibraryWidget;
class MessageWidget;
class CoreSystemAccess;
class GUIObject;
class GUIModelObject;
class GUIContainerObject;
class GUISystem;


class ModelObjectLoadData
{
public:
    QString type;
    QString name;
    qreal posX, posY, rotation;
    bool isFlipped;
    int nameTextPos;
    int textVisible;
    bool portsHidden;
    bool namesHidden;
    QMap<QString, double> defaultParameterMap;

    void readDomElement(QDomElement &rDomElement);

protected:
    void readGuiDataFromDomElement(QDomElement &rDomElement);

};

class SystemLoadData :public ModelObjectLoadData
{
public:
    QString externalfilepath;
    QDomElement embededSystemDomElement;

    void readDomElement(QDomElement &rDomElement);
};


class ConnectorLoadData
{
public:
    QString startComponentName, endComponentName, startPortName, endPortName;
    QVector<QPointF> pointVector;
    QStringList geometryList;

    void readDomElement(QDomElement &rDomElement);
};

class ParameterLoadData
{
public:
    QString componentName, parameterName;
    QString parameterValue;
    //QString parameterGlobalKey;

    void readDomElement(QDomElement &rDomElement);
};

class StartValueLoadData
{
public:
    QString portName, variable;
    QString startValue;

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

class SystemParameterLoadData
{
public:
    QString name;
    double value;

    void readDomElement(QDomElement &rDomElement);
};

class FavoriteParameterLoadData
{
public:
    QString componentName;
    QString portName;
    QString dataName;
    QString dataUnit;

    void readDomElement(QDomElement &rDomElement);
};

class PlotAliasLoadData
{
public:
    QString alias;
    QString componentName;
    QString portName;
    QString dataName;

    void readDomElement(QDomElement &rDomElement);
};

GUIModelObject* loadGUIModelObject(const ModelObjectLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
GUIModelObject* loadGUIModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);

GUIModelObject* loadGUISystemObject(SystemLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings);
GUIModelObject* loadGUISystemObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings);

GUIModelObject* loadContainerPortObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);

void loadConnector(const ConnectorLoadData &rData, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);
void loadConnector(QDomElement &rDomElement, GUIContainerObject* pSystem, undoStatus undoSettings=UNDO);

void loadParameterValue(const ParameterLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings=UNDO);
void loadParameterValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings=UNDO);

void loadStartValue(const StartValueLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings=UNDO);
void loadStartValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings=UNDO);
void loadSystemParameter(const SystemParameterLoadData &rData, GUIContainerObject* pSystem);
void loadSystemParameter(QDomElement &rDomElement, GUIContainerObject* pSystem);
void loadFavoriteParameter(const FavoriteParameterLoadData &rData, GUIContainerObject* pSystem);
void loadFavoriteParameter(QDomElement &rDomElement, GUIContainerObject* pSystem);
void loadPlotAlias(const PlotAliasLoadData &rData, GUIContainerObject* pSystem);
void loadPlotAlias(QDomElement &rDomElement, GUIContainerObject* pSystem);

void loadTextWidget(QDomElement &rDomElement, GUIContainerObject *pSystem, undoStatus undoSettings=UNDO);
void loadBoxWidget(QDomElement &rDomElement, GUIContainerObject *pSystem, undoStatus undoSettings=UNDO);

#endif // LOADOBJECTS_H
