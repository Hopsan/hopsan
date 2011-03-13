//!
//! @file   Configuration.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for the configuration object
//!
//$Id$

#include "common.h"
#include "Configuration.h"
#include "Utilities/XMLUtilities.h"
#include "Utilities/GUIUtilities.h"
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/PyDockWidget.h"

#include <QDomElement>
#include <QMessageBox>
#include <QMap>


//! @class Configuration
//! @brief The Configuration class is used as a global XML-based storage for program configuration variables
//!
//! Use loadFromXml or saveToXml functions to read or write to hopsanconfig.xml. Use get, set, add and clear functions to access the data.
//!


//! @brief Saves the current settings to hopsanconfig.xml
void Configuration::saveToXml()
{
        //Write to hopsanconfig.xml
    QDomDocument domDocument;
    QDomElement configRoot = domDocument.createElement("hopsanconfig");
    domDocument.appendChild(configRoot);

    QDomElement settings = appendDomElement(configRoot,"settings");
    appendDomBooleanNode(settings, "showwelcomedialog", mShowWelcomeDialog);
    appendDomTextNode(settings, "backgroundcolor", mBackgroundColor.name());
    appendDomBooleanNode(settings, "antialiasing", mAntiAliasing);
    appendDomBooleanNode(settings, "invertwheel", mInvertWheel);
    appendDomBooleanNode(settings, "snapping", mSnapping);
    appendDomBooleanNode(settings, "progressbar", mEnableProgressBar);
    appendDomValueNode(settings, "progressbar_step", mProgressBarStep);
    appendDomBooleanNode(settings, "multicore", mUseMulticore);
    appendDomValueNode(settings, "numberofthreads", mNumberOfThreads);
    appendDomBooleanNode(settings, "togglenamesbuttonchecked", gpMainWindow->toggleNamesAction->isChecked());
    appendDomBooleanNode(settings, "toggleportsbuttonchecked", gpMainWindow->togglePortsAction->isChecked());

    QDomElement style = appendDomElement(configRoot, "style");

    QMap<QString, QMap<QString, QMap<QString, QPen> > >::iterator it1;
    QMap<QString, QMap<QString, QPen> >::iterator it2;
    QMap<QString, QPen>::iterator it3;

    for(it1 = mPenStyles.begin(); it1 != mPenStyles.end(); ++it1)
    {
        for(it2 = it1.value().begin(); it2 != it1.value().end(); ++it2)
        {
            for(it3 = it2.value().begin(); it3 != it2.value().end(); ++it3)
            {
                QDomElement tempElement = appendDomElement(style, "penstyle");
                tempElement.setAttribute("type", it1.key());
                tempElement.setAttribute("gfxtype", it2.key());
                tempElement.setAttribute("situation", it3.key());
                tempElement.setAttribute("color", it3.value().color().name());
                tempElement.setAttribute("width", it3.value().width());
                tempElement.setAttribute("style", it3.value().style());
                tempElement.setAttribute("capstyle", it3.value().capStyle());
            }
        }
    }

    QDomElement libs = appendDomElement(configRoot, "libs");
    for(int i=0; i<mUserLibs.size(); ++i)
    {
        appendDomTextNode(libs, "userlib", mUserLibs.at(i));
    }

    QDomElement models = appendDomElement(configRoot, "models");
    for(int i=0; i<mLastSessionModels.size(); ++i)
    {
        if(mLastSessionModels.at(i) != "")
        {
            appendDomTextNode(models, "lastsessionmodel", mLastSessionModels.at(i));
        }
    }
    for(int i = 0; i<mRecentModels.size(); ++i)
    {
        if(mRecentModels.at(i) != "")
            appendDomTextNode(models, "recentmodel", mRecentModels.at(i));
    }

    QDomElement units = appendDomElement(configRoot, "units");
    QMap<QString, QString>::iterator itdu;
    for(itdu = mDefaultUnits.begin(); itdu != mDefaultUnits.end(); ++itdu)
    {
        QDomElement xmlTemp = appendDomElement(units, "defaultunit");
        xmlTemp.setAttribute("name", itdu.key());
        xmlTemp.setAttribute("unit", itdu.value());
    }
    QMap<QString, QMap<QString, double> >::iterator itpcu;
    QMap<QString, double>::iterator itcu;
    for(itpcu = mCustomUnits.begin(); itpcu != mCustomUnits.end(); ++itpcu)
    {
        for(itcu = itpcu.value().begin(); itcu != itpcu.value().end(); ++itcu)
        {
            QDomElement xmlTemp = appendDomElement(units, "customunit");
            xmlTemp.setAttribute("name", itpcu.key());
            xmlTemp.setAttribute("unit", itcu.key());
            xmlTemp.setAttribute("scale", itcu.value());
        }
    }

    //Save python session
    QDomElement python = appendDomElement(configRoot, "python");
    gpMainWindow->getPythonDock()->saveSettingsToDomElement(python);

    appendRootXMLProcessingInstruction(domDocument);

    //Save to file
    const int IndentSize = 4;
    QFile xmlsettings(gExecPath+QString(MAINPATH) + "hopsanconfig.xml");
    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << gExecPath+QString(MAINPATH) << "settings.xml";
        return;
    }
    QTextStream out(&xmlsettings);
    domDocument.save(out, IndentSize);
}


