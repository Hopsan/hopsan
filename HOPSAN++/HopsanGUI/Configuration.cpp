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
#include "PyDock.h"

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
    appendDomTextNode(settings, "backgroundcolor", mBackgroundColor.name());
    appendDomBooleanNode(settings, "antialiasing", mAntiAliasing);
    appendDomBooleanNode(settings, "invertwheel", mInvertWheel);
    appendDomBooleanNode(settings, "snapping", mSnapping);
    appendDomBooleanNode(settings, "progressbar", mEnableProgressBar);
    appendDomValueNode(settings, "progressbar_step", mProgressBarStep);
    appendDomBooleanNode(settings, "multicore", mUseMulticore);
    appendDomValueNode(settings, "numberofthreads", mNumberOfThreads);

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
    for(size_t i=0; i<mUserLibs.size(); ++i)
    {
        appendDomTextNode(libs, "userlib", mUserLibs.at(i));
    }

    QDomElement models = appendDomElement(configRoot, "models");
    for(size_t i=0; i<mLastSessionModels.size(); ++i)
    {
        if(mLastSessionModels.at(i) != "")
        {
            appendDomTextNode(models, "lastsessionmodel", mLastSessionModels.at(i));
        }
    }
    for(size_t i = 0; i<mRecentModels.size(); ++i)
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
    QFile xmlsettings(QString(MAINPATH) + "hopsanconfig.xml");
    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << QString(MAINPATH) << "settings.xml";
        return;
    }
    QTextStream out(&xmlsettings);
    domDocument.save(out, IndentSize);
}


