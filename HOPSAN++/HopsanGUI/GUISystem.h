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
class MainWindow;

#include "CoreSystemAccess.h"

class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem( QPoint position, qreal rotation, AppearanceData appearanceData, GUISystem *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *parent);
    ~GUISystem();


    void loadFromHMF(QString modelFileName=QString());

    GraphicsScene *mpScene;
    ProjectTab *mpParentProjectTab;
    MainWindow *mpMainWindow;

    //QString mModelFileName;

    typedef QMap<QString, GUIObject*> GUIObjectMapT;
    GUIObjectMapT mGUIObjectMap;
    QList<GUIObject *> mSelectedGUIObjectsList;
    GUIObject* addGUIObject(AppearanceData appearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, undoStatus undoSettings = UNDO);
    void deleteGUIObject(QString componentName, undoStatus undoSettings=UNDO);
    void renameGUIObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    bool haveGUIObject(QString name);
    GUIObject *getGUIObject(QString name);

    QList<GUIConnector *> mSelectedSubConnectorsList;
    QList<GUIConnector *> mSubConnectorList;
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
    void updateStartTime();
    void updateTimeStep();
    void updateStopTime();
    void disableUndo();
    void updateUndoStatus();
    void updateSimulationSetupWidget();
    void setGfxType(graphicsType gfxType);

signals:
    void deselectAllNameText();
    void hideAllNameText();
    void showAllNameText();
    void deselectAllGUIObjects();
    void selectAllGUIObjects();
    void deselectAllGUIConnectors();
    void selectAllGUIConnectors();
    void setAllGfxType(graphicsType);
    void checkMessages();
    void deleteSelected();

private:
    void commonConstructorCode();

    QString *mpCopyData;
    double mStartTime;
    double mStopTime;
    double mTimeStep;
    QString mUserIconPath;
    QString mIsoIconPath;

public:
    CoreSystemAccess *mpCoreSystemAccess; //!< @todo make this private later


    //! Old subsystem stuff
public:
    QString getTypeName();
    void setName(QString newName);
    void setTypeCQS(QString typestring);
    QString getTypeCQS();

    void loadFromFileNOGUI(QString modelFileName=QString());

    void saveToTextStream(QTextStream &rStream, QString prepend);

    QVector<QString> getParameterNames();

    enum { TYPE = GUISYSTEM };
    int type() const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openParameterDialog();
    void createPorts();

public:
        QString mModelFilePath; //!< @todo should be public

private:

    bool   mIsEmbedded;
    QString mLoadType;
};

#endif // GUISYSTEM_H
