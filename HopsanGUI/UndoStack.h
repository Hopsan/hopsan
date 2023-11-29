/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

//Undo defines
#define UNDO_ADDEDOBJECT "addedobject"
#define UNDO_ADDEDCONNECTOR "addedconnector"
#define UNDO_DELETEDOBJECT "deletedobject"
#define UNDO_DELETEDCONNECTOR "deletedconnector"
#define UNDO_MOVEDOBJECT "movedobject"
#define UNDO_RENAME "rename"
#define UNDO_MODIFIEDCONNECTOR "modifiedconnector"
#define UNDO_ROTATE "rotate"
#define UNDO_VERTICALFLIP "verticalflip"
#define UNDO_HORIZONTALFLIP "horizontalflip"
#define UNDO_NAMEVISIBILITYCHANGE "namevisibilitychange"
#define UNDO_ALWAYSVISIBLECHANGE "alwaysvisiblechange"
#define UNDO_REMOVEDALIASES "removedaliases"
#define UNDO_PASTE "paste"
#define UNDO_MOVEDMULTIPLE "movedmultiple"
#define UNDO_CUT "cut"
#define UNDO_CHANGEDPARAMETERS "changedparameters"
#define UNDO_HIDEALLNAMES "hideallnames"
#define UNDO_SHOWALLNAMES "showallnames"
#define UNDO_MOVEDWIDGET "movedwidget"
#define UNDO_MOVEDMULTIPLEWIDGETS "movedmultiplewidgets"
#define UNDO_ALIGNX "alignx"
#define UNDO_ALIGNY "aligny"
#define UNDO_DISTRIBUTEX "distributex"
#define UNDO_DISTRIBUTEY "distributey"
#define UNDO_DELETEDSYSTEMPORT "deletedsystemport"
#define UNDO_DELETEDSUBSYSTEM "deletedsubsystem"
#define UNDO_ADDEDSYSTEMPORT "addedsystemport"
#define UNDO_ADDEDSUBSYSTEM "addedsubsystem"
#define UNDO_MOVEDCONNECTOR "movedconnector"
#define UNDO_CHANGEDPARAMETER "changedparameter"
#define UNDO_ADDEDTEXTBOXWIDGET "addedtextboxwidget"
#define UNDO_ADDEDIMAGEWIDGET "addedimagewidget"
#define UNDO_DELETEDIMAGEWIDGET "deletedimagewidget"
#define UNDO_MODIFIEDIMAGEWIDGET "modifiedimagewidget"
#define UNDO_DELETEDTEXTBOXWIDGET "deletedtextboxwidget"
#define UNDO_RESIZEDTEXTBOXWIDGET "resizedtextboxwidget"
#define UNDO_MODIFIEDTEXTBOXWIDGET "modifiedtextboxwidget"

    //Forward Declarations
class ModelObject;
class GraphicsView;
class Connector;
class MainWindow;
class SystemObject;
class Widget;
class UndoWidget;

class UndoStack
{
friend class UndoWidget;

public:
    UndoStack(SystemObject *parentSystem);

    QDomElement toXml();
    void fromXml(QDomElement &undoElement);
    void setEnabled(bool enabled);
    void clear(QString errorMsg = "");
    void newPost(QString type = "");
    void insertPost(QString str);
    void undoOneStep();
    void redoOneStep();

    void registerDeletedObject(ModelObject *item);
    void registerDeletedConnector(Connector *item);
    void registerAddedObject(ModelObject *item);
    void registerAddedConnector(Connector *pConnector);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, QPointF newPos, Connector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName);
    void registerRotatedObject(const QString objectName, const double angle);
    void registerVerticalFlip(QString objectName);
    void registerHorizontalFlip(QString objectName);
    void registerChangedParameter(QString objectName, QString parameterName, QString oldValueTxt, QString newValueTxt);
    void registerChangedStartValue(QString objectName, QString portName, QString parameterName, QString oldValueTxt, QString newValueTxt);
    void registerNameVisibilityChange(QString objectName, bool isVisible);
    void registerRemovedAliases(QStringList &aliases);
    void registerAlwaysVisibleChange(QString objectName, bool isVisible);

    void registerAddedWidget(Widget *item);
    void registerDeletedWidget(Widget *item);
    void registerMovedWidget(Widget *item, QPointF oldPos, QPointF newPos);
    void registerResizedTextBoxWidget(const int index, const double w_old, const double h_old, const double w_new, const double h_new, const QPointF oldPos, const QPointF newPos);
    void registerModifiedWidget(Widget *pItem);

private:
    SystemObject *mpParentSystemObject;
    int mCurrentStackPosition;
    bool mEnabled;

    void addTextboxwidget(const QDomElement &rStuffElement);
    void removeTextboxWidget(const QDomElement &rStuffElement);
    void modifyTextboxWidget(QDomElement &rStuffElement);

    void addImageWidget(const QDomElement &rStuffElement);
    void removeImageWidget(const QDomElement &rStuffElement);
    void modifyImageWidget(QDomElement &rStuffElement);

    QDomElement getCurrentPost();
    QDomDocument mDomDocument;
    QDomElement mUndoRoot;
};


#endif // UNDOSTACK_H
