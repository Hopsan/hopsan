/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   Configuration.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for the configuration object
//!
//$Id$

//Hopsan includes
#include "common.h"
#include "global.h"
#include "version_gui.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Utilities/XMLUtilities.h"
#include "MessageHandler.h"

#include "Widgets/PyDockWidget.h"
//! @todo this config object should not need to include PyDockWidget

//Qt includes
#include <QDomElement>
#include <QMessageBox>
#include <QMap>
#include <QAction>
#include <QApplication>
#include <QMainWindow>


//! @class Configuration
//! @brief The Configuration class is used as a global XML-based storage for program configuration variables
//!
//! Use loadFromXml or saveToXml functions to read or write to hopsanconfig.xml. Use get, set, add and clear functions to access the data.
//!


//! @brief Saves the current settings to hopsanconfig.xml
Configuration::Configuration()
{
    // Resgister configuration options

    // String settings
    mStringSettings.insert(CFG_REMOTEHOPSANADDRESS, "");
    mStringSettings.insert(CFG_REMOTEHOPSANADDRESSSERVERADDRESS, "");
    mStringSettings.insert(CFG_LOADMODELDIR, gpDesktopHandler->getModelsPath());
    mStringSettings.insert(CFG_MODELGFXDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_PLOTDATADIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_PLOTGFXDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_SIMULINKEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_SUBSYSTEMDIR, gpDesktopHandler->getModelsPath());
    mStringSettings.insert(CFG_MODELICAMODELSDIR, gpDesktopHandler->getModelsPath());
    mStringSettings.insert(CFG_EXTERNALLIBDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_SCRIPTDIR, gpDesktopHandler->getScriptsPath());
    mStringSettings.insert(CFG_PLOTWINDOWDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_FMUIMPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_FMUEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_LABVIEWEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_GCC32DIR, "");
    mStringSettings.insert(CFG_GCC64DIR, "");
    mStringSettings.insert(CFG_PLOTGFXIMAGEFORMAT, "png");
    mStringSettings.insert(CFG_PLOTGFXDIMENSIONSUNIT, "px");

    // Bool settings
    mBoolSettings.insert(CFG_USEREMOTEADDRESSSERVER, false);
    mBoolSettings.insert(CFG_USEREMOTEOPTIMIZATION, false);
    mBoolSettings.insert(CFG_PLOTWINDOWSONTOP, true);
    mBoolSettings.insert(CFG_PLOTGFXUSESCREENSIZE, false);
    mBoolSettings.insert(CFG_PLOTGFXKEEPASPECT, true);
    mBoolSettings.insert(CFG_AUTOLIMITGENERATIONS, false);
    mBoolSettings.insert(CFG_CACHELOGDATA, true);
    mBoolSettings.insert(CFG_SHOWHIDDENNODEDATAVARIABLES, false);
    mBoolSettings.insert(CFG_AUTOBACKUP, true);
    mBoolSettings.insert(CFG_GROUPMESSAGESBYTAG, true);
    mBoolSettings.insert(CFG_GROUPMESSAGESBYTAG, true);
    mBoolSettings.insert(CFG_TOGGLENAMESBUTTONCHECKED, true);
    mBoolSettings.insert(CFG_TOGGLEPORTSBUTTONCHECKED, true);
    mBoolSettings.insert(CFG_SNAPPING, true);
    mBoolSettings.insert(CFG_INVERTWHEEL, false);
    mBoolSettings.insert(CFG_ANTIALIASING, true);
    mBoolSettings.insert(CFG_NATIVESTYLESHEET, false);
    mBoolSettings.insert(CFG_SHOWPOPUPHELP, true);
    mBoolSettings.insert(CFG_MULTICORE, false);
    mBoolSettings.insert(CFG_PROGRESSBAR, true);
    mBoolSettings.insert(CFG_SETPWDTOMWD, false);
    mBoolSettings.insert(CFG_SHOWLICENSEONSTARTUP, true);

    // Integer settings
    mIntegerSettings.insert(CFG_LIBRARYSTYLE, 0);
    mIntegerSettings.insert(CFG_PROGRESSBARSTEP, 100);
    mIntegerSettings.insert(CFG_NUMBEROFTHREADS, 0);
    mIntegerSettings.insert(CFG_GENERATIONLIMIT, 100);
    mIntegerSettings.insert(CFG_PLOEXPORTVERSION, 1);

    // Double settings
    mDoubleSettings.insert(CFG_PLOTGFXDPI, 96);
}

