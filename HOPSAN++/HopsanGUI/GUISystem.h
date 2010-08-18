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
class ProjectTab;
//class GUIContainerObject;
class UndoStack;
#include "CoreSystemAccess.h"

class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem(AppearanceData appearanceData, QPoint position, qreal rotation, GUISystem *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, ProjectTab *parentProjectTab = 0, QGraphicsItem *parent = 0);
    GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *parent);
    ~GUISystem();
    void constructorStuff(ProjectTab *parentProjectTab);

    GraphicsScene *mpScene;
    ProjectTab *mpParentProjectTab;

    QString mModelFileName;

    typedef QMap<QString, GUIObject*> GUIObjectMapT;
    GUIObjectMapT mGUIObjectMap;
    GUIObject* addGUIObject(AppearanceData appearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, undoStatus undoSettings = UNDO);
    void deleteGUIObject(QString componentName, undoStatus undoSettings=UNDO);
    void renameGUIObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    bool haveGUIObject(QString name);
    GUIObject *getGUIObject(QString name);

    QVector<GUIConnector *> mConnectorVector;
    GUIConnector* findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);

    UndoStack *mUndoStack;

    bool mPortsHidden;
    bool mUndoDisabled;
    bool mIsCreatingConnector;
    bool mIsRenamingObject;
    bool mJustStoppedCreatingConnector;
    bool isObjectSelected();
    bool isConnectorSelected();
    GUIObject *mpTempGUIObject;
    GUIConnector *mpTempConnector;
    graphicsType mGfxType;

    void updateStartTime();
    void updateTimeStep();
    void updateStopTime();
    double getStartTime();
    double getTimeStep();
    double getStopTime();

    QString getIsoIconPath();
    QString getUserIconPath();
    void setIsoIconPath(QString path);
    void setUserIconPath(QString path);

public slots:
    //void addSystemPort();
    void createConnector(GUIPort *pPort, undoStatus undoSettings=UNDO);
    void cutSelected();
    void copySelected();
    void paste();
    void selectAll();
    void deselectAll();
    void hideNames();
    void showNames();
    void hidePorts(bool doIt);
    void undo();
    void redo();
    void clearUndo();

signals:
    void deselectAllNameText();
    void deselectAllGUIObjects();
    void deselectAllGUIConnectors();
    void checkMessages();
    void deleteSelected();

private:
    QString *mpCopyData;
    double mStartTime;
    double mStopTime;
    double mTimeStep;
    QString mUserIconPath;
    QString mIsoIconPath;

public:
    CoreSystemAccess mCoreSystemAccess; //!< @todo make this private later


    //! Old subsystem stuff
public:
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
    bool   mIsEmbedded;
    QString mLoadType;
};

#endif // GUISYSTEM_H