//! @brief Updates all settings from hopsanconfig.xml
void Configuration::loadFromXml()
{
    //Apply default values
    mInvertWheel = false;
    mUseMulticore = false;
    mNumberOfThreads = 0;
    mEnableProgressBar = true;
    mProgressBarStep = 50;
    mSnapping = true;
    mBackgroundColor = QColor("white");
    mAntiAliasing = true;
    mLastSessionModels.clear();
    mRecentModels.clear();

    mDefaultUnits.insert("Value", "-");
    mDefaultUnits.insert("Pressure", "Pa");
    mDefaultUnits.insert("Flow", "m^3/s");
    mDefaultUnits.insert("Force", "N");
    mDefaultUnits.insert("Position", "m");
    mDefaultUnits.insert("Velocity", "m/s");
    mDefaultUnits.insert("Torque", "Nm");
    mDefaultUnits.insert("Angle", "rad");
    mDefaultUnits.insert("Angular Velocity", "rad/s");

    QMap<QString, QPen> isoPenMap;
    QMap<QString, QPen> userPenMap;

    QMap<QString, QMap<QString, QPen> > powerPenMap;
    isoPenMap.insert("Primary", QPen(QColor("black"),1, Qt::SolidLine, Qt::RoundCap));
    isoPenMap.insert("Active", QPen(QColor("red"), 2, Qt::SolidLine, Qt::RoundCap));
    isoPenMap.insert("Hover", QPen(QColor("darkRed"),2, Qt::SolidLine, Qt::RoundCap));
    powerPenMap.insert("Iso", isoPenMap);
    userPenMap.insert("Primary", QPen(QColor("black"),2, Qt::SolidLine, Qt::RoundCap));
    userPenMap.insert("Active", QPen(QColor("red"), 3, Qt::SolidLine, Qt::RoundCap));
    userPenMap.insert("Hover", QPen(QColor("darkRed"),3, Qt::SolidLine, Qt::RoundCap));
    powerPenMap.insert("User", userPenMap);
    mPenStyles.insert("Power", powerPenMap);

    isoPenMap.clear();
    userPenMap.clear();

    QMap<QString, QMap<QString, QPen> > signalPenMap;
    isoPenMap.insert("Primary", QPen(QColor("blue"),1, Qt::DashLine));
    isoPenMap.insert("Active", QPen(QColor("red"), 2, Qt::DashLine));
    isoPenMap.insert("Hover", QPen(QColor("darkRed"),2, Qt::DashLine));
    signalPenMap.insert("Iso", isoPenMap);
    userPenMap.insert("Primary", QPen(QColor("blue"),1, Qt::DashLine));
    userPenMap.insert("Active", QPen(QColor("red"), 2, Qt::DashLine));
    userPenMap.insert("Hover", QPen(QColor("darkRed"),2, Qt::DashLine));
    signalPenMap.insert("User", userPenMap);
    mPenStyles.insert("Signal", signalPenMap);

    isoPenMap.clear();

    QMap<QString, QMap<QString, QPen> >nonFinishedPenMap;
    isoPenMap.insert("Primary", QPen(QColor("lightslategray"),3,Qt::SolidLine, Qt::RoundCap));
    nonFinishedPenMap.insert("Iso", isoPenMap);
    nonFinishedPenMap.insert("User", isoPenMap);
    mPenStyles.insert("NonFinished", nonFinishedPenMap);


    //Definition of custom units
    QMap<QString, double> PressureUnitMap;
    PressureUnitMap.insert("Pa", 1);
    PressureUnitMap.insert("Bar", 1e-5);
    PressureUnitMap.insert("MPa", 1e-6);
    PressureUnitMap.insert("psi", 1.450326e-4);
    QMap<QString, double> FlowUnitMap;
    FlowUnitMap.insert("m^3/s", 1);
    FlowUnitMap.insert("l/min", 60000);
    QMap<QString, double> ForceUnitMap;
    ForceUnitMap.insert("N", 1);
    ForceUnitMap.insert("kN", 1e-3);
    QMap<QString, double> PositionUnitMap;
    PositionUnitMap.insert("m", 1);
    PositionUnitMap.insert("mm", 1000);
    PositionUnitMap.insert("cm", 100);
    PositionUnitMap.insert("inch", 39.3700787);
    PositionUnitMap.insert("ft", 3.2808);
    QMap<QString, double> VelocityUnitMap;
    VelocityUnitMap.insert("m/s", 1);
    QMap<QString, double> TorqueUnitMap;
    TorqueUnitMap.insert("Nm", 1);
    QMap<QString, double> AngleUnitMap;
    AngleUnitMap.insert("rad", 1);
    AngleUnitMap.insert("deg", 57.296);
    QMap<QString, double> AngularVelocityUnitMap;
    AngularVelocityUnitMap.insert("rad/s", 1);
    AngularVelocityUnitMap.insert("deg/s", 57.296);
    AngularVelocityUnitMap.insert("rev/s", 0.159155);
    AngularVelocityUnitMap.insert("rpm", 9.549296585);
    QMap<QString, double> ValueUnitMap;
    ValueUnitMap.insert("-", 1);
    mCustomUnits.insert("Pressure", PressureUnitMap);
    mCustomUnits.insert("Flow", FlowUnitMap);
    mCustomUnits.insert("Force", ForceUnitMap);
    mCustomUnits.insert("Position", PositionUnitMap);
    mCustomUnits.insert("Velocity", VelocityUnitMap);
    mCustomUnits.insert("Torque", TorqueUnitMap);
    mCustomUnits.insert("Angle", AngleUnitMap);
    mCustomUnits.insert("Angular Velocity", AngularVelocityUnitMap);
    mCustomUnits.insert("Value", ValueUnitMap);


    //Read from hopsanconfig.xml
    QFile file(QString(MAINPATH) + "hopsanconfig.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIWarningMessage("Unable to read settings file. Using default settings.");
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

            //Load settings to PyDock in MainWindow
            QDomElement pythonElement = configRoot.firstChildElement("python");
            gpMainWindow->getPythonDock()->loadSettingsFromDomElement(pythonElement);
        }
    }
    file.close();
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
