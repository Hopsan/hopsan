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
    void store(GUIObject *item);
    void store(GUIConnector *item);
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