//! @brief Updates all settings from hopsanconfig.xml
void Configuration::loadFromXml()
{
    //Read from hopsandefaults.xml
    loadDefaultsFromXml();

    //Read from hopsanconfig.xml
    QFile file(gExecPath+QString(MAINPATH) + "hopsanconfig.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIWarningMessage("Unable to find configuration file. Configuration file was recreated with default settings.");
        return;
    }
    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 gpMainWindow->tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != "hopsanconfig")
        {
            QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                     "The file is not an Hopsan Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != hopsanconfig");
        }
        else
        {
            QDomElement settingsElement = configRoot.firstChildElement("settings");

            if(!settingsElement.firstChildElement("showwelcomedialog").isNull())
                mShowWelcomeDialog = parseDomBooleanNode(settingsElement.firstChildElement("showwelcomedialog"));
            if(!settingsElement.firstChildElement("backgroundcolor").isNull())
                mBackgroundColor.setNamedColor(settingsElement.firstChildElement("backgroundcolor").text());
            if(!settingsElement.firstChildElement("antialiasing").isNull())
                mAntiAliasing = parseDomBooleanNode(settingsElement.firstChildElement("antialiasing"));
            if(!settingsElement.firstChildElement("invertwheel").isNull())
                mInvertWheel = parseDomBooleanNode(settingsElement.firstChildElement("invertwheel"));
            if(!settingsElement.firstChildElement("snapping").isNull())
                mSnapping = parseDomBooleanNode(settingsElement.firstChildElement("snapping"));
            if(!settingsElement.firstChildElement("progressbar").isNull())
                mEnableProgressBar = parseDomBooleanNode(settingsElement.firstChildElement("progressbar"));
            if(!settingsElement.firstChildElement("progressbar_step").isNull())
                mProgressBarStep = parseDomValueNode(settingsElement.firstChildElement("progressbar_step"));
            if(!settingsElement.firstChildElement("multicore").isNull())
                mUseMulticore = parseDomBooleanNode(settingsElement.firstChildElement("multicore"));
            if(!settingsElement.firstChildElement("numberofthreads").isNull())
                mNumberOfThreads = parseDomValueNode(settingsElement.firstChildElement("numberofthreads"));
            if(!settingsElement.firstChildElement("togglenamesbuttonchecked").isNull())
                mToggleNamesButtonCheckedLastSession = parseDomBooleanNode(settingsElement.firstChildElement("togglenamesbuttonchecked"));
            if(!settingsElement.firstChildElement("toggleportsbuttonchecked").isNull())
                mTogglePortsButtonCheckedLastSession = parseDomBooleanNode(settingsElement.firstChildElement("toggleportsbuttonchecked"));


            QDomElement styleElement = configRoot.firstChildElement("style");
            QDomElement penElement = styleElement.firstChildElement("penstyle");
            while(!penElement.isNull())
            {
                QString type = penElement.attribute("type");
                QString gfxType = penElement.attribute("gfxtype");
                QString situation = penElement.attribute("situation");
                QString color = penElement.attribute("color");
                int width = penElement.attribute("width").toInt();
                Qt::PenStyle style = Qt::PenStyle(penElement.attribute("style").toInt());
                Qt::PenCapStyle capStyle = Qt::PenCapStyle(penElement.attribute("capstyle").toInt());
                QPen pen = QPen(QColor(color), width, style, capStyle);

                if(!mPenStyles.contains(type))
                {
                    QMap<QString, QMap<QString, QPen> > tempMap;
                    mPenStyles.insert(type, tempMap);
                }
                if(!mPenStyles.find(type).value().contains(gfxType))
                {
                    QMap<QString, QPen> tempMap;
                    mPenStyles.find(type).value().insert(gfxType, tempMap);
                }
                mPenStyles.find(type).value().find(gfxType).value().insert(situation, pen);

                penElement = penElement.nextSiblingElement("penstyle");
            }

            QDomElement libsElement = configRoot.firstChildElement("libs");
            QDomElement userLibElement = libsElement.firstChildElement("userlib");
            while (!userLibElement.isNull())
            {
                mUserLibs.prepend(userLibElement.text());
                userLibElement = userLibElement.nextSiblingElement(("userlib"));
            }

            QDomElement modelsElement = configRoot.firstChildElement("models");
            QDomElement lastSessionElement = modelsElement.firstChildElement("lastsessionmodel");
            while (!lastSessionElement.isNull())
            {
                mLastSessionModels.prepend(lastSessionElement.text());
                lastSessionElement = lastSessionElement.nextSiblingElement("lastsessionmodel");
            }
            QDomElement recentModelElement = modelsElement.firstChildElement("recentmodel");
            while (!recentModelElement.isNull())
            {
                mRecentModels.prepend(recentModelElement.text());
                recentModelElement = recentModelElement.nextSiblingElement("recentmodel");
            }

            QDomElement unitsElement = configRoot.firstChildElement("units");
            QDomElement defaultUnitElement = unitsElement.firstChildElement("defaultunit");
            while (!defaultUnitElement.isNull())
            {
                mDefaultUnits.insert(defaultUnitElement.attribute("name"),
                                     defaultUnitElement.attribute("unit"));
                defaultUnitElement = defaultUnitElement.nextSiblingElement("defaultunit");
            }
            QDomElement customUnitElement = unitsElement.firstChildElement("customunit");
            while (!customUnitElement.isNull())
            {
                QString physicalQuantity = customUnitElement.attribute("name");
                QString unitName = customUnitElement.attribute("unit");
                double unitScale = customUnitElement.attribute("scale").toDouble();

                if(!mCustomUnits.find(physicalQuantity).value().contains(unitName))
                {
                    mCustomUnits.find(physicalQuantity).value().insert(unitName, unitScale);
                }
                customUnitElement = customUnitElement.nextSiblingElement("customunit");
            }

            //Load settings to PyDockWidget in MainWindow
            QDomElement pythonElement = configRoot.firstChildElement("python");
            gpMainWindow->getPythonDock()->loadSettingsFromDomElement(pythonElement);
        }
    }
    file.close();
}



