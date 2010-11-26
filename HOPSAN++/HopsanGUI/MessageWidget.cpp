#include "MessageWidget.h"
#include "MainWindow.h"
#include "CoreSystemAccess.h"

using namespace hopsan;

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

QSize MessageWidget::sizeHint() const
{
    QSize size = QTextEdit::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 48; //pixels
    return size;
}

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


//    //*****Core Interaction*****
//    HopsanCoreMessage msg;
//    HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
//    if (pHopsanCore != 0)
//    {
//        size_t nmsg = pHopsanCore->checkMessage();
//        for (size_t idx=0; idx < nmsg; ++idx)
//        {
//            msg = pHopsanCore->getMessage();
//            mMessageList.append(msg);
//            this->updateDisplay();
//        }
//    }
//    else
//    {
//        printGUIMessage("The hopsan core pointer is not set, can not get core messages");
//    }
//    //**************************
}

void MessageWidget::printGUIMessage(QString message)
{
//    hopsan::HopsanCoreMessage msg;
//    msg.message = message.toStdString();
//    msg.type = hopsan::HopsanCoreMessage::DEFAULT;
    this->mMessageList.append(QPair<QString, QString>(message, "default"));
    this->updateDisplay();
}

void MessageWidget::printGUIErrorMessage(QString message)
{
//    hopsan::HopsanCoreMessage msg;
//    msg.message = message.toStdString();
//    msg.type = hopsan::HopsanCoreMessage::ERROR;
    this->mMessageList.append(QPair<QString, QString>(message, "error"));
    this->updateDisplay();
}

void MessageWidget::printGUIWarningMessage(QString message)
{
//    hopsan::HopsanCoreMessage msg;
//    msg.message = message.toStdString();
//    msg.type = hopsan::HopsanCoreMessage::WARNING;
    this->mMessageList.append(QPair<QString, QString>(message, "warning"));
    this->updateDisplay();
}

void MessageWidget::printGUIInfoMessage(QString message)
{
//    hopsan::HopsanCoreMessage msg;
//    msg.message = message.toStdString();
//    msg.type = hopsan::HopsanCoreMessage::INFO;
    this->mMessageList.append(QPair<QString, QString>(message, "info"));
    this->updateDisplay();
}

void MessageWidget::checkMessages()
{
    printCoreMessages();
}


void MessageWidget::showErrorMessages(bool value)
{
    mShowErrorMessages = value;
    updateDisplay();
}

void MessageWidget::showWarningMessages(bool value)
{
    mShowWarningMessages = value;
    updateDisplay();
}

void MessageWidget::showInfoMessages(bool value)
{
    mShowInfoMessages = value;
    updateDisplay();
}

void MessageWidget::showDefaultMessages(bool value)
{
    mShowDefaultMessages = value;
    updateDisplay();
}

void MessageWidget::showDebugMessages(bool value)
{
    mShowDebugMessages = value;
    updateDisplay();
}



