#ifndef COMPONENTPROPERTIESDIALOG3_H
#define COMPONENTPROPERTIESDIALOG3_H

#include <QtGui>
#include "CoreAccess.h"

class ModelObject;
class ParameterSettingsLayout;
class MainWindow;
class SystemParametersWidget;

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
    enum ColumnEnumT {Name, Alias, Unit, Description, Value, Scale, ShowPort, NumCols};
    VariableTableWidget(ModelObject *pModelObject, QWidget *pParent);
    bool setStartValues();
    bool setAliasNames();

private:
    void createTableRow(const int row, const CoreVariameterDescription &rData, const VariameterTypEnumT variametertype);
    void createSeparatorRow(const int row, const QString &rName, const VariameterTypEnumT variametertype);
    void selectValueTextColor(const int row);
    bool cleanAndVerifyParameterValue(QString &rValue, const QString type);
    bool setAliasName(const int row);
    ModelObject *mpModelObject;
};

class ComponentPropertiesDialog3 : public QDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog3(ModelObject *pModelObject, QWidget *pParent=0);

signals:
    void lockModelWidget(bool);

protected slots:
    void okPressed();
    void editPortPos();
    void copyToNewComponent();
    void recompile();
    virtual void reject();

protected:
    bool setAliasNames();
    bool setVariableValues();
    void setName();
    virtual void closeEvent(QCloseEvent* event);

private:
    QGridLayout* createNameAndTypeEdit();
    QDialogButtonBox* createButtonBox();
    QWidget* createHelpWidget();
    QWidget* createSourcodeBrowser(QString &rFilePath);
    void createEditStuff();

    ModelObject *mpModelObject;
    LibraryWidget *mpLibrary;
    SystemParametersWidget *mpSystemParametersWidget;
    QLineEdit *mpNameEdit;
    VariableTableWidget *mpVariableTableWidget;

    QSpinBox *mpInputPortsSpinBox;
    QSpinBox *mpOutputPortsSpinBox;
    QTextEdit *mpTextEdit;
    QPushButton *mpNewComponentButton;
    QPushButton *mpRecompileButton;
    QTextEdit *mpSourceCodeTextEdit;
    QComboBox *mpSolverComboBox;
};

// Help class declarations
//----------------------------------------------------------------------

class PlotScaleSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    PlotScaleSelectionWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent);

private slots:
    void createPlotScaleSelectionMenu();
    void registerCustomScale();


private:
    QLineEdit *mpPlotScaleEdit;
    ModelObject *mpModelObject;
    QString mVariableTypeName, mVariablePortDataName;
};


class ParameterValueSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    ParameterValueSelectionWidget(const CoreVariameterDescription &rData, VariableTableWidget::VariameterTypEnumT type, ModelObject *pModelObject, QWidget *pParent);
    void setValueText(const QString &rText);
    QString getValue() const;
    const QString &getDataType() const;
public slots:
    void refreshValueTextStyle();
private slots:
    void setValue();
    void setConditionalValue(const int idx);
    void resetDefault();
    void createSysParameterSelectionMenu();
private:
    QString mVariableDataType, mVariablePortDataName, mVariablePortName;
    QLineEdit *mpValueEdit;
    QComboBox *mpConditionalValueComboBox;
    VariableTableWidget::VariameterTypEnumT mVariameterType;
    ModelObject *mpModelObject;
    void setDefaultValueTextStyle();
    void decideBackgroundColor(QString &rStyleString);
};

class HideShowPortWidget : public QWidget
{
    Q_OBJECT
public:
    HideShowPortWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent);
signals:
    void toggled(bool);
private slots:
    void hideShowPort(const bool doShow);
private:
    QString mPortName;
    ModelObject *mpModelObject;
};

//----------------------------------------------------------------------

#endif // COMPONENTPROPERTIESDIALOG3_H
