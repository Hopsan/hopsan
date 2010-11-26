#include "MessageWidget.h"
#include "MainWindow.h"
#include "CoreAccess.h"

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
    QTextEdit::clear();
    QStringList usedTags;
    QList<GUIMessage>::iterator it;
    for(it=mMessageList.begin(); it!=mMessageList.end(); ++it)
    {
        if( !((*it).type == "error" && !mShowErrorMessages) &&
            !((*it).type == "warning" && !mShowWarningMessages) &&
            !((*it).type == "info" && !mShowInfoMessages) &&
            !((*it).type == "default" && !mShowDefaultMessages) &&
            !((*it).type == "debug" && !mShowDebugMessages))
        {
            setMessageColor((*it).type);
            qDebug() << (*it).tag;
            if(!(*it).tag.isEmpty() && mGroupByTag)
            {
                qDebug() << "Blä 1";
                if(!usedTags.contains((*it).tag))
                {
                    qDebug() << "Blä 2";
                    usedTags.append((*it).tag);
                    QString numString;
                    numString.setNum(tagCount((*it).tag)-1);
                    append((*it).message + " (" + numString + " similar)");
                }
            }
            else
            {
                append((*it).message);
            }
        }
    }
}


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




GUIMessage::GUIMessage(QString message, QString type, QString tag)
{
    this->message = message;
    this->type = type;
    this->tag = tag;
}

