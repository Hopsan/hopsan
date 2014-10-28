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

#define CFG_HOPSANCONFIG "hopsanconfig"
#define CFG_SETTINGS "settings"

#define CFG_LIBRARYSTYLE "librarystyle"
#define CFG_PLOEXPORTVERSION "ploexportversion"

#define CFG_SHOWHIDDENNODEDATAVARIABLES "showhiddennodedatavariables"
#define CFG_SHOWPOPUPHELP "showpopuphelp"
#define CFG_NATIVESTYLESHEET "nativestylesheet"
#define CFG_ANTIALIASING "antialiasing"
#define CFG_INVERTWHEEL "invertwheel"
#define CFG_SNAPPING "snapping"
#define CFG_PROGRESSBAR "progressbar"
#define CFG_PROGRESSBARSTEP "progressbar_step"
#define CFG_MULTICORE "multicore"
#define CFG_NUMBEROFTHREADS "numberofthreads"
#define CFG_TOGGLENAMESBUTTONCHECKED "togglenamesbuttonchecked"
#define CFG_TOGGLEPORTSBUTTONCHECKED "toggleportsbuttonchecked"
#define CFG_GROUPMESSAGESBYTAG "groupmessagesbytag"
#define CFG_GENERATIONLIMIT "generationlimit"
#define CFG_CACHELOGDATA "cachelogdata"
#define CFG_AUTOBACKUP "autobackup"
#define CFG_AUTOLIMITGENERATIONS "autolimitgenerations"
#define CFG_SETPWDTOMWD "setpwdtomwd"

#define CFG_PLOTGFXIMAGEFORMAT "plotgfximageformat"
#define CFG_PLOTGFXDIMENSIONSUNIT "plotgfxdimensionsunit"
#define CFG_PLOTGFXDPI "plotgfxdpi"
#define CFG_PLOTGFXSIZE "plotgfxsize"
#define CFG_PLOTGFXKEEPASPECT "plotgfxkeepaspect"
#define CFG_PLOTGFXUSESCREENSIZE "plotgfxusescreensize"
#define CFG_BACKGROUNDCOLOR "backgroundcolor"

#define CFG_LOADMODELDIR "loadmodeldir"
#define CFG_MODELGFXDIR "modelgfxdir"
#define CFG_PLOTDATADIR "plotdatadir"
#define CFG_PLOTGFXDIR "plotgfxdir"
#define CFG_SIMULINKEXPORTDIR "simulinkexportdir"
#define CFG_SUBSYSTEMDIR "subsystemdir"
#define CFG_MODELICAMODELSDIR "modelicamodelsdir"
#define CFG_EXTERNALLIBDIR "externallibdir"
#define CFG_SCRIPTDIR "scriptdir"
#define CFG_PLOTWINDOWDIR "plotwindowdir"
#define CFG_FMUDIR "fmudir"
#define CFG_FMUIMPORTDIR "fmuimportdir"
#define CFG_FMUEXPORTDIR "fmuexportdir"
#define CFG_LABVIEWEXPORTDIR "labviewexportdir"

#define CFG_PENSTYLE "penstyle"
#define CFG_TYPE "type"
#define CFG_GFXTYPE "gfxtype"
#define CFG_SITUATION "situation"
#define CFG_COLOR "color"
#define CFG_WIDTH "width"
#define CFG_CAPSTYLE "capstyle"

#define CFG_UNITSCALES "unitscales"
#define CFG_QUANTITY "quantity"

#define CFG_SIUNIT "siunit"
#define CFG_UNITSCALE "unitscale"
#define CFG_UNIT "unit"
#define CFG_UNITS "units"
#define CFG_DEFAULTUNIT "defaultunit"
#define CFG_NAME "name"
#define CFG_UNIT "unit"
#define CFG_CUSTOMUNIT "customunit"
#define CFG_SCALE "scale"

#define CFG_PYTHON "python"
#define CFG_HCOM "hcom"
#define CFG_PWD "pwd"
#define CFG_COMMAND "command"

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>
#include <QPen>
#include <QPalette>
#include <QFont>
#include <QDomElement>

#include "common.h"
#include "UnitScale.h"

class Configuration : public QObject
{
    Q_OBJECT

public:
    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();

    bool getShowPopupHelp() const;
    bool getInvertWheel() const;
    bool getToggleNamesButtonCheckedLastSession() const;
    bool getTogglePortsButtonCheckedLastSession() const;
    int getProgressBarStep() const;
    bool getEnableProgressBar() const;
    bool getSnapping() const;
    bool getAutoSetPwdToMwd() const;

    bool getUseMulticore() const;
    int getNumberOfThreads() const;

