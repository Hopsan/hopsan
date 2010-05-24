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
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Based on: HopsanGUI
 */

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
friend class UndoWidget;

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
    void registerVerticalFlip(GUIObject *item);
    void registerHorizontalFlip(GUIObject *item);
    void clear();
    void newPost();
    void insertPost(QStringList(list));
    //void newRedoPost();
    void undoOneStep();
    void redoOneStep();

private:
    GraphicsView *mpParentView;
    QList< QList<QStringList> > mStack;
    int mCurrentStackPosition;
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
    QPushButton *clearButton;

    void show();
    void refreshList();
};


#endif // UNDOSTACK_H
