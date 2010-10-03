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

#include <QtGui>

#include "GlobalParametersWidget.h"

#include <QDialog.h>

#include <MainWindow.h>

//! Construtor.
GlobalParametersWidget::GlobalParametersWidget(MainWindow *parent)
    : QDialog(parent)
{
    mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("UndoWidget");
    this->resize(400,500);
    this->setWindowTitle("Undo History");

    addButton = new QPushButton(tr("&Add"));
    addButton->setAutoDefault(true);

    removeButton = new QPushButton(tr("&Remove"));
    removeButton->setAutoDefault(true);

    mGlobalParametersTable = new QTableWidget(0,1);
    mGlobalParametersTable->setBaseSize(400, 500);
    mGlobalParametersTable->setColumnWidth(0, 200);
    mGlobalParametersTable->setColumnCount(2);
    mGlobalParametersTable->setRowCount(3);
    mGlobalParametersTable->horizontalHeader()->setStretchLastSection(true);
    mGlobalParametersTable->horizontalHeader()->hide();

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(mGlobalParametersTable, 0, 0, 1, 2);
    mainLayout->addWidget(addButton, 1, 0);
    mainLayout->addWidget(removeButton, 1, 1);
    setLayout(mainLayout);
}


////! Reimplementation of show function, which updates the list every time before the widget is displayed.
//void UndoWidget::show()
//{
//    refreshList();
//    QDialog::show();
//}


////! Refresh function for the list. Reads from the current undo stack and displays the results in a table.
//void UndoWidget::refreshList()
//{
//    if(mpParentMainWindow->mpProjectTabs->count() == 0)
//    {
//        return;
//    }
//    mTempStack = mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mStack;
//    QTableWidgetItem *item;

//    mUndoTable->clear();
//    mUndoTable->setRowCount(0);

//    int x = 0;

//    if(mTempStack.empty())
//    {
//        item = new QTableWidgetItem();
//        //! @todo what the heck is this suposed to mean !ENUM, item->setFlags(!Qt::ItemIsEditable);
//        item->setText("No undo history found.");
//        item->setBackgroundColor(QColor("white"));
//        mUndoTable->insertRow(x);
//        mUndoTable->setItem(x,0,item);
//        ++x;
//        mUndoTable->verticalHeader()->hide();
//        item->setTextAlignment(Qt::AlignCenter);
//    }
//    else if(mTempStack[0].empty())
//    {
//        item = new QTableWidgetItem();
//        //! @todo what the heck is this suposed to mean !ENUM, item->setFlags(!Qt::ItemIsEditable);
//        item->setText("No undo history found.");
//        item->setBackgroundColor(QColor("white"));
//        mUndoTable->insertRow(x);
//        mUndoTable->setItem(x,0,item);
//        ++x;
//        mUndoTable->verticalHeader()->hide();
//        item->setTextAlignment(Qt::AlignCenter);
//    }
//    else
//    {
//        mUndoTable->verticalHeader()->show();
//    }

//    for(int i = mTempStack.size()-1; i != -1; --i)
//    {
//        for(int j = mTempStack[i].size()-1; j != -1; --j)
//        {
//            item = new QTableWidgetItem();
//            //! @todo what the heck is this suposed to mean !ENUM, item->setFlags(!Qt::ItemIsEditable);

//                // Translate the command words into better looking explanations
//            QTextStream stream(&mTempStack[i][j]);
//            QString commandword;
//            stream >> commandword;

//            if (commandword == "DELETEDOBJECT")
//            {
//                item->setText("Deleted Object");
//            }
//            else if (commandword == "DELETEDCONNECTOR")
//            {
//                item->setText("Deleted Connector");
//            }
//            else if (commandword == "ADDEDOBJECT")
//            {
//                item->setText("Added Object");
//            }
//            else if (commandword == "ADDEDCONNECTOR")
//            {
//                item->setText("Added Connector");
//            }
//            else if (commandword == "RENAMEDOBJECT")
//            {
//                item->setText("Renamed Object");
//            }
//            else if (commandword == "MODIFIEDCONNECTOR")
//            {
//                item->setText("Modified Connector");
//            }
//            else if (commandword == "MOVEDOBJECT")
//            {
//                item->setText("Moved Object");
//            }
//            else if (commandword == "ROTATEDOBJECT")
//            {
//                item->setText("Rotated Object");
//            }
//            else if (commandword == "VERTICALFLIP")
//            {
//                item->setText("Flipped Vertical");
//            }
//            else if (commandword == "HORIZONTALFLIP")
//            {
//                item->setText("Flipped Horizontal");
//            }
//            else
//            {
//                item->setText(commandword);
//            }

//                // Figure out which color to use.
//            if(i > mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
//            {
//                if (i%2 == 0)
//                {
//                    item->setBackgroundColor(QColor("white"));
//                }
//                else
//                {
//                    item->setBackgroundColor(QColor("antiquewhite"));
//                }
//            }
//            else if(i < mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
//            {
//                if (i%2 == 0)
//                {
//                    item->setBackgroundColor(QColor("white"));
//                    item->setTextColor(QColor("black"));
//                }
//                else
//                {
//                    item->setBackgroundColor(QColor("antiquewhite"));
//                    item->setTextColor(QColor("black"));
//                }
//            }
//            else
//            {
//                item->setBackgroundColor(QColor("lightgreen"));
//                item->setTextColor(QColor("black"));
//                QFont tempFont = item->font();
//                tempFont.setBold(true);
//                item->setFont(tempFont);
//            }

//                //Insert a new line in the table and display the action
//            if(commandword != "PARAMETER")                              //Don't show parameter lines, because they "belong" to the object lines.
//            {
//                mUndoTable->insertRow(x);
//                mUndoTable->setItem(x,0,item);
//                ++x;
//            }
//        }
//    }
//}