void Configuration::loadDefaultsFromXml()
{
    //Read from hopsandefaults.xml
    QFile file(gExecPath+QString(MAINPATH) + "hopsandefaults");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 "Unable to read default configuration file. Please reinstall program.\n" + gExecPath+QString(MAINPATH));

        qApp->quit();
    }
    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 gpMainWindow->tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != "hopsandefaults")
        {
            QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                     "The file is not an Hopsan Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != hopsandefaults");
        }
        else
        {
            QDomElement settingsElement = configRoot.firstChildElement("settings");

            if(!settingsElement.firstChildElement("showwelcomedialog").isNull())
                mShowWelcomeDialog = parseDomBooleanNode(settingsElement.firstChildElement("showwelcomedialog"));
            if(!settingsElement.firstChildElement("backgroundcolor").isNull())
                mBackgroundColor.setNamedColor(settingsElement.firstChildElement("backgroundcolor").text());
            if(!settingsElement.firstChildElement("antialiasing").isNull())
                mAntiAliasing = parseDomBooleanNode(settingsElement.firstChildElement("antialiasing"));
            if(!settingsElement.firstChildElement("invertwheel").isNull())
                mInvertWheel = parseDomBooleanNode(settingsElement.firstChildElement("invertwheel"));
            if(!settingsElement.firstChildElement("snapping").isNull())
                mSnapping = parseDomBooleanNode(settingsElement.firstChildElement("snapping"));
            if(!settingsElement.firstChildElement("progressbar").isNull())
                mEnableProgressBar = parseDomBooleanNode(settingsElement.firstChildElement("progressbar"));
            if(!settingsElement.firstChildElement("progressbar_step").isNull())
                mProgressBarStep = parseDomValueNode(settingsElement.firstChildElement("progressbar_step"));
            if(!settingsElement.firstChildElement("multicore").isNull())
                mUseMulticore = parseDomBooleanNode(settingsElement.firstChildElement("multicore"));
            if(!settingsElement.firstChildElement("numberofthreads").isNull())
                this->mNumberOfThreads = parseDomValueNode(settingsElement.firstChildElement("numberofthreads"));
            if(!settingsElement.firstChildElement("togglenamesbuttonchecked").isNull())
                mToggleNamesButtonCheckedLastSession = parseDomBooleanNode(settingsElement.firstChildElement("togglenamesbuttonchecked"));
            if(!settingsElement.firstChildElement("toggleportsbuttonchecked").isNull())
                mTogglePortsButtonCheckedLastSession = parseDomBooleanNode(settingsElement.firstChildElement("toggleportsbuttonchecked"));

            QDomElement styleElement = configRoot.firstChildElement("style");
            QDomElement penElement = styleElement.firstChildElement("penstyle");
            while(!penElement.isNull())
            {
                QString type = penElement.attribute("type");
                QString gfxType = penElement.attribute("gfxtype");
                QString situation = penElement.attribute("situation");
                QString color = penElement.attribute("color");
                int width = penElement.attribute("width").toInt();
                Qt::PenStyle style = Qt::PenStyle(penElement.attribute("style").toInt());
                Qt::PenCapStyle capStyle = Qt::PenCapStyle(penElement.attribute("capstyle").toInt());
                QPen pen = QPen(QColor(color), width, style, capStyle);

                if(!mPenStyles.contains(type))
                {
                    QMap<QString, QMap<QString, QPen> > tempMap;
                    mPenStyles.insert(type, tempMap);
                }
                if(!mPenStyles.find(type).value().contains(gfxType))
                {
                    QMap<QString, QPen> tempMap;
                    mPenStyles.find(type).value().insert(gfxType, tempMap);
                }
                mPenStyles.find(type).value().find(gfxType).value().insert(situation, pen);

                penElement = penElement.nextSiblingElement("penstyle");
            }
            QDomElement unitsElement = configRoot.firstChildElement("units");
            QDomElement defaultUnitElement = unitsElement.firstChildElement("defaultunit");
            while (!defaultUnitElement.isNull())
            {
                mDefaultUnits.insert(defaultUnitElement.attribute("name"),
                                     defaultUnitElement.attribute("unit"));
                defaultUnitElement = defaultUnitElement.nextSiblingElement("defaultunit");
            }
            mCustomUnits.insert("Pressure", QMap<QString, double>());
            mCustomUnits.insert("Flow", QMap<QString, double>());
            mCustomUnits.insert("Force", QMap<QString, double>());
            mCustomUnits.insert("Position", QMap<QString, double>());
            mCustomUnits.insert("Velocity", QMap<QString, double>());
            mCustomUnits.insert("Torque", QMap<QString, double>());
            mCustomUnits.insert("Angle", QMap<QString, double>());
            mCustomUnits.insert("Angular Velocity", QMap<QString, double>());
            mCustomUnits.insert("Value", QMap<QString, double>());
            QDomElement customUnitElement = unitsElement.firstChildElement("customunit");
            while (!customUnitElement.isNull())
            {
                QString physicalQuantity = customUnitElement.attribute("name");
                QString unitName = customUnitElement.attribute("unit");
                double unitScale = customUnitElement.attribute("scale").toDouble();
                if(!mCustomUnits.find(physicalQuantity).value().contains(unitName))
                {
                    mCustomUnits.find(physicalQuantity).value().insert(unitName, unitScale);
                }
                customUnitElement = customUnitElement.nextSiblingElement("customunit");
            }
        }
    }
    file.close();

    //! @todo This shall not be hard coded!
    mPalette = QPalette(QColor("black"), QColor("blue"), QColor("lightblue"), QColor("darkblue"), QColor("blue"),QColor("text"),QColor("gray"), QColor(244,247,251), QColor(222,231,241));

    mStyleSheet.append("QPushButton                 { border: 1px solid gray;   border-style: outset;   border-radius: 5px;     padding: 4px;   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(222,231,241), stop: 1 white);     min-width: 80px; }");
    mStyleSheet.append("QPushButton:pressed         { border: 2px solid blue;   border-style: outset;   border-radius: 5px;     padding: 4px;   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 yellow, stop: 1 white);               min-width: 80px; }");
    mStyleSheet.append("QPushButton:hover:pressed   { border: 2px solid blue;   border-style: outset;   border-radius: 5px;     padding: 4px;   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 yellow, stop: 1 white);               min-width: 80px; }");
    mStyleSheet.append("QPushButton:default         { border: 1px solid gray;   border-style: outset;   border-radius: 5px;     padding: 4px;   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(222,231,241), stop: 1 white);     min-width: 80px; }");
    mStyleSheet.append("QPushButton:hover           { border: 2px solid blue;   border-style: outset;   border-radius: 5px;     padding: 4px;   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(222,231,241), stop: 1 white);     min-width: 80px; }");

    mStyleSheet.append("QTabWidget:pane                  { border: 1px solid gray; background-color: rgb(222,231,241); padding: 0px; position: absolute; top: -0px; margin-top: -1px; } ");

    mStyleSheet.append("QTabBar:tab:top                     { background-color: rgb(244,247,251);                                                                                       border: 1px solid gray;     border-bottom: 1px solid gray;          border-top-left-radius: 4px;        border-top-right-radius: 4px;       padding: 5px; }");
    mStyleSheet.append("QTabBar:tab:top:hover               { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 white, stop: 1 rgb(244,247,251));                   border: 1px solid gray;     border-bottom: 1px solid gray;          border-top-left-radius: 4px;        border-top-right-radius: 4px;       padding: 0px; }");
    mStyleSheet.append("QTabBar:tab:top:selected            { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(244,247,251), stop: 1 rgb(222,231,241));        border: 1px solid gray;     border-bottom: 1px rgb(244,247,251);    border-top-left-radius: 4px;        border-top-right-radius: 4px;       padding: 5px; }");
    mStyleSheet.append("QTabBar:tab:top:selected:hover      { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 white, stop: 1 rgb(222,231,241));                   border: 1px solid gray;     border-bottom: 1px rgb(244,247,251);    border-top-left-radius: 4px;        border-top-right-radius: 4px;       padding: 0px; }");

    mStyleSheet.append("QTabBar:tab:bottom                  { background-color: rgb(244,247,251);                                                                                       border: 1px solid gray;     border-top: 1px solid gray;             border-bottom-left-radius: 4px;     border-bottom-right-radius: 4px;    padding: 5px; }");
    mStyleSheet.append("QTabBar:tab:bottom:hover            { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(244,247,251), stop: 1 white);                   border: 1px solid gray;     border-top: 1px solid gray;             border-bottom-left-radius: 4px;     border-bottom-right-radius: 4px;    padding: 0px; }");
    mStyleSheet.append("QTabBar:tab:bottom:selected         { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(222,231,241), stop: 1 rgb(244,247,251));        border: 1px solid gray;     border-top: 1px rgb(244,247,251);       border-bottom-left-radius: 4px;     border-bottom-right-radius: 4px;    padding: 5px; }");
    mStyleSheet.append("QTabBar:tab:bottom:selected:hover   { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(222,231,241), stop: 1 white);                   border: 1px solid gray;     border-top: 1px rgb(244,247,251);       border-bottom-left-radius: 4px;     border-bottom-right-radius: 4px;    padding: 0px; }");

    mStyleSheet.append("QTreeWidget { background-color: rgb(244,247,251); }");

    mStyleSheet.append("QTextEdit { background-color: rgb(244,247,251); }");

    mStyleSheet.append("QListWidget { background-color: rgb(244,247,251); }");
    mStyleSheet.append("QListWidget:item { background-color: rgb(244,247,251); }");
    mStyleSheet.append("QListWidget:item:hover { background-color: palegreen; }");

    //mPalette = gpMainWindow->palette();
    //mStyleSheet.clear();

    return;
}



