#ifndef COMPONENTPROPERTIESDIALOG3_H
#define COMPONENTPROPERTIESDIALOG3_H

#include <QtGui>
#include "Dialogs/ModelObjectPropertiesDialog.h"
#include "CoreAccess.h"

class Component;
class ParameterSettingsLayout;
class MainWindow;

class RowAwareToolButton :public QToolButton
{
    Q_OBJECT

public:
    RowAwareToolButton(const int row) : QToolButton()
    {
        mRow = row;
        connect(this, SIGNAL(clicked()), this, SLOT(clickedSlot()));
    }

    void setRow(const int row)
    {
        mRow = row;
    }

signals:
    void triggeredAtRow(int);

private:
    int mRow;

private slots:
    void clickedSlot()
    {
        emit triggeredAtRow(mRow);
    }
};

class VariableTableWidget :public QTableWidget
{
    Q_OBJECT
public:
    enum ColumnEnumT {Name, Alias, Unit, Description, Type, Value, Scale, ResetButton, SysparButton, ShowHidePortButton, NumCols};
    VariableTableWidget(Component *pComponent, QWidget *pParent);

private slots:
    void resetDefaultValueAtRow(int row);
    void selectSystemParameterAtRow(int row);
    void makePortAtRow(int row, bool isPort);

private:
    void createTableRow(const int row, const CoreParameterData &rData);
    void createSeparatorRow(const int row, const QString name);
    Component *mpComponent;

};

class ComponentPropertiesDialog3 : public ModelObjectPropertiesDialog
{
    Q_OBJECT

public:
    ComponentPropertiesDialog3(Component *pComponent, QWidget *pParent=0);

protected slots:
    void okPressed();
    void editPortPos();

protected:
    void setParametersAndStartValues();
    void setName();

private:
    Component *mpComponent;
    VariableTableWidget *mpVariableTableWidget;

    void createEditStuff();
    void createHelpStuff();
    void createNameAndTypeStuff();
    bool interpretedAsStartValue(QString &parameterDescription);

    QGridLayout *mpMainLayout;


    QLineEdit *mpNameEdit;

    QWidget *mpExtension;
};

#endif // COMPONENTPROPERTIESDIALOG3_H