void Configuration::saveToXml()
{
    if (mWriteOnSave)
    {
        // Write to hopsanconfig.xml
        QDomDocument domDocument;
        QDomElement configRoot = domDocument.createElement(CFG_HOPSANCONFIG);
        configRoot.setAttribute(HMF_HOPSANGUIVERSIONTAG, HOPSANGUIVERSION);
        domDocument.appendChild(configRoot);

        QDomElement settings = appendDomElement(configRoot,CFG_SETTINGS);

        // Write string settings
        for(auto it=mStringSettings.begin(); it!=mStringSettings.end(); ++it)
        {
            appendDomTextNode(settings, it.key(), it.value());
        }

        // Write bool settings
        for(auto it=mBoolSettings.begin(); it!=mBoolSettings.end(); ++it)
        {
            appendDomBooleanNode(settings, it.key(), it.value());
        }

        // Write integer settings
        for(auto it=mIntegerSettings.begin(); it!=mIntegerSettings.end(); ++it)
        {
            appendDomIntegerNode(settings, it.key(), it.value());
        }

        // Write double settings
        for(auto it=mDoubleSettings.begin(); it!=mDoubleSettings.end(); ++it)
        {
            appendDomValueNode(settings, it.key(), it.value());
        }

        // Write other settings
        appendDomValueNode2(settings, CFG_PLOTGFXSIZE, mPlotGfxSize.width(), mPlotGfxSize.height());
        appendDomTextNode(settings, CFG_BACKGROUNDCOLOR, mBackgroundColor.name());

        // Write style
        QDomElement style = appendDomElement(configRoot, HMF_STYLETAG);
        for(auto it1 = mPenStyles.begin(); it1 != mPenStyles.end(); ++it1)
        {
            for(auto it2 = it1.value().begin(); it2 != it1.value().end(); ++it2)
            {
                for(auto it3 = it2.value().begin(); it3 != it2.value().end(); ++it3)
                {
                    QString type;
                    if(it1.key() == PowerConnectorStyle) type = "Power";
                    if(it1.key() == SignalConnectorStyle) type = "Signal";
                    if(it1.key() == BrokenConnectorStyle) type = "Broken";
                    if(it1.key() == UndefinedConnectorStyle) type = "Undefined";

                    QDomElement tempElement = appendDomElement(style, CFG_PENSTYLE);
                    tempElement.setAttribute(CFG_TYPE, type);
                    tempElement.setAttribute(CFG_GFXTYPE, it2.key());
                    tempElement.setAttribute(CFG_SITUATION, it3.key());
                    tempElement.setAttribute(CFG_COLOR, it3.value().color().name());
                    setQrealAttribute(tempElement, CFG_WIDTH, it3.value().widthF());
                    tempElement.setAttribute(HMF_STYLETAG, it3.value().style());
                    tempElement.setAttribute(CFG_CAPSTYLE, it3.value().capStyle());
                }
            }
        }

        QDomElement libs = appendDomElement(configRoot, XML_LIBS);
        for(int i=0; i<mUserLibs.size(); ++i)
        {
            QDomElement xmlUserLib = appendDomTextNode(libs, XML_USERLIB, mUserLibs.at(i).absoluteFilePath());
            QString typeStr = XML_LIBTYPE_INTERNAL;
            if(mUserLibTypes.at(i) == ExternalLib)
            {
                typeStr = XML_LIBTYPE_EXTERNAL;
            }
            else if(mUserLibTypes.at(i) == FmuLib)
            {
                typeStr = XML_LIBTYPE_FMU;
            }
            xmlUserLib.setAttribute(XML_LIBTYPE, typeStr);
        }

        QDomElement modelicaFilesXml = appendDomElement(configRoot, XML_MODELICAFILES);
        for(int i=0; i<mModelicaFiles.size(); ++i)
        {
            appendDomTextNode(modelicaFilesXml, XML_MODELICAFILE, mModelicaFiles.at(i).absoluteFilePath());
        }

        QDomElement models = appendDomElement(configRoot, XML_MODELS);
        for(int i=0; i<mLastSessionModels.size(); ++i)
        {
            if(mLastSessionModels.at(i) != "")
            {
                appendDomTextNode(models, XML_LASTSESSIONMODEL, mLastSessionModels.at(i));
            }
        }
        for(int i = mRecentModels.size()-1; i>-1; --i)
        {
            if(mRecentModels.at(i) != "")
                appendDomTextNode(models, XML_RECENTMODEL, mRecentModels.at(i));
        }
        for(int i = mRecentGeneratorModels.size()-1; i>-1; --i)
        {
            if(mRecentGeneratorModels.at(i) != "")
                appendDomTextNode(models, XML_RECENTGENERATORMODEL, mRecentGeneratorModels.at(i));
        }


        QDomElement xmlUnitScales = appendDomElement(configRoot, CFG_UNITSCALES);
        QMap<QString, QuantityUnitScale >::iterator qit;
        for(qit = mUnitScales.begin(); qit != mUnitScales.end(); ++qit)
        {
            QDomElement xmlQuantity = appendDomElement(xmlUnitScales, CFG_QUANTITY);
            xmlQuantity.setAttribute(HMF_NAMETAG, qit.key());
            QString siunit = qit.value().siunit;
            if (!siunit.isEmpty())
            {
                xmlQuantity.setAttribute(CFG_SIUNIT, siunit);
            }
            QMap<QString, UnitScale>::iterator cuit;
            for(cuit = qit.value().customScales.begin(); cuit != qit.value().customScales.end(); ++cuit)
            {
                if (cuit.key() != siunit)
                {
                    QDomElement xmlUS = appendDomTextNode(xmlQuantity, CFG_UNITSCALE, cuit.value().mScale);
                    xmlUS.setAttribute(CFG_UNIT, cuit.key());
                }
            }
        }

        QDomElement units = appendDomElement(configRoot, CFG_UNITS);
        QMap<QString, QString>::iterator itdu;
        for(itdu = mSelectedDefaultUnits.begin(); itdu != mSelectedDefaultUnits.end(); ++itdu)
        {
            QDomElement xmlTemp = appendDomElement(units, CFG_DEFAULTUNIT);
            xmlTemp.setAttribute(CFG_NAME, itdu.key());
            xmlTemp.setAttribute(CFG_UNIT, itdu.value());
        }


        //Save python session
#ifdef USEPYTHONQT
        QDomElement python = appendDomElement(configRoot, CFG_PYTHON);
        gpPythonTerminalWidget->saveSettingsToDomElement(python);
#endif

        QDomElement hcom = appendDomElement(configRoot, CFG_HCOM);
        appendDomTextNode(hcom, CFG_PWD, mHcomWorkingDirectory);
        for(int i=0; i<mTerminalHistory.size(); ++i)
        {
            appendDomTextNode(hcom, CFG_COMMAND, mTerminalHistory.at(i));
        }

        appendRootXMLProcessingInstruction(domDocument);

        //Save to file
        if(!QDir(gpDesktopHandler->getConfigPath()).exists())
        {
            QDir().mkpath(gpDesktopHandler->getConfigPath());
        }
        QFile xmlsettings(gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml"));
        if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Failed to open config file for writing: "+gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml"));
            return;
        }
        QTextStream out(&xmlsettings);
        domDocument.save(out, XMLINDENTATION);
    }
}


