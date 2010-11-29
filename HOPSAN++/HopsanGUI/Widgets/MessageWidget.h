#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QPlainTextEdit>
#include "HopsanCore.h"


class MainWindow;
class CoreMessagesAccess;
class GUIMessage;

class MessageWidget : public QTextEdit
{
    Q_OBJECT
public:
    MessageWidget(MainWindow *pParent=0);
    void printCoreMessages();
    void printGUIMessage(QString message, QString tag=QString());
    void printGUIErrorMessage(QString message, QString tag=QString());
    void printGUIWarningMessage(QString message, QString tag=QString());
    void printGUIInfoMessage(QString message, QString tag=QString());
    void printGUIDebugMessage(QString message, QString tag=QString());
    QSize sizeHint() const;

public slots:
    void clear();
    void checkMessages();
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDefaultMessages(bool value);
    void showDebugMessages(bool value);

private:
    void setMessageColor(QString type);
    void updateDisplay();
    size_t tagCount(QString tag);
    QList< GUIMessage > mMessageList;
    bool mGroupByTag;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDefaultMessages;
    bool mShowDebugMessages;


    CoreMessagesAccess *mpCoreAccess;

};


class GUIMessage
{
public:
    GUIMessage(QString message, QString type, QString tag="");
    QString message;
    QString type;
    QString tag;
};

#endif // MESSAGEWIDGET_H
