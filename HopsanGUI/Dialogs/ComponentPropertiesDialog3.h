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
//! @file   ComponentPropertiesDialog3.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @brief Contains the component properties dialog
//!
//$Id$

#ifndef COMPONENTPROPERTIESDIALOG3_H
#define COMPONENTPROPERTIESDIALOG3_H

#include <QTableWidget>
#include <QDialog>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QComboBox>
#include <QEvent>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>

#include "CoreAccess.h"
#include "UnitScale.h"
#include "Utilities/EventFilters.h"


class ModelObject;
class SystemParametersWidget;
class SystemProperties;


class TableWidgetTotalSize : public QTableWidget
{
public:
    TableWidgetTotalSize(QWidget *pParent=0);
    QSize sizeHint() const;
    void setMaxVisibleRows(const int maxRows);
private:
    int mMaxVisibleRows;
};

class VariableTableWidget :public TableWidgetTotalSize
{
    Q_OBJECT
public:
    enum VariameterTypEnumT {InputVaraiable, OutputVariable, OtherVariable, Constant}; //!< @todo maybe not only here
    enum ColumnEnumT {Name, Alias, Description, Unit, Value, Quantity, PlotSettings, ShowPort, NumCols};
    VariableTableWidget(ModelObject *pModelObject, QWidget *pParent);
    bool setStartValues();
    //bool setCustomPlotScaleValues();
    bool setAliasNames();
    void setValue(const QString &rName, const QString &rValue);
    QString getSelectedUnit(const QString &rName);

protected:
    virtual bool focusNextPrevChild(bool next);

private:
    void createTableRow(const int row, const CoreVariameterDescription &rData, const VariameterTypEnumT variametertype);
    void createSeparatorRow(const int row, const QString &rName, const VariameterTypEnumT variametertype);
    void selectValueTextColor(const int row);
    bool cleanAndVerifyParameterValue(QString &rValue, const QString type);
    bool setAliasName(const int row);
    ModelObject *mpModelObject;

signals:
    void refreshInternalPortInfo();
};

class ComponentPropertiesDialog3 : public QDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog3(ModelObject *pModelObject, QWidget *pParent=0);

signals:
    void setLimitedModelEditingLock(bool);

protected slots:
    void showEvent(QShowEvent *event);
    void okPressed();
    void applyAndSimulatePressed();
    void applyPressed();
    void openSourceCode();
    void editPortPos();
    virtual void reject();
    void openDescription();
    void applyParameterSet();

protected:
    bool setAliasNames();
    bool setVariableValues();
    void setName();
    void setSystemProperties();
    virtual void closeEvent(QCloseEvent* event);

private:
    QGridLayout* createNameAndTypeEdit();
    QDialogButtonBox* createOKButtonBox();
    QWidget* createHelpWidget();
    void createEditStuff();

    ModelObject *mpModelObject;
    QLineEdit *mpNameEdit;
    VariableTableWidget *mpVariableTableWidget;
    SystemProperties *mpSystemProperties=nullptr;
    bool mAllowEditing=false;
    QComboBox *mpSetsComboBox;
};

// Help class declarations
//----------------------------------------------------------------------

class SystemProperties : public QObject
{
    Q_OBJECT

public:
    SystemProperties(SystemObject *pSystemObject, QWidget *pParentWidget);

    QWidget* createSystemSettings();
    QWidget* createAppearanceSettings();
    QWidget* createModelinfoSettings();

public slots:
    void setValues();

private:
    SystemObject *mpSystemObject;

    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLineEdit *mpNumLogSamplesEdit;
    QLineEdit *mpLogStartTimeEdit;
    QLineEdit *mpIsoIconScaleEdit;
    QLineEdit *mpUserIconScaleEdit;
    QPushButton *mpIsoIconBrowseButton;
    QPushButton *mpUserIconBrowseButton;
    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpEnableUndoCheckBox;
    QCheckBox *mpSaveUndoCheckBox;

    QCheckBox *mpDisableAnimationCheckBox;

