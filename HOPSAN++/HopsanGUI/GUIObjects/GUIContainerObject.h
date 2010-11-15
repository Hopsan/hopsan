#ifndef GUICONTAINEROBJECT_H
#define GUICONTAINEROBJECT_H

#include "GUIModelObject.h"
#include "GraphicsScene.h"
#include "CoreSystemAccess.h"
#include "GUIWidgets.h"

//Forward Declarations
class ProjectTab;
class UndoStack;
class MainWindow;
class GraphicsScene;
class GUIContainerObject;

class GUIContainerObject : public GUIModelObject
{
    Q_OBJECT
public:
    enum CONTAINERSTATUS {CLOSED, OPEN, ROOT};
    GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUIContainerObject *system=0, QGraphicsItem *parent = 0);
    void makeRootSystem();
    virtual void updateExternalPortPositions();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y);

    virtual CoreSystemAccess* getCoreSystemAccessPtr();

    GraphicsScene* getContainedScenePtr();
    //void addConnector(GUIConnector *pConnector);

    //Add Gui Widgets
    virtual void addTextWidget(QPoint position);
    virtual void addBoxWidget(QPoint position);

    //Handle GuiModelObjects
    virtual GUIModelObject* addGUIModelObject(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, undoStatus undoSettings = UNDO);
    virtual GUIModelObject *getGUIModelObject(QString name);
    virtual void deleteGUIModelObject(QString componentName, undoStatus undoSettings=UNDO);
    virtual void renameGUIModelObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    virtual bool haveGUIModelObject(QString name);

    virtual GUIConnector* findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    virtual void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);

    virtual QString getIsoIconPath();
    virtual QString getUserIconPath();
    virtual void setIsoIconPath(QString path);
    virtual void setUserIconPath(QString path);

    virtual void setIsCreatingConnector(bool isCreatingConnector);
    virtual bool getIsCreatingConnector();

    bool isObjectSelected();
    bool isConnectorSelected();

    //Public member variable
    //!< @todo make this private later
    QFileInfo mModelFileInfo; //!< @todo should not be public
    UndoStack *mUndoStack;
    ProjectTab *mpParentProjectTab;
    MainWindow *mpMainWindow;

    QList<GUIConnector *> mSelectedSubConnectorsList;
    QList<GUIConnector *> mSubConnectorList;

    //SHOULD BE PROTECTED
    typedef QHash<QString, GUIModelObject*> GUIModelObjectMapT;
    GUIModelObjectMapT mGUIModelObjectMap;
    QList<GUITextWidget *> mTextWidgetList;
    QList<GUIBoxWidget *> mBoxWidgetList;
    QList<GUIObject *> mSelectedGUIObjectsList;

    bool mPortsHidden;
    bool mUndoDisabled;
    bool mIsRenamingObject;
    bool mJustStoppedCreatingConnector;

    GUIModelObject *mpTempGUIModelObject;
    GUIConnector *mpTempConnector;
    graphicsType mGfxType;

public slots:
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
//    void updateStartTime();
//    void updateTimeStep();
//    void updateStopTime();
    void disableUndo();
    void updateUndoStatus();
//    void updateSimulationParametersInToolBar();
    void setGfxType(graphicsType gfxType);
    void deselectSelectedNameText();

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
    void componentChanged();

protected:
    //virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    //virtual void saveCoreDataToDomElement(QDomElement &rDomElement);

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void openParameterDialog();
    virtual void createPorts();

    bool mIsCreatingConnector;

    CONTAINERSTATUS getContainerStatus();
    CONTAINERSTATUS mContainerStatus;

    //CoreSystemAccess *mpCoreSystemAccess;
    GraphicsScene *mpScene;

};

#endif // GUICONTAINEROBJECT_H
