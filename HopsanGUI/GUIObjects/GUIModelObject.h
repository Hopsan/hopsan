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
//! @file   GUIModelObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIModelObject class (The baseclass for all objects representing model parts)
//!
//$Id$

#ifndef GUIMODELOBJECT_H
#define GUIMODELOBJECT_H

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"
#include "UnitScale.h"
#include "CoreAccess.h"
#include <QGraphicsSvgItem>

class Connector;
class ModelObjectDisplayName;
class Port;
class SystemObject;
class ComponentPropertiesDialog3;

class ModelObject : public WorkspaceObject
{
    Q_OBJECT

public:
    explicit ModelObject(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected = Deselected,
                         GraphicsTypeEnumT graphics = UserGraphics, SystemObject *pParentSystem=0, QGraphicsItem *pParentGraphicsItem=0);
    virtual ~ModelObject();
    virtual void deleteInHopsanCore();

    virtual void setParentSystemObject(SystemObject *pParentSystem) override;

    QStringList getParentSystemNameHieararchy() const;
    virtual QStringList getSystemNameHieararchy() const;

    // Name methods
    virtual void setName(QString name);
    virtual QString getName() const;
    virtual void refreshDisplayName(const QString overrideName="");
    virtual QString getTypeName() const = 0;
    QString getSubTypeName() const;
    void setSubTypeName(const QString subTypeName);
    virtual int getNameTextPos();
    virtual void setNameTextPos(int textPos);

    // CQS methods
    virtual QString getTypeCQS() const {return "hasNoCqsType";}

    // Appearance methods
    void setAppearanceDataBasePath(const QString basePath);
    virtual ModelObjectAppearance* getAppearanceData();
    virtual const SharedModelObjectAppearanceT getLibraryAppearanceData() const;
    bool isVisible();
    QGraphicsSvgItem *getIcon();
    void setAlwaysVisible(const bool visible, UndoStatusEnumT undoSettings=Undo);
    void setNameTextAlwaysVisible(const bool isVisible);

    // Help methods
    const QString &getHelpPicture() const;
    const QString &getHelpText() const;
    const QStringList &getHelpLinks() const;
    QString getHelpHtmlPath() const;

    // Parameter methods
    virtual QStringList getParameterNames();
    virtual void getParameters(QVector<CoreParameterData> &rParameterDataVec);
    virtual void getParameter(const QString paramName, CoreParameterData &rData);
    virtual QString getParameterValue(const QString paramName);
    virtual bool hasParameter(const QString &rParamName);

    virtual QString getDefaultParameterValue(const QString &rParamName) const;

    virtual bool setParameterValue(QString name, QString valueTxt, bool force=false);
    virtual QString getStartValueTxt(QString portName, QString variable);
    virtual bool setStartValue(QString portName, QString variable, QString sysParName);

    virtual bool registerCustomParameterUnitScale(QString name, UnitConverter us);
    virtual bool unregisterCustomParameterUnitScale(QString name);
    virtual bool getCustomParameterUnitScale(QString name, UnitConverter &rUs);

    virtual void openPropertiesDialog();

    void saveParameterValuesToFile(QString parameterFile = {});
    virtual void loadParameterValuesFromFile(QString parameterFile = {});

    virtual QMap<QString, QString> getParameterSets() const;

    // VariableAlias method
    //! @todo parameters and port variables should be more similar in the future, so that we do not need handle them separately
    virtual QMap<QString, QString> getVariableAliases(const QString &rPortName="") const;
    //virtual void getVariableDataDescriptions(QVector<CoreVariableData> &rVarDataDescriptions);
    virtual void getVariameterDescriptions(QVector<CoreVariameterDescription> &rVariameterDescriptions) const;

    // Custom variable plot settings methods
    void setInvertPlotVariable(const QString &rName, bool tf);
    bool getInvertPlotVariable(const QString &rName) const;
    void setVariablePlotLabel(const QString &rName, const QString &rLabel);
    QString getVariablePlotLabel(const QString &rName) const;

    // Custom quantity methods
    bool setModifyableSignalQuantity(const QString &rVariablePortDataName, const QString &rQuantity);
    QString getModifyableSignalQuantity(const QString &rVariablePortDataName);