    QCheckBox *mpTimeStepCheckBox;
    QCheckBox *mpKeepValuesAsStartValues;
    QLineEdit *mpTimeStepEdit;

    QLineEdit *mpAuthorEdit;
    QLineEdit *mpEmailEdit;
    QLineEdit *mpAffiliationEdit;
    QTextEdit *mpDescriptionEdit;

private slots:
    void fixTimeStepInheritance(bool value);
    void browseUser();
    void browseIso();
    void clearLogData();
};

class PlotSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    PlotSettingsWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent);
    QLineEdit* plotLabel();
private slots:
    void invertPlot(bool tf);
    void setPlotLabel(QString label);
private:
    ModelObject *mpModelObject;
    QLineEdit *mpPlotLabel;
    QString mVariablePortDataName, mOriginalPlotLabel;
    bool mOrigInverted;

};

class QuantitySelectionWidget : public QWidget
{
    Q_OBJECT
public:
    QuantitySelectionWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent);
    void registerCustomQuantity();
    bool hasChanged() const;
    //QLineEdit *getQuantityLabel() const;

private slots:
    void createQuantitySelectionMenu();

private:
    QLabel *mpQuantityLabel;
    ModelObject *mpModelObject;
    QString mVariableTypeName, mVariablePortDataName, mOriginalUnit, mQuantity;
};

class UnitSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    UnitSelectionWidget(const QString &rQuantity, QWidget *pParent);
    void setUnitScaling(const UnitConverter &rUs);
    QString getSelectedUnit() const;
    double getSelectedUnitScale() const;
    void getSelectedUnitScale(UnitConverter &rUnitScale) const;
    bool isDefaultSelected() const;

public slots:
    void resetDefault();

signals:
    void unitChanged(const UnitConverter &rUnitScale);

private slots:
    void selectionChanged(int idx);

private:
    QComboBox *mpUnitComboBox;
    QList<UnitConverter> mUnitScales;
    QString mQuantity;
    QString mBaseUnit;
    int mBaseUnitIndex;
};

class ParameterValueSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    ParameterValueSelectionWidget(const CoreVariameterDescription &rData, VariableTableWidget::VariameterTypEnumT type, ModelObject *pModelObject, QWidget *pParent);
    void setValueText(const QString &rText);
    QString getValueText() const;
    const QString &getDataType() const;
    const QString &getName() const;
    UnitSelectionWidget *getUnitSelectionWidget();
    bool isValueDisabled() const;
    QLineEdit *getValueEditPtr() const;
    void setValueAndScale_nosignals(QString value, UnitConverter &rCustomUS);

public slots:
    void refreshValueTextStyle();
    void rescaleByUnitScale(const UnitConverter &rUnitScale);
    bool checkIfSysParEntered();

private slots:
    void setValue();
    void setConditionalValue(const int idx);
    void resetDefault();
    void createSysParameterSelectionMenu();
    void openValueEditDialog();
    void openFileBrowserDialog();

private:
    QString mVariableDataType, mVariablePortDataName, mVariablePortName;
    QLineEdit *mpValueEdit;
    QComboBox *mpConditionalValueComboBox;
    UnitSelectionWidget *mpUnitSelectionWidget;
    VariableTableWidget::VariameterTypEnumT mVariameterType;
    ModelObject *mpModelObject;
    UnitConverter mCustomScale;
    UnitConverter mDefaultUnitScale;
    void setDefaultValueTextStyle();
    void decideBackgroundColor(QString &rStyleString);
};

class HideShowPortWidget : public QWidget
{
    Q_OBJECT
public:
    HideShowPortWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent);
    void refreshPortToggleState();
    QCheckBox *getCheckBoxPtr() const;
signals:
    void toggled(bool);
private slots:
    void hideShowPort(const bool doShow);
private:
    QCheckBox *mpCheckBox;
    QString mPortName;
    ModelObject *mpModelObject;
};

//----------------------------------------------------------------------

#endif // COMPONENTPROPERTIESDIALOG3_H
