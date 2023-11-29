/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   GUIContainerObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI Container class (base class for Systems and Groups)
//!
//$Id$

#ifndef GUICONTAINEROBJECT_H
#define GUICONTAINEROBJECT_H

#include <QSlider>

#include "GUIModelObject.h"
#include "CopyStack.h"
#include "LogDataHandler2.h"
#include "GraphicsViewPort.h"

//Forward Declarations
class PlotWindow;
class ModelWidget;
class UndoStack;
class MainWindow;
class QGraphicsScene;
class Port;
class Widget;
class TextBoxWidget;
class ImageWidget;
class QTableView;
class QRadioButton;

//Forward Declaration
class ModelWidget;


class OptParameter
{
public:
    QString mComponentName, mParameterName;
    double mMin, mMax;
};


class Objectives
{
public:
    QString mFunctionName;
    QStringList mData;
    QList<QStringList> mVariableInfo;
    double mWeight, mNorm, mExp;
};


class OptimizationSettings
{
public:
    OptimizationSettings();

    QString mScriptFile;
    int mNiter;
    int mNsearchp;
    double mRefcoeff;
    double mRandfac;
    double mForgfac;
    double mPartol;
    bool mPlot;
    bool mSavecsv;
    bool mFinalEval;
    bool mlogPar;

    //Parameters
    QVector<OptParameter> mParamters;
    QVector<Objectives> mObjectives;
};


class SensitivityAnalysisParameter
{
public:
    QString compName, parName;
    double min, max;      //Used for square distribution
    double aver, sigma;   //Used for normal distribution
};


class SensitivityAnalysisVariable
{
public:
    QString compName, portName, varName;
};


class SensitivityAnalysisSettings
{
public:
    SensitivityAnalysisSettings();

    enum DistributionEnumT { UniformDistribution, NormalDistribution };

    int nIter;     //Number of iterations
    QVector<SensitivityAnalysisParameter> parameters;
    QVector<SensitivityAnalysisVariable> variables;
    DistributionEnumT distribution;
};

class SystemObject : public ModelObject
{
    friend class UndoStack;     //! @todo Not sure about this, but the alternative would be to have lots and lots of access functions only used by undo stack...
    Q_OBJECT
public:
    enum ContainerEdgeEnumT {RightEdge, BottomEdge, LeftEdge, TopEdge};
    SystemObject(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected = Deselected, GraphicsTypeEnumT gfxType = UserGraphics, SystemObject *pParentSystem=0, QGraphicsItem *pParent=0);
    SystemObject(ModelWidget *parentModelWidget, QGraphicsItem *pParentGraphicsItem);
    virtual ~SystemObject();

    void deleteInHopsanCore() override;

    QString getTypeName() const override;
    void setName(QString newName) override;
    QString getTypeCQS() const override;

    double getTimeStep();
    bool doesInheritTimeStep();

    bool isTopLevelContainer() const;
    QStringList getSystemNameHieararchy() const override;

    void hasChanged();
    ModelWidget *mpModelWidget = nullptr;  //!< @todo not public

    //Signal/slot connection methods
    virtual void makeMainWindowConnectionsAndRefresh();
    virtual void unmakeMainWindowConnectionsAndRefresh();

    // Scene and view related methods
    QGraphicsScene *getContainedScenePtr();
    void setGraphicsViewport(GraphicsViewPort vp);
    GraphicsViewPort getGraphicsViewport() const;

    // Core access
    virtual CoreSystemAccess *getCoreSystemAccessPtr();
    SystemObject *getParentSystemObject() override;

    //GUIModelObjects and GUIWidgets methods
    void takeOwnershipOf(QList<ModelObject*> &rModeObjectlist, QList<Widget*> &rWidgetList);

    //GUIModelObject methods
    ModelObject *addModelObject(QString fullTypeName, QPointF position, double rotation=0, SelectionStatusEnumT startSelected = Deselected, NameVisibilityEnumT nameStatus = UseDefault, UndoStatusEnumT undoSettings = Undo);
    ModelObject *addModelObject(ModelObjectAppearance* pAppearanceData, QPointF position, double rotation=0, SelectionStatusEnumT startSelected = Deselected, NameVisibilityEnumT nameStatus = UseDefault, UndoStatusEnumT undoSettings = Undo);
    void deleteModelObject(const QString &rObjectName, UndoStatusEnumT undoSettings=Undo);
    void renameModelObject(QString oldName, QString newName, UndoStatusEnumT undoSettings=Undo);

