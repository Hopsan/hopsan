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

class GUIContainerObject : public GUIModelObject
{
    Q_OBJECT
public:
    enum CONTAINERSTATUS {CLOSED, OPEN, ROOT};
    GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, GUIContainerObject *system=0, QGraphicsItem *parent = 0);
    void makeRootSystem();

    //Scene and Core access
    GraphicsScene *getContainedScenePtr();
    virtual CoreSystemAccess *getCoreSystemAccessPtr();

    //Handle GuiModelObjects and GuiWidgets
    void addTextWidget(QPoint position);
    void addBoxWidget(QPoint position);
    GUIModelObject *addGUIModelObject(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, nameVisibility nameStatus = NAMEVISIBLE, undoStatus undoSettings = UNDO);
    GUIModelObject *getGUIModelObject(QString name);
    void deleteGUIModelObject(QString componentName, undoStatus undoSettings=UNDO);
    void renameGUIModelObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    bool hasGUIModelObject(QString name);

    //Handle connectors
    GUIConnector *findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    bool hasConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);
    void setIsCreatingConnector(bool isCreatingConnector);
    bool getIsCreatingConnector();

    //Handle container appearance
    QString getIsoIconPath();
    QString getUserIconPath();
    void setIsoIconPath(QString path);
    void setUserIconPath(QString path);
    void updateExternalPortPositions();
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y); //!< @todo maybe not public

    bool isObjectSelected();
    bool isConnectorSelected();

    //Enter and exit a container object
    //! @todo dont know if these names are good
    virtual void enterContainer();
    virtual void exitContainer();

    //This need to be here for now, and be public, used in containerPropertiesDialog by systems
    virtual void setTypeCQS(QString typestring){}
    virtual void setNumberOfLogSamples(size_t nSamples){}

    //Public member variable
    //!< @todo make this private later
    QFileInfo mModelFileInfo; //!< @todo should not be public
    UndoStack *mUndoStack;
    ProjectTab *mpParentProjectTab;
    //MainWindow *mpMainWindow;

    QList<GUIConnector *> mSelectedSubConnectorsList;
    QList<GUIConnector *> mSubConnectorList;

    //SHOULD BE PROTECTED
    typedef QHash<QString, GUIModelObject*> GUIModelObjectMapT;
    GUIModelObjectMapT mGUIModelObjectMap;
    QList<GUITextWidget *> mTextWidgetList;
    QList<GUIBoxWidget *> mBoxWidgetList;
    QList<GUIObject *> mSelectedGUIObjectsList;
    QList<GUIWidget *> mSelectedGUIWidgetsList;

    bool mPortsHidden;
    bool mUndoDisabled;
    bool mIsRenamingObject;
    bool mJustStoppedCreatingConnector;

    GUIModelObject *mpTempGUIModelObject;
    GUIConnector *mpTempConnector;
    graphicsType mGfxType;

public slots:
        //Selection
    void selectAll();
    void deselectAll();
    void deselectSelectedNameText();
        //show/hide
    void hideNames();
    void showNames();
    void hidePorts(bool doIt);
        //Create and remove
    void createConnector(GUIPort *pPort, undoStatus undoSettings=UNDO);
    void groupSelected(QPointF pt);
        //CopyPaste
    void cutSelected(CopyStack *xmlStack = 0);
    void copySelected(CopyStack *xmlStack = 0);
    void paste(CopyStack *xmlStack = 0);
        //UndoRedo
    void undo();
    void redo();
    void clearUndo();
    void disableUndo();
    void updateUndoStatus();
        //Appearance and settings
    void setGfxType(graphicsType gfxType);
    void openPropertiesDialogSlot();

signals:
        //Selection
    void deselectAllNameText();
    void deselectAllGUIObjects();
    void selectAllGUIObjects();
    void deselectAllGUIConnectors();
    void selectAllGUIConnectors();
        //HideShow
    void hideAllNameText();
    void showAllNameText();
        //Other
    void checkMessages();
    void deleteSelected();
    void setAllGfxType(graphicsType);
    void componentChanged();


protected:
    //virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    //virtual void saveCoreDataToDomElement(QDomElement &rDomElement);
    //Protected methods
    virtual void createPorts();
    virtual void openPropertiesDialog();
    //Protected overloaded Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

protected slots:


private:
    bool mIsCreatingConnector;

    CONTAINERSTATUS getContainerStatus();
    CONTAINERSTATUS mContainerStatus;

    GraphicsScene *mpScene;

    double mPasteOffset;

};

#endif // GUICONTAINEROBJECT_H
