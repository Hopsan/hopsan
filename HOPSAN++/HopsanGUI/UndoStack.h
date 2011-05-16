/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   UndoStack.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
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
class GUIModelObject;
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
    void registerDeletedObject(GUIModelObject *item);
    void registerDeletedConnector(GUIConnector *item);
    void registerAddedObject(GUIModelObject *item);
    void registerAddedConnector(GUIConnector *item);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, QPointF newPos, GUIConnector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName);
    void registerRotatedObject(QString objectName, double angle);
    void registerVerticalFlip(QString objectName);
    void registerHorizontalFlip(QString objectName);
    void registerChangedParameter(QString objectName, QString parameterName, QString oldValueTxt, QString newValueTxt);
    void registerChangedStartValue(QString objectName, QString portName, QString parameterName, QString oldValueTxt, QString newValueTxt);
    void registerNameVisibilityChange(QString objectName, bool isVisible);
    void registerAddedBoxWidget(GUIBoxWidget *item);
    void registerDeletedBoxWidget(GUIBoxWidget *item);
    void registerResizedBoxWidget(int index, double w_old, double h_old, double w_new, double h_new, QPointF oldPos, QPointF newPos);
    void registerModifiedBoxWidgetStyle(int index, int oldLineWidth, Qt::PenStyle oldLineStyle, QColor oldLineColor, int lineWidth, Qt::PenStyle lineStyle, QColor lineColor);
    void registerAddedTextWidget(GUITextWidget *item);
    void registerDeletedTextWidget(GUITextWidget *item);
    void registerModifiedTextWidget(int index, QString oldText, QFont oldFont, QColor oldColor, QString text, QFont font, QColor color);
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
