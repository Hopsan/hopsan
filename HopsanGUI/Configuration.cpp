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
#include "Utilities/GUIUtilities.h"
#include "MessageHandler.h"
#include "Widgets/PyDockWidget.h"
//! @todo this config object should not need to include all those other things, rather they should prepare their data and sent it into the config object, or something similar

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
void Configuration::saveToXml()
{
        //Write to hopsanconfig.xml
    QDomDocument domDocument;
    QDomElement configRoot = domDocument.createElement(CFG_HOPSANCONFIG);
    configRoot.setAttribute(HMF_HOPSANGUIVERSIONTAG, HOPSANGUIVERSION);
    domDocument.appendChild(configRoot);

    QDomElement settings = appendDomElement(configRoot,CFG_SETTINGS);
    appendDomIntegerNode(settings, CFG_LIBRARYSTYLE, mLibraryStyle);
    appendDomBooleanNode(settings, CFG_SHOWPOPUPHELP, mShowPopupHelp);
    appendDomBooleanNode(settings, CFG_NATIVESTYLESHEET, mUseNativeStyleSheet);
    appendDomTextNode(settings, CFG_BACKGROUNDCOLOR, mBackgroundColor.name());
    appendDomBooleanNode(settings, CFG_ANTIALIASING, mAntiAliasing);
    appendDomBooleanNode(settings, CFG_INVERTWHEEL, mInvertWheel);
    appendDomBooleanNode(settings, CFG_SNAPPING, mSnapping);
    appendDomBooleanNode(settings, CFG_PROGRESSBAR, mEnableProgressBar);
    appendDomIntegerNode(settings, CFG_PROGRESSBARSTEP, mProgressBarStep);
    appendDomBooleanNode(settings, CFG_MULTICORE, mUseMulticore);
    appendDomIntegerNode(settings, CFG_NUMBEROFTHREADS, mNumberOfThreads);
    appendDomBooleanNode(settings, CFG_TOGGLENAMESBUTTONCHECKED, gpToggleNamesAction->isChecked());
    appendDomBooleanNode(settings, CFG_TOGGLEPORTSBUTTONCHECKED, gpTogglePortsAction->isChecked());
    appendDomBooleanNode(settings, CFG_GROUPMESSAGESBYTAG, mGroupMessagesByTag);
    appendDomIntegerNode(settings, CFG_GENERATIONLIMIT, mGenerationLimit);
    appendDomBooleanNode(settings, CFG_AUTOLIMITGENERATIONS, mAutoLimitLogDataGenerations);
    appendDomBooleanNode(settings, CFG_CACHELOGDATA, mCacheLogData);
    appendDomBooleanNode(settings, CFG_AUTOBACKUP, mAutoBackup);
    appendDomBooleanNode(settings, CFG_SETPWDTOMWD, mSetPwdToMwd);
    appendDomBooleanNode(settings, CFG_SETPWDTOMWD, mSetPwdToMwd);
    appendDomBooleanNode(settings, CFG_PLOTWINDOWSONTOP, mPlotWindowsOnTop);
    appendDomTextNode(settings, CFG_LOADMODELDIR, mLoadModelDir);
    appendDomTextNode(settings, CFG_MODELGFXDIR, mModelGfxDir);
    appendDomTextNode(settings, CFG_PLOTDATADIR, mPlotDataDir);
    appendDomTextNode(settings, CFG_PLOTGFXDIR, mPlotGfxDir);
    appendDomTextNode(settings, CFG_SIMULINKEXPORTDIR, mSimulinkExportDir);
    appendDomTextNode(settings, CFG_SUBSYSTEMDIR, mSubsystemDir);
    appendDomTextNode(settings, CFG_MODELICAMODELSDIR, mModelicaModelsDir);
    appendDomTextNode(settings, CFG_EXTERNALLIBDIR, mExternalLibDir);
    appendDomTextNode(settings, CFG_SCRIPTDIR, mScriptDir);
    appendDomTextNode(settings, CFG_PLOTWINDOWDIR, mPlotWindowDir);
    appendDomTextNode(settings, CFG_FMUIMPORTDIR, mFmuImportDir);
    appendDomTextNode(settings, CFG_FMUEXPORTDIR, mFmuExportDir);
    appendDomTextNode(settings, CFG_LABVIEWEXPORTDIR, mLabViewExportDir);
    appendDomTextNode(settings, CFG_GCC32DIR, mGcc32Dir);
    appendDomTextNode(settings, CFG_GCC64DIR, mGcc64Dir);
    appendDomIntegerNode(settings, CFG_PLOEXPORTVERSION, mPLOExportVersion);
    appendDomBooleanNode(settings, CFG_SHOWHIDDENNODEDATAVARIABLES, mShowHiddenNodeDataVariables);
    appendDomTextNode(settings, CFG_PLOTGFXIMAGEFORMAT, mPlotGfxImageFormat);
    appendDomTextNode(settings, CFG_PLOTGFXDIMENSIONSUNIT, mPlotGfxDimensionsUnit);
    appendDomIntegerNode(settings, CFG_PLOTGFXDPI, mPlotGfxDPI);
    appendDomValueNode2(settings, CFG_PLOTGFXSIZE, mPlotGfxSize.width(), mPlotGfxSize.height());
    appendDomBooleanNode(settings, CFG_PLOTGFXKEEPASPECT, mPlotGfxKeepAspect);
    appendDomBooleanNode(settings, CFG_PLOTGFXUSESCREENSIZE, mPlotGfxUseScreenSize);

    QDomElement style = appendDomElement(configRoot, HMF_STYLETAG);

    QMap<ConnectorStyleEnumT, QMap<QString, QMap<QString, QPen> > >::iterator it1;
    QMap<QString, QMap<QString, QPen> >::iterator it2;
    QMap<QString, QPen>::iterator it3;

    for(it1 = mPenStyles.begin(); it1 != mPenStyles.end(); ++it1)
    {
        for(it2 = it1.value().begin(); it2 != it1.value().end(); ++it2)
        {
            for(it3 = it2.value().begin(); it3 != it2.value().end(); ++it3)
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
        appendDomTextNode(libs, XML_USERLIB, mUserLibs.at(i).absoluteFilePath());
        QString typeStr = XML_LIBTYPE_INTERNAL;
        if(mUserLibTypes.at(i) == ExternalLib)
        {
            typeStr = XML_LIBTYPE_EXTERNAL;
        }
        else if(mUserLibTypes.at(i) == FmuLib)
        {
            typeStr = XML_LIBTYPE_FMU;
        }
        libs.lastChildElement(XML_USERLIB).setAttribute(XML_LIBTYPE, typeStr);
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

    //-------------------------------------------------------------------------------------
    //! @deprecated This code should be removed in the future 20140414 /Peter
    appendComment(units, "Note! These customunit tags are deprecated and should no longer be used! (Used for backwards compatibility)");
    QMap<QString, QuantityUnitScale >::iterator itpcu;
    QMap<QString, UnitScale>::iterator itcu;
    for(itpcu = mUnitScales.begin(); itpcu != mUnitScales.end(); ++itpcu)
    {
        for(itcu = itpcu.value().customScales.begin(); itcu != itpcu.value().customScales.end(); ++itcu)
        {
            QDomElement xmlTemp = appendDomElement(units, CFG_CUSTOMUNIT);
            xmlTemp.setAttribute(CFG_NAME, itpcu.key());
            xmlTemp.setAttribute(CFG_UNIT, itcu.key());
            setQrealAttribute(xmlTemp, CFG_SCALE, itcu.value().mScale.toDouble());
        }
    }
    appendComment(units, "Note! These customunit tags are deprecated and should no longer be used! (Used for backwards compatibility)");
    //-------------------------------------------------------------------------------------

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
    QFile file(gpDesktopHandler->getMainPath() + "hopsandefaults");
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

//! @brief Utility function that loads user settings from dom element
void Configuration::loadUserSettings(QDomElement &rDomElement)
{
    mLibraryStyle = parseDomIntegerNode(rDomElement.firstChildElement(CFG_LIBRARYSTYLE), mLibraryStyle);
    mPLOExportVersion = parseDomIntegerNode(rDomElement.firstChildElement(CFG_PLOEXPORTVERSION), mPLOExportVersion);

    if(!rDomElement.firstChildElement(CFG_SHOWHIDDENNODEDATAVARIABLES).isNull())
        mShowHiddenNodeDataVariables = parseDomBooleanNode(rDomElement.firstChildElement(CFG_SHOWHIDDENNODEDATAVARIABLES), mShowHiddenNodeDataVariables);
    if(!rDomElement.firstChildElement(CFG_SHOWPOPUPHELP).isNull())
        mShowPopupHelp = parseDomBooleanNode(rDomElement.firstChildElement(CFG_SHOWPOPUPHELP), mShowPopupHelp);
    if(!rDomElement.firstChildElement(CFG_NATIVESTYLESHEET).isNull())
        mUseNativeStyleSheet = parseDomBooleanNode(rDomElement.firstChildElement(CFG_NATIVESTYLESHEET), mUseNativeStyleSheet);
    if(!rDomElement.firstChildElement(CFG_ANTIALIASING).isNull())
        mAntiAliasing = parseDomBooleanNode(rDomElement.firstChildElement(CFG_ANTIALIASING), mAntiAliasing);
    if(!rDomElement.firstChildElement(CFG_INVERTWHEEL).isNull())
        mInvertWheel = parseDomBooleanNode(rDomElement.firstChildElement(CFG_INVERTWHEEL), mInvertWheel);
    if(!rDomElement.firstChildElement(CFG_SNAPPING).isNull())
        mSnapping = parseDomBooleanNode(rDomElement.firstChildElement(CFG_SNAPPING), mSnapping);
    if(!rDomElement.firstChildElement(CFG_SETPWDTOMWD).isNull())
        mSetPwdToMwd = parseDomBooleanNode(rDomElement.firstChildElement(CFG_SETPWDTOMWD), mSetPwdToMwd);
    if(!rDomElement.firstChildElement(CFG_PLOTWINDOWSONTOP).isNull())
        mPlotWindowsOnTop = parseDomBooleanNode(rDomElement.firstChildElement(CFG_PLOTWINDOWSONTOP), mPlotWindowsOnTop);
    if(!rDomElement.firstChildElement(CFG_PROGRESSBAR).isNull())
        mEnableProgressBar = parseDomBooleanNode(rDomElement.firstChildElement(CFG_PROGRESSBAR), mEnableProgressBar);
    if(!rDomElement.firstChildElement(CFG_PROGRESSBARSTEP).isNull())
        mProgressBarStep = parseDomIntegerNode(rDomElement.firstChildElement(CFG_PROGRESSBARSTEP), mProgressBarStep);
    if(!rDomElement.firstChildElement(CFG_MULTICORE).isNull())
        mUseMulticore = parseDomBooleanNode(rDomElement.firstChildElement(CFG_MULTICORE), mUseMulticore);
    if(!rDomElement.firstChildElement(CFG_NUMBEROFTHREADS).isNull())
        mNumberOfThreads = parseDomIntegerNode(rDomElement.firstChildElement(CFG_NUMBEROFTHREADS), mNumberOfThreads);
    if(!rDomElement.firstChildElement(CFG_TOGGLENAMESBUTTONCHECKED).isNull())
        mToggleNamesButtonCheckedLastSession = parseDomBooleanNode(rDomElement.firstChildElement(CFG_TOGGLENAMESBUTTONCHECKED), mToggleNamesButtonCheckedLastSession);
    if(!rDomElement.firstChildElement(CFG_TOGGLEPORTSBUTTONCHECKED).isNull())
        mTogglePortsButtonCheckedLastSession = parseDomBooleanNode(rDomElement.firstChildElement(CFG_TOGGLEPORTSBUTTONCHECKED), mTogglePortsButtonCheckedLastSession);
    if(!rDomElement.firstChildElement(CFG_GROUPMESSAGESBYTAG).isNull())
        mGroupMessagesByTag = parseDomBooleanNode(rDomElement.firstChildElement(CFG_GROUPMESSAGESBYTAG), mGroupMessagesByTag);
    if(!rDomElement.firstChildElement(CFG_GENERATIONLIMIT).isNull())
        mGenerationLimit = parseDomIntegerNode(rDomElement.firstChildElement(CFG_GENERATIONLIMIT), mGenerationLimit);
    if(!rDomElement.firstChildElement(CFG_CACHELOGDATA).isNull())
        mCacheLogData = parseDomBooleanNode(rDomElement.firstChildElement(CFG_CACHELOGDATA), mCacheLogData);
    if(!rDomElement.firstChildElement(CFG_AUTOBACKUP).isNull())
        mCacheLogData = parseDomBooleanNode(rDomElement.firstChildElement(CFG_AUTOBACKUP), mAutoBackup);
    if(!rDomElement.firstChildElement(CFG_AUTOLIMITGENERATIONS).isNull())
        mAutoLimitLogDataGenerations = parseDomBooleanNode(rDomElement.firstChildElement(CFG_AUTOLIMITGENERATIONS), mAutoLimitLogDataGenerations);
    if(!rDomElement.firstChildElement(CFG_PLOTGFXIMAGEFORMAT).isNull())
        mPlotGfxImageFormat = rDomElement.firstChildElement(CFG_PLOTGFXIMAGEFORMAT).text();
    if(!rDomElement.firstChildElement(CFG_PLOTGFXDIMENSIONSUNIT).isNull())
        mPlotGfxDimensionsUnit = rDomElement.firstChildElement(CFG_PLOTGFXDIMENSIONSUNIT).text();
    if(!rDomElement.firstChildElement(CFG_PLOTGFXDPI).isNull())
        mPlotGfxDPI = parseDomIntegerNode(rDomElement.firstChildElement(CFG_PLOTGFXDPI), mPlotGfxDPI);
    if(!rDomElement.firstChildElement(CFG_PLOTGFXSIZE).isNull())
    {
        double width = mPlotGfxSize.width();
        double height = mPlotGfxSize.height();
        parseDomValueNode2(rDomElement.firstChildElement(CFG_PLOTGFXSIZE), width, height);
        mPlotGfxSize.setWidth(width);
        mPlotGfxSize.setHeight(height);
    }
    if(!rDomElement.firstChildElement(CFG_PLOTGFXKEEPASPECT).isNull())
        mPlotGfxKeepAspect = parseDomBooleanNode(rDomElement.firstChildElement(CFG_PLOTGFXKEEPASPECT),mPlotGfxKeepAspect);
    if(!rDomElement.firstChildElement(CFG_PLOTGFXUSESCREENSIZE).isNull())
        mPlotGfxUseScreenSize = parseDomBooleanNode(rDomElement.firstChildElement(CFG_PLOTGFXUSESCREENSIZE),mPlotGfxUseScreenSize);

    if(!rDomElement.firstChildElement(CFG_BACKGROUNDCOLOR).isNull())
        mBackgroundColor.setNamedColor(rDomElement.firstChildElement(CFG_BACKGROUNDCOLOR).text());
    if(!rDomElement.firstChildElement(CFG_LOADMODELDIR).isNull())
        mLoadModelDir = rDomElement.firstChildElement(CFG_LOADMODELDIR).text();
    if(!rDomElement.firstChildElement(CFG_MODELGFXDIR).isNull())
        mModelGfxDir = rDomElement.firstChildElement(CFG_MODELGFXDIR).text();
    if(!rDomElement.firstChildElement(CFG_PLOTDATADIR).isNull())
        mPlotDataDir = rDomElement.firstChildElement(CFG_PLOTDATADIR).text();
    if(!rDomElement.firstChildElement(CFG_PLOTGFXDIR).isNull())
        mPlotGfxDir = rDomElement.firstChildElement(CFG_PLOTGFXDIR).text();
    if(!rDomElement.firstChildElement(CFG_SIMULINKEXPORTDIR).isNull())
        mSimulinkExportDir = rDomElement.firstChildElement(CFG_SIMULINKEXPORTDIR).text();
    if(!rDomElement.firstChildElement(CFG_SUBSYSTEMDIR).isNull())
        mSubsystemDir = rDomElement.firstChildElement(CFG_SUBSYSTEMDIR).text();
    if(!rDomElement.firstChildElement(CFG_MODELICAMODELSDIR).isNull())
        mModelicaModelsDir = rDomElement.firstChildElement(CFG_MODELICAMODELSDIR).text();
    if(!rDomElement.firstChildElement(CFG_EXTERNALLIBDIR).isNull())
        mExternalLibDir = rDomElement.firstChildElement(CFG_EXTERNALLIBDIR).text();
    if(!rDomElement.firstChildElement(CFG_SCRIPTDIR).isNull())
        mScriptDir = rDomElement.firstChildElement(CFG_SCRIPTDIR).text();
    if(!rDomElement.firstChildElement(CFG_PLOTWINDOWDIR).isNull())
        mPlotWindowDir = rDomElement.firstChildElement(CFG_PLOTWINDOWDIR).text();
    if(!rDomElement.firstChildElement("fmudir").isNull())
        mFmuImportDir = rDomElement.firstChildElement("fmudir").text();
    if(!rDomElement.firstChildElement(CFG_FMUIMPORTDIR).isNull())
        mFmuImportDir = rDomElement.firstChildElement(CFG_FMUIMPORTDIR).text();
    if(!rDomElement.firstChildElement("fmuexportdir").isNull())
        mFmuExportDir = rDomElement.firstChildElement("fmuexportdir").text();
    if(!rDomElement.firstChildElement(CFG_LABVIEWEXPORTDIR).isNull())
        mLabViewExportDir = rDomElement.firstChildElement(CFG_LABVIEWEXPORTDIR).text();
    if(!rDomElement.firstChildElement(CFG_GCC32DIR).isNull())
        mGcc32Dir = rDomElement.firstChildElement(CFG_GCC32DIR).text();
    if(!rDomElement.firstChildElement(CFG_GCC64DIR).isNull())
        mGcc64Dir = rDomElement.firstChildElement(CFG_GCC64DIR).text();
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

    //! @deprecated This code is only used for backwards compatibility, remove it in the future /Peter 20140414
    QDomElement customUnitElement = rDomElement.firstChildElement(CFG_CUSTOMUNIT);
    while (!customUnitElement.isNull())
    {
        QString physicalQuantity = customUnitElement.attribute(HMF_NAMETAG);
        QString unitName = customUnitElement.attribute(CFG_UNIT);
        QString unitScale = customUnitElement.attribute(CFG_SCALE);
        if (!mUnitScales.contains(physicalQuantity))
        {
            mUnitScales.insert(physicalQuantity, QuantityUnitScale());
        }
        if(!mUnitScales.value(physicalQuantity).customScales.contains(unitName))
        {
            mUnitScales.find(physicalQuantity).value().customScales.insert(unitName, UnitScale(unitName, unitScale));
        }
        else
        {
            // Compre with old deprected settings to make sure scale is same as already loaded from new xml format
            QString currentVal = mUnitScales.find(physicalQuantity).value().customScales.value(unitName).mScale;
            if (currentVal != unitScale)
            {
                qDebug() << "Warning unit scales unequal for: " << physicalQuantity+":"+unitName << " " << currentVal << " != " << unitScale;
            }
        }
        customUnitElement = customUnitElement.nextSiblingElement(CFG_CUSTOMUNIT);
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
            qit.value().customScales.insert(siunit, UnitScale(siunit, "1.0"));
        }

        QDomElement xmlUnitscale = xmlQuantity.firstChildElement(CFG_UNITSCALE);
        while (!xmlUnitscale.isNull())
        {
            QString unit = xmlUnitscale.attribute(CFG_UNIT);
            qit.value().customScales.insert(unit, UnitScale(unit, xmlUnitscale.text()));
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


//! @brief Returns which library style to use
int Configuration::getLibraryStyle() const
{
    return this->mLibraryStyle;
}


//! @brief Returns whether or not the popup help shall be shown
bool Configuration::getShowPopupHelp() const
{
    return this->mShowPopupHelp;
}


//! @brief Returns whether or not the welcome dialog shall be shown
bool Configuration::getUseNativeStyleSheet() const
{
    return this->mUseNativeStyleSheet;
}


//! @brief Returns whether or not invert wheel shall be used
bool Configuration::getInvertWheel() const
{
    return this->mInvertWheel;
}


//! @brief Returns whether or not multi-threading shall be used
bool Configuration::getUseMulticore() const
{
    return this->mUseMulticore;
}


//! @brief Returns number of simulation threads that shall be used
int Configuration::getNumberOfThreads() const
{
    return this->mNumberOfThreads;
}


//! @brief Returns whether or not the toggle names button was checked at the end of last session
bool Configuration::getToggleNamesButtonCheckedLastSession() const
{
    return this->mToggleNamesButtonCheckedLastSession;
}


//! @brief Returns whether or not the toggle ports button was checked at the end of last session
bool Configuration::getTogglePortsButtonCheckedLastSession() const
{
    return this->mTogglePortsButtonCheckedLastSession;
}


//! @brief Returns the step size that shall be used in progress bar
int Configuration::getProgressBarStep() const
{
    return this->mProgressBarStep;
}


//! @brief Returns whether or not the progress bar shall be displayed during simulation
bool Configuration::getEnableProgressBar() const
{
    return this->mEnableProgressBar;
}


//! @brief Returns the background color
QColor Configuration::getBackgroundColor() const
{
    return this->mBackgroundColor;
}


//! @brief Returns whether or not anti-aliasing shall be used
bool Configuration::getAntiAliasing() const
{
    return this->mAntiAliasing;
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
    return this->mSnapping;
}

//! @brief Returns whether ot not PWD should automatically be set to MWD in HCOM
bool Configuration::getAutoSetPwdToMwd() const
{
    return mSetPwdToMwd;
}

bool Configuration::getPlotWindowsOnTop() const
{
    return mPlotWindowsOnTop;
}


//! @brief Returns a list of paths to recently opened models
QStringList Configuration::getRecentModels() const
{
    return this->mRecentModels;
}


//! @brief Returns a list of paths to recently opened models in component generator
QStringList Configuration::getRecentGeneratorModels() const
{
    return this->mRecentGeneratorModels;
}



//! @brief Returns a list of paths to models that were open last time program was closed
QStringList Configuration::getLastSessionModels() const
{
    return this->mLastSessionModels;
}


//! @brief Returns the selected default unit for the specified physical quantity
//! @param key Name of the physical quantity (e.g. "Pressure" or "Velocity")
QString Configuration::getDefaultUnit(const QString &rPhysicalQuantity) const
{
    if(mSelectedDefaultUnits.contains(rPhysicalQuantity))
        return this->mSelectedDefaultUnits.find(rPhysicalQuantity).value();
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
        rUnitScale.mPhysicalQuantity = rPhysicalQuantity;
    }
}

//! @brief Returns a list of the Physical Quantities associated with this unit (hopefully only one)
//! @param [in] rUnit The unit to lookup
QStringList Configuration::getPhysicalQuantitiesForUnit(const QString &rUnit) const
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

QString Configuration::getSIUnit(const QString &rQuantity)
{
    return  mUnitScales.value(rQuantity, QuantityUnitScale()).siunit;
}

bool Configuration::isRegisteredSIUnit(const QString &rUnitName) const
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

int Configuration::getPLOExportVersion() const
{
    return mPLOExportVersion;
}

bool Configuration::getShowHiddenNodeDataVariables() const
{
    return mShowHiddenNodeDataVariables;
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
    if(this->mUseNativeStyleSheet)
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
#ifdef WIN32
    return mFont;
#else
    return qApp->font();
#endif
}


//! @brief Returns the current style sheet
QString Configuration::getStyleSheet() const
{
    if(mUseNativeStyleSheet)
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


//! @brief Returns the group message by tag setting
bool Configuration::getGroupMessagesByTag()
{
    return mGroupMessagesByTag;
}


//! @brief Returns the limit setting for plot generations
int Configuration::getGenerationLimit() const
{
    return mGenerationLimit;
}

//! @brief Returns the cache log data setting
bool Configuration::getCacheLogData() const
{
    return mCacheLogData;
}

//! @brief Returns the auto backup setting
bool Configuration::getAutoBackup() const
{
    return mAutoBackup;
}

//! @brief Returns the auto limit log data generations setting
bool Configuration::getAutoLimitLogDataGenerations()
{
    return mAutoLimitLogDataGenerations;
}

//! @brief Returns the plot graphics export image format setting
QString Configuration::getPlotGfxImageFormat()
{
    return mPlotGfxImageFormat;
}

//! @brief Returns the plot graphics export dimension unit setting
QString Configuration::getPlotGfxDimensionsUnit()
{
    return mPlotGfxDimensionsUnit;
}

//! @brief Returns the plot graphics export DPI setting
double Configuration::getPlotGfxDPI()
{
    return mPlotGfxDPI;
}

//! @brief Returns the plot graphics export size setting
QSizeF Configuration::getPlotGfxSize()
{
    return mPlotGfxSize;
}

//! @brief Returns the plot graphics export keep aspect ratio setting
bool Configuration::getPlotGfxKeepAspect()
{
    return mPlotGfxKeepAspect;
}

//! @brief Returns the plot graphics export use screen size setting
bool Configuration::getPlotGfxUseScreenSize()
{
    return mPlotGfxUseScreenSize;
}




//! @brief Returns the last used model directory
QString Configuration::getLoadModelDir()
{
    if(mLoadModelDir.isEmpty())
    {
        return gpDesktopHandler->getModelsPath();
    }
    return mLoadModelDir;
}


//! @brief Returns the last used model graphics export directory
QString Configuration::getModelGfxDir()
{
    if(mModelGfxDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mModelGfxDir;
}


//! @brief Returns the last used plot data export directory
QString Configuration::getPlotDataDir()
{
    if(mPlotDataDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mPlotDataDir;
}


//! @brief Returns the last used plot graphics export directory
QString Configuration::getPlotGfxDir()
{
    if(mPlotGfxDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mPlotGfxDir;
}


//! @brief Returns the last used simulink export directory
QString Configuration::getSimulinkExportDir()
{
    if(mSimulinkExportDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mSimulinkExportDir;
}


//! @brief Returns the last used external subsystem directory
QString Configuration::getSubsystemDir()
{
    if(mSubsystemDir.isEmpty())
    {
        return gpDesktopHandler->getModelsPath();
    }
    return mSubsystemDir;
}


//! @brief Returns the last used modelica model directory
QString Configuration::getModelicaModelsDir()
{
    if(mModelicaModelsDir.isEmpty())
    {
        return gpDesktopHandler->getModelsPath();
    }
    return mModelicaModelsDir;
}


//! @brief Returns the last used external library directory
QString Configuration::getExternalLibDir()
{
    if(mExternalLibDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mExternalLibDir;
}


//! @brief Returns the last used scripts directory
QString Configuration::getScriptDir()
{
    if(mScriptDir.isEmpty())
    {
        return gpDesktopHandler->getScriptsPath();
    }
    return mScriptDir;
}


//! @brief Returns the last used scripts directory
QString Configuration::getPlotWindowDir()
{
    if(mPlotWindowDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mPlotWindowDir;
}


//! @brief Returns the last used directory for importing FMUs
QString Configuration::getFmuImportDir()
{
    if(mFmuImportDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mFmuImportDir;
}

QStringList Configuration::getTerminalHistory()
{
    return mTerminalHistory;
}

QString Configuration::getHcomWorkingDirectory() const
{
    return mHcomWorkingDirectory;
}

//! @brief Returns the last used directory for importing FMUs
QString Configuration::getFmuExportDir()
{
    if(mFmuExportDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mFmuExportDir;
}

QString Configuration::getLabViewExportDir()
{
    if(mLabViewExportDir.isEmpty())
    {
        return gpDesktopHandler->getDocumentsPath();
    }
    return mLabViewExportDir;
}

QString Configuration::getGcc32Dir()
{
    return mGcc32Dir;
}

QString Configuration::getGcc64Dir()
{
    return mGcc64Dir;
}

int Configuration::getParallelAlgorithm()
{
    return mParallelAlgorighm;
}


//! @brief Set function for library style option
//! @param value Desired setting
void Configuration::setLibraryStyle(int value)
{
    this->mLibraryStyle = value;
    saveToXml();
}


void Configuration::setShowPopupHelp(bool value)
{
    this->mShowPopupHelp = value;
    saveToXml();
}


void Configuration::setUseNativeStyleSheet(bool value)
{
    this->mUseNativeStyleSheet = value;
    saveToXml();
}

//! @brief Set function for invert wheel option
//! @param value Desired setting
void Configuration::setInvertWheel(bool value)
{
    this->mInvertWheel = value;
    saveToXml();
}


//! @brief Set function for multi-threading option
//! @param value Desired setting
void Configuration::setUseMultiCore(bool value)
{
    this->mUseMulticore = value;
    saveToXml();
}


//! @brief Set function for number of simulation threads
//! @param value Desired number of threads
void Configuration::setNumberOfThreads(size_t value)
{
    this->mNumberOfThreads = value;
    saveToXml();
}


//! @brief Set function for progress bar time step
//! @param value Desired step
void Configuration::setProgressBarStep(int value)
{
    this->mProgressBarStep = value;
    saveToXml();
}


//! @brief Set function for use progress bar setting
//! @param value Desired setting
void Configuration::setEnableProgressBar(bool value)
{
    this->mEnableProgressBar = value;
    saveToXml();
}


//! @brief Set function for background color setting
//! @param value Desired color
void Configuration::setBackgroundColor(const QColor &value)
{
    this->mBackgroundColor = value;
    saveToXml();
}


//! @brief Set function for anti-aliasing setting
//! @param value Desired setting
void Configuration::setAntiAliasing(const bool &value)
{
    this->mAntiAliasing = value;
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
        this->mUserLibs.append(file);
        this->mUserLibTypes.append(type);
    }
    saveToXml();
}


//! @brief Removes a user library from the library list
//! @param value Path to the library that is to be removed
void Configuration::removeUserLib(const QString &value)
{
    QFileInfo file(value);
    for(int i=0; i<mUserLibs.size(); ++i)
    {
        if(mUserLibs[i] == file)
        {
            mUserLibs.removeAt(i);
            mUserLibTypes.removeAt(i);
            --i;
        }
    }
    saveToXml();
}


//! @brief Tells whether or not a specified user library exist in the library list
//! @param value Path to the library
bool Configuration::hasUserLib(const QString &value) const
{
    QFileInfo file(value);
    return mUserLibs.contains(file);
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


//! @brief Set function for connector snapping setting
//! @param value Desired setting
void Configuration::setSnapping(bool value)
{
    this->mSnapping = value;
    saveToXml();
}


//! @brief Set function for automatically setting PWD to MWD in HCOM
void Configuration::setAutoSetPwdToMwd(const bool value)
{
    mSetPwdToMwd = value;
    saveToXml();
}

void Configuration::setPlotWindowsOnTop(const bool value)
{
    mPlotWindowsOnTop = value;
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


void Configuration::setGroupMessagesByTag(bool value)
{
    mGroupMessagesByTag = value;
    saveToXml();
}


void Configuration::setGenerationLimit(int value)
{
    mGenerationLimit = value;
    saveToXml();
}

void Configuration::setCacheLogData(const bool value)
{
    mCacheLogData = value;
    saveToXml();
}

void Configuration::setAutoBackup(const bool value)
{
    mAutoBackup = value;
    saveToXml();
}

void Configuration::setAutoLimitLogDataGenerations(const bool value)
{
    mAutoLimitLogDataGenerations = value;
}

void Configuration::setShowHiddenNodeDataVariables(const bool value)
{
    mShowHiddenNodeDataVariables = value;
}


void Configuration::setLoadModelDir(QString value)
{
    mLoadModelDir = value;
}


void Configuration::setModelGfxDir(QString value)
{
    mModelGfxDir = value;
}


void Configuration::setPlotDataDir(QString value)
{
    mPlotDataDir = value;
}


void Configuration::setPlotGfxDir(QString value)
{
    mPlotGfxDir = value;
}


void Configuration::setSimulinkExportDir(QString value)
{
    mSimulinkExportDir = value;
}


void Configuration::setSubsystemDir(QString value)
{
    mSubsystemDir = value;
}

void Configuration::setModelicaModelsDir(QString value)
{
    mModelicaModelsDir = value;
}

void Configuration::setExternalLibDir(QString value)
{
    mExternalLibDir = value;
}

void Configuration::setScriptDir(QString value)
{
    mScriptDir = value;
}

void Configuration::setPlotWindowDir(QString value)
{
    mPlotWindowDir = value;
}

void Configuration::storeTerminalHistory(QStringList value)
{
    mTerminalHistory = value;
}

void Configuration::setHcomWorkingDirectory(QString value)
{
    mHcomWorkingDirectory = value;
}

void Configuration::setFmuImportDir(QString value)
{
    mFmuImportDir = value;
}

void Configuration::setFmuExportDir(QString value)
{
    mFmuExportDir = value;
}

void Configuration::setLabViewExportDir(QString value)
{
    mLabViewExportDir = value;
}

void Configuration::setGcc32Dir(QString value)
{
    mGcc32Dir = value;
}

void Configuration::setGcc64Dir(QString value)
{
    mGcc64Dir = value;
}

void Configuration::setPlotGfxImageFormat(QString value)
{
    mPlotGfxImageFormat = value;
}

void Configuration::setPlotGfxDimensionsUnit(QString value)
{
    mPlotGfxDimensionsUnit = value;
}

void Configuration::setPlotGfxDPI(double value)
{
    mPlotGfxDPI = value;
}

void Configuration::setPlotGfxSize(QSizeF value)
{
    mPlotGfxSize = value;
}

void Configuration::setPlotGfxKeepAspect(bool value)
{
    mPlotGfxKeepAspect = value;
}

void Configuration::setPlotGfxUseScreenSize(bool value)
{
    mPlotGfxUseScreenSize = value;
}

void Configuration::setParallelAlgorithm(int value)
{
    mParallelAlgorighm = value;
}