    void replaceComponent(QString name, QString newType);

    ModelObject *getModelObject(const QString &rModelObjectName);
    QList<ModelObject* > getModelObjects() const;
    QStringList getModelObjectNames() const;
    Port *getModelObjectPort(const QString modelObjectName, const QString portName);
    bool hasModelObject(const QString &rName) const;

    void rememberSelectedModelObject(ModelObject *object);
    void forgetSelectedModelObject(ModelObject *object);
    QList<ModelObject *> getSelectedModelObjectPtrs();
    bool isSubObjectSelected();

    // Alias methods
    bool setVariableAlias(const QString &rFullName, const QString &rAlias);
    QString getVariableAlias(const QString &rFullName);
    QString getFullNameFromAlias(const QString alias);
    QStringList getAliasNames();

    //GUIWidgets methods
    TextBoxWidget *addTextBoxWidget(QPointF position, UndoStatusEnumT undoSettings=Undo);
    TextBoxWidget *addTextBoxWidget(QPointF position, int desiredWidgetId, UndoStatusEnumT undoSettings=Undo);
    ImageWidget *addImageWidget(QPointF position, UndoStatusEnumT undoSettings=Undo);
    ImageWidget *addImageWidget(QPointF position, int desiredWidgetId, UndoStatusEnumT undoSettings=Undo);
    void deleteWidget(Widget *pWidget, UndoStatusEnumT undoSettings=Undo);
    void deleteWidget(const int id, UndoStatusEnumT undoSettings=Undo);

    QList<Widget*> getWidgets() const;
    Widget *getWidget(const int id) const;

    void rememberSelectedWidget(Widget *widget);
    void forgetSelectedWidget(Widget *widget);
    QList<Widget *> getSelectedGUIWidgetPtrs();

    // Parameter Methods
    QStringList getParameterNames() override;
    void getParameters(QVector<CoreParameterData> &rParameterDataVec) override;
    void getParameter(const QString paramName, CoreParameterData &rData) override;
    QString getParameterValue(const QString paramName) override;
    bool hasParameter(const QString &rParamName) override;
    virtual bool setParameterValue(QString name, QString value, bool force=false) override;
    virtual bool setParameter(const CoreParameterData &rParameter, bool force=false);
    virtual bool setOrAddParameter(const CoreParameterData &rParameter, bool force=false);
    virtual bool renameParameter(const QString oldName, const QString newName);
    void loadParameterValuesFromFile(QString parameterFile = {}) override;
    void getAllParametersAndValuesRecursively(QString prefix, QStringList &names, QStringList &values, QStringList &dataTypes, QStringList &quantities, QStringList &units);

    // NumHop Methods
    void setNumHopScript(const QString &rScript);
    QString getNumHopScript() const;
    void runNumHopScript(const QString &rScript, bool printOutput, QString &rOutput);

    //Handle connector methods
    bool hasConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    Connector *findConnector(QString startComp, QString startPort, QString endComp, QString endPort);
    void rememberSubConnector(Connector *pConnector);
    void removeSubConnector(Connector* pConnector, UndoStatusEnumT undoSettings=Undo);
    bool isConnectorSelected();
    void rememberSelectedSubConnector(Connector *pConnector);
    void forgetSelectedSubConnector(Connector *pConnector);
    bool isCreatingConnector();
    void cancelCreatingConnector();
    void addOneConnectorLine(QPointF pos);
    void removeOneConnectorLine(QPointF pos);
    void makeConnectorDiagonal(bool diagonal);
    void updateTempConnector(QPointF pos);

