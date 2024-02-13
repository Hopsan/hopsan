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
//! @file   MessageWidget.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-xx
//! @version $Id$
//!
//! @brief Contains the MessageWidget that displays messages to the user
//!

#include "global.h"
#include "MessageWidget.h"
#include "Configuration.h"

#include <QDateTime>
#include <QTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QScrollBar>
#include <QEvent>

//! @class MessageWidget
//! @brief The class for the message widget at the bottom of the main window
//!
//! Messages are either gathered from core or added directly from GUI. They are added to a list and then shown by calling the updateDisplay() function.
//!

//! @brief Constructor for the message widget class
//! @param pParent Parent pointer (not necessary)
MessageWidget::MessageWidget(QWidget *pParent)
    : QWidget(pParent)
{
    this->setMouseTracking(true);

    mpTextEdit = new QTextEdit(this);
    mpTextEdit->setReadOnly(true);
    mpTextEdit->setFont(QFont(this->font().family(), 8));
    mpTextEdit->setMouseTracking(true);
    mpTextEdit->installEventFilter(this);

    mGroupByTag = gpConfig->getBoolSetting(cfg::groupmessagesbytag);

    mShowErrorMessages = true;
    mShowWarningMessages = true;
    mShowInfoMessages = true;
    mShowDebugMessages = false;

    QPushButton *pClearMessageWidgetButton = new QPushButton("Clear Messages");
    QFont tempFont = pClearMessageWidgetButton->font();
    tempFont.setBold(true);
    pClearMessageWidgetButton->setFont(tempFont);
    pClearMessageWidgetButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QToolButton *pShowErrorMessagesButton = new QToolButton();
    pShowErrorMessagesButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowErrorMessages.svg"));
    pShowErrorMessagesButton->setCheckable(true);
    pShowErrorMessagesButton->setChecked(true);
    pShowErrorMessagesButton->setToolTip("Show Error Messages");

    QToolButton *pShowWarningMessagesButton = new QToolButton();
    pShowWarningMessagesButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowWarningMessages.svg"));
    pShowWarningMessagesButton->setCheckable(true);
    pShowWarningMessagesButton->setChecked(true);
    pShowWarningMessagesButton->setToolTip("Show Warning Messages");

    QToolButton *pShowInfoMessagesButton = new QToolButton();
    pShowInfoMessagesButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowInfoMessages.svg"));
    pShowInfoMessagesButton->setCheckable(true);
    pShowInfoMessagesButton->setChecked(true);
    pShowInfoMessagesButton->setToolTip("Show Info Messages");

    QToolButton *pShowDebugMessagesButton = new QToolButton();
    pShowDebugMessagesButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowDebugMessages.svg"));
    pShowDebugMessagesButton->setCheckable(true);
    pShowDebugMessagesButton->setChecked(false);
    pShowDebugMessagesButton->setToolTip("Show Debug Messages");

    mpGroupByTagCheckBox = new QCheckBox("Group Similar Messages");
    mpGroupByTagCheckBox->setChecked(gpConfig->getBoolSetting(cfg::groupmessagesbytag));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTextEdit,0,0,1,7);
    pLayout->addWidget(pClearMessageWidgetButton,1,0,1,1);
    pLayout->addWidget(pShowErrorMessagesButton,1,1,1,1);
    pLayout->addWidget(pShowWarningMessagesButton,1,2,1,1);
    pLayout->addWidget(pShowInfoMessagesButton,1,3,1,1);
    pLayout->addWidget(pShowDebugMessagesButton,1,4,1,1);
    pLayout->addWidget(mpGroupByTagCheckBox, 1,5,1,1);
    pLayout->setContentsMargins(4,4,4,4);

    this->setLayout(pLayout);
    connect(pClearMessageWidgetButton, SIGNAL(clicked()),this, SLOT(clear()));
    connect(pShowErrorMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showErrorMessages(bool)));
    connect(pShowWarningMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showWarningMessages(bool)));
    connect(pShowInfoMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showInfoMessages(bool)));
    connect(pShowDebugMessagesButton, SIGNAL(toggled(bool)), this, SLOT(showDebugMessages(bool)));
    connect(mpGroupByTagCheckBox, SIGNAL(toggled(bool)), this, SLOT(setGroupByTag(bool)));

}

void MessageWidget::addText(const QString &rText)
{
    receiveMessage(GUIMessage(rText, "", UndefinedMessageType));
}


//! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the message widget when docked
QSize MessageWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    //Set very small height. A minimum apparently stops at reasonable size.
    size.rheight() = 1; //pixels
    return size;
}


//! @brief Loads message widget settings from configuration object
//! This is needed because messages widget must be created before configuration
//! (so that config can print messages), which means that message widgets cannot
//! load the config directly in the constructor.
void MessageWidget::loadConfig()
{
    mpGroupByTagCheckBox->setChecked(gpConfig->getBoolSetting(cfg::groupmessagesbytag));
}


bool MessageWidget::textEditHasFocus()
{
    return mpTextEdit->hasFocus();
}

