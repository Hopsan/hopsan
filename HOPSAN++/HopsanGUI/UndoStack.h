//$Id$

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

#include <QDomElement>
#include <QDomDocument>

    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;
class MainWindow;
class GUIContainerObject;
class GUIWidget;
class GUIBoxWidget;
class GUITextWidget;

class UndoStack : public QObject
{
friend class UndoWidget;

public:
    UndoStack(GUIContainerObject *parentSystem);
    void registerDeletedObject(GUIObject *item);
    void registerDeletedConnector(GUIConnector *item);
    void registerAddedObject(GUIObject *item);
    void registerAddedConnector(GUIConnector *item);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName);
    void registerRotatedObject(QString objectName);
    void registerVerticalFlip(QString objectName);
    void registerHorizontalFlip(QString objectName);
    void registerChangedParameter(QString objectName, QString parameterName, double oldValue, double newValue);
    void registerNameVisibilityChange(QString objectName, bool isVisible);
    void registerAddedBoxWidget(GUIBoxWidget *item);
    void registerDeletedBoxWidget(GUIBoxWidget *item);
    void registerAddedTextWidget(GUITextWidget *item);
    void registerDeletedTextWidget(GUITextWidget *item);
    void registerMovedWidget(GUIWidget *item, QPointF oldPos, QPointF newPos);
    void clear(QString errorMsg = "");
    void newPost(QString type = "");
    void insertPost(QString str);
    void undoOneStep();
    void redoOneStep();

private:
    GUIContainerObject *mpParentContainerObject;
    QList< QList<QString> > mStack;
    int mCurrentStackPosition;

    QDomElement getCurrentPost();
    QDomDocument mDomDocument;
    QDomElement mUndoRoot;
};


#endif // UNDOSTACK_H
