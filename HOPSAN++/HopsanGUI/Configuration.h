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

#include "common.h"

class MainWindow;

class Configuration
{

public:
    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();

    int getLibraryStyle();
    bool getShowWelcomeDialog();
    bool getShowPopupHelp();
    bool getUseNativeStyleSheet();
    bool getInvertWheel();
    bool getUseMulticore();
    size_t getNumberOfThreads();
    bool getToggleNamesButtonCheckedLastSession();
    bool getTogglePortsButtonCheckedLastSession();
    int getProgressBarStep();
    bool getEnableProgressBar();
    QColor getBackgroundColor();
    bool getAntiAliasing();
    QStringList getUserLibs();
    bool getSnapping();
    QStringList getRecentModels();
    QStringList getLastSessionModels();
    QString getDefaultUnit(QString key);
    QMap<QString, double> getCustomUnits(QString key);
    QPen getPen(connectorStyle style, graphicsType gfxType, QString situation);
    QPalette getPalette();
    QFont getFont();
    QString getStyleSheet();
    QString getLastScriptFile();
    bool getGroupMessagesByTag();
    int getGenerationLimit();
    QString getLoadModelDir();
    QString getModelGfxDir();
    QString getPlotDataDir();
    QString getPlotGfxDir();
    QString getSimulinkExportDir();
    QString getSubsystemDir();
    QString getModelicaModelsDir();

    void setLibraryStyle(int value);
    void setShowWelcomeDialog(bool value);
    void setShowPopupHelp(bool value);
    void setUseNativeStyleSheet(bool value);
    void setInvertWheel(bool value);
    void setUseMultiCore(bool value);
    void setNumberOfThreads(size_t value);
    void setProgressBarStep(int value);
    void setEnableProgressBar(bool value);
    void setBackgroundColor(QColor value);
    void setAntiAliasing(bool value);
    void addUserLib(QString value);
    void removeUserLib(QString value);
    bool hasUserLib(QString value);
    void setSnapping(bool value);
    void addRecentModel(QString value);
    void addLastSessionModel(QString value);
    void clearLastSessionModels();
    void setDefaultUnit(QString key, QString value);
    void addCustomUnit(QString dataname, QString unitname, double scale);
    void setLastScriptFile(QString file);
    void setGroupMessagesByTag(bool value);
    void setGenerationLimit(int value);
    void setLoadModelDir(QString value);
    void setModelGfxDir(QString value);
    void setPlotDataDir(QString value);
    void setPlotGfxDir(QString value);
    void setSimulinkExportDir(QString value);
    void setSubsystemDir(QString value);
    void setModelicaModelsDir(QString value);

private:
    int mLibraryStyle;
    bool mShowWelcomeDialog;
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
    bool mSnapping;
    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QMap<QString, QString> mDefaultUnits;
    QMap< QString, QMap<QString, double> > mCustomUnits;
    QPalette mPalette;
    QFont mFont;
    QString mStyleSheet;
    QString mLastScriptFile;
    bool mGroupMessagesByTag;
    int mGenerationLimit;
    QString mLoadModelDir;
    QString mModelGfxDir;
    QString mPlotDataDir;
    QString mPlotGfxDir;
    QString mSimulinkExportDir;
    QString mSubsystemDir;
    QString mModelicaModelsDir;

    QMap < connectorStyle, QMap< QString, QMap<QString, QPen> > > mPenStyles;
};

#endif // CONFIGURATION_H
