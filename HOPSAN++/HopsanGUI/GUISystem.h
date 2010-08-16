#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#include <QGraphicsWidget>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QPoint>

#include "common.h"
#include "GUIObject.h"

//Forward Declaration
class AppearanceData;
//class GUIContainerObject;

class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem(AppearanceData appearanceData, QPoint position, qreal rotation, GraphicsScene *scene, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);

    void deleteInHopsanCore();

    QString getTypeName();
    void setName(QString newName, renameRestrictions renameSettings=UNRESTRICTED);
    void setTypeCQS(QString typestring);
    QString getTypeCQS();
    void loadFromFile(QString modelFileName=QString());

    void saveToTextStream(QTextStream &rStream, QString prepend);

    QVector<QString> getParameterNames();

    enum { Type = UserType + 4 };
    int type() const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openParameterDialog();
    void createPorts();

private:
    QString mModelFilePath;
    //QString mGraphicsFilePath;
    bool   mIsEmbedded;
    QString mLoadType;
};

#endif // GUISYSTEM_H
