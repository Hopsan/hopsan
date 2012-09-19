/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HcomWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-11-19
//!
//! @brief Contains the HcomWidget that dissplays messages to the user
//!
//$Id$

#ifndef HCOMWIDGET_H
#define HCOMWIDGET_H

#include <QtGui>

class MainWindow;
class Port;

class HcomWidget : public QTextEdit
{
    Q_OBJECT
public:
    HcomWidget(MainWindow *pParent=0);
    QSize sizeHint() const;
    void loadConfig();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);
    virtual void keyPressEvent(QKeyEvent * event);

private:
    //Cursor & keypress functions
    bool isOnLastLine();
    void handleEnterKeyPress();
    void handleUpKeyPress();
    void handleDownKeyPress();
    void handleTabKeyPress();
    void cancelAutoComplete();
    void cancelRecentHistory();

    //Command functions
    void executeCommand(QString cmd);
    void executePlotCommand(QString cmd);
    void executeDisplayParameterCommand(QString cmd);
    void executeChangeParameterCommand(QString cmd);
    void executeChangeSimulationSettingsCommand(QString cmd);
    void executeHelpCommand(QString cmd);

    //Help functions
    QString getShortVariableName(QString var);

    QTextEdit *mpTextEdit;
    QGridLayout *mpLayout;

    QStringList mCmdList;

    //Plotting
    int mCurrentPlotWindow;

    //Recent history (up and down keys)
    QStringList mHistory;
    int mCurrentHistoryItem;

    //Autocomplete
    QString mAutoCompleteFilter;
    QStringList mAutoCompleteResults;
    int mCurrentAutoCompleteIndex;
};

#endif // HCOMWIDGET_H
