#ifndef SystemParameterSWIDGET_H
#define SystemParameterSWIDGET_H

#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QToolButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <qwidget.h>
#include <qlabel.h>

#include <common.h>

//Ska bort:
#include <ProjectTabWidget.h>
#include <MainWindow.h>
#include <QIcon>
#include <GUIObjects/GUISystem.h>


    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;
class MainWindow;
class GUISystem;
class GUIModelObject;


class SystemParametersWidget : public QWidget
{
    Q_OBJECT
public:
    SystemParametersWidget(MainWindow *parent = 0);

    //MainWindow *mpParentMainWindow;

    double getParameter(QString name);
    bool hasParameter(QString name);

public slots:
    void setParameter(QString name, double value);
    void setParameters();

private slots:
    void openComponentPropertiesDialog();
    void addParameter();
    void removeSelectedParameters();
    void update();

signals:
    void modifiedSystemParameter();

private:
    QLabel *mpNameLabel;
    QLineEdit *mpNameBox;
    QLabel *mpValueLabel;
    QLineEdit *mpValueBox;
    QPushButton *mpAddInDialogButton;
    QPushButton *mpDoneInDialogButton;
    //QList< QPair<QString, double> > mContents;

    QTableWidget *mpSystemParametersTable;
    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
    QPushButton *mpCloseButton;
    QGridLayout *mpGridLayout;
};


//! @class This class implement a Layout for user interaction of values with name, value and units
class ParameterLayout : public QGridLayout
{
    Q_OBJECT

public:

    ParameterLayout(QString dataName="", QString descriptionName="", double dataValue=0, QString unitName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);

    ParameterLayout(QString dataName="", QString descriptionName="", QString dataValue="", QString unitName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);

    void commonConstructorCode(QString dataName="", QString descriptionName="", QString dataValue="", QString unitName="", GUIModelObject *pGUIModelObject=0, QWidget *parent=0);

    QString getDescriptionName();

    QString getDataName();

    double getDataValue();
    QString getDataValueTxt();

protected slots:

    void showListOfSystemParameters();

protected:
    GUIModelObject *mpGUIModelObject;

    QLabel mDataNameLabel;
    QLabel mDescriptionNameLabel;
    QLineEdit mDataValuesLineEdit;
    QLabel mUnitNameLabel;

    QToolButton mSystemParameterToolButton;
};


#endif // SystemParameterSWIDGET_H
