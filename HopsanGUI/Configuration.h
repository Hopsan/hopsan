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

namespace cfg {
    constexpr auto hopsanconfig = "hopsanconfig";
    constexpr auto settings = "settings";
    constexpr auto librarystyle = "librarystyle";
    constexpr auto ploexportversion = "ploexportversion";
    constexpr auto checkfordevelopmentupdates = "checkfordevelopmentupdates";
    constexpr auto showlicenseonstartup = "showlicenseonstartup2";
    constexpr auto showhiddennodedatavariables = "showhiddennodedatavariables";
    constexpr auto showpopuphelp = "showpopuphelp";
    constexpr auto nativestylesheet = "nativestylesheet";
    constexpr auto antialiasing = "antialiasing";
    constexpr auto invertwheel = "invertwheel";
    constexpr auto zoomstep = "zoomstep";
    constexpr auto snapping = "snapping";
    constexpr auto progressbar = "progressbar";
    constexpr auto progressbarstep = "progressbar_step";
    constexpr auto multicore = "multicore";
    constexpr auto numberofthreads = "numberofthreads";
    constexpr auto togglenamesbuttonchecked = "togglenamesbuttonchecked";
    constexpr auto toggleportsbuttonchecked = "toggleportsbuttonchecked";
    constexpr auto groupmessagesbytag = "groupmessagesbytag";
    constexpr auto generationlimit = "generationlimit";
    constexpr auto cachelogdata = "cachelogdata";
    constexpr auto autobackup = "autobackup";
    constexpr auto autolimitgenerations = "autolimitgenerations";
    constexpr auto setpwdtomwd = "setpwdtomwd";
    constexpr auto plotwindowsontop = "plotwindowsontop";
    constexpr auto logduringsimulation = "logduringsimulation";
    constexpr auto logsteps = "logsteps";
    constexpr auto plotgfximageformat = "plotgfximageformat";
    constexpr auto plotgfxdimensionunit = "plotgfxdimensionsunit";
    constexpr auto plotgfxdpi = "plotgfxdpi";
    constexpr auto plotgfxsize = "plotgfxsize";
    constexpr auto plotgfxkeepaspect = "plotgfxkeepaspect";
    constexpr auto plotgfxusescreensize = "plotgfxusescreensize";
    constexpr auto backgroundcolor = "backgroundcolor";
    namespace dir {
        constexpr auto parameterexport = "parameterexportdir";
        constexpr auto parameterimport = "parameterimportdir";
        constexpr auto loadmodel = "loadmodeldir";
        constexpr auto loadscript = "loadmodeldir";
        constexpr auto modelgfx = "modelgfxdir";
        constexpr auto plotdata = "plotdatadir";
        constexpr auto plotgfx = "plotgfxdir";
        constexpr auto simulinkexport = "simulinkexportdir";
        constexpr auto subsystem = "subsystemdir";
        constexpr auto externallib = "externallibdir";
        constexpr auto script = "scriptdir";
        constexpr auto plotwindow = "plotwindowdir";
        constexpr auto fmuimport = "fmuimportdir";
        constexpr auto fmuexport = "fmuexportdir";
        constexpr auto exeexport = "exeexportdir";
        constexpr auto labviewexport = "labviewexportdir";
        constexpr auto gcc32 = "gcc32dir";
        constexpr auto gcc64 = "gcc64dir";
        constexpr auto customtemppath = "customtemppath";
    }
    constexpr auto preferincludedcompiler = "preferincludedcompiler";
    constexpr auto remotehopsanaddress = "remotehopsanaddress";
    constexpr auto remotehopsanaddresserveraddress = "remotehopsanaddresserveraddress";
    constexpr auto useremoteaddresserver = "useremotehopsanaddressserver";
    constexpr auto useremoteoptimization = "useremoteoptimization";
    constexpr auto remotehopsanuseridentification = "remotehopsanuserid";
    constexpr auto remoteshorttimeout = "remoteshorttimeout";
    constexpr auto remotelongtimeout = "remotelongtimeout";
    constexpr auto penstyle = "penstyle";
    constexpr auto type = "type";
    constexpr auto gfxtype = "gfxtype";
    constexpr auto situation = "situation";
    constexpr auto color = "color";
    constexpr auto width = "width";
    constexpr auto capstyle = "capstyle";
    constexpr auto style = "style";
    constexpr auto unitsettings = "unitsettings";
    constexpr auto quantity = "quantity";
    constexpr auto baseunit = "baseunit";
    constexpr auto unit = "unit";
    constexpr auto units = "units";
    constexpr auto defaultdisplayunits = "defaultdisplayunit";
    constexpr auto name = "name";
    constexpr auto customunit = "customunit";
    constexpr auto scale = "scale";
    constexpr auto offset = "offset";
    constexpr auto frombaseexpr = "frombaseexpr";
    constexpr auto tobaseexpr = "tobaseexpr";
    constexpr auto hcom = "hcom";
    constexpr auto pwd = "pwd";
    constexpr auto command = "command";
    constexpr auto libs = "libs";
    constexpr auto userlib = "userlib";
    namespace libtype {
        constexpr auto root = "libtype";
        constexpr auto internal = "internal";
        constexpr auto external = "external";
        constexpr auto fmu = "fmu";
    }
    namespace models {
        constexpr auto root = "models";
        constexpr auto lastsession = "lastsessionmodel";
        constexpr auto recent = "recentmodel";
        constexpr auto recentgenerator = "recentgeneratormodel";
    }
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