//! @brief Updates all settings from hopsanconfig.xml
void Configuration::loadFromXml()
{
    //Read from hopsandefaults.xml
    loadDefaultsFromXml();

    //Read from hopsanconfig.xml
    QFile file(gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gpMessageHandler->addWarningMessage(QString("Unable to find configuration file: %1, Configuration file was recreated with default settings.").arg(file.fileName()));
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Hopsan"),
                                 gpMainWindowWidget->tr("hopsanconfig.xml: Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != CFG_HOPSANCONFIG)
        {
            QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Hopsan"),
                                     "The file is not an Hopsan Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != hopsanconfig");
        }
        else
        {
            verifyConfigurationCompatibility(configRoot);     //Check version compatibility

            //Load user settings
            QDomElement settingsElement = configRoot.firstChildElement(HMF_SETTINGS);
            loadUserSettings(settingsElement);

            //Load style settings
            QDomElement styleElement = configRoot.firstChildElement(HMF_STYLETAG);
            loadStyleSettings(styleElement);

            //Load library settings
            QDomElement libsElement = configRoot.firstChildElement(XML_LIBS);
            loadLibrarySettings(libsElement);

            //Load Modelica files settings
            QDomElement modelicaFilesElement = configRoot.firstChildElement(XML_MODELICAFILES);
            loadModelicaFilesSettings(modelicaFilesElement);

            //Load model settings
            QDomElement modelsElement = configRoot.firstChildElement(XML_MODELS);
            loadModelSettings(modelsElement);

            //Load unit settings
            QDomElement unitscalesElement = configRoot.firstChildElement(CFG_UNITSCALES);
            loadUnitScales(unitscalesElement);
            QDomElement unitsElement = configRoot.firstChildElement(CFG_UNITS);
            loadUnitSettings(unitsElement);

            //Load settings to PyDockWidget in MainWindow
            QDomElement pythonElement = configRoot.firstChildElement(CFG_PYTHON);
            QDomElement hcomElement = configRoot.firstChildElement(CFG_HCOM);
            loadScriptSettings(pythonElement, hcomElement);
        }
    }
    file.close();
}



void Configuration::loadDefaultsFromXml()
{
    //Read from hopsandefaults.xml
#ifdef Q_OS_OSX
    QFile file(gpDesktopHandler->getResourcesPath() + "hopsandefaults");
#else
    QFile file(gpDesktopHandler->getMainPath() + "hopsandefaults");
#endif
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Hopsan"),
                                 "Unable to read default configuration file. Please reinstall program.\n" + gpDesktopHandler->getMainPath());

        qApp->quit();
    }
    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Hopsan"),
                                 gpMainWindowWidget->tr("hopsandefaults: Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != "hopsandefaults")
        {
            QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Hopsan"),
                                     "The file is not an Hopsan Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != hopsandefaults");
        }
        else
        {
            verifyConfigurationCompatibility(configRoot);

                //Load default user settings
            QDomElement settingsElement = configRoot.firstChildElement(CFG_SETTINGS);
            loadUserSettings(settingsElement);

                //Load default GUI style
            QDomElement styleElement = configRoot.firstChildElement(HMF_STYLETAG);
            loadStyleSettings(styleElement);

                //Load default units
            QDomElement unitscalesElement = configRoot.firstChildElement(CFG_UNITSCALES);
            loadUnitScales(unitscalesElement);
            QDomElement unitsElement = configRoot.firstChildElement(CFG_UNITS);
            loadUnitSettings(unitsElement);
        }
    }
    file.close();

    //Internal settings, not stored in xml (change later)
    mParallelAlgorighm = 0;

    return;
}

