#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QPlainTextEdit>
#include "HopsanCore.h"


class MainWindow;
class CoreMessagesAccess;

class MessageWidget : public QTextEdit
{
    Q_OBJECT
public:
    MessageWidget(MainWindow *pParent=0);
    void printCoreMessages();
    void printGUIMessage(QString message);
    void printGUIErrorMessage(QString message);
    void printGUIWarningMessage(QString message);
    void printGUIInfoMessage(QString message);
    void printGUIDebugMessage(QString message);
    QSize sizeHint() const;

public slots:
    void checkMessages();
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDefaultMessages(bool value);
    void showDebugMessages(bool value);

private:
    void setMessageColor(QString type);
    void updateDisplay();
    QList< QPair<QString, QString> > mMessageList;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDefaultMessages;
    bool mShowDebugMessages;

    CoreMessagesAccess *mpCoreAccess;

};

#endif // MESSAGEWIDGET_H
