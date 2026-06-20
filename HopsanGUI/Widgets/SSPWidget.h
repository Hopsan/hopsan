#ifndef SSPWIDGET_H
#define SSPWIDGET_H

#include <QFileInfo>
#include <QObject>
#include <QTreeWidget>
#include <QWidget>

//Forward declarations
struct sspHandle;
struct ssdHandle;
struct ssvParameterSetHandle;
struct ssmParameterMappingHandle;
struct ssdSystemHandle;

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

class SSPWidget : public QWidget
{
    Q_OBJECT
public:
    SSPWidget(QWidget *pParent=0);

    void addSSP(QFileInfo path);

public slots:
    void saveSspModel(QTreeWidgetItem *item);

protected slots:
    void openSSDModel(QTreeWidgetItem*item, int);
    void openSSVEditor(QTreeWidgetItem*item, int);
    void openSSMEditor(QTreeWidgetItem*item, int);

private:
    SSPTreeWidget *mpTree;

    QMap<QTreeWidgetItem *, sspHandle *> itemToSspMap;
    QMap<QTreeWidgetItem *, ssdHandle *> itemToSsdMap;
    QMap<QTreeWidgetItem *, ssdSystemHandle *> itemToSystemMap;
    QMap<QTreeWidgetItem *, ssvParameterSetHandle *> itemToSsvMap;
    QMap<QTreeWidgetItem *, ssmParameterMappingHandle *> itemToSsmMap;
    QMap<sspHandle *, QFileInfo> mSspFileMap;
};

#endif // SSPWIDGET_H