void Configuration::beginMultiSet()
{
    mWriteOnSave = false;
}

void Configuration::endMultiSet()
{
    mWriteOnSave = true;
}

QString Configuration::getStringSetting(const QString &rName) const
{
    Q_ASSERT(mStringSettings.contains(rName));
    return mStringSettings.value(rName, "StringSettingNotFond");
}

bool Configuration::getBoolSetting(const QString &rName) const
{
    Q_ASSERT(mBoolSettings.contains(rName));
    return mBoolSettings.value(rName, false);
}

int Configuration::getIntegerSetting(const QString &rName) const
{
    Q_ASSERT(mIntegerSettings.contains(rName));
    return mIntegerSettings.value(rName, -1);
}

double Configuration::getDoubleSetting(const QString &rName) const
{
    Q_ASSERT(mDoubleSettings.contains(rName));
    return mDoubleSettings.value(rName, -1);
}

void Configuration::setStringSetting(const QString &rName, const QString &rValue)
{
    Q_ASSERT(mStringSettings.contains(rName));
    mStringSettings.insert(rName, rValue);
    refreshQuickAccessVariables();
    saveToXml();
}

void Configuration::setBoolSetting(const QString &rName, const bool value)
{
    Q_ASSERT(mBoolSettings.contains(rName));
    mBoolSettings.insert(rName, value);
    refreshQuickAccessVariables();
    saveToXml();
}

void Configuration::setIntegerSetting(const QString &rName, const int value)
{
    Q_ASSERT(mIntegerSettings.contains(rName));
    mIntegerSettings.insert(rName, value);
    refreshQuickAccessVariables();
    saveToXml();
}

void Configuration::setDoubleSetting(const QString &rName, const double value)
{
    Q_ASSERT(mDoubleSettings.contains(rName));
    mDoubleSettings.insert(rName, value);
    refreshQuickAccessVariables();
    saveToXml();
}

//! @brief Utility function that loads user settings from dom element
void Configuration::loadUserSettings(QDomElement &rDomElement)
{
    // Load string settings
    for (auto it = mStringSettings.begin(); it != mStringSettings.end(); ++it)
    {
        it.value() = parseDomStringNode(rDomElement.firstChildElement(it.key()), it.value());
    }

    // Load bool settings
    for (auto it = mBoolSettings.begin(); it != mBoolSettings.end(); ++it)
    {
        it.value() = parseDomBooleanNode(rDomElement.firstChildElement(it.key()), it.value());
    }

    // Load integer settings
    for (auto it = mIntegerSettings.begin(); it != mIntegerSettings.end(); ++it)
    {
        it.value() = parseDomIntegerNode(rDomElement.firstChildElement(it.key()), it.value());
    }

    // Load double settings
    for (auto it = mDoubleSettings.begin(); it != mDoubleSettings.end(); ++it)
    {
        it.value() = parseDomValueNode(rDomElement.firstChildElement(it.key()), it.value());
    }

    // Load other settings
    if(!rDomElement.firstChildElement(CFG_PLOTGFXSIZE).isNull())
    {
        double width = mPlotGfxSize.width();
        double height = mPlotGfxSize.height();
        parseDomValueNode2(rDomElement.firstChildElement(CFG_PLOTGFXSIZE), width, height);
        mPlotGfxSize.setWidth(width);
        mPlotGfxSize.setHeight(height);
    }

    mBackgroundColor.setNamedColor(parseDomStringNode(rDomElement.firstChildElement(CFG_BACKGROUNDCOLOR), mBackgroundColor.name()));

    refreshQuickAccessVariables();
}



