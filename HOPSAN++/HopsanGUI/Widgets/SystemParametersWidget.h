//!
//! @file   SystemParametersWidget.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-11-20
//!
//! @brief Contains a System parameter widget class
//!
//$Id$

#ifndef SYSTEMPARAMETERSWIDGET_H
#define SYSTEMPARAMETERSWIDGET_H

#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>


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

#endif // SYSTEMPARAMETERSWIDGET_H