//! @brief Returns whether or not the welcome dialog shall be shown
bool Configuration::getShowWelcomeDialog()
{
    return this->mShowWelcomeDialog;
}


//! @brief Returns whether or not invert wheel shall be used
bool Configuration::getInvertWheel()
{
    return this->mInvertWheel;
}


//! @brief Returns whether or not multi-threading shall be used
bool Configuration::getUseMulticore()
{
    return this->mUseMulticore;
}


//! @brief Returns number of simulation threads that shall be used
size_t Configuration::getNumberOfThreads()
{
    return this->mNumberOfThreads;
}


//! @brief Returns whether or not the toggle names button was checked at the end of last session
bool Configuration::getToggleNamesButtonCheckedLastSession()
{
    return this->mToggleNamesButtonCheckedLastSession;
}


//! @brief Returns whether or not the toggle ports button was checked at the end of last session
bool Configuration::getTogglePortsButtonCheckedLastSession()
{
    return this->mTogglePortsButtonCheckedLastSession;
}


//! @brief Returns the step size that shall be used in progress bar
int Configuration::getProgressBarStep()
{
    return this->mProgressBarStep;
}


//! @brief Returns whether or not the progress bar shall be displayed during simulation
bool Configuration::getEnableProgressBar()
{
    return this->mEnableProgressBar;
}


