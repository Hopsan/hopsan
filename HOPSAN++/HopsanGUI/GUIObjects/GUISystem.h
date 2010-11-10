#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#include <QGraphicsWidget>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QPoint>
#include <QFileInfo>

#include "GUIContainerObject.h"
#include "CoreSystemAccess.h"
#include "GUIWidgets.h"
#include "common.h"

//Forward Declaration
class ProjectTab;
class UndoStack;
class MainWindow;
class GraphicsScene;


class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem( QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUISystem *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *parent);
    ~GUISystem();

    void addTextWidget(QPoint position);
    void addBoxWidget(QPoint position);

    GUIModelObject* addGUIObject(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation=0, selectionStatus startSelected = DESELECTED, undoStatus undoSettings = UNDO);
    void deleteGUIModelObject(QString componentName, undoStatus undoSettings=UNDO);
    void renameGUIObject(QString oldName, QString newName, undoStatus undoSettings=UNDO);
    bool haveGUIObject(QString name);
    GUIModelObject *getGUIModelObject(QString name);

    GUIConnector* findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void removeConnector(GUIConnector* pConnector, undoStatus undoSettings=UNDO);

    bool isObjectSelected();
    bool isConnectorSelected();

    double getStartTime();
    double getTimeStep();
    double getStopTime();

    size_t getNumberOfLogSamples();
    void setNumberOfLogSamples(size_t nSamples);

    QString getIsoIconPath();
    QString getUserIconPath();
    void setIsoIconPath(QString path);
    void setUserIconPath(QString path);

    void updateExternalPortPositions();

    void setIsCreatingConnector(bool isCreatingConnector);
    bool getIsCreatingConnector();

    QString getTypeName();
    void setName(QString newName);
    void setTypeCQS(QString typestring);
    QString getTypeCQS();

    void saveToTextStream(QTextStream &rStream, QString prepend);
    void saveToDomElement(QDomElement &rDomElement);
    void loadFromHMF(QString modelFilePath=QString());
    void loadFromDomElement(QDomElement &rDomElement);

    QVector<QString> getParameterNames();

        //Public member variable
    CoreSystemAccess *mpCoreSystemAccess; //!< @todo make this private later
    QFileInfo mModelFileInfo; //!< @todo should not be public
    UndoStack *mUndoStack;
//    GraphicsScene *mpScene;
    ProjectTab *mpParentProjectTab;
    MainWindow *mpMainWindow;

    QList<GUIConnector *> mSelectedSubConnectorsList;
    QList<GUIConnector *> mSubConnectorList;

    typedef QHash<QString, GUIModelObject*> GUIModelObjectMapT;
    GUIModelObjectMapT mGUIModelObjectMap;
    QList<GUITextWidget *> mTextWidgetList;
    QList<GUIBoxWidget *> mBoxWidgetList;
    QList<GUIObject *> mSelectedGUIObjectsList;

    bool mPortsHidden;
    bool mUndoDisabled;
    bool mIsRenamingObject;
    bool mJustStoppedCreatingConnector;

    GUIModelObject *mpTempGUIObject;
    GUIConnector *mpTempConnector;
    graphicsType mGfxType;

    enum { Type = GUISYSTEM };
    int type() const;

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
    void updateStartTime();
    void updateTimeStep();
    void updateStopTime();
    void disableUndo();
    void updateUndoStatus();
    void updateSimulationParametersInToolBar();
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
    void saveCoreDataToDomElement(QDomElement &rDomElement);
    void saveSystemAppearanceToDomElement(QDomElement &rDomElement);

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void openParameterDialog();
    void createPorts();

    bool mIsCreatingConnector;

private:
    void commonConstructorCode();

    double mStartTime;
    double mStopTime;
    double mTimeStep;
    size_t mNumberOfLogSamples;

    bool   mIsEmbedded;
    QString mLoadType;
};

#endif // GUISYSTEM_H
