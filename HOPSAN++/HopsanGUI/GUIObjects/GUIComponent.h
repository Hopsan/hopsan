//!
//! @file   GUIComponent.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI class representing Components
//!
//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include "GUIModelObject.h"
#include "../common.h"
#include "../Utilities/XMLUtilities.h"
#include <assert.h>

//Forward declarations
class ProjectTabWidget;
class GUIConnector;
class GUIPort;
class GUIContainerObject;

class GUIComponent : public GUIModelObject
{
    Q_OBJECT

public:
    GUIComponent(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUIContainerObject *pParentContainer, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS);
    ~GUIComponent();

    QVector<QString> getParameterNames();
    QString getParameterUnit(QString name);
    QString getParameterDescription(QString name);
    double getParameterValue(QString name);
    QString getParameterValueTxt(QString name);
    bool setParameterValue(QString name, QString sysParName);
    QString getStartValueTxt(QString portName, QString variable);
    bool setStartValue(QString portName, QString variable, QString sysParName);

    //void setName(QString name, renameRestrictions renameSettings=UNRESTRICTED);
    QString getTypeName();
    QString getTypeCQS();

    enum { Type = GUICOMPONENT };
    int type() const;

protected:
    void saveCoreDataToDomElement(QDomElement &rDomElement);

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openPropertiesDialog();

    void createPorts();
};

#endif // GUICOMPONENT_H
