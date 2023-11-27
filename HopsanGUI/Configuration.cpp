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
#include "CoreAccess.h"

//Qt includes
#include <QDomElement>
#include <QMessageBox>
#include <QMap>
#include <QAction>
#include <QApplication>
#include <QMainWindow>

#include <functional>

namespace {

class SaveFileHelper
{
public:
    static QString currentDateTime()
    {
        return QDate::currentDate().toString("yyMMdd") + QTime::currentTime().toString("HHmmss");
    }

    static QString findFreeFilePath(const QString &desiredPath)
    {
        QString path = desiredPath;
        int ctr=1;
        while(QFile::exists(path))
        {
            path = appendToFileName(desiredPath, QString("_%1").arg(ctr));
        }
        return path;
    }

    static QString appendToFileName(const QFileInfo &fi, const QString &appendText)
    {
        // Note! Absolute path (rather than canonical path) is used here so that a path is returned even if the file does not yet exist
        return fi.absolutePath() + "/" + fi.baseName()+appendText + "." + fi.completeSuffix();
    }

    static QString appendToFileName(const QString &filePath, const QString &appendText)
    {
        QFileInfo fi(filePath);
        return appendToFileName(fi, appendText);
    }

    static void saveFileBackupExisting(const QString &filePath, std::function<void (QFile&)> saveFunction, const QString &backupText)
    {
        QString backup_path = appendToFileName(filePath, backupText);
        QString temp_path = findFreeFilePath(appendToFileName(filePath, "_temporary_"+currentDateTime()));

        // Save to a temp file first to avoid corrupt "real file" if Hopsan crash during the save operation
        QFile temp_file(temp_path);
        if (!temp_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            gpMessageHandler->addErrorMessage("Failed to open configuration file for writing: "+temp_path);
            return;
        }

        // Run the provided save-function to save to temporary file
        saveFunction(temp_file);
        temp_file.close();

        // Now lets backup the old config file
        // First remove the old backup file
        if (QFile::exists(backup_path))
        {
            QFile::remove(backup_path);
        }

        // Then rename the existing actual file so that it becomes the new backup
        if (QFile::exists(filePath))
        {
            QFile::rename(filePath, backup_path);
        }

        // Now rename the temp file to the actual file name
        QFile::rename(temp_path, filePath);
    }
};

}


//! @class Configuration
//! @brief The Configuration class is used as a global XML-based storage for program configuration variables
//!
//! Use loadFromXml or saveToXml functions to read or write to hopsanconfig.xml. Use get, set, add and clear functions to access the data.
//!