//! @brief Returns the background color
QColor Configuration::getBackgroundColor()
{
    return this->mBackgroundColor;
}


//! @brief Returns whether or not anti-aliasing shall be used
bool Configuration::getAntiAliasing()
{
    return this->mAntiAliasing;
}


//! @brief Returns a list of paths to the user libraries that shall be loaded
QStringList Configuration::getUserLibs()
{
    return this->mUserLibs;
}


//! @brief Returns whether or not connector snapping shall be used
bool Configuration::getSnapping()
{
    return this->mSnapping;
}


//! @brief Returns a list of paths to recently opened models
QStringList Configuration::getRecentModels()
{
    return this->mRecentModels;
}


//! @brief Returns a list of paths to models that were open last time program was closed
QStringList Configuration::getLastSessionModels()
{
    return this->mLastSessionModels;
}


//! @brief Returns the selected default unit for the specified physical quantity
//! @param key Name of the physical quantity (e.g. "Pressure" or "Velocity")
QString Configuration::getDefaultUnit(QString key)
{
    return this->mDefaultUnits.find(key).value();
}


//! @brief Returns a map with custom units (names and scale factor) for specified physical quantity
//! @param key Name of the physical quantity (e.g. "Pressure" or "Velocity")
QMap<QString, double> Configuration::getCustomUnits(QString key)
{
    return this->mCustomUnits.find(key).value();
}