//! @brief Utility function that loads style settings from dom element
void Configuration::loadStyleSettings(QDomElement &rDomElement)
{
    QDomElement penElement = rDomElement.firstChildElement(CFG_PENSTYLE);
    while(!penElement.isNull())
    {
        QString type = penElement.attribute(CFG_TYPE);
        QString gfxType = penElement.attribute(CFG_GFXTYPE);
        QString situation = penElement.attribute(CFG_SITUATION);
        QString color = penElement.attribute(CFG_COLOR);
        double width = penElement.attribute(CFG_WIDTH).toDouble();
        Qt::PenStyle penstyle = Qt::PenStyle(penElement.attribute(HMF_STYLETAG).toInt());
        Qt::PenCapStyle capStyle = Qt::PenCapStyle(penElement.attribute(CFG_CAPSTYLE).toInt());
        QPen pen = QPen(QColor(color), width, penstyle, capStyle);

        ConnectorStyleEnumT style;
        if(type=="Power") style = PowerConnectorStyle;
        else if(type=="Signal") style = SignalConnectorStyle;
        else if(type=="Broken") style = BrokenConnectorStyle;
        /*if(type=="Undefined")*/else style = UndefinedConnectorStyle;

        if(!mPenStyles.contains(style))
        {
            QMap<QString, QMap<QString, QPen> > tempMap;
            mPenStyles.insert(style, tempMap);
        }
        if(!mPenStyles.find(style).value().contains(gfxType))
        {
            QMap<QString, QPen> tempMap;
            mPenStyles.find(style).value().insert(gfxType, tempMap);
        }
        mPenStyles.find(style).value().find(gfxType).value().insert(situation, pen);

        penElement = penElement.nextSiblingElement(CFG_PENSTYLE);
    }
    QDomElement paletteElement = rDomElement.firstChildElement("palette");
    if(!paletteElement.isNull())
    {
        double red, green, blue;
        QColor windowText, button, light, dark, mid, text, bright_text, base, window;
        parseRgbString(paletteElement.attribute("windowText"), red, green, blue);
        windowText.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("button"), red, green, blue);
        button.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("light"), red, green, blue);
        light.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("dark"), red, green, blue);
        dark.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("mid"), red, green, blue);
        mid.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("text"), red, green, blue);
        text.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("bright_text"), red, green, blue);
        bright_text.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("base"), red, green, blue);
        base.setRgb(red, green, blue);
        parseRgbString(paletteElement.attribute("window"), red, green, blue);
        window.setRgb(red, green, blue);
        mPalette = QPalette(windowText, button, light, dark, mid, text, bright_text, base, window);
    }
    QDomElement fontElement = rDomElement.firstChildElement("font");
    if(!fontElement.isNull())
    {
        mFont = QFont(fontElement.attribute("family"), fontElement.attribute("size").toInt());
        mFont.setStyleStrategy(QFont::PreferAntialias);
        mFont.setStyleHint(QFont::SansSerif);
    }
    QDomElement styleSheetElement = rDomElement.firstChildElement("stylesheet");
    if(!styleSheetElement.isNull())
    {
        mStyleSheet.append(styleSheetElement.text());
    }
}


//! @brief Utility function that loads selected default units from xml
void Configuration::loadUnitSettings(QDomElement &rDomElement)
{
    QDomElement xmlDefaultUnit = rDomElement.firstChildElement(CFG_DEFAULTUNIT);
    while (!xmlDefaultUnit.isNull())
    {
        mSelectedDefaultUnits.insert(xmlDefaultUnit.attribute(HMF_NAMETAG), xmlDefaultUnit.attribute(CFG_UNIT));
        xmlDefaultUnit = xmlDefaultUnit.nextSiblingElement(CFG_DEFAULTUNIT);
    }
}

//! @brief Utility function that loads unit scales from xml
void Configuration::loadUnitScales(QDomElement &rDomElement)
{
    QDomElement xmlQuantity = rDomElement.firstChildElement(CFG_QUANTITY);
    while (!xmlQuantity.isNull())
    {
        QString quantity = xmlQuantity.attribute(HMF_NAMETAG);
        QString siunit = xmlQuantity.attribute(CFG_SIUNIT);

        QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(quantity);
        if (qit == mUnitScales.end())
        {
            qit = mUnitScales.insert(quantity, QuantityUnitScale());
        }
        if (!siunit.isEmpty())
        {
            qit.value().siunit = siunit;
            qit.value().customScales.insert(siunit, UnitScale(quantity, siunit, "1.0"));
        }

        QDomElement xmlUnitscale = xmlQuantity.firstChildElement(CFG_UNITSCALE);
        while (!xmlUnitscale.isNull())
        {
            QString unit = xmlUnitscale.attribute(CFG_UNIT);
            qit.value().customScales.insert(unit, UnitScale(quantity, unit, xmlUnitscale.text()));
            xmlUnitscale = xmlUnitscale.nextSiblingElement(CFG_UNITSCALE);
        }

        xmlQuantity = xmlQuantity.nextSiblingElement(CFG_QUANTITY);
    }
}


//! @brief Utility function that loads library settings
void Configuration::loadLibrarySettings(QDomElement &rDomElement)
{
    QDomElement userLibElement = rDomElement.firstChildElement(XML_USERLIB);
    while (!userLibElement.isNull())
    {
        mUserLibs.append(QFileInfo(userLibElement.text()));
        QString typeStr = userLibElement.attribute(XML_LIBTYPE);
        if(typeStr == XML_LIBTYPE_EXTERNAL)
        {
            mUserLibTypes.append(ExternalLib);
        }
        else if(typeStr == XML_LIBTYPE_FMU)
        {
            mUserLibTypes.append(FmuLib);
        }
        else
        {
            mUserLibTypes.append(InternalLib);
        }
        userLibElement = userLibElement.nextSiblingElement((XML_USERLIB));
    }
}

