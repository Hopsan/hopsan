#include <QColor>

#include "MessageHandler.h"
#include "Widgets/MessageWidget.h"

MessageHandler::MessageHandler(MessageWidget *pMessageWidget)
{
    mpMessageWidget = pMessageWidget;
}

void MessageHandler::addInfoMessage(const QString &msg)
{
    mpMessageWidget->addText("Info: "+msg, Qt::black);
}

void MessageHandler::addWarningMessage(const QString &msg)
{
    mpMessageWidget->addText("Warning: "+msg, Qt::darkYellow);
}

void MessageHandler::addErrorMessage(const QString &msg)
{
    mpMessageWidget->addText("Error: "+msg, Qt::red);
}
