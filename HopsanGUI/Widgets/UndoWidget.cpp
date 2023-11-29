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
//! @file   UndoWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains the undo widget class (which displays the stack)
//!
//$Id$

#include <QHeaderView>

#include "Widgets/UndoWidget.h"
#include "UndoStack.h"
#include "ModelWidget.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIContainerObject.h"
#include "common.h"
#include "global.h"
#include "ModelHandler.h"


//! @class UndoWidget
//! @brief The UndoWidget class is used to display a list of undo operations in the GUI.
//!
//! The undo widget is updated by calling the refreshList() function. It asks the undo stack in the current system for undo posts, translate their tags to readable
//! names and prints the list. It does not have a pointer to the undo stack because it depends on which system is open.
//!


//! @brief Constructor for undo list widget
//! @param parent Pointer to the parent main window
UndoWidget::UndoWidget(QWidget *parent)
    : QDialog(parent)
{
    //Set the name and size of the main window
    this->setObjectName("UndoWidget");
    this->resize(400,500);
    this->setWindowTitle("Undo History");

    mpRedoButton = new QPushButton(tr("&Redo"));
    mpRedoButton->setAutoDefault(false);
    mpRedoButton->setFixedHeight(30);
    QFont tempFont = mpRedoButton->font();
    tempFont.setBold(true);
    mpRedoButton->setFont(tempFont);

    mpUndoButton = new QPushButton(tr("&Undo"));
    mpUndoButton->setAutoDefault(false);
    mpUndoButton->setFixedHeight(30);
    mpUndoButton->setFont(tempFont);

    mpClearButton = new QPushButton(tr("&Clear"));
    mpClearButton->setAutoDefault(false);
    mpClearButton->setFixedHeight(30);
    mpClearButton->setFont(tempFont);

    mUndoTable = new QTableWidget(0,1);
    mUndoTable->setBaseSize(400, 500);
    mUndoTable->setColumnWidth(0, 400);
    mUndoTable->horizontalHeader()->setStretchLastSection(true);
    mUndoTable->horizontalHeader()->hide();

    mpLayout = new QGridLayout();
    mpLayout->setContentsMargins(4,4,4,4);
    mpLayout->addWidget(mUndoTable, 0, 0);
    mpLayout->addWidget(mpUndoButton, 1, 0);
    mpLayout->addWidget(mpRedoButton, 2, 0);
    mpLayout->addWidget(mpClearButton, 3, 0);
    setLayout(mpLayout);
}


//! @brief Reimplementation of show function. Updates the list every time before the widget is displayed.
void UndoWidget::show()
{
    refreshList();
    QDialog::show();
}