void Configuration::loadModelicaFilesSettings(QDomElement &rDomElement)
{
    QDomElement modelicaFileElement = rDomElement.firstChildElement(XML_MODELICAFILE);
    while(!modelicaFileElement.isNull())
    {
        mModelicaFiles.append(QFileInfo(modelicaFileElement.text()));
        modelicaFileElement = modelicaFileElement.nextSiblingElement(XML_MODELICAFILE);
    }
}


//! @brief Utility function that loads model settings
void Configuration::loadModelSettings(QDomElement &rDomElement)
{
    QDomElement lastSessionElement = rDomElement.firstChildElement(XML_LASTSESSIONMODEL);
    while (!lastSessionElement.isNull())
    {
        mLastSessionModels.prepend(lastSessionElement.text());
        lastSessionElement = lastSessionElement.nextSiblingElement(XML_LASTSESSIONMODEL);
    }
    QDomElement recentModelElement = rDomElement.firstChildElement(XML_RECENTMODEL);
    while (!recentModelElement.isNull())
    {
        mRecentModels.prepend(recentModelElement.text());
        recentModelElement = recentModelElement.nextSiblingElement(XML_RECENTMODEL);
    }
    QDomElement recentGeneratorModelElement = rDomElement.firstChildElement(XML_RECENTGENERATORMODEL);
    while (!recentGeneratorModelElement.isNull())
    {
        mRecentGeneratorModels.prepend(recentGeneratorModelElement.text());
        recentGeneratorModelElement = recentGeneratorModelElement.nextSiblingElement(XML_RECENTGENERATORMODEL);
    }
}


//! @brief Utility function that loads script settings from dom elements
void Configuration::loadScriptSettings(QDomElement &rPythonElement, QDomElement &rHcomElement)
{
    if(!rPythonElement.isNull())
    {
        QDomElement lastScriptElement = rPythonElement.firstChildElement("lastscript");
        mLastPyScriptFile = lastScriptElement.attribute("file");
    }

    if(!rHcomElement.isNull())
    {
        QDomElement pwdElement = rHcomElement.firstChildElement(CFG_PWD);
        if(!pwdElement.isNull())
        {
            mHcomWorkingDirectory = pwdElement.text();
        }
        QDomElement commandElement = rHcomElement.firstChildElement(CFG_COMMAND);
        while(!commandElement.isNull())
        {
            mTerminalHistory.append(commandElement.text());
            commandElement = commandElement.nextSiblingElement(CFG_COMMAND);
        }
    }
}

void Configuration::refreshQuickAccessVariables()
{
    mInvertWheel = getBoolSetting(CFG_INVERTWHEEL);
    mShowPopupHelp = getBoolSetting(CFG_SHOWPOPUPHELP);
    mCacheLogData = getBoolSetting(CFG_CACHELOGDATA);
    mUseMulticore = getBoolSetting(CFG_MULTICORE);
    mProgressBarStep = getIntegerSetting(CFG_PROGRESSBARSTEP);
    mSnapping = getBoolSetting(CFG_SNAPPING);
    mGenerationLimit = getIntegerSetting(CFG_GENERATIONLIMIT);
}


//! @brief Returns whether or not the popup help shall be shown
bool Configuration::getShowPopupHelp() const
{
    return mShowPopupHelp;
}

//! @brief Returns whether or not invert wheel shall be used
bool Configuration::getInvertWheel() const
{
    return mInvertWheel;
}

bool Configuration::getCacheLogData() const
{
    return mCacheLogData;
}


//! @brief Returns whether or not multi-threading shall be used
bool Configuration::getUseMulticore() const
{
    return this->mUseMulticore;
}



//! @brief Returns the step size that shall be used in progress bar
int Configuration::getProgressBarStep() const
{
    return mProgressBarStep;
}

int Configuration::getGenerationLimit() const
{
    return mGenerationLimit;
}


//! @brief Returns the background color
QColor Configuration::getBackgroundColor() const
{
    return this->mBackgroundColor;
}


//! @brief Returns a list of paths to the user libraries that shall be loaded
QStringList Configuration::getUserLibs() const
{
    QStringList ret;
    Q_FOREACH(const QFileInfo &file, mUserLibs)
    {
        ret << file.absoluteFilePath();
    }

    return ret;
}


//! @brief Returns a list of the library types for all loaded libraries
QList<LibraryTypeEnumT> Configuration::getUserLibTypes() const
{
    return mUserLibTypes;
}


//! @brief Returns a list of all loaded Modelica files
QStringList Configuration::getModelicaFiles() const
{
    QStringList ret;
    Q_FOREACH(const QFileInfo &file, mModelicaFiles)
    {
        ret << file.absoluteFilePath();
    }

    return ret;
}


//! @brief Returns whether or not connector snapping shall be used
bool Configuration::getSnapping() const
{
    return mSnapping;
}



//! @brief Returns a list of paths to recently opened models
QStringList Configuration::getRecentModels() const
{
    return mRecentModels;
}


//! @brief Returns a list of paths to recently opened models in component generator
QStringList Configuration::getRecentGeneratorModels() const
{
    return mRecentGeneratorModels;
}



