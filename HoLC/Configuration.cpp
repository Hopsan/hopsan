//Qt includes
#include <QDomElement>
#include <QDesktopServices>
#include <QMessageBox>
//#include <QMap>
//#include <QAction>
//#include <QApplication>
#include <QMainWindow>

#include "Configuration.h"
#include "Utilities/XMLUtilities.h"

//! @class Configuration
//! @brief The Configuration class is used as a global XML-based storage for program configuration variables
//!
//! Use loadFromXml or saveToXml functions to read or write to holcconfig.xml. Use get, set, add and clear functions to access the data.
//!


Configuration::Configuration(QWidget *pParent)
{
    mpParent = pParent;
}


//! @brief Saves the current settings to holcconfig.xml
void Configuration::saveToXml()
{
    QString configPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/";

        //Write to holcconfig.xml
    QDomDocument domDocument;
    QDomElement configRoot = domDocument.createElement("holcconfig");
    configRoot.setAttribute("version", "1.0");
    domDocument.appendChild(configRoot);

    QDomElement settings = appendDomElement(configRoot,"settings");
    appendDomTextNode(settings, "projectpath", mProjectPath);

    appendRootXMLProcessingInstruction(domDocument);

    //Save to file
    if(!QDir(configPath).exists())
    {
        QDir().mkpath(configPath);
    }
    QFile xmlsettings(configPath + QString("holcconfig.xml"));
    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        QMessageBox::critical(mpParent, "Error", "Failed to open config file for writing: "+configPath+"holcconfig.xml");
        return;
    }
    QTextStream out(&xmlsettings);
    domDocument.save(out, 4);
}


//! @brief Updates all settings from holcconfig.xml
void Configuration::loadFromXml()
{
    //Read from holcdefaults.xml
    loadDefaultsFromXml();

    //Read from holcconfig.xml
    QFile file(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/" + QString("holcconfig.xml"));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(mpParent, "Warning", "Unable to find configuration file. Configuration file was recreated with default settings.");
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(mpParent, mpParent->tr("HoLC"),
                                 mpParent->tr("holcconfig.xml: Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != "holcconfig")
        {
            QMessageBox::information(mpParent, mpParent->tr("HoLC"),
                                     "The file is not an HoLC Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != holcconfig");
        }
        else
        {
            //Load user settings
            QDomElement settingsElement = configRoot.firstChildElement("settings");
            loadUserSettings(settingsElement);

            //Load style settings
            QDomElement styleElement = configRoot.firstChildElement("style");
            loadStyleSettings(styleElement);
        }
    }
    file.close();
}



void Configuration::loadDefaultsFromXml()
{
    QString execPath = qApp->applicationDirPath().append('/');

    //Read from holcdefaults.xml
    QFile file(execPath + "../holcdefaults");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(mpParent, mpParent->tr("Hopsan"),
                                 "Unable to read default configuration file. Please reinstall program.");

        qApp->quit();
    }
    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(mpParent, mpParent->tr("Hopsan"),
                                 mpParent->tr("holcdefaults: Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != "holcdefaults")
        {
            QMessageBox::information(mpParent, mpParent->tr("Hopsan"),
                                     "The file is not an Hopsan Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != holcdefaults");
        }
        else
        {
                //Load default user settings
            QDomElement settingsElement = configRoot.firstChildElement("settings");
            loadUserSettings(settingsElement);

                //Load default GUI style
            QDomElement styleElement = configRoot.firstChildElement("style");
            loadStyleSettings(styleElement);
        }
    }
    file.close();

    return;
}

//! @brief Utility function that loads user settings from dom element
void Configuration::loadUserSettings(QDomElement &rDomElement)
{
    mProjectPath = rDomElement.firstChildElement("projectpath").text();
}



//! @brief Utility function that loads style settings from dom element
void Configuration::loadStyleSettings(QDomElement &rDomElement)
{
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
    QDomElement styleSheetElement = rDomElement.firstChildElement("stylesheet");
    if(!styleSheetElement.isNull())
    {
        mStyleSheet.append(styleSheetElement.text());
    }
}


//! @brief Returns which library style to use
QString Configuration::getProjectPath()
{
    return mProjectPath;
}

void Configuration::setProjectPath(QString value)
{
    mProjectPath = value;
    saveToXml();
}


QPalette Configuration::getPalette()
{
    return mPalette;
}


//! @brief Returns the current style sheet
QString Configuration::getStyleSheet()
{
    return mStyleSheet;
}
