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
#include "MainWindow.h"
#include "CoreAccess.h"
#include "Configuration.h"
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
    mpTextEdit->setFont(QFont(this->font().family(), 8));

    mGroupByTag = gConfig.getGroupMessagesByTag();

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
    mpGroupByTagCheckBox->setChecked(gConfig.getGroupMessagesByTag());

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


//! @brief Loads message widget settings from configuration object
//! This is needed because messages widget must be created before configuration
//! (so that config can print messages), which means that message widgets cannot
//! load the config directly in the constructor.
void MessageWidget::loadConfig()
{
    mpGroupByTagCheckBox->setChecked(gConfig.getGroupMessagesByTag());
}


bool MessageWidget::textEditHasFocus()
{
    return mpTextEdit->hasFocus();
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


void MessageWidget::updateNewMessagesOnly()
{
        //Loop through message list and print messages
    for(int msg=0; msg<mNewMessageList.size(); ++msg)
    {
        appendOneMessage(mNewMessageList.at(msg));
    }

    mPrintedMessageList.append(mNewMessageList);
    mNewMessageList.clear();
}


//! @brief Updates the displayed messages from the message list
void MessageWidget::updateEverything()
{
    mpTextEdit->clear();         //Clear the message box (we can not call this->clear(), since this would also clear the message list and we wouldn't have anything to print)
    mPrintedMessageList.append(mNewMessageList);
    mNewMessageList.clear();
    mLastTag = QString();
    mSubsequentTags = 1;

        //Loop through message list and print messages
    for(int msg=0; msg<mPrintedMessageList.size(); ++msg)
    {
        appendOneMessage(mPrintedMessageList.at(msg));
    }
}


void MessageWidget::appendOneMessage(GUIMessage msg)
{
    if( !(msg.type == "error" && !mShowErrorMessages) &&          //Do not show message if its type shall not be shown
        !(msg.type == "warning" && !mShowWarningMessages) &&
        !(msg.type == "info" && !mShowInfoMessages) &&
        !(msg.type == "debug" && !mShowDebugMessages))
    {
        if(!msg.tag.isEmpty() && msg.tag == mLastTag && mGroupByTag)     //Message is tagged, and group by tag setting is active
        {
            ++mSubsequentTags;
            QString numString;
            numString.setNum(mSubsequentTags);
            mpTextEdit->undo();
            setMessageColor(msg.type);
            mpTextEdit->append("[" + msg.time + "] " + msg.message + "    (" + numString + " similar)");
        }
        else        //Message is not tagged, or group by tag setting is not active
        {
            mSubsequentTags = 1;
            setMessageColor(msg.type);
            mpTextEdit->append("[" + msg.time + "] " + msg.message);
            mLastTag =msg.tag;
        }
    }
}


//! @brief Obtains messages from core and prints them in the message widget
void MessageWidget::printCoreMessages()
{
    int nmsg = mpCoreAccess->getNumberOfMessages();
    //nmsg = 0; //!< @warning Fix for Petter should not be checked into the repository

    bool playErrorSound = false;
    for (int idx=0; idx < nmsg; ++idx)
    {
        QString message, type, tag;
        mpCoreAccess->getMessage(message, type, tag);
        if(type == "error")
            playErrorSound = true;
        mNewMessageList.append(GUIMessage(message, type, tag));
        updateNewMessagesOnly();
    }
    if(playErrorSound)
    {
        QSound::play(QString(SOUNDSPATH) + "error.wav");
    }
}


//! @brief Prints a GUI error message
//! @param message String containing the message
void MessageWidget::printGUIErrorMessage(QString message, QString tag)
{
    QSound::play(QString(SOUNDSPATH) + "error.wav");
    mNewMessageList.append(GUIMessage(message.prepend("Error: "), "error", tag));
    updateNewMessagesOnly();
}


//! @brief Prints a GUI warning message
//! @param message String containing the message
void MessageWidget::printGUIWarningMessage(QString message, QString tag)
{
    mNewMessageList.append(GUIMessage(message.prepend("Warning: "), "warning", tag));
    updateNewMessagesOnly();
}


//! @brief Prints a GUI info message
//! @param message String containing the message
void MessageWidget::printGUIInfoMessage(QString message, QString tag)
{
    mNewMessageList.append(GUIMessage(message.prepend("Info: "), "info", tag));
    updateNewMessagesOnly();
}


//! @brief Prints a GUI info message
//! @param message String containing the message
void MessageWidget::printGUIDebugMessage(QString message, QString tag)
{
    mNewMessageList.append(GUIMessage(message.prepend("Debug: "), "debug", tag));
    updateNewMessagesOnly();
}


//! @brief Clear function for message widget, this will empty the message widget and also remove all messages from the list
void MessageWidget::clear()
{
    mpTextEdit->clear();
    mPrintedMessageList.clear();
    mNewMessageList.clear();
    updateEverything();
}


//! @brief Slot that checks messages from core and prints them
//! @todo Is this function necessary? All it does is calling another one...
void MessageWidget::checkMessages()
{
    printCoreMessages();
}


//! @brief Tells the message widget wether or not messages shall be grouped by tags
//! @param value True means that messages shall be grouped
void MessageWidget::setGroupByTag(bool value)
{
    mGroupByTag = value;
    gConfig.setGroupMessagesByTag(value);
    updateEverything();
}


//! @brief Tells the message widget wether or not it shall show error messages
//! @param value True means show messages
void MessageWidget::showErrorMessages(bool value)
{
    mShowErrorMessages = value;
    updateEverything();
}


//! @brief Tells the message widget wether or not it shall show warning messages
//! @param value True means show messages
void MessageWidget::showWarningMessages(bool value)
{
    mShowWarningMessages = value;
    updateEverything();
}


//! @brief Tells the message widget wether or not it shall show info messages
//! @param value True means show messages
void MessageWidget::showInfoMessages(bool value)
{
    mShowInfoMessages = value;
    updateEverything();
}


//! @brief Tells the message widget wether or not it shall show debug messages
//! @param value True means show messages
void MessageWidget::showDebugMessages(bool value)
{
    mShowDebugMessages = value;
    updateEverything();
}


void MessageWidget::copy()
{
    if(mpTextEdit->hasFocus())
    {
        mpTextEdit->copy();
    }
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

