#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

class MessageWidget;

class MessageHandler
{
public:
    MessageHandler(MessageWidget *pMessageWidget);

public slots:
    void addInfoMessage(const QString &msg);
    void addWarningMessage(const QString &msg);
    void addErrorMessage(const QString &msg);
    void clear();

private:
    MessageWidget *mpMessageWidget;
};

#endif // MESSAGEHANDLER_H