    int getLibraryStyle() const;
    bool getUseNativeStyleSheet() const;
    QColor getBackgroundColor() const;
    QPalette getPalette() const;
    QFont getFont() const;
    QString getStyleSheet() const;
    QPen getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, QString situation) const;
    bool getAntiAliasing() const;

    QStringList getUserLibs() const;
    QList<LibraryTypeEnumT> getUserLibTypes() const;

    QStringList getModelicaFiles() const;

    QStringList getRecentModels() const;
    QStringList getRecentGeneratorModels() const;
    QStringList getLastSessionModels() const;

    QString getLastPyScriptFile() const;

    QStringList getUnitQuantities() const;
    QString getDefaultUnit(const QString &rPhysicalQuantity) const;
    QMap<QString, double> getUnitScales(const QString &rPhysicalQuantity);
    void getUnitScales(const QString &rQuantity, QList<UnitScale> &rUnitScales);
    bool hasUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    double getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    void getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit, UnitScale &rUnitScale) const;
    QStringList getPhysicalQuantitiesForUnit(const QString &rUnit) const;
    QString getSIUnit(const QString &rQuantity);
    bool isRegisteredSIUnit(const QString &rUnitName) const;

    void removeUnitScale(const QString &rQuantity, const QString &rUnit);

    int getPLOExportVersion() const;
    bool getShowHiddenNodeDataVariables() const;

    bool getGroupMessagesByTag();
    QStringList getTerminalHistory();
    QString getHcomWorkingDirectory() const;
    int getGenerationLimit() const;
    bool getCacheLogData() const;
    bool getAutoBackup() const;

    bool getAutoLimitLogDataGenerations();

    QString getPlotGfxImageFormat();
    QString getPlotGfxDimensionsUnit();
    double getPlotGfxDPI();
    QSizeF getPlotGfxSize();
    bool getPlotGfxKeepAspect();
    bool getPlotGfxUseScreenSize();

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

    int getParallelAlgorithm();

    void setLibraryStyle(int value);
    void setShowPopupHelp(bool value);
    void setUseNativeStyleSheet(bool value);
    void setInvertWheel(bool value);
    void setUseMultiCore(bool value);
    void setNumberOfThreads(size_t value);
    void setProgressBarStep(int value);
    void setEnableProgressBar(bool value);
    void setBackgroundColor(const QColor &value);
    void setAntiAliasing(const bool &value);
    void addUserLib(const QString &value, LibraryTypeEnumT type);
    void removeUserLib(const QString &value);
    bool hasUserLib(const QString &value) const;
    void addModelicaFile(const QString &value);
    void setSnapping(const bool value);
    void setAutoSetPwdToMwd(const bool value);
    void addRecentModel(QString value);
    void removeRecentModel(QString value);
    void addRecentGeneratorModel(QString value);
    void addLastSessionModel(QString value);
    void clearLastSessionModels();
    void setDefaultUnit(QString key, QString value);
    void addCustomUnit(QString quantity, QString unitname, double scale);
    void setLastPyScriptFile(QString file);
    void setGroupMessagesByTag(bool value);
    void setGenerationLimit(int value);
    void setCacheLogData(const bool value);
    void setAutoBackup(const bool value);
    void setAutoLimitLogDataGenerations(const bool value);
    void setShowHiddenNodeDataVariables(const bool value);
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
    void setPlotGfxImageFormat(QString value);
    void setPlotGfxDimensionsUnit(QString value);
    void setPlotGfxDPI(double value);
    void setPlotGfxSize(QSizeF value);
    void setPlotGfxKeepAspect(bool value);
    void setPlotGfxUseScreenSize(bool value);


    void setParallelAlgorithm(int value);

private:
    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);
    void loadUnitSettings(QDomElement &rDomElement);
    void loadUnitScales(QDomElement &rDomElement);
    void loadLibrarySettings(QDomElement &rDomElement);
    void loadModelicaFilesSettings(QDomElement &rDomElement);
    void loadModelSettings(QDomElement &rDomElement);
    void loadScriptSettings(QDomElement &rPythonElement, QDomElement &rHcomElement);

    class QuantityUnitScale
    {
    public:
        QString siunit;
        QString selectedDefaultUnit;
        QMap<QString, UnitScale> customScales;
    };

    int mLibraryStyle;
    bool mShowPopupHelp;
    bool mUseNativeStyleSheet;
    bool mInvertWheel;
    bool mUseMulticore;
    int mNumberOfThreads;
    bool mToggleNamesButtonCheckedLastSession;
    bool mTogglePortsButtonCheckedLastSession;
    int mProgressBarStep;
    bool mEnableProgressBar;
    QColor mBackgroundColor;
    bool mAntiAliasing;
    QList<QFileInfo> mUserLibs;
    QList<LibraryTypeEnumT> mUserLibTypes;
    QList<QFileInfo> mModelicaFiles;
    bool mSnapping;
    bool mSetPwdToMwd;
    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QStringList mRecentGeneratorModels;
    QMap<QString, QString> mSelectedDefaultUnits;
    QMap<QString, QuantityUnitScale> mUnitScales;
    QPalette mPalette;
    QFont mFont;
    QString mStyleSheet;
    QString mLastPyScriptFile;
    bool mGroupMessagesByTag;
    int mGenerationLimit;
    bool mCacheLogData;
    bool mAutoBackup;
    bool mAutoLimitLogDataGenerations;
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
    int mPLOExportVersion;
    bool mShowHiddenNodeDataVariables;
    QString mPlotGfxImageFormat;
    QString mPlotGfxDimensionsUnit;
    double mPlotGfxDPI;
    QSizeF mPlotGfxSize;
    bool mPlotGfxKeepAspect;
    bool mPlotGfxUseScreenSize;

    int mParallelAlgorighm;

    QMap < ConnectorStyleEnumT, QMap< QString, QMap<QString, QPen> > > mPenStyles;

signals:
    void recentModelsListChanged();
};

#endif // CONFIGURATION_H
