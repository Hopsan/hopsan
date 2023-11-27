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

#define CFG_CHECKFORDEVELOPMENTUPDATES "checkfordevelopmentupdates"
#define CFG_SHOWLICENSEONSTARTUP "showlicenseonstartup2"
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
#define CFG_LOGDURINGSIMULATION "logduringsimulation"
#define CFG_LOGSTEPS "logsteps"

#define CFG_PLOTGFXIMAGEFORMAT "plotgfximageformat"
#define CFG_PLOTGFXDIMENSIONSUNIT "plotgfxdimensionsunit"
#define CFG_PLOTGFXDPI "plotgfxdpi"
#define CFG_PLOTGFXSIZE "plotgfxsize"
#define CFG_PLOTGFXKEEPASPECT "plotgfxkeepaspect"
#define CFG_PLOTGFXUSESCREENSIZE "plotgfxusescreensize"
#define CFG_BACKGROUNDCOLOR "backgroundcolor"

#define CFG_PARAMETEREXPORTDIR "parameterexportdir"
#define CFG_PARAMETERIMPORTDIR "parameterimportdir"
#define CFG_LOADMODELDIR "loadmodeldir"
#define CFG_LOADSCRIPTDIR "loadmodeldir"
#define CFG_MODELGFXDIR "modelgfxdir"
#define CFG_PLOTDATADIR "plotdatadir"
#define CFG_PLOTGFXDIR "plotgfxdir"
#define CFG_SIMULINKEXPORTDIR "simulinkexportdir"
#define CFG_SUBSYSTEMDIR "subsystemdir"
#define CFG_EXTERNALLIBDIR "externallibdir"
#define CFG_SCRIPTDIR "scriptdir"
#define CFG_PLOTWINDOWDIR "plotwindowdir"
#define CFG_FMUDIR "fmudir"
#define CFG_FMUIMPORTDIR "fmuimportdir"
#define CFG_FMUEXPORTDIR "fmuexportdir"
#define CFG_EXEEXPORTDIR "exeexportdir"
#define CFG_LABVIEWEXPORTDIR "labviewexportdir"
#define CFG_PREFERINCLUDEDCOMPILER "preferincludedcompiler"
#define CFG_GCC32DIR "gcc32dir"
#define CFG_GCC64DIR "gcc64dir"
#define CFG_CUSTOMTEMPPATH "customtemppath"

#define CFG_REMOTEHOPSANADDRESS "remotehopsanaddress"
#define CFG_REMOTEHOPSANADDRESSSERVERADDRESS "remotehopsanaddressserveraddress"
#define CFG_USEREMOTEADDRESSSERVER "useremotehopsanaddressserver"
#define CFG_USEREMOTEOPTIMIZATION "useremoteoptimization"
#define CFG_REMOTEHOPSANUSERIDENTIFICATION "remotehopsanuserid"
#define CFG_REMOTESHORTTIMEOUT "remoteshorttimeout"
#define CFG_REMOTELONGTIMEOUT "remotelongtimeout"

#define CFG_PENSTYLE "penstyle"
#define CFG_TYPE "type"
#define CFG_GFXTYPE "gfxtype"
#define CFG_SITUATION "situation"
#define CFG_COLOR "color"
#define CFG_WIDTH "width"
#define CFG_CAPSTYLE "capstyle"

#define CFG_UNITSETTINGS "unitsettings"
#define CFG_QUANTITY "quantity"

namespace cfg {
    namespace siunits {
        constexpr auto kg = "kg";
        constexpr auto m = "m";
        constexpr auto s = "s";
        constexpr auto A = "A";
        constexpr auto K = "K";
        constexpr auto mol = "mol";
        constexpr auto cd = "cd";
        constexpr auto rad = "rad";
    }
}

#define CFG_BASEUNIT "baseunit"
#define CFG_UNIT "unit"
#define CFG_UNITS "units"
#define CFG_DEFAULTDISPALYUNIT "defaultdisplayunit"
#define CFG_NAME "name"
#define CFG_UNIT "unit"
#define CFG_CUSTOMUNIT "customunit"
#define CFG_SCALE "scale"
#define CFG_OFFSET "offset"
#define CFG_FROMBASEEXPR "frombaseexpr"
#define CFG_TOBBASEEXPR "tobaseexpr"

#define CFG_HCOM "hcom"
#define CFG_PWD "pwd"
#define CFG_COMMAND "command"

namespace cfg {
    namespace paths {
        constexpr auto corelogfile = "corelogfile";
    }
}

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

    // Compiler settings
    QString getGCCPath() const;
    QString getCompilerPath(const ArchitectureEnumT arch);

    // Style and appearance related methods
    QColor getBackgroundColor() const;
    void setBackgroundColor(const QColor &value);
    QPalette getPalette() const;
    QFont getFont() const;
    QString getStyleSheet() const;
    QPen getPen(ConnectorStyleEnumT style, GraphicsTypeEnumT gfxType, QString situation) const;

    // Libraries
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
    void getUnitScales(const QString &rQuantity, QList<UnitConverter> &rUnitScales);
    bool hasUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    double getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit) const;
    void getUnitScale(const QString &rPhysicalQuantity, const QString &rUnit, UnitConverter &rUnitScale) const;
    UnitConverter getUnitScaleUC(const QString &rPhysicalQuantity, const QString &rUnit) const;
    QStringList getQuantitiesForUnit(const QString &rUnit) const;
    QString getBaseUnit(const QString &rQuantity);
    void getBaseUnitSIExponents(const QString &rQuantity, int &rKg, int &rM, int &rS, int &rA, int &rK, int &rMol, int &rCd, int &rRad);
    bool isRegisteredBaseUnit(const QString &rUnitName) const;
    void removeUnitScale(const QString &rQuantity, const QString &rUnit);
    bool haveQuantity(const QString &rQuantity) const;


    // Terminal and scripts
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
    void loadModelSettings(QDomElement &rDomElement);
    void loadScriptSettings(QDomElement &rHcomElement);
    void refreshQuickAccessVariables();
    void refreshIfDesktopPath(const QString &cfgKey);

    class QuantityUnitScale
    {
    public:
        QString baseunit;
        int kg;
        int m;
        int s;
        int A;
        int K;
        int mol;
        int cd;
        int rad;
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

    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QStringList mRecentGeneratorModels;

    QMap<QString, QString> mSelectedDefaultUnits;
    QMap<QString, QuantityUnitScale> mUnitScales;

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