//! @brief Saves the current settings to hopsanconfig.xml
Configuration::Configuration()
{
    // Register configuration options
    registerSettings();
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


        QDomElement xmlUnitScales = appendDomElement(configRoot, CFG_UNITSETTINGS);
        QMap<QString, QuantityUnitScale >::iterator qit;
        for(qit = mUnitScales.begin(); qit != mUnitScales.end(); ++qit)
        {
            QDomElement xmlQuantity = appendDomElement(xmlUnitScales, CFG_QUANTITY);
            xmlQuantity.setAttribute(HMF_NAMETAG, qit.key());
            QString baseunit = qit.value().baseunit;
            if (!baseunit.isEmpty())
            {
                xmlQuantity.setAttribute(CFG_BASEUNIT, baseunit);
            }
            xmlQuantity.setAttribute(cfg::siunits::kg, qit.value().kg);
            xmlQuantity.setAttribute(cfg::siunits::m, qit.value().m);
            xmlQuantity.setAttribute(cfg::siunits::s, qit.value().s);
            xmlQuantity.setAttribute(cfg::siunits::A, qit.value().A);
            xmlQuantity.setAttribute(cfg::siunits::K, qit.value().K);
            xmlQuantity.setAttribute(cfg::siunits::mol, qit.value().mol);
            xmlQuantity.setAttribute(cfg::siunits::cd, qit.value().cd);
            xmlQuantity.setAttribute(cfg::siunits::rad, qit.value().rad);

            // Save the default selected display unit
            QMap<QString, QString>::iterator itdu = mSelectedDefaultUnits.find(qit.key());
            if (itdu!=mSelectedDefaultUnits.end())
            {
                xmlQuantity.setAttribute(CFG_DEFAULTDISPALYUNIT, itdu.value());
            }

            // Save all custom (non built in) unit conversions
            QMap<QString, UnitConverter>::iterator cuit;
            for(cuit = qit.value().customUnits.begin(); cuit != qit.value().customUnits.end(); ++cuit)
            {
                // Do not save custom units for the base unit (would be 1) and do not save those that are built-in
                if ( (cuit.key() != baseunit) &&
                     !qit.value().builtInUnitconversions.contains(cuit.key()) )
                {
                    QDomElement xmlUS = appendDomElement(xmlQuantity, CFG_UNIT);
                    xmlUS.setAttribute(CFG_NAME, cuit.key());
                    xmlUS.setAttribute(CFG_SCALE, cuit.value().mScale);
                    xmlUS.setAttribute(CFG_OFFSET, cuit.value().mOffset);
                }
            }
        }

        QDomElement hcom = appendDomElement(configRoot, CFG_HCOM);
        appendDomTextNode(hcom, CFG_PWD, mHcomWorkingDirectory);
        for(int i=0; i<mTerminalHistory.size(); ++i)
        {
            appendDomTextNode(hcom, CFG_COMMAND, mTerminalHistory.at(i));
        }

        appendRootXMLProcessingInstruction(domDocument);

        // Save to file

        QString config_file_path = gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml");

        // If destination does not exist, create it
        if(!QDir(gpDesktopHandler->getConfigPath()).exists())
        {
            QDir().mkpath(gpDesktopHandler->getConfigPath());
        }

        // Use save file helper to avoid corrupting existing destination file if program crash during save
        SaveFileHelper sfh;
        auto saveFunc = [&](QFile& outFile) {
            QByteArray temp_data;
            QTextStream temp_data_stream(&temp_data);
            domDocument.save(temp_data_stream, XMLINDENTATION);
            outFile.write(temp_data);
        };
        sfh.saveFileBackupExisting(config_file_path, saveFunc, "_backup");
    }
}


//! @brief Updates all settings from hopsanconfig.xml
void Configuration::loadFromXml()
{
    // Read from default configuration
    loadDefaultsFromXml();

    // Read from hopsanconfig.xml
    QString file_path = gpDesktopHandler->getConfigPath() + QString("hopsanconfig.xml");
    QString file_backup_path = SaveFileHelper::appendToFileName(file_path, "_backup");

    QFileInfo fi(file_path);
    QFile file(fi.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Note! We dont want to use the backup here, a missing file means reset
        gpMessageHandler->addWarningMessage(QString("Unable to find configuration file: %1, Configuration file was recreated with default settings.")
                                            .arg(fi.absoluteFilePath()));
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        gpMessageHandler->addErrorMessage(QString("Parse error in configuration file: %1, Error: %2, Line: %3, Column: %4")
                                          .arg(fi.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
        if (QFile::exists(file_backup_path))
        {
            gpMessageHandler->addWarningMessage("Found backup! Restoring backup config file (but preserving your broken file)");
            file.close();
            QString conf_file_broken = SaveFileHelper::findFreeFilePath(SaveFileHelper::appendToFileName(file_path, "_broken_"+SaveFileHelper::currentDateTime()));
            QFile::rename(file_path, conf_file_broken);
            // Restore from backup, the backup file must be remove, in case it also is corrupt (dont want to get stuck in a loop)
            QFile::rename(file_backup_path, file_path);
        }
        // Try again to load with restored configuration file
        loadFromXml();
        return;
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

            //Load model settings
            QDomElement modelsElement = configRoot.firstChildElement(XML_MODELS);
            loadModelSettings(modelsElement);

            //Load unit settings
            QDomElement unitscalesElement = configRoot.firstChildElement(CFG_UNITSETTINGS);
            loadUnitSettings(unitscalesElement, false);

            // ---------------------------------------------- used for backwards compatibility can be removed after 0.7.x is complete
            //! @todo deprecated
            QDomElement unitsElement = configRoot.firstChildElement(CFG_UNITS);
            QDomElement xmlDefaultUnit = unitsElement.firstChildElement("defaultunit");
            while (!xmlDefaultUnit.isNull())
            {
                mSelectedDefaultUnits.insert(xmlDefaultUnit.attribute(HMF_NAMETAG), xmlDefaultUnit.attribute(CFG_UNIT));
                xmlDefaultUnit = xmlDefaultUnit.nextSiblingElement("defaultunit");
            }
            // ----------------------------------------------

            //Load settings to HcomDockWidget in MainWindow
            QDomElement hcomElement = configRoot.firstChildElement(CFG_HCOM);
            loadScriptSettings(hcomElement);
        }
    }
    file.close();
}



void Configuration::loadDefaultsFromXml()
{
#ifdef Q_OS_OSX
    QFile file(gpDesktopHandler->getResourcesPath() + "hopsan-default-configuration.xml");
#else
    QFile file(gpDesktopHandler->getMainPath() + "hopsan-default-configuration.xml");
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
                                 gpMainWindowWidget->tr("hopsan-default-configuration.xml: Parse error at line %1, column %2:\n%3")
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

            // Load default user settings
            QDomElement settingsElement = configRoot.firstChildElement(CFG_SETTINGS);
            loadUserSettings(settingsElement);

            // Load default GUI style
            QDomElement styleElement = configRoot.firstChildElement(HMF_STYLETAG);
            loadStyleSettings(styleElement);

            // Load default units
            QDomElement unitscalesElement = configRoot.firstChildElement(CFG_UNITSETTINGS);
            loadUnitSettings(unitscalesElement, true);
        }
    }
    file.close();

    //Internal settings, not stored in xml (change later)
    mParallelAlgorighm = 0;

    return;
}

