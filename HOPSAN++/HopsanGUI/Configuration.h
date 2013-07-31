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
//! @file   Configuration.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for the configuration object
//!
//$Id$

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>
#include <QPen>
#include <QPalette>
#include <QFont>
#include <QDomElement>

#include "common.h"

class MainWindow;

class Configuration
{

public:
    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();

    bool getShowPopupHelp();
    bool getInvertWheel();
    bool getToggleNamesButtonCheckedLastSession();
    bool getTogglePortsButtonCheckedLastSession();
    int getProgressBarStep();
    bool getEnableProgressBar();
    bool getSnapping();

    bool getUseMulticore();
    size_t getNumberOfThreads();

    int getLibraryStyle();
    bool getUseNativeStyleSheet();
    QColor getBackgroundColor();
    QPalette getPalette();
    QFont getFont();
    QString getStyleSheet();
    QPen getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, QString situation);
    bool getAntiAliasing();

    QStringList getUserLibs();
    QStringList getUserLibFolders();

    QStringList getRecentModels();
    QStringList getRecentGeneratorModels();
    QStringList getLastSessionModels();
    bool getAlwaysLoadLastSession();

    QString getLastScriptFile();
    QString getInitScript();

    QString getDefaultUnit(QString key) const;
    QMap<QString, double> getCustomUnits(QString key);
    bool hasUnitScale(const QString key, const QString unit) const;
    double getUnitScale(const QString key, const QString unit) const;

    bool getGroupMessagesByTag();
    QStringList getTerminalHistory();
    QString getHcomWorkingDirectory() const;
    int getGenerationLimit() const;
    bool getCacheLogData() const;

    QString getLoadModelDir();
    QString getModelGfxDir();
    QString getPlotDataDir();
    QString getPlotGfxDir();
    QString getSimulinkExportDir();
    QString getSubsystemDir();
    QString getModelicaModelsDir();
    QString getExternalLibDir();
    QString getScriptDir();
    QString getPlotWindowDir();
    QString getFmuImportDir();
    QString getFmuExportDir();
    QString getLabViewExportDir();

    void setLibraryStyle(int value);
    void setAlwaysLoadLastSession(bool value);
    void setShowPopupHelp(bool value);
    void setUseNativeStyleSheet(bool value);
    void setInvertWheel(bool value);
    void setUseMultiCore(bool value);
    void setNumberOfThreads(size_t value);
    void setProgressBarStep(int value);
    void setEnableProgressBar(bool value);
    void setBackgroundColor(QColor value);
    void setAntiAliasing(bool value);
    void addUserLib(QString value, QString libName="");
    void removeUserLib(QString value);
    bool hasUserLib(QString value) const;
    void setSnapping(bool value);
    void addRecentModel(QString value);
    void removeRecentModel(QString value);
    void addRecentGeneratorModel(QString value);
    void addLastSessionModel(QString value);
    void clearLastSessionModels();
    void setDefaultUnit(QString key, QString value);
    void addCustomUnit(QString dataname, QString unitname, double scale);
    void setLastScriptFile(QString file);
    void setInitScript(QString script);
    void setGroupMessagesByTag(bool value);
    void setGenerationLimit(int value);
    void setCacheLogData(const bool value);
    void setLoadModelDir(QString value);
    void setModelGfxDir(QString value);
    void setPlotDataDir(QString value);
    void setPlotGfxDir(QString value);
    void setSimulinkExportDir(QString value);
    void setSubsystemDir(QString value);
    void setModelicaModelsDir(QString value);
    void setExternalLibDir(QString value);
    void setScriptDir(QString value);
    void setPlotWindowDir(QString value);
    void storeTerminalHistory(QStringList value);
    void setHcomWorkingDirectory(QString value);
    void setFmuImportDir(QString value);
    void setFmuExportDir(QString value);
    void setLabViewExportDir(QString value);

private:
    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);
    void loadUnitSettings(QDomElement &rDomElement);
    void loadLibrarySettings(QDomElement &rDomElement);
    void loadModelSettings(QDomElement &rDomElement);
    void loadScriptSettings(QDomElement &rPythonElement, QDomElement &rHcomElement);

    int mLibraryStyle;
    bool mAlwaysLoadLastSession;
    bool mShowPopupHelp;
    bool mUseNativeStyleSheet;
    bool mInvertWheel;
    bool mUseMulticore;
    size_t mNumberOfThreads;
    bool mToggleNamesButtonCheckedLastSession;
    bool mTogglePortsButtonCheckedLastSession;
    int mProgressBarStep;
    bool mEnableProgressBar;
    QColor mBackgroundColor;
    bool mAntiAliasing;
    QStringList mUserLibs;
    QStringList mUserLibFolders;
    bool mSnapping;
    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QStringList mRecentGeneratorModels;
    QMap<QString, QString> mDefaultUnits;
    QMap< QString, QMap<QString, double> > mCustomUnits;
    QPalette mPalette;
    QFont mFont;
    QString mStyleSheet;
    QString mLastScriptFile;
    QString mInitScript;
    bool mGroupMessagesByTag;
    int mGenerationLimit;
    bool mCacheLogData;
    QString mLoadModelDir;
    QString mModelGfxDir;
    QString mPlotDataDir;
    QString mPlotGfxDir;
    QString mSimulinkExportDir;
    QString mSubsystemDir;
    QString mModelicaModelsDir;
    QString mExternalLibDir;
    QString mScriptDir;
    QString mPlotWindowDir;
    QStringList mTerminalHistory;
    QString mHcomWorkingDirectory;
    QString mFmuImportDir;
    QString mFmuExportDir;
    QString mLabViewExportDir;

    QMap < ConnectorStyleEnumT, QMap< QString, QMap<QString, QPen> > > mPenStyles;
};

#endif // CONFIGURATION_H
