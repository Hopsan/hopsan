//$Id$

#ifndef GUISYSTEMPORT_H
#define GUISYSTEMPORT_H

#include "GUIModelObject.h"

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
