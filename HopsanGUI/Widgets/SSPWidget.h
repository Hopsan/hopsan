#include <QWidget>
#include <QObject>
#include <QTreeWidget>

#ifndef SSPWIDGET_H
#define SSPWIDGET_H

#include <QFileInfo>

//Forward declarations
class sspHandle;
class ssdHandle;

class SSPWidget : public QWidget
{
    Q_OBJECT
public:
    SSPWidget(QWidget *pParent=0);

    void addSSP(QFileInfo path);
public slots:

protected slots:
    void openSSDModel(QTreeWidgetItem*item, int);

private:
    QTreeWidget *mpTree;

    QMap<QTreeWidgetItem *, sspHandle *> itemToSspMap;
    QMap<QTreeWidgetItem *, ssdHandle *> itemToSsdMap;
};

#endif // SSPWIDGET_H
