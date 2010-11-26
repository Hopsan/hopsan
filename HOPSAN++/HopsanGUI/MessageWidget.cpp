#include "MessageWidget.h"
#include "MainWindow.h"

using namespace hopsan;

MessageWidget::MessageWidget(MainWindow *pParent)
    : QTextEdit(pParent)
{
    mShowErrorMessages = true;
    mShowWarningMessages = true;
    mShowInfoMessages = true;
    mShowDefaultMessages = true;
    mShowDebugMessages = false;
}

QSize MessageWidget::sizeHint() const
{
    QSize size = QTextEdit::sizeHint();
    //Set very small height. A minimum apperantly stops at resonable size.
    size.rheight() = 48; //pixels
    return size;
}

void MessageWidget::setMessageColor(int type)
{
    if (type == HopsanCoreMessage::ERROR)
    {
        setTextColor("RED");
    }
    else if (type == HopsanCoreMessage::WARNING)
    {
        setTextColor("ORANGE");
    }
    else if (type == HopsanCoreMessage::INFO)
    {
        setTextColor("GREEN");
    }
    else if (type == HopsanCoreMessage::DEFAULT)
    {
        setTextColor("BLACK");
    }
    else
    {
        setTextColor("GRAY");
    }
}


void MessageWidget::updateDisplay()
{
    this->clear();
    QList<hopsan::HopsanCoreMessage>::iterator it;
    for(it=mMessageList.begin(); it!=mMessageList.end(); ++it)
    {
        if( !((*it).type == hopsan::HopsanCoreMessage::ERROR && !mShowErrorMessages) &&
            !((*it).type == hopsan::HopsanCoreMessage::WARNING && !mShowWarningMessages) &&
            !((*it).type == hopsan::HopsanCoreMessage::INFO && !mShowInfoMessages) &&
            !((*it).type == hopsan::HopsanCoreMessage::DEFAULT && !mShowDefaultMessages) &&
            !((*it).type == hopsan::HopsanCoreMessage::DEBUG && !mShowDebugMessages))
        {
            setMessageColor((*it).type);
            append(QString::fromStdString((*it).message));
        }
    }
}


void MessageWidget::printCoreMessages()
{
    //*****Core Interaction*****
    HopsanCoreMessage msg;
    HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
    if (pHopsanCore != 0)
    {
        size_t nmsg = pHopsanCore->checkMessage();
        for (size_t idx=0; idx < nmsg; ++idx)
        {
            msg = pHopsanCore->getMessage();
            mMessageList.append(msg);
            this->updateDisplay();
        }
    }
    else
    {
        printGUIMessage("The hopsan core pointer is not set, can not get core messages");
    }
    //**************************
}

void MessageWidget::printGUIMessage(QString message)
{
    hopsan::HopsanCoreMessage msg;
    msg.message = message.toStdString();
    msg.type = hopsan::HopsanCoreMessage::DEFAULT;
    this->mMessageList.append(msg);
    this->updateDisplay();
}

void MessageWidget::printGUIErrorMessage(QString message)
{
    hopsan::HopsanCoreMessage msg;
    msg.message = message.toStdString();
    msg.type = hopsan::HopsanCoreMessage::ERROR;
    this->mMessageList.append(msg);
    this->updateDisplay();
}

void MessageWidget::printGUIWarningMessage(QString message)
{
    hopsan::HopsanCoreMessage msg;
    msg.message = message.toStdString();
    msg.type = hopsan::HopsanCoreMessage::WARNING;
    this->mMessageList.append(msg);
    this->updateDisplay();
}

void MessageWidget::printGUIInfoMessage(QString message)
{
    hopsan::HopsanCoreMessage msg;
    msg.message = message.toStdString();
    msg.type = hopsan::HopsanCoreMessage::INFO;
    this->mMessageList.append(msg);
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



