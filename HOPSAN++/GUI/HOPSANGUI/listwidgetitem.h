#ifndef LISTWIDGETITEM_H
#define LISTWIDGETITEM_H

#include <QListWidgetItem>

class ListWidgetItem : public QListWidgetItem
{
public:
    ListWidgetItem(QListWidget *parent = 0,int type = 1000);
    ~ListWidgetItem();

    QString *iconDir;
};

#endif // LISTWIDGETITEM_H
