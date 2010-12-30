//!
//! @file   GUISystemPort.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the ContainerPort class
//!
//$Id$

#ifndef GUISYSTEMPORT_H
#define GUISYSTEMPORT_H

#include "GUIModelObject.h"

//! @todo Rename this and cpp file
class GUIContainerPort : public GUIModelObject
{
    Q_OBJECT
public:
    GUIContainerPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUIContainerObject *pParentContainer, selectionStatus startSelected = SELECTED, graphicsType gfxType = USERGRAPHICS);
    ~GUIContainerPort();
    QString getTypeName();
    void setName(QString newName, renameRestrictions renameSettings);


    enum { Type = GUICONTAINERPORT };
    int type() const;

protected:
    void createPorts();
    //void saveToDomElement(QDomElement &rDomElement);
    void saveCoreDataToDomElement(QDomElement &rDomElement);

private:
    bool mIsSystemPort;
    GUIPort *mpGuiPort;
};

#endif // GUISYSTEMPORT_H
