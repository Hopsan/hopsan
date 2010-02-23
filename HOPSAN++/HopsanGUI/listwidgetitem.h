#ifndef LISTWIDGETITEM_H
#define LISTWIDGETITEM_H

#include <QListWidgetItem>
//#include <QStringList>


class QStringList;

class ListWidgetItem : public QListWidgetItem
{
public:
    ListWidgetItem(const QIcon &icon, const QString &text, QListWidget *parent = 0);
    ListWidgetItem(const QListWidgetItem &other);
    ~ListWidgetItem();

    void setParameterData(QStringList list);
    QStringList getParameterData();


private:
    QStringList mParameterData;
};

#endif // LISTWIDGETITEM_H