void MessageWidget::receiveMessage(const GUIMessage &rMessage)
{
    mNewMessageList.append(rMessage);
    printNewMessagesOnly();
}


//! @brief Sets the color for the next added message, depending on type of message
//! @param type Type name of the message (error, warning, info etc...)
void MessageWidget::determineMessageColor(MessageTypeEnumT type)
{
    if (type == Error || type == Fatal)
    {
        mpTextEdit->setTextColor("RED");
    }
    else if (type == Warning)
    {
        mpTextEdit->setTextColor(QColor(216, 115, 0));
    }
    else if (type == Info)
    {
        mpTextEdit->setTextColor("BLACK");
    }
    else if (type == Debug)
    {
        mpTextEdit->setTextColor("BLUE");
    }
    else
    {
        mpTextEdit->setTextColor("GRAY");
    }
}


void MessageWidget::printNewMessagesOnly()
{
    // Loop through message list and print messages
    for(int msg=0; msg<mNewMessageList.size(); ++msg)
    {
        printOneMessage(mNewMessageList.at(msg));
    }

    mPrintedMessageList.append(mNewMessageList);
    mNewMessageList.clear();

    mpTextEdit->verticalScrollBar()->setValue(mpTextEdit->verticalScrollBar()->maximum());
}


//! @brief Updates the displayed messages from the message list
void MessageWidget::reprintEverything()
{
    // Clear the message box (we can not call this->clear(), since this would also clear the message list and we wouldn't have anything to print)
    mpTextEdit->clear();
    mPrintedMessageList.append(mNewMessageList);
    mNewMessageList.clear();
    mLastTag = QString();
    mSubsequentTags = 1;

    // Loop through message list and print messages
    for(int msg=0; msg<mPrintedMessageList.size(); ++msg)
    {
        printOneMessage(mPrintedMessageList.at(msg));
    }

    mpTextEdit->verticalScrollBar()->setValue(mpTextEdit->verticalScrollBar()->maximum());
}


void MessageWidget::printOneMessage(const GUIMessage &rMessage)
{
    // Only show message if its type shall be shown
    if( (rMessage.mType == Info    && mShowInfoMessages)     ||
        (rMessage.mType == Warning && mShowWarningMessages)  ||
        (rMessage.mType == Error   && mShowErrorMessages)    ||
        (rMessage.mType == Debug   && mShowDebugMessages)    ||
        (rMessage.mType == UndefinedMessageType)             ||
        (rMessage.mType == Fatal) )
    {

        QString msg;
        // Add the message with or without timestamp
        if (!rMessage.mTimestamp.isEmpty())
        {
            msg.append("[" + rMessage.mTimestamp + "] ");
        }
        msg.append(rMessage.mMessage);

        // If message is tagged, and group by tag setting is active
        if( mGroupByTag && !rMessage.mTag.isEmpty() && (rMessage.mTag == mLastTag) )
        {
            ++mSubsequentTags;
            mpTextEdit->undo();
            msg.append(QString("    (%1 similar)").arg(mSubsequentTags));
        }
        // Else message is not tagged, or group by tag setting is not active
        else
        {
            mSubsequentTags = 1;
            mLastTag = rMessage.mTag;
        }

        // Write the message (needs to be printed all at once)
        determineMessageColor(rMessage.mType);
        mpTextEdit->append(msg);
    }
}


//! @brief Clear function for message widget, this will empty the message widget and also remove all messages from the list
void MessageWidget::clear()
{
    mpTextEdit->clear();
    mPrintedMessageList.clear();
    mNewMessageList.clear();
    reprintEverything();
}


//! @brief Tells the message widget whether or not messages shall be grouped by tags
//! @param value True means that messages shall be grouped
void MessageWidget::setGroupByTag(bool value)
{
    mGroupByTag = value;
    gpConfig->setBoolSetting(cfg::groupmessagesbytag, value);
    reprintEverything();
}


//! @brief Tells the message widget whether or not it shall show error messages
//! @param value True means show messages
void MessageWidget::showErrorMessages(bool value)
{
    mShowErrorMessages = value;
    reprintEverything();
}


//! @brief Tells the message widget whether or not it shall show warning messages
//! @param value True means show messages
void MessageWidget::showWarningMessages(bool value)
{
    mShowWarningMessages = value;
    reprintEverything();
}


//! @brief Tells the message widget whether or not it shall show info messages
//! @param value True means show messages
void MessageWidget::showInfoMessages(bool value)
{
    mShowInfoMessages = value;
    reprintEverything();
}


//! @brief Tells the message widget whether or not it shall show debug messages
//! @param value True means show messages
void MessageWidget::showDebugMessages(bool value)
{
    mShowDebugMessages = value;
    reprintEverything();
}


void MessageWidget::copy()
{
    if(mpTextEdit->hasFocus())
    {
        mpTextEdit->copy();
    }
}


void MessageWidget::mouseMoveEvent(QMouseEvent *event)
{
    mpTextEdit->setFrameShape(QFrame::NoFrame);
    QWidget::mouseMoveEvent(event);
}


bool MessageWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        mpTextEdit->setFrameShape(QFrame::NoFrame);
        return true;
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}