//! @brief Returns a list of paths to models that were open last time program was closed
QStringList Configuration::getLastSessionModels() const
{
    return mLastSessionModels;
}


//! @brief Returns the selected default unit for the specified physical quantity
//! @param key Name of the physical quantity (e.g. "Pressure" or "Velocity")
QString Configuration::getDefaultUnit(const QString &rPhysicalQuantity) const
{
    if(mSelectedDefaultUnits.contains(rPhysicalQuantity))
        return mSelectedDefaultUnits.find(rPhysicalQuantity).value();
    else
        return "";
}


//! @brief Returns a map with custom units (names and scale factor) for specified physical quantity
//! @param[in] rPhysicalQuantity Name of the physical quantity (e.g. "Pressure" or "Velocity")
//! @todo We should rewrite the code using this function to handle unitscale objects directly instead
QMap<QString, double> Configuration::getUnitScales(const QString &rPhysicalQuantity)
{
    QMap<QString, double> dummy;
    if(mUnitScales.contains(rPhysicalQuantity))
    {
        QMap<QString, UnitScale> &rMap = mUnitScales.find(rPhysicalQuantity).value().customScales;
        QMap<QString, UnitScale>::iterator it;
        for (it=rMap.begin(); it!=rMap.end(); ++it)
        {
            dummy.insert(it.value().mUnit, it.value().toDouble());
        }
    }
    return dummy;
}

void Configuration::getUnitScales(const QString &rQuantity, QList<UnitScale> &rUnitScales)
{
    QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(rQuantity);
    if (qit != mUnitScales.end())
    {
        rUnitScales = qit.value().customScales.values();
    }
}

bool Configuration::hasUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const
{
    if (mUnitScales.contains(rPhysicalQuantity))
    {
        return mUnitScales.find(rPhysicalQuantity).value().customScales.contains(rUnit);
    }
    return false;
}

//! @brief Returns unit scale for a particular physical quantity and unit
//! @note Returns 0 if nothing is found
double Configuration::getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const
{
    if (mUnitScales.contains(rPhysicalQuantity))
    {
        return mUnitScales.find(rPhysicalQuantity).value().customScales.value(rUnit,UnitScale("",0)).toDouble();
    }
    return 0;
}

void Configuration::getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit, UnitScale &rUnitScale) const
{
    rUnitScale.clear();
    if (mUnitScales.contains(rPhysicalQuantity))
    {
        rUnitScale = mUnitScales.find(rPhysicalQuantity).value().customScales.value(rUnit,UnitScale("",0));
    }
}

//! @brief Returns a list of the Physical Quantities associated with this unit (hopefully only one)
//! @param [in] rUnit The unit to lookup
QStringList Configuration::getQuantitiesForUnit(const QString &rUnit) const
{
    QStringList list;
    QMap< QString, QuantityUnitScale >::const_iterator it;
    for (it=mUnitScales.begin(); it!=mUnitScales.end(); ++it)
    {
        if (it.value().customScales.contains(rUnit))
        {
            list.append(it.key());
        }
    }
    return list;
}

QString Configuration::getBaseUnit(const QString &rQuantity)
{
    return  mUnitScales.value(rQuantity, QuantityUnitScale()).siunit;
}

bool Configuration::isRegisteredBaseUnit(const QString &rUnitName) const
{
    QMap< QString, QuantityUnitScale >::const_iterator it;
    for (it=mUnitScales.begin(); it!=mUnitScales.end(); ++it)
    {
        if (it.value().siunit == rUnitName)
        {
            return true;
        }
    }
    return false;
}

void Configuration::removeUnitScale(const QString &rQuantity, const QString &rUnit)
{
    QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(rQuantity);
    if (qit != mUnitScales.end())
    {
        if (rUnit != qit.value().siunit)
        {
            qit.value().customScales.remove(rUnit);
        }
    }
}

//! @brief Returns connector pen for specified connector type
//! @param style Style of connector (POWERCONNECTOR, SIGNALCONNECTOR or UNDEFINEDCONNECTOR)
//! @param gfxType Graphics type (User or Iso)
//! @param situation Defines when connector is used (Primary, Hovered, Active)
QPen Configuration::getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, QString situation) const
{
    QString gfxString;
    if(gfxType == ISOGraphics) { gfxString = "Iso"; }
    else { gfxString = "User"; }

    if(mPenStyles.contains(style))
    {
        if(mPenStyles.find(style).value().contains(gfxString))
        {
            if(mPenStyles.find(style).value().find(gfxString).value().contains(situation))
            {
                return mPenStyles.find(style).value().find(gfxString).value().find(situation).value();
            }
        }
    }
    return QPen(QColor("black"), 1, Qt::SolidLine, Qt::SquareCap);
}


QPalette Configuration::getPalette() const
{
    if(getBoolSetting(CFG_NATIVESTYLESHEET))
    {
        QMainWindow *dummy = new QMainWindow();
        QPalette dummyPalette = dummy->palette();
        delete(dummy);
        return dummyPalette;
    }
    else
        return mPalette;
}


