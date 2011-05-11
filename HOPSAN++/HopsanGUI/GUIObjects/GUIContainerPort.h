//!
//! @file   GUIContainerPort.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the ContainerPort class
//!
//$Id$

#ifndef GUICONTAINERPORT_H
#define GUICONTAINERPORT_H

#include "GUIModelObject.h"

//! @todo Rename this and cpp file
class GUIContainerPort : public GUIModelObject
{
    Q_OBJECT
public:
    GUIContainerPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUIContainerObject *pParentContainer, selectionStatus startSelected = SELECTED, graphicsType gfxType = USERGRAPHICS);
    ~GUIContainerPort();
    QString getTypeName();
    void setDisplayName(QString name);

    enum { Type = GUICONTAINERPORT };
    int type() const;

protected:
    void createPorts();
    void saveCoreDataToDomElement(QDomElement &rDomElement);
    void openPropertiesDialog();
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    bool mIsSystemPort;
    GUIPort *mpGuiPort;
};

#endif