    //Handle container appearance
    QString getIconPath(const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType);
    void setIconPath(const QString path, const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType);
    ContainerEdgeEnumT findPortEdge(QPointF center, QPointF pt); //!< @todo maybe not public
    virtual void refreshAppearance() override;
    void refreshExternalPortsAppearanceAndPosition() override;
    void calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y); //!< @todo maybe not public

    //Plot and simulation results methods
    QSharedPointer<LogDataHandler2> getLogDataHandler();

    // Get the IDs of the component libraries that the components in this model come from
    QStringList getRequiredComponentLibraries() const;

    //Undo/redo methods
    UndoStack *getUndoStackPtr();

    //Copy/paste methods
    CopyStack *getDragCopyStackPtr();

    //These (overloaded versions) are used in containerPropertiesDialog by systems
    virtual size_t getNumberOfLogSamples();
    virtual void setNumberOfLogSamples(size_t nSamples);
    virtual double getLogStartTime() const;
    virtual void setLogStartTime(const double logStartT);

    //Model info
    void setModelInfo(const QString &author, const QString &email, const QString &affiliation, const QString &description);
    void getModelInfo(QString &author, QString &email, QString &affiliation, QString &description) const;

    //Model and script file methods
    void setModelFile(QString path);
    const QFileInfo &getModelFileInfo() const;
    void setModelFileInfo(QFile &rFile, const QString relModelPath="") override;
    QString getModelFilePath() const;
    QString getModelPath() const;

    //Numbered sections methods
    void selectSection(int no, bool append=false);
    void assignSection(int no);

    //Losses methods
    bool areLossesVisible();
    QRadioButton *mpAvgPwrRadioButton;

    //Animation methods
    bool isAnimationDisabled();
    void setAnimationDisabled(bool disabled);

    // Export methods
    void exportToLabView();
    void exportToFMU1_32();
    void exportToFMU1_64();
    void exportToFMU2_32();
    void exportToFMU2_64();
    void exportToFMU3_32();
    void exportToFMU3_64();
    void exportToFMU(QString savePath, int version, ArchitectureEnumT arch);
    void exportToSimulink();
    void exportToExecutableModel(QString savePath, ArchitectureEnumT arch);

    // Type info
    virtual int type() const override;
    virtual QString getHmfTagName() const override;

    void getSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings);
    void setSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings);
    void getOptimizationSettings(OptimizationSettings &optSettings);
    void setOptimizationSettings(OptimizationSettings &optSettings);

    void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents = FullModel) override;
    void loadFromDomElement(QDomElement domElement) override;


public slots:

    void setTimeStep(const double timeStep);
    void setVisibleIfSignal(bool visible);

    //Selection slots
    void selectAll();
    void deselectAll();
    void deselectSelectedNameText();
    QPointF getCenterPointFromSelection();

    //Show/hide slots
    void hideNames();
    void showNames();
    void toggleNames(bool value);
    void toggleSignals(bool value);
    void showSubcomponentPorts(bool doShowThem);

    //Connector slots
    Connector* createConnector(Port *pPort, UndoStatusEnumT undoSettings=Undo);
    Connector* createConnector(Port *pPort1, Port *pPort2, UndoStatusEnumT undoSettings=Undo);

    //Copy/paste slots
    void cutSelected(CopyStack *xmlStack = 0);
    void copySelected(CopyStack *xmlStack = 0);
    void paste(CopyStack *xmlStack = 0);

    //Alignment slots
    void alignX();
    void alignY();
    void distributeX();
    void distributeY();

    //Undo/redo slots
    void undo();
    void redo();
    void clearUndo();
    void setUndoEnabled(bool enabled, bool dontAskJustDoIt=false);
    void setSaveUndo(bool save);
    bool isUndoEnabled();
    bool getSaveUndo();
    void updateMainWindowButtons();

    //Appearance slots
    void setGfxType(GraphicsTypeEnumT gfxType);
    GraphicsTypeEnumT getGfxType();
    bool areSubComponentPortsShown();
    bool areSubComponentNamesShown();
    bool areSignalsVisible();

    //Properties slots
    void openPropertiesDialogSlot();

    //Enter and exit a container object
    void enterContainer();
    void exitContainer();

    //Rotate and flip slots
    void rotateSubObjects90cw();
    void rotateSubObjects90ccw();
    void flipSubObjectsHorizontal();
    void flipSubObjectsVertical();

    //Losses
    void showLosses(bool show);
    void showLossesFromDialog();
    void hideLosses();

    //Simulation time measurements
    void measureSimulationTime();
    void plotMeasuredSimulationTime();
    void exportMesasuredSimulationTime();

    //External/internal subsystems
    bool isAncestorOfExternalSubsystem();
    bool isExternal();

    QList<Connector*> getSubConnectorPtrs();

