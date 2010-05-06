#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QList>
#include <QStringList>
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUIConnector.h"
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>

    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;

class UndoStack : public QObject
{
//friend class undoWidget;

public:
    UndoStack(GraphicsView *parentView);
    ~UndoStack();
    void registerDeletedObject(GUIObject *item);
    void registerDeletedConnector(GUIConnector *item);
    void registerAddedObject(GUIObject *item);
    void registerAddedConnector(GUIConnector *item);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName);
    void registerRotatedObject(GUIObject *item);
    void clear();
    void newPost();
    void insertPost(QStringList(list));
    //void newRedoPost();
    void undoOneStep();
    void redoOneStep();

    QList< QList<QStringList> > mStack; //! @todo These two shall not be public, but for some reason friend class doesn't work...
    int mCurrentStackPosition;

private:
    GraphicsView *mpParentView;
};


class UndoWidget : public QDialog
{
public:
    UndoWidget(MainWindow *parent = 0);
    ~UndoWidget();

    MainWindow *mpParentMainWindow;

    QTableWidget *mUndoTable;
    QList< QList<QStringList> > mTempStack;

    QPushButton *hideButton;
    QPushButton *undoButton;
    QPushButton *redoButton;

    void show();
    void refreshList();
};


#endif // UNDOSTACK_H
