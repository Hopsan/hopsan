#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QList>
#include <QStringList>
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUIConnector.h"

    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;

class UndoStack
{
public:
    UndoStack(GraphicsView *parentView);
    ~UndoStack();
    void registerDeletedObject(GUIObject *item);
    void registerDeletedConnector(GUIConnector *item);
    void registerAddedObject(QString itemName);
    void registerAddedConnector(GUIConnector *item);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, GUIConnector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QString objectName);
    void clear();
    void newPost();
    void undoOneStep();
    void redoOneStep();

private:
    QList<QStringList> mStack;
    int mCurrentStackPosition;
    GraphicsView *mpParentView;
};

#endif // UNDOSTACK_H