//! @brief Returns connector pen for specified connector type
//! @param type Type of connector (Power, Signal, NonFinished)
//! @param gfxType Graphics type (User or Iso)
//! @param situation Defines when connector is used (Primary, Hovered, Active)
QPen Configuration::getPen(QString type, graphicsType gfxType, QString situation)
{
    QString gfxString;
    if(gfxType == ISOGRAPHICS) { gfxString = "Iso"; }
    else { gfxString = "User"; }

    if(mPenStyles.contains(type))
    {
        if(mPenStyles.find(type).value().contains(gfxString))
        {
            if(mPenStyles.find(type).value().find(gfxString).value().contains(situation))
            {
                return mPenStyles.find(type).value().find(gfxString).value().find(situation).value();
            }
        }
    }
    return QPen(QColor("black"), 1, Qt::SolidLine, Qt::SquareCap);
}


QPalette Configuration::getPalette()
{
    return mPalette;
}


QString Configuration::getStyleSheet()
{
    return mStyleSheet;
}


//! @brief Set function for invert wheel option
//! @param value Desired setting
void Configuration::setShowWelcomeDialog(bool value)
{
    this->mShowWelcomeDialog = value;
}

//! @brief Set function for invert wheel option
//! @param value Desired setting
void Configuration::setInvertWheel(bool value)
{
    this->mInvertWheel = value;
}


