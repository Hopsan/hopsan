//!
//! @file   UndoStack.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
//$Id$

#include <QtGui>

#include "../MainWindow.h"
#include "../UndoStack.h"
#include "../ProjectTabWidget.h"
#include "../Utilities/GUIUtilities.h"
#include "../GUIObjects/GUIContainerObject.h"
#include "../GUIObjects/GUISystem.h"

#include "../common.h"


//! @class UndoWidget
//! @brief The UndoWidget class is used to display a list of undo operations in the GUI.
//!
//! The undo widget is updated by calling the refreshList() function. It asks the undo stack in the current system for undo posts, translate their tags to readable
//! names and prints the list. It does not have a pointer to the undo stack because it depends on which system is open.
//!


//! @brief Construtor for undo list widget
//! @param parent Pointer to the parent main window
UndoWidget::UndoWidget(MainWindow *parent)
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
    if(gpMainWindow->mpProjectTabs->count() == 0)
    {
        return;
    }

    QTableWidgetItem *item;
    mUndoTable->clear();
    mUndoTable->setRowCount(0);


    //XML//

    QColor oddColor = QColor("white");
    QColor evenColor = QColor("whitesmoke");
    QColor activeColor = QColor("chartreuse");

    size_t pos = 0;
    bool found = true;

    QDomElement undoRoot = gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mUndoRoot;
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
                    if(pos == gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
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
                        if(pos == gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mCurrentStackPosition)
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
    //qDebug() << gpMainWindow->mpProjectTabs->getCurrentSystem()->mUndoStack->mDomDocument.toString();
}


//! @brief Help function that translates undo tags into more readable strings
//! @param tag Tag name to translate
QString UndoWidget::translateTag(QString tag)
{
    QMap<QString, QString> tagMap;
    tagMap.insert("addedobject",            "Added Object");
    tagMap.insert("addedconnector",         "Added Connector");
    tagMap.insert("deletedobject",          "Deleted Object");
    tagMap.insert("deletedconnector",       "Deleted Connector");
    tagMap.insert("movedobject",            "Moved Object");
    tagMap.insert("rename",                 "Renamed Object");
    tagMap.insert("modifiedconnector",      "Modified Connector");
    tagMap.insert("rotate",                 "Rotated Object");
    tagMap.insert("flipvertical",           "Flipped Vertical");
    tagMap.insert("fliphorizontal",         "Flipped Horizontal");
    tagMap.insert("namevisibilitychange",   "Changed Name Visibility");
    tagMap.insert("paste",                  "Paste");
    tagMap.insert("movedmultiple",          "Moved Objects");
    tagMap.insert("cut",                    "Cut");
    tagMap.insert("changedparameters",      "Changed Parameter(s)");
    tagMap.insert("hideallnames",           "Hide All Name Text");
    tagMap.insert("showallnames",           "Show All Name Text");

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
