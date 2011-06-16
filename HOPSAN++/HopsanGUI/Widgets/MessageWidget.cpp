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
//! @file   MessageWidget.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-xx
//! @version $Id$
//!
//! @brief Contains the MessageWidget that dissplays messages to the user
//!

#include "MessageWidget.h"
#include "../MainWindow.h"
#include "../CoreAccess.h"
#include "../Configuration.h"
#include <QDateTime>

using namespace hopsan;

//! @class MessageWidget
//! @brief The class for the message widget at the bottom of the main window
//!
//! Messages are either gathered from core or added directly from GUI. They are added to a list and then shown by calling the updateDisplay() function.
//!

//! @brief Constructor for the message widget class
//! @param pParent Parent pointer (not necessary)
MessageWidget::MessageWidget(MainWindow *pParent)
    : QWidget(pParent)
{


    mpTextEdit = new QTextEdit(this);
    mpTextEdit->setReadOnly(true);
    //mpTextEdit->setPalette(QPalette(QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("white"), QColor("whitesmoke")));
    //mpTextEdit->setTextBackgroundColor(QColor("gray"));

    mpTextEdit->setFont(QFont(this->font().family(), 8));

    mGroupByTag = false;

    mShowErrorMessages = true;
    mShowWarningMessages = true;
    mShowInfoMessages = true;
    mShowDebugMessages = false;

    mpCoreAccess = new CoreMessagesAccess;

    mpClearMessageWidgetButton = new QPushButton("Clear Messages");
    QFont tempFont = mpClearMessageWidgetButton->font();
    tempFont.setBold(true);
    mpClearMessageWidgetButton->setFont(tempFont);
    mpClearMessageWidgetButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    mpShowErrorMessagesButton = new QToolButton();
    mpShowErrorMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowErrorMessages.png"));
    mpShowErrorMessagesButton->setCheckable(true);
    mpShowErrorMessagesButton->setChecked(true);
    mpShowErrorMessagesButton->setToolTip("Show Error Messages");

    mpShowWarningMessagesButton = new QToolButton();
    mpShowWarningMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowWarningMessages.png"));
    mpShowWarningMessagesButton->setCheckable(true);
    mpShowWarningMessagesButton->setChecked(true);
    mpShowWarningMessagesButton->setToolTip("Show Warning Messages");

    mpShowInfoMessagesButton = new QToolButton();
    mpShowInfoMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowInfoMessages.png"));
    mpShowInfoMessagesButton->setCheckable(true);
    mpShowInfoMessagesButton->setChecked(true);
    mpShowInfoMessagesButton->setToolTip("Show Info Messages");

    mpShowDebugMessagesButton = new QToolButton();
    mpShowDebugMessagesButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ShowDebugMessages.png"));
    mpShowDebugMessagesButton->setCheckable(true);
    mpShowDebugMessagesButton->setChecked(false);
    mpShowDebugMessagesButton->setToolTip("Show Debug Messages");

    mpGroupByTagCheckBox = new QCheckBox("Group Similar Messages");
    mpGroupByTagCheckBox->setChecked(false);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpTextEdit,0,0,1,7);
    mpLayout->addWidget(mpClearMessageWidgetButton,1,0,1,1);
    mpLayout->addWidget(mpShowErrorMessagesButton,1,1,1,1);
    mpLayout->addWidget(mpShowWarningMessagesButton,1,2,1,1);
    mpLayout->addWidget(mpShowInfoMessagesButton,1,3,1,1);
    mpLayout->addWidget(mpShowDebugMessagesButton,1,4,1,1);
    mpLayout->addWidget(mpGroupByTagCheckBox, 1,5,1,1);

    mpLayout->setContentsMargins(4,4,4,4);

    this->setLayout(mpLayout);
    connect(mpClearMessageWidgetButton, SIGNAL(clicked()),this, SLOT(clear()));
    connect(mpShowErrorMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showErrorMessages(bool)));
    connect(mpShowWarningMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showWarningMessages(bool)));
    connect(mpShowInfoMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showInfoMessages(bool)));
    connect(mpShowDebugMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showDebugMessages(bool)));
    connect(mpGroupByTagCheckBox, SIGNAL(toggled(bool)), this, SLOT(setGroupByTag(bool)));

}


//! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the message widget when docked
QSize MessageWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 1; //pixels
    return size;
}


//! @brief Sets the color for the next added message, depending on type of message
//! @param type Type name of the message (error, warning, info etc...)
void MessageWidget::setMessageColor(QString type)
{
    if (type == "error")
    {
        mpTextEdit->setTextColor("RED");
    }
    else if (type == "warning")
    {
        mpTextEdit->setTextColor(QColor(216, 115, 0));
    }
    else if (type == "info")
    {
        mpTextEdit->setTextColor("BLACK");
    }
    else if (type == "debug")
    {
        mpTextEdit->setTextColor("BLUE");
    }
    else
    {
        mpTextEdit->setTextColor("GRAY");
    }
}


