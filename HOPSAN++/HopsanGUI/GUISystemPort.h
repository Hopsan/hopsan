#ifndef GUISYSTEMPORT_H
#define GUISYSTEMPORT_H

#include "GUIObject.h"

class GUISystemPort : public GUIModelObject
{
    Q_OBJECT
public:
    GUISystemPort(AppearanceData* pAppearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected = SELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    ~GUISystemPort();
    QString getTypeName();
    void setName(QString newName, renameRestrictions renameSettings);


    enum { Type = GUISYSTEMPORT };
    int type() const;

protected:
    void createPorts();
    //void saveToDomElement(QDomElement &rDomElement);
    //void saveCoreDataToDomElement(QDomElement &rDomElement);

private:
    GUIPort *mpGuiPort;
};

#endif // GUISYSTEMPORT_H
