#ifndef LISTWIDGETITEM_H
#define LISTWIDGETITEM_H

#include <QListWidgetItem>

class ListWidgetItem : public QListWidgetItem
{
public:
    ListWidgetItem(QListWidget *parent = 0,int type = 1000);
    ~ListWidgetItem();

    void setIconPath(QString iconPath);
    QString getIconPath();

    void setNumberOfPorts(QString nrPorts);
    QString getNumberOfPorts();

private:
    QString miconPath;
    QString mnrPorts;
};

#endif // LISTWIDGETITEM_H