//! @brief Updates the displayed messages from the message list
void MessageWidget::updateDisplay()
{
    mpTextEdit->clear();         //Clear the message box (we can not call this->clear(), since this would also clear the message list and we wouldn't have anything to print)
    //QStringList usedTags;

        //Loop through message list and print messages
    for(int msg=0; msg<mMessageList.size(); ++msg)
    {
        if( !(mMessageList.at(msg).type == "error" && !mShowErrorMessages) &&          //Do not show message if its type shall not be shown
            !(mMessageList.at(msg).type == "warning" && !mShowWarningMessages) &&
            !(mMessageList.at(msg).type == "info" && !mShowInfoMessages) &&
            !(mMessageList.at(msg).type == "debug" && !mShowDebugMessages))
        {
            setMessageColor(mMessageList.at(msg).type);
            if(!mMessageList.at(msg).tag.isEmpty() && mGroupByTag)     //Message is tagged, and group by tag setting is active
            {
                size_t nTags = subsequentTagCount(mMessageList.at(msg).tag, msg);
                if(nTags == 1)                      //There is only one tag, so avoid appending "(0 similar)"
                {
                    mpTextEdit->append("[" + mMessageList.at(msg).time + "] " + mMessageList.at(msg).message);
                }
                else                                //There are more than one tag, so append ("X similar)"
                {
                    QString numString;
                    numString.setNum(nTags-1);
                    mpTextEdit->append("[" + mMessageList.at(msg).time + "] " + mMessageList.at(msg).message + "    (" + numString + " similar)");
                }
                msg += nTags-1;
            }
            else        //Message is not tagged, or group by tag setting is not active
            {
                mpTextEdit->append("[" + mMessageList.at(msg).time + "] " + mMessageList.at(msg).message);
            }
        }
    }
}


//! @brief Counts number of subsequent tags of the same name from a certain position in the message list
//! @param tag Tag name to look for
//! @param startIdx Position in list to start looking
size_t MessageWidget::subsequentTagCount(QString tag, size_t startIdx)
{
    size_t nTags = 0;
    for(int i=startIdx; i<mMessageList.size(); ++i)
    {
        if(mMessageList.at(i).tag == tag)
        {
            ++nTags;
        }
        else
        {
            return nTags;
        }
    }
    return nTags;
}


//! @brief Help function that counts how many messages with a specified tag that exists in message list
//! @param tag Name of the tag that shall be counted
//! @todo This is no longer used and can safely be removed
size_t MessageWidget::tagCount(QString tag)
{
    size_t nTags = 0;
    QList<GUIMessage>::iterator it;
    for(it=mMessageList.begin(); it!=mMessageList.end(); ++it)
    {
        if((*it).tag == tag)
        {
            ++nTags;
        }
    }
    return nTags;
}


//! @brief Obtains messages from core and prints them in the message widget
void MessageWidget::printCoreMessages()
{
    int nmsg = mpCoreAccess->getNumberOfMessages();

    for (int idx=0; idx < nmsg; ++idx)
    {
        QString message, type, tag;
        mpCoreAccess->getMessage(message, type, tag);
        mMessageList.append(GUIMessage(message, type, tag));
        updateDisplay();
    }
}


//! @brief Prints a GUI error message
//! @param message String containing the message
void MessageWidget::printGUIErrorMessage(QString message, QString tag)
{
    mMessageList.append(GUIMessage(message.prepend("Error: "), "error", tag));
    updateDisplay();
}


//! @brief Prints a GUI warning message
//! @param message String containing the message
void MessageWidget::printGUIWarningMessage(QString message, QString tag)
{
    mMessageList.append(GUIMessage(message.prepend("Warning: "), "warning", tag));
    updateDisplay();
}


//! @brief Prints a GUI info message
//! @param message String containing the message
void MessageWidget::printGUIInfoMessage(QString message, QString tag)
{
    mMessageList.append(GUIMessage(message.prepend("Info: "), "info", tag));
    updateDisplay();
}


//! @brief Prints a GUI info message
//! @param message String containing the message
void MessageWidget::printGUIDebugMessage(QString message, QString tag)
{
    mMessageList.append(GUIMessage(message.prepend("Debug: "), "debug", tag));
    updateDisplay();
}


//! @brief Clear function for message widget, this will empty the message widget and also remove all messages from the list
void MessageWidget::clear()
{
    mpTextEdit->clear();
    mMessageList.clear();
}


//! @brief Slot that checks messages from core and prints them
void MessageWidget::checkMessages()
{
    printCoreMessages();
}


//! @brief Tells the message widget wether or not messages shall be grouped by tags
//! @param value True means that messages shall be grouped
void MessageWidget::setGroupByTag(bool value)
{
    mGroupByTag = value;
    updateDisplay();
}


//! @brief Tells the message widget wether or not it shall show error messages
//! @param value True means show messages
void MessageWidget::showErrorMessages(bool value)
{
    mShowErrorMessages = value;
    updateDisplay();
}


//! @brief Tells the message widget wether or not it shall show warning messages
//! @param value True means show messages
void MessageWidget::showWarningMessages(bool value)
{
    mShowWarningMessages = value;
    updateDisplay();
}


//! @brief Tells the message widget wether or not it shall show info messages
//! @param value True means show messages
void MessageWidget::showInfoMessages(bool value)
{
    mShowInfoMessages = value;
    updateDisplay();
}


//! @brief Tells the message widget wether or not it shall show debug messages
//! @param value True means show messages
void MessageWidget::showDebugMessages(bool value)
{
    mShowDebugMessages = value;
    updateDisplay();
}



//! @class GUIMessage
//! @brief The GUIMessage class represent a message in the message widget
//!
//! There are three public strings; message, type and tag. These can be accessed directly.
//!

//! @brief Constructor for the GUIMessage class
GUIMessage::GUIMessage(QString message, QString type, QString tag)
{    
    QTime time = QTime::currentTime();

    this->message = message;
    this->type = type;
    this->tag = tag;
    this->time = time.toString();
}

