#include "MessageWidget.h"
#include "MainWindow.h"
#include "CoreAccess.h"

using namespace hopsan;


//! @brief Constructor for the message widget class
//! @param pParent Parent pointer (not necessary)
MessageWidget::MessageWidget(MainWindow *pParent)
    : QTextEdit(pParent)
{
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
    this->clear();
    QList< QPair<QString, QString> >::iterator it;
    for(it=mMessageList.begin(); it!=mMessageList.end(); ++it)
    {
        if( !((*it).second == "error" && !mShowErrorMessages) &&
            !((*it).second == "warning" && !mShowWarningMessages) &&
            !((*it).second == "info" && !mShowInfoMessages) &&
            !((*it).second == "default" && !mShowDefaultMessages) &&
            !((*it).second == "debug" && !mShowDebugMessages))
        {
            setMessageColor((*it).second);
            append((*it).first);
        }
    }
}


//! @brief Obtains messages from core and prints them in the message widget
void MessageWidget::printCoreMessages()
{
    size_t nmsg = mpCoreAccess->getNumberOfMessages();
    for (size_t idx=0; idx < nmsg; ++idx)
    {
        QString message, type;
        mpCoreAccess->getMessage(message, type);
        mMessageList.append(QPair<QString, QString>(message, type));
        this->updateDisplay();
    }
}


//! @brief Prints a GUI message of default type
//! @param message String containing the message
void MessageWidget::printGUIMessage(QString message)
{
    mMessageList.append(QPair<QString, QString>(message, "default"));
    updateDisplay();
}


//! @brief Prints a GUI error message
//! @param message String containing the message
void MessageWidget::printGUIErrorMessage(QString message)
{
    mMessageList.append(QPair<QString, QString>(message.prepend("Error: "), "error"));
    updateDisplay();
}


//! @brief Prints a GUI warning message
//! @param message String containing the message
void MessageWidget::printGUIWarningMessage(QString message)
{
    mMessageList.append(QPair<QString, QString>(message.prepend("Warning: "), "warning"));
    updateDisplay();
}


//! @brief Prints a GUI info message
//! @param message String containing the message
void MessageWidget::printGUIInfoMessage(QString message)
{
    mMessageList.append(QPair<QString, QString>(message.prepend("Info: "), "info"));
    updateDisplay();
}


//! @brief Prints a GUI info message
//! @param message String containing the message
void MessageWidget::printGUIDebugMessage(QString message)
{
    mMessageList.append(QPair<QString, QString>(message.prepend("Debug: "), "debug"));
    updateDisplay();
}


//! @brief Slot that checks messages from core and prints them
void MessageWidget::checkMessages()
{
    printCoreMessages();
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



