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
//! @file   PyDockWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a derived QDockWidget class that contain a Python console
//!
//$Id$

#ifndef PYDOCKWIDGET_H
#define PYDOCKWIDGET_H

#ifdef USEPYTHONQT

#include <QDockWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QToolButton>
#include <QtXml>

#include "MessageHandler.h"

// Forward declarations
class PythonQtScriptingConsole;

class PythonTerminalWidget : public QWidget
{
    Q_OBJECT

public:
    PythonTerminalWidget(QWidget * parent = 0);
    //QSize sizeHint() const;
    QString getScriptFileName();
    void saveSettingsToDomElement(QDomElement &rDomElement);
    QString getLastOutput();
    QString runCommand(QString command);

public slots:
    void runPyScript();
    void runPyScript(QString command);
    void runMultipleCommands(QString command, int n);
    void printMessage(const GUIMessage &rMessage);

private slots:
    void loadPyScript();

private:
    PythonQtScriptingConsole *mpPyConsole;
    QLineEdit *mpScriptFileLineEdit;
    QToolButton *mpLoadScriptButton;
    bool mDoPrintHopsanMessage;
};
#endif

#endif // PYDOCKWIDGET_H
