#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QPlainTextEdit>
#include "HopsanCore.h"

class MessageWidget : public QTextEdit
{
    Q_OBJECT
private:
    HopsanEssentials *mpHopsanCore;
    void setMessageColor(int type);

public:
    MessageWidget(QWidget *pParent=0);
    void setHopsanCorePtr(HopsanEssentials *pHopsanCore);
    void printCoreMessages();
    void printGUIMessage(QString message);

public slots:
    void checkMessages();
};

#endif // MESSAGEWIDGET_H
