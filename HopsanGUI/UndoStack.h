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
namespace undo {
    constexpr auto what = "what";
    constexpr auto stuff = "stuff";
    constexpr auto addedobject = "addedobject";
    constexpr auto addedconnector = "addedconnector";
    constexpr auto deletedobject = "deletedobject";
    constexpr auto deletedconnector = "deletedconnector";
    constexpr auto movedobject = "movedobject";
    constexpr auto rename = "rename";
    constexpr auto modifiedconnector = "modifiedconnector";
    constexpr auto rotate = "rotate";
    constexpr auto verticalflip = "verticalflip";
    constexpr auto horizontalflip = "horizontalflip";
    constexpr auto namevisibilitychange = "namevisibilitychange";
    constexpr auto alwaysvisiblechange = "alwaysvisiblechange";
    constexpr auto removedaliases = "removedaliases";
    constexpr auto paste = "paste";
    constexpr auto movedmultiple = "movedmultiple";
    constexpr auto cut = "cut";
    constexpr auto changedparameters = "changedparameters";
    constexpr auto hideallnames = "hideallnames";
    constexpr auto showallnames = "showallnames";
    constexpr auto movedwidget = "movedwidget";
    constexpr auto movedmultiplewidgets = "movedmultiplewidgets";
    constexpr auto alignx = "alignx";
    constexpr auto aligny = "aligny";
    constexpr auto distributex = "distributex";
    constexpr auto distributey = "distributey";
    constexpr auto deletedsystemport = "deletedsystemport";
    constexpr auto deletedsubsystem = "deletedsubsystem";
    constexpr auto addedsystemport = "addedsystemport";
    constexpr auto addedsubsystem = "addedsubsystem";
    constexpr auto movedconnector = "movedconnector";
    constexpr auto changedparameter = "changedparameter";
    constexpr auto addedtextboxwidget = "addedtextboxwidget";
    constexpr auto addedimagewidget = "addedimagewidget";
    constexpr auto deletedimagewidget = "deletedimagewidget";
    constexpr auto modifiedimagewidget = "modifiedimagewidget";
    constexpr auto deletedtextboxwidget = "deletedtextboxwidget";
    constexpr auto resizedtextboxwidget = "resizedtextboxwidget";
    constexpr auto modifiedtextboxwidget = "modifiedtextboxwidget";
    constexpr auto simulationtimechanged = "simulationtimechanged";
}

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

    void registerSimulationTimeChanged(QString oldStartTime, QString oldTimeStep, QString oldStopTime, QString startTime, QString timeStep, QString stopTime);

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
