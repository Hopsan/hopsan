#ifndef PROJECTFILESWIDGET_H
#define PROJECTFILESWIDGET_H

#include <QWidget>
#include <QTreeWidget>

#include "Handlers/FileHandler.h"

class ProjectFilesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectFilesWidget(QWidget *parent = 0);

    QTreeWidget *mpTreeWidget;

signals:
    void deleteRequested(QTreeWidgetItem*);

public slots:
    QTreeWidgetItem *addFile(const FileObject *pFile);
    void addAsterisk();
    void removeAsterisks();
    void removeItem(QTreeWidgetItem *pItem);
    void clear();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTreeWidgetItem *mpProjectFilesTopLevelItem;
    QTreeWidgetItem *mpComponentFilesTopLevelItem;
    QTreeWidgetItem *mpAuxiliaryFilesTopLevelItem;
};

#endif // PROJECTFILESTREE_H
