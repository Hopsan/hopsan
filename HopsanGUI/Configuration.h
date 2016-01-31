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

#define CFG_SHOWLICENSEONSTARTUP "showlicenseonstartup"
#define CFG_SHOWHIDDENNODEDATAVARIABLES "showhiddennodedatavariables"
#define CFG_SHOWPOPUPHELP "showpopuphelp"
#define CFG_NATIVESTYLESHEET "nativestylesheet"
#define CFG_ANTIALIASING "antialiasing"
#define CFG_INVERTWHEEL "invertwheel"
#define CFG_ZOOMSTEP "zoomstep"
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
#define CFG_PLOTWINDOWSONTOP "plotwindowsontop"

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
#define CFG_PREFERINCLUDEDCOMPILER "preferincludedcompiler"
#define CFG_GCC32DIR "gcc32dir"
#define CFG_GCC64DIR "gcc64dir"

#define CFG_REMOTEHOPSANADDRESS "remotehopsanaddress"
#define CFG_REMOTEHOPSANADDRESSSERVERADDRESS "remotehopsanaddressserveraddress"
#define CFG_USEREMOTEADDRESSSERVER "useremotehopsanaddressserver"
#define CFG_USEREMOTEOPTIMIZATION "useremoteoptimization"

#define CFG_PENSTYLE "penstyle"
#define CFG_TYPE "type"
#define CFG_GFXTYPE "gfxtype"
#define CFG_SITUATION "situation"
#define CFG_COLOR "color"
#define CFG_WIDTH "width"
#define CFG_CAPSTYLE "capstyle"

#define CFG_UNITSETTINGS "unitsettings"
#define CFG_QUANTITY "quantity"

#define CFG_BASEUNIT "baseunit"
#define CFG_UNIT "unit"
#define CFG_UNITS "units"
#define CFG_DEFAULTDISPALYUNIT "defaultdisplayunit"
#define CFG_NAME "name"
#define CFG_UNIT "unit"
#define CFG_CUSTOMUNIT "customunit"
#define CFG_SCALE "scale"
#define CFG_OFFSET "offset"

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
    Configuration();

    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();
    void reset();

    void beginMultiSet();
    void endMultiSet();

    QString getStringSetting(const QString &rName) const;
    bool getBoolSetting(const QString &rName) const;
    int getIntegerSetting(const QString &rName) const;
    double getDoubleSetting(const QString &rName) const;

    void setStringSetting(const QString &rName, const QString &rValue);
    void setBoolSetting(const QString &rName, const bool value);
    void setIntegerSetting(const QString &rName, const int value);
    void setDoubleSetting(const QString &rName, const double value);

    // Quick access methods (to avoid cost of settings lookup)
    bool getShowPopupHelp() const;
    bool getInvertWheel() const;
    bool getCacheLogData() const;
    bool getUseMulticore() const;
    int getProgressBarStep() const;
    int getGenerationLimit() const;
    bool getSnapping() const;
    double getZoomStep() const;

    // Other settings
    QSizeF getPlotGfxSize();
    int getParallelAlgorithm();
    void setPlotGfxSize(const QSizeF size);
    void setParallelAlgorithm(int value);
    QString getGCCPath() const;

    // Style and appearance related methods
    QColor getBackgroundColor() const;
    void setBackgroundColor(const QColor &value);
    QPalette getPalette() const;
    QFont getFont() const;
    QString getStyleSheet() const;
    QPen getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, QString situation) const;

    // Libraries
    void addModelicaFile(const QString &value);
    QStringList getModelicaFiles() const;
    void addUserLib(const QString &value, LibraryTypeEnumT type);
    void removeUserLib(const QString &value);
    QStringList getUserLibs() const;
    QList<LibraryTypeEnumT> getUserLibTypes() const;

    // Models
    QStringList getRecentModels() const;
    void addRecentModel(QString value);
    void removeRecentModel(QString value);
    void addLastSessionModel(QString value);
    void clearLastSessionModels();
    QStringList getLastSessionModels() const;
    void addRecentGeneratorModel(QString value);
    QStringList getRecentGeneratorModels() const;


    // Units and scales
    QStringList getUnitQuantities() const;
    QString getDefaultUnit(const QString &rPhysicalQuantity) const;
    void setDefaultUnit(QString key, QString value);
    void addCustomUnit(QString quantity, QString unitname, QString scale, QString offset);
    QMap<QString, double> getUnitScales(const QString &rPhysicalQuantity);
    void getUnitScales(const QString &rQuantity, QList<UnitConverter> &rUnitScales);
    bool hasUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    double getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    void getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit, UnitConverter &rUnitScale) const;
    QStringList getQuantitiesForUnit(const QString &rUnit) const;
    QString getBaseUnit(const QString &rQuantity);
    bool isRegisteredBaseUnit(const QString &rUnitName) const;
    void removeUnitScale(const QString &rQuantity, const QString &rUnit);
    bool haveQuantity(const QString &rQuantity) const;


    // Terminal and scripts
    QString getLastPyScriptFile() const;
    void setLastPyScriptFile(QString file);

    QStringList getTerminalHistory();
    void storeTerminalHistory(QStringList value);

    QString getHcomWorkingDirectory() const;
    void setHcomWorkingDirectory(QString value);


private:
    void registerSettings();
    void loadUserSettings(QDomElement &rDomElement);
    void loadStyleSettings(QDomElement &rDomElement);
    void loadUnitSettings(QDomElement &rDomElement, bool tagAsBuiltIn);
    void loadLibrarySettings(QDomElement &rDomElement);
    void loadModelicaFilesSettings(QDomElement &rDomElement);
    void loadModelSettings(QDomElement &rDomElement);
    void loadScriptSettings(QDomElement &rPythonElement, QDomElement &rHcomElement);
    void refreshQuickAccessVariables();

    class QuantityUnitScale
    {
    public:
        QString baseunit;
        QString selectedDefaultUnit;
        QStringList builtInUnitconversions;
        QMap<QString, UnitConverter> customUnits;
    };

    QColor mBackgroundColor;
    QPalette mPalette;
    QFont mFont;
    QString mStyleSheet;

    QList<QFileInfo> mUserLibs;
    QList<LibraryTypeEnumT> mUserLibTypes;
    QList<QFileInfo> mModelicaFiles;

    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QStringList mRecentGeneratorModels;

    QMap<QString, QString> mSelectedDefaultUnits;
    QMap<QString, QuantityUnitScale> mUnitScales;

    QString mLastPyScriptFile;
    QStringList mTerminalHistory;
    QString mHcomWorkingDirectory;

    QSizeF mPlotGfxSize;
    int mParallelAlgorighm;

    bool mWriteOnSave = true;

    // Settings variable maps
    QMap<QString, QString> mStringSettings;
    QMap<QString, bool> mBoolSettings;
    QMap<QString, int> mIntegerSettings;
    QMap<QString, double> mDoubleSettings;

    QMap < ConnectorStyleEnumT, QMap< QString, QMap<QString, QPen> > > mPenStyles;

    // Quick access settings variables
    bool mInvertWheel;
    bool mShowPopupHelp;
    bool mCacheLogData;
    bool mUseMulticore;
    int mProgressBarStep;
    int mGenerationLimit;
    bool mSnapping;
    double mZoomStep;

signals:
    void recentModelsListChanged();
};

#endif // CONFIGURATION_H
