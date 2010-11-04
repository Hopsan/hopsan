#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QGridLayout>

    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;
class MainWindow;
class GUISystem;

class UndoStack : public QObject
{
friend class UndoWidget;

public:
    UndoStack(GUISystem *parentSystem);
    void registerDeletedObject(GUIObject *item);
    void registerDeletedConnector(GUIConnector *item);
    void registerAddedObject(GUIObject *item);
    void registerAddedConnector(GUIConnector *item);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName);
    void registerRotatedObject(GUIObject *item);
    void registerVerticalFlip(GUIObject *item);
    void registerHorizontalFlip(GUIObject *item);
    void clear();
    void newPost();
    void insertPost(QString str);
    void undoOneStep();
    void redoOneStep();

private:
    GUISystem *mpParentSystem;
    QList< QList<QString> > mStack;
    int mCurrentStackPosition;
};


class UndoWidget : public QDialog
{
public:
    UndoWidget(MainWindow *parent = 0);
    void show();
    void refreshList();
    QPushButton *getUndoButton();
    QPushButton *getRedoButton();
    QPushButton *getClearButton();

    MainWindow *mpParentMainWindow;

private:
    QTableWidget *mUndoTable;
    QList< QList<QString> > mTempStack;
    QPushButton *mpUndoButton;
    QPushButton *mpRedoButton;
    QPushButton *mpClearButton;
    QGridLayout *mpLayout;
};


#endif // UNDOSTACK_H
