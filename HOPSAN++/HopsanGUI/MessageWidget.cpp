#include "MessageWidget.h"
#include "mainwindow.h"


MessageWidget::MessageWidget(MainWindow *pParent)
    : QTextEdit(pParent)
{
    mpParentMainWindow = pParent;
    mpHopsanCore = 0;

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
    else
    {
        setTextColor("BLACK");
    }
}

void MessageWidget::setHopsanCorePtr(HopsanEssentials* pHopsanCore)
{
    mpHopsanCore = pHopsanCore;
}

void MessageWidget::printCoreMessages()
{
    HopsanCoreMessage msg;
    if (mpHopsanCore != 0)
    {
        size_t nmsg = mpHopsanCore->checkMessage();
        for (size_t idx=0; idx < nmsg; ++idx)
        {
            msg = mpHopsanCore->getMessage();
            if (true) //! @todo Debug level rules
            {
                setMessageColor(msg.type);
                append(QString::fromStdString(msg.message));
            }
        }
    }
    else
    {
        printGUIMessage("The hopsan core pointer is not set, can not get core messages");
    }
}

void MessageWidget::printGUIMessage(QString message)
{
    //! @todo make better
    setMessageColor(-1);
    append(message);
}

void MessageWidget::printGUIErrorMessage(QString message)
{
    //! @todo make better
    setMessageColor(HopsanCoreMessage::ERROR);
    append(message);
}

void MessageWidget::printGUIWarningMessage(QString message)
{
    //! @todo make better
    setMessageColor(HopsanCoreMessage::WARNING);
    append(message);
}

void MessageWidget::printGUIInfoMessage(QString message)
{
    //! @todo make better
    setMessageColor(HopsanCoreMessage::INFO);
    append(message);
}

void MessageWidget::checkMessages()
{
    printCoreMessages();
}