signals:

    //Selection signals
    void deselectAllNameText();
    void deselectAllGUIObjects();
    void selectAllGUIObjects();
    void deselectAllConnectors();
    void selectAllConnectors();

    //Hide/show name text
    void hideAllNameText();
    void showAllNameText();
    void showOrHideAllNameText(bool doShow); //!< @todo use this instead of two separate show hide

    // Hide/Show subcomponent ports
    void showOrHideAllSubComponentPorts(bool doShow);

    //Hide/show signals components
    void showOrHideSignals(bool doShow);

    //Other signals
    void checkMessages();
    void deleteSelected();
    void setAllGfxType(GraphicsTypeEnumT);
    void componentChanged();
    void connectorRemoved();
    void rotateSelectedObjectsRight();
    void rotateSelectedObjectsLeft();
    void flipSelectedObjectsHorizontal();
    void flipSelectedObjectsVertical();

    void systemParametersChanged();

    void aliasChanged(const QString &rFullName, const QString &rAlias);

protected:

    //Protected methods
    virtual Port* createRefreshExternalPort(QString portName) override;
    virtual void renameExternalPort(QString oldName, QString newName);
    //virtual void openPropertiesDialog();
    void clearContents();
    void forgetSubConnector(Connector *pConnector);
    void refreshInternalSystemPortGraphics();

    //Help function for creating system ports
    virtual void addExternalSystemPortObject(ModelObject *pModelObject);

    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement) override;
    void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel) override;

    //Protected overloaded Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    //Scene pointer and graphics viewport
    QGraphicsScene *mpScene;
    GraphicsViewPort mGraphicsViewPort;

    //Model and script file members
    QFileInfo mModelFileInfo;
    QString mNumHopScript;

    //Model object members
    typedef QHash<QString, ModelObject*> ModelObjectMapT;
    ModelObjectMapT mModelObjectMap;
    QList<ModelObject *> mSelectedModelObjectsList;

    //Connector members
    QList<Connector *> mSelectedSubConnectorsList;
    QList<Connector *> mSubConnectorList;
    Connector *mpTempConnector;
    bool mIsCreatingConnector;

    //Widget members
    QMap<size_t, Widget *> mWidgetMap;
    QList<Widget *> mSelectedWidgetsList;

    //Contained object appearance members
    bool mShowSubComponentPorts;
    bool mShowSubComponentNames;
    bool mSignalsVisible;
    GraphicsTypeEnumT mGfxType;

    //Undo-redo members
    UndoStack *mpUndoStack;
    bool mUndoEnabled;
    bool mSaveUndoStack;

    //Copy-paste members
    CopyStack *mpDragCopyStack;
    double mPasteOffset;

    //Numbered sections members
    QList< QList<ModelObject *> > mSection;

    //Losses members
    bool mLossesVisible;
    QDialog *mpLossesDialog;
    QSlider *mpMinLossesSlider;
    QRadioButton *mpEnergyRadioButton;

    //Model information
    QString mAuthor;
    QString mEmail;
    QString mAffiliation;
    QString mDescription;

    //Time measurement dialog
    QTableView *mpComponentTable;
    QTableView *mpTypeTable;

    //Animation members
    bool mAnimationDisabled = false;

private:
    void commonConstructorCode();
    void loadSensitivityAnalysisSettingsFromDomElement(QDomElement &rDomElement);
    void saveSensitivityAnalysisSettingsToDomElement(QDomElement &rDomElement);
    void loadOptimizationSettingsFromDomElement(QDomElement &rDomElement);
    void saveOptimizationSettingsToDomElement(QDomElement &rDomElement);

    int mNumberOfLogSamples;
    double mLogStartTime;

    QString mLoadType;
    CoreSystemAccess *mpCoreSystemAccess;

    OptimizationSettings mOptSettings;
    SensitivityAnalysisSettings mSensSettings;
};

#endif // GUICONTAINEROBJECT_H