QFont Configuration::getFont() const
{
    //! @note Embedded truetype fonts does not seem to work in Linux, so ignore them
#ifdef _WIN32
    return mFont;
#else
    return qApp->font();
#endif
}


//! @brief Returns the current style sheet
QString Configuration::getStyleSheet() const
{
    if(getBoolSetting(CFG_NATIVESTYLESHEET))
        return QString();
    else
        return mStyleSheet;
}


//! @brief Returns the last used script file
QString Configuration::getLastPyScriptFile() const
{
    return mLastPyScriptFile;
}

QStringList Configuration::getUnitQuantities() const
{
    return mUnitScales.keys();
}


//! @brief Returns the plot graphics export size setting
QSizeF Configuration::getPlotGfxSize()
{
    return mPlotGfxSize;
}


QStringList Configuration::getTerminalHistory()
{
    return mTerminalHistory;
}

QString Configuration::getHcomWorkingDirectory() const
{
    return mHcomWorkingDirectory;
}


int Configuration::getParallelAlgorithm()
{
    return mParallelAlgorighm;
}


//! @brief Set function for background color setting
//! @param value Desired color
void Configuration::setBackgroundColor(const QColor &value)
{
    mBackgroundColor = value;
    saveToXml();
}

//! @brief Adds a user library to the library list
//! @param value Path to the new library
//! @param type Type of library
void Configuration::addUserLib(const QString &value, LibraryTypeEnumT type)
{
    QFileInfo file(value);
    if(!mUserLibs.contains(file))
    {
        mUserLibs.append(file);
        mUserLibTypes.append(type);
        saveToXml();
    }
}


//! @brief Removes a user library from the library list
//! @param value Path to the library that is to be removed
void Configuration::removeUserLib(const QString &value)
{
    bool didRemove=false;
    QFileInfo file(value);
    for(int i=0; i<mUserLibs.size(); ++i)
    {
        if(mUserLibs[i].absoluteFilePath() == file.absoluteFilePath())
        {
            mUserLibs.removeAt(i);
            mUserLibTypes.removeAt(i);
            --i;
            didRemove=true;
        }
    }
    if (didRemove)
    {
        saveToXml();
    }
}


//! @brief Adds a new Modelica file to the configuration
void Configuration::addModelicaFile(const QString &value)
{
    QFileInfo file(value);
    if(!mModelicaFiles.contains(file))
    {
        mModelicaFiles.append(file);
    }
    saveToXml();
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
    saveToXml();

    emit recentModelsListChanged();
}

void Configuration::removeRecentModel(QString value)
{
    mRecentModels.removeAll(value);
    saveToXml();

    emit recentModelsListChanged();
}


//! @brief Adds a model to the list of recently opened models by component generator
//! @brief value Path to the model
void Configuration::addRecentGeneratorModel(QString value)
{
    mRecentGeneratorModels.removeAll(value);
    mRecentGeneratorModels.prepend(value);
    while(mRecentGeneratorModels.size() > 10)
    {
        mRecentGeneratorModels.pop_back();
    }
    saveToXml();
}



//! @brief Adds a model to the list of models that was open last time program closed
//! @param value Path to the model
void Configuration::addLastSessionModel(QString value)
{
    mLastSessionModels.append(value);
    saveToXml();
}


//! @brief Removes all last session models from the list
void Configuration::clearLastSessionModels()
{
    mLastSessionModels.clear();
    saveToXml();
}


//! @brief Sets the default unit for specified physical quantity
//! @param key Name of the physical quantity (e.g. "Pressure" or "Velocity")
//! @param value Name of the desired default unit
void Configuration::setDefaultUnit(QString key, QString value)
{
    this->mSelectedDefaultUnits.remove(key);
    this->mSelectedDefaultUnits.insert(key, value);
    saveToXml();
}


//! @brief Adds a new custom unit to the specified physical quantity
//! @param quantity Name of the physical quantity (e.g. "Pressure" or "Velocity")
//! @param unitname Name of the new unit
//! @param scale Scale factor from SI unit to the new unit
void Configuration::addCustomUnit(QString quantity, QString unitname, double scale)
{
    //! @todo what if quantity does not exist, should we add it? what about SI unit then, think its better to ahve a separat functin for that
    QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(quantity);
    if (qit != mUnitScales.end())
    {
        qit.value().customScales.insert(unitname, UnitScale(unitname, scale));
    }
    saveToXml();
}



void Configuration::setLastPyScriptFile(QString file)
{
    mLastPyScriptFile = file;
    saveToXml();
}



void Configuration::setPlotGfxSize(const QSizeF size)
{
    mPlotGfxSize = size;
}

void Configuration::storeTerminalHistory(QStringList value)
{
    mTerminalHistory = value;
}

void Configuration::setHcomWorkingDirectory(QString value)
{
    mHcomWorkingDirectory = value;
}

void Configuration::setParallelAlgorithm(int value)
{
    mParallelAlgorighm = value;
}
