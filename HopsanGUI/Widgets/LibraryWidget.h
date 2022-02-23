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
//! @file   LibraryWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-10-23
//!
//! @brief Contains classes for Library Widgets
//!
//$Id$

#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

//Qt includes
#include <QObject>
#include <QWidget>
#include <QSize>
#include <QTreeWidget>
#include <QListWidget>
#include <QToolButton>
#include <QLabel>

//Hopsan includes
#include "GUIObjects/GUIModelObjectAppearance.h"

//Forward declarations
class LibraryHandler;
class GUIComponentLibrary;

//! @brief Library widget class
class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    // Public Member functions
    LibraryWidget(QWidget *parent=0);
    QSize sizeHint() const;
    void setGfxType(GraphicsTypeEnumT gfxType);

    GraphicsTypeEnumT mGfxType;

public slots:
    void update();

private slots:
    void handleItemClick(QTreeWidgetItem* item, int column);
    void handleItemDoubleClick(QTreeWidgetItem* item, int column);

protected:
    void mouseMoveEvent(QMouseEvent *event);

private:
    void getAllSubTreeItems(QTreeWidgetItem *pParentItem, QList<QTreeWidgetItem *> &rSubItems);
    bool isComponentItem(QTreeWidgetItem *item);
    bool isExternalLibrariesItem(QTreeWidgetItem *item);
    bool isExternalLibraryItem(QTreeWidgetItem *item);
    bool isFmuLibrariesItem(QTreeWidgetItem *item);
    bool isExternalComponentItem(QTreeWidgetItem *item);
    bool hasSourceCode(QTreeWidgetItem *item);
    bool hasModelFile(QTreeWidgetItem *item);
    bool isFmuLibraryItem(QTreeWidgetItem *item);
    QTreeWidgetItem *getLibraryItem(QSharedPointer<GUIComponentLibrary> pLibrary);

    //GUI Stuff
    QTreeWidget *mpTree;
    QTreeWidgetItem *mpCreateExternalLibraryItem;
    QTreeWidgetItem *mpLoadLibraryItem;
    QLineEdit *mpFilterEdit;

    //Maps between GUI objects and library contents
    QMap<QTreeWidgetItem *, QString> mItemToTypeNameMap;        //Map between component items and typenames
    QMap<QTreeWidgetItem *, QSharedPointer<GUIComponentLibrary> > mItemToLibraryMap;    //Map between component items and libraries it might origin from
};

#endif // LIBRARYWIDGET_H