//! @brief Refresh function for the list. Reads from the current undo stack and displays the results in the table.
void UndoWidget::refreshList()
{
    if(gpModelHandler->count() == 0)
    {
        mpClearButton->setEnabled(false);
        mpUndoButton->setEnabled(false);
        mpRedoButton->setEnabled(false);
        return;
    }
    else
    {
        mpClearButton->setEnabled(true);
        mpUndoButton->setEnabled(true);
        mpRedoButton->setEnabled(true);
    }

    QTableWidgetItem *item;
    mUndoTable->clear();
    mUndoTable->setRowCount(0);


    //XML//

    QColor oddColor = QColor("white");
    QColor evenColor = QColor("whitesmoke");
    QColor activeColor = QColor("chartreuse");

    if(gpModelHandler->count() == 0 || !gpModelHandler->getCurrentViewContainerObject())
    {
        return;
    }

    int pos = 0;
    bool found = true;

    //qDebug() << "refreshList for Undo in: " << gpModelHandler->getCurrentContainer();
    //qDebug() << "refreshList for Undo in: " << gpModelHandler->getCurrentContainer()->getName();
    QDomElement undoRoot = gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mUndoRoot;
    QDomElement postElement = undoRoot.firstChildElement("post");
    while(found)
    {
        found = false;
        while(!postElement.isNull())
        {
            if(postElement.attribute("number").toInt() == pos)
            {
                //Found correct number, write it's contents to the list
                found = true;
                if(postElement.attribute("type") != QString())
                {
                    item = new QTableWidgetItem();
                    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    item->setText(translateTag(postElement.attribute("type")));
                    if(pos == gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                    {
                        item->setBackgroundColor(activeColor);
                    }
                    else if(pos%2 == 0)
                    {
                        item->setBackgroundColor(evenColor);
                    }
                    else
                    {
                        item->setBackgroundColor(oddColor);
                    }
                    if(pos > gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                    {
                        item->setForeground(QColor("gray"));
                    }
                    mUndoTable->insertRow(0);
                    mUndoTable->setItem(0,0,item);
                }
                else
                {
                    QDomElement stuffElement = postElement.firstChildElement("stuff");
                    while(!stuffElement.isNull())
                    {
                        item = new QTableWidgetItem();
                        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                        item->setText(translateTag(stuffElement.attribute("what")));
                        if(pos == gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                        {
                            item->setBackgroundColor(activeColor);
                        }
                        else if(pos%2 == 0)
                        {
                            item->setBackgroundColor(evenColor);
                        }
                        else
                        {
                            item->setBackgroundColor(oddColor);
                        }
                        if(pos > gpModelHandler->getCurrentViewContainerObject()->getUndoStackPtr()->mCurrentStackPosition)
                        {
                            item->setForeground(QColor("gray"));
                        }
                        mUndoTable->insertRow(0);
                        mUndoTable->setItem(0,0,item);
                        stuffElement = stuffElement.nextSiblingElement("stuff");
                    }
                }
                break;
            }
            postElement = postElement.nextSiblingElement("post");
        }
        ++pos;
    }
    //qDebug() << gpModelHandler->getCurrentContainer()->mUndoStack->mDomDocument.toString();
}


//! @brief Help function that translates undo tags into more readable strings
//! @param tag Tag name to translate
QString UndoWidget::translateTag(QString tag)
{
    QMap<QString, QString> tagMap;
    tagMap.insert(UNDO_ADDEDOBJECT,             "Added Object");
    tagMap.insert(UNDO_ADDEDCONNECTOR,          "Added Connector");
    tagMap.insert(UNDO_DELETEDOBJECT,           "Deleted Object");
    tagMap.insert(UNDO_DELETEDCONNECTOR,        "Deleted Connector");
    tagMap.insert(UNDO_MOVEDOBJECT,             "Moved Object");
    tagMap.insert(UNDO_RENAME,                  "Renamed Object");
    tagMap.insert(UNDO_MODIFIEDCONNECTOR,       "Modified Connector");
    tagMap.insert(UNDO_ROTATE,                  "Rotated Object");
    tagMap.insert(UNDO_VERTICALFLIP,            "Flipped Vertical");
    tagMap.insert(UNDO_HORIZONTALFLIP,          "Flipped Horizontal");
    tagMap.insert(UNDO_NAMEVISIBILITYCHANGE,    "Changed Name Visibility");
    tagMap.insert(UNDO_ALWAYSVISIBLECHANGE,     "Toggle Component Always Visible");
    tagMap.insert(UNDO_PASTE,                   "Paste");
    tagMap.insert(UNDO_MOVEDMULTIPLE,           "Moved Objects");
    tagMap.insert(UNDO_CUT,                     "Cut");
    tagMap.insert(UNDO_CHANGEDPARAMETERS,       "Changed Parameter(s)");
    tagMap.insert(UNDO_HIDEALLNAMES,            "Hide All Name Text");
    tagMap.insert(UNDO_SHOWALLNAMES,            "Show All Name Text");
    tagMap.insert(UNDO_MOVEDWIDGET,             "Moved Widget");
    tagMap.insert(UNDO_MOVEDMULTIPLEWIDGETS,    "Moved Widgets");
    tagMap.insert(UNDO_ALIGNX,                  "Align Vertical");
    tagMap.insert(UNDO_ALIGNY,                  "Align Horizontal");
    tagMap.insert(UNDO_DISTRIBUTEX,             "Distribute Horizontally");
    tagMap.insert(UNDO_DISTRIBUTEY,             "Distribute Vertically");
    tagMap.insert(UNDO_DELETEDSYSTEMPORT,       "Deleted System Port");
    tagMap.insert(UNDO_DELETEDSUBSYSTEM,        "Deleted Subsystem");
    tagMap.insert(UNDO_ADDEDSYSTEMPORT,         "Added System Port");
    tagMap.insert(UNDO_ADDEDSUBSYSTEM,          "Added Subsystem");
    tagMap.insert(UNDO_MOVEDCONNECTOR,          "Moved Connector");
    tagMap.insert(UNDO_CHANGEDPARAMETER,        "Changed Parameter");
    tagMap.insert(UNDO_ADDEDTEXTBOXWIDGET,      "Added Text Box Widget");
    tagMap.insert(UNDO_ADDEDIMAGEWIDGET,        "Added Image Widget");
    tagMap.insert(UNDO_DELETEDIMAGEWIDGET,      "Deleted Image Widget");
    tagMap.insert(UNDO_MODIFIEDIMAGEWIDGET,     "Modified Image Widget");
    tagMap.insert(UNDO_DELETEDTEXTBOXWIDGET,    "Deleted Text Box Widget");
    tagMap.insert(UNDO_RESIZEDTEXTBOXWIDGET,    "Resized Text Box Widget");
    tagMap.insert(UNDO_MODIFIEDTEXTBOXWIDGET,   "Modified Text Box Widget");

    if(tagMap.contains(tag))
        return tagMap.find(tag).value();
    else
        return "Undefined Action";
}


//! @brief Returns a pointer to the undo button
QPushButton *UndoWidget::getUndoButton()
{
    return mpUndoButton;
}


//! @brief Returns a pointer to the redo button
QPushButton *UndoWidget::getRedoButton()
{
    return mpRedoButton;
}


//! @brief Returns a pointer to the clear button
QPushButton *UndoWidget::getClearButton()
{
    return mpClearButton;
}
