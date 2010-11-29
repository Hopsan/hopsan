//!
//! @file   PyDock.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a derived QDockWidget class that contain a Python console
//!
//$Id$

#ifndef PYDOCK_H
#define PYDOCK_H

#include <QtGui>
#include <QtXml>

class MainWindow;
class PythonQtScriptingConsole;

class PyDock : public QDockWidget
{
    Q_OBJECT

public:
    PyDock(MainWindow *pMainWindow, QWidget * parent = 0);
    QString getScriptFileName();
    void saveSettingsToDomElement(QDomElement &rDomElement);
    void loadSettingsFromDomElement(QDomElement &rDomElement);

public slots:
    void runPyScript();

private:
    PythonQtScriptingConsole *mpPyConsole;
    QLineEdit *mpScriptFileLineEdit;
};

#endif // PYDOCK_H
