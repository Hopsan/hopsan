#ifndef LISTWIDGETITEM_H
#define LISTWIDGETITEM_H

#include <QListWidgetItem>

class ListWidgetItem : public QListWidgetItem
{
public:
    ListWidgetItem(const QIcon &icon, const QString &text, QListWidget *parent = 0);
    ListWidgetItem(const QListWidgetItem &other);
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
