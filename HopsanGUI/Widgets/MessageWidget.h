/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   MessageWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-10-xx
//!
//! @brief Contains the MessageWidget that displays messages to the user
//!
//$Id$

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QWidget>
#include "MessageHandler.h"

// Forward Declaration
class QTextEdit;
class QCheckBox;

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    MessageWidget(QWidget *pParent=0);
    void addText(const QString &rText);
    QSize sizeHint() const;
    void loadConfig();
    bool textEditHasFocus();

public slots:
    void receiveMessage(const GUIMessage &rMessage);
    void setGroupByTag(bool value);
    void showErrorMessages(bool value);
    void showWarningMessages(bool value);
    void showInfoMessages(bool value);
    void showDebugMessages(bool value);
    void clear();
    void copy();

protected:
    void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void printNewMessagesOnly();
    void reprintEverything();
    void determineMessageColor(MessageTypeEnumT type);
    void printOneMessage(const GUIMessage &rMessage);
    QList< GUIMessage > mNewMessageList;
    QList< GUIMessage > mPrintedMessageList;
    bool mGroupByTag;
    bool mShowErrorMessages;
    bool mShowInfoMessages;
    bool mShowWarningMessages;
    bool mShowDebugMessages;
    QString mLastTag;
    int mSubsequentTags;

    QTextEdit *mpTextEdit;
    QCheckBox *mpGroupByTagCheckBox;
};




#endif // MESSAGEWIDGET_H
