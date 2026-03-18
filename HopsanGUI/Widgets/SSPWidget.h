#include <QWidget>
#include <QObject>
#include <QTreeWidget>

#ifndef SSPWIDGET_H
#define SSPWIDGET_H

#include <QFileInfo>

//Forward declarations
class sspHandle;
class ssdHandle;
class ssvParameterSetHandle;
class ssmParameterMappingHandle;
class ssdSystemHandle;
class SSPTreeWidget;

class SSPWidget : public QWidget
{
    Q_OBJECT
public:
    SSPWidget(QWidget *pParent=0);

    void addSSP(QFileInfo path);

    public slots:

protected slots:
    void openSSDModel(QTreeWidgetItem*item, int);
    void openSSVEditor(QTreeWidgetItem*item, int);

private:
    SSPTreeWidget *mpTree;

    QMap<QTreeWidgetItem *, sspHandle *> itemToSspMap;
    QMap<QTreeWidgetItem *, ssdHandle *> itemToSsdMap;
    QMap<QTreeWidgetItem *, ssdSystemHandle *> itemToSystemMap;
    QMap<QTreeWidgetItem *, ssvParameterSetHandle *> itemToSsvMap;
    QMap<QTreeWidgetItem *, ssmParameterMappingHandle *> itemToSsmMap;
};

class SSPTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit SSPTreeWidget(QWidget *parent = nullptr);

    enum ItemType {
        SSPItem,
        SSDItem = QTreeWidgetItem::UserType + 1,
        SystemItem,
        FMUItem,
        SSVItem,
        SSMItem
    };

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};



#endif // SSPWIDGET_H