    // Load and save methods
    virtual void loadFromDomElement(QDomElement domElement) override;
    virtual void saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel) override;
    virtual void setModelFileInfo(QFile &rFile, const QString relModelPath="");

    // Connector methods
    QList<Connector*> getConnectorPtrs();
    void rememberConnector(Connector *item);
    void forgetConnector(Connector *item);

    // Port methods
    void showPorts(bool visible);
    Port *getPort(const QString &rName);
    QList<Port*> &getPortListPtrs();
    virtual Port* createRefreshExternalPort(QString portName);
    virtual void removeExternalPort(QString portName, bool keepBrokenConnector=false);

    // Type info
    virtual int type() const override = 0;
    virtual QString getHmfTagName() const override;

    void getLosses(double &total, QMap<QString, double> domainSpecificLosses);
    bool isLossesDisplayVisible();

    void setFallbackDomElement(const QDomElement &rDomElement);
    QDomElement getFallbackDomElement();

public slots:
    virtual void refreshAppearance();
    virtual void refreshExternalPortsAppearanceAndPosition();
    void deleteMe(UndoStatusEnumT undoSettings=Undo) override;
    virtual void rotate(double angle, UndoStatusEnumT undoSettings = Undo) override;
    virtual void flipVertical(UndoStatusEnumT undoSettings = Undo) override;
    virtual void flipHorizontal(UndoStatusEnumT undoSettings = Undo) override;
    void hideName(UndoStatusEnumT undoSettings = NoUndo);
    void showName(UndoStatusEnumT undoSettings = NoUndo);
    void setIcon(GraphicsTypeEnumT);
    void showLosses();
    void hideLosses();
    void redrawConnectors();
    void highlight();
    void unHighlight();
    void setIsLocked(bool value);
    void setDisabled(bool value);
    bool isDisabled();

signals:
    void nameChanged();
    void quantityChanged(QString, QString);

protected:
    // Protected methods
    virtual QAction *buildBaseContextMenu(QMenu &rMenue, QGraphicsSceneContextMenuEvent* pEvent);

    // Reimplemented Qt methods
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    // Save and load methods
    virtual QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    virtual void saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents=FullModel);

    // Protected members
    ModelObjectAppearance mModelObjectAppearance;

    QString mName;
    ModelObjectDisplayName *mpNameText;
    double mTextOffset;
    int mNameTextPos;

    bool mAlwaysVisible;
    bool mNameTextAlwaysVisible;
    bool mNameTextVisible;

    GraphicsTypeEnumT mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;
    QString mLastIconPath;
    double mLastIconScale;

    QMap<QString, QString> mDefaultParameterValues;
    QMap<QString, UnitConverter> mRegisteredCustomParameterUnitScales;
    QMap<QString, bool> mRegisteredInvertPlotVariables;
    QMap<QString, QString> mRegisteredPlotLabels;

    QList<Port*> mPortListPtrs;
    QList<Connector*> mConnectorPtrs;

    QGraphicsTextItem *mpLossesDisplay;

    double mTotalLosses;
    QMap<QString, double> mDomainSpecificLosses;

    // Used by C++ components
    QString mCppCode;
    int mnCppInputs;
    int mnCppOutputs;

    bool mDragCopying;
    QWidget *mpDialogParentWidget;
    QPointer<ComponentPropertiesDialog3> mpPropertiesDialog;

    QTimer mDragCopyTimer;

    QDomElement mFallbackDomElement;

protected slots:
    void snapNameTextPosition(QPointF pos);
    void calcNameTextPositions(QVector<QPointF> &rPts);
    void setNameTextScale(double scale);
    void setIconZoom(const double zoom);
    void setDragCopying();

private:
    void refreshIconPosition();
};


class ModelObjectDisplayName : public QGraphicsTextItem
{
    Q_OBJECT

public:
    ModelObjectDisplayName(ModelObject *pParent);

public slots:
    void deselect();

signals:
    void textMoved(QPointF pos);

protected:
    // Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    // Protected members
    ModelObject* mpParentModelObject;
};

#endif // GUIMODELOBJECT_H
