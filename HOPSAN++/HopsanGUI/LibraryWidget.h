/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   LibraryWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Library Widgets
//!
//$Id$

#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <string>
#include <map>
#include <QListWidget>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>
#include "AppearanceData.h"


class LibraryContentItem;

#include <QListWidgetItem>
#include <QStringList>


//Här borde en funktion finnas som växlar mellan iso-ikon och user-ikon
class LibraryContentItem : public QListWidgetItem
{
public:
    LibraryContentItem(AppearanceData *pAppearanceData, QListWidget *parent = 0);
    LibraryContentItem(const QListWidgetItem &other);
    AppearanceData *getAppearanceData();

//public slots:
    void selectIcon(bool useIso=false);

private:
    AppearanceData *mpAppearanceData;

};

//Forward declaration
class LibraryWidget;

class LibraryContent : public QListWidget
{
    Q_OBJECT

public:
    LibraryContent(LibraryContent *pParentLibraryContent, LibraryWidget *pParentLibraryWidget);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void itemEntered(QListWidgetItem *item);

    QPoint dragStartPosition;
    LibraryWidget *mpParentLibraryWidget;
};


class MainWindow;

class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    //Member functions
    LibraryWidget(MainWindow *parent = 0);
    void addEmptyLibrary(QString libraryName, QString parentLibraryName=QString());
    void addLibrary(QString libDir, QString parentLib=QString());
    void addLibraryContentItem(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent);
    AppearanceData *getAppearanceData(QString componentType);
    AppearanceData *getAppearanceDataByDisplayName(QString displayName);


public slots:
    void addLibrary();
    void useIsoGraphics(bool useISO);

private slots:
    void showLib(QTreeWidgetItem * item, int column);
    void hideAllLib();

private:
    //Member variables
    MainWindow *mpParentMainWindow;
    QMap<QString, AppearanceData*> mAppearanceDataMap;
    QMap<QString, QString> mName2TypeMap; //!< @todo This is a temporary hack
    QTreeWidget *mpTree;
    QMap<QString, LibraryContent*> mLibraryContentMapPtrs;
    QVBoxLayout *mpGrid;
    QVector<LibraryContentItem*> mpContentItems;

};

#endif // LIBRARYWIDGET_H
