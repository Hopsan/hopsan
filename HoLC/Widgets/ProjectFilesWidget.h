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
    void update();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTreeWidgetItem *mpProjectFilesTopLevelItem;
    QTreeWidgetItem *mpComponentFilesTopLevelItem;
    QTreeWidgetItem *mpAuxiliaryFilesTopLevelItem;
    QTreeWidgetItem *mpAppearanceFilesTopLevelItem;
};

#endif // PROJECTFILESTREE_H