//! @brief Clear some settings
void Configuration::reset()
{
    mStringSettings.clear();
    mBoolSettings.clear();
    mIntegerSettings.clear();
    mDoubleSettings.clear();
    mUnitScales.clear();
    mSelectedDefaultUnits.clear();
    registerSettings();
    loadDefaultsFromXml();
    saveToXml();
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
    refreshIfDesktopPath(rName);
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
        refreshIfDesktopPath(it.key());
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


//! @brief Utility function that loads unit scales from xml
void Configuration::loadUnitSettings(QDomElement &rDomElement, bool tagAsBuiltIn)
{
    QDomElement xmlQuantity = rDomElement.firstChildElement(CFG_QUANTITY);
    while (!xmlQuantity.isNull())
    {
        QString quantity = xmlQuantity.attribute(HMF_NAMETAG);
        QString baseunit = xmlQuantity.attribute(CFG_BASEUNIT);
        int kg = parseAttributeInt(xmlQuantity, cfg::siunits::kg, 0);
        int m = parseAttributeInt(xmlQuantity, cfg::siunits::m, 0);
        int s = parseAttributeInt(xmlQuantity, cfg::siunits::s, 0);
        int A = parseAttributeInt(xmlQuantity, cfg::siunits::A, 0);
        int K = parseAttributeInt(xmlQuantity, cfg::siunits::K, 0);
        int mol = parseAttributeInt(xmlQuantity, cfg::siunits::mol, 0);
        int cd = parseAttributeInt(xmlQuantity, cfg::siunits::cd, 0);
        int rad = parseAttributeInt(xmlQuantity, cfg::siunits::rad, 0);
        QString deafdisplayunit = xmlQuantity.attribute(CFG_DEFAULTDISPALYUNIT, baseunit);
        if (!deafdisplayunit.isEmpty())
        {
            mSelectedDefaultUnits.insert(quantity, deafdisplayunit);
        }


        QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(quantity);
        if (qit == mUnitScales.end())
        {
            qit = mUnitScales.insert(quantity, QuantityUnitScale());
        }
        // insert if it defines a new base unit, but do not overwrite built in definitions
        if (!baseunit.isEmpty() && !qit.value().builtInUnitconversions.contains(baseunit))
        {
            qit.value().baseunit = baseunit;
            qit.value().kg = kg;
            qit.value().m = m;
            qit.value().s = s;
            qit.value().A = A;
            qit.value().K = K;
            qit.value().mol = mol;
            qit.value().cd = cd;
            qit.value().rad = rad;
            qit.value().customUnits.insert(baseunit, UnitConverter(quantity, baseunit, "1.0", ""));
            if (tagAsBuiltIn)
            {
                qit.value().builtInUnitconversions.append(baseunit);
            }
        }

        QDomElement xmlUnitscale = xmlQuantity.firstChildElement(CFG_UNIT);
        while (!xmlUnitscale.isNull())
        {
            QString unitname = xmlUnitscale.attribute(CFG_NAME);
            QString fromBaseToUnitExpr = xmlUnitscale.attribute(CFG_FROMBASEEXPR);
            QString toBaseUnitFromUnitExpr = xmlUnitscale.attribute(CFG_TOBBASEEXPR);
            // prevent overwriting existing built in unit conversions
            if (!qit.value().builtInUnitconversions.contains(unitname))
            {
                QString scale = xmlUnitscale.attribute(CFG_SCALE);
                QString offset = xmlUnitscale.attribute(CFG_OFFSET);
                // If scale or offset would be empty here, then result of evaluation would be "pi"
                if (!scale.isEmpty())
                {
                    scale = QString("%1").arg(evalWithNumHop(scale),0,'g',10);
                }
                if (!offset.isEmpty())
                {
                    offset = QString("%1").arg(evalWithNumHop(offset),0,'g',10);
                }
                if (!(toBaseUnitFromUnitExpr.isEmpty() || fromBaseToUnitExpr.isEmpty())) {
                    qit.value().customUnits.insert(unitname, UnitConverter(UnitConverter::Expression, quantity, unitname, toBaseUnitFromUnitExpr, fromBaseToUnitExpr));
                }
                else {
                    qit.value().customUnits.insert(unitname, UnitConverter(quantity, unitname, scale, offset));
                }
                if (tagAsBuiltIn)
                {
                    qit.value().builtInUnitconversions.append(unitname);
                }
            }
            xmlUnitscale = xmlUnitscale.nextSiblingElement(CFG_UNIT);
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
void Configuration::loadScriptSettings(QDomElement &rHcomElement)
{
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
    mZoomStep = getDoubleSetting(CFG_ZOOMSTEP);
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
    for(const QFileInfo &file : mUserLibs) {
        ret << file.absoluteFilePath();
    }

    return ret;
}


//! @brief Returns a list of the library types for all loaded libraries
QList<LibraryTypeEnumT> Configuration::getUserLibTypes() const
{
    return mUserLibTypes;
}


//! @brief Returns whether or not connector snapping shall be used
bool Configuration::getSnapping() const
{
    return mSnapping;
}

double Configuration::getZoomStep() const
{
    return mZoomStep;
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

//! @brief Populates a list with custom units (names and scale factor) for specified physical quantity.
//! @param[in] rQuantity Name of the physical quantity (e.g. "Pressure" or "Velocity").
//! @param[out] rUnitScales A list of UnitConverter objects.
void Configuration::getUnitScales(const QString &rQuantity, QList<UnitConverter> &rUnitScales)
{
    QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(rQuantity);
    if (qit != mUnitScales.end())
    {
        rUnitScales = qit.value().customUnits.values();
        std::sort(rUnitScales.begin(), rUnitScales.end(), UnitConverter::isScaleLesserThan);
    }
}

bool Configuration::hasUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const
{
    if (mUnitScales.contains(rPhysicalQuantity))
    {
        return mUnitScales.find(rPhysicalQuantity).value().customUnits.contains(rUnit);
    }
    return false;
}

//! @brief Returns unit scale for a particular physical quantity and unit
//! @note Returns 0 if nothing is found
double Configuration::getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const
{
    if (mUnitScales.contains(rPhysicalQuantity))
    {
        return mUnitScales.find(rPhysicalQuantity).value().customUnits.value(rUnit,UnitConverter("",0)).scaleToDouble();
    }
    return 0;
}

void Configuration::getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit, UnitConverter &rUnitScale) const
{
    rUnitScale = getUnitScaleUC(rPhysicalQuantity, rUnit);
}

UnitConverter Configuration::getUnitScaleUC(const QString &rPhysicalQuantity, const QString &rUnit) const
{
    if (mUnitScales.contains(rPhysicalQuantity))
    {
        return mUnitScales.find(rPhysicalQuantity).value().customUnits.value(rUnit, UnitConverter("",0));
    }
    return {};
}

//! @brief Returns a list of the Physical Quantities associated with this unit (hopefully only one)
//! @param [in] rUnit The unit to lookup
QStringList Configuration::getQuantitiesForUnit(const QString &rUnit) const
{
    QStringList list;
    QMap< QString, QuantityUnitScale >::const_iterator it;
    for (it=mUnitScales.begin(); it!=mUnitScales.end(); ++it)
    {
        if (it.value().customUnits.contains(rUnit))
        {
            list.append(it.key());
        }
    }
    return list;
}

QString Configuration::getBaseUnit(const QString &rQuantity)
{
    return  mUnitScales.value(rQuantity, QuantityUnitScale()).baseunit;
}

void Configuration::getBaseUnitSIExponents(const QString &rQuantity, int &rKg, int &rM, int &rS, int &rA, int &rK, int &rMol, int &rCd, int &rRad)
{
    QuantityUnitScale us = mUnitScales.value(rQuantity, QuantityUnitScale());
    rKg = us.kg;
    rM = us.m;
    rS = us.s;
    rA = us.A;
    rK = us.K;
    rMol = us.mol;
    rCd = us.cd;
    rRad = us.rad;
}

bool Configuration::isRegisteredBaseUnit(const QString &rUnitName) const
{
    QMap< QString, QuantityUnitScale >::const_iterator it;
    for (it=mUnitScales.begin(); it!=mUnitScales.end(); ++it)
    {
        if (it.value().baseunit == rUnitName)
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
        if (rUnit != qit.value().baseunit)
        {
            qit.value().customUnits.remove(rUnit);
        }
    }
}

bool Configuration::haveQuantity(const QString &rQuantity) const
{
    return mUnitScales.contains(rQuantity);
}

//! @brief Returns connector pen for specified connector type
//! @param style Style of connector (POWERCONNECTOR, SIGNALCONNECTOR or UNDEFINEDCONNECTOR)
//! @param gfxType Graphics type (User or ISO)
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
//! @param offset Scale factor from SI unit to the new unit
void Configuration::addCustomUnit(QString quantity, QString unitname, QString scale, QString offset)
{
    //! @todo what if quantity does not exist, should we add it? what about SI unit then, think its better to have a separate function for that
    QMap<QString, QuantityUnitScale>::iterator qit = mUnitScales.find(quantity);
    if (qit != mUnitScales.end())
    {
        qit.value().customUnits.insert(unitname, UnitConverter(quantity, unitname, scale, offset));
    }
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

void Configuration::registerSettings()
{
    // String settings
    mStringSettings.insert(CFG_REMOTEHOPSANADDRESS, "");
    mStringSettings.insert(CFG_REMOTEHOPSANADDRESSSERVERADDRESS, "");
    mStringSettings.insert(CFG_REMOTEHOPSANUSERIDENTIFICATION, "");
    mStringSettings.insert(CFG_PARAMETEREXPORTDIR, "");
    mStringSettings.insert(CFG_PARAMETERIMPORTDIR, "");
    mStringSettings.insert(CFG_LOADMODELDIR, gpDesktopHandler->getModelsPath());
    mStringSettings.insert(CFG_MODELGFXDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_PLOTDATADIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_PLOTGFXDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_SIMULINKEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_SUBSYSTEMDIR, gpDesktopHandler->getModelsPath());
    mStringSettings.insert(CFG_EXTERNALLIBDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_SCRIPTDIR, gpDesktopHandler->getScriptsPath());
    mStringSettings.insert(CFG_PLOTWINDOWDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_FMUIMPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_FMUEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_EXEEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_LABVIEWEXPORTDIR, gpDesktopHandler->getDocumentsPath());
    mStringSettings.insert(CFG_CUSTOMTEMPPATH, "");
    mStringSettings.insert(CFG_GCC64DIR, "");
    mStringSettings.insert(CFG_GCC32DIR, "");
#ifndef _WIN32
#ifdef HOPSANCOMPILED64BIT
    mStringSettings.insert(CFG_GCC64DIR, "/usr/bin");
#else
    mStringSettings.insert(CFG_GCC32DIR, "/usr/bin");
#endif
    //! @todo OSX ?
#endif
    mStringSettings.insert(CFG_PLOTGFXIMAGEFORMAT, "png");
    mStringSettings.insert(CFG_PLOTGFXDIMENSIONSUNIT, "px");
    mStringSettings.insert(cfg::paths::corelogfile, "");

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
#if defined(_WIN32)
    mBoolSettings.insert(CFG_NATIVESTYLESHEET, false);
#else
    mBoolSettings.insert(CFG_NATIVESTYLESHEET, true);
#endif
    mBoolSettings.insert(CFG_SHOWPOPUPHELP, true);
    mBoolSettings.insert(CFG_MULTICORE, false);
    mBoolSettings.insert(CFG_PROGRESSBAR, true);
    mBoolSettings.insert(CFG_SETPWDTOMWD, false);
    mBoolSettings.insert(CFG_SHOWLICENSEONSTARTUP, true);
    mBoolSettings.insert(CFG_CHECKFORDEVELOPMENTUPDATES, false);
    mBoolSettings.insert(CFG_LOGDURINGSIMULATION, false);
#ifdef _WIN32
    mBoolSettings.insert(CFG_PREFERINCLUDEDCOMPILER, true);
#else
    mBoolSettings.insert(CFG_PREFERINCLUDEDCOMPILER, false);
#endif

    // Integer settings
    mIntegerSettings.insert(CFG_LIBRARYSTYLE, 0);
    mIntegerSettings.insert(CFG_PROGRESSBARSTEP, 100);
    mIntegerSettings.insert(CFG_NUMBEROFTHREADS, 0);
    mIntegerSettings.insert(CFG_GENERATIONLIMIT, 100);
    mIntegerSettings.insert(CFG_PLOEXPORTVERSION, 1);
    mIntegerSettings.insert(CFG_REMOTESHORTTIMEOUT, 5);
    mIntegerSettings.insert(CFG_REMOTELONGTIMEOUT, 30);
    mIntegerSettings.insert(CFG_LOGSTEPS, 100);

    // Double settings
    mDoubleSettings.insert(CFG_PLOTGFXDPI, 96.0);
    mDoubleSettings.insert(CFG_ZOOMSTEP, 15.0);
}

void Configuration::setParallelAlgorithm(int value)
{
    mParallelAlgorighm = value;
}

QString Configuration::getGCCPath() const
{
    if (getBoolSetting(CFG_PREFERINCLUDEDCOMPILER))
    {
        QString compilerpath = gpDesktopHandler->getIncludedCompilerPath();
        if (!compilerpath.isEmpty())
        {
            return compilerpath;
        }
    }

#ifdef HOPSANCOMPILED64BIT
    return getStringSetting(CFG_GCC64DIR);
#else
    return getStringSetting(CFG_GCC32DIR);
#endif
}

void Configuration::refreshIfDesktopPath(const QString &cfgKey)
{
    if (cfgKey == CFG_CUSTOMTEMPPATH)
    {
        gpDesktopHandler->setCustomTempPath(mStringSettings[CFG_CUSTOMTEMPPATH]);
    }
}

QString Configuration::getCompilerPath(const ArchitectureEnumT arch)
{
    int a;
    QString cfgkey;

    if(arch == ArchitectureEnumT::x86)
    {
        a = 32;
        cfgkey = CFG_GCC32DIR;
    }
    else //x64
    {
        a = 64;
        cfgkey = CFG_GCC64DIR;
    }

    QString compilerPath;
    if (getBoolSetting(CFG_PREFERINCLUDEDCOMPILER))
    {
        compilerPath = gpDesktopHandler->getIncludedCompilerPath(a);
    }
    if (compilerPath.isEmpty())
    {
        compilerPath = getStringSetting(cfgkey);
    }
    return compilerPath;
}
