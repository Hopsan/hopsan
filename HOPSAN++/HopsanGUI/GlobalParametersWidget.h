#ifndef GLOBALPARAMETERSWIDGET_H
#define GLOBALPARAMETERSWIDGET_H

#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QGridLayout>
#include <qwidget.h>
#include <qlabel.h>

    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;
class MainWindow;
class GUISystem;


class GlobalParametersWidget : public QWidget
{
    Q_OBJECT
public:
    GlobalParametersWidget(MainWindow *parent = 0);

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
    void modifiedGlobalParameter();

private:
    QLabel *mpNameLabel;
    QLineEdit *mpNameBox;
    QLabel *mpValueLabel;
    QLineEdit *mpValueBox;
    QPushButton *mpAddInDialogButton;
    QPushButton *mpDoneInDialogButton;
    //QList< QPair<QString, double> > mContents;

    QTableWidget *mpGlobalParametersTable;
    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
    QPushButton *mpCloseButton;
    QGridLayout *mpGridLayout;
    QMap<QString,double> mGlobalParametersMap;
};


#endif // GLOBALPARAMETERSWIDGET_H
