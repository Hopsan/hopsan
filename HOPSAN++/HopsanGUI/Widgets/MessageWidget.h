//!
//! @file   MessageWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-10-xx
//!
//! @brief Contains the MessageWidget that dissplays messages to the user
//!
//$Id$

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QtGui>

class MainWindow;
class CoreMessagesAccess;
class GUIMessage;

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    MessageWidget(MainWindow *pParent=0);
    void printCoreMessages();
    void printGUIInfoMessage(QString message, QString tag=QString());
    void printGUIErrorMessage(QString message, QString tag=QString());
    void printGUIWarningMessage(QString message, QString tag=QString());
    void printGUIDebugMessage(QString message, QString tag=QString());
    QSize sizeHint() const;

public slots:
    void clear();
    void checkMessages();
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);

private:
    void setMessageColor(QString type);
    void updateDisplay();
    size_t subsequentTagCount(QString tag, size_t startIdx);
    size_t tagCount(QString tag);
    QList< GUIMessage > mMessageList;
    bool mGroupByTag;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;

    CoreMessagesAccess *mpCoreAccess;

    QTextEdit *mpTextEdit;
    QGridLayout *mpLayout;
    QPushButton *mpClearMessageWidgetButton;
    QToolButton *mpShowErrorMessagesButton;
    QToolButton *mpShowWarningMessagesButton;
    QToolButton *mpShowInfoMessagesButton;
    QToolButton *mpShowDebugMessagesButton;
    QCheckBox *mpGroupByTagCheckBox;
};


class GUIMessage
{
public:
    GUIMessage(QString message, QString type, QString tag="");
    QString message;
    QString type;
    QString tag;
    QString time;
};

#endif // MESSAGEWIDGET_H
