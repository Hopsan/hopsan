#include "MessageWidget.h"
#include "../MainWindow.h"
#include "../CoreAccess.h"

using namespace hopsan;

//! @class MessageWidget
//! @brief The class for the message widget at the bottom of the main window
//!
//! Messages are either gathered from core or added directly from GUI. They are added to a list and then shown by calling the updateDisplay() function.
//!

//! @brief Constructor for the message widget class
//! @param pParent Parent pointer (not necessary)
MessageWidget::MessageWidget(MainWindow *pParent)
    : QTextEdit(pParent)
{
    mGroupByTag = false;

    mShowErrorMessages = true;
    mShowWarningMessages = true;
    mShowInfoMessages = true;
    mShowDefaultMessages = true;
    mShowDebugMessages = false;

    mpCoreAccess = new CoreMessagesAccess;
}


//! @brief Reimplementation of QTextEdit::sizeHint(), probably used to reduce the size of the message widget
QSize MessageWidget::sizeHint() const
{
    QSize size = QTextEdit::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 48; //pixels
    return size;
}


//! @brief Sets the color for the next added message, depending on type of message
//! @param type Type name of the message (error, warning, info etc...)
void MessageWidget::setMessageColor(QString type)
{
    if (type == "error")
    {
        setTextColor("RED");
    }
    else if (type == "warning")
    {
        setTextColor("ORANGE");
    }
    else if (type == "info")
    {
        setTextColor("GREEN");
    }
    else if (type == "default")
    {
        setTextColor("BLACK");
    }
    else if (type == "debug")
    {
        setTextColor("BLUE");
    }
    else
    {
        setTextColor("GRAY");
    }
}


//! @brief Updates the displayed messages from the message list
void MessageWidget::updateDisplay()
{
    QTextEdit::clear();         //Clear the message box (we can not call this->clear(), since this would also clear the message list and we wouldn't have anything to print)
    QStringList usedTags;

        //Loop through message list and print messages
    QList<GUIMessage>::iterator it;
    for(it=mMessageList.begin(); it!=mMessageList.end(); ++it)
    {
        if( !((*it).type == "error" && !mShowErrorMessages) &&          //Do not show message if its type shall not be shown
            !((*it).type == "warning" && !mShowWarningMessages) &&
            !((*it).type == "info" && !mShowInfoMessages) &&
            !((*it).type == "default" && !mShowDefaultMessages) &&
            !((*it).type == "debug" && !mShowDebugMessages))
        {
            setMessageColor((*it).type);
            if(!(*it).tag.isEmpty() && mGroupByTag)     //Message is tagged, and group by tag setting is active
            {
                if(!usedTags.contains((*it).tag))       //Check that tag is not used before
                {
                    usedTags.append((*it).tag);
                    size_t nTags = tagCount((*it).tag);
                    if(nTags == 1)                      //There is only one tag, so avoid appending "(0 similar)"
                    {
                        append((*it).message);
                    }
                    else                                //There are more than one tag, so append ("X similar)"
                    {
                        QString numString;
                        numString.setNum(nTags-1);
                        append((*it).message + " (" + numString + " similar)");
                    }
                }
            }
            else        //Message is not tagged, or group by tag setting is not active
            {
                append((*it).message);
            }
        }
    }
}


//! @brief Help function that counts how many messages with a specified tag that exists in message list
//! @param tag Name of the tag that shall be counted
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
    size_t nmsg = mpCoreAccess->getNumberOfMessages();
    for (size_t idx=0; idx < nmsg; ++idx)
    {
        QString message, type, tag;
        mpCoreAccess->getMessage(message, type, tag);
        mMessageList.append(GUIMessage(message, type, tag));
        this->updateDisplay();
    }
}


//! @brief Prints a GUI message of default type
//! @param message String containing the message
void MessageWidget::printGUIMessage(QString message, QString tag)
{
    mMessageList.append(GUIMessage(message, "default", tag));
    updateDisplay();
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
    QTextEdit::clear();
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


//! @brief Tells the message widget wether or not it shall show default messages
//! @param value True means show messages
void MessageWidget::showDefaultMessages(bool value)
{
    mShowDefaultMessages = value;
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
    this->message = message;
    this->type = type;
    this->tag = tag;
}

