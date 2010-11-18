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
#include "MessageWidget.h"

#include <QDomElement>
#include <QMessageBox>
#include <QMap>


Configuration::Configuration()
{
}

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

QMap<QString, double> testMap;
testMap.insert("Hej", 123);

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
    gpMainWindow->mpMessageWidget->printGUIErrorMessage("Unable to read settings file. Using default settings.");
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
            mUseMulticore = parseDomValueNode(settingsElement.firstChildElement("numberofthreads"));

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
    }
}
file.close();
}






bool Configuration::getInvertWheel()
{
    return this->mInvertWheel;
}


bool Configuration::getUseMulticore()
{
    return this->mUseMulticore;
}


size_t Configuration::getNumberOfThreads()
{
    return this->mNumberOfThreads;
}


int Configuration::getProgressBarStep()
{
    return this->mProgressBarStep;
}


bool Configuration::getEnableProgressBar()
{
    return this->mEnableProgressBar;
}


QColor Configuration::getBackgroundColor()
{
    return this->mBackgroundColor;
}


bool Configuration::getAntiAliasing()
{
    return this->mAntiAliasing;
}


QStringList Configuration::getUserLibs()
{
    return this->mUserLibs;
}


bool Configuration::getSnapping()
{
    return this->mSnapping;
}


QStringList Configuration::getRecentModels()
{
    return this->mRecentModels;
}


QStringList Configuration::getLastSessionModels()
{
    return this->mLastSessionModels;
}


QString Configuration::getDefaultUnit(QString key)
{
    return this->mDefaultUnits.find(key).value();
}


QMap<QString, double> Configuration::getCustomUnits(QString key)
{
    return this->mCustomUnits.find(key).value();
}



void Configuration::setInvertWheel(bool value)
{
    this->mInvertWheel = value;
}


void Configuration::setUseMultiCore(bool value)
{
    this->mUseMulticore = value;
}


void Configuration::setNumberOfThreads(size_t value)
{
    this->mNumberOfThreads = value;
}


void Configuration::setProgressBarStep(int value)
{
    this->mProgressBarStep = value;
}


void Configuration::setEnableProgressBar(bool value)
{
    this->mEnableProgressBar = value;
}


void Configuration::setBackgroundColor(QColor value)
{
    this->mBackgroundColor = value;
}


void Configuration::setAntiAliasing(bool value)
{
    this->mAntiAliasing = value;
}


void Configuration::addUserLib(QString value)
{
    if(!mUserLibs.contains(value))
    {
        this->mUserLibs.append(value);
    }
}


void Configuration::removeUserLib(QString value)
{
    mUserLibs.removeAll(value);
}


bool Configuration::hasUserLib(QString value)
{
    return mUserLibs.contains(value);
}


void Configuration::setSnapping(bool value)
{
    this->mSnapping = value;
}


void Configuration::addRecentModel(QString value)
{
    mRecentModels.removeAll(value);
    mRecentModels.prepend(value);
    while(mRecentModels.size() > 10)
    {
        mRecentModels.pop_back();
    }
}


void Configuration::addLastSessionModel(QString value)
{
    mLastSessionModels.append(value);
}


void Configuration::clearLastSessionModels()
{
    mLastSessionModels.clear();
}


void Configuration::setDefaultUnit(QString key, QString value)
{
    this->mDefaultUnits.remove(key);
    this->mDefaultUnits.insert(key, value);
}


void Configuration::addCustomUnit(QString dataname, QString parname, double scale)
{
    this->mCustomUnits.find(dataname).value().insert(parname, scale);
}


