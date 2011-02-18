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

#include <QtGui>
#include <QtXml>

class MainWindow;
class PythonQtScriptingConsole;

class PyDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    PyDockWidget(MainWindow *pMainWindow, QWidget * parent = 0);
    QString getScriptFileName();
    void saveSettingsToDomElement(QDomElement &rDomElement);
    void loadSettingsFromDomElement(QDomElement &rDomElement);

public slots:
    void runPyScript();

private:
    PythonQtScriptingConsole *mpPyConsole;
    QLineEdit *mpScriptFileLineEdit;
};

#endif // PYDOCKWIDGET_H
