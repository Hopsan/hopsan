#ifndef COMPONENTPROPERTIESDIALOG3_H
#define COMPONENTPROPERTIESDIALOG3_H

#include <QtGui>
#include "Dialogs/ModelObjectPropertiesDialog.h"
#include "CoreAccess.h"

class Component;
class ParameterSettingsLayout;
class MainWindow;

class VariableTableWidget :public QTableWidget
{
    Q_OBJECT
public:
    VariableTableWidget(Component *pComponent, QWidget *pParent);

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