//! @brief Set function for multi-threading option
//! @param value Desired setting
void Configuration::setUseMultiCore(bool value)
{
    this->mUseMulticore = value;
}


//! @brief Set function for number of simlation threads
//! @param value Desired number of threads
void Configuration::setNumberOfThreads(size_t value)
{
    this->mNumberOfThreads = value;
}


//! @brief Set function for progress bar time step
//! @param value Desired step
void Configuration::setProgressBarStep(int value)
{
    this->mProgressBarStep = value;
}


//! @brief Set function for use progress bar setting
//! @param value Desired setting
void Configuration::setEnableProgressBar(bool value)
{
    this->mEnableProgressBar = value;
}


//! @brief Set function for background color setting
//! @param value Desired color
void Configuration::setBackgroundColor(QColor value)
{
    this->mBackgroundColor = value;
}


//! @brief Set function for anti-aliasing setting
//! @param value Desired setting
void Configuration::setAntiAliasing(bool value)
{
    this->mAntiAliasing = value;
}


//! @brief Adds a user library to the library list
//! @param value Path to the new library
void Configuration::addUserLib(QString value)
{
    if(!mUserLibs.contains(value))
    {
        this->mUserLibs.append(value);
    }
}


//! @brief Removes a user library from the library list
//! @param value Path to the library that is to be removed
void Configuration::removeUserLib(QString value)
{
    mUserLibs.removeAll(value);
}


//! @brief Tells whether or not a specified user library exist in the library list
//! @param value Path to the library
bool Configuration::hasUserLib(QString value)
{
    return mUserLibs.contains(value);
}


//! @brief Set function for connector snapping setting
//! @param value Desired setting
void Configuration::setSnapping(bool value)
{
    this->mSnapping = value;
}


//! @brief Adds a model to the list of recently opened models
//! @brief value Path to the model
void Configuration::addRecentModel(QString value)
{
    mRecentModels.removeAll(value);
    mRecentModels.prepend(value);
    while(mRecentModels.size() > 10)
    {
        mRecentModels.pop_back();
    }
}


//! @brief Adds a model to the list of models that was open last time program closed
//! @param value Path to the model
void Configuration::addLastSessionModel(QString value)
{
    mLastSessionModels.append(value);
}


//! @brief Removes all last session models from the list
void Configuration::clearLastSessionModels()
{
    mLastSessionModels.clear();
}


//! @brief Sets the default unit for specified physical quantity
//! @param key Name of the physical quantity (e.g. "Pressure" or "Velocity")
//! @param value Name of the desired default unit
void Configuration::setDefaultUnit(QString key, QString value)
{
    this->mDefaultUnits.remove(key);
    this->mDefaultUnits.insert(key, value);
}


//! @brief Adds a new custom unit to the specified physical quantity
//! @param dataname Name of the physical quantity (e.g. "Pressure" or "Velocity")
//! @param unitname Name of the new unit
//! @param scale Scale factor from SI unit to the new unit
void Configuration::addCustomUnit(QString dataname, QString unitname, double scale)
{
    this->mCustomUnits.find(dataname).value().insert(unitname, scale);
}
