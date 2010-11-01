#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <QGraphicsWidget>
#include <QObject>
#include <QGraphicsSvgItem>
#include <QPen>
#include <QtXml>
//! @todo clean up these includes and forward declarations
#include "common.h"

#include "AppearanceData.h"
#include <assert.h>

class ProjectTabWidget;
class GraphicsScene;
class GraphicsView;
class GUIConnector;
class GUIObjectDisplayName;
class Component;
class GUIObjectSelectionBox;
class GUIPort;
class GUISystem;
//class GUIObject;

#include "GUIObject.h"

class GUIComponent : public GUIObject
{
    Q_OBJECT
public:
    GUIComponent(AppearanceData* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    ~GUIComponent();

    QVector<QString> getParameterNames();
    QString getParameterUnit(QString name);
    QString getParameterDescription(QString name);
    double getParameterValue(QString name);
    void setParameterValue(QString name, double value);

    void saveToTextStream(QTextStream &rStream, QString prepend=QString());
    void saveToDomElement(QDomElement &rDomElement);

    void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    QString getTypeName();
    QString getTypeCQS();

    enum { Type = GUICOMPONENT };
    int type() const;

protected:
    void saveCoreDataToDomElement(QDomElement &rDomElement);

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

    void createPorts();

    //QString mComponentTypeName;

public slots:

private:

};

#endif // GUICOMPONENT_H
