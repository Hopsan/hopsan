#include "MessageWidget.h"

MessageWidget::MessageWidget(QWidget *pParent)
    : QPlainTextEdit(pParent)
{
    mpHopsanCore = 0;

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
                //! @todo coloring depending on type maybe
                appendPlainText(QString::fromStdString(msg.message));
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
    appendPlainText(message);
}

void MessageWidget::checkMessages()
{
    printCoreMessages();
}
